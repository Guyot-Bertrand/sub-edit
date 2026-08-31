#!/usr/bin/env bash
# Les tests sous ASan et UBSan. Le preset asan enregistre aussi les tests de
# bout en bout, sans filtre d'étiquette : c'est ici qu'ils tournent.
set -euo pipefail
# shellcheck source=src/scripts/gate/common.sh
source "$(dirname "${BASH_SOURCE[0]}")/common.sh"
step "tests sous sanitizers"
cmake --preset asan
cmake --build --preset asan -j "${JOBS}"
ctest --preset asan
