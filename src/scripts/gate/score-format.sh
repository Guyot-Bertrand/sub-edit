#!/usr/bin/env bash
# The score of format detection, replayed and compared with its record.
#
# **The same gesture as `score.sh`, on the other unknown of phase 9.** With two
# formats the question was closed in advance; with nine, `.sub` and `.txt` each
# designate two formats and a detection can be wrong. What is measured here is
# therefore a classifier, exactly as #290 established for the encoding.
#
# **The private corpus is never read here**, as for the two other records:
# absent from one machine in two, it would make the same gate say two things.
#
# What the step builds is the project's detection on the command line, and
# nothing else: the core and a fifty-line program.

set -euo pipefail

# shellcheck source=src/scripts/gate/common.sh
source "$(dirname "${BASH_SOURCE[0]}")/common.sh"

step "format detection score"

cmake --preset dev >/dev/null
cmake --build --preset dev -j "${JOBS}" --target subedit_detect_format

"${REPO_ROOT}/src/scripts/score-format-detection.py" \
    --detector "${REPO_ROOT}/build/dev/bin/subedit_detect_format {}" \
    --journal
