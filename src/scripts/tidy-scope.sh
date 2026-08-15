#!/usr/bin/env bash
# Les fichiers que clang-tidy doit analyser pour une modification donnée.
#
# Écrit un `.cpp` par ligne, à passer à `make tidy TIDY_FILES="$(…)"`. Sans
# argument, ou si rien ne permet de restreindre, il écrit la liste complète :
# **le défaut est toujours d'en analyser plus, jamais moins.**
#
#     ./src/scripts/tidy-scope.sh origin/main
#
# Pourquoi restreindre. clang-tidy est le gros de `make check`, et le quota
# d'Actions est fini. Une pull request qui touche un fichier n'a pas besoin de
# faire réanalyser les cent autres — mais elle a besoin de faire réanalyser tout
# ce que son fichier peut casser, et c'est là que le calcul se mérite.
#
# **La fermeture transitive des en-têtes est le point.** Modifier
# `hearing_impaired.hpp` concerne les deux `.cpp` qui l'incluent ; modifier
# `project.hpp` concerne presque tout le projet, à travers des en-têtes qui
# l'incluent sans qu'aucun `.cpp` ne le nomme. Un seul niveau d'inclusion
# manquerait ces derniers, et manquerait donc l'avertissement.
#
# L'analyse complète reste jouée chaque semaine sur `main` — voir ci.yml. Ce
# script réduit ce que coûte une pull request, il ne supprime rien.

set -euo pipefail

readonly REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"

# Un fichier de cette liste change la façon dont *tout* est analysé ou compilé :
# la liste des contrôles, les drapeaux, les presets. Restreindre après l'un
# d'eux n'aurait aucun sens.
#
# Un CMakeLists.txt imbriqué n'y est **pas**, délibérément : le plus souvent il
# ajoute un fichier à une cible, ce qui ne change rien à la façon dont les
# autres sont analysés. Il met en cause son répertoire, et c'est traité plus
# bas. S'il changeait un drapeau de compilation, seul l'hebdomadaire complet le
# verrait — c'est le risque assumé, et il est écrit.
readonly WIDE_PATTERN='^(\.clang-tidy|Makefile|CMakePresets\.json|cmake/|src/scripts/tidy-scope\.sh)$'

# Le CMakeLists.txt racine est large — il fixe la norme et les options — mais
# **toute pull request du projet le touche**, pour le seul bump de patch que la
# convention impose. Le compter large sans regarder ce qu'il change ferait
# retomber chaque pull request sur l'analyse complète, et cette restriction ne
# servirait jamais à rien.
#
# On regarde donc ce qui a bougé dedans : si ce n'est que la ligne VERSION, rien
# de ce qui gouverne l'analyse n'a changé.
version_bump_only() {
    local touched
    touched="$(git -C "${REPO_ROOT}" diff -U0 "${base}...HEAD" -- CMakeLists.txt \
        | grep -E '^[+-]' | grep -vE '^(\+\+\+|---)' || true)"
    [[ -n "${touched}" ]] || return 0
    ! grep -qvE '^[+-][[:space:]]*VERSION[[:space:]]' <<<"${touched}"
}

everything() {
    find "${REPO_ROOT}/src" -name '*.cpp' -printf '%P\n' 2>/dev/null | sed 's|^|src/|' | sort
}

base="${1:-}"
if [[ -z "${base}" ]]; then
    everything
    exit 0
fi

# Une base inconnue — clone superficiel, référence absente — n'est pas une
# raison d'analyser moins. On le dit, et on analyse tout.
if ! git -C "${REPO_ROOT}" rev-parse --verify --quiet "${base}" >/dev/null; then
    printf 'base introuvable : %s — analyse complète\n' "${base}" >&2
    everything
    exit 0
fi

changed="$(git -C "${REPO_ROOT}" diff --name-only "${base}...HEAD" || true)"
if [[ -z "${changed}" ]]; then
    exit 0
fi

if grep -qE "${WIDE_PATTERN}" <<<"${changed}"; then
    everything
    exit 0
fi

if grep -qxF 'CMakeLists.txt' <<<"${changed}" && ! version_bump_only; then
    everything
    exit 0
fi

# Les en-têtes touchés, puis tous ceux qui les incluent, jusqu'au point fixe.
# Un en-tête est désigné par son nom de fichier : c'est ce qu'une ligne
# `#include` porte, que le chemin soit absolu depuis src/lib ou relatif au
# répertoire. Viser plus large ici ne coûte que du temps d'analyse ; viser trop
# étroit coûterait un avertissement manqué.
headers="$(grep -E '\.hpp$' <<<"${changed}" || true)"
seen=""
while [[ -n "${headers}" ]]; do
    seen="$(printf '%s\n%s\n' "${seen}" "${headers}" | sed '/^$/d' | sort -u)"
    names="$(xargs -r -n1 basename <<<"${headers}" | sort -u)"

    found=""
    while IFS= read -r name; do
        [[ -n "${name}" ]] || continue
        found+="$(grep -rlF --include='*.hpp' "${name}" "${REPO_ROOT}/src" 2>/dev/null \
            | sed "s|^${REPO_ROOT}/||" || true)"$'\n'
    done <<<"${names}"

    # Ce qui est nouveau à ce tour, et rien d'autre : sans quoi la boucle ne
    # s'arrêterait jamais, un en-tête se contenant lui-même.
    headers="$(printf '%s' "${found}" | sed '/^$/d' | sort -u | comm -23 - <(printf '%s\n' "${seen}"))"
done

# Les .cpp : ceux qui ont changé, plus ceux qui incluent l'un des en-têtes
# atteints, plus tout un répertoire dont le CMakeLists.txt a bougé.
scope="$(grep -E '\.cpp$' <<<"${changed}" | grep -E '^src/' || true)"$'\n'

while IFS= read -r lists; do
    [[ -n "${lists}" ]] || continue
    directory="${REPO_ROOT}/$(dirname "${lists}")"
    [[ -d "${directory}" ]] || continue
    scope+="$(find "${directory}" -name '*.cpp' 2>/dev/null \
        | sed "s|^${REPO_ROOT}/||" || true)"$'\n'
done <<<"$(grep -E '/CMakeLists\.txt$' <<<"${changed}" || true)"
if [[ -n "${seen}" ]]; then
    while IFS= read -r header; do
        [[ -n "${header}" ]] || continue
        scope+="$(grep -rlF --include='*.cpp' "$(basename "${header}")" "${REPO_ROOT}/src" 2>/dev/null \
            | sed "s|^${REPO_ROOT}/||" || true)"$'\n'
    done <<<"${seen}"
fi

# Un fichier supprimé par la pull request est encore dans le diff, et n'existe
# plus sur le disque : l'analyser échouerait sur un fichier introuvable.
printf '%s' "${scope}" | sed '/^$/d' | sort -u | while IFS= read -r file; do
    [[ -f "${REPO_ROOT}/${file}" ]] && printf '%s\n' "${file}"
done
