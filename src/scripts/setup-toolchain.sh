#!/usr/bin/env bash
# Installs the development toolchain.
#
# Targets Ubuntu 24.04. The script is idempotent: it installs only what is
# missing, and can be rerun with no side effect.
#
# Requires sudo for the APT packages. git-cliff is not packaged, so it is
# fetched from the GitHub releases into ~/.local/bin.
#
# Two reduced modes:
#
#   --git-cliff-only   installs git-cliff only, and nothing else
#   --list-packages    writes the APT packages, one per line, installs nothing

set -euo pipefail

readonly REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
readonly LOCAL_BIN="${HOME}/.local/bin"
readonly GIT_CLIFF_VERSION="2.10.1"

readonly MODE="${1:-}"

info() { printf '\033[1m%s\033[0m\n' "$*"; }
skip() { printf '  already present: %s\n' "$*"; }

# APT packages: command name -> package name.
# clang-tidy 20 rather than the default version: libstdc++ keeps <expected>
# behind __cpp_concepts >= 202002L, a value Clang 18 does not declare. With it,
# std::expected is invisible to static analysis.
declare -A APT_TOOLS=(
    [ninja]=ninja-build
    [clang-tidy-20]=clang-tidy-20
    [clang-format]=clang-format
    [gcovr]=gcovr
    [ccache]=ccache
    [gh]=gh
    # ffprobe came in with phase 6, which reads the frame rate and the
    # duration from the video container. The package is ffmpeg, of which it is
    # only one of the executables — the command is what is probed, never the
    # package, otherwise a partial installation would pass for a complete one.
    #
    # **The project does not require it from a user**: it requires it from a
    # development machine, which is not the same promise. The code carries the
    # branch of its absence, and src/test/unit/core/io/find_executable_test.cpp
    # walks it without uninstalling anything.
    [ffprobe]=ffmpeg
    # jq came in with the pruner of Actions runs: the proof of its selection,
    # in verify-gates.sh, exercises it on a hand-written list, hence locally.
    # GitHub machines have it out of the box, a freshly installed workstation
    # not necessarily — and a proof that does not run is no proof.
    [jq]=jq
    # msgfmt compiles the .po catalogues into .mo: the fixtures of the reader's tests (#658), then
    # the shipped translations (#659). It is a build tool, never a runtime one.
    [msgfmt]=gettext
    # pkg-config is how libmpv is found: it provides no CMake configuration
    # file, and `find_package(PkgConfig REQUIRED)` fails without it. GitHub
    # machines have it out of the box; a freshly installed desktop Ubuntu, not
    # always — and that is the one this script promises to make buildable.
    [pkg-config]=pkg-config
    # The three of packaging, which came in with #244 — and not before, as
    # ADR 0004 asks: a dependency installed to validate files that do not exist
    # is not justified. They exist since that ticket.
    #
    # The first two validate the desktop files the installation puts down: the
    # `.desktop` and the AppStream metadata. A desktop Ubuntu has them out of
    # the box; a bare build machine, not necessarily.
    [desktop-file-validate]=desktop-file-utils
    [appstreamcli]=appstream
    # **`rpmbuild` is on no Ubuntu**, and without it CPack's RPM generator
    # produces nothing — silently, which is the worse of the two: the target
    # succeeds and the package is missing. The probed command is not `rpm`,
    # which sometimes exists alone, but the one CPack needs.
    [rpmbuild]=rpm
    # **groff came in with #268**, which asked whether each check goes through
    # the tool that matters. The man page was read with `sed` and had never
    # been rendered; it now is, by the one that really renders it, that is the
    # one `man` calls. A desktop Ubuntu has it out of the box, with `man-db`; a
    # bare build image, not always.
    [groff]=groff
    # **man came in with #683**, which generates the command line's pages from its manual and
    # lints them with the tool a reader uses: `check-man-pages.py` and the end-to-end test of
    # `CLI-I18N-04` render every page with `man -l`. A desktop Ubuntu has it; a bare image, not
    # always. `python3` is not probed: the scripts of this directory need it already.
    [man]=man-db
)

