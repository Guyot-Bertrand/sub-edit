#!/usr/bin/env bash
# The manual: its examples, its screenshots, its references.
#
# **Three nets, and they do not see the same thing.** The `console` blocks come
# from the binary, produced by really running the command; the images come from
# the real window, shown with no screen; the internal references were checked by
# nothing until #243 — a renamed section leaves behind anchors that display as
# cleanly as a right link.
#
# **One set of screenshots per language** (#672): the shots carry the window's
# words. English, the source, lives in `docs/manual/subedit-gui/captures/`, and
# every language of `src/po/LINGUAS` in `docs/i18n/<lang>/manual/subedit-gui/captures/`,
# taken under its shipped catalogue. A language added to that list gets its set
# from this loop, with no code to change.
#
# `--check` checks without rewriting anything. Without it, everything is
# regenerated: the examples are rewritten, and the shots that moved are
# promoted. The program never writes a reference — it writes `<name>.new.png`,
# and the comparator decides afterwards, tree by tree.

set -euo pipefail

# shellcheck source=src/scripts/gate/common.sh
source "$(dirname "${BASH_SOURCE[0]}")/common.sh"

readonly CAPTURES="${REPO_ROOT}/docs/manual/subedit-gui/captures"
readonly LINGUAS="${REPO_ROOT}/src/po/LINGUAS"

check=0

usage() {
    cat >&2 <<'USAGE'
usage: manual.sh [--check]

  --check  checks without regenerating, and fails on the first difference
USAGE
    exit 2
}

while (($# > 0)); do
    case "$1" in
    --check) check=1; shift ;;
    -h | --help) usage ;;
    *) printf 'unknown argument: %s\n' "$1" >&2; usage ;;
    esac
done

step "manual examples"

timed configure cmake --preset dev >/dev/null
timed build cmake --build --preset dev -j "${JOBS}" --target subedit-cli subedit_screenshots subedit_list_shortcuts

if ((check)); then
    timed examples "${REPO_ROOT}/src/scripts/generate-manual.sh" --check
else
    timed examples "${REPO_ROOT}/src/scripts/generate-manual.sh"
fi

step "screenshots"

# One shot of every screen and its comparison with the references of the same tree.
#
# **The `|| return` is not decoration.** `timed` calls this function on the left of a `||`, where
# bash suspends `set -e` for everything the function runs: without it, a failed shot would go on to
# the comparison, and the step would take the comparator's verdict for its own.
shoot() {
    local directory="$1"
    shift
    "${REPO_ROOT}/build/dev/bin/subedit_screenshots" --output-dir "${directory}" "$@" || return
    if ((check)); then
        "${REPO_ROOT}/src/scripts/compare-screenshots.py" --dir "${directory}" --check
    else
        "${REPO_ROOT}/src/scripts/compare-screenshots.py" --dir "${directory}"
    fi
}

timed "screenshots (en)" shoot "${CAPTURES}"
# Every installed language, in the order of the list; blank lines and comments are not languages.
while IFS= read -r language; do
    language="${language%%#*}"
    language="${language//[[:space:]]/}"
    [[ -n "${language}" ]] || continue
    timed "screenshots (${language})" shoot "${REPO_ROOT}/docs/i18n/${language}/manual/subedit-gui/captures" \
        --language "${language}"
done <"${LINGUAS}"

"${REPO_ROOT}/src/scripts/check-screenshots.py"
"${REPO_ROOT}/src/scripts/check-manual-links.py"
"${REPO_ROOT}/src/scripts/check-translation-structure.py"
"${REPO_ROOT}/src/scripts/check-cli-manual.py" --binary "${REPO_ROOT}/build/dev/bin/subedit-cli"
"${REPO_ROOT}/src/scripts/check-gui-manual.py" --binary "${REPO_ROOT}/build/dev/bin/subedit_list_shortcuts"
