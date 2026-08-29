#!/usr/bin/env bash
# Installe dans un préfixe temporaire, puis lance ce qui vient d'y être déposé.
#
# **Le contour de la phase 7 est « ce qui manque pour qu'un tiers installe et
# utilise l'outil », et aucun test unitaire ne dit cela.** Tout ce que le projet
# éprouve par ailleurs s'exécute depuis l'arbre de construction, où les fichiers
# sont présents par accident de disposition : ils sont là parce que le dépôt les
# contient, pas parce qu'une règle les a copiés.
#
# La faute la plus commune d'un premier empaquetage est justement celle-là — un
# fichier de données que le binaire cherche à côté de lui et que personne n'a
# copié. Elle est invisible depuis l'arbre de construction et ne se voit qu'à la
# première installation propre. C'est celle-ci.
#
# ## Ce qu'il vérifie
#
#   1. les deux binaires s'installent, se lancent, et disent la bonne version ;
#   2. **chaque page du manuel du dépôt se retrouve sous le préfixe** — la liste
#      attendue est calculée depuis `docs/manual`, jamais recopiée : une liste
#      écrite à la main se périme au premier chapitre ajouté, en silence ;
#   3. le préfixe temporaire ne laisse rien derrière lui.
#
# ## Ce qu'il ne vérifie pas, et il vaut mieux l'écrire
#
# **Rien du `.deb` ni du `.rpm`** : les paquets natifs n'existent pas encore —
# ADR 0023, issue #244 — et ce script ne regarde qu'une installation par
# `cmake --install`. Quand ils existeront, deux contrôles s'ajouteront ici :
#
# | Vérifiable | Comment |
# | :--------- | :------ |
# | la liste des fichiers du `.rpm` | `rpm -qlp` |
# | ses dépendances déclarées | `rpm -qp --requires` |
# | l'accord des deux paquets | la même liste, confrontée à celle du `.deb` |
#
# La dernière est la plus utile, et c'est la seule qui ne demande aucune Fedora :
# les deux paquets sortent de la même installation, donc leurs listes de fichiers
# doivent coïncider, et un écart entre elles est un défaut des règles
# `install()`. **Ce qu'aucune machine Ubuntu ne peut prouver reste entier** : un
# `.rpm` construit ici ne peut pas y être installé, donc « il s'installe » n'est
# pas une phrase que ce dépôt sait vérifier aujourd'hui. Il faudrait un
# conteneur Fedora — dépendance lourde pour la porte, à rouvrir le jour où
# quelqu'un installera vraiment le paquet.
#
# **Rien des fichiers de données du paquet** — `.desktop`, icône, AppStream. Ils
# n'existent pas dans le dépôt et viennent avec #244. `desktop-file-validate` et
# `appstreamcli validate` les valideront alors, et c'est à ce moment-là que les
# paquets `desktop-file-utils` et `appstream` entreront dans
# `setup-toolchain.sh` : l'ADR 0004 demande qu'une dépendance se justifie, et
# une dépendance installée pour valider des fichiers qui n'existent pas ne se
# justifie pas. Il en va de même de `rpm`, absent de la machine de
# développement, dont `rpmbuild` sera nécessaire au générateur RPM de CPack.

set -euo pipefail

readonly REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
readonly MANUAL_DIR="${REPO_ROOT}/docs/manual"

readonly RED=$'\033[31m'
readonly GREEN=$'\033[32m'
readonly RESET=$'\033[0m'

build_dir="${REPO_ROOT}/build/release"

usage() {
    cat >&2 <<'USAGE'
usage: check-installation.sh [--build-dir <répertoire>]

  --build-dir  l'arbre de construction à installer (défaut : build/release)
USAGE
    exit 2
}

