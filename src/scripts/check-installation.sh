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
#   4. **les cinq fichiers de bureau sont là, et trois d'entre eux se
#      valident** — le `.desktop`, les métadonnées AppStream et l'icône, celle-ci
#      lue par le chargeur des bureaux GTK plutôt que par n'importe lequel ;
#   5. **les deux pages de manuel ne mentent pas, et se rendent** — une par
#      binaire depuis #272 : leur version est celle du binaire, le répertoire de
#      manuel qu'elles nomment est celui où le manuel a été déposé, chacune porte
#      le nom du binaire qu'elle décrit, les sous-commandes que celle de la ligne
#      de commande énumère sont celles que le binaire énumère, et groff les rend
#      sans un avertissement ;
#   6. **une installation mise en scène ne touche rien au-dehors** — `DESTDIR`
#      non vide, préfixe `/usr`, et rien d'écrit hors de la mise en scène ;
#   7. **les deux paquets natifs se construisent, et disent la même chose** — un
#      `.deb` et un `.rpm`, dont les listes de fichiers coïncident et dont les
#      dépendances sont déclarées ;
#   8. **le `.rpm` ne possède que ses propres répertoires** — ceux de la
#      distribution ne sont pas à lui, et `dnf` refuse la transaction entière
#      s'il les revendique.
#
# ## Ce qu'il ne vérifie pas, et où cela se vérifie
#
# **Que le `.rpm` s'installe.** Un `.rpm` construit sur Ubuntu ne peut pas y être
# installé : l'éprouver demande une Fedora. Elle existe depuis #266, en
# conteneur — `src/scripts/check-rpm.sh`, une fois par semaine — et elle n'est
# pas ici parce qu'elle demande le réseau, ce qu'une porte ne doit pas demander.
#
# | Vérifié ici | Vérifié par check-rpm.sh |
# | :---------- | :----------------------- |
# | la liste des fichiers, par `rpm -qlp` | que la transaction `dnf` aboutit |
# | les dépendances déclarées, par `rpm -qp --requires` | que ces noms existent chez Fedora |
# | l'accord des deux paquets | que les binaires installés se lancent |
# | que le `.rpm` ne possède aucun répertoire partagé | que `man` trouve la page installée |
#
# **La dernière ligne de gauche est la moitié bon marché de la droite.** Le
# `.rpm` revendiquait huit répertoires de la distribution et `dnf` refusait la
# transaction entière ; la cause se lit dans le paquet, sans Fedora, et c'est ce
# que le contrôle local fait désormais à chaque pull request.
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

# L'arbre que ce projet crée, et le seul qu'un paquet ait le droit de posséder.
# Écrit ici comme `APP_ID` l'est, et pour la même raison : ce contrôle confronte
# ce que les règles déposent à ce qu'on attend.
readonly DATA_SUBDIR="share/subedit"

# Les binaires livrés, et leurs pages de manuel — une par binaire depuis #272.
# Écrits ici comme `APP_ID` l'est : ce contrôle confronte ce que les règles
# déposent à ce qu'on attend, donc il le dit de son côté.
readonly BINARIES=(subedit-cli subedit-gui)

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

    local expected=("${desktop}" "${metainfo}" "${icon}")
    local name
    for name in "${BINARIES[@]}"; do
        expected+=("${prefix}/share/man/man1/${name}.1.gz")
    done

    local file
    for file in "${expected[@]}"; do
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

# ## Les pages de manuel ne mentent pas
#
# **Deux pages depuis #272, une par binaire.** Ce qui vaut pour les deux est
# vérifié pour les deux ; ce qui ne vaut que pour la ligne de commande — la
# liste des sous-commandes — reste à elle.
#
# Chaque affirmation est du genre qu'une page de manuel porte pendant des années
# sans que personne la relise.
check_man_pages() {
    local name
    for name in "${BINARIES[@]}"; do
        check_one_man_page "${name}"
    done
}

