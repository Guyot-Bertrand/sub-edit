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

check_core_has_no_ui
check_executables_are_thin
check_scripts_are_executable
check_version_matches_tag
check_test_names_are_not_options

if (( failures > 0 )); then
    printf '\n%s%d invariant(s) d architecture violé(s)%s\n' "${RED}" "${failures}" "${RESET}" >&2
    exit 1
fi
