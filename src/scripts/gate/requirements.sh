#!/usr/bin/env bash
# The requirements register, compared with the tests and the phase specs.
set -euo pipefail
# shellcheck source=src/scripts/gate/common.sh
source "$(dirname "${BASH_SOURCE[0]}")/common.sh"
step "requirements"
timed configure cmake --preset dev >/dev/null
timed build cmake --build --preset dev -j "${JOBS}" --target subedit_e2e_test subedit_gui_test
timed register "${REPO_ROOT}/src/scripts/check-requirements.sh"
