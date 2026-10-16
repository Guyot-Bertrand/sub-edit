#!/usr/bin/env bash
# Builds the `.rpm` on the Fedora it runs on, against the ICU that Fedora ships.
#
#   build-rpm.sh
#
# **Why this script exists**: an `.rpm` built on Ubuntu requires
# `libicuuc.so.74` and `libicui18n.so.74`, whose symbols carry the number of
# ICU's major version (`u_strToUTF8_74`). A Fedora that has neither ICU 74 nor
# its compatibility package refuses to install it, and nothing in the package
# can remedy that. The published `.rpm` is therefore built **on each Fedora
# that is published**, against its ICU, and named after it
# (`subedit-X.Y.Z-1.fc43.x86_64.rpm`). See ADR 0023.
#
# To be run in a `fedora:N` image, as root: it installs what it needs.
# The `.deb` is not concerned, it is built as before.

set -euo pipefail

readonly REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"

[[ -r /etc/fedora-release ]] || { echo "this script runs on Fedora" >&2; exit 1; }

# The same libraries `setup-toolchain.sh` installs on Debian;
# `mpv-devel` carries `mpv.pc`, `cli11-devel` the header-only library.
dnf install -y --setopt=install_weak_deps=False \
    gcc-c++ cmake ninja-build git rpm-build pkgconf-pkg-config python3 \
    qt6-qtbase-devel mpv-devel libicu-devel enchant2-devel cli11-devel

# The ceiling comes from the environment, as for the other scripts: the workflow
# sets it to `nproc`, a local call keeps the cautious default.
readonly JOBS="${JOBS:-2}"
export CMAKE_GENERATOR="Ninja"

cd "${REPO_ROOT}"
cmake --preset release -DSUBEDIT_LTO_JOBS="${JOBS}"
cmake --build --preset release -j "${JOBS}" --target subedit-cli subedit-gui
(cd build/release && cpack -G RPM >/dev/null)

ls -1 build/release/subedit*.rpm
