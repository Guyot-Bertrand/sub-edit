#!/usr/bin/env bash
# Le registre d'exigences, confronté aux tests et aux specs de phase.
set -euo pipefail
# shellcheck source=src/scripts/gate/common.sh
source "$(dirname "${BASH_SOURCE[0]}")/common.sh"
step "exigences"
cmake --preset dev >/dev/null
cmake --build --preset dev -j "${JOBS}" --target subedit_e2e_test subedit_gui_test
"${REPO_ROOT}/src/scripts/check-requirements.sh"
