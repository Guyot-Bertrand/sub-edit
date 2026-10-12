#!/usr/bin/env bash
# No user-visible string escapes translation beyond the recorded count — spec 15, D10, #661.
#
# The scan reads the tracked C++ of `src/lib` and `src/exe` and takes about a second, so it
# sits in `check`, beside `arch`, before anything is built: a literal added without its
# translation hook costs a second to hear about. Its cases are replayed by the `fixtures` step,
# and `verify-gates.sh` proves on a scratch copy that an unwrapped literal is refused.
set -euo pipefail
# shellcheck source=src/scripts/gate/common.sh
source "$(dirname "${BASH_SOURCE[0]}")/common.sh"
step "user-visible strings outside the translation API"
"${REPO_ROOT}/src/scripts/check-translatable-strings.py"
