#!/usr/bin/env bash
# Installs the `.rpm` on a real Fedora, and runs what comes out of it.
#
# **This is the check ADR 0023 named without being able to write it** — "to
# reopen when someone really installs the package", issue #266. Development
# happens on Ubuntu: `rpmbuild` produces an `.rpm` there and `rpm` installs
# nothing. Three rows of the ADR's table were therefore left empty, and the
# costliest of the three was this one — **that the dependency names exist in
# the distribution.** They are written by hand in `cmake/Packaging.cmake`,
# because CPack knows nothing about Fedora, and a wrong name produces a
# perfectly valid package that refuses to install.
#
# ## What it found on the first try
#
# **The package did not install**, and not for the reason being watched. The
# dependencies were right; the directories were not. The `.rpm` claimed to own
# `/usr/share/applications`, `/usr/share/icons` and six others, which
# `filesystem` and `hicolor-icon-theme` already own, and `dnf` refused the
# transaction on six conflicts. The exclusion had existed since #244 and
# excluded nothing — relative paths against package paths. See
# `cmake/Packaging.cmake`.
#
# **Nobody could see it without running the transaction.** `rpm -qlp` reads a
# list of files; an ownership conflict is not in the package, it is between
# the package and the distribution.
#
# ## What it checks
#
#   1. `dnf install` resolves the dependencies **and** carries the transaction
#      through;
#   2. every path the package announces is where it announces it;
#   3. the two installed binaries start and state the right version;
#   4. the two manual pages are found by `man`, which is the tool that reads
#      them.
#
# ## What it costs, and why it is in no gate
#
# It downloads Qt and its dependencies, nearly three hundred megabytes, and
# needs the network. **A gate that depends on the network is red on an outage
# day, for a reason foreign to the repository** — the rule `verify-gates.sh`
# already applies to itself. It is therefore run by hand, and once a week on
# `main`: `.github/workflows/fedora.yml`. The repository's Actions quota has
# already run out once (#232, #233), and a container per pull request is not
# free.
#
# **What guards pull requests in the meantime is cheaper and narrower:**
# `check-installation.sh` refuses an `.rpm` that claims to own a directory that
# is not its own. That is the cause of the defect above, and it can be read in
# the package without Fedora. The two checks do not see the same thing, which
# is why there are two of them.
#
# ## The container is not a Fedora desktop, on one point
#
# A Fedora image sets `tsflags=nodocs`, which drops documentation at install
# time — a desktop does not. The check undoes it explicitly, otherwise it would
# find the manual page missing from a package that carries it.

set -euo pipefail

readonly REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
# shellcheck source=src/scripts/lib/project.sh
source "$(dirname "${BASH_SOURCE[0]}")/lib/project.sh"

readonly RED=$'\033[31m'
readonly GREEN=$'\033[32m'
readonly RESET=$'\033[0m'

# **A named version, never `latest`.** An image that changes underfoot would
# make a failure impossible to replay, and would make this repository bear the
# accidents of a moving distribution.
image="fedora:42"
build_dir="${REPO_ROOT}/build/release"

usage() {
    cat >&2 <<'USAGE'
usage: check-rpm.sh [--build-dir <directory>] [--image <image>]

  --build-dir  the tree where the .rpm is found (default: build/release)
  --image      the Fedora image to run          (default: fedora:42)
USAGE
    exit 2
}

