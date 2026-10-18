#!/usr/bin/env bash
# The message template, extracted from the code and the patterns: src/po/subedit.pot — issues #659
# and #666.
#
# **What the template lists**: every literal passed to the API of `core/i18n/messages.hpp`
# (`translate`, `translateIn`, `translatePlural`) in `src/lib` and `src/exe`. Tests are not read:
# their messages are not the user's. The window has no keyword of its own: `gui/messages.hpp`
# bears the same three names, and no `tr` is left (#665).
#
# **And the names and descriptions of the shipped correction patterns** (spec 15, D6): the files of
# `packaging/patterns/` are Gaupol's, read as they are (ADR 0037), and their `Name` and
# `Description` are English. The window and the command line look them up in the catalogue when
# they show them, by their English text, as they look up any message; so the template must hold
# them, and the files themselves are never rewritten. They are read as the desktop-entry files
# they resemble — `[group]`, then `Key=value` — with `Name` and `Description` as the only
# keywords, and joined to the extraction of the code.
#
# **Reproducible, byte for byte** — what the issue demands, and what `--check` proves: the sources
# are sorted, locations name only the file (a line number would change on every insertion and
# drown a translator's diff), and the one line xgettext dates itself, `POT-Creation-Date`, is
# brought back to the template value the tool writes when it does not know the time. No version
# either: a bump of the patch must not rewrite the template.
#
#   update-pot.sh           rewrites src/po/subedit.pot
#   update-pot.sh --check   fails when the versioned template is not what the sources produce

set -euo pipefail

readonly REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
readonly POT="${REPO_ROOT}/src/po/subedit.pot"

# shellcheck source=src/scripts/lib/output.sh
source "$(dirname "${BASH_SOURCE[0]}")/lib/output.sh"

mode="write"
case "${1:-}" in
"") ;;
--check) mode="check" ;;
*)
    printf 'usage: update-pot.sh [--check]\n' >&2
    exit 2
    ;;
esac

if ! command -v xgettext >/dev/null 2>&1; then
    printf '%s✗ xgettext cannot be found%s\n' "${RED}" "${RESET}" >&2
    printf '  install it with: ./src/scripts/setup-toolchain.sh\n' >&2
    exit 1
fi

scratch="$(mktemp -d)"
trap 'rm -rf "${scratch}"' EXIT

extract() {
    cd "${REPO_ROOT}"
    local sources patterns
    sources="$(find src/lib src/exe -type f \( -name '*.cpp' -o -name '*.hpp' \) | LC_ALL=C sort)"
    # A pattern file is named after its kind; the `.conf` beside it and the README are not read.
    patterns="$(find packaging/patterns -type f \( -name '*.common-error' -o -name '*.capitalization' \
        -o -name '*.hearing-impaired' -o -name '*.line-break' \) | LC_ALL=C sort)"

    # The patterns first, then the code joined to them: the last extraction writes the header,
    # and the code's is the one that names the package.
    # shellcheck disable=SC2086  # one word per file: no path of the repository has a space
    xgettext --language=Desktop --from-code=UTF-8 --force-po --sort-by-file \
        --add-location=file -k -kName -kDescription \
        --output="${scratch}/subedit.pot" ${patterns}

    # `-k` alone drops the default keywords (`_`, `gettext`…): only the project's count.
    # shellcheck disable=SC2086  # as above
    xgettext --language=C++ --from-code=UTF-8 --join-existing --sort-by-file \
        --add-location=file --add-comments=TRANSLATORS \
        --package-name=subedit --copyright-holder='the subedit authors' \
        --msgid-bugs-address='https://github.com/Guyot-Bertrand/sub-edit/issues' \
        -k -ktranslate:1 -ktranslateIn:1c,2 -ktranslatePlural:1,2 \
        --output="${scratch}/subedit.pot" ${sources}

    sed 's/^"POT-Creation-Date: .*\\n"$/"POT-Creation-Date: YEAR-MO-DA HO:MI+ZONE\\n"/' \
        "${scratch}/subedit.pot"
}

if [[ "${mode}" == "write" ]]; then
    extract >"${POT}.new"
    mv "${POT}.new" "${POT}"
    printf '%s✓%s %s\n' "${GREEN}" "${RESET}" "src/po/subedit.pot"
    exit 0
fi

if diff -u "${POT}" <(extract) >&2; then
    printf '%s✓%s the template src/po/subedit.pot is what the sources produce\n' "${GREEN}" "${RESET}"
else
    printf '%s✗%s src/po/subedit.pot is stale: "make pot" rewrites it\n' "${RED}" "${RESET}" >&2
    exit 1
fi
