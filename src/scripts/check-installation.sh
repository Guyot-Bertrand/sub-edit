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
#   3. le préfixe temporaire ne laisse rien derrière lui ;
#   4. **les quatre fichiers de bureau sont là, et trois d'entre eux se
#      valident** — le `.desktop`, les métadonnées AppStream et l'icône, celle-ci
#      lue par le chargeur des bureaux GTK plutôt que par n'importe lequel ;
#   5. **la page de manuel ne ment pas** — sa version est celle du binaire, le
#      répertoire de manuel qu'elle nomme est celui où le manuel a été déposé, et
#      les sous-commandes qu'elle énumère sont celles que le binaire énumère ;
#   6. **une installation mise en scène ne touche rien au-dehors** — `DESTDIR`
#      non vide, préfixe `/usr`, et rien d'écrit hors de la mise en scène ;
#   7. **les deux paquets natifs se construisent, et disent la même chose** — un
#      `.deb` et un `.rpm`, dont les listes de fichiers coïncident et dont les
#      dépendances sont déclarées.
#
# ## Ce qu'il ne vérifie pas, et il vaut mieux l'écrire
#
# **Que le `.rpm` s'installe.** Un `.rpm` construit sur Ubuntu ne peut pas y être
# installé : l'éprouver demande une machine Fedora, que ni la porte ni la CI
# n'ont — ADR 0023. L'asymétrie est assumée et le manuel la dit.
#
# | Vérifié ici | Non vérifiable ici |
# | :---------- | :----------------- |
# | la liste des fichiers, par `rpm -qlp` | que le paquet s'installe |
# | les dépendances déclarées, par `rpm -qp --requires` | que ces noms existent chez Fedora |
# | l'accord des deux paquets | que le binaire se lance une fois installé |
#
# **L'accord des deux listes est le plus utile des trois**, et le seul qui ne
# demande aucune Fedora : les deux paquets sortent de la même installation, donc
# un écart entre leurs listes est un défaut des règles `install()`. C'est ce qui
# met à l'épreuve la moitié de l'ADR 0023 qui dit « l'installation d'abord ».
#
# **Que le `.deb` s'installe** n'est pas vérifié non plus, et pour une raison
# différente : `dpkg -i` demande les droits de l'administrateur, qu'une porte de
# qualité n'a pas et ne doit pas demander. Ce qui est vérifié est ce que le
# paquet contient et ce qu'il déclare, plus le fait que les mêmes binaires,
# installés par `cmake --install`, se lancent.

set -euo pipefail

readonly REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
readonly MANUAL_DIR="${REPO_ROOT}/docs/manual"

# L'identifiant de l'application, celui que portent les trois fichiers de
# bureau. Écrit ici comme dans `cmake/Installation.cmake` : ce contrôle existe
# pour confronter ce que les règles déposent à ce qu'on attend, donc il doit le
# dire de son côté plutôt que de le lire du leur.
readonly APP_ID="io.github.guyot_bertrand.subedit"

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

# ## Les quatre fichiers de bureau
#
# **Deux d'entre eux se valident, et les deux autres ne se valident que par leur
# présence.** Un SVG mal formé se verrait à l'œil et nulle part ailleurs ; une
# page de manuel se relit plus bas, où elle est confrontée au binaire.
check_desktop_files() {
    local desktop="${prefix}/share/applications/${APP_ID}.desktop"
    local metainfo="${prefix}/share/metainfo/${APP_ID}.metainfo.xml"
    local icon="${prefix}/share/icons/hicolor/scalable/apps/${APP_ID}.svg"
    local page="${prefix}/share/man/man1/subedit-cli.1.gz"

    local file
    for file in "${desktop}" "${metainfo}" "${icon}" "${page}"; do
        [[ -f "${file}" ]] || report_failure "absent de l'installation : ${file#"${prefix}/"}"
    done

    if [[ -f "${desktop}" ]]; then
        if desktop-file-validate "${desktop}" >/dev/null 2>&1; then
            report_success "le fichier .desktop est valide"
        else
            report_failure "desktop-file-validate refuse le fichier installé :
$(desktop-file-validate "${desktop}" 2>&1 | sed 's/^/    /')"
        fi
    fi

    if [[ -f "${metainfo}" ]]; then
        if appstreamcli validate --no-net "${metainfo}" >/dev/null 2>&1; then
            report_success "les métadonnées AppStream sont valides"
        else
            report_failure "appstreamcli refuse les métadonnées installées :
$(appstreamcli validate --no-net "${metainfo}" 2>&1 | sed 's/^/    /')"
        fi
    fi

    # **L'icône est lue, et par le chargeur des bureaux GTK** — #260. Elle était
    # le seul fichier livré dont rien ne regardait le contenu, et elle était
    # cassée : un commentaire avant la balise racine la rendait méconnaissable
    # pour gdk-pixbuf, qui affichait une tuile vide. Inkscape et Qt l'ouvraient
    # sans un mot ; c'est pourquoi le contrôle passe par celui-là et pas un
    # autre.
    if [[ -f "${icon}" ]]; then
        if "${REPO_ROOT}/src/scripts/check-icon.py" "${icon}" >/dev/null 2>&1; then
            report_success "l'icône se charge, et se voit sur les deux fonds"
        else
            report_failure "l'icône installée ne tient pas :
$("${REPO_ROOT}/src/scripts/check-icon.py" "${icon}" 2>&1 | sed 's/^/    /')"
        fi
    fi
}

