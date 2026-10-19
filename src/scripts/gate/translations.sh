#!/usr/bin/env bash
# No installed language falls behind the template — spec 15, D10, #670.
#
# The template is merged, as `make po` would merge it, into a scratch copy of every catalogue of
# `src/po/`; an installed language (a line of `LINGUAS`) must come out complete, a seed is only
# reported. It reads `.po` files and runs msgmerge, builds nothing and takes about a second, so
# it sits in `check-local` right after `pot-check`, which proves the template it reads is fresh.
# Its cases are replayed by the `fixtures` step, and `verify-gates.sh` proves on a scratch copy
# that an emptied French entry is refused and the same defect in a seed is not.
#
# **And the review caveat agrees with `src/po/UNREVIEWED`** (#720, spec 15 D5): a language the list
# names says it is machine-translated in its catalogue, its manual pages and the Language sections
# of the English manual, and a language it no longer names says it nowhere
# (`check-review-caveat.py`, a fraction of a second; proven by `verify-gates.sh`).
set -euo pipefail
# shellcheck source=src/scripts/gate/common.sh
source "$(dirname "${BASH_SOURCE[0]}")/common.sh"
step "translations against the template: installed languages complete, seeds reported"
"${REPO_ROOT}/src/scripts/check-translations.py"
"${REPO_ROOT}/src/scripts/check-review-caveat.py"
