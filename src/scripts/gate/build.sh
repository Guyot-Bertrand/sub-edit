#!/usr/bin/env bash
# The build of the dev preset.
set -euo pipefail
# shellcheck source=src/scripts/gate/common.sh
source "$(dirname "${BASH_SOURCE[0]}")/common.sh"
step "build (dev)"
timed configure cmake --preset dev
timed build cmake --build --preset dev -j "${JOBS}"
