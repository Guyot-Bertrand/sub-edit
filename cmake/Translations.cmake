# The message catalogues: versioned `.po` → `.mo` at build time → installation.
#
# **This module compiles and installs; it neither extracts nor merges.** `make pot` and `make po`
# (`src/scripts/update-pot.sh`, `update-po.sh`) write files of the repository, which a build never
# does — a build tree does not modify the sources. The template is kept up to date by a step of
# `check-local`.
#
# **The installed languages are those of `src/po/LINGUAS`**, one per line. A `.po` that is not
# there — the seeds imported from Gaupol, issue #667 — stays in the repository without being
# compiled or shipped: a language enters the packages when it is complete and reviewed (spec 15,
# D5).
#
# **The build tree reproduces the installation**, as for the patterns (#566): the `.mo` files are
# written under `<build>/share/subedit/locale/<language>/LC_MESSAGES/subedit.mo`, where a binary
# of `<build>/bin` finds them at `../share/subedit/locale`. The installed path is the same, under
# `<prefix>/share/subedit/locale`.
#
# `msgfmt` is a build tool, never a runtime one: the reader of `core/i18n/` reads the `.mo` itself
# (ADR 0042), and nothing of gettext is required on the user's machine.

include(GNUInstallDirs)

find_program(SUBEDIT_MSGFMT msgfmt)
if(NOT SUBEDIT_MSGFMT)
    message(
        FATAL_ERROR
        "msgfmt cannot be found, and the build compiles the message catalogues.\n"
        "  install it with: ./src/scripts/setup-toolchain.sh")
endif()

set(SUBEDIT_PO_DIR "${CMAKE_SOURCE_DIR}/src/po")
set(SUBEDIT_LOCALE_BUILD_DIR "${CMAKE_BINARY_DIR}/share/subedit/locale")

# The languages file is an input of the configuration: adding one must reconfigure.
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${SUBEDIT_PO_DIR}/LINGUAS")
file(STRINGS "${SUBEDIT_PO_DIR}/LINGUAS" subedit_linguas)

set(subedit_catalogues "")
foreach(language IN LISTS subedit_linguas)
    string(STRIP "${language}" language)
    if(language STREQUAL "" OR language MATCHES "^#")
        continue()
    endif()

    set(po "${SUBEDIT_PO_DIR}/${language}.po")
    if(NOT EXISTS "${po}")
        message(FATAL_ERROR "src/po/LINGUAS names \"${language}\", and ${po} does not exist")
    endif()

    set(mo "${SUBEDIT_LOCALE_BUILD_DIR}/${language}/LC_MESSAGES/subedit.mo")
    # `--check`: the header is complete, the formats of the messages match those of the original.
    add_custom_command(
        OUTPUT "${mo}"
        COMMAND "${CMAKE_COMMAND}" -E make_directory "${SUBEDIT_LOCALE_BUILD_DIR}/${language}/LC_MESSAGES"
        COMMAND "${SUBEDIT_MSGFMT}" --check --output-file "${mo}" "${po}"
        DEPENDS "${po}"
        COMMENT "msgfmt ${language}.po"
        VERBATIM)
    list(APPEND subedit_catalogues "${mo}")

    install(FILES "${mo}"
            DESTINATION "${CMAKE_INSTALL_DATADIR}/subedit/locale/${language}/LC_MESSAGES")
endforeach()

# In `all`: a `.mo` that no target asks for would not be built, hence not installed.
add_custom_target(subedit_locale ALL DEPENDS ${subedit_catalogues})

# **Both executables depend on it**, and that is what makes the previous sentence hold: the gates
# build only the targets they name, not `all`. Without this dependency, a binary built alone would
# have no catalogue next to it, and `cmake --install` would fail on a missing file — which
# `check-installation.sh` showed at the first attempt.
add_dependencies(subedit-cli subedit_locale)
add_dependencies(subedit-gui subedit_locale)
