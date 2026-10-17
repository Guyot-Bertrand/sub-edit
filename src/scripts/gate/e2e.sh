#!/usr/bin/env bash
# The end-to-end tests alone, in Release.
#
# Filtered by the CTest label `e2e` and not by name: a unit test whose name came
# close would not fool the filter. Shares the release tree with `bench.sh` and
# `release.sh`, which therefore do not rebuild it.
set -euo pipefail
# shellcheck source=src/scripts/gate/common.sh
source "$(dirname "${BASH_SOURCE[0]}")/common.sh"
step "end-to-end tests (release)"
timed configure cmake --preset release -DSUBEDIT_LTO_JOBS="${JOBS}"
timed build cmake --build --preset release -j "${JOBS}" --target subedit_e2e_test
timed tests "${REPO_ROOT}/src/scripts/limit-cores.sh" ctest --preset release -L e2e