# APT packages with no command to probe: a file tells whether they are present.
#
# CLI11 is a header-only library — `command -v` would never see it, and putting
# it in the table above would reinstall it on every run. The probed path is the
# one `find_package(CLI11)` ends up reading.
#
# **Qt came in at phase 5, and the `--with-qt` mode went away with it.** It
# described a world where Qt was optional: that is no longer true, the gate
# builds everything, so anyone who runs `make check` needs it. It had never been
# used anyway, and its flaw was to keep the Qt packages out of
# `--list-packages`, hence out of the CI cache — which would have restored an
# incomplete toolchain with nothing to report it.
#
# **libmpv came in at phase 6**, which puts a player in the window — see
# docs/adr/0020-libmpv-pour-le-lecteur-integre.md. The probed path is a `.pc`
# file and not a `Qt6Config.cmake`: libmpv provides no CMake configuration file,
# only what `pkg-config` needs to find it, so that is the path the configuration
# ends up reading.
#
# **BLAS and LAPACK are there for libmpv, and it is less absurd than it looks.**
# The chain is this: libmpv pulls libavfilter, which pulls ffmpeg's speech
# recognition filter, which pulls `libsphinxbase`, which needs `libblas.so.3`
# and `liblapack.so.3`.
#
# Those two are **virtual packages** — several real packages provide them,
# `libblas3`, `libopenblas0-pthread`, `libmkl-rt`. That is exactly what the
# dependency closure of the CI package cache cannot resolve: it restores
# `libsphinxbase3` with no provider, and the chain arrives with a hole in it.
# Linking then stops on undefined symbols, and loading libmpv would fail the
# same way.
#
# The probed path is the link managed by the alternatives system: if any
# provider is already there, nothing is installed — a machine that already has
# OpenBLAS does not gain a second one.
#
# **ICU came in at phase 8** — ADR 0027 —, and it is the first external
# dependency of the core, hence of `subedit-cli`, which had none. The probed
# path is the converter's header: the runtime library is already there on any
# machine that runs Qt, it is the headers that are missing.
#
# **The last two came in with #260**, which found an icon the desktop could not
# read. The check that catches it goes through gdk-pixbuf — the loader of GTK
# desktops, and the only one that refused the faulty file — so it needs the
# Python module that reaches it and the plugin that can read an SVG. A desktop
# Ubuntu has them out of the box; a bare build machine does not.
declare -A APT_LIBS=(
    [/usr/include/CLI/CLI.hpp]=libcli11-dev
    [/usr/include/unicode/ucnv.h]=libicu-dev
    [/usr/include/enchant-2/enchant.h]=libenchant-2-dev
    [/usr/lib/x86_64-linux-gnu/cmake/Qt6/Qt6Config.cmake]=qt6-base-dev
    [/usr/lib/x86_64-linux-gnu/pkgconfig/mpv.pc]=libmpv-dev
    [/usr/lib/x86_64-linux-gnu/libblas.so.3]=libblas3
    [/usr/lib/x86_64-linux-gnu/liblapack.so.3]=liblapack3
    [/usr/lib/python3/dist-packages/gi/__init__.py]=python3-gi
    [/usr/lib/x86_64-linux-gnu/gdk-pixbuf-2.0/2.10.0/loaders/libpixbufloader-svg.so]=librsvg2-common
)

