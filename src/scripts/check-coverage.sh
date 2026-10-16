#!/usr/bin/env bash
# Compares the number of uncovered lines with the ratchet, and records it.
#
# The ratchet is on a LINE COUNT, not a percentage: a percentage rises as soon
# as well-tested code is added, which tightens the ratchet without anyone having
# decided it. See docs/adr/0015-memoire-des-mesures.md.
#
# The `line_percent` rate of the gcovr summary comes ALREADY ROUNDED to one
# decimal; it is never used for comparing. Only `line_total` and `line_covered`
# are authoritative.
#
# python3 reads the JSON: it is a dependency we already have, gcovr itself is
# written in it.

set -euo pipefail

readonly REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
readonly RATCHET="${REPO_ROOT}/docs/mesures/couverture.md"

readonly RED=$'\033[31m'
readonly GREEN=$'\033[32m'
readonly RESET=$'\033[0m'

summary=""
record=0

# Removes the temporary file of the ratchet's atomic write, however the script
# ends — success, failure, interruption. tmp_ratchet stays empty until
# write_ratchet has created something; once the rename is done, the file no
# longer exists and the rm -f below does nothing. Its counterpart in
# record-bench.sh relies on a Python try/except; here an EXIT trap plays the
# same role with what bash has.
#
# The function always ends with `return 0`: an EXIT trap whose last command
# fails (here the `[[ ]]` test when tmp_ratchet is empty) silently overwrites
# the script's exit code with its own — a ✓ on screen would then return
# exit 1, unrelated to what the script has just done.
tmp_ratchet=""
cleanup_tmp_ratchet() {
    [[ -n "${tmp_ratchet}" && -e "${tmp_ratchet}" ]] && rm -f "${tmp_ratchet}"
    return 0
}
trap cleanup_tmp_ratchet EXIT

usage() {
    cat >&2 <<'USAGE'
usage: check-coverage.sh --summary <summary.json> [--record]

  --summary  the JSON summary produced by gcovr --json-summary
  --record   rewrites docs/mesures/couverture.md from that summary
USAGE
    exit 2
}

while (( $# > 0 )); do
    case "$1" in
        --summary) [[ $# -ge 2 ]] || usage; summary="$2"; shift 2 ;;
        --record)  record=1; shift ;;
        -h|--help) usage ;;
        *) printf 'unknown argument: %s\n' "$1" >&2; usage ;;
    esac
done

[[ -n "${summary}" ]] || usage
[[ -f "${summary}" ]] || {
    printf 'coverage summary not found: %s\n' "${summary}" >&2
    printf '  produce it with: make coverage\n' >&2
    exit 1
}
[[ -f "${RATCHET}" ]] || {
    printf 'ratchet not found: %s\n' "${RATCHET}" >&2
    exit 1
}

# --record reads build/coverage-report/summary.json without ever regenerating
# it (see the "ratchet" recipe of the Makefile): it is an artefact this script
# does not build and whose freshness it does not own. A summary older than the
# most recent source of src/lib/ is the trace of a "make ratchet" run without a
# "make coverage" first, or of a leftover from another tool (verify-gates.sh
# injected one until it cleaned up after itself) — recording it would freeze a
# measurement that no longer matches the code.
if (( record == 1 )); then
    stale_source="$(find "${REPO_ROOT}/src/lib" -type f -newer "${summary}" -print -quit)"
    if [[ -n "${stale_source}" ]]; then
        printf '%s✗%s stale coverage summary: %s is newer than %s\n' \
            "${RED}" "${RESET}" "${stale_source#"${REPO_ROOT}"/}" "${summary}" >&2
        printf '  regenerate it with: make coverage\n' >&2
        exit 1
    fi
fi

# The ratchet has two textual anchors: the "## Record" heading (the boundary
# between the preamble and the rewritten section) and the "| count | `file` |"
# rows of its table. A missing or duplicated anchor — a heading renamed by
# hand, a badly resolved merge — crashes neither the sed that looks for it nor
# the python that compares: the first cuts nothing, the second carries on with
# a truncated table. Checking it once here, before any read or write, is better
# than a ✓ on a file that is already inconsistent.
heading_count="$(grep -c '^## Record$' "${RATCHET}" || true)"
if [[ "${heading_count}" -ne 1 ]]; then
    printf '%s✗%s the "## Record" heading of %s appears %s times, once expected\n' \
        "${RED}" "${RESET}" "${RATCHET#"${REPO_ROOT}"/}" "${heading_count}" >&2
    printf '  fix the file by hand before running again.\n' >&2
    exit 1