check_desktop_files

# ## La page de manuel ne ment pas
#
# Trois affirmations, et chacune est le genre qu'une page de manuel porte
# pendant des années sans que personne la relise.
check_man_page() {
    local compressed="${prefix}/share/man/man1/subedit-cli.1.gz"
    [[ -f "${compressed}" ]] || return

    # Lue décompressée, comme `man` la lit. Dans une variable et non dans un
    # fichier : la fonction sort par plusieurs chemins, et un temporaire de plus
    # serait un temporaire à effacer sur chacun d'eux.
    local page
    page="$(gzip -dc "${compressed}")"

    local announced
    announced="$(sed -n 's/^\.TH .* "subedit \([0-9.]*\)".*/\1/p' <<< "${page}")"
    if [[ "${announced}" != "${VERSION}" ]]; then
        report_failure "la page de manuel annonce « ${announced} », attendu « ${VERSION} »"
    fi

    # Le répertoire qu'elle nomme est celui où le manuel a réellement été
    # déposé. C'est le défaut qu'a eu le premier `.deb` produit : la page,
    # engendrée à la configuration, annonçait /usr/local dans un paquet qui
    # installe sous /usr.
    local named
    named="$(sed -n 's/^\.I \(.*subedit\/manual\)$/\1/p' <<< "${page}")"
    if [[ "${named}" != "${prefix}/share/subedit/manual" ]]; then
        report_failure "la page de manuel nomme « ${named} », le manuel est sous « ${prefix}/share/subedit/manual »"
    fi

    # **Les sous-commandes qu'elle énumère sont celles du binaire.** Une
    # sous-commande ajoutée sans être écrite ici donnerait une page qui en
    # oublie une, et rien d'autre ne le dirait.
    local written listed
    written="$(sed -n '/^\.SH SUBCOMMANDS/,/^\.SH [A-Z]/p' <<< "${page}" \
        | sed -n 's/^\.B \([a-z][a-z-]*\)$/\1/p' | sort -u)"
    listed="$(cd "${prefix}" && QT_QPA_PLATFORM=offscreen XDG_CONFIG_HOME="${prefix}/config" \
        "${prefix}/bin/subedit-cli" --help 2>&1 \
        | awk '/^Subcommands:/ { seen = 1; next } seen && NF { print $1 }' | sort -u)"

    if [[ "${written}" != "${listed}" ]]; then
        report_failure "la page de manuel et le binaire ne nomment pas les mêmes sous-commandes :
$(diff <(printf '%s\n' "${written}") <(printf '%s\n' "${listed}") | sed 's/^/    /')"
        return
    fi

    report_success "la page de manuel dit la version, le manuel et les $(printf '%s\n' "${listed}" | wc -l) sous-commandes du binaire"
}

check_man_page

# ## Une installation mise en scène ne touche rien au-dehors
#
# **C'est ce qu'un empaqueteur fait**, et c'est la seule façon de découvrir
# qu'une règle écrit à un chemin absolu : une installation sous `DESTDIR` qui
# tenterait d'écrire dans `/usr` échouerait ici, faute de droits, plutôt que de
# réussir chez quelqu'un qui en a.
check_staged_install() {
    local staged
    staged="$(mktemp -d)"

    if ! DESTDIR="${staged}" cmake --install "${build_dir}" --prefix /usr >/dev/null 2>&1; then
        report_failure "une installation avec DESTDIR échoue — une règle écrit hors de la mise en scène"
        rm -rf "${staged}"
        return
    fi

    local outside
    outside="$(find "${staged}" -mindepth 1 -maxdepth 1 ! -name usr -printf '%f\n')"
    if [[ -n "${outside}" ]]; then
        report_failure "une installation avec DESTDIR pose autre chose que le préfixe :
$(printf '    %s\n' ${outside})"
        rm -rf "${staged}"
        return
    fi

    if [[ ! -x "${staged}/usr/bin/subedit-cli" ]]; then
        report_failure "une installation avec DESTDIR ne dépose pas les binaires sous le préfixe"
        rm -rf "${staged}"
        return
    fi

    report_success "une installation avec DESTDIR ne touche rien en dehors"
    rm -rf "${staged}"
}

