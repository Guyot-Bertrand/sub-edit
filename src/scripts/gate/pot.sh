#!/usr/bin/env bash
# The message template is what the code produces — issue #659.
set -euo pipefail
# shellcheck source=src/scripts/gate/common.sh
source "$(dirname "${BASH_SOURCE[0]}")/common.sh"
step "message template"
"${REPO_ROOT}/src/scripts/update-pot.sh" --check