fi
if grep -qE '^(<{7} |={7}$|>{7} )' "${RATCHET}"; then
    printf '%s✗%s git conflict marker found in %s\n' \
        "${RED}" "${RESET}" "${RATCHET#"${REPO_ROOT}"/}" >&2
    printf '  resolve the merge by hand before running again.\n' >&2
    exit 1
fi

# Prints the total, then one "count<TAB>file" line per file concerned, from the
# largest count to the smallest. Fails (non-zero code) if the JSON is
# unreadable, incomplete or degenerate — line_total at zero or files empty,
# which a wrong --filter or a build without .gcda would produce, and which is a
# legitimate state for no summary of this project.
read_summary() {
    python3 - "$1" <<'PY'
import json, sys

with open(sys.argv[1], encoding="utf-8") as handle:
    data = json.load(handle)

line_total = data["line_total"]
line_covered = data["line_covered"]
files = data.get("files", [])

if line_total <= 0 or not files:
    sys.exit(1)

print(line_total - line_covered)
rows = [
    (entry["line_total"] - entry["line_covered"], entry["filename"])
    for entry in files
    if entry["line_total"] > entry["line_covered"]
]
for count, name in sorted(rows, key=lambda row: (-row[0], row[1])):
    print(f"{count}\t{name}")
PY
}

# Process substitution (mapfile < <(...)) is not covered by pipefail: a failing
# python3 would be invisible there, and the gate would announce a success on a
# truncated read. Command substitution is — measured_raw="$(...)" faithfully
# passes on the exit code of read_summary, caught here by the explicit test
# rather than by set -e, so as to answer with a usable ✗ instead of a Python
# traceback.
if ! measured_raw="$(read_summary "${summary}" 2>/dev/null)"; then
    printf '%s✗%s coverage summary unreadable or empty: %s\n' \
        "${RED}" "${RESET}" "${summary}" >&2
    printf '  regenerate it with: make coverage\n' >&2
    exit 1
fi
mapfile -t measured <<< "${measured_raw}"
readonly measured_total="${measured[0]}"

# gcovr 7.0 emits integers for line_total and line_covered, but the JSON
# summary does not guarantee it: a future version emitting them as floats would
# make `(( ))` below fail with an arithmetic syntax error — an error that
# `if (( ... ))` swallows silently (false test, script not interrupted), to the
# point of falling into the final green branch with a measured total such as
# "53.0" printed as is. A line count is legitimate only as an integer; checking
# it here turns a silent failure into a ✗.
[[ "${measured_total}" =~ ^[0-9]+$ ]] || {
    printf '%s✗%s coverage total is not an integer: %s\n' \
        "${RED}" "${RESET}" "${measured_total}" >&2
    printf '  suspicious summary: %s\n' "${summary}" >&2
    exit 1
}

# The ratchet's anchor: a "total: N" line in a code block.
recorded_total="$(sed -n 's/^[[:space:]]\+total[[:space:]]*:[[:space:]]*\([0-9]\+\)[[:space:]]*$/\1/p' \
    "${RATCHET}" | head -1)"

[[ -n "${recorded_total}" ]] || {
    printf '%s✗%s no readable "total: N" line in %s\n' \
        "${RED}" "${RESET}" "${RATCHET#"${REPO_ROOT}"/}" >&2
    exit 1
}

# The pattern above captures digits only, so recorded_total is necessarily an
# integer if it matched — this test documents the invariant and guards,
# symmetrically, against a future loosening of the pattern.
[[ "${recorded_total}" =~ ^[0-9]+$ ]] || {
    printf '%s✗%s recorded total is not an integer: %s in %s\n' \
        "${RED}" "${RESET}" "${recorded_total}" "${RATCHET#"${REPO_ROOT}"/}" >&2
    exit 1
}

# The data rows of the current table ("| count | `file` |"), leaving out the
# header and the separator row, which do not match this pattern.
mapfile -t recorded_rows < <(sed -n 's/^| *\([0-9][0-9]*\) *| `\([^`]*\)` *|$/\1\t\2/p' "${RATCHET}")

