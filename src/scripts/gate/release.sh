#!/usr/bin/env bash
# The two optimised binaries, and nothing else.
#
# Neither the bench nor the end-to-end harness: they have their own steps. What
# is asked for here is what a user runs, not what tests it.
#
# **`JOBS` governs LTO**, for the reason written in `bench.sh`: without it,
# link-time optimisation starts as many processes as there are cores at every
# link. That is what makes this step preferable to the two `cmake` commands
# typed by hand, which know nothing of it.

set -euo pipefail

# shellcheck source=src/scripts/gate/common.sh
source "$(dirname "${BASH_SOURCE[0]}")/common.sh"

step "binaries (release)"

timed configure cmake --preset release -DSUBEDIT_LTO_JOBS="${JOBS}"
timed build cmake --build --preset release -j "${JOBS}" --target subedit-cli subedit-gui

printf '  binaries: build/release/bin/subedit-cli, build/release/bin/subedit-gui\n'
