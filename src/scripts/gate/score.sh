#!/usr/bin/env bash
# The score of encoding detection, replayed and compared with its record.
#
# **It existed, and nobody used it twice** — issue #311.
# `score-encoding-detection.py` dates from #290, and the journal's 9/9 had been
# recorded by hand at version 0.8.16: nothing replayed it, so it grew old
# without anyone learning it. It is the most discreet form of the defect the
# phase 7 review wrote down — there, a check went through a tool that was not
# the one that counted; here the tool is the right one.
#
# **The private corpus is never read here.** It is absent from any machine that
# does not have it, so a gate that read it would say two different things
# depending on the workstation. `--private` remains a manual gesture.
#
# What the step builds is the project's detection on the command line, and
# nothing else: the core and a forty-line program.

set -euo pipefail

# shellcheck source=src/scripts/gate/common.sh
source "$(dirname "${BASH_SOURCE[0]}")/common.sh"

step "encoding detection score"

timed configure cmake --preset dev >/dev/null
timed build cmake --build --preset dev -j "${JOBS}" --target subedit_detect_encoding

"${REPO_ROOT}/src/scripts/score-encoding-detection.py" \
    --detector "${REPO_ROOT}/build/dev/bin/subedit_detect_encoding {}" \
    --journal
