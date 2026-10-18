#!/usr/bin/env bash
# Rewrites the manual's invocation examples with what the commands produce.
#
# The manual must carry one real invocation example per command. Copied by
# hand, such an example is stale documentation on probation: the only version
# number it contained was corrected twelve times during phase 2, and nothing
# checked that it was true.
#
# The project already has the idiom: CHANGELOG.md is generated from the history,
# then committed. An invocation example is of the same nature — it is no longer
# written, the command that illustrates it is declared.
#
#     <!-- example: subedit-cli --help file.srt; echo $? -->
#     ```console
#     $ subedit-cli --help file.srt; echo $?
#     subedit 0.2.4
#     0
#     ```
#
# The marker carries a whole shell line, run by `sh -c`. That is what lets the
# examples chain a command and the reading of its exit status, as a user would.
#
# Two modes:
#
#   (no argument)  rewrites the blocks in the files
#   --check        compares without writing, and names the stale blocks
#
# `--check` compares the commands' output with the file's content, without
# consulting git. A `git diff` after regeneration would be wrong in a common
# case: a freshly written, uncommitted manual section is not stale, but the diff
# would see it and refuse.

set -euo pipefail

# The examples are those of the English manual: every command they run speaks English,
# whatever the machine's locale — `LANGUAGE=en` names the source language first (spec 15,
# D4), and changes nothing else of the locale the examples run in.
export LANGUAGE=en

readonly REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
readonly MANUAL_DIR="${REPO_ROOT}/docs/manual"
# The dev preset, not release: a program's output does not depend on its
# optimisation level, and the quality gate already builds that one.
readonly BIN_DIR="${REPO_ROOT}/build/dev/bin"
readonly FENCE_OPEN='```console'
readonly FENCE_CLOSE='```'

# shellcheck source=src/scripts/lib/output.sh
source "$(dirname "${BASH_SOURCE[0]}")/lib/output.sh"

mode="write"
case "${1:-}" in
    "") ;;
    --check) mode="check" ;;
    *)
        printf 'usage: %s [--check]\n' "$(basename "$0")" >&2
        exit 2
        ;;
esac
readonly mode

rewritten=0
stale=0

# The file being rewritten, so that stopping on an error leaves no residue
# behind.
output_file=""
trap 'rm -f "${output_file}"' EXIT

# A stop always names the file and the line: a manual has several blocks, and
# "the command failed" without saying which helps nobody.
die() {
    local location="$1"
    shift
    printf '%s✗ %s%s\n' "${RED}" "${location}" "${RESET}" >&2
    local line
    for line in "$@"; do
        printf '    %s\n' "${line}" >&2
    done
    exit 1
}

# Fills example_output and example_status rather than writing to the output:
# the caller needs both, and a command substitution returns only one.
example_output=""
example_status=0

run_example() {
    local command="$1"

    # Temporary directory: an example's "file.srt" must never designate a file
    # of the repository, nor leave a trace there.
    local workdir
    workdir="$(mktemp -d)"

    example_status=0
    # Both outputs are merged, in the order the user would see them. The input
    # is closed: an example waiting for input would block the generation
    # instead of failing.
    example_output="$(
        cd "${workdir}" \
            && PATH="${BIN_DIR}:${PATH}" sh -c "${command}" 2>&1 </dev/null
    )" || example_status=$?

    rm -rf "${workdir}"
}

# The block as it should be: the prompt, the command, then the output.
render_block() {
    local command="$1"
    local output="$2"
    printf '$ %s\n%s\n' "${command}" "${output}"
}

