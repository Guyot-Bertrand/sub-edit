#!/usr/bin/env bash
# Les fixtures engendrées disent-elles ce que leur table annonce.
set -euo pipefail
# shellcheck source=src/scripts/gate/common.sh
source "$(dirname "${BASH_SOURCE[0]}")/common.sh"
step "fixtures vidéo"
"${REPO_ROOT}/src/scripts/video-fixtures.sh" --check
step "fixtures de grille"
"${REPO_ROOT}/src/scripts/subtitle-fixtures.py" --check
