#!/usr/bin/env bash
# Vérifie les invariants du projet que la relecture ne tiendrait pas.
#
# Ils sont énoncés dans docs/specs/00-fondations.md. Les inscrire dans une
# vérification automatique plutôt que dans une relecture est ce qui les rend
# vrais dans six mois.

set -euo pipefail

readonly REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
readonly MAIN_MAX_LINES=40

readonly RED=$'\033[31m'
readonly GREEN=$'\033[32m'
readonly RESET=$'\033[0m'

failures=0

report_failure() {
    printf '%s✗%s %s\n' "${RED}" "${RESET}" "$*" >&2
    failures=$((failures + 1))
}

report_success() {
    printf '%s✓%s %s\n' "${GREEN}" "${RESET}" "$*"
}

# Invariant 1 — subedit_core ne connaît ni Qt ni aucune interface graphique.
check_core_has_no_ui() {
    local core="${REPO_ROOT}/src/lib/subedit/core"
    [[ -d "${core}" ]] || return 0

    local offenders
    offenders="$(grep -rlE '#include[[:space:]]*[<"](Q[A-Z]|QtCore|QtWidgets|QtGui|gtk|gtkmm)' \
        "${core}" 2>/dev/null || true)"

    if [[ -n "${offenders}" ]]; then
        report_failure "subedit_core inclut une dépendance d'interface graphique :
$(printf '    %s\n' ${offenders})
    le cœur doit rester utilisable par la CLI et les tests, sans toolkit."
    else
        report_success "subedit_core est libre de toute dépendance d'interface"
    fi
}

# Invariant 2 — un main ne contient que du câblage.
check_executables_are_thin() {
    local main_file
    while IFS= read -r -d '' main_file; do
        local relative="${main_file#"${REPO_ROOT}"/}"

        local lines
        lines="$(grep -cve '^[[:space:]]*$' -e '^[[:space:]]*//' "${main_file}" || true)"
        if (( lines > MAIN_MAX_LINES )); then
            report_failure "${relative} fait ${lines} lignes utiles, maximum ${MAIN_MAX_LINES}
    déplacer la logique dans une bibliothèque de src/lib."
            continue
        fi

        if grep -qE '^[[:space:]]*(class|struct)[[:space:]]+[A-Z]' "${main_file}"; then
            report_failure "${relative} définit un type
    un exécutable câble, il ne modélise pas."
            continue
        fi

        report_success "${relative} reste du câblage (${lines} lignes utiles)"
    done < <(find "${REPO_ROOT}/src/exe" -name 'main.cpp' -print0 2>/dev/null)
}

# Invariant 3 — les scripts sont exécutables dans l'index git.
#
# Un script commité en 100644 s'exécute sans problème en local, où le bit
# d'exécution existe sur le disque, et échoue en CI sur un « Permission
# denied » après un clone frais. Le mode enregistré dans git est donc la seule
# source de vérité qui compte.
check_scripts_are_executable() {
    local offenders
    offenders="$(git -C "${REPO_ROOT}" ls-files -s src/scripts \
        | awk '$1 != "100755" { print $4 }')"

    if [[ -n "${offenders}" ]]; then
        report_failure "scripts non exécutables dans l'index git :
$(printf '    %s\n' ${offenders})
    corriger avec : git update-index --chmod=+x <fichier>"
    else
        report_success "les scripts de src/scripts sont exécutables"
    fi
}

