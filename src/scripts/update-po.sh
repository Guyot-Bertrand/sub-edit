#!/usr/bin/env bash
# Fusionne le gabarit dans chaque catalogue de langue — issue #659.
#
# Pour chaque langue de src/po/LINGUAS : `msgmerge` met `<lang>.po` à jour d'après
# `subedit.pot` — messages neufs ajoutés, messages disparus mis en commentaire, phrases
# modifiées reprises en « fuzzy » par ressemblance. Le gabarit est réécrit d'abord : fusionner
# un gabarit périmé n'apprendrait rien.

set -euo pipefail

readonly REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
readonly PO_DIR="${REPO_ROOT}/src/po"

RED=$'\033[31m'
GREEN=$'\033[32m'
RESET=$'\033[0m'

if ! command -v msgmerge >/dev/null 2>&1; then
    printf '%s✗ msgmerge est introuvable%s\n' "${RED}" "${RESET}" >&2
    printf '  l'\''installer avec : ./src/scripts/setup-toolchain.sh\n' >&2
    exit 1
fi

"${REPO_ROOT}/src/scripts/update-pot.sh"

while IFS= read -r language; do
    [[ -n "${language}" && "${language}" != \#* ]] || continue
    msgmerge --update --backup=none --quiet "${PO_DIR}/${language}.po" "${PO_DIR}/subedit.pot"
    printf '%s✓%s %s\n' "${GREEN}" "${RESET}" "src/po/${language}.po"
done <"${PO_DIR}/LINGUAS"
