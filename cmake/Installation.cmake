# What `cmake --install` lays down, and where.
#
# **None of this can be checked from the build tree.** A data file that the
# binary looks for next to itself is present there by an accident of layout: it
# is there because the repository contains it, not because a rule copied it.
# The defect shows only at the first clean installation, and that is what
# `check-installation.sh` performs — issue #239.
#
# The paths come from `GNUInstallDirs` and never from a string written by hand:
# `lib` or `lib64`, `share` or something else, the distribution decides, and a
# packager who relocates `CMAKE_INSTALL_PREFIX` expects everything to follow.
#
# **The six lines of ADR 0023 are here, since #244.** The first two — the
# binaries and the manual — were set by #239, because an installation check had
# nothing to prove without them; the other four arrived with the files they lay
# down, which did not exist either.
#
# **The names of the three desktop files are the application identifier**,
# `io.github.guyot_bertrand.subedit`, and not `subedit`. That is what desktops
# expect: `appstreamcli` refuses a metadata file whose name does not repeat the
# identifier, and an icon found by the theme is an icon whose name is the one
# the `Icon=` key writes.

include(GNUInstallDirs)

install(TARGETS subedit-cli subedit-gui RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")

# The whole manual, subdirectories and screenshots included.
#
# **A directory and not a list of files**, and that is a decision: a list goes
# stale at the first chapter added, silently, and the installation check would
# see the gap only if someone thought of putting the new page in the list —
# that is, never. `DIRECTORY` makes the defect structurally impossible for what
# lives under `docs/manual`.
#
# `Help ▸ Manual` will open what is laid down here — issue #245.
install(
    DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/docs/manual/"
    DESTINATION "${CMAKE_INSTALL_DATADIR}/subedit/manual")

# Gaupol's correction patterns, as they are — ADR 0037, issue #498.
#
# **A directory, for the manual's reason**: a list would go stale at the first
# language added. And **the location is the one the program reads relative to
# its executable** (`../share/subedit/patterns`), for any prefix.
install(
    DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/packaging/patterns/"
    DESTINATION "${CMAKE_INSTALL_DATADIR}/subedit/patterns")

# **The build tree reproduces the installation** — issue #566. The binary looks
# for its patterns at `../share/subedit/patterns` from its executable; a binary
# started from `build/<preset>/bin` therefore finds them the same way, through a
# link that CMake sets, rather than through a debugging option that the release
# would not contain. It is a link and not a copy: the patterns are the
# repository's, and a change shows there without reconfiguring.
file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/share/subedit")
file(
    CREATE_LINK "${CMAKE_CURRENT_SOURCE_DIR}/packaging/patterns"
    "${CMAKE_BINARY_DIR}/share/subedit/patterns"
    RESULT patterns_link_result
    COPY_ON_ERROR SYMBOLIC)
if(NOT patterns_link_result STREQUAL "0")
    message(WARNING "the patterns are not visible from the build tree: ${patterns_link_result}")
endif()

# The application identifier, written once. The three desktop files carry it in
# their name, and the `.desktop` carries it again in its `Icon=` key: four
# places for a single name, hence a single place to say it.
set(SUBEDIT_APP_ID "io.github.guyot_bertrand.subedit")

install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/packaging/${SUBEDIT_APP_ID}.svg"
        DESTINATION "${CMAKE_INSTALL_DATADIR}/icons/hicolor/scalable/apps")

install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/packaging/${SUBEDIT_APP_ID}.desktop"
        DESTINATION "${CMAKE_INSTALL_DATADIR}/applications")

install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/packaging/${SUBEDIT_APP_ID}.metainfo.xml"
        DESTINATION "${CMAKE_INSTALL_DATADIR}/metainfo")

# **The man page is generated at installation time, not at configuration
# time.** Two values are injected into it: the version number, which would be
# wrong at the first increment if it were copied into the source, and the
# directory of the full manual, which the page names in full.
#
# The second is the reason for the `install(CODE)`. The prefix is known only at
# installation — CPack sets `/usr`, `make install-check` a temporary directory,
# a user whatever they want. Configured too early, the page announced
# `/usr/local` in a package that installs under `/usr`: measured on the first
# `.deb` produced, and that is exactly the kind of lie a generated file is
# meant to make impossible.
#
# **It is also compressed, and that is not an affectation**: the Debian policy
# requires a compressed man page, `rpmbuild` compresses its own on its own, and
# a bare page in the `.deb` would thus have been both a breach of the policy and
# a difference between the two packages — a difference that
# `check-installation.sh` refuses. `-n` so that the archive carries neither a
# timestamp nor an original name: without it, two builds of the same file would
# give two different sets of bytes.
# **Each of the two binaries has its own** — issue #272. `subedit-gui` had none,
# and the `SEE ALSO` of its neighbour made that a decision: "a window is not
# described by a man page". The counterexample was in the reference repository
# — the only document Gaupol installs is a man page, and it is the one for its
# graphical program. What a page describes of a graphical application is not
# its window but its command-line surface, which exists.
#
# The list is written once: both pages are generated and installed by the same
# path, so a third one would need only one more name.
set(SUBEDIT_MAN_PAGES subedit-cli subedit-gui)

install(
    CODE "
        set(PROJECT_VERSION \"${PROJECT_VERSION}\")
        set(SUBEDIT_MANUAL_DIR \"\${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_DATADIR}/subedit/manual\")
        foreach(page ${SUBEDIT_MAN_PAGES})
            configure_file(
                \"${CMAKE_CURRENT_SOURCE_DIR}/packaging/\${page}.1.in\"
                \"${CMAKE_CURRENT_BINARY_DIR}/\${page}.1\"
                @ONLY)
            execute_process(
                COMMAND gzip -9 -n -f \"${CMAKE_CURRENT_BINARY_DIR}/\${page}.1\"
                COMMAND_ERROR_IS_FATAL ANY)
        endforeach()")

foreach(page IN LISTS SUBEDIT_MAN_PAGES)
    install(FILES "${CMAKE_CURRENT_BINARY_DIR}/${page}.1.gz"
            DESTINATION "${CMAKE_INSTALL_MANDIR}/man1")
endforeach()
