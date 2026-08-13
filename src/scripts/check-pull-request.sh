#!/usr/bin/env bash
# Vérifie les trois obligations d'une pull request que rien d'autre n'attrape.
#
# La CI vérifie déjà la porte de qualité, la grammaire des messages de commit et
# le titre de la pull request. Trois obligations lui échappaient, toutes
# mécaniquement décidables, toutes faites à la main jusqu'ici : la ligne
# « Closes #N », le bump du patch et la régénération du journal.
#
# Appelé par .github/workflows/pull-request.yml, qui lui passe les valeurs de
# l'événement GitHub par l'environnement. Isolé ici plutôt qu'écrit dans le
# YAML : un script se teste en local, pas une étape de workflow — et
# src/scripts/verify-gates.sh prouve que chacun des trois contrôles se referme.
#
# Entrées :
#
#   PR_BODY   corps de la pull request
#   BASE_SHA  commit de la branche de base, pour y lire la version de départ
#
# Arguments : les contrôles à exécuter parmi « closes », « version » et
# « changelog ». Sans argument, les trois. Les nommer un par un sert à la
# preuve : une injection ne démontre son contrôle que si elle est seule à
# pouvoir faire échouer l'exécution.
#
# Les contrôles demandés sont tous évalués, sans arrêt au premier échec : une
# pull request à qui il manque deux choses les apprend en un tour.

set -euo pipefail

readonly REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"

PR_BODY="${PR_BODY:-}"
BASE_SHA="${BASE_SHA:-}"

readonly RED=$'\033[31m'
readonly GREEN=$'\033[32m'
readonly BOLD=$'\033[1m'
readonly RESET=$'\033[0m'

failures=0

pass() {
    printf '  %s✓%s %s\n' "${GREEN}" "${RESET}" "$1"
}

# Premier argument : le constat. Les suivants : les lignes d'explication.
fail() {
    printf '  %s✗%s %s\n' "${RED}" "${RESET}" "$1"
    shift
    local line
    for line in "$@"; do
        printf '      %s\n' "${line}"
    done
    failures=$((failures + 1))
}

# --- 1. Le corps porte « Closes #N » ----------------------------------------
#
# Le seul des trois contrôles avec une preuve mesurée : pendant la phase 2, les
# pull requests disaient « Ferme #22 », en appliquant la règle de langue du
# projet. GitHub ne reconnaît que ses propres mots-clés, tous anglais — cinq
# issues sont restées ouvertes après la fusion de leur pull request.
#
# Seul « Closes » est accepté ici, alors que GitHub en reconnaît huit (« fixes »,
# « resolves »…). C'est délibéré : CLAUDE.md nomme « Closes #N », et un contrôle
# plus permissif que la règle laisserait la règle dériver — ce qu'il existe
# précisément pour empêcher.
check_closes() {
    # GitHub rend le corps d'une pull request avec des fins de ligne CRLF. Le
    # \r final ferait échouer l'ancre de fin de ligne sur un corps correct.
    local body="${PR_BODY//$'\r'/}"

    # La ligne peut porter autre chose après le numéro — GitHub ferme l'issue
    # dans ce cas, et refuser « Closes #42, la première des trois » refuserait
    # une pull request correcte. Ce qui est exigé est que la ligne commence par
    # le mot-clé et son numéro.
    if grep -Eq '^[[:space:]]*Closes #[0-9]+([^0-9].*)?$' <<<"${body}"; then
        pass "le corps porte une ligne « Closes #N »"
        return
    fi

    fail "le corps ne porte pas de ligne « Closes #N »" \
        "GitHub ne ferme une issue que sur ses propres mots-clés, tous anglais :" \
        "« Ferme #42 » n'en est pas un, et l'issue reste ouverte après la fusion." \
        "Ajouter une ligne « Closes #42 » au corps de la pull request."
}