while (( $# > 0 )); do
    case "$1" in
        --build-dir) [[ $# -ge 2 ]] || usage; build_dir="$2"; shift 2 ;;
        -h|--help)   usage ;;
        *)           printf 'argument inconnu : %s\n' "$1" >&2; usage ;;
    esac
done

failures=0

report_failure() {
    printf '%s✗%s %s\n' "${RED}" "${RESET}" "$*" >&2
    failures=$((failures + 1))
}

report_success() {
    printf '%s✓%s %s\n' "${GREEN}" "${RESET}" "$*"
}

if [[ ! -d "${build_dir}" ]]; then
    printf '%s✗ arbre de construction absent : %s%s\n' "${RED}" "${build_dir}" "${RESET}" >&2
    printf '  le construire avec « make release »\n' >&2
    exit 1
fi

# Le préfixe ET le répertoire de configuration des binaires lancés. Le second
# pour la règle du projet : un binaire qu'un contrôle lance ne touche jamais la
# configuration de qui le lance — voir check-config-home.sh.
prefix="$(mktemp -d)"
cleanup() {
    rm -rf "${prefix}"
}
trap cleanup EXIT

cmake --install "${build_dir}" --prefix "${prefix}" >/dev/null

# ## Les binaires installés se lancent, et disent la bonne version
#
# Lancés depuis un répertoire qui n'est ni le dépôt ni l'arbre de construction :
# c'est la moitié du contrôle. Un binaire qui trouve un fichier « à côté de lui »
# le trouve encore quand on le lance depuis le dépôt.
readonly VERSION="$(sed -n 's/^[[:space:]]*VERSION[[:space:]]\{1,\}\([0-9.]\{1,\}\)[[:space:]]*$/\1/p' \
    "${REPO_ROOT}/CMakeLists.txt" | head -1)"

check_binary() {
    local name="$1"
    local binary="${prefix}/bin/${name}"

    if [[ ! -x "${binary}" ]]; then
        report_failure "${name} n'a pas été installé dans bin/"
        return
    fi

    local said
    said="$(cd "${prefix}" && QT_QPA_PLATFORM=offscreen XDG_CONFIG_HOME="${prefix}/config" \
        "${binary}" --version 2>&1)" || {
        report_failure "${name} installé ne se lance pas"
        return
    }

    if [[ "${said}" != "subedit ${VERSION}" ]]; then
        report_failure "${name} annonce « ${said} », attendu « subedit ${VERSION} »"
        return
    fi

    report_success "${name} s'installe, se lance, et annonce ${VERSION}"
}

check_binary subedit-cli
check_binary subedit-gui

# ## Chaque page du manuel se retrouve sous le préfixe
#
# **La liste attendue est calculée, jamais recopiée.** Une liste écrite à la
# main se périme au premier chapitre ajouté, et le manque ne se verrait que si
# quelqu'un pensait à l'y inscrire — c'est-à-dire jamais. Les captures d'écran
# comptent autant que les pages : une image absente est une page trouée.
check_manual() {
    local installed="${prefix}/share/subedit/manual"

    if [[ ! -d "${installed}" ]]; then
        report_failure "le manuel n'a pas été installé dans share/subedit/manual"
        return
    fi

    local missing=()
    local file
    while IFS= read -r -d '' file; do
        local relative="${file#"${MANUAL_DIR}/"}"
        [[ -f "${installed}/${relative}" ]] || missing+=("${relative}")
    done < <(find "${MANUAL_DIR}" -type f -print0)

    if (( ${#missing[@]} > 0 )); then
        report_failure "le manuel installé est incomplet :
$(printf '    %s\n' "${missing[@]}")
    les règles install() de cmake/Installation.cmake ne copient pas tout"
        return
    fi

    local count
    count="$(find "${MANUAL_DIR}" -type f | wc -l)"
    report_success "les ${count} fichiers du manuel sont installés"
}

check_manual

if (( failures > 0 )); then
    printf '%s%d contrôle(s) d'\''installation en échec%s\n' "${RED}" "${failures}" "${RESET}" >&2
    exit 1
fi

printf '%s✓%s une installation propre s'\''installe et se lance\n' "${GREEN}" "${RESET}"
