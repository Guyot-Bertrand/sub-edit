#!/usr/bin/env bash
# The `.rpm`, installed on a real Fedora — issue #266.
#
# **A step apart, which belongs to no gate**, like `packages` and for one more
# reason: it needs a container and the network. A gate that depends on the
# network is red on an outage day, for a reason foreign to the repository. It is
# run by hand, and once a week on `main` — `.github/workflows/fedora.yml`.
#
# The package is rebuilt before being tested: testing the one lying in the tree
# would mean answering about a version nobody can name any more.

set -euo pipefail

# shellcheck source=src/scripts/gate/common.sh
source "$(dirname "${BASH_SOURCE[0]}")/common.sh"

"$(dirname "${BASH_SOURCE[0]}")/packages.sh"

step "the .rpm on a Fedora"

"${REPO_ROOT}/src/scripts/check-rpm.sh" --build-dir "${REPO_ROOT}/build/release"