while (($# > 0)); do
    case "$1" in
    --build-dir) [[ $# -ge 2 ]] || usage; build_dir="$2"; shift 2 ;;
    --image) [[ $# -ge 2 ]] || usage; image="$2"; shift 2 ;;
    -h | --help) usage ;;
    *) printf 'unknown argument: %s\n' "$1" >&2; usage ;;
    esac
done

# **Podman first, Docker next.** Podman is what a Fedora ships with, and it
# runs without a daemon; Docker is what a development Ubuntu most often has.
# Both take the same arguments for what is asked of them here.
engine=""
for candidate in podman docker; do
    if command -v "${candidate}" >/dev/null 2>&1 && "${candidate}" info >/dev/null 2>&1; then
        engine="${candidate}"
        break
    fi
done

if [[ -z "${engine}" ]]; then
    printf '%s✗ neither podman nor docker is usable%s\n' "${RED}" "${RESET}" >&2
    printf '  this check installs the package on a real Fedora; it needs a container\n' >&2
    printf '  Ubuntu : sudo apt install podman\n' >&2
    exit 1
fi

readonly VERSION="$(project_version "${REPO_ROOT}/CMakeLists.txt")"

package="$(find "${build_dir}" -maxdepth 1 -name 'subedit*.rpm' 2>/dev/null | head -1)"

if [[ -z "${package}" ]]; then
    printf '%s✗ no .rpm in %s%s\n' "${RED}" "${build_dir}" "${RESET}" >&2
    printf '  produce it with "make packages"\n' >&2
    exit 1
fi

# The package is mounted alone, in a directory of its own: the container has
# no reason to see the repository, and what it does not see cannot distort what
# it tests. It is the same reason check-installation.sh runs the installed
# binaries from somewhere other than the build tree.
staging="$(mktemp -d)"
cleanup() {
    rm -rf "${staging}"
}
trap cleanup EXIT

cp "${package}" "${staging}/"

printf 'package: %s\n' "$(basename "${package}")"
printf 'image:   %s   (%s)\n\n' "${image}" "${engine}"

# The check itself, run in the container. Written here rather than in a file
# of its own: it has a single caller, and one more file would be read without
# this one — yet it means nothing alone.
#
# **`-i` and not only `--rm`**: without it, the container's standard input is
# connected to nothing, and the `bash -s` below reads an empty script. It then
# returns zero having done nothing, which is the worst kind of green — and that
# is exactly what happened on the first try.
#
# **`Z` on the mount**, for SELinux: without it, an unprivileged podman on
# Fedora does not read the mounted directory. Docker accepts it and ignores it
# where SELinux is absent, so it costs nothing to whoever does not need it.
#
# The installed languages, computed from src/po/LINGUAS: the container does not
# see the repository, so it is handed the list (#659).
languages="$(grep -v '^[[:space:]]*\(#\|$\)' "${REPO_ROOT}/src/po/LINGUAS" | tr '\n' ' ')"

"${engine}" run --rm -i \
    -v "${staging}:/pkg:ro,Z" \
    -e "EXPECTED=${VERSION}" \
    -e "LANGUAGES=${languages}" \
    "${image}" \
    /usr/bin/bash -s <<'INNER'
set -uo pipefail

RED=$'\033[31m'
GREEN=$'\033[32m'
RESET=$'\033[0m'

failures=0

report_failure() {
    printf '%s✗%s %s\n' "${RED}" "${RESET}" "$*" >&2
    failures=$((failures + 1))
}

report_success() {
    printf '%s✓%s %s\n' "${GREEN}" "${RESET}" "$*"
}

printf '  %s\n\n' "$(cat /etc/fedora-release)"

## 1. The package installs
#
# **`--setopt=tsflags=` undoes the image's `nodocs`**, which is not a Fedora
# setting but a container setting: without it, the manual page would be dropped
# at install time and check 4 would fail on a flawless package.
#
# The output is kept and written only on failure: it is four hundred lines of
# downloads, and what matters in it fits in six.
if ! dnf -y --setopt=tsflags= install /pkg/*.rpm > /tmp/dnf.log 2>&1; then
    report_failure "dnf refuses to install the package:
$(grep -vE '^\[[0-9]+/[0-9]+\]|^ *$' /tmp/dnf.log | tail -20 | sed 's/^/    /')"
    printf '\n%s%d check(s) failed%s\n' "${RED}" "${failures}" "${RESET}" >&2
    exit 1
fi

installed="$(rpm -q --qf '%{VERSION}' subedit)"
if [[ "${installed}" != "${EXPECTED}" ]]; then
    report_failure "the installed package announces \"${installed}\", expected \"${EXPECTED}\""
else
    report_success "dnf installs the package, and resolves $(rpm -qp --requires /pkg/*.rpm 2>/dev/null \
        | grep -vc '^rpmlib(') dependency(ies)"
fi

## 2. Every announced path is where it is announced
#
# **The list comes from the package, never from here.** A list written in this
# script would go stale with the first added file, and the gap would only show
# if someone thought of adding it.
missing=()
while IFS= read -r path; do
    [[ -e "${path}" ]] || missing+=("${path}")
done < <(rpm -ql subedit)

if ((${#missing[@]} > 0)); then
    report_failure "the package announces paths it did not lay down:
$(printf '    %s\n' "${missing[@]}")"
else
    report_success "the $(rpm -ql subedit | wc -l) announced paths are where the package announces them"
fi

## 3. The two installed binaries start
#
# From the root, and not from a directory that might contain something: a
# binary that finds a file "next to it" must still find it when started from
# elsewhere. `XDG_CONFIG_HOME` is moved as everywhere else in this repository —
# a check does not touch the configuration of whoever runs it.
check_binary() {
    local name="$1"
    local said

    said="$(cd / && QT_QPA_PLATFORM=offscreen XDG_CONFIG_HOME=/tmp/config \
        "/usr/bin/${name}" --version 2>&1)" || {
        report_failure "installed ${name} does not start:
$(printf '%s\n' "${said}" | sed 's/^/    /')"
        return
    }

    # The first line alone: the libraries and languages follow it, and they are the machine's (#716).
    said="${said%%$'\n'*}"
    if [[ "${said}" != "subedit ${EXPECTED}" ]]; then
        report_failure "${name} announces \"${said}\", expected \"subedit ${EXPECTED}\""
        return
    fi

    report_success "${name} starts from the installation, and announces ${EXPECTED}"
}

check_binary subedit-cli
check_binary subedit-gui

## 4. The manual page is found by man
#
# **By `man` and not by a `test -f`**: what is at stake is not that a file
# exists, it is that a user who types `man subedit-cli` gets something. The
# directory may be right and outside the `manpath`, and nothing else would say
# so — that is the lesson of #268, check with the tool that matters.
#
# `util-linux` along with `man-db`, and it is not superfluous: `man` formats
# through `col -b`, which the minimal image lacks — `util-linux-core` is not
# enough, `col` is in `util-linux`. Without it, `man` exits with error 127 and
# renders nothing, a failure that looks like a missing page and is not one.
if ! dnf -y --setopt=tsflags= install man-db util-linux > /tmp/man.log 2>&1; then
    report_failure "man-db does not install in the container; the manual page is not tested"
else
    # **Both binaries, since #272.** A page installed under the wrong name is
    # found by its path and never by `man`; that is precisely what this loop
    # tests, and a `test -f` would not tell.
    pages=0
    for name in subedit-cli subedit-gui; do
        found="$(man -w "${name}" 2>/dev/null || true)"
        if [[ -z "${found}" ]]; then
            report_failure "man does not find ${name}; the page is outside the manpath"
        elif ! man "${name}" 2>/dev/null | grep -q "${name}"; then
            report_failure "man finds the page of ${name} but renders nothing"
        else
            pages=$((pages + 1))
        fi
    done
    ((pages == 2)) && report_success "man renders both installed pages"
fi

## 5. The full manual is readable
#
# The path is the one the manual page announces, not one written here: it is
# what the help window will open on this machine.
named="$(man subedit-cli 2>/dev/null | grep -o '/usr/share/subedit/manual' | head -1)"
if [[ -z "${named}" ]]; then
    report_failure "the manual page does not name the manual's directory"
elif [[ ! -r "${named}/index.md" ]]; then
    report_failure "the manual announced under ${named} is not readable there"
else
    report_success "the manual is readable under ${named} ($(find "${named}" -type f | wc -l) files)"
fi

## 6. The catalogue of every installed language is laid down
#
# The package already announces them (check 2); this one compares with the repository's list,
# because a package without a catalogue announces itself complete. The path is the one the reader
# of `core/i18n/` looks for.
for language in ${LANGUAGES}; do
    mo="/usr/share/subedit/locale/${language}/LC_MESSAGES/subedit.mo"
    if [[ ! -s "${mo}" ]]; then
        report_failure "the \"${language}\" catalogue is not laid down: ${mo}"
    else
        report_success "the \"${language}\" catalogue is laid down under ${mo%/LC_MESSAGES/*}"
    fi
done

if ((failures > 0)); then
    printf '\n%s%d check(s) failed%s\n' "${RED}" "${failures}" "${RESET}" >&2
    exit 1
fi

printf '\n%s✓%s the package installs on Fedora, and what it lays down works\n' \
    "${GREEN}" "${RESET}"
INNER
