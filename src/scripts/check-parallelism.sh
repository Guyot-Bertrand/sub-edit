#!/usr/bin/env bash
# Checks that no parallelism escapes $(JOBS).
#
# The Makefile caps local parallelism at two cores by default — see the comment
# above JOBS in the Makefile. That discipline holds only as long as every site
# of parallelism respects it: a recipe that hard-codes `-j 8`, a script that
# runs `xargs -P 4` or relies on `nproc`, a CMake module that writes
# `-flto=8`, all silently get around the cap. Nothing stopped them before this
# script; that is what it fixes.
#
# Three families of files:
#
#   - Makefile         : the recipes, that is the lines that begin with a tab.
#   - src/scripts/     : every file, at any depth, picked by its `.sh`
#                        extension or by its shebang — the git hooks
#                        (`hooks/pre-commit`, `hooks/commit-msg`) have neither
#                        at the top level, and are real bash scripts.
#   - cmake/*.cmake,
#     CMakeLists.txt   : the top-level one and the eight under `src/`, which
#                        declare the test and benchmark targets — precisely
#                        where hard-coded parallelism would be most likely to
#                        show up.
#
# The recognised forms are described above `token_violation`, in a single
# place that the Makefile and the scripts both depend on: a form learnt once
# is learnt everywhere. That is what was missing before — a bare `-j` failed
# in the Makefile and passed in a script.
#
# Comments and documentation examples must not trigger the check: the
# Makefile's comment shows `make build JOBS=8`, the one in cmake/Lto.cmake
# shows `-DSUBEDIT_LTO_JOBS=8`. A check that cries wolf ends up disabled, and
# then protects nothing — hence the care taken to strip trailing comments
# *before* scanning the tokens, not after.

set -euo pipefail

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"

# --root points the check at a temporary copy, to put it to the test without
# touching the real working tree.
while [[ $# -gt 0 ]]; do
    case "$1" in
        --root)
            REPO_ROOT="$2"
            shift 2
            ;;
        *)
            printf 'unknown argument: %s\n' "$1" >&2
            exit 1
            ;;
    esac
done
readonly REPO_ROOT

# Two files escape the script scan, each for a reason of its own:
#
#   - check-parallelism.sh spells out `-j`, `-P`, `xargs` and `nproc`, as
#     patterns to recognise and not as commands to run. Checking it against
#     its own patterns would prove nothing but that it contains itself.
#   - verify-gates.sh carries the injected defects that prove the gates close,
#     among them a Makefile recipe with `-j 8`. Those are test data, not
#     executed code. The file used to pass only because the closing quote was
#     glued to the digit: a one-character rewording would have made
#     `make parallelism` fail on the project's own test data. A written
#     exclusion is better than a syntax that holds by accident.
#
# The price of the second, worth knowing: real parallelism that showed up one
# day in verify-gates.sh would no longer be seen. The file runs nothing but
# make targets, which makes the risk theoretical.
readonly EXCLUDED_SCRIPTS=(
    check-parallelism.sh
    verify-gates.sh
)

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

# Strips a trailing comment, respecting quotes: a `#` inside a single- or
# double-quoted string ('run #1', "run #1") does not begin a comment; only one
# that appears outside quotes and at the start of a word (start of line, or
# preceded by a space or a shell metacharacter such as `&`, `;`, `|`, `(`)
# does — `task &#glued` is a valid comment without a space, `echo "run #1" &`
# is not one. A character-by-character scan, not a pattern: a pattern based on
# " #" would be caught out by exactly the first case.
#
# Quotes escaped by `\` inside a double-quoted string are followed. Without
# that, `printf "a \" # b" &` took the `\"` for a closing quote, took the `#`
# for a comment, and let the final `&` escape the check.
#
# The limit that remains, written here so that this comment promises no more
# than the code delivers: escaping is followed one level deep only, and
# substitutions (`$(...)`, `` ` ``) are not parsed. Going further would mean
# writing a shell parser, out of proportion with what this check protects.
strip_trailing_comment() {
    local line="$1"
    local -i i len=${#line}
    local c prev="" in_single=0 in_double=0
    local result=""

    for ((i = 0; i < len; i++)); do
        c="${line:i:1}"

        if (( in_single )); then
            [[ "${c}" == "'" ]] && in_single=0
        elif (( in_double )); then
            # In a double-quoted string, `\` protects the next character: both
            # are copied as a block, without the second being interpreted.
            if [[ "${c}" == '\' ]] && (( i + 1 < len )); then
                result+="${c}${line:i+1:1}"
                prev="${line:i+1:1}"
                i+=1
                continue
            fi
            [[ "${c}" == '"' ]] && in_double=0
        elif [[ "${c}" == "'" ]]; then
            in_single=1
        elif [[ "${c}" == '"' ]]; then
            in_double=1
        elif [[ "${c}" == '#' ]]; then
            case "${prev}" in
                ''|[[:space:]]|'&'|';'|'|'|'(')
                    break
                    ;;
            esac
        fi

        result+="${c}"
        prev="${c}"
    done

    printf '%s' "${result}"
}

# A parallelism value is legitimate when it defers to $(JOBS) rather than
# deciding for itself.
is_jobs_value() {
    case "$1" in
        '$(JOBS)'|'${JOBS}'|'"${JOBS}"'|'$JOBS'|'"$JOBS"') return 0 ;;
        *) return 1 ;;
    esac
}

