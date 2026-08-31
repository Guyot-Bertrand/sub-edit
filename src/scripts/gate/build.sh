#!/usr/bin/env bash
# La compilation du preset dev.
set -euo pipefail
# shellcheck source=src/scripts/gate/common.sh
source "$(dirname "${BASH_SOURCE[0]}")/common.sh"
step "compilation (dev)"
cmake --preset dev
cmake --build --preset dev -j "${JOBS}"
