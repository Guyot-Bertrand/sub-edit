#!/usr/bin/env bash
# Runs a command on at most $JOBS cores — threads included.
#
# **What `JOBS` did not bound** — issue #630. `JOBS` caps the number of processes `make` and
# `ninja` launch. It says nothing of the threads a process creates by itself, and libmpv creates
# some for every player: measured, a single player test case — three tenths of a second — occupies
# more than five cores in bursts, and `ffmpeg`, which makes the bench video, eight. The gate, which
# allowed itself two cores, therefore took many more each time it went through there, without
# anything it reads — `make parallelism` included — being able to see it.
#
# **Affinity is the only bound threads inherit.** Set on the process that launches the others, it
# holds for all those that libmpv, Qt or ffmpeg start afterwards, however they count cores.
# Measured on the player cases: system time drops from 1.6 s to 0.2 s and the duration does not
# grow — threads that cannot spread out get in each other's way less than they did.
#
# **The last cores, not the first**: the desktop and the interrupts live on the first ones, and a
# gate that occupies two cores does better far from what the person running it is using. Without
# `taskset`, or on a machine that refuses those cores, the command runs all the same, and says so —
# a gate that stopped there would penalise the most modest machine.
#
# `JOBS` comes from the environment, as for every step; CI sets it to all its cores, and the
# command is then not constrained.
#
#   limit-cores.sh ctest --preset asan

set -euo pipefail

if (($# == 0)); then
    printf 'usage: %s COMMAND [ARGUMENT…]\n' "$(basename "$0")" >&2
    exit 2
fi

readonly JOBS="${JOBS:-2}"
online="$(getconf _NPROCESSORS_ONLN)"
readonly online

# Every core is allowed: nothing to bound.
if ((JOBS >= online)); then
    exec "$@"
fi

if ! command -v taskset >/dev/null 2>&1; then
    printf '%s: taskset is missing, the command runs on every core\n' "$(basename "$0")" >&2
    exec "$@"
fi

cores="$((online - JOBS))-$((online - 1))"
if ! taskset --cpu-list "${cores}" true 2>/dev/null; then
    printf '%s: cores %s are refused, the command runs on every core\n' \
        "$(basename "$0")" "${cores}" >&2
    exec "$@"
fi

exec taskset --cpu-list "${cores}" "$@"