check_one_man_page() {
    local name="$1"
    local compressed="${prefix}/share/man/man1/${name}.1.gz"
    [[ -f "${compressed}" ]] || return

    # Lue décompressée, comme `man` la lit. Dans une variable et non dans un
    # fichier : la fonction sort par plusieurs chemins, et un temporaire de plus
    # serait un temporaire à effacer sur chacun d'eux.
    local page
    page="$(gzip -dc "${compressed}")"

    local announced
    announced="$(sed -n 's/^\.TH .* "subedit \([0-9.]*\)".*/\1/p' <<< "${page}")"
    if [[ "${announced}" != "${VERSION}" ]]; then
        report_failure "${name}.1 annonce « ${announced} », attendu « ${VERSION} »"
        return
    fi

    # Le répertoire qu'elle nomme est celui où le manuel a réellement été
    # déposé. C'est le défaut qu'a eu le premier `.deb` produit : la page,
    # engendrée à la configuration, annonçait /usr/local dans un paquet qui
    # installe sous /usr.
    local named
    named="$(sed -n 's/^\.I \(.*subedit\/manual\)$/\1/p' <<< "${page}")"
    if [[ "${named}" != "${prefix}/share/subedit/manual" ]]; then
        report_failure "${name}.1 nomme « ${named} », le manuel est sous « ${prefix}/share/subedit/manual »"
        return
    fi

    # **Le nom qu'elle porte est celui du binaire qu'elle décrit.** Deux pages
    # engendrées par la même boucle depuis deux gabarits : une ligne `.TH`
    # recopiée d'un gabarit à l'autre donnerait deux pages du même nom, dont une
    # que `man` ne trouverait jamais.
    local titled
    titled="$(sed -n 's/^\.TH \([A-Z-]*\) .*/\1/p' <<< "${page}" | tr 'A-Z' 'a-z')"
    if [[ "${titled}" != "${name}" ]]; then
        report_failure "${name}.1 se titre « ${titled} », attendu « ${name} »"
        return
    fi

    # **Les sous-commandes qu'elle énumère sont celles du binaire.** Une
    # sous-commande ajoutée sans être écrite ici donnerait une page qui en
    # oublie une, et rien d'autre ne le dirait. Seule la page de la ligne de
    # commande en a — la fenêtre n'en a pas.
    if ! grep -q '^\.SH SUBCOMMANDS' <<< "${page}"; then
        report_success "${name}.1 dit la bonne version, le bon manuel, et porte son nom"
        return
    fi

    local written listed
    written="$(sed -n '/^\.SH SUBCOMMANDS/,/^\.SH [A-Z]/p' <<< "${page}" \
        | sed -n 's/^\.B \([a-z][a-z-]*\)$/\1/p' | sort -u)"
    listed="$(cd "${prefix}" && QT_QPA_PLATFORM=offscreen XDG_CONFIG_HOME="${prefix}/config" \
        "${prefix}/bin/${name}" --help 2>&1 \
        | awk '/^Subcommands:/ { seen = 1; next } seen && NF { print $1 }' | sort -u)"

    if [[ "${written}" != "${listed}" ]]; then
        report_failure "${name}.1 et le binaire ne nomment pas les mêmes sous-commandes :
$(diff <(printf '%s\n' "${written}") <(printf '%s\n' "${listed}") | sed 's/^/    /')"
        return
    fi

    report_success "${name}.1 dit la version, le manuel et les $(printf '%s\n' "${listed}" | wc -l) sous-commandes du binaire"
}

check_man_pages

# ## Les pages de manuel sont rendues par l'outil qui les rend
#
# **Les contrôles ci-dessus lisent les pages au `sed` ; aucun ne les rend.**
# C'est exactement le défaut que #260 a payé sur l'icône, et #268 l'a cherché
# partout ailleurs : une vérification qui passe par un outil qui n'est pas celui
# qui compte. Une page de manuel se lit avec `man`, c'est-à-dire avec groff, et
# personne ne la lui avait jamais donnée.
#
# Elle passait — et avec deux avertissements, sur deux caractères UTF-8 d'une
# ligne de commentaire. Sans conséquence pour un lecteur ; la question n'était
# pas là. La question était qu'aucun contrôle ne l'aurait dit.
#
# **`-z` et non un rendu** : on veut le diagnostic, pas la page. **`-ww` et non
# le silence par défaut** : groff se tait sur presque tout, et un contrôle qui
# ne demande rien n'apprend rien. **Sans `-k`** : preconv rendrait l'UTF-8
# lisible et masquerait précisément ce qu'on cherche. `man` l'appelle chez nous,
# ne l'appelle pas partout, et une page qui tient sans lui tient avec.
#
# **Le code de sortie ne dit rien** — groff avertit et rend 0. C'est donc la
# sortie d'erreur qui est lue, et le moindre octet est un échec.
check_man_pages_render() {
    if ! command -v groff >/dev/null 2>&1; then
        report_failure "groff est absent : les pages de manuel ne peuvent pas être rendues
    l'installer avec « ./src/scripts/setup-toolchain.sh »"
        return
    fi

    local name rendered=0
    for name in "${BINARIES[@]}"; do
        local compressed="${prefix}/share/man/man1/${name}.1.gz"
        [[ -f "${compressed}" ]] || continue

        local complaints
        complaints="$(gzip -dc "${compressed}" | groff -ww -z -man - 2>&1 >/dev/null)"

        if [[ -n "${complaints}" ]]; then
            report_failure "groff se plaint de ${name}.1 :
$(sed 's/^/    /' <<< "${complaints}")
    les pages sont écrites en ASCII de bout en bout, commentaires compris —
    voir l'en-tête de packaging/${name}.1.in"
            return
        fi

        rendered=$((rendered + 1))
    done

    report_success "les ${rendered} pages de manuel se rendent sans un avertissement de groff"
}

