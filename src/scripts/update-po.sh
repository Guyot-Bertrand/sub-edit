#!/usr/bin/env bash
# Merge the template into every language catalogue — issues #659 and #667.
#
# The template is rewritten first: merging a stale template would teach nothing. Then every
# `src/po/<lang>.po` is brought up to date with `msgmerge` against `subedit.pot` — new messages
# added empty, vanished ones commented out as obsolete:
#
# - **a language of src/po/LINGUAS**, installed and reviewed, is merged with fuzzy matching: a
#   changed sentence comes back as its old translation flagged "fuzzy", which a translator reviews;
# - **a seed** — any other `.po`, imported from Gaupol by `seed-catalogues.py` and not installed
#   (spec 15, D5) — is merged without fuzzy matching: nobody reviews a seed, so a guess would only
#   add noise that the next re-seed throws away. Its translations are kept, and a fresh seed is
#   left byte for byte as it is.

set -euo pipefail

readonly REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
readonly PO_DIR="${REPO_ROOT}/src/po"

RED=$'\033[31m'
GREEN=$'\033[32m'
RESET=$'\033[0m'

if ! command -v msgmerge >/dev/null 2>&1; then
    printf '%s✗ msgmerge cannot be found%s\n' "${RED}" "${RESET}" >&2
    printf '  install it with: ./src/scripts/setup-toolchain.sh\n' >&2
    exit 1
fi

"${REPO_ROOT}/src/scripts/update-pot.sh"

installed="$(grep -v '^[[:space:]]*\(#\|$\)' "${PO_DIR}/LINGUAS" || true)"

for catalogue in "${PO_DIR}"/*.po; do
    language="$(basename "${catalogue}" .po)"
    if grep -qxF "${language}" <<<"${installed}"; then
        msgmerge --update --backup=none --quiet "${catalogue}" "${PO_DIR}/subedit.pot"
        printf '%s✓%s %s\n' "${GREEN}" "${RESET}" "src/po/${language}.po"
    else
        msgmerge --update --backup=none --quiet --no-fuzzy-matching "${catalogue}" "${PO_DIR}/subedit.pot"
        printf '%s✓%s %s (seed)\n' "${GREEN}" "${RESET}" "src/po/${language}.po"
    fi
done

# A language of LINGUAS without its file is an error the build reports too; say it here first.
while IFS= read -r language; do
    [[ -n "${language}" ]] || continue
    if [[ ! -f "${PO_DIR}/${language}.po" ]]; then
        printf '%s✗%s src/po/LINGUAS names "%s", and src/po/%s.po does not exist\n' \
            "${RED}" "${RESET}" "${language}" "${language}" >&2
        exit 1
    fi
done <<<"${installed}"
