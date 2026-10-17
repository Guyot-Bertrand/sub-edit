#!/usr/bin/env bash
# Installs into a temporary prefix, then runs what was just placed there.
#
# **The scope of phase 7 is "what a third party needs to install and use the
# tool", and no unit test says that.** Everything else the project tests runs
# from the build tree, where files are present by an accident of layout: they
# are there because the repository contains them, not because a rule copied
# them.
#
# The most common mistake of a first packaging is exactly that one — a data
# file the binary looks for next to itself and that nobody copied. It is
# invisible from the build tree and only shows at the first clean install.
# This is that install.
#
# ## What it checks
#
#   1. the two binaries install, start, and report the right version;
#   2. **every page of the repository's manual ends up under the prefix** — the
#      expected list is computed from `docs/manual`, never copied: a list
#      written by hand goes stale at the first added chapter, silently;
#      **the correction patterns end up under `share/subedit/patterns`** — same
#      rule, same computation: without them the program no longer corrects
#      anything and nobody notices — #498;
#   3. the temporary prefix leaves nothing behind;
#   4. **the five desktop files are there, and three of them validate** — the
#      `.desktop`, the AppStream metadata and the icon, the latter read by the
#      loader of GTK desktops rather than by just any loader;
#   5. **the two manual pages do not lie, and render** — one per binary since
#      #272: their version is the binary's, the manual directory they name is
#      the one the manual was installed in, each carries the name of the binary
#      it describes, the subcommands the command-line page lists are the ones
#      the binary lists, and groff renders them without a single warning;
#   6. **a staged install touches nothing outside** — non-empty `DESTDIR`,
#      prefix `/usr`, and nothing written outside the staging area;
#   7. **the two native packages build, and say the same thing** — a `.deb`
#      and an `.rpm`, whose file lists match and whose dependencies are
#      declared;
#   8. **the `.rpm` owns only its own directories** — the distribution's are
#      not its own, and `dnf` refuses the whole transaction if it claims them.
#
# `--list` lists them, `--only` runs a single one.
#
# ## What it does not check, and where that is checked
#
# **That the `.rpm` installs.** An `.rpm` built on Ubuntu cannot be installed
# there: testing it takes a Fedora. One exists since #266, in a container —
# `src/scripts/check-rpm.sh`, once a week — and it is not here because it needs
# the network, which a gate must not need.
#
# | Checked here | Checked by check-rpm.sh |
# | :----------- | :---------------------- |
# | the file list, through `rpm -qlp` | that the `dnf` transaction succeeds |
# | the declared dependencies, through `rpm -qp --requires` | that those names exist on Fedora |
# | the agreement of the two packages | that the installed binaries start |
# | that the `.rpm` owns no shared directory | that `man` finds the installed page |
#
# **The last line on the left is the cheap half of the right.** The `.rpm`
# claimed eight directories of the distribution and `dnf` refused the whole
# transaction; the cause can be read in the package, without Fedora, and that
# is what the local check now does on every pull request.
#
# **The agreement of the two lists is the most useful of the three**, and the
# only one that needs no Fedora: both packages come out of the same install, so
# a difference between their lists is a defect of the `install()` rules. That
# is what tests the half of ADR 0023 that says "the install first".
#
# **That the `.deb` installs** is not checked either, for a different reason:
# `dpkg -i` needs administrator rights, which a quality gate does not have and
# must not ask for. What is checked is what the package contains and what it
# declares, plus the fact that the same binaries, installed by
# `cmake --install`, start.
#
# ## One thing or two? — issue #271, settled by measurement
#
# **The question was fair, and its premise was wrong.** This script went from
# three checks to twelve and from a hundred and forty lines to six hundred; it
# builds two native packages, and "two runs of CPack" sounded like something
# one pays for. Hence the proposal: move the packages to a target of their own,
# which `check-local` would chain or not.
#
# Timed, at load 0.9, three consistent runs:
#
# | Check | Cost |
# | :---- | ---: |
# | `cmake --install` into the prefix | 20 ms |
# | the two binaries start | 130 ms |
# | the installed manual | 5 ms |
# | the desktop files, their three validations | 145 ms |
# | the two manual pages | 19 ms |
# | their rendering by groff | 44 ms |
# | the install under `DESTDIR` | 23 ms |
# | **the two native packages, CPack included** | **650 ms** |
# | **everything** | **≈ 1.0 s** |
#
# `make install-check`, with a warm release tree, costs 1.6 s end to end.
# **Moving the packages out would save six hundred and fifty milliseconds on a
# `make check-local` that lasts minutes.** The question of cost is settled:
# there is none.
#
# **The question of substance remains, and the answer is "one thing".** What
# this script asserts is not "the install is correct" but "what a third party
# installs works" — and a native package is *how* a third party installs.
# Splitting it would create exactly the danger the issue names: a half-checked
# install is worse than an install known to be unchecked.
#
# **What was missing was not a second target, it was being able to resume in
# the middle** — the defect #269 fixed for the gate, and that was paid here by
# replaying the twelve checks to fix one. Hence `--only`, `--list`, and the
# guard that comes with them: an unknown name makes the invocation fail rather
# than run none. Without arguments, everything runs; that is the entry point
# for whoever does not want to choose.

