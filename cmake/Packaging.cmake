# The two native packages — ADR 0023, issue #244.
#
# **They come out of the installation, not out of a separate description.**
# CPack packages what `cmake --install` lays down; there is therefore no list
# of files to keep in step, and a package cannot diverge from what the project
# installs. That is half of the ADR's decision: installation first, formats
# next.
#
# **The second format is what puts that half to the test.** "Installation
# first" is a claim as long as a single format exercises it; had the `.rpm`
# required touching the `install()` rules, they would have been wrong. It did
# not, and `check-installation.sh` checks it by comparing the two lists of
# files.
#
# **What really differs between the two is here, and that is all**: the names
# of the dependency packages. The same libraries are named differently on
# Debian and on Fedora, and that is where a package can be wrong without
# anything showing it at build time — a wrong name produces a perfectly valid
# package that refuses to install.

include(GNUInstallDirs)

set(CPACK_PACKAGE_NAME "subedit")
set(CPACK_PACKAGE_VENDOR "Bertrand Guyot")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_HOMEPAGE_URL "${PROJECT_HOMEPAGE_URL}")
set(CPACK_PACKAGE_CONTACT "Bertrand Guyot <bertrand.guyot.vpc@orange.fr>")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")

# **In English, like everything the user reads.** This summary shows in a
# software centre and in `apt show`; it is the same sentence as the AppStream
# `summary` and as the binary's description line.
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Read, inspect and retime subtitle files")

set(CPACK_GENERATOR "DEB;RPM")

# **Each family names its packages its own way, and we let it.**
# `subedit_0.7.12_amd64.deb` on one side, `subedit-0.7.12-1.x86_64.rpm` on the
# other. Without these two lines, CPack gives both the same neutral name —
# `subedit-0.7.12-Linux` — which states neither the architecture nor the
# revision, and which a distribution tool does not recognise.
set(CPACK_DEBIAN_FILE_NAME "DEB-DEFAULT")
set(CPACK_RPM_FILE_NAME "RPM-DEFAULT")

# ## What differs: the dependencies
#
# **Neither is deduced.** CPack can deduce Debian dependencies through
# `dpkg-shlibdeps`, but only on the machine where the package is built and for
# the libraries the linker named; it knows nothing of Fedora. Both lists are
# therefore written, and they are the only thing this file keeps by hand.
#
# Qt 6 and libmpv first: they are the two runtime dependencies that ADR 0020
# and phase 5 brought in.
#
# **ICU came in at phase 8**, and it is the only one of the three that
# `subedit-cli` pulls in too — ADR 0027. The two families do not name it the
# same: `libicu74` on Debian and Ubuntu, where the number follows ICU's major
# version, `libicu` on Fedora, where there is none. A single number and not an
# alternative: ICU's `SONAME` changes with every major version, so a package
# that accepted `libicu70` would let a library be installed that the binary
# cannot load. That is exactly the place where a package can be wrong without
# anything showing it at build time — #266 paid for it once — and
# `make rpm-check` is what now catches it, by installing the `.rpm` on a real
# Fedora.
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libqt6widgets6 (>= 6.4), libqt6gui6 (>= 6.4), libqt6core6 (>= 6.4), libmpv2 | libmpv1, libicu74, libenchant-2-2")
set(CPACK_DEBIAN_PACKAGE_SECTION "video")
set(CPACK_DEBIAN_PACKAGE_RECOMMENDS "ffmpeg")

# `ffmpeg` as a recommendation and not a dependency: the tool works without it
# — the window only stops offering the frame rate the film declares. Making it
# a dependency would pull a heavy package in for a convenience, and would lie
# about what the program requires.
#
# **The `.rpm` carries none, for two reasons that point the same way.** The
# first is mechanical: CPack probes `rpm --suggests`, which fails on Ubuntu's
# `rpmbuild`, and then removes the four weak dependency tags — a `SUGGESTS`
# set here would therefore be dropped silently on the very machine that builds
# the package. The second is substantive: `ffmpeg` is not in Fedora's default
# repositories, and naming the package that replaces it would be guessing, on a
# distribution this repository cannot test. The manual says what `ffmpeg`
# brings; that is the right place to tell whoever has no package manager to
# learn it from.
set(CPACK_RPM_PACKAGE_REQUIRES "qt6-qtbase-gui >= 6.4, mpv-libs, libicu, enchant2")
# **The distribution is in the file name**: `subedit-X.Y.Z-1.fc43.x86_64.rpm`.
# The published `.rpm` is built on each Fedora, against its ICU, and two of
# them are not interchangeable — `src/scripts/build-rpm.sh`.
set(CPACK_RPM_PACKAGE_RELEASE_DIST ON)
set(CPACK_RPM_PACKAGE_LICENSE "GPL-3.0-or-later")
set(CPACK_RPM_PACKAGE_GROUP "Applications/Multimedia")