check_man_pages_render

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

    check_rpm_directories "${rpm}"
}

# ## Le `.rpm` ne possède que ses propres répertoires
#
# **Le contrôle le moins cher de ce fichier, et il a attrapé le défaut le plus
# coûteux** — issue #266. Un `.rpm` qui déclare posséder `/usr/share/icons`
# entre en conflit avec `hicolor-icon-theme`, qui le possède vraiment, et `dnf`
# refuse la transaction entière. Le paquet reste parfaitement valide : il
# s'inspecte, il s'énumère, et il ne s'installe pas.
#
# `cmake/Packaging.cmake` excluait déjà ces répertoires, en chemins relatifs
# alors que CPack en attend d'absolus — l'exclusion n'a jamais rien exclu, du
# jour où elle a été écrite jusqu'à ce que `check-rpm.sh` joue une vraie
# transaction sur Fedora. **Une exclusion qui n'exclut rien ressemble en tout
# point à une exclusion qui marche**, et c'est pour cela que ce contrôle-ci
# existe : il regarde le résultat, jamais l'intention.
#
# **La règle est celle-ci : les seuls répertoires du paquet sont sous
# `share/subedit`.** C'est le seul arbre que ce projet crée ; tous les autres
# où il dépose quelque chose — `bin`, `applications`, `icons`, `man`,
# `metainfo` — appartiennent à la distribution. Le jour où le projet en créera
# un ailleurs, ce contrôle échouera et la règle sera relue, ce qui est
# exactement ce qu'on veut d'elle.
#
# **Local, et il ne remplace pas la Fedora.** Il voit la cause de ce défaut-là ;
# il ne verrait pas un nom de dépendance erroné, ni une bibliothèque manquante
# à l'exécution. C'est `src/scripts/check-rpm.sh` qui les voit, et il demande un
# conteneur et le réseau — donc une fois par semaine, pas à chaque pull request.
check_rpm_directories() {
    local package="$1"

    # `--dump` donne le mode de chaque entrée, et un répertoire s'y reconnaît à
    # son `04…` : on lit le type plutôt que de le deviner sur le nom.
    # `/usr/lib/.build-id/…` est écarté ici comme il l'est de la comparaison des
    # deux listes, et pour la même raison : `rpmbuild` pose ces répertoires pour
    # ses propres outils de débogage, rien de ce que le projet installe ne les
    # nomme, et la transaction Fedora les accepte — c'est une propriété du
    # format, pas de ce qu'on empaquette.
    local owned
    owned="$(rpm -qp --dump "${package}" 2>/dev/null \
        | awk '$5 ~ /^04/ { print $1 }' \
        | grep -v '^/usr/lib/\.build-id\(/\|$\)' \
        | grep -v "^/usr/${DATA_SUBDIR}\(/\|$\)" || true)"

    if [[ -n "${owned}" ]]; then
        report_failure "le .rpm déclare posséder des répertoires qui ne sont pas à lui :
$(printf '%s\n' "${owned}" | sed 's/^/    /')
    la distribution les possède déjà, et dnf refusera la transaction
    les exclure dans CPACK_RPM_EXCLUDE_FROM_AUTO_FILELIST_ADDITION, en chemins absolus"
        return
    fi

    report_success "le .rpm ne possède que ses propres répertoires"
}

check_packages

if (( failures > 0 )); then
    printf '%s%d contrôle(s) d'\''installation en échec%s\n' "${RED}" "${failures}" "${RESET}" >&2
    exit 1
fi

printf '%s✓%s une installation propre s'\''installe et se lance\n' "${GREEN}" "${RESET}"
