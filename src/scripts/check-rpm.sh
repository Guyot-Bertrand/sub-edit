#!/usr/bin/env bash
# Installe le `.rpm` sur une vraie Fedora, et lance ce qui vient d'en sortir.
#
# **C'est le contrôle que l'ADR 0023 nommait sans pouvoir l'écrire** — « à
# rouvrir quand quelqu'un installera vraiment le paquet », issue #266. Le
# développement se fait sur Ubuntu : `rpmbuild` y produit un `.rpm` et `rpm` n'y
# installe rien. Trois lignes du tableau de l'ADR restaient donc vides, et la
# plus coûteuse des trois était celle-ci — **que les noms de dépendances
# existent dans la distribution.** Ils sont écrits à la main dans
# `cmake/Packaging.cmake`, parce que CPack ne sait rien de Fedora, et un nom
# erroné produit un paquet parfaitement valide qui refuse de s'installer.
#
# ## Ce qu'il a trouvé du premier coup
#
# **Le paquet ne s'installait pas**, et pas pour la raison qu'on surveillait.
# Les dépendances étaient bonnes ; c'étaient les répertoires. Le `.rpm`
# déclarait posséder `/usr/share/applications`, `/usr/share/icons` et six
# autres, que `filesystem` et `hicolor-icon-theme` possèdent déjà, et `dnf`
# refusait la transaction sur six conflits. L'exclusion existait depuis #244 et
# n'excluait rien — chemins relatifs contre chemins de paquet. Voir
# `cmake/Packaging.cmake`.
#
# **Personne ne pouvait le voir sans jouer la transaction.** `rpm -qlp` lit une
# liste de fichiers ; un conflit de propriété n'est pas dans le paquet, il est
# entre le paquet et la distribution.
#
# ## Ce qu'il vérifie
#
#   1. `dnf install` résout les dépendances **et** mène la transaction à son
#      terme ;
#   2. chaque chemin que le paquet annonce est là où il l'annonce ;
#   3. les deux binaires installés se lancent et disent la bonne version ;
#   4. la page de manuel se trouve par `man`, qui est l'outil qui la lit.
#
# ## Ce qu'il coûte, et pourquoi il n'est dans aucune porte
#
# Il télécharge Qt et ses dépendances, soit près de trois cents mégaoctets, et
# demande le réseau. **Une porte qui dépend du réseau est rouge un jour de
# panne, pour une raison étrangère au dépôt** — c'est la règle que
# `verify-gates.sh` s'applique déjà. Il se lance donc à la main, et une fois par
# semaine sur `main` : `.github/workflows/fedora.yml`. Le quota d'Actions du
# dépôt a déjà été épuisé une fois (#232, #233), et un conteneur par pull
# request n'est pas gratuit.
#
# **Ce qui garde les pull requests entre-temps est moins cher et plus étroit :**
# `check-installation.sh` refuse un `.rpm` qui déclare posséder un répertoire
# qui n'est pas à lui. C'est la cause du défaut ci-dessus, et elle se lit dans
# le paquet sans Fedora. Les deux contrôles ne voient pas la même chose, et
# c'est pour cela qu'ils sont deux.
#
# ## Le conteneur n'est pas un bureau Fedora, sur un point
#
# Une image Fedora pose `tsflags=nodocs`, qui fait écarter la documentation à
# l'installation — un bureau ne le fait pas. Le contrôle le défait
# explicitement, faute de quoi il jugerait la page de manuel absente d'un
# paquet qui la porte.

set -euo pipefail

readonly REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"

readonly RED=$'\033[31m'
readonly GREEN=$'\033[32m'
readonly RESET=$'\033[0m'

# **Une version nommée, jamais `latest`.** Une image qui change sous les pieds
# rendrait un échec impossible à rejouer, et ferait porter à ce dépôt les
# accidents d'une distribution qui bouge.
image="fedora:42"
build_dir="${REPO_ROOT}/build/release"

usage() {
    cat >&2 <<'USAGE'
usage: check-rpm.sh [--build-dir <répertoire>] [--image <image>]

  --build-dir  l'arbre où trouver le .rpm (défaut : build/release)
  --image      l'image Fedora à lancer  (défaut : fedora:42)
USAGE
    exit 2
}

