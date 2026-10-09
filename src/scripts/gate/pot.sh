#!/usr/bin/env bash
# Le gabarit des messages est ce que le code produit — issue #659.
set -euo pipefail
# shellcheck source=src/scripts/gate/common.sh
source "$(dirname "${BASH_SOURCE[0]}")/common.sh"
step "gabarit des messages"
"${REPO_ROOT}/src/scripts/update-pot.sh" --check