install_apt_tools() {
    local missing=()
    local cmd
    for cmd in "${!APT_TOOLS[@]}"; do
        if command -v "${cmd}" >/dev/null 2>&1; then
            skip "${cmd}"
        else
            missing+=("${APT_TOOLS[${cmd}]}")
        fi
    done

    local probe
    for probe in "${!APT_LIBS[@]}"; do
        if [[ -e "${probe}" ]]; then
            skip "${APT_LIBS[${probe}]}"
        else
            missing+=("${APT_LIBS[${probe}]}")
        fi
    done

    if (( ${#missing[@]} == 0 )); then
        info "APT packages: nothing to install"
        return
    fi

    info "installing the packages: ${missing[*]}"
    sudo apt-get update -qq
    sudo apt-get install -y "${missing[@]}"
}

# gh is not in the repositories of every Ubuntu version. The test is on the
# package's availability, not on the text of `apt-cache policy`, which is
# translated and therefore depends on the locale.
ensure_gh_repository() {
    command -v gh >/dev/null 2>&1 && return 0
    apt-cache show gh >/dev/null 2>&1 && return 0
    info "adding the GitHub CLI repository"
    sudo mkdir -p -m 755 /etc/apt/keyrings
    curl -fsSL https://cli.github.com/packages/githubcli-archive-keyring.gpg \
        | sudo tee /etc/apt/keyrings/githubcli-archive-keyring.gpg >/dev/null
    sudo chmod go+r /etc/apt/keyrings/githubcli-archive-keyring.gpg
    echo "deb [arch=$(dpkg --print-architecture) signed-by=/etc/apt/keyrings/githubcli-archive-keyring.gpg] https://cli.github.com/packages stable main" \
        | sudo tee /etc/apt/sources.list.d/github-cli.list >/dev/null
}

install_git_cliff() {
    if command -v git-cliff >/dev/null 2>&1; then
        skip "git-cliff"
        return
    fi
    info "installing git-cliff ${GIT_CLIFF_VERSION} into ${LOCAL_BIN}"
    mkdir -p "${LOCAL_BIN}"
    local archive="git-cliff-${GIT_CLIFF_VERSION}-x86_64-unknown-linux-gnu.tar.gz"
    local url="https://github.com/orhun/git-cliff/releases/download/v${GIT_CLIFF_VERSION}/${archive}"
    local tmp
    tmp="$(mktemp -d)"

    # Explicit cleanup rather than a `trap RETURN`: such a trap persists beyond
    # the function that sets it and fires again on the return of the following
    # ones, when the variable it refers to no longer exists.
    local status=0
    {
        curl -fsSL "${url}" -o "${tmp}/${archive}" &&
            tar -xzf "${tmp}/${archive}" -C "${tmp}" &&
            find "${tmp}" -type f -name git-cliff -perm -u+x \
                -exec install -m 755 {} "${LOCAL_BIN}/git-cliff" \;
    } || status=$?
    rm -rf "${tmp}"
    (( status == 0 )) || die "installing git-cliff failed (status ${status})"

    if ! command -v git-cliff >/dev/null 2>&1; then
        printf '  add %s to the PATH to use git-cliff\n' "${LOCAL_BIN}"
    fi
}

# Detects a misconfigured `c++` alternative, which points to gcc instead of
# g++: C++ then compiles, but does not link against libstdc++.
check_cxx_alternative() {
    local target
    target="$(readlink -f /usr/bin/c++ 2>/dev/null || true)"
    [[ "${target}" == *gcc* ]] || return 0

    info "anomaly detected: /usr/bin/c++ points to ${target}"
    printf '  gcc compiles C++ but does not link libstdc++; every build will fail.\n'
    printf '  fix: sudo update-alternatives --install /usr/bin/c++ c++ /usr/bin/g++ 100\n'
    if [[ -x /usr/bin/g++ ]]; then
        sudo update-alternatives --install /usr/bin/c++ c++ /usr/bin/g++ 100
        sudo update-alternatives --set c++ /usr/bin/g++
        printf '  fixed: c++ -> %s\n' "$(readlink -f /usr/bin/c++)"
    fi
}

report() {
    info "state of the toolchain"
    local cmd
    local path_warning=0
    for cmd in cmake ninja g++ clang-tidy-20 clang-format gcovr ccache git git-cliff gh jq msgfmt ffprobe pkg-config; do
        if command -v "${cmd}" >/dev/null 2>&1; then
            printf '  \033[32m✓\033[0m %-14s %s\n' "${cmd}" "$("${cmd}" --version 2>/dev/null | head -1)"
        elif [[ -x "${LOCAL_BIN}/${cmd}" ]]; then
            # Installed, but outside this terminal's PATH. On Ubuntu,
            # ~/.profile adds ~/.local/bin only at login, and only if the
            # directory already existed — which is not the case when this
            # script has just created it.
            printf '  \033[33m!\033[0m %-14s %s\n' "${cmd}" \
                "$("${LOCAL_BIN}/${cmd}" --version 2>/dev/null | head -1)"
            path_warning=1
        else
            printf '  \033[31m✗\033[0m %-14s missing\n' "${cmd}"
        fi
    done

    local probe
    for probe in "${!APT_LIBS[@]}"; do
        if [[ -e "${probe}" ]]; then
            printf '  \033[32m✓\033[0m %-14s %s\n' "${APT_LIBS[${probe}]}" "${probe}"
        else
            printf '  \033[31m✗\033[0m %-14s missing\n' "${APT_LIBS[${probe}]}"
        fi
    done

    if (( path_warning )); then
        printf '\n  \033[33m!\033[0m installed in %s, missing from the PATH of this terminal.\n' "${LOCAL_BIN}"
        printf '      effective at the next login, or right away with:\n'
        printf '        export PATH="%s:$PATH"\n' "${LOCAL_BIN}"
        printf '      the make targets work without it.\n'
    fi
}

# Writes the list of APT packages, without installing anything.
#
# It serves the CI cache, which needs to name what it restores. Having this
# script say it rather than the YAML is what keeps **a single list**: a second
# one, copied into a workflow, would diverge at the first package added, and the
# cache would then restore an incomplete toolchain with nothing to report it.
list_packages() {
    printf '%s\n' "${APT_TOOLS[@]}" "${APT_LIBS[@]}" | sort -u
}

main() {
    if [[ "${MODE}" == "--list-packages" ]]; then
        list_packages
        return
    fi

    # The pull request checks job builds nothing: of the whole chain, it needs
    # only git-cliff, to regenerate the changelog and compare. Installing cmake,
    # clang-tidy and gcovr for it would cost several minutes on every edit of a
    # pull request body. The mode lives here rather than in the YAML so that
    # git-cliff's version number stays written in one place.
    if [[ "${MODE}" == "--git-cliff-only" ]]; then
        install_git_cliff
        return
    fi

    check_cxx_alternative
    ensure_gh_repository
    install_apt_tools
    install_git_cliff
    "${REPO_ROOT}/src/scripts/install-hooks.sh"
    "${REPO_ROOT}/src/scripts/reference.sh" lock >/dev/null 2>&1 || true
    report
}

main "$@"
