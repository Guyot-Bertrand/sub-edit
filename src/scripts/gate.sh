#!/usr/bin/env bash
# The quality gate, and the order of its steps — issue #269.
#
# ## What it brings, and why it exists
#
# The steps used to live in `Makefile` recipes, chained through sub-`make`
# calls. That works, and it has a defect paid on every late failure — **there is
# no way to resume in the middle.** A coverage failure at the thirty-fifth
# minute meant paying again for the first thirty-four, format and static
# analysis included, although nothing had moved there.
#
#     ./src/scripts/gate.sh --list
#     ./src/scripts/gate.sh check --from coverage
#     ./src/scripts/gate.sh check --only tidy
#     ./src/scripts/gate.sh check-local --skip bench
#
# **The order lives here and nowhere else.** It is the only thing this file
# knows and the steps do not: each one runs alone, without knowing what comes
# before it. `src/scripts/gate/<step>.sh` holds what a step does; this file
# holds when.
#
# ## What it does not do
#
# **It does not replace `make`.** The targets remain, reduced to one call: they
# are the interface one types, and `verify-gates.sh` invokes them to prove that
# each gate closes. This script is what they call, and what one calls directly
# to resume in the middle.
#
# **It skips nothing silently.** An unknown step name, given to `--from`,
# `--only` or `--skip`, makes the invocation fail. A typo that ran no step and
# returned zero would be the worst defect of all: a green gate that did
# nothing.

set -euo pipefail

readonly REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
readonly STEPS_DIR="${REPO_ROOT}/src/scripts/gate"

readonly BOLD=$'\033[1m'
readonly GREEN=$'\033[32m'
readonly RED=$'\033[31m'
readonly RESET=$'\033[0m'

# **The order of `check`, and it is not arbitrary.**
#
# The record of untracked files frames everything else, not only the tests:
# what is watched is what the gate leaves behind, wherever it comes from. The
# format goes before the build because it costs a second, and a format failure
# should cost a second. Static analysis comes after the build because it is
# one.
#
# The scan of user-visible strings (#661) follows `arch` for the same reason the
# format comes first: it reads the sources, builds nothing, and costs a second.
readonly CHECK_STEPS=(
    untracked-record
    config-home-record
    format-check
    arch
    untranslated
    build
    tidy
    asan
    coverage
    untracked
    config-home
)

# **The order of `check-local`, cheapest first**, so that a failure costs
# seconds rather than the whole chain: parallelism (a grep, under a second),
# message template (xgettext over the sources), catalogues against that
# template (msgmerge into a scratch copy, a second), fixtures (two calls to
# ffprobe), encoding then format detection score (the core and a forty-line
# program each), conversion loss (the command line, then some twenty calls),
# manual (one more binary, and Qt), requirements (incremental build), end to
# end (release build), installation — which shares that tree — then the
# benchmarks.
readonly LOCAL_STEPS=(
    parallelism
    pot-check
    translations
    fixtures
    score
    score-format
    conversion
    manual-check
    requirements
    e2e
    install-check
    bench
)

# What each step name runs. Steps that carry logic live in `gate/`; those
# that already are a script of their own are called as they are, rather than
# wrapped for the sake of it.
run_step() {
    case "$1" in
    untracked-record) "${REPO_ROOT}/src/scripts/check-untracked.sh" --record ;;
    config-home-record) "${REPO_ROOT}/src/scripts/check-config-home.sh" --record ;;
    format-check) "${STEPS_DIR}/format.sh" --check ;;
    format) "${STEPS_DIR}/format.sh" ;;
    arch) printf '%s▸ architecture invariants%s\n' "${BOLD}" "${RESET}"
        "${REPO_ROOT}/src/scripts/check-architecture.sh" ;;
    untranslated) "${STEPS_DIR}/untranslated.sh" ;;
    build) "${STEPS_DIR}/build.sh" ;;
    tidy) "${STEPS_DIR}/tidy.sh" ;;
    asan) "${STEPS_DIR}/asan.sh" ;;
    coverage) "${STEPS_DIR}/coverage.sh" ;;
    untracked) printf '%s▸ files left behind%s\n' "${BOLD}" "${RESET}"
        "${REPO_ROOT}/src/scripts/check-untracked.sh" --compare ;;
    config-home) printf '%s▸ user configuration%s\n' "${BOLD}" "${RESET}"
        "${REPO_ROOT}/src/scripts/check-config-home.sh" --compare ;;
    parallelism) printf '%s▸ controlled parallelism%s\n' "${BOLD}" "${RESET}"
        "${REPO_ROOT}/src/scripts/check-parallelism.sh" ;;
    pot-check) "${STEPS_DIR}/pot.sh" ;;
    translations) "${STEPS_DIR}/translations.sh" ;;
    fixtures) "${STEPS_DIR}/fixtures.sh" ;;
    score) "${STEPS_DIR}/score.sh" ;;
    score-format) "${STEPS_DIR}/score-format.sh" ;;
    conversion) "${STEPS_DIR}/conversion.sh" ;;
    manual-check) "${STEPS_DIR}/manual.sh" --check ;;
    manual) "${STEPS_DIR}/manual.sh" ;;
    requirements) "${STEPS_DIR}/requirements.sh" ;;
    e2e) "${STEPS_DIR}/e2e.sh" ;;
    install-check) "${STEPS_DIR}/install-check.sh" ;;
    bench) "${STEPS_DIR}/bench.sh" ;;
    release) "${STEPS_DIR}/release.sh" ;;
    packages) "${STEPS_DIR}/packages.sh" ;;
    rpm-check) "${STEPS_DIR}/rpm-check.sh" ;;
    *)
        printf '%sunknown step: %s%s\n' "${RED}" "$1" "${RESET}" >&2
        return 1
        ;;
    esac
}

