#!/usr/bin/env bash
# What a conversion loses, replayed and compared with its record.
#
# **The same gesture as `score.sh`, on the phase's other unknown.** A detection
# has a rate; a conversion has a loss. Neither is looked at once: what matters
# is the comparison with the previous measurement, and a measurement nobody
# replays grows old without anyone learning it — that is what issue #311 found
# of the score, which had slept through a whole phase.
#
# **The private corpus is never read here**, as for the score: absent from one
# machine in two, it would make the same gate say two things.
#
# What the step builds is the command line itself. The measurement is therefore
# of what the user gets, not of a test path.

set -euo pipefail

# shellcheck source=src/scripts/gate/common.sh
source "$(dirname "${BASH_SOURCE[0]}")/common.sh"

step "conversion loss"

timed configure cmake --preset dev >/dev/null
timed build cmake --build --preset dev -j "${JOBS}" --target subedit-cli

"${REPO_ROOT}/src/scripts/measure-conversion-loss.py" \
    --binary "${REPO_ROOT}/build/dev/bin/subedit-cli" \
    --journal
