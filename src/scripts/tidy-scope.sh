#!/usr/bin/env bash
# Les fichiers que clang-tidy doit analyser pour une modification donnée.
#
# Écrit un `.cpp` par ligne sur la sortie standard, à passer à
# `make tidy TIDY_FILES="$(…)"`. Sur la sortie d'erreur, il écrit **ce qu'il a
# retenu et d'où il l'a tiré** : un périmètre qu'on ne voit pas est un périmètre
# qu'on ne vérifie pas.
#
#     ./src/scripts/tidy-scope.sh origin/main
#
# Sans argument, ou si rien ne permet de restreindre, il écrit la liste
# complète : **le défaut est toujours d'en analyser plus, jamais moins.**
#
# Pourquoi restreindre. clang-tidy est **90 % de la porte** — 751 s mesurées sur
# 827, ccache chaud, contre 76 s pour tout le reste réuni. Le coût est linéaire,
# environ 6 s par fichier, donc la restriction rapporte exactement en proportion
# de ce qu'une branche ne touche pas.
#
# **La fermeture transitive des en-têtes est le point.** Modifier
# `hearing_impaired.hpp` concerne les deux `.cpp` qui l'incluent ; modifier
# `project.hpp` concerne presque tout le projet, à travers des en-têtes qui
# l'incluent sans qu'aucun `.cpp` ne le nomme. Un seul niveau d'inclusion
# manquerait ces derniers, et manquerait donc l'avertissement.
#
# **Le travail non commité compte.** C'est le défaut que la relecture de la
# phase 4 a trouvé : le script ne regardait que `base...HEAD`, ce qui est juste
# en intégration continue — le travail y est toujours commité — et faux en
# local, où l'on lance la porte *avant* de commiter. Sur une branche sans commit
# et quatre fichiers modifiés, il rendait zéro fichier, donc une porte verte qui
# n'avait rien analysé.
#
# L'analyse complète reste jouée chaque semaine sur `main` — voir ci.yml. Ce
# script réduit ce que coûte une pull request, il ne supprime rien.

set -euo pipefail

readonly REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"

# Un fichier de cette liste change la façon dont *tout* est analysé ou compilé :
# la liste des contrôles, les drapeaux, les presets. Restreindre après l'un
# d'eux n'aurait aucun sens.
readonly WIDE_PATTERN='^(\.clang-tidy|Makefile|CMakePresets\.json|cmake/|src/scripts/tidy-scope\.sh)$'

readonly VERSION_LINE='^[+-][[:space:]]*VERSION[[:space:]]'
readonly SOURCE_LINE='^[+-][[:space:]]*[A-Za-z0-9_./-]+\.(cpp|hpp)[[:space:]]*$'

say() { printf '%s\n' "$*" >&2; }

everything() {
    find "${REPO_ROOT}/src" -name '*.cpp' -printf '%P\n' 2>/dev/null | sed 's|^|src/|' | sort
}

all_of_it() {
    local why="$1"
    local count
    count="$(everything | wc -l)"
    say "périmètre de clang-tidy : les ${count} fichiers — ${why}"
    everything
}

base="${1:-}"
if [[ -z "${base}" ]]; then
    all_of_it "aucune base donnée"
    exit 0
fi

# Une base inconnue — clone superficiel, référence absente — n'est pas une
# raison d'analyser moins.
if ! git -C "${REPO_ROOT}" rev-parse --verify --quiet "${base}" >/dev/null; then
    all_of_it "base introuvable : ${base}"
    exit 0
fi

# **Le point de comparaison est l'ancêtre commun, et l'autre bout est l'arbre de
# travail.** Un simple `git diff ancêtre` embrasse ainsi d'un coup les trois
# états où une modification peut se trouver : commitée sur la branche, ajoutée à
# l'index, ou seulement écrite sur le disque.
fork="$(git -C "${REPO_ROOT}" merge-base "${base}" HEAD)"