# --- 2. Le patch a été incrémenté -------------------------------------------
#
# Le contrôle porte sur « différent », pas sur « patch + 1 » : une pull request
# qui clôt une phase bumpe le mineur et remet le patch à zéro, ce qui fait aussi
# diverger le numéro sans l'incrémenter.
declared_version() {
    sed -n 's/^[[:space:]]*VERSION[[:space:]]\+\([0-9][0-9.]*\)[[:space:]]*$/\1/p' | head -1
}

check_version() {
    if [[ -z "${BASE_SHA}" ]]; then
        fail "BASE_SHA n'est pas renseigné" \
            "sans commit de base, la version de départ est illisible."
        return
    fi

    local base=""
    base="$(git -C "${REPO_ROOT}" show "${BASE_SHA}:CMakeLists.txt" 2>/dev/null \
        | declared_version || true)"
    if [[ -z "${base}" ]]; then
        fail "version illisible dans CMakeLists.txt à ${BASE_SHA:0:8}" \
            "l'historique est-il complet ? le workflow exige fetch-depth: 0."
        return
    fi

    local head=""
    head="$(declared_version <"${REPO_ROOT}/CMakeLists.txt" || true)"
    if [[ -z "${head}" ]]; then
        fail "version illisible dans CMakeLists.txt" \
            "project(VERSION) est la source du numéro courant."
        return
    fi

    if [[ "${head}" != "${base}" ]]; then
        pass "la version passe de ${base} à ${head}"
        return
    fi

    fail "la version n'a pas bougé (${head})" \
        "toute pull request incrémente le patch dans son propre diff." \
        "Bumper project(VERSION) dans CMakeLists.txt."
}

# --- 3. Le CHANGELOG a été régénéré -----------------------------------------
#
# La comparaison se fait entre le fichier tel qu'il est et le fichier tel que
# git-cliff le produit — pas avec « git diff », qui confronterait la copie de
# travail à l'index et laisserait passer une modification que la régénération
# vient d'effacer.
#
# Pas de faux positif sur les commits d'entretien : un commit `chore` est écarté
# par cliff.toml, donc une pull request qui n'en contient que ne fait pas bouger
# le journal — et le contrôle passe, correctement.
check_changelog() {
    local changelog="${REPO_ROOT}/CHANGELOG.md"
    local backup
    backup="$(mktemp)"
    cp "${changelog}" "${backup}"

    if ! make -C "${REPO_ROOT}" --no-print-directory changelog >/dev/null 2>&1; then
        cp "${backup}" "${changelog}"
        rm -f "${backup}"
        fail "la régénération du journal a échoué" \
            "git-cliff est-il installé ? ./src/scripts/setup-toolchain.sh l'installe."
        return
    fi

    local difference=""
    difference="$(diff -u --label 'CHANGELOG.md (commité)' --label 'CHANGELOG.md (régénéré)' \
        "${backup}" "${changelog}" || true)"

    cp "${backup}" "${changelog}"
    rm -f "${backup}"

    if [[ -z "${difference}" ]]; then
        pass "CHANGELOG.md est à jour"
        return
    fi

    fail "CHANGELOG.md n'a pas été régénéré" \
        "lancer « make changelog » et commiter le résultat."
    printf '%s\n' "${difference}" | sed 's/^/      /'
}

# --- Exécution --------------------------------------------------------------

controls=("$@")
if (( ${#controls[@]} == 0 )); then
    controls=(closes version changelog)
fi

printf '%scontrôles de pull request%s\n\n' "${BOLD}" "${RESET}"

for control in "${controls[@]}"; do
    case "${control}" in
        closes) check_closes ;;
        version) check_version ;;
        changelog) check_changelog ;;
        *)
            printf '%scontrôle inconnu : %s%s\n' "${RED}" "${control}" "${RESET}" >&2
            printf 'attendus : closes, version, changelog\n' >&2
            exit 2
            ;;
    esac
done

printf '\n'
if (( failures > 0 )); then
    printf '%s%d contrôle(s) en échec%s\n' "${RED}" "${failures}" "${RESET}" >&2
    exit 1
fi
printf '%sles contrôles passent%s\n' "${GREEN}" "${RESET}"