# Does the line invoke a tool that parallelises? Tells a build `-j` apart from
# a `-j` that happens to be another program's flag — `jq -j`, for instance.
# Without that distinction, reporting every bare `-j` would be precisely the
# false positive this check must avoid.
invokes_parallel_tool() {
    [[ "$1" =~ (^|[^[:alnum:]_-])(make|cmake|ninja|xargs)([^[:alnum:]_-]|$) ]]
}

# Prints the reason for the bypass, or nothing. The recognised forms:
#
#   -j 8, -P 4, -j8, -P4          a fixed number of processes
#   -j, -P without a number       worse still: as many processes as cores, on
#                                 a line that builds
#   --parallel 8, --parallel=8    the long form of cmake --build
#   MAKEFLAGS=-j8                 parallelism passed through the environment
#
# The line received must already be stripped of its trailing comment: that is
# the caller's job, because only the caller knows whether the file is shell.
token_violation() {
    local line="$1"

    local -a tokens
    read -ra tokens <<< "${line}"

    local i token next
    for i in "${!tokens[@]}"; do
        token="${tokens[${i}]}"
        next="${tokens[$((i + 1))]:-}"

        case "${token}" in
            -j|-P)
                is_jobs_value "${next}" && continue
                if [[ "${next}" =~ ^[0-9]+$ ]]; then
                    printf 'hard-coded parallelism (%s %s)' "${token}" "${next}"
                    return 0
                fi
                if invokes_parallel_tool "${line}"; then
                    if [[ -z "${next}" ]]; then
                        printf 'unbounded parallelism (%s without a value)' "${token}"
                    else
                        printf 'parallelism outside $(JOBS) (%s %s)' "${token}" "${next}"
                    fi
                    return 0
                fi
                ;;
            -j[0-9]*|-P[0-9]*)
                printf 'hard-coded parallelism (%s)' "${token}"
                return 0
                ;;
            --parallel)
                is_jobs_value "${next}" && continue
                printf 'hard-coded parallelism (--parallel %s)' "${next:-without a value}"
                return 0
                ;;
            --parallel=*)
                is_jobs_value "${token#--parallel=}" && continue
                printf 'hard-coded parallelism (%s)' "${token}"
                return 0
                ;;
            *MAKEFLAGS=*)
                # The pattern is broad on purpose: in a Makefile recipe, the
                # `@` that silences the line is glued to the token —
                # `@MAKEFLAGS=-j8`.
                #
                # The `-j` must start a word: `--jobserver-auth` contains one
                # as text, not as an option.
                if [[ "${token#*MAKEFLAGS=}" =~ (^|[^[:alnum:]-])-j ]]; then
                    printf 'parallelism passed through MAKEFLAGS (%s)' "${token}"
                    return 0
                fi
                ;;
        esac
    done

    return 0
}

