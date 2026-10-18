#!/usr/bin/env bash
# The timings of the gate: what measures a step, a phase of a step or a proof,
# and the local journal that keeps them.
#
# **This file is sourced, not executed** — by `common.sh`, hence by every step,
# and by `gate.sh` and `verify-gates.sh`. It defines functions and variables
# prefixed `timing_`, and nothing `readonly`: its readers already define their
# own colours and root, and a second `readonly` of the same name would kill them.
# For the same reason it takes from `lib/project.sh`, which defines functions
# and no variable.
#
# ## Why a journal
#
# The table that `gate.sh` writes at the end of a run (#686) answers « where did
# this run go ». It does not answer « is that more than usual », and that is the
# question a slow run raises: a `make check` whose static analysis took an hour
# could not be told apart from a forced reconfiguration or a busy machine,
# because nothing remembered the previous ones. Each measurement therefore goes
# with what it needs to be compared — the load average at its start and end, and
# the `JOBS` the steps were given — and is appended to `build/gate-timings.tsv`.
# `src/scripts/timings.sh` reads it.
#
# **The journal is local and disposable.** It lives under `build/`, which git
# ignores — so `check-untracked.sh` does not see it either — and `make clean`
# erases it. It is an observation of one machine, not a record of the project:
# `docs/mesures/` is where measurements that others must be able to read go.
#
# **Writing to it never fails a gate.** A full disk or a read-only tree makes
# the line go missing, not the step turn red.
#
# What is written, and when:
#
#   * `gate.sh check|check-local`  one line per step, and the total of a whole
#                                  run that passed, as `(total)`;
#   * the phases of a step         one line each (`asan/tests`), when the step
#                                  runs inside one of those two suites — a step
#                                  run alone, as the proofs of `verify-gates.sh`
#                                  do with an injected defect, writes nothing;
#   * `verify-gates.sh`            one line per proof, and the `(total)` of a
#                                  run that reached its end.
#
# Columns, separated by tabs: date of the run's start, git short revision,
# project version, suite, name, seconds, load at start, load at end, JOBS.
#
# `GATE_TIMINGS_FILE` moves the journal, which is what a test of the reader
# needs; `gate.sh` exports `GATE_TIMINGS_SUITE`, `GATE_TIMINGS_RUN` and
# `GATE_TIMINGS_STEP` so that the phases of a step land under its name.

if [[ "${BASH_SOURCE[0]}" == "${0}" ]]; then
    printf 'this file is sourced, not executed: source %s\n' "${BASH_SOURCE[0]}" >&2
    exit 2
fi

# shellcheck source=src/scripts/lib/project.sh
source "$(dirname "${BASH_SOURCE[0]}")/../lib/project.sh"

timing_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/../../.." && pwd)"
timing_file="${GATE_TIMINGS_FILE:-${timing_root}/build/gate-timings.tsv}"
# Read once, when sourced: `verify-gates.sh` injects defects into `CMakeLists.txt`, and the version
# of a run is the one it started with.
timing_rev="$(git -C "${timing_root}" rev-parse --short HEAD 2>/dev/null || printf '-')"
timing_version="$(project_version "${timing_root}/CMakeLists.txt" 2>/dev/null || true)"

# The one-minute load average, or « - » where `/proc` does not give it.
timing_load() {
    local load _
    if read -r load _ </proc/loadavg 2>/dev/null; then
        printf '%s' "${load}"
    else
        printf '-'
    fi
}

# The `JOBS` the steps run with. The default is that of `common.sh`, which takes
# it from the `Makefile`: a direct call to `gate.sh` without `JOBS` gives the
# steps two, and the journal must say what they were given.
timing_jobs() {
    printf '%s' "${JOBS:-2}"
}

# Appends one line. Arguments: suite, name, seconds, load at start, load at end.
timing_record() {
    local suite="$1" name="$2" seconds="$3" load_start="$4" load_end="$5"
    local run="${GATE_TIMINGS_RUN:-}"
    [[ -n "${run}" ]] || run="$(date -Iseconds)"
    {
        mkdir -p "$(dirname "${timing_file}")"
        [[ -s "${timing_file}" ]] \
            || printf 'date\trev\tversion\tsuite\tname\tseconds\tload_start\tload_end\tjobs\n' \
                >"${timing_file}"
        # A tab in a name would shift every column after it.
        printf '%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\n' \
            "${run}" "${timing_rev}" "${timing_version:--}" "${suite}" "${name//$'\t'/ }" \
            "${seconds}" "${load_start}" "${load_end}" "$(timing_jobs)" >>"${timing_file}"
    } 2>/dev/null || true
}

# Runs a phase of a step, and writes what it cost: `timed build cmake --build …`.
#
# The line goes to standard error, so that a caller who silences a command —
# `timed configure cmake --preset dev >/dev/null` — silences the command and not
# its measurement. It is written on a failure too, and the status is returned
# unchanged: the step's `set -e` decides, as it did without the wrapper.
#
# **A function given to it runs without `set -e`**: bash suspends it for the left of a `||`, the
# whole call included. The function must therefore stop on its own failures — see `shoot` in
# `manual.sh`.
timed() {
    local label="$1"
    shift
    local started=${SECONDS} load_start load_end seconds status=0
    load_start="$(timing_load)"
    "$@" || status=$?
    seconds=$((SECONDS - started))
    load_end="$(timing_load)"
    printf '  · %s: %d s (load %s→%s)\n' "${label}" "${seconds}" "${load_start}" "${load_end}" >&2
    if [[ -n "${GATE_TIMINGS_SUITE:-}" ]]; then
        timing_record "${GATE_TIMINGS_SUITE}" "${GATE_TIMINGS_STEP:-?}/${label}" \
            "${seconds}" "${load_start}" "${load_end}"
    fi
    return "${status}"
}
