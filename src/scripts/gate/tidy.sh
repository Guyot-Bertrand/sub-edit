#!/usr/bin/env bash
# Static analysis, a property of the build — issue #269.
#
# **Nothing to choose here, and that is the whole point.** The scope is not
# computed: clang-tidy is hooked onto the compile rule of each source, so the
# build system replays exactly what had an input change — the source, a header
# of its dependency file, or its command line. See `cmake/Tidy.cmake` for what
# this replaces and why.
#
# To reanalyse everything: `rm -rf build/tidy`.
set -euo pipefail
# shellcheck source=src/scripts/gate/common.sh
source "$(dirname "${BASH_SOURCE[0]}")/common.sh"
step "static analysis"
cmake --preset tidy
cmake --build --preset tidy -j "${JOBS}"