set -euo pipefail

readonly REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
readonly MANUAL_DIR="${REPO_ROOT}/docs/manual"
readonly PATTERNS_DIR="${REPO_ROOT}/packaging/patterns"
readonly LINGUAS="${REPO_ROOT}/src/po/LINGUAS"

# The application identifier, the one the three desktop files carry. Written
# here as in `cmake/Installation.cmake`: this check exists to compare what the
# rules install with what is expected, so it must state it on its side rather
# than read it from theirs.
readonly APP_ID="io.github.guyot_bertrand.subedit"

# The tree this project creates, and the only one a package may own. Written
# here as `APP_ID` is, and for the same reason: this check compares what the
# rules install with what is expected.
readonly DATA_SUBDIR="share/subedit"

# The shipped binaries, and their manual pages — one per binary since #272.
# Written here as `APP_ID` is: this check compares what the rules install with
# what is expected, so it states it on its side.
readonly BINARIES=(subedit-cli subedit-gui)

readonly RED=$'\033[31m'
readonly GREEN=$'\033[32m'
readonly RESET=$'\033[0m'

build_dir="${REPO_ROOT}/build/release"

# **The order of the checks, and the only place it is written.** Each one runs
# alone, knowing nothing of the others: the install into the prefix is done
# once for all, before them, and none depends on what another left behind.
# That is what makes `--only` safe.
readonly CONTROLS=(binaries manual patterns catalogues desktop pages render destdir packages)

usage() {
    cat >&2 <<'USAGE'
usage: check-installation.sh [--build-dir <directory>] [--only <check>]
       check-installation.sh --list

  --build-dir  the build tree to install (default: build/release)
  --only       runs only this check, repeatable
  --list       prints the checks, in order

Without --only, everything runs: that is the entry point for whoever does not
want to choose, and a half-checked install is worse than an install known to
be unchecked.
USAGE
    exit 2
}

only=()