process_file() {
    local file="$1"
    local relative="${file#"${REPO_ROOT}/"}"

    output_file="$(mktemp)"

    local lineno=0
    local line=""

    while IFS= read -r line <&3; do
        lineno=$((lineno + 1))

        if [[ "${line}" != *'<!-- example:'* ]]; then
            printf '%s\n' "${line}" >>"${output_file}"
            continue
        fi

        local marker_line=${lineno}
        local location="${relative}:${marker_line}"

        [[ "${line}" =~ ^[[:space:]]*'<!-- example:'[[:space:]]*(.*[^[:space:]])[[:space:]]*'-->'[[:space:]]*$ ]] \
            || die "${location}" \
                "unreadable marker: ${line}" \
                "expected form: <!-- example: <shell line> -->"
        local command="${BASH_REMATCH[1]}"

        local fence=""
        IFS= read -r fence <&3 || die "${location}" \
            "the marker is the last line of the file" \
            "a marker announces the block that follows it; one is needed."
        lineno=$((lineno + 1))

        [[ "${fence}" == "${FENCE_OPEN}" ]] || die "${location}" \
            "the marker is not followed by a ${FENCE_OPEN} block" \
            "found instead: ${fence}"

        # The existing block is read to be compared, never to be kept.
        local previous=""
        local closed=0
        local block_line=""
        while IFS= read -r block_line <&3; do
            lineno=$((lineno + 1))
            if [[ "${block_line}" == "${FENCE_CLOSE}" ]]; then
                closed=1
                break
            fi
            previous+="${block_line}"$'\n'
        done
        (( closed == 1 )) || die "${location}" \
            "the block opened after the marker is never closed"

        run_example "${command}"

        if (( example_status == 127 )); then
            die "${location}" \
                "command not found: ${command}" \
                "the examples run with ${BIN_DIR#"${REPO_ROOT}/"} at the head of the PATH;" \
                "build first, with \"make build\"."
        fi

        if (( example_status != 0 )); then
            die "${location}" \
                "the command returned status ${example_status}: ${command}" \
                "an example of the manual illustrates an invocation that works." \
                "output:" \
                "${example_output}"
        fi

        # An empty block illustrates nothing, and the worst outcome would be a
        # manual silently emptied of its examples. A silent command is therefore
        # an error, not an empty block written without a word.
        [[ -n "${example_output}" ]] || die "${location}" \
            "the command produced nothing: ${command}" \
            "an empty block illustrates nothing; the manual will not be emptied silently."

        local expected
        expected="$(render_block "${command}" "${example_output}")"

        {
            printf '%s\n' "${line}"
            printf '%s\n' "${FENCE_OPEN}"
            printf '%s\n' "${expected}"
            printf '%s\n' "${FENCE_CLOSE}"
        } >>"${output_file}"

        if [[ "${previous}" == "${expected}"$'\n' ]]; then
            continue
        fi

        if [[ "${mode}" == "check" ]]; then
            stale=$((stale + 1))
            printf '  %s✗%s %s — the block no longer matches "%s"\n' \
                "${RED}" "${RESET}" "${location}" "${command}"
        else
            rewritten=$((rewritten + 1))
            printf '  %s✓%s %s — "%s"\n' \
                "${GREEN}" "${RESET}" "${location}" "${command}"
        fi
    done 3<"${file}"

    if [[ "${mode}" == "write" ]]; then
        cp "${output_file}" "${file}"
    fi
    rm -f "${output_file}"
    output_file=""
}

if [[ ! -d "${BIN_DIR}" ]]; then
    die "${BIN_DIR#"${REPO_ROOT}/"}" \
        "missing build directory" \
        "the examples need the binaries: run \"make build\"."
fi

if [[ "${mode}" == "check" ]]; then
    printf '%smanual examples — check%s\n' "${BOLD}" "${RESET}"
else
    printf '%smanual examples%s\n' "${BOLD}" "${RESET}"
fi

# The source manual and every translation of it, `docs/i18n/<lang>/manual/`, whatever the language
# (#671): a translated page carries the same examples, byte for byte (check-translation-structure.py),
# so they are replayed and rewritten with the source's, never left to drift.
while IFS= read -r manual_file; do
    process_file "${manual_file}"
done < <(
    shopt -s nullglob
    find "${MANUAL_DIR}" "${REPO_ROOT}"/docs/i18n/*/manual -name '*.md' -type f | sort
)

if [[ "${mode}" == "check" ]]; then
    if (( stale > 0 )); then
        printf '%s%d stale block(s) — run "make manual"%s\n' \
            "${RED}" "${stale}" "${RESET}" >&2
        exit 1
    fi
    printf '%sthe examples are up to date%s\n' "${GREEN}" "${RESET}"
else
    if (( rewritten > 0 )); then
        printf '%s%d block(s) rewritten%s\n' "${GREEN}" "${rewritten}" "${RESET}"
    else
        printf 'no block to rewrite\n'
    fi
fi
