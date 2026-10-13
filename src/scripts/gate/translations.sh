#!/usr/bin/env bash
# No installed language falls behind the template — spec 15, D10, #670.
#
# The template is merged, as `make po` would merge it, into a scratch copy of every catalogue of
# `src/po/`; an installed language (a line of `LINGUAS`) must come out complete, a seed is only
# reported. It reads `.po` files and runs msgmerge, builds nothing and takes about a second, so
# it sits in `check-local` right after `pot-check`, which proves the template it reads is fresh.
# Its cases are replayed by the `fixtures` step, and `verify-gates.sh` proves on a scratch copy
# that an emptied French entry is refused and the same defect in a seed is not.
set -euo pipefail
# shellcheck source=src/scripts/gate/common.sh
source "$(dirname "${BASH_SOURCE[0]}")/common.sh"
step "translations against the template: installed languages complete, seeds reported"
"${REPO_ROOT}/src/scripts/check-translations.py"
