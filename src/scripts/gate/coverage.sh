#!/usr/bin/env bash
# The coverage of the libraries, measured and compared with the ratchet.
#
# Six gestures, and two of them exist against a specific defect:
#
#   * `clean-stale-coverage.sh` discards a tree where a `.gcno` lingers that no
#     source is attached to any more — the leftover of a moved file, on which
#     gcovr fails naming neither the culprit nor the remedy;
#   * the `.gcda` files are deleted after the build and before the tests.
#     Those of an earlier run survive a rebuild and gcov merges them: a file
#     modified since would give a wrong rate, with no other sign than a warning
#     drowned in the output.
#
# The tests stay sequential: `ctest` only parallelises on an explicit `-j`, and
# giving it one would merge the `.gcda` files of several concurrent runs.

set -euo pipefail

# shellcheck source=src/scripts/gate/common.sh
source "$(dirname "${BASH_SOURCE[0]}")/common.sh"

require gcovr

readonly REPORT="${REPO_ROOT}/build/coverage-report"

step "coverage"

"${REPO_ROOT}/src/scripts/clean-stale-coverage.sh"

cmake --preset coverage
cmake --build --preset coverage -j "${JOBS}"

find "${REPO_ROOT}/build/coverage" -name '*.gcda' -delete

"${REPO_ROOT}/src/scripts/limit-cores.sh" ctest --preset coverage

mkdir -p "${REPORT}"
gcovr --root "${REPO_ROOT}" "${REPO_ROOT}/build/coverage/src" \
    --filter 'src/lib/' \
    --exclude-unreachable-branches --exclude-throw-branches \
    --print-summary \
    --json-summary "${REPORT}/summary.json" \
    --html-details "${REPORT}/index.html" \
    --txt "${REPORT}/summary.txt"

printf 'report: build/coverage-report/index.html\n'

"${REPO_ROOT}/src/scripts/check-coverage.sh" --summary "${REPORT}/summary.json"
