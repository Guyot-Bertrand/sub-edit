#!/usr/bin/env bash
# The installation into a temporary prefix, and what is run there.
#
# **No other check says this.** Everything else runs from the build tree, where
# a data file is present by accident of layout — it is there because the
# repository contains it, not because a rule copied it. The defect only shows at
# the first clean installation.
set -euo pipefail
# shellcheck source=src/scripts/gate/common.sh
source "$(dirname "${BASH_SOURCE[0]}")/common.sh"
"$(dirname "${BASH_SOURCE[0]}")/release.sh"
step "installation into a temporary prefix"
"${REPO_ROOT}/src/scripts/check-installation.sh" --build-dir "${REPO_ROOT}/build/release"