while (( $# > 0 )); do
    case "$1" in
        --build-dir) [[ $# -ge 2 ]] || usage; build_dir="$2"; shift 2 ;;
        --only)      [[ $# -ge 2 ]] || usage; only+=("$2"); shift 2 ;;
        --list)      printf '%s\n' "${CONTROLS[@]}"; exit 0 ;;
        -h|--help)   usage ;;
        *)           printf 'unknown argument: %s\n' "$1" >&2; usage ;;
    esac
done

# **Every name given must designate a check**, and that is checked before a
# single one runs. It is the guard of `gate.sh`, for the same reason: without
# it, `--only pakages` would run zero checks and return zero — an install
# declared correct that nobody looked at.
for named in ${only+"${only[@]}"}; do
    if ! printf '%s\n' "${CONTROLS[@]}" | grep -qxF "${named}"; then
        printf '%s✗ "%s" is not a check of this script%s\n' \
            "${RED}" "${named}" "${RESET}" >&2
        printf '  the checks are:\n' >&2
        printf '    %s\n' "${CONTROLS[@]}" >&2
        exit 2
    fi
done

# True if this check must run: all of them, or the ones named.
wanted() {
    (( ${#only[@]} == 0 )) && return 0
    printf '%s\n' "${only[@]}" | grep -qxF "$1"
}

failures=0

report_failure() {
    printf '%s✗%s %s\n' "${RED}" "${RESET}" "$*" >&2
    failures=$((failures + 1))
}

report_success() {
    printf '%s✓%s %s\n' "${GREEN}" "${RESET}" "$*"
}

if [[ ! -d "${build_dir}" ]]; then
    printf '%s✗ build tree missing: %s%s\n' "${RED}" "${build_dir}" "${RESET}" >&2
    printf '  build it with "make release"\n' >&2
    exit 1
fi

# The prefix AND the configuration directory of the binaries that are run. The
# latter for the project's rule: a binary a check runs never touches the
# configuration of whoever runs it — see check-config-home.sh.
prefix="$(mktemp -d)"
cleanup() {
    rm -rf "${prefix}"
}
trap cleanup EXIT

cmake --install "${build_dir}" --prefix "${prefix}" >/dev/null

# ## The installed binaries start, and report the right version
#
# Run from a directory that is neither the repository nor the build tree: that
# is half of the check. A binary that finds a file "next to itself" still finds
# it when run from the repository.
readonly VERSION="$(sed -n 's/^[[:space:]]*VERSION[[:space:]]\{1,\}\([0-9.]\{1,\}\)[[:space:]]*$/\1/p' \
    "${REPO_ROOT}/CMakeLists.txt" | head -1)"

check_binary() {
    local name="$1"
    local binary="${prefix}/bin/${name}"

    if [[ ! -x "${binary}" ]]; then
        report_failure "${name} was not installed in bin/"
        return
    fi

    local said
    # LANGUAGE=en: what is compared is the English the binary speaks, whatever the
    # machine's locale and whatever catalogue is installed beside it (spec 15, D4).
    said="$(cd "${prefix}" && QT_QPA_PLATFORM=offscreen XDG_CONFIG_HOME="${prefix}/config" LANGUAGE=en \
        "${binary}" --version 2>&1)" || {
        report_failure "the installed ${name} does not start"
        return
    }

    if [[ "${said}" != "subedit ${VERSION}" ]]; then
        report_failure "${name} reports \"${said}\", expected \"subedit ${VERSION}\""
        return
    fi

    report_success "${name} installs, starts, and reports ${VERSION}"
}

if wanted binaries; then
    check_binary subedit-cli
    check_binary subedit-gui
fi

# The languages `src/po/LINGUAS` installs, one per line — read by the manual's check and the
# catalogues'.
installed_languages() {
    grep -v '^[[:space:]]*\(#\|$\)' "${LINGUAS}" || true
}

# ## Every page of the manual ends up under the prefix
#
# **The expected list is computed, never copied.** A list written by hand goes
# stale at the first added chapter, and the gap would show only if someone
# thought of adding it — that is, never. The screenshots count as much as the
# pages: a missing image is a page with a hole in it.
#
# **And every installed language's tree, under a directory of its name** — spec 15, D9, issue
# #682: `docs/i18n/<language>/manual` of each language `src/po/LINGUAS` names, under
# `share/subedit/manual/<language>`, which `Help ▸ Manual` reads before English. A language with
# no translated manual has nothing to check.
check_manual_tree() {
    local source="$1" installed="$2" rule="$3"

    if [[ ! -d "${installed}" ]]; then
        report_failure "the manual was not installed in ${installed#"${prefix}/"}"
        return
    fi

    local missing=()
    local file
    while IFS= read -r -d '' file; do
        local relative="${file#"${source}/"}"
        [[ -f "${installed}/${relative}" ]] || missing+=("${relative}")
    done < <(find "${source}" -type f -print0)

    if (( ${#missing[@]} > 0 )); then
        report_failure "the manual installed in ${installed#"${prefix}/"} is incomplete:
$(printf '    %s\n' "${missing[@]}")
    the install() rules of ${rule} do not copy everything"
        return
    fi

    local count
    count="$(find "${source}" -type f | wc -l)"
    report_success "the ${count} files of the manual are installed in ${installed#"${prefix}/"}"
}

check_manual() {
    check_manual_tree "${MANUAL_DIR}" "${prefix}/share/subedit/manual" cmake/Installation.cmake

    local language
    while IFS= read -r language; do
        local translated="${REPO_ROOT}/docs/i18n/${language}/manual"
        [[ -d "${translated}" ]] || continue
        check_manual_tree "${translated}" "${prefix}/share/subedit/manual/${language}" \
            cmake/Translations.cmake
    done < <(installed_languages)
}

if wanted manual; then
    check_manual
fi

# ## Every pattern file ends up under the prefix
#
# **Same rule as the manual, and for the same reason: the list is computed.**
# A missing pattern file is not an error the program reports, it is a language
# for which nothing is corrected any more — and nobody notices. The location is
# the one the program reads relative to its executable, `share/subedit/patterns`
# (ADR 0037).
check_patterns() {
    local installed="${prefix}/share/subedit/patterns"

    if [[ ! -d "${installed}" ]]; then
        report_failure "the correction patterns were not installed in share/subedit/patterns"
        return
    fi

    local missing=()
    local file
    while IFS= read -r -d '' file; do
        local relative="${file#"${PATTERNS_DIR}/"}"
        [[ -f "${installed}/${relative}" ]] || missing+=("${relative}")
    done < <(find "${PATTERNS_DIR}" -type f -print0)

    if (( ${#missing[@]} > 0 )); then
        report_failure "the installed patterns are incomplete:
$(printf '    %s\n' "${missing[@]}")
    the install() rule of cmake/Installation.cmake does not copy everything"
        return
    fi

    local count
    count="$(find "${PATTERNS_DIR}" -type f | wc -l)"
    report_success "the ${count} pattern files are installed"
}

if wanted patterns; then
    check_patterns
fi

# ## The catalogue of every installed language ends up under the prefix
#
# **The list is computed, from `src/po/LINGUAS`** and not written here, for the reason of the
# manual and the patterns. A missing catalogue is not an error the program reports: the interface
# falls back to English without a word, and nobody sees it until a user complains. The location
# is the one the reader of `core/i18n/` looks in (issue #659); the file must also be a `.mo` — an
# empty file installed in the right place is the defect that a presence check does not see.
check_catalogues() {
    local root="${prefix}/share/subedit/locale"
    local language checked=0

    while IFS= read -r language; do
        local mo="${root}/${language}/LC_MESSAGES/subedit.mo"
        if [[ ! -f "${mo}" ]]; then
            report_failure "the \"${language}\" catalogue was not installed: ${mo#"${prefix}/"} is missing
    the install() rule of cmake/Translations.cmake does not install it"
            continue
        fi
        # The magic number of a `.mo`, in either byte order.
        local magic
        magic="$(od -An -tx1 -N4 "${mo}" | tr -d ' \n')"
        if [[ "${magic}" != "de120495" && "${magic}" != "950412de" ]]; then
            report_failure "${mo#"${prefix}/"} is not a compiled catalogue (magic number \"${magic}\")"
            continue
        fi
        checked=$((checked + 1))
    done < <(installed_languages)

    # **And no other language**: the seeds imported from Gaupol live in src/po/ beside the shipped
    # catalogues and must not be installed until they are complete and reviewed (spec 15, D5, #667).
    local directory
    for directory in "${root}"/*/; do
        [[ -d "${directory}" ]] || continue
        language="$(basename "${directory}")"
        if ! installed_languages | grep -qxF "${language}"; then
            report_failure "a catalogue is installed for \"${language}\", which src/po/LINGUAS does not name
    a seed ships only once it is complete and reviewed (spec 15, D5)"
        fi
    done

    if (( checked > 0 )) && (( failures == 0 )); then
        report_success "the ${checked} language catalogue(s) are installed under share/subedit/locale"
    fi
}

if wanted catalogues; then
    check_catalogues
fi

# ## The four desktop files
#
# **Two of them validate, and the other two are validated only by their
# presence.** A malformed SVG would show to the eye and nowhere else; a manual
# page is reread further down, where it is confronted with the binary.
check_desktop_files() {
    local desktop="${prefix}/share/applications/${APP_ID}.desktop"
    local metainfo="${prefix}/share/metainfo/${APP_ID}.metainfo.xml"
    local icon="${prefix}/share/icons/hicolor/scalable/apps/${APP_ID}.svg"

    local expected=("${desktop}" "${metainfo}" "${icon}")
    local name
    for name in "${BINARIES[@]}"; do
        expected+=("${prefix}/share/man/man1/${name}.1.gz")
    done

    local file
    for file in "${expected[@]}"; do
        [[ -f "${file}" ]] || report_failure "missing from the install: ${file#"${prefix}/"}"
    done

    if [[ -f "${desktop}" ]]; then
        if desktop-file-validate "${desktop}" >/dev/null 2>&1; then
            report_success "the .desktop file is valid"
        else
            report_failure "desktop-file-validate rejects the installed file:
$(desktop-file-validate "${desktop}" 2>&1 | sed 's/^/    /')"
        fi
    fi

    if [[ -f "${metainfo}" ]]; then
        if appstreamcli validate --no-net "${metainfo}" >/dev/null 2>&1; then
            report_success "the AppStream metadata is valid"
        else
            report_failure "appstreamcli rejects the installed metadata:
$(appstreamcli validate --no-net "${metainfo}" 2>&1 | sed 's/^/    /')"
        fi
    fi

    # **The icon is read, and by the loader of GTK desktops** — #260. It was the
    # only shipped file whose content nothing looked at, and it was broken: a
    # comment before the root element made it unrecognisable to gdk-pixbuf,
    # which showed an empty tile. Inkscape and Qt opened it without a word;
    # that is why the check goes through that loader and not another.
    if [[ -f "${icon}" ]]; then
        if "${REPO_ROOT}/src/scripts/check-icon.py" "${icon}" >/dev/null 2>&1; then
            report_success "the icon loads, and shows on both backgrounds"
        else
            report_failure "the installed icon does not hold:
$("${REPO_ROOT}/src/scripts/check-icon.py" "${icon}" 2>&1 | sed 's/^/    /')"
        fi
    fi
}

if wanted desktop; then
    check_desktop_files
fi

# ## The manual pages do not lie
#
# **Two pages since #272, one per binary.** What holds for both is checked for
# both; what holds only for the command line — the list of subcommands — stays
# with it.
#
# Each assertion is of the kind a manual page carries for years without anyone
# rereading it.
check_man_pages() {
    local name
    for name in "${BINARIES[@]}"; do
        check_one_man_page "${name}"
    done
}

check_one_man_page() {
    local name="$1"
    local compressed="${prefix}/share/man/man1/${name}.1.gz"
    [[ -f "${compressed}" ]] || return

    # Read decompressed, as `man` reads it. In a variable and not in a file:
    # the function exits through several paths, and one more temporary file
    # would be one more to delete on each of them.
    local page
    page="$(gzip -dc "${compressed}")"

    local announced
    announced="$(sed -n 's/^\.TH .* "subedit \([0-9.]*\)".*/\1/p' <<< "${page}")"
    if [[ "${announced}" != "${VERSION}" ]]; then
        report_failure "${name}.1 reports \"${announced}\", expected \"${VERSION}\""
        return
    fi

    # The directory it names is the one the manual was actually installed in.
    # That is the defect the first `.deb` produced had: the page, generated at
    # configure time, announced /usr/local in a package that installs under
    # /usr.
    local named
    named="$(sed -n 's/^\.I \(.*subedit\/manual\)$/\1/p' <<< "${page}")"
    if [[ "${named}" != "${prefix}/share/subedit/manual" ]]; then
        report_failure "${name}.1 names \"${named}\", the manual is under \"${prefix}/share/subedit/manual\""
        return
    fi

    # **The name it carries is the one of the binary it describes.** Two pages
    # generated by the same loop from two templates: a `.TH` line copied from
    # one template to the other would give two pages with the same name, one of
    # which `man` would never find.
    local titled
    titled="$(sed -n 's/^\.TH \([A-Z-]*\) .*/\1/p' <<< "${page}" | tr 'A-Z' 'a-z')"
    if [[ "${titled}" != "${name}" ]]; then
        report_failure "${name}.1 is titled \"${titled}\", expected \"${name}\""
        return
    fi

    # **The subcommands it lists are the binary's.** A subcommand added without
    # being written here would give a page that forgets one, and nothing else
    # would say so. Only the command-line page has them — the window has none.
    if ! grep -q '^\.SH SUBCOMMANDS' <<< "${page}"; then
        report_success "${name}.1 states the right version, the right manual, and carries its name"
        return
    fi

    local written listed
    written="$(sed -n '/^\.SH SUBCOMMANDS/,/^\.SH [A-Z]/p' <<< "${page}" \
        | sed -n 's/^\.B \([a-z][a-z-]*\)$/\1/p' | sort -u)"
    # LANGUAGE=en: the heading read is the English one (spec 15, D4).
    listed="$(cd "${prefix}" && QT_QPA_PLATFORM=offscreen XDG_CONFIG_HOME="${prefix}/config" LANGUAGE=en \
        "${prefix}/bin/${name}" --help 2>&1 \
        | awk '/^Subcommands:/ { seen = 1; next } seen && NF { print $1 }' | sort -u)"

    if [[ "${written}" != "${listed}" ]]; then
        report_failure "${name}.1 and the binary do not name the same subcommands:
$(diff <(printf '%s\n' "${written}") <(printf '%s\n' "${listed}") | sed 's/^/    /')"
        return
    fi

    report_success "${name}.1 states the version, the manual and the $(printf '%s\n' "${listed}" | wc -l) subcommands of the binary"
}

if wanted pages; then
    check_man_pages
fi

# ## The manual pages are rendered by the tool that renders them
#
# **The checks above read the pages with `sed`; none renders them.** That is
# exactly the defect #260 paid for on the icon, and #268 looked for it
# everywhere else: a check that goes through a tool that is not the one that
# matters. A manual page is read with `man`, that is with groff, and nobody had
# ever handed it over.
#
# It passed — with two warnings, on two UTF-8 characters of a comment line. Of
# no consequence for a reader; that was not the question. The question was
# that no check would have said so.
#
# **`-z` and not a rendering**: we want the diagnostics, not the page. **`-ww`
# and not the default silence**: groff keeps quiet about almost everything, and
# a check that asks nothing learns nothing. **Without `-k`**: preconv would make
# the UTF-8 readable and hide precisely what we are looking for. `man` calls it
# here, does not call it everywhere, and a page that holds without it holds
# with it.
#
# **The exit code says nothing** — groff warns and returns 0. So it is the
# error output that is read, and the slightest byte is a failure.
check_man_pages_render() {
    if ! command -v groff >/dev/null 2>&1; then
        report_failure "groff is missing: the manual pages cannot be rendered
    install it with \"./src/scripts/setup-toolchain.sh\""
        return
    fi

    local name rendered=0
    for name in "${BINARIES[@]}"; do
        local compressed="${prefix}/share/man/man1/${name}.1.gz"
        [[ -f "${compressed}" ]] || continue

        local complaints
        complaints="$(gzip -dc "${compressed}" | groff -ww -z -man - 2>&1 >/dev/null)"

        if [[ -n "${complaints}" ]]; then
            report_failure "groff complains about ${name}.1:
$(sed 's/^/    /' <<< "${complaints}")
    the pages are written in ASCII from end to end, comments included —
    see the header of packaging/${name}.1.in"
            return
        fi

        rendered=$((rendered + 1))
    done

    report_success "the ${rendered} manual pages render without a single groff warning"
}

if wanted render; then
    check_man_pages_render
fi

# ## A staged install touches nothing outside
#
# **That is what a packager does**, and it is the only way to discover that a
# rule writes to an absolute path: an install under `DESTDIR` that tried to
# write into `/usr` would fail here, for lack of rights, rather than succeed
# for someone who has them.
check_staged_install() {
    local staged
    staged="$(mktemp -d)"

    if ! DESTDIR="${staged}" cmake --install "${build_dir}" --prefix /usr >/dev/null 2>&1; then
        report_failure "an install with DESTDIR fails — a rule writes outside the staging area"
        rm -rf "${staged}"
        return
    fi

    local outside
    outside="$(find "${staged}" -mindepth 1 -maxdepth 1 ! -name usr -printf '%f\n')"
    if [[ -n "${outside}" ]]; then
        report_failure "an install with DESTDIR places something other than the prefix:
$(printf '    %s\n' ${outside})"
        rm -rf "${staged}"
        return
    fi

    if [[ ! -x "${staged}/usr/bin/subedit-cli" ]]; then
        report_failure "an install with DESTDIR does not place the binaries under the prefix"
        rm -rf "${staged}"
        return
    fi

    report_success "an install with DESTDIR touches nothing outside"
    rm -rf "${staged}"
}

if wanted destdir; then
    check_staged_install
fi

# ## The two native packages
#
# **Their file lists must match**, and that is the check that tests the half of
# ADR 0023: both come out of the same install, so a difference between them is
# a defect of the `install()` rules and not of the format.
#
# **Only two things are left out of the comparison, and each is a property of
# the format and not of what is installed.** Leaving them out is what makes the
# check usable; leaving them out *by name* is what keeps it from hiding a real
# defect.
#
# | Left out | Why |
# | :------- | :-- |
# | directories | a `.deb` owns none, an `.rpm` owns those its distribution does not already own |
# | `/usr/lib/.build-id/…` | `rpmbuild` places these links for its own debugging tools; nothing installs them |
#
# **The compression of the manual page is not in it**, and that is deliberate:
# `rpmbuild` compresses its own, the Debian policy requires it too, so the
# install compresses it itself and both packages carry the same
# `subedit-cli.1.gz`. A difference that could be removed was better than one
# more difference to excuse here.
check_packages() {
    local built=("${build_dir}"/subedit[-_]*.deb "${build_dir}"/subedit[-_]*.rpm)
    rm -f "${built[@]}" 2>/dev/null || true

    if ! (cd "${build_dir}" && cpack -G "DEB;RPM" >/dev/null 2>&1); then
        report_failure "cpack does not produce the two packages
    (cd ${build_dir} && cpack -G 'DEB;RPM') to see why"
        return
    fi

    local deb rpm
    deb="$(find "${build_dir}" -maxdepth 1 -name 'subedit*.deb' | head -1)"
    rpm="$(find "${build_dir}" -maxdepth 1 -name 'subedit*.rpm' | head -1)"

    if [[ -z "${deb}" || -z "${rpm}" ]]; then
        report_failure "cpack produced only one package out of two (deb: \"${deb}\", rpm: \"${rpm}\")"
        return
    fi

    # The `.deb` marks its directories with a trailing slash; `rpm -qlp` does
    # not tell them apart, but `--dump` gives the mode of each entry, and a
    # directory is recognised there by its `04…`. So the type is read rather
    # than guessed from the name.
    local in_deb in_rpm
    in_deb="$(dpkg-deb -c "${deb}" \
        | awk '$6 !~ /\/$/ { print substr($6, 2) }' \
        | grep -v '^/usr/lib/\.build-id/' | sort)"
    in_rpm="$(rpm -qp --dump "${rpm}" 2>/dev/null \
        | awk '$5 !~ /^04/ { print $1 }' \
        | grep -v '^/usr/lib/\.build-id/' | sort)"

    if [[ "${in_deb}" != "${in_rpm}" ]]; then
        report_failure "the two packages do not carry the same files:
$(diff <(printf '%s\n' "${in_deb}") <(printf '%s\n' "${in_rpm}") | sed 's/^/    /')
    both come out of the same install; a difference is a defect of the install() rules"
        return
    fi

    report_success "the .deb and the .rpm carry the same $(printf '%s\n' "${in_deb}" | wc -l) files"

    # **The catalogues of the installed languages, by name** (#659): the comparison above only
    # sees a difference between the two packages, and two packages without a catalogue match.
    local language
    while IFS= read -r language; do
        local wanted_path="/usr/share/subedit/locale/${language}/LC_MESSAGES/subedit.mo"
        if ! grep -qxF "${wanted_path}" <<<"${in_deb}"; then
            report_failure "the .deb does not carry the \"${language}\" catalogue (${wanted_path})"
        fi
        if ! grep -qxF "${wanted_path}" <<<"${in_rpm}"; then
            report_failure "the .rpm does not carry the \"${language}\" catalogue (${wanted_path})"
        fi
    done < <(installed_languages)

    # **The dependencies, for their part, do not match and must not**: the same
    # libraries have different names on Debian and on Fedora. What is checked
    # is that neither list is empty — a package with no declared dependency
    # installs and does not start.
    local deb_deps rpm_deps
    deb_deps="$(dpkg-deb -f "${deb}" Depends)"
    rpm_deps="$(rpm -qp --requires "${rpm}" 2>/dev/null | grep -v '^rpmlib(' || true)"

    [[ -n "${deb_deps}" ]] || report_failure "the .deb declares no dependency"
    [[ -n "${rpm_deps}" ]] || report_failure "the .rpm declares no dependency"

    if [[ -n "${deb_deps}" && -n "${rpm_deps}" ]]; then
        report_success "both packages declare their dependencies, each in its family's names"
    fi

    # **Enchant, by name** — D6 of docs/specs/12-correction.md, #507. The
    # previous check only sees a non-empty list: it would let through a package
    # that does not pull in the spell checker's library, and that would install
    # without being able to start. The two families do not name it the same.
    if [[ "${deb_deps}" == *libenchant-2-2* ]]; then
        report_success "the .deb pulls in Enchant (libenchant-2-2)"
    else
        report_failure "the .deb does not pull in Enchant: libenchant-2-2 is missing from its dependencies"
    fi
    if grep -q '^enchant2\b' <<<"${rpm_deps}"; then
        report_success "the .rpm pulls in Enchant (enchant2)"
    else
        report_failure "the .rpm does not pull in Enchant: enchant2 is missing from its dependencies"
    fi

    check_rpm_directories "${rpm}"
}

# ## The `.rpm` owns only its own directories
#
# **The cheapest check in this file, and it caught the costliest defect** —
# issue #266. An `.rpm` that declares it owns `/usr/share/icons` conflicts with
# `hicolor-icon-theme`, which really owns it, and `dnf` refuses the whole
# transaction. The package stays perfectly valid: it can be inspected, it can
# be listed, and it does not install.
#
# `cmake/Packaging.cmake` already excluded these directories, as relative
# paths whereas CPack expects absolute ones — the exclusion never excluded
# anything, from the day it was written until `check-rpm.sh` ran a real
# transaction on Fedora. **An exclusion that excludes nothing looks in every
# respect like an exclusion that works**, and that is why this check exists:
# it looks at the result, never at the intention.
#
# **The rule is this: the package's only directories are under
# `share/subedit`.** That is the only tree this project creates; all the others
# where it places something — `bin`, `applications`, `icons`, `man`,
# `metainfo` — belong to the distribution. The day the project creates one
# elsewhere, this check will fail and the rule will be reread, which is exactly
# what is wanted of it.
#
# **Local, and it does not replace Fedora.** It sees the cause of that
# particular defect; it would not see a wrong dependency name, nor a library
# missing at run time. `src/scripts/check-rpm.sh` sees those, and it needs a
# container and the network — so once a week, not on every pull request.
check_rpm_directories() {
    local package="$1"

    # `--dump` gives the mode of each entry, and a directory is recognised there
    # by its `04…`: the type is read rather than guessed from the name.
    # `/usr/lib/.build-id/…` is left out here as it is from the comparison of
    # the two lists, and for the same reason: `rpmbuild` places these
    # directories for its own debugging tools, nothing the project installs
    # names them, and the Fedora transaction accepts them — it is a property of
    # the format, not of what is packaged.
    local owned
    owned="$(rpm -qp --dump "${package}" 2>/dev/null \
        | awk '$5 ~ /^04/ { print $1 }' \
        | grep -v '^/usr/lib/\.build-id\(/\|$\)' \
        | grep -v "^/usr/${DATA_SUBDIR}\(/\|$\)" || true)"

    if [[ -n "${owned}" ]]; then
        report_failure "the .rpm declares it owns directories that are not its own:
$(printf '%s\n' "${owned}" | sed 's/^/    /')
    the distribution already owns them, and dnf will refuse the transaction
    exclude them in CPACK_RPM_EXCLUDE_FROM_AUTO_FILELIST_ADDITION, as absolute paths"
        return
    fi

    report_success "the .rpm owns only its own directories"
}

if wanted packages; then
    check_packages
fi

if (( failures > 0 )); then
    printf '%s%d installation check(s) failed%s\n' "${RED}" "${failures}" "${RESET}" >&2
    exit 1
fi

printf '%s✓%s a clean install installs and starts\n' "${GREEN}" "${RESET}"