usage() {
    cat >&2 <<'USAGE'
usage: gate.sh <check|check-local> [--from <step>] [--only <step>] [--skip <step>]
       gate.sh --list
       gate.sh <step>

  --from   resumes at this step, and runs everything after it
  --only   runs only that one, repeatable
  --skip   runs everything but that one, repeatable
  --list   writes the steps of each suite, in order

Without a named suite, the first argument is a step and it runs alone.
USAGE
    exit 2
}

list_steps() {
    printf '%scheck%s\n' "${BOLD}" "${RESET}"
    printf '  %s\n' "${CHECK_STEPS[@]}"
    printf '%scheck-local%s\n' "${BOLD}" "${RESET}"
    printf '  %s\n' "${LOCAL_STEPS[@]}"
}

# True if `$1` is one of the following arguments. Serves the three filters,
# which ask the same question of three different lists.
contains() {
    local needle="$1"
    shift
    local one
    for one in "$@"; do
        [[ "${one}" == "${needle}" ]] && return 0
    done
    return 1
}

(($# > 0)) || usage

if [[ "$1" == "--list" ]]; then
    list_steps
    exit 0
fi

suite="$1"
shift

# A gate reduced to a few tests is not run by mistake (see gate/asan.sh).
if [[ -n "${GATE_PROOF_TESTS:-}" && ( "${suite}" == check || "${suite}" == check-local ) ]]; then
    printf '%s✗ GATE_PROOF_TESTS is reserved for verify-gates.sh, and would reduce %s to a few tests%s\n' \
        "${RED}" "${suite}" "${RESET}" >&2
    exit 2
fi

case "${suite}" in
check) steps=("${CHECK_STEPS[@]}") ;;
check-local) steps=("${LOCAL_STEPS[@]}") ;;
-h | --help) usage ;;
*)
    # A step named alone: no filter, no suite.
    (($# == 0)) || usage
    run_step "${suite}"
    exit $?
    ;;
esac

from=""
only=()
skip=()

while (($# > 0)); do
    case "$1" in
    --from) [[ $# -ge 2 ]] || usage; from="$2"; shift 2 ;;
    --only) [[ $# -ge 2 ]] || usage; only+=("$2"); shift 2 ;;
    --skip) [[ $# -ge 2 ]] || usage; skip+=("$2"); shift 2 ;;
    -h | --help) usage ;;
    *) printf 'unknown argument: %s\n' "$1" >&2; usage ;;
    esac
done

# **Every name given must designate a step of the suite**, and the check
# happens before a single one runs. Without it, `--from covrage` would run zero
# steps and return zero: a green gate that did nothing, which this project
# refuses everywhere else.
for named in ${from:+"${from}"} ${only+"${only[@]}"} ${skip+"${skip[@]}"}; do
    if ! contains "${named}" "${steps[@]}"; then
        printf '%s✗ « %s » is not a step of %s%s\n' "${RED}" "${named}" "${suite}" "${RESET}" >&2
        printf '  the steps are:\n' >&2
        printf '    %s\n' "${steps[@]}" >&2
        exit 2
    fi
done

selected=()
reached=0
for one in "${steps[@]}"; do
    [[ -n "${from}" && "${one}" == "${from}" ]] && reached=1
    [[ -n "${from}" && "${reached}" -eq 0 ]] && continue
    ((${#only[@]} == 0)) || contains "${one}" "${only[@]}" || continue
    ((${#skip[@]} == 0)) || ! contains "${one}" "${skip[@]}" || continue
    selected+=("${one}")
done

# The same guard as above, from the other side: the names were right and yet
# the combination keeps nothing — `--only a --skip a`, for instance.
if ((${#selected[@]} == 0)); then
    printf '%s✗ no step selected%s\n' "${RED}" "${RESET}" >&2
    exit 2
fi

printf '%s%s — %d step(s)%s\n' "${BOLD}" "${suite}" "${#selected[@]}" "${RESET}"

# **The time of each step**, written on exit — even on a failure: the last line is then the step
# that failed, with what it cost. It is the figure that was missing to decide what to lighten
# (#686): without it, « the gate is long » designates nothing.
durations=()
report_durations() {
    local status=$?
    ((${#durations[@]} == 0)) || {
        printf '\n%sstep durations of %s%s\n' "${BOLD}" "${suite}" "${RESET}"
        local entry
        for entry in "${durations[@]}"; do
            printf '%7d s  %s\n' "${entry%% *}" "${entry#* }"
        done
    }
    return "${status}"
}
trap report_durations EXIT

for one in "${selected[@]}"; do
    started=${SECONDS}
    status=0
    run_step "${one}" || status=$?
    durations+=("$((SECONDS - started)) ${one}")
    ((status == 0)) || exit "${status}"
done

printf '%s✓ %s passed%s\n' "${GREEN}" "${suite}" "${RESET}"
