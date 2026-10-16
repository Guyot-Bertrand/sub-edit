#!/usr/bin/env bash
# The two native packages, produced from the release tree — ADR 0023.
#
# **A step apart, which belongs to no gate.** `check-installation.sh` already
# builds both packages and compares their file lists — that is where they are
# tested. This one exists to have them at hand, named and in place, on the day
# they are published.

set -euo pipefail

# shellcheck source=src/scripts/gate/common.sh
source "$(dirname "${BASH_SOURCE[0]}")/common.sh"

require cpack
require rpmbuild

"$(dirname "${BASH_SOURCE[0]}")/release.sh"

step "native packages"

(cd "${REPO_ROOT}/build/release" && cpack -G "DEB;RPM" >/dev/null)

ls -1 "${REPO_ROOT}"/build/release/subedit*.deb "${REPO_ROOT}"/build/release/subedit*.rpm \
    | sed 's/^/  /'
