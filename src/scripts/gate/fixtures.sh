#!/usr/bin/env bash
# Do the generated fixtures say what their table announces, and do the checks replay their cases.
set -euo pipefail
# shellcheck source=src/scripts/gate/common.sh
source "$(dirname "${BASH_SOURCE[0]}")/common.sh"
step "video fixtures"
"${REPO_ROOT}/src/scripts/video-fixtures.sh" --check
step "grid fixtures"
"${REPO_ROOT}/src/scripts/subtitle-fixtures.py" --check
step "encoding fixtures"
"${REPO_ROOT}/src/scripts/encoding-fixtures.py" --check
step "correction patterns"
"${REPO_ROOT}/src/scripts/pattern-oracle.py" --check
step "cross-check of adjust: the script against the counts written by hand"
"${REPO_ROOT}/src/scripts/measure-duration-constraints.py" --check-fixtures
step "shifted pairs"
"${REPO_ROOT}/src/scripts/shifted-pairs.py" --check
step "wrong-rate conversions"
"${REPO_ROOT}/src/scripts/wrong-rate-pairs.py" --check
step "translated manual structure"
"${REPO_ROOT}/src/scripts/check-translation-structure.py" --check-fixtures
step "repository language: the classifier and the allowlist rules"
"${REPO_ROOT}/src/scripts/check-comment-language.py" --check-fixtures
