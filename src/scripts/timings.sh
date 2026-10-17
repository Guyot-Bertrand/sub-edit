#!/usr/bin/env bash
# Reads the local journal of the gate's timings, and says which ones moved.
#
#     ./src/scripts/timings.sh            the last 5 runs of each step and proof
#     ./src/scripts/timings.sh 10         the last 10
#     ./src/scripts/timings.sh 5 check    only the suite `check`
#     make timings RUNS=10
#
# **The question it answers is « is that more than usual ».** The table that
# `gate.sh` and `verify-gates.sh` write at the end of a run says where its time
# went; it cannot say whether a static analysis of an hour is an accident,
# because it remembers nothing. The journal does — `build/gate-timings.tsv`,
# written by `gate/timing.sh`, whose header says what goes in it and when.
#
# One row per step, phase or proof, suite by suite, in the order they first
# appear in the journal: the last runs side by side, oldest first, then the
# median of the runs shown **before** the latest, the load at the start and end of the
# latest, and its `JOBS`. A latest run above one and a half times that median is
# flagged — with the load and the `JOBS` beside it, which are what tell a busy
# machine or a reduced parallelism from a slower gate.
#
# **Below ten seconds, nothing is flagged.** A phase that goes from two seconds
# to four has doubled, and means nothing: whole seconds are too coarse there,
# and a flag that fires on noise teaches one to stop reading it.
#
# Each row compares a name with its own past, so the columns of two rows are
# not the same runs: a proof added yesterday has one run, the step beside it
# fifty. `GATE_TIMINGS_FILE` reads another journal, as `gate/timing.sh` writes
# one.
#
# Exit status: 0, flagged or not — this is a reading, not a gate; 1 if there is
# no journal; 2 on a wrong argument.

set -euo pipefail

readonly REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
readonly JOURNAL="${GATE_TIMINGS_FILE:-${REPO_ROOT}/build/gate-timings.tsv}"
readonly RATIO=1.5
readonly FLOOR=10

usage() {
    cat >&2 <<'USAGE'
usage: timings.sh [RUNS] [SUITE]   in either order

  RUNS   how many of the last runs to show for each name   (default: 5)
  SUITE  check, check-local or verify-gates                 (default: all)
USAGE
    exit 2
}

# A number is the count of runs, anything else the suite, in either order — `make timings SUITE=check`
# passes the suite alone.
runs=5
suite=""
(($# <= 2)) || usage
for argument in "$@"; do
    case "${argument}" in
    -h | --help) usage ;;
    0 | -*) usage ;;
    *[!0-9]*) suite="${argument}" ;;
    *) runs="${argument}" ;;
    esac
done

if [[ ! -s "${JOURNAL}" ]]; then
    printf 'no journal: %s\n' "${JOURNAL}" >&2
    printf '  it is written by « make check », « make check-local » and « make verify-gates »\n' >&2
    exit 1
fi

# Under the C locale, so that a French one does not write the median « 103,5 » nor read the ratio
# « 1.5 » as one.
LC_ALL=C awk -F '\t' -v runs="${runs}" -v only="${suite}" -v ratio="${RATIO}" -v floor="${FLOOR}" '
# The median of the values of `list` from index `from` to `to`, sorted in place in a copy.
function median(list, from, to,    copy, n, i, j, value) {
    n = 0
    for (i = from; i <= to; i++) copy[++n] = list[i] + 0
    for (i = 2; i <= n; i++) {
        value = copy[i]
        for (j = i - 1; j >= 1 && copy[j] > value; j--) copy[j + 1] = copy[j]
        copy[j + 1] = value
    }
    if (n % 2) return copy[(n + 1) / 2]
    return (copy[n / 2] + copy[n / 2 + 1]) / 2
}

NR == 1 && $1 == "date" { next }
NF < 9 { next }
only != "" && $4 != only { next }
{
    key = $4 SUBSEP $5
    if (!(key in placed)) {
        if (!($4 in names)) suites[++suite_count] = $4
        # A phase is journalled before its step, which ends after it: the step takes its place
        # first, so that it reads above its phases.
        if (index($5, "/") > 1) {
            parent = substr($5, 1, index($5, "/") - 1)
            if (!(($4 SUBSEP parent) in placed)) {
                placed[$4 SUBSEP parent] = 1
                order[$4, ++names[$4]] = parent
            }
        }
        placed[key] = 1
        order[$4, ++names[$4]] = $5
    }
    seconds[key, ++count[key]] = $6
    load[key] = sprintf("%5s→%-5s", $7, $8)
    jobs[key] = $9
    latest[$4] = $1 "  " $2 "  " $3
}

END {
    if (suite_count == 0) {
        printf "nothing in the journal%s\n", (only != "" ? " for " only : "")
        exit 0
    }
    flagged = 0
    for (s = 1; s <= suite_count; s++) {
        suite = suites[s]
        printf "\n\033[1m%s\033[0m — last %d run(s) of each, oldest first; latest run %s\n", suite, runs, latest[suite]
        header = ""
        for (i = 1; i <= runs; i++) header = header sprintf("%6s", i == runs ? "last" : "")
        printf "%s  %7s  %-11s  %4s  %s\n", header, "median", "   load", "jobs", "name"
        for (n = 1; n <= names[suite]; n++) {
            name = order[suite, n]
            key = suite SUBSEP name
            total = count[key] + 0
            if (total == 0) continue
            first = total - runs + 1
            if (first < 1) first = 1
            row = ""
            for (i = 0; i < runs - (total - first + 1); i++) row = row sprintf("%6s", "")
            for (i = first; i <= total; i++) row = row sprintf("%6d", seconds[key, i])
            mark = ""
            if (total - first >= 1) {
                delete previous
                for (i = first; i < total; i++) previous[i - first + 1] = seconds[key, i]
                middle = median(previous, 1, total - first)
                shown = sprintf("%7.1f", middle)
                last = seconds[key, total] + 0
                if (last >= floor && last > ratio * middle) {
                    mark = sprintf("  \033[33m⚠ %s×\033[0m", middle > 0 ? sprintf("%.1f", last / middle) : "∞")
                    flagged++
                }
            } else {
                shown = sprintf("%7s", "-")
            }
            printf "%s  %s  %s  %4s  %s%s\n", row, shown, load[key], jobs[key], name, mark
        }
    }
    if (flagged) printf "\n%d latest run(s) above %s× the median of the previous ones\n", flagged, ratio
}
' "${JOURNAL}"
