#!/usr/bin/env bash
# The benchmarks, in Release, and the record added to the journal.
#
# **`JOBS` also governs link-time optimisation.** Without it, LTO starts with
# `-flto=auto`, that is as many processes as cores, at every link — a
# parallelism that shows in no `-j` and saturates a machine on which one is
# doing something else.
#
# The load is read before building and measuring: a measurement taken while a
# browser compiles JavaScript says nothing about the code, it says the state of
# the machine.
#
# **Above the threshold, nothing is written** — issue #270. The record used to
# enter the journal, without the right to set an extreme, with its load in the
# heading. It was a half-measure in the literal sense: a version section nobody
# can compare with anything, since nobody knows what in it comes from the code.
# Almost one record in two of phase 7 was of that kind.
#
# **A measurement that cannot be read is worth less than a missing one**,
# because it takes the place of the one that is missing: the journal has one
# section per version, so the dirty section hides the fact that there was no
# measurement. Writing nothing says so.
#
# The benchmarks run all the same, and their console output stays in front of
# whoever ran the command: what is refused is the record, not the look.
#
# **It is not a failure.** A busy machine is not a defect of the code, and
# `make check-local` has no reason to turn red over it.

set -euo pipefail

# shellcheck source=src/scripts/gate/common.sh
source "$(dirname "${BASH_SOURCE[0]}")/common.sh"

readonly BENCH_MAX_LOAD="${BENCH_MAX_LOAD:-1.5}"
readonly XML="${REPO_ROOT}/build/release/bench.xml"

step "benchmarks (release)"

# **The load is read before the build, not after** — end-of-phase-8 review.
# `await-quiet.sh` reads the one-minute average, that is the past; reading it
# after a Release build means asking whether the machine is free right after
# occupying it oneself. The guard was then answering about its own work.
#
# Measured, two runs one minute apart on the same machine:
#
#     full check-local, with Release rebuild         2.21   refused
#     make bench alone, Release tree already fresh   1.32   recorded
#
# The second started from a **busier** machine than the first and was recorded.
# What separates them is the build this step launches.
#
# What the guard answers now: « was the machine ours when we started ». It is
# the right question — what must be ruled out is a competing load, a browser or
# a development environment, and that one is there before as well as during.
# What it stops seeing is a load that starts during the build; the dispersion
# criterion sees that one afterwards.
#
# The exit status is read, not discarded: it decides whether the record will be
# written. The final load is written in both cases.
quiet=0
load="$("${REPO_ROOT}/src/scripts/await-quiet.sh" --below "${BENCH_MAX_LOAD}")" || quiet=$?

timed configure cmake --preset release -DSUBEDIT_LTO_JOBS="${JOBS}"
timed build cmake --build --preset release -j "${JOBS}" --target subedit_bench

# **The video of the seek and step bench** (#611), made in the build tree and handed to the bench
# through the environment. Without ffmpeg, or if making it fails, it is missing: the bench says so
# and abstains for those measurements — it is not a failure, it is a machine that has nothing to
# take them with.
film="${REPO_ROOT}/build/release/bench-film.mp4"
if command -v ffmpeg >/dev/null 2>&1 && command -v ffprobe >/dev/null 2>&1 \
    && timed "bench video" "${REPO_ROOT}/src/scripts/limit-cores.sh" \
        "${REPO_ROOT}/src/scripts/video-fixtures.sh" --film "${film}"; then
    export SUBEDIT_BENCH_FILM="${film}"
else
    printf '%s⚠%s no film-sized video (ffmpeg missing?) — the seek and step bench abstains.\n' \
        "${YELLOW}" "${RESET}" >&2
fi

# **Under `limit-cores.sh`, like the tests**: libmpv creates its threads by itself, and the player
# bench occupied far more than the two cores the gate allows itself — the bound those threads
# inherit is the affinity, set on the process that launches them.
timed benchmarks "${REPO_ROOT}/src/scripts/limit-cores.sh" "${REPO_ROOT}/build/release/bin/subedit_bench" \
    --reporter console \
    --reporter "xml::out=${XML}"

if ((quiet != 0)); then
    printf '\n%s⚠%s load %s, threshold %s — nothing is added to the journal.\n' \
        "${YELLOW}" "${RESET}" "${load}" "${BENCH_MAX_LOAD}" >&2
    printf '  the figures above tell the state of the machine as much as that of the code;\n' >&2
    printf '  replay « make bench » when it is quiet, or note in the PR that there is\n' >&2
    printf '  no measurement for this version.\n' >&2
    exit 0
fi

"${REPO_ROOT}/src/scripts/record-bench.sh" \
    --xml "${XML}" --mode Release --load "${load}" --below "${BENCH_MAX_LOAD}"