write_ratchet() {
    local -a new_rows=()
    local index
    for (( index = 1; index < ${#measured[@]}; index++ )); do
        new_rows+=("${measured[index]}")
    done

    # Idempotence: if neither the total nor the breakdown has moved, rewrite
    # nothing. Without this test, write_ratchet set today's date on every call,
    # even on an unchanged measurement — a tracked file that moves with nothing
    # to say.
    local unchanged=1
    if [[ "${measured_total}" != "${recorded_total}" ]] \
        || [[ "${#new_rows[@]}" -ne "${#recorded_rows[@]}" ]]; then
        unchanged=0
    else
        for (( index = 0; index < ${#new_rows[@]}; index++ )); do
            if [[ "${new_rows[index]}" != "${recorded_rows[index]}" ]]; then
                unchanged=0
                break
            fi
        done
    fi

    if (( unchanged == 1 )); then
        printf '%s✓%s ratchet already up to date: nothing to record again\n' \
            "${GREEN}" "${RESET}"
        return 0
    fi

    local version
    version="$(sed -n 's/^[[:space:]]*VERSION[[:space:]]\+\([0-9][0-9.]*\)[[:space:]]*$/\1/p' \
        "${REPO_ROOT}/CMakeLists.txt" | head -1)"
    [[ -n "${version}" ]] || {
        printf '%s✗%s unreadable version in %s/CMakeLists.txt\n' \
            "${RED}" "${RESET}" "${REPO_ROOT}" >&2
        exit 1
    }

    local today
    today="$(date +%Y-%m-%d)"

    # Atomic write: a temporary file next to the target, renamed afterwards —
    # the same principle as record-bench.sh. mktemp creates it as 0600; its
    # mode is aligned on the original file's before publishing it, otherwise
    # the rename would replace a tracked 644 file with one the group and others
    # cannot read. tmp_ratchet is cleaned up by the EXIT trap set at the top of
    # the script if the write is interrupted.
    local original_mode
    original_mode="$(stat -c '%a' "${RATCHET}")"
    tmp_ratchet="$(mktemp "${RATCHET}.XXXXXX")"

    {
        # The preamble is kept as is: only the "Record" section is rewritten.
        # The cut is at the section heading so as not to lose the prose.
        sed '/^## Record$/,$d' "${RATCHET}"
        printf '## Record\n\n    total: %s\n\n' "${measured_total}"
        printf 'Recorded on version %s, on %s.\n\n' "${version}" "${today}"
        printf '| Lines | File |\n| ----: | :--- |\n'
        local row
        for row in "${new_rows[@]}"; do
            printf '| %s | `%s` |\n' "${row%%$'\t'*}" "${row#*$'\t'}"
        done
    } > "${tmp_ratchet}"
    chmod "${original_mode}" "${tmp_ratchet}"
    mv "${tmp_ratchet}" "${RATCHET}"
    tmp_ratchet=""
    printf '%s✓%s ratchet recorded: %s uncovered line(s)\n' \
        "${GREEN}" "${RESET}" "${measured_total}"
}

if (( record == 1 )); then
    write_ratchet
    exit 0
fi

if (( measured_total > recorded_total )); then
    printf '%s✗%s coverage went down: %s uncovered line(s), against %s recorded\n' \
        "${RED}" "${RESET}" "${measured_total}" "${recorded_total}" >&2
    printf '\n  current breakdown:\n' >&2
    for (( index = 1; index < ${#measured[@]}; index++ )); do
        printf '    %4s  %s\n' \
            "${measured[index]%%$'\t'*}" "${measured[index]#*$'\t'}" >&2
    done
    printf '\n  compare with the table of %s to see what moved.\n' \
        "${RATCHET#"${REPO_ROOT}"/}" >&2
    printf '  cover the added lines, or justify raising the ratchet.\n' >&2
    exit 1
fi

if (( measured_total < recorded_total )); then
    printf '%s✓%s coverage improves: %s uncovered line(s), against %s recorded\n' \
        "${GREEN}" "${RESET}" "${measured_total}" "${recorded_total}"
    printf '  "make ratchet" records this progress.\n'
    exit 0
fi

printf '%s✓%s %s uncovered line(s), as recorded\n' \
    "${GREEN}" "${RESET}" "${measured_total}"
