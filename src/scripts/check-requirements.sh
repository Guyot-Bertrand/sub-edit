#!/usr/bin/env bash
# Compares the requirement registry with what surrounds it: the tests that cite
# it, and the phase specs that promise it.
#
# Three directions, all needed — see docs/adr/0014-registre-d-exigences.md:
#
#   coverage  — an `implemented` requirement is cited by at least one test, and
#               a requirement that is not implemented is cited by none;
#   reference — every tag shaped like an identifier designates a requirement
#               of the registry;
#   scoping   — the requirement table of a phase spec and the registry rows
#               carrying that phase say the same thing.
#
# The tags come from the binary itself, never from an analysis of the C++.
# Catch2 knows them exactly; a grep on TEST_CASE would guess them — it would
# miss a tag produced by a macro and invent those of a commented-out file.
#
# **The third direction was born of the review of phase 7, after two phases
# through which the same defect slipped.** Phase 16 had removed `CLI-SNAP-03`
# from the code without its spec following, and let `GUI-GRID-03` appear without
# writing it in; phase 7 merged `GUI-THEME-02` into `GUI-THEME-01` the same way.
# Twice in a row, the spec promised a number of requirements the registry no
# longer carried — and the only thing that saw it was a human reading the two
# columns side by side. What two reviews found, a check finds every time.

set -euo pipefail

readonly REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"

registry="${REPO_ROOT}/docs/exigences.md"

# **Several binaries, not one.** A command-line requirement is proven by
# running the binary and reading what it writes; an interface requirement is
# proven by building the window in the test's process and driving it. Both are
# "what the binary shows" in the registry's sense, but they cannot live in the
# same executable: the end-to-end harness links no library of what it tests,
# and that is what gives it its value.
#
# The check therefore gathers the tags of every binary it is given.
binaries=()

readonly RED=$'\033[31m'
readonly GREEN=$'\033[32m'
readonly RESET=$'\033[0m'

# Shape of an identifier: capitals, segments separated by a dash, a two-digit
# number last. It is what tells `CLI-VERSION-01` from an ordinary tag such as
# `e2e` or `framerate`, without keeping a list.
readonly ID_PATTERN='^[A-Z][A-Z0-9]*(-[A-Z0-9]+)*-[0-9][0-9]$'

failures=0

usage() {
    cat >&2 <<USAGE
usage: check-requirements.sh [--registry <file>] [--binary <executable>]

  --registry  registry to read     (default: docs/exigences.md)
  --binary    binary to query, repeatable
              (default: the end-to-end and interface harnesses)
USAGE
    exit 2
}