changed_lines_of() {
    git -C "${REPO_ROOT}" diff -U0 "${fork}" -- "$1" \
        | grep -E '^[+-]' | grep -vE '^(\+\+\+|---)' || true
}

# Vrai si toutes les lignes changées de $1 répondent au motif $2 — donc si ce
# fichier n'a bougé que d'une façon dont on sait qu'elle ne change rien à
# l'analyse.
#
# **C'est ce qui rend la restriction utile plutôt que théorique.** Les fichiers
# de construction bougent à presque chaque pull request, et pour deux raisons
# qui ne gouvernent rien : le bump de patch que la convention impose, et un
# fichier source ajouté à une cible. Les compter larges sans les lire ferait
# retomber chaque pull request sur l'analyse complète, et ce script ne servirait
# jamais. Une ligne qui ne répond pas au motif — un drapeau, un chemin
# d'inclusion, une bibliothèque — rend la main à l'analyse complète.
only_lines_matching() {
    local touched
    touched="$(changed_lines_of "$1")"
    [[ -n "${touched}" ]] || return 0
    ! grep -qvE "$2" <<<"${touched}"
}

# Tout ce qui diffère de l'ancêtre commun, plus ce que git ne suit pas encore :
# un fichier source fraîchement créé n'a jamais autant besoin d'être analysé.
changed="$({
    git -C "${REPO_ROOT}" diff --name-only "${fork}"
    git -C "${REPO_ROOT}" ls-files --others --exclude-standard
} | sort -u)"

if [[ -z "${changed}" ]]; then
    say "périmètre de clang-tidy : rien — aucune différence avec ${base}"
    exit 0
fi

if grep -qE "${WIDE_PATTERN}" <<<"${changed}"; then
    all_of_it "un fichier gouvernant l'analyse a changé"
    exit 0
fi

# La racine fixe la norme et les options : large, sauf quand elle n'a reçu que
# le bump de patch que la convention impose à chaque pull request.
if grep -qxF 'CMakeLists.txt' <<<"${changed}" \
    && ! only_lines_matching CMakeLists.txt "${VERSION_LINE}"; then
    all_of_it "CMakeLists.txt racine a changé ailleurs que sur sa version"
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
# atteints, plus tout un répertoire dont le CMakeLists.txt a bougé autrement
# qu'en ajoutant une source.
scope="$(grep -E '\.cpp$' <<<"${changed}" | grep -E '^src/' || true)"$'\n'

while IFS= read -r lists; do
    [[ -n "${lists}" ]] || continue
    only_lines_matching "${lists}" "${SOURCE_LINE}" && continue
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

# Un fichier supprimé par la modification est encore dans le diff, et n'existe
# plus sur le disque : l'analyser échouerait sur un fichier introuvable.
retained="$(printf '%s' "${scope}" | sed '/^$/d' | sort -u | while IFS= read -r file; do
    [[ -f "${REPO_ROOT}/${file}" ]] && printf '%s\n' "${file}"
done)"

# **Le garde-fou.** Des sources ont changé et le calcul ne retient rien : c'est
# que le calcul est en défaut, jamais que le travail l'est. Rendre la liste vide
# donnerait une porte verte qui n'a rien analysé — l'échec silencieux qu'on
# cherche précisément à rendre impossible. On retombe sur tout, bruyamment.
sources="$(grep -E '\.(cpp|hpp)$' <<<"${changed}" | grep -E '^src/' || true)"
if [[ -z "${retained}" && -n "${sources}" ]]; then
    all_of_it "des sources ont changé mais le calcul n'a rien retenu, ce qui est un défaut du calcul"
    exit 0
fi

if [[ -z "${retained}" ]]; then
    say "périmètre de clang-tidy : rien — aucune source touchée depuis ${base}"
    exit 0
fi

say "périmètre de clang-tidy : $(wc -l <<<"${retained}") fichier(s) sur $(everything | wc -l), depuis ${base}"
printf '%s\n' "${retained}"