# Invariant 5 — aucun nom de cas de test ne commence par un tiret.
#
# CTest passe le nom du cas en argument au binaire de test, et Catch2 lit alors
# un nom commençant par un tiret comme une de ses propres options :
# « -v alone is the default » devient une demande de verbosité, et le cas
# échoue sans que son intitulé ait rien d'anormal.
#
# Ce qui rend le défaut coûteux, c'est qu'il ne se voit **que** à travers CTest,
# donc dans la porte et dans la CI. Lancer le binaire à la main exécute tous
# les cas d'un coup, sans jamais nommer aucun d'eux : le test passe. Il a été
# payé deux fois pendant l'écriture de la CLI avant d'être inscrit ici.
# Invariant — aucun intitulé de cas de test n est en français.
#
# **La règle de langue avait une moitié dérivée, et une moitié qu on a crue
# fausse** — issue #273. Elle disait « anglais pour les commentaires et les
# intitulés de tests ». Les intitulés étaient bel et bien anglais — sauf ceux d
# une phase, quatre-vingt-dix-huit d un coup, et un `--list-tests` rendait deux
# langues mêlées : c est ce que ce contrôle-ci tient, et cette moitié-là est
# réglée.
#
# **L autre moitié a été retournée à l envers, et #312 l a remise d aplomb.**
# #273 a conclu que les commentaires étaient français partout, en cherchant les
# fichiers portant au moins un caractère accentué — une mesure qui ne pouvait
# rendre que cela. Comptée ligne à ligne : 6 460 anglaises contre 1 161 sur le
# C++, et l inverse exact sur les scripts. Le C++ se commente donc en anglais,
# et `check_cpp_comments_are_english` le tient désormais.
#
# La frontière retenue : **ce que le binaire imprime est en anglais, ce qui
# explique pourquoi est en français.** Un intitulé de test est du premier côté ;
# un commentaire du second.
#
# **Le contrôle est un filet, et il le dit.** Reconnaître une langue est une
# heuristique : celle-ci cherche un caractère hors ASCII, ou l un des mots
# outils qui n existent qu en français. Rejouée sur l arbre d avant la
# traduction, elle relevait 102 intitulés — dont deux qu une relecture attentive
# avait laissés passer. C est ce qui lui vaut sa place.
#
# **L exemption est nommée, une seule, et elle n est pas une entorse** : un
# intitulé anglais qui cite un mot-clé du format des fixtures, lequel est
# français. Une liste par titre plutôt qu un motif, pour qu ajouter une
# exemption soit un geste qu on voit dans un diff.
check_test_titles_are_english() {
    local tests="${REPO_ROOT}/src/test"
    [[ -d "${tests}" ]] || return 0

    local french_words='le|les|un|une|des|du|et|qui|que|ne|pas|dans|pour|avec'
    french_words+='|sans|est|sont|aux|cette|ses|leur|elle|sur|quand|deux|rien'
    french_words+='|tel|telle|autre|autres|chaque|toujours|jamais|donc'

    # Les intitulés anglais qui portent une donnée française, et pourquoi.
    local -a allowed=(
        # Le mot-clé du format .cas, qui est français parce que les fixtures le
        # sont : l intitulé le cite comme son voisin cite « = ».
        '"an expected of supprimé means the subtitle does not survive"'
    )

    local found
    found="$(grep -rhoP 'TEST_CASE\(\s*"(?:[^"\\]|\\.)*"' "${tests}" 2>/dev/null \
        | sed 's/^TEST_CASE(\s*//' \
        | grep -P "[^\x00-\x7F]|(^|[^[:alnum:]])(${french_words})([^[:alnum:]]|$)" \
        | sort -u || true)"

    local one
    for one in "${allowed[@]}"; do
        found="$(grep -Fxv "${one}" <<< "${found}" || true)"
    done

    if [[ -n "${found}" ]]; then
        report_failure "des intitulés de cas de test ne sont pas en anglais :
$(printf '%s\n' "${found}" | sed 's/^/    /')
    ce que le binaire imprime est en anglais ; ce qui explique pourquoi est en
    français. Un intitulé cite parfois une donnée française — l inscrire alors
    dans la liste « allowed » de ce contrôle, avec sa raison."
    else
        report_success "aucun intitulé de cas de test n est en français"
    fi
}

check_test_names_are_not_options() {
    local tests="${REPO_ROOT}/src/test"
    [[ -d "${tests}" ]] || return 0

    local offenders
    offenders="$(grep -rn 'TEST_CASE("-' "${tests}" 2>/dev/null || true)"

    if [[ -n "${offenders}" ]]; then
        report_failure "un nom de cas de test commence par un tiret :
$(printf '    %s\n' "${offenders}")
    CTest le passe en argument, et Catch2 y lit une option ; le renommer."
    else
        report_success "aucun nom de cas de test ne peut passer pour une option"
    fi
}

# Invariant 4 — un tag de version sur HEAD correspond à la version déclarée.
#
# Le tag et project(VERSION) sont deux écritures du même numéro, et rien ne les
# tenait ensemble : un tag posé sans bumper le CMake donne un binaire qui
# annonce une version périmée. La vérification est inerte tant qu'aucun tag ne
# pointe sur HEAD, donc elle ne gêne pas le travail courant.
# Invariant 6 — rien sous src/ ne lit le dépôt de référence.
#
# reference/gaupol est un clone de Gaupol, présent pour être lu par un humain.
# Il n'est pas suivi par git, ne fait pas partie du projet, et son arborescence
# est maintenue non inscriptible. Il est absent de la CI, absent de toute
# machine fraîchement clonée, et absent d'une archive du dépôt.
#
# Un test qui le lit passe donc chez qui l'a et échoue chez tous les autres —
# ou, pire, se déclare ignoré et ne prouve plus rien tout en restant vert. La
# tentation est concrète : la phase 4 reprend peut-être le format de motifs de
# Gaupol, et ses fichiers sont là, à portée de chemin relatif.
#
# La donnée de test versionnée vit dans src/test/data/. src/data/ n'est pas une
# option : il est ignoré par git, c'est le corpus privé de chaque machine.
#
# Le contrôle porte sur le code et les tests, pas sur src/scripts : ce sont eux
# qui pilotent le verrou du clone et interdisent de le commiter, et les nommer
# est leur travail. Une liste d'exceptions par fichier aurait grandi à chaque
# script ajouté ; viser les trois répertoires qui n'ont rien à y faire ne
# grandit jamais.
check_nothing_reads_the_reference() {
    local offenders=""
    local directory
    for directory in lib exe test; do
        [[ -d "${REPO_ROOT}/src/${directory}" ]] || continue
        offenders+="$(grep -rn 'reference/' "${REPO_ROOT}/src/${directory}" 2>/dev/null || true)"
    done

    if [[ -n "${offenders}" ]]; then
        report_failure "du code de src/ désigne le dépôt de référence :
$(printf '    %s\n' "${offenders}")
    Il est absent de la CI et de toute machine qui ne l'a pas cloné.
    La donnée de test versionnée vit dans src/test/data/."
    else
        report_success "rien sous src/ ne lit le dépôt de référence"
    fi
}

# Invariant 7 — rien sous src/ n'est engendré par un outil.
#
# Quatre portes filtrent sur `src/` : le format, l'analyse statique, le cliquet
# de couverture et le calcul du périmètre de clang-tidy. Aucune ne sait
# distinguer une ligne écrite d'une ligne produite ; toutes sont pourtant
# correctes, et **pour une seule raison** : Qt engendre dans `build/`, et `src/`
# ne contient que ce qu'un humain a tapé.
#
# Cette raison n'était écrite nulle part, donc rien ne la tenait. Un `.ui`
# compilé au mauvais endroit, un fichier recopié depuis un répertoire de
# construction, et les quatre portes se mettent à lire du code que personne n'a
# écrit — sans rien signaler.
#
# **Le cliquet de couverture est le plus insidieux des quatre.** Un format qui
# se plaint se voit ; un compte de lignes faussé ne se voit pas, il se croit.
#
# Ce que le contrôle cherche : les noms que moc, uic et rcc donnent à leur
# production, et l avertissement qu ils écrivent en tête de chaque fichier. Si
# un jour un fichier engendré doit vraiment vivre sous src/, ce contrôle le
# nommera — et c est là qu il faudra rouvrir la question, pas après coup.
check_sources_are_handwritten() {
    local by_name by_marker offenders

    by_name="$(find "${REPO_ROOT}/src" \
        \( -name 'moc_*.cpp' -o -name 'ui_*.h' -o -name 'qrc_*.cpp' -o -name '*.moc' \
        -o -name 'mocs_compilation*.cpp' \) -printf '%P\n' 2>/dev/null || true)"

    # L avertissement que les trois outils de Qt écrivent en tête de ce qu ils
    # produisent, et qu aucun fichier écrit à la main ne porte.
    #
    # **Il est assemblé plutôt qu écrit en clair**, sans quoi ce script se
    # dénoncerait lui-même : il vit sous src/, et le motif qu il cherche y
    # figurerait. Le défaut a été constaté à la première exécution.
    local marker="All changes made in this file"
    by_marker="$(grep -rl "${marker} will be lost" \
        "${REPO_ROOT}/src" 2>/dev/null | sed "s|^${REPO_ROOT}/src/||" || true)"

    offenders="$(printf '%s\n%s\n' "${by_name}" "${by_marker}" | sed '/^$/d' | sort -u)"

    if [[ -n "${offenders}" ]]; then
        report_failure "du code engendré vit sous src/ :
$(printf '    %s\n' ${offenders})
    Le format, l analyse statique, la couverture et le périmètre de clang-tidy
    filtrent tous sur src/ en supposant qu il est écrit à la main."
    else
        report_success "rien sous src/ n est engendré par un outil"
    fi
}

check_version_matches_tag() {
    local tag
    # « || true » n'est pas décoratif : sans tag sur HEAD, grep sort en 1, et
    # set -o pipefail joint à set -e interromprait le script au lieu de le
    # laisser conclure qu'il n'y a rien à confronter. C'est le cas ordinaire en
    # intégration continue, où le clone ne récupère pas les tags.
    tag="$(git -C "${REPO_ROOT}" tag --points-at HEAD 2>/dev/null \
        | grep -E '^v[0-9]+\.[0-9]+\.[0-9]+$' | head -1 || true)"

    if [[ -z "${tag}" ]]; then
        report_success "aucun tag de version sur HEAD, rien à confronter"
        return 0
    fi

    # Lue dans le commit, et non dans l'arbre de travail. Un tag désigne un
    # commit ; le confronter à des fichiers modifiés depuis compare deux choses
    # différentes. Le cas se présente au début de chaque phase : la branche part
    # du commit fusionné, qui porte le tag de la phase précédente, et le premier
    # bump de la nouvelle phase déclencherait une fausse alerte.
    local declared
    declared="$(git -C "${REPO_ROOT}" show HEAD:CMakeLists.txt 2>/dev/null \
        | sed -n 's/^[[:space:]]*VERSION[[:space:]]\+\([0-9][0-9.]*\)[[:space:]]*$/\1/p' | head -1)"

    if [[ "v${declared}" != "${tag}" ]]; then
        report_failure "le tag ${tag} et CMakeLists.txt (${declared:-absent}) ne s'accordent pas
    bumper project(VERSION) avant de poser le tag, sinon le binaire ment."
    else
        report_success "le tag ${tag} correspond à la version déclarée"
    fi
}

# Invariant 8 — le modèle ne dépend d'aucune opération.
#
# ADR 0018 tranche le sens : `format/` traduit des octets en modèle, donc il a
# besoin de lui ; le modèle n'a jamais besoin d'un lecteur. Ce qui rend la règle
# tenable, c'est qu'elle est mécanique — la frontière s'est effacée une fois
# déjà, quand les types propres aux formats ont été rangés sous `model/` sans
# que rien ne le signale.
check_model_depends_on_no_operation() {
    local model="${REPO_ROOT}/src/lib/subedit/core/model"
    [[ -d "${model}" ]] || return 0

    local offenders
    offenders="$(grep -rlE '#include[[:space:]]*<subedit/core/(format|io|edit|command)/' \
        "${model}" 2>/dev/null || true)"

    if [[ -n "${offenders}" ]]; then
        report_failure "le modèle dépend d une opération :
$(printf '    %s\n' ${offenders})
    format/, io/, edit/ et command/ dépendent du modèle ; l inverse ferme un cycle."
    else
        report_success "le modèle ne dépend d aucune opération"
    fi
}

# Invariant — le C++ se commente en anglais, tout le reste en français.
#
# **La frontière est celle du fichier, et le compte le dit** : sur `src/`, 6 460
# lignes de commentaire anglaises contre 1 161 françaises ; sur les scripts,
# 1 971 françaises contre 6 ; sur le système de construction, 386 contre 4.
# Chaque famille de fichiers a sa langue, et une seule des deux a dérivé.
#
# **Elle a dérivé, et la dérive a une date.** Par semaine d'écriture, la part de
# français dans les commentaires C++ : 0,0 %, puis 0,3 %, puis 2,7 %, puis
# 30,2 % et 32,8 %. Trois semaines d'anglais, puis les phases d'interface. C'est
# ce que #273 a vu à la fin de la quatrième semaine, et il a conclu que le dépôt
# avait toujours été français — en cherchant les fichiers portant au moins un
# caractère accentué, ce qui ne pouvait rendre que cela. Tranché en #312.
#
# Le contrôle vit dans un script à lui : il compte, et un compte se lit mieux en
# Python qu'en awk. Voir `check-comment-language.py` pour ce qu'il reconnaît et
# pour la faiblesse d'un cliquet global.
check_cpp_comments_are_english() {
    if "${REPO_ROOT}/src/scripts/check-comment-language.py"; then
        return 0
    fi

    failures=$((failures + 1))
}

check_core_has_no_ui
check_cpp_comments_are_english
check_sources_are_handwritten
check_executables_are_thin
check_scripts_are_executable
check_version_matches_tag
check_test_names_are_not_options
check_test_titles_are_english
check_nothing_reads_the_reference
check_model_depends_on_no_operation

if (( failures > 0 )); then
    printf '\n%s%d invariant(s) d architecture violé(s)%s\n' "${RED}" "${failures}" "${RESET}" >&2
    exit 1
fi