# Invariant 1 — every Makefile recipe that parallelises goes through $(JOBS).
#
# Only recipe lines are read, that is the ones that begin with a tab. The rest
# of the file — including the comments that show `make build JOBS=8` — is not
# code.
check_makefile() {
    local makefile="${REPO_ROOT}/Makefile"
    [[ -f "${makefile}" ]] || { report_success "no Makefile, nothing to check"; return 0; }

    local -a offenders=()
    local line reason
    while IFS= read -r line; do
        [[ "${line}" == $'\t'* ]] || continue

        reason="$(token_violation "${line}")"
        if [[ -n "${reason}" ]]; then offenders+=("${reason}: ${line#$'\t'}"); fi
    done < "${makefile}"

    if (( ${#offenders[@]} > 0 )); then
        report_failure "Makefile: parallelism that bypasses \$(JOBS):
$(printf '    %s\n' "${offenders[@]}")"
    else
        report_success "Makefile: all parallelism goes through \$(JOBS)"
    fi
}

# The files under src/scripts/ that are shell scripts: those that carry the
# extension, and those that carry a shebang. The git hooks have only the
# latter — they are installed under the name git imposes, with no extension.
collect_scripts() {
    local file base first excluded
    while IFS= read -r -d '' file; do
        base="$(basename "${file}")"

        excluded=0
        local candidate
        for candidate in "${EXCLUDED_SCRIPTS[@]}"; do
            if [[ "${base}" == "${candidate}" ]]; then
                excluded=1
                break
            fi
        done
        (( excluded == 0 )) || continue

        if [[ "${file}" == *.sh ]]; then
            printf '%s\0' "${file}"
            continue
        fi

        first=""
        IFS= read -r first < "${file}" 2>/dev/null || true
        if [[ "${first}" =~ ^#!.*(bash|sh)([[:space:]]|$) ]]; then
            printf '%s\0' "${file}"
        fi
    done < <(find "${REPO_ROOT}/src/scripts" -type f -print0 2>/dev/null | sort -z || true)
}

# Invariant 2 — the scripts introduce no parallelism of their own.
#
# One ✓ per invariant, not one ✓ per scanned file: the list of offenders is
# printed only on failure, otherwise every script added to the project would
# add a permanent line to an output meant to stay readable.
check_scripts() {
    local -a offenders=()
    local script relative line trimmed without_comment reason

    while IFS= read -r -d '' script; do
        relative="${script#"${REPO_ROOT}"/}"

        while IFS= read -r line; do
            trimmed="${line#"${line%%[![:space:]]*}"}"
            [[ -z "${trimmed}" || "${trimmed}" == '#'* ]] && continue

            # The trailing comment goes first: otherwise
            # `cmake --build . # example: -j 4` would be reported as a
            # bypass, although the command contains none.
            without_comment="$(strip_trailing_comment "${line}")"

            reason="$(token_violation "${without_comment}")"

            if [[ -z "${reason}" ]] \
                && [[ "${without_comment}" =~ (^|[^[:alnum:]_])nproc([^[:alnum:]_]|$) ]]; then
                reason="call to nproc"
            fi

            if [[ -z "${reason}" ]]; then
                # `task & # comment` is still a task sent to the background:
                # it is the end of the command that must be tested, not the
                # end of the comment.
                local trailing="${without_comment%"${without_comment##*[![:space:]]}"}"
                if [[ "${trailing}" == *'&' && "${trailing}" != *'&&' ]]; then
                    reason="command sent to the background (&)"
                fi
            fi

            [[ -n "${reason}" ]] && offenders+=("${relative}: ${reason}: ${trimmed}")
        done < "${script}"
    done < <(collect_scripts)

    if (( ${#offenders[@]} > 0 )); then
        report_failure "src/scripts: script-specific parallelism:
$(printf '    %s\n' "${offenders[@]}")"
    else
        report_success "src/scripts: no script introduces its own parallelism"
    fi
}

# The project's CMake files: the modules, the top-level CMakeLists, and those
# under src/ — the latter declare the test and benchmark targets.
# The `|| true` are not superstition: under `pipefail`, a
# `find missing-directory | sort` makes the pipeline fail, which under
# `set -e` stops the collection at its first line — the following scans are
# then never reached, and the check returns a green verdict having read
# nothing. The defect did not show on the repository, where all these
# directories exist; it appeared under `--root`, that is precisely when one
# tries to put the check to the test.
collect_cmake() {
    find "${REPO_ROOT}/cmake" -maxdepth 1 -name '*.cmake' -print0 2>/dev/null | sort -z || true
    if [[ -f "${REPO_ROOT}/CMakeLists.txt" ]]; then
        printf '%s\0' "${REPO_ROOT}/CMakeLists.txt"
    fi
    find "${REPO_ROOT}/src" -name 'CMakeLists.txt' -type f -print0 2>/dev/null | sort -z || true
}

# Invariant 3 — LTO hard-codes no number of processes outside
# SUBEDIT_LTO_JOBS, and no CMake file parallelises on its own.
check_cmake() {
    local -a offenders=()
    local file relative line trimmed reason

    while IFS= read -r -d '' file; do
        relative="${file#"${REPO_ROOT}"/}"

        while IFS= read -r line; do
            trimmed="${line#"${line%%[![:space:]]*}"}"
            [[ -z "${trimmed}" || "${trimmed}" == '#'* ]] && continue

            if [[ "${line}" == *'-flto='* && "${line}" != *'-flto=${SUBEDIT_LTO_JOBS}'* ]]; then
                offenders+=("${relative}: hard-coded -flto=: ${trimmed}")
                continue
            fi

            reason="$(token_violation "${line}")"
            [[ -n "${reason}" ]] && offenders+=("${relative}: ${reason}: ${trimmed}")
        done < "${file}"
    done < <(collect_cmake)

    if (( ${#offenders[@]} > 0 )); then
        report_failure "cmake: hard-coded parallelism:
$(printf '    %s\n' "${offenders[@]}")"
    else
        # Established: none of these files hard-codes -flto= outside
        # SUBEDIT_LTO_JOBS, nor parallelises otherwise. That says nothing of
        # the ones that never mention the subject — which is why the sentence
        # holds true for them too.
        report_success "cmake: no hard-coded parallelism"
    fi
}

# Invariant 4 — every run of the tests, and of the bench, goes through `limit-cores.sh`, issue #630.
#
# **`JOBS` bounds processes, not threads**, and libmpv as well as ffmpeg create threads of their
# own accord: a player test case of three tenths of a second kept more than five cores busy. The
# three previous invariants read what is written; this one reads what a process does by itself,
# and the only bound its threads inherit is the affinity that `limit-cores.sh` sets on the
# command it runs.
#
# The check is a form: a line that runs `ctest` — Makefile recipe or script — without naming
# `limit-cores.sh`. **The bench binary is in the same case**, noted in the review of phase 14:
# `subedit_bench` opens players, hence libmpv threads, and `gate/bench.sh` ran it bare. It is
# recognised by its path, `bin/subedit_bench`: a `--target subedit_bench` builds it, it does not
# run it. The check says nothing about a binary run by hand, and does not claim to.
invokes_ctest() {
    [[ "$1" =~ (^|[^[:alnum:]_.-])ctest([^[:alnum:]_.-]|$) ]] ||
        [[ "$1" =~ bin/subedit_bench([^[:alnum:]_.-]|$) ]]
}

check_tests() {
    local -a offenders=()
    local line trimmed without_comment script relative

    local makefile="${REPO_ROOT}/Makefile"
    if [[ -f "${makefile}" ]]; then
        while IFS= read -r line; do
            [[ "${line}" == $'\t'* ]] || continue
            without_comment="$(strip_trailing_comment "${line}")"
            if invokes_ctest "${without_comment}" && [[ "${without_comment}" != *limit-cores.sh* ]]; then
                offenders+=("Makefile: tests outside limit-cores.sh: ${line#$'\t'}")
            fi
        done < "${makefile}"
    fi

    while IFS= read -r -d '' script; do
        relative="${script#"${REPO_ROOT}"/}"
        while IFS= read -r line; do
            trimmed="${line#"${line%%[![:space:]]*}"}"
            [[ -z "${trimmed}" || "${trimmed}" == '#'* ]] && continue
            without_comment="$(strip_trailing_comment "${line}")"
            if invokes_ctest "${without_comment}" && [[ "${without_comment}" != *limit-cores.sh* ]]; then
                offenders+=("${relative}: tests outside limit-cores.sh: ${trimmed}")
            fi
        done < "${script}"
    done < <(collect_scripts)

    if (( ${#offenders[@]} > 0 )); then
        report_failure "tests run without bounding their threads:
$(printf '    %s\n' "${offenders[@]}")"
    else
        report_success "tests: every run goes through limit-cores.sh"
    fi
}

check_makefile
check_scripts
check_cmake
check_tests

if (( failures > 0 )); then
    printf '\n%s%d bypass(es) of $(JOBS)%s\n' "${RED}" "${failures}" "${RESET}" >&2
    exit 1
fi