while (($# > 0)); do
    case "$1" in
    --build-dir) [[ $# -ge 2 ]] || usage; build_dir="$2"; shift 2 ;;
    --image) [[ $# -ge 2 ]] || usage; image="$2"; shift 2 ;;
    -h | --help) usage ;;
    *) printf 'argument inconnu : %s\n' "$1" >&2; usage ;;
    esac
done

# **Podman d'abord, Docker ensuite.** Podman est ce qu'une Fedora a d'origine,
# et il tourne sans démon ; Docker est ce qu'une Ubuntu de développement a le
# plus souvent. Les deux prennent les mêmes arguments pour ce qu'on leur
# demande ici.
engine=""
for candidate in podman docker; do
    if command -v "${candidate}" >/dev/null 2>&1 && "${candidate}" info >/dev/null 2>&1; then
        engine="${candidate}"
        break
    fi
done

if [[ -z "${engine}" ]]; then
    printf '%s✗ ni podman ni docker n'\''est utilisable%s\n' "${RED}" "${RESET}" >&2
    printf '  ce contrôle installe le paquet sur une vraie Fedora ; il lui faut un conteneur\n' >&2
    printf '  Ubuntu : sudo apt install podman\n' >&2
    exit 1
fi

readonly VERSION="$(sed -n 's/^[[:space:]]*VERSION[[:space:]]\{1,\}\([0-9.]\{1,\}\)[[:space:]]*$/\1/p' \
    "${REPO_ROOT}/CMakeLists.txt" | head -1)"

package="$(find "${build_dir}" -maxdepth 1 -name 'subedit*.rpm' 2>/dev/null | head -1)"

if [[ -z "${package}" ]]; then
    printf '%s✗ aucun .rpm dans %s%s\n' "${RED}" "${build_dir}" "${RESET}" >&2
    printf '  le produire avec « make packages »\n' >&2
    exit 1
fi

# Le paquet est monté seul, dans un répertoire à lui : le conteneur n'a aucune
# raison de voir le dépôt, et ce qu'il ne voit pas ne peut pas fausser ce qu'il
# éprouve. C'est la même raison qui fait lancer les binaires installés depuis
# ailleurs que l'arbre de construction, dans check-installation.sh.
staging="$(mktemp -d)"
cleanup() {
    rm -rf "${staging}"
}
trap cleanup EXIT

cp "${package}" "${staging}/"

printf 'paquet : %s\n' "$(basename "${package}")"
printf 'image   : %s   (%s)\n\n' "${image}" "${engine}"

# Le contrôle lui-même, joué dans le conteneur. Écrit ici plutôt que dans un
# fichier à part : il n'a qu'un appelant, et un fichier de plus se lirait sans
# celui-ci — or il ne veut rien dire seul.
#
# **`-i` et non pas seulement `--rm`** : sans lui, l'entrée standard du
# conteneur n'est reliée à rien, et le `bash -s` ci-dessous lit un script vide.
# Il rend alors zéro sans avoir rien fait, ce qui est le pire des verts — et
# c'est exactement ce qui est arrivé au premier essai.
#
# **`Z` sur le montage**, pour SELinux : sans lui, un podman sans privilèges sur
# Fedora ne lit pas le répertoire monté. Docker l'accepte et l'ignore là où
# SELinux n'est pas là, donc il ne coûte rien à celui qui n'en a pas besoin.
"${engine}" run --rm -i \
    -v "${staging}:/pkg:ro,Z" \
    -e "ATTENDU=${VERSION}" \
    "${image}" \
    /usr/bin/bash -s <<'INNER'
set -uo pipefail

RED=$'\033[31m'
GREEN=$'\033[32m'
RESET=$'\033[0m'

failures=0

report_failure() {
    printf '%s✗%s %s\n' "${RED}" "${RESET}" "$*" >&2
    failures=$((failures + 1))
}

report_success() {
    printf '%s✓%s %s\n' "${GREEN}" "${RESET}" "$*"
}

printf '  %s\n\n' "$(cat /etc/fedora-release)"

## 1. Le paquet s'installe
#
# **`--setopt=tsflags=` défait le `nodocs` de l'image**, qui n'est pas un
# réglage de Fedora mais un réglage de conteneur : sans lui, la page de manuel
# serait écartée à l'installation et le contrôle 4 échouerait sur un paquet
# irréprochable.
#
# La sortie est gardée et n'est écrite qu'en cas d'échec : elle fait quatre
# cents lignes de téléchargements, et ce qui compte y tient en six.
if ! dnf -y --setopt=tsflags= install /pkg/*.rpm > /tmp/dnf.log 2>&1; then
    report_failure "dnf refuse d'installer le paquet :
$(grep -vE '^\[[0-9]+/[0-9]+\]|^ *$' /tmp/dnf.log | tail -20 | sed 's/^/    /')"
    printf '\n%s%d contrôle(s) en échec%s\n' "${RED}" "${failures}" "${RESET}" >&2
    exit 1
fi

installed="$(rpm -q --qf '%{VERSION}' subedit)"
if [[ "${installed}" != "${ATTENDU}" ]]; then
    report_failure "le paquet installé annonce « ${installed} », attendu « ${ATTENDU} »"
else
    report_success "dnf installe le paquet, et résout $(rpm -qp --requires /pkg/*.rpm 2>/dev/null \
        | grep -vc '^rpmlib(') dépendance(s)"
fi

## 2. Chaque chemin annoncé est là où il est annoncé
#
# **La liste vient du paquet, jamais d'ici.** Une liste écrite dans ce script
# se périmerait au premier fichier ajouté, et le manque ne se verrait que si
# quelqu'un pensait à l'y inscrire.
missing=()
while IFS= read -r path; do
    [[ -e "${path}" ]] || missing+=("${path}")
done < <(rpm -ql subedit)

if ((${#missing[@]} > 0)); then
    report_failure "le paquet annonce des chemins qu'il n'a pas déposés :
$(printf '    %s\n' "${missing[@]}")"
else
    report_success "les $(rpm -ql subedit | wc -l) chemins annoncés sont là où le paquet les annonce"
fi

## 3. Les deux binaires installés se lancent
#
# Depuis la racine, et non depuis un répertoire qui contiendrait quelque chose :
# un binaire qui trouve un fichier « à côté de lui » le trouve encore quand on
# le lance d'ailleurs. `XDG_CONFIG_HOME` est déplacé comme partout ailleurs
# dans ce dépôt — un contrôle ne touche pas la configuration de qui le lance.
check_binary() {
    local name="$1"
    local said

    said="$(cd / && QT_QPA_PLATFORM=offscreen XDG_CONFIG_HOME=/tmp/config \
        "/usr/bin/${name}" --version 2>&1)" || {
        report_failure "${name} installé ne se lance pas :
$(printf '%s\n' "${said}" | sed 's/^/    /')"
        return
    }

    if [[ "${said}" != "subedit ${ATTENDU}" ]]; then
        report_failure "${name} annonce « ${said} », attendu « subedit ${ATTENDU} »"
        return
    fi

    report_success "${name} se lance depuis l'installé, et annonce ${ATTENDU}"
}

check_binary subedit-cli
check_binary subedit-gui

## 4. La page de manuel se trouve par man
#
# **Par `man` et non par un `test -f`** : ce qui est en jeu n'est pas qu'un
# fichier existe, c'est qu'un utilisateur qui tape `man subedit-cli` obtienne
# quelque chose. Le répertoire peut être juste et hors du `manpath`, et rien
# d'autre ne le dirait — c'est la leçon de #268, vérifier avec l'outil qui
# compte.
# `util-linux` avec `man-db`, et il n'est pas superflu : `man` formate en
# passant par `col -b`, que l'image minimale n'a pas — `util-linux-core` ne
# suffit pas, `col` est dans `util-linux`. Sans lui, `man` sort en erreur 127 et
# ne rend rien, un échec qui ressemble à une page absente et qui n'en est pas un.
if ! dnf -y --setopt=tsflags= install man-db util-linux > /tmp/man.log 2>&1; then
    report_failure "man-db ne s'installe pas dans le conteneur ; la page de manuel n'est pas éprouvée"
else
    found="$(man -w subedit-cli 2>/dev/null || true)"
    if [[ -z "${found}" ]]; then
        report_failure "man ne trouve pas subedit-cli ; la page est hors du manpath"
    elif ! man subedit-cli 2>/dev/null | grep -q 'subedit-cli'; then
        report_failure "man trouve la page mais n'en rend rien"
    else
        report_success "man subedit-cli rend la page installée (${found})"
    fi
fi

## 5. Le manuel complet est lisible
#
# Le chemin est celui que la page de manuel annonce, et non un chemin écrit
# ici : c'est ce que la fenêtre d'aide ouvrira sur cette machine.
named="$(man subedit-cli 2>/dev/null | grep -o '/usr/share/subedit/manual' | head -1)"
if [[ -z "${named}" ]]; then
    report_failure "la page de manuel ne nomme pas le répertoire du manuel"
elif [[ ! -r "${named}/index.md" ]]; then
    report_failure "le manuel annoncé sous ${named} n'y est pas lisible"
else
    report_success "le manuel est lisible sous ${named} ($(find "${named}" -type f | wc -l) fichiers)"
fi

if ((failures > 0)); then
    printf '\n%s%d contrôle(s) en échec%s\n' "${RED}" "${failures}" "${RESET}" >&2
    exit 1
fi

printf '\n%s✓%s le paquet s'\''installe sur Fedora, et ce qu'\''il dépose fonctionne\n' \
    "${GREEN}" "${RESET}"
INNER