while (( $# > 0 )); do
    case "$1" in
        --registry) [[ $# -ge 2 ]] || usage; registry="$2"; shift 2 ;;
        --binary)   [[ $# -ge 2 ]] || usage; binaries+=("$2"); shift 2 ;;
        -h|--help)  usage ;;
        *)          printf 'unknown argument: %s\n' "$1" >&2; usage ;;
    esac
done

# The default can only be set here: a value set before parsing the arguments
# would be added to those the caller gives instead of giving way to them.
if (( ${#binaries[@]} == 0 )); then
    binaries=("${REPO_ROOT}/build/dev/bin/subedit_e2e_test"
              "${REPO_ROOT}/build/dev/bin/subedit_gui_test")
fi

report_failure() {
    printf '%s✗%s %s\n' "${RED}" "${RESET}" "$*" >&2
    failures=$((failures + 1))
}

report_success() {
    printf '%s✓%s %s\n' "${GREEN}" "${RESET}" "$*"
}

[[ -f "${registry}" ]] || { printf 'registry not found: %s\n' "${registry}" >&2; exit 1; }
for binary in "${binaries[@]}"; do
    [[ -x "${binary}" ]] || {
        printf 'test binary not found: %s\n' "${binary}" >&2
        printf '  build it with: make build\n' >&2
        exit 1
    }
done

# Reads the registry table and prints "identifier state phase struck", one per
# line. The four fields have no space, so a bare `read` reads them.
#
# States are compared with index() rather than a regular expression: the
# column may carry a reason after the state word, and a substring search
# compares bytes, the same way under gawk and mawk.
read_registry() {
    awk -F'|' -v pattern="${ID_PATTERN}" '
        NF < 4 { next }
        {
            raw = $2
            id = raw
            gsub(/[`~ \t]/, "", id)
            if (id !~ pattern) next

            phase = $(NF - 2)
            gsub(/[` \t]/, "", phase)

            state = $(NF - 1)
            if      (index(state, "implemented")) label = "implemented"
            else if (index(state, "planned"))     label = "planned"
            else if (index(state, "abandoned"))   label = "abandoned"
            else if (index(state, "replaced"))    label = "replaced"
            else                                  label = "unknown"

            print id, label, phase, (index(raw, "~~") ? "struck" : "plain")
        }
    ' "${registry}"
}

# Reads a phase spec and prints "identifier struck", one per line.
#
# **Every table row of the file, not only the requirements section.** Two
# reasons. An identifier appears in a first cell nowhere else — checked on the
# nine specs —, so anchoring on a heading brings nothing; and a renamed heading
# would silence the check instead of making it fail, which is the worse of the
# two behaviours.
read_spec_ids() {
    awk -F'|' -v pattern="${ID_PATTERN}" '
        /^\|/ {
            raw = $2
            id = raw
            gsub(/[`~ \t]/, "", id)
            if (id !~ pattern) next
            print id, (index(raw, "~~") ? "struck" : "plain")
        }
    ' "$1"
}

declare -A state_of=()
duplicates=()

declare -A phase_of=()
declare -A struck_in_registry=()

while read -r id state phase struck; do
    if [[ -n "${state_of[${id}]:-}" ]]; then
        duplicates+=("${id}")
    fi
    state_of["${id}"]="${state}"
    phase_of["${id}"]="${phase}"
    struck_in_registry["${id}"]="${struck}"
done < <(read_registry)

if (( ${#state_of[@]} == 0 )); then
    report_failure "no requirement read in ${registry#"${REPO_ROOT}"/}
    is the table well formed?"
    printf '\n%s1 mismatch between the registry, the specs and the tests%s\n' "${RED}" "${RESET}" >&2
    exit 1
fi

declare -A cited=()

# Queries the binary and captures its output and its exit code separately,
# before any processing by grep/tr/sort.
#
# A pipeline placed in a process substitution (`< <(cmd | grep …)`) is not seen
# by `set -e`: a failure of the binary — a crash, a --list-tags option gone, a
# Catch2 output that changes shape — would otherwise be swallowed silently, and
# `cited` would stay empty without a single line saying so. A checker that
# answers "all is well" when it could read nothing is worse than no checker.
#
# stdout and stderr are captured separately, the latter in a temporary file:
# merging them with `2>&1` would extract tags from the binary's diagnostics —
# an `[IDENTIFIER]` mentioned in an error message would then be taken for a
# real citation, in either direction. stderr is still read and shown on
# failure, so that the diagnosis of a crash stays visible.
list_tags_stderr="$(mktemp)"
trap 'rm -f "${list_tags_stderr}"' EXIT

list_tags_stdout=""
list_tags_status=0
for binary in "${binaries[@]}"; do
    one=""
    one="$("${binary}" --list-tags 2>"${list_tags_stderr}")" || {
        report_failure "the test binary failed on --list-tags:
$(sed 's/^/    /' "${list_tags_stderr}")
    check that ${binary} answers --list-tags, and that Catch2 has not changed its output."
        list_tags_status=1
        continue
    }
    list_tags_stdout+="${one}"$'\n'
done

if (( list_tags_status != 0 )); then
    :
else
    # Every bracketed group of the whole standard output is extracted, rather
    # than anchoring on "N spaces, bracket": Catch2 puts every spelling of a
    # tag on one line, and wraps the line at 70 columns when it is longer.
    # Neither the header nor the footer of the output contains brackets. The
    # "|| true" keeps a grep without a match — an output without a single tag —
    # from silently failing the process substitution: it is for the guard that
    # follows to say so, not for `set -e` to swallow it.
    while read -r tag; do
        [[ -n "${tag}" ]] && cited["${tag}"]=1
    done < <(printf '%s' "${list_tags_stdout}" | grep -o '\[[^][]*\]' | tr -d '[]' | sort -u || true)

    # Symmetric to the guard on ${#state_of[@]} above: zero tags is never
    # legitimate here, the test binary always carries at least [e2e].
    if (( ${#cited[@]} == 0 )); then
        report_failure "no tag read in the standard output of the queried binaries
    has the shape of Catch2's output changed? the test binary always carries
    at least the [e2e] tag; its absence signals a defect, never a normal
    state."
    fi
fi

# 1 — an identifier is never reused, hence never written twice.
if (( ${#duplicates[@]} > 0 )); then
    report_failure "identifiers present more than once in the registry:
$(printf '    %s\n' "${duplicates[@]}")
    an identifier is never reused."
else
    report_success "${#state_of[@]} identifiers, all distinct"
fi

# 2 — every state asserts something checkable about the presence of a test.
uncovered=()
unexpected=()
unknown_state=()

for id in "${!state_of[@]}"; do
    case "${state_of[${id}]}" in
        implemented)
            [[ -n "${cited[${id}]:-}" ]] || uncovered+=("${id}")
            ;;
        planned|abandoned|replaced)
            [[ -z "${cited[${id}]:-}" ]] || unexpected+=("${id} (${state_of[${id}]})")
            ;;
        *)
            unknown_state+=("${id}")
            ;;
    esac
done

if (( ${#unknown_state[@]} > 0 )); then
    report_failure "unrecognised states:
$(printf '    %s\n' "${unknown_state[@]}")
    the four states are: planned, implemented, abandoned, replaced."
fi

if (( ${#uncovered[@]} > 0 )); then
    report_failure "requirements declared implemented that no test cites:
$(printf '    %s\n' "${uncovered[@]}")
    add the tag to an end-to-end test, or correct the state."
else
    report_success "every implemented requirement is cited by at least one test"
fi

if (( ${#unexpected[@]} > 0 )); then
    report_failure "requirements not implemented that a test cites nevertheless:
$(printf '    %s\n' "${unexpected[@]}")
    a test that cites a requirement proves it is implemented;
    change the state, or remove the test left behind."
else
    report_success "no requirement that is not implemented is cited"
fi

# 3 — every tag shaped like an identifier designates one.
dangling=()
for tag in "${!cited[@]}"; do
    [[ "${tag}" =~ ${ID_PATTERN} ]] || continue
    [[ -n "${state_of[${tag}]:-}" ]] || dangling+=("${tag}")
done

if (( ${#dangling[@]} > 0 )); then
    report_failure "tags shaped like an identifier that designate no requirement:
$(printf '    %s\n' "${dangling[@]}")
    add the requirement to the registry, or correct the tag."
else
    report_success "every tag shaped like an identifier designates a requirement"
fi

# 4 — the requirement table of a phase spec says what the registry carries for
#     that phase, and nothing else.
#
# **The missing direction, which let two phases through.** The registry is
# alive — a requirement is born there, gets struck, changes state; the spec is
# written once at scoping and nobody reopens it. Without a comparison, it stays
# on what the phase believed it promised and not on what it promises.
#
# The phase is read from the file name — `07-finitions….md` carries phase 7 —
# and not from its content: it is the one place where it cannot be wrong. A
# spec without a single identifier is skipped: phases 0 to 2 have no
# requirements, and the registry carries none for them, which the reverse
# direction checks anyway.
specs_dir="${REPO_ROOT}/docs/specs"
spec_absent=()
spec_extra=()
spec_wrong_phase=()
spec_wrong_strike=()
registry_wrong_strike=()
spec_count=0

# The registry's strike-through and its state say the same thing, and that has
# to be true before using it to judge the specs: the state is authoritative
# below, and a registry that struck at random would get the spec blamed.
for id in "${!state_of[@]}"; do
    expected="plain"
    case "${state_of[${id}]}" in
        abandoned|replaced) expected="struck" ;;
        *) ;;
    esac
    [[ "${struck_in_registry[${id}]}" == "${expected}" ]] \
        || registry_wrong_strike+=("${id} (${state_of[${id}]}, written ${struck_in_registry[${id}]})")
done

if (( ${#registry_wrong_strike[@]} > 0 )); then
    report_failure "in the registry, the strike-through and the state contradict each other:
$(printf '    %s\n' "${registry_wrong_strike[@]}")
    an abandoned or replaced requirement is struck through, the other two states are not."
fi

if [[ -d "${specs_dir}" ]]; then
    declare -A spec_of_phase=()

    for spec in "${specs_dir}"/[0-9][0-9]-*.md; do
        [[ -e "${spec}" ]] || continue
        base="$(basename "${spec}")"
        # 10# forces base ten: without it "07" would be read as octal, and "08"
        # would not be read at all.
        phase="$(( 10#${base%%-*} ))"
        spec_of_phase["${phase}"]="${base}"

        declare -A in_spec=()
        while read -r id struck; do
            [[ -n "${id}" ]] || continue
            in_spec["${id}"]="${struck}"
            spec_count=$((spec_count + 1))

            if [[ -z "${state_of[${id}]:-}" ]]; then
                spec_extra+=("${id} (${base})")
                continue
            fi
            if [[ "${phase_of[${id}]}" != "${phase}" ]]; then
                spec_wrong_phase+=("${id} (${base}, registry: phase ${phase_of[${id}]})")
                continue
            fi
            expected="plain"
            case "${state_of[${id}]}" in
                abandoned|replaced) expected="struck" ;;
                *) ;;
            esac
            [[ "${struck}" == "${expected}" ]] \
                || spec_wrong_strike+=("${id} (${base}, ${state_of[${id}]}, written ${struck})")
        done < <(read_spec_ids "${spec}")

        for id in "${!state_of[@]}"; do
            [[ "${phase_of[${id}]}" == "${phase}" ]] || continue
            [[ -n "${in_spec[${id}]:-}" ]] || spec_absent+=("${id} (${base})")
        done

        unset in_spec
    done

    # A phase the registry names and no spec carries: either the phase is
    # written the wrong way round, or the column carries a typo. Both show here
    # and nowhere else.
    orphan_phases=()
    for id in "${!state_of[@]}"; do
        phase="${phase_of[${id}]}"
        [[ -n "${spec_of_phase[${phase}]:-}" ]] || orphan_phases+=("${id} (phase ${phase})")
    done

    if (( ${#orphan_phases[@]} > 0 )); then
        report_failure "requirements whose phase matches no spec:
$(printf '    %s\n' "${orphan_phases[@]}")
    correct the \"Phase\" column, or write the phase's spec."
    fi
fi

if (( ${#spec_extra[@]} > 0 )); then
    report_failure "requirements promised by a spec and missing from the registry:
$(printf '    %s\n' "${spec_extra[@]}")
    add the requirement to the registry, or remove it from the spec."
fi

if (( ${#spec_absent[@]} > 0 )); then
    report_failure "registry requirements missing from the table of their spec:
$(printf '    %s\n' "${spec_absent[@]}")
    add the row to the phase's spec — the spec is what promises."
fi

if (( ${#spec_wrong_phase[@]} > 0 )); then
    report_failure "requirements whose spec and registry do not state the same phase:
$(printf '    %s\n' "${spec_wrong_phase[@]}")"
fi

if (( ${#spec_wrong_strike[@]} > 0 )); then
    report_failure "requirements whose strike-through in the spec contradicts the registry's state:
$(printf '    %s\n' "${spec_wrong_strike[@]}")
    an abandoned or replaced requirement is struck through on both sides."
fi

if (( ${#spec_extra[@]} + ${#spec_absent[@]} + ${#spec_wrong_phase[@]} + ${#spec_wrong_strike[@]} == 0 )); then
    report_success "${spec_count} spec requirements, all in agreement with the registry"
fi

if (( failures > 0 )); then
    printf '\n%s%d mismatch(es) between the registry, the specs and the tests%s\n' \
        "${RED}" "${failures}" "${RESET}" >&2
    exit 1
fi
