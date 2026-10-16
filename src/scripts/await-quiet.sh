#!/usr/bin/env bash
# Waits for the machine to be quiet enough for a measurement to mean
# something, and writes the final load.
#
# A performance measurement taken while a browser compiles JavaScript says
# nothing about the code: it says the state of the machine. The performance
# journal paid for it — the record of version 0.2.15, taken under a load of
# 7.2, set twelve maxima at once, one of them at 3.4 times its usual value, on
# a ticket that touched no measured path.
#
# Waiting rather than refusing at once: the most frequent cause of a high load
# here is the preceding target — `make check-local` builds and runs the
# end-to-end tests right before calling the benchmarks, and the one-minute
# average takes a minute to forget them.
#
# **Thirty seconds, and no more** — issue #270. The delay used to be three
# minutes, with a heuristic that gave up earlier if the load did not drop. The
# measurement showed both were too much, and for the same reason: **what
# recovers recovers in twenty seconds, what does not recover does not recover in
# three minutes either.**
#
# What `e2e` and `install-check` leave behind peaks at 1.78 five seconds after
# they end — the one-minute average lags behind what just finished — and drops
# back under the threshold around the twentieth. What lasts longer comes from
# elsewhere: the tail of the `make check` that precedes `make check-local` in
# the prescribed sequence, or a desktop session. Both last minutes and have no
# reason to stop because one is waiting.
#
# The four runs that settled it are in docs/mesures/performances.md.
#
# Standard output: the final load, a number, always written.
# Exit status: 0 if it went under the threshold, 1 otherwise. The caller
# decides what to do with a measurement taken on a busy machine; this script
# only decides to wait.

set -euo pipefail

readonly INTERVAL=5

below=2.0
timeout=30

usage() {
    cat >&2 <<'USAGE'
usage: await-quiet.sh [--below <load>] [--timeout <seconds>]

  --below    load threshold under which to measure   (default: 2.0)
  --timeout  maximum wait, in seconds                (default: 30)

Writes the final load on standard output. Returns 0 if it went under the
threshold, 1 if the delay expired — the load is written in both cases.
USAGE
    exit 2
}

while (( $# > 0 )); do
    case "$1" in
        --below)   [[ $# -ge 2 ]] || usage; below="$2";   shift 2 ;;
        --timeout) [[ $# -ge 2 ]] || usage; timeout="$2"; shift 2 ;;
        -h|--help) usage ;;
        *) printf 'unknown argument: %s\n' "$1" >&2; usage ;;
    esac
done

# The one-minute average, not the number of runnable tasks: that one is
# instantaneous, so a sample can fall between two bursts and declare quiet a
# machine that is not.
current_load() {
    cut -d' ' -f1 /proc/loadavg
}

# awk rather than bc: awk is in coreutils in practice, bc is one more package,
# and the project's dependency rule holds for scripts too.
is_below() {
    awk -v load="$1" -v limit="$2" 'BEGIN { exit !(load < limit) }'
}

# No /proc — another system, an exotic container. The check does not apply
# rather than failing: better an unqualified measurement than a build chain
# that refuses to run.
if [[ ! -r /proc/loadavg ]]; then
    printf 'unknown\n'
    exit 0
fi

# **There is no decay heuristic any more, and nothing left to arbitrate.** The
# script used to judge whether the load « dropped enough » for waiting to make
# sense, and gave up otherwise. That was useful against a three-minute delay;
# against thirty seconds, the question no longer arises — sampling goes on to
# the end of the delay, which costs less than deciding.
waited=0
load="$(current_load)"
while ! is_below "${load}" "${below}"; do
    if (( waited >= timeout )); then
        printf '%s\n' "${load}"
        exit 1
    fi
    sleep "${INTERVAL}"
    waited=$(( waited + INTERVAL ))
    load="$(current_load)"
done

printf '%s\n' "${load}"