check_staged_install

# ## Les deux paquets natifs
#
# **Leurs listes de fichiers doivent coïncider**, et c'est le contrôle qui met à
# l'épreuve la moitié de l'ADR 0023 : les deux sortent de la même installation,
# donc un écart entre elles est un défaut des règles `install()` et non du
# format.
#
# **Deux choses seulement sont écartées de la comparaison, et chacune est une
# propriété du format et non de ce qu'on installe.** Les écarter est ce qui rend
# le contrôle utilisable ; les écarter *nommément* est ce qui l'empêche de
# cacher un vrai défaut.
#
# | Écarté | Pourquoi |
# | :----- | :------- |
# | les répertoires | un `.deb` n'en possède aucun, un `.rpm` possède ceux que sa distribution ne possède pas déjà |
# | `/usr/lib/.build-id/…` | `rpmbuild` pose ces liens pour ses propres outils de débogage ; rien ne les installe |
#
# **La compression de la page de manuel n'y est pas**, et c'est délibéré :
# `rpmbuild` compresse la sienne, la charte Debian l'exige aussi, donc
# l'installation la compresse elle-même et les deux paquets portent le même
# `subedit-cli.1.gz`. Un écart qu'on pouvait supprimer valait mieux qu'un écart
# de plus à excuser ici.
check_packages() {
    local built=("${build_dir}"/subedit[-_]*.deb "${build_dir}"/subedit[-_]*.rpm)
    rm -f "${built[@]}" 2>/dev/null || true

    if ! (cd "${build_dir}" && cpack -G "DEB;RPM" >/dev/null 2>&1); then
        report_failure "cpack ne produit pas les deux paquets
    (cd ${build_dir} && cpack -G 'DEB;RPM') pour voir pourquoi"
        return
    fi

    local deb rpm
    deb="$(find "${build_dir}" -maxdepth 1 -name 'subedit*.deb' | head -1)"
    rpm="$(find "${build_dir}" -maxdepth 1 -name 'subedit*.rpm' | head -1)"

    if [[ -z "${deb}" || -z "${rpm}" ]]; then
        report_failure "cpack n'a produit qu'un paquet sur deux (deb : « ${deb} », rpm : « ${rpm} »)"
        return
    fi

    # Le `.deb` marque ses répertoires d'une barre finale ; `rpm -qlp` ne les
    # distingue pas, mais `--dump` donne le mode de chaque entrée, et un
    # répertoire s'y reconnaît à son `04…`. On lit donc le type plutôt que de le
    # deviner sur le nom.
    local in_deb in_rpm
    in_deb="$(dpkg-deb -c "${deb}" \
        | awk '$6 !~ /\/$/ { print substr($6, 2) }' \
        | grep -v '^/usr/lib/\.build-id/' | sort)"
    in_rpm="$(rpm -qp --dump "${rpm}" 2>/dev/null \
        | awk '$5 !~ /^04/ { print $1 }' \
        | grep -v '^/usr/lib/\.build-id/' | sort)"

    if [[ "${in_deb}" != "${in_rpm}" ]]; then
        report_failure "les deux paquets ne portent pas les mêmes fichiers :
$(diff <(printf '%s\n' "${in_deb}") <(printf '%s\n' "${in_rpm}") | sed 's/^/    /')
    les deux sortent de la même installation ; un écart est un défaut des règles install()"
        return
    fi

    report_success "le .deb et le .rpm portent les mêmes $(printf '%s\n' "${in_deb}" | wc -l) fichiers"

    # **Les dépendances, elles, ne coïncident pas et ne le doivent pas** : les
    # mêmes bibliothèques s'appellent autrement chez Debian et chez Fedora. Ce
    # qui est vérifié est qu'aucune des deux listes n'est vide — un paquet sans
    # dépendance déclarée s'installe et ne se lance pas.
    local deb_deps rpm_deps
    deb_deps="$(dpkg-deb -f "${deb}" Depends)"
    rpm_deps="$(rpm -qp --requires "${rpm}" 2>/dev/null | grep -v '^rpmlib(' || true)"

    [[ -n "${deb_deps}" ]] || report_failure "le .deb ne déclare aucune dépendance"
    [[ -n "${rpm_deps}" ]] || report_failure "le .rpm ne déclare aucune dépendance"

    if [[ -n "${deb_deps}" && -n "${rpm_deps}" ]]; then
        report_success "les deux paquets déclarent leurs dépendances, chacun dans les noms de sa famille"
    fi
}

check_packages

if (( failures > 0 )); then
    printf '%s%d contrôle(s) d'\''installation en échec%s\n' "${RED}" "${failures}" "${RESET}" >&2
    exit 1
fi

printf '%s✓%s une installation propre s'\''installe et se lance\n' "${GREEN}" "${RESET}"
