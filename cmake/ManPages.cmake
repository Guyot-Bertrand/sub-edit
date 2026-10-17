# The `man` pages of `subedit-cli`, generated from its manual — spec 15, D9, ADR 0044, #683.
#
# **No page is written by hand, and none is committed.** `src/scripts/md-to-man.py` converts a
# manual tree into `subedit-cli(1)` and one `subedit-cli-<name>(1)` per subcommand: the English
# tree `docs/manual/subedit-cli`, and the tree `docs/i18n/<language>/manual/subedit-cli` of every
# language of `src/po/LINGUAS` that has one. A change to the manual changes the pages with no
# further step, and the two can no longer drift apart.
#
# **Generated twice, and each time for a reason.**
#
# | When | Where | Why |
# | :--- | :---- | :-- |
# | at build time | `<build>/share/man/[<language>/]man1` | the build fails on a page outside the subset the converter handles, and the end-to-end tests read the pages there |
# | at installation | `<build>/man-install`, compressed, then `<prefix>/share/man/[<language>/]man1` | the FILES section names the directory the manual is installed in, which only the installation knows |
#
# The second is the reason the hand-written page had an `install(CODE)` too: configured early, it
# announced `/usr/local` in a package that installs under `/usr` — measured on the first `.deb`.
#
# **The pages are compressed by the installation** (`gzip -9 -n`), for the reason
# `Installation.cmake` gives for the window's page: the Debian policy requires it, `rpmbuild`
# would do it on its own, and both packages then carry the same bytes. `-n` keeps the name and
# the date out of the archive.
#
# The build tree reproduces the installation, as for the catalogues: `<build>/share/man` is laid
# out as `<prefix>/share/man` is, so `man -M <build>/share/man subedit-cli` reads what would be
# installed.

include(GNUInstallDirs)

find_program(SUBEDIT_PYTHON python3)
if(NOT SUBEDIT_PYTHON)
    message(
        FATAL_ERROR
        "python3 cannot be found, and the build generates the man pages from the manual.\n"
        "  install it with: ./src/scripts/setup-toolchain.sh")
endif()

set(SUBEDIT_MAN_SCRIPT "${CMAKE_SOURCE_DIR}/src/scripts/md-to-man.py")
set(SUBEDIT_MAN_BUILD_DIR "${CMAKE_BINARY_DIR}/share/man")
set(subedit_man_install_dir "${CMAKE_BINARY_DIR}/man-install")

# The trees, as parallel lists: the manual read, the directory under `man/` it is written to, and
# the directory under `share/subedit/manual` it is installed in.
set(subedit_man_trees "${CMAKE_SOURCE_DIR}/docs/manual/subedit-cli")
set(subedit_man_subdirs "man1")
set(subedit_man_manual_suffixes "-")
foreach(language IN LISTS SUBEDIT_LANGUAGES)
    set(tree "${CMAKE_SOURCE_DIR}/docs/i18n/${language}/manual/subedit-cli")
    if(EXISTS "${tree}")
        list(APPEND subedit_man_trees "${tree}")
        list(APPEND subedit_man_subdirs "${language}/man1")
        list(APPEND subedit_man_manual_suffixes "${language}")
    endif()
endforeach()

set(subedit_man_outputs "")
set(subedit_man_install_commands "")
list(LENGTH subedit_man_trees subedit_man_count)
math(EXPR subedit_man_last "${subedit_man_count} - 1")
foreach(position RANGE ${subedit_man_last})
    list(GET subedit_man_trees ${position} tree)
    list(GET subedit_man_subdirs ${position} subdir)
    list(GET subedit_man_manual_suffixes ${position} suffix)
    set(manual "${CMAKE_INSTALL_DATADIR}/subedit/manual")
    if(NOT suffix STREQUAL "-")
        set(manual "${manual}/${suffix}")
    endif()

    # Every page of the tree is an input: the converter reads them all, the ones that give no page
    # included, since it refuses a construct outside its subset wherever it is.
    file(GLOB pages CONFIGURE_DEPENDS "${tree}/*.md")
    set(output "${SUBEDIT_MAN_BUILD_DIR}/${subdir}/subedit-cli.1")
    add_custom_command(
        OUTPUT "${output}"
        COMMAND
            "${SUBEDIT_PYTHON}" "${SUBEDIT_MAN_SCRIPT}" --manual "${tree}" --output-dir
            "${SUBEDIT_MAN_BUILD_DIR}/${subdir}" --version "${PROJECT_VERSION}" --manual-dir
            "${CMAKE_INSTALL_PREFIX}/${manual}"
        DEPENDS ${pages} "${SUBEDIT_MAN_SCRIPT}" "${CMAKE_SOURCE_DIR}/src/scripts/doc_vocabulary.py"
        COMMENT "md-to-man.py ${subdir}"
        VERBATIM)
    list(APPEND subedit_man_outputs "${output}")

    string(
        APPEND
        subedit_man_install_commands
        "
        execute_process(
            COMMAND \"${SUBEDIT_PYTHON}\" \"${SUBEDIT_MAN_SCRIPT}\" --manual \"${tree}\"
                    --output-dir \"${subedit_man_install_dir}/${subdir}\" --version \"${PROJECT_VERSION}\"
                    --manual-dir \"\${CMAKE_INSTALL_PREFIX}/${manual}\"
            OUTPUT_QUIET
            COMMAND_ERROR_IS_FATAL ANY)")
endforeach()

# In `all`, and a dependency of the command line: the gates build the targets they name and not
# `all`, and the packages are made from a tree that built only the two executables.
add_custom_target(subedit_man_pages ALL DEPENDS ${subedit_man_outputs})
add_dependencies(subedit-cli subedit_man_pages)

# Regenerated at every installation, from an empty directory: a page that a removed subcommand
# left behind must not be installed.
install(
    CODE "
        file(REMOVE_RECURSE \"${subedit_man_install_dir}\")
        ${subedit_man_install_commands}
        file(GLOB_RECURSE subedit_man_generated \"${subedit_man_install_dir}/*.1\")
        foreach(page IN LISTS subedit_man_generated)
            execute_process(COMMAND gzip -9 -n -f \"\${page}\" COMMAND_ERROR_IS_FATAL ANY)
        endforeach()")

install(
    DIRECTORY "${subedit_man_install_dir}/"
    DESTINATION "${CMAKE_INSTALL_MANDIR}"
    FILES_MATCHING
    PATTERN "*.1.gz")