# **Enchant 2 came in at phase 12** — D6 of docs/specs/12-correction.md —, and
# `subedit-cli` pulls it in too. Unlike ICU, its name carries no major version
# number: `libenchant-2-2` on Debian and Ubuntu, where the first `2` is the API
# and the second the `SONAME`, `enchant2` on Fedora. The dictionaries, for
# their part, are not a dependency: the program works without them, and the
# feature switches off saying so.
#
# ## The packaging prefix, written rather than assumed
#
# `/usr` is the default of both generators, and leaving it implicit had a
# precise cost: the exclusion list below is written in the paths **of the
# package**, hence prefixed, and there was nothing to prefix them with. Naming
# it here gives both the same starting point.
set(CPACK_PACKAGING_INSTALL_PREFIX "/usr")

# **The directories the distribution already owns are not in the package.**
# A `.rpm` that declares it owns `/usr/share/applications` conflicts with
# `filesystem`, which really owns it, and `dnf` refuses the transaction. The
# Debian generator does not have that problem: a `.deb` does not own its
# directories.
#
# **This list had existed since #244 and excluded nothing** — issue #266. It
# was written in relative paths, `share/applications`, because
# `CMAKE_INSTALL_DATADIR` is relative; CPack compares package paths, hence
# absolute ones, and never found a single match. The `.rpm` owned its eight
# shared directories, and installation failed on six conflicts:
#
#     file /usr/share/applications from install of subedit-0.8.4-1.x86_64
#     conflicts with file from package filesystem-3.18-47.fc42.x86_64
#
# Nothing could tell: `rpm -qlp` reads a list of files, not a transaction, and
# the repository had no Fedora to play one on. It has had one since
# `src/scripts/check-rpm.sh`.
#
# **The conflict is about the mode**, and that is what makes it invisible to
# the eye: CPack declares these directories as 0775, `filesystem` owns them as
# 0755. Two packages can share a directory as long as they describe it alike;
# these did not describe it alike.
set(CPACK_RPM_EXCLUDE_FROM_AUTO_FILELIST_ADDITION
    "${CPACK_PACKAGING_INSTALL_PREFIX}/${CMAKE_INSTALL_DATADIR}/applications"
    "${CPACK_PACKAGING_INSTALL_PREFIX}/${CMAKE_INSTALL_DATADIR}/metainfo"
    "${CPACK_PACKAGING_INSTALL_PREFIX}/${CMAKE_INSTALL_DATADIR}/icons"
    "${CPACK_PACKAGING_INSTALL_PREFIX}/${CMAKE_INSTALL_DATADIR}/icons/hicolor"
    "${CPACK_PACKAGING_INSTALL_PREFIX}/${CMAKE_INSTALL_DATADIR}/icons/hicolor/scalable"
    "${CPACK_PACKAGING_INSTALL_PREFIX}/${CMAKE_INSTALL_DATADIR}/icons/hicolor/scalable/apps"
    "${CPACK_PACKAGING_INSTALL_PREFIX}/${CMAKE_INSTALL_MANDIR}"
    "${CPACK_PACKAGING_INSTALL_PREFIX}/${CMAKE_INSTALL_MANDIR}/man1")
# The man pages of every installed language go under `man/<language>/man1` (#683), which the
# distribution owns as it owns `man/man1`: Fedora's `filesystem` lays them down.
foreach(language IN LISTS SUBEDIT_LANGUAGES)
    list(APPEND CPACK_RPM_EXCLUDE_FROM_AUTO_FILELIST_ADDITION
         "${CPACK_PACKAGING_INSTALL_PREFIX}/${CMAKE_INSTALL_MANDIR}/${language}"
         "${CPACK_PACKAGING_INSTALL_PREFIX}/${CMAKE_INSTALL_MANDIR}/${language}/man1")
endforeach()

# **No post-installation script, and that is a decision.** Gaupol calls
# `update-desktop-database` from its Makefile, and only when `DESTDIR` is empty
# — because a staged installation must touch nothing outside. A native package
# does not have that concern, for another reason: `dpkg` and `rpm` call the
# desktop directory triggers themselves. A script that did it again would do
# the work twice, and would do it badly the day the distribution changed its
# mechanism.

include(CPack)
