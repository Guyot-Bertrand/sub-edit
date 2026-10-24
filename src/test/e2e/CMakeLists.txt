# End-to-end tests.
#
# They do not link the logic they check: they start the real binary and read
# what a user would see of it — standard output, standard error, exit code.
# The tree mirrors src/exe, as that of unit/ mirrors src/lib.

add_executable(
    subedit_e2e_test
    cli/cli_run.cpp
    cli/config_home_test.cpp
    cli/expected_file_test.cpp
    cli/generated_batch_test.cpp
    cli/recursive_test.cpp
    cli/replace_test.cpp
    cli/json_test.cpp
    cli/language_test.cpp
    cli/dry_run_test.cpp
    cli/usage_test.cpp
    cli/version_report_test.cpp
    cli/inspect_test.cpp
    cli/narration_test.cpp
    cli/adjust_test.cpp
    cli/config_test.cpp
    cli/batch_test.cpp
    cli/convert_test.cpp
    cli/formats_test.cpp
    cli/append_test.cpp
    cli/hearing_impaired_test.cpp
    cli/pair_test.cpp
    cli/list_encodings_test.cpp
    cli/shift_test.cpp
    cli/snap_test.cpp
    cli/split_file_test.cpp
    cli/sort_test.cpp
    cli/transcribe_test.cpp
    cli/disjoint_test.cpp
    cli/join_test.cpp
    cli/text_rewriting_test.cpp
    cli/transform_test.cpp
    cli/translation_test.cpp
    cli/correct_test.cpp
    cli/correct_oracle_test.cpp
    cli/pattern_catalogue_test.cpp
    cli/framerate_test.cpp
    cli/shipped_languages_test.cpp
    cli/man_pages_test.cpp
    gui/version_test.cpp)

# subedit::core for versionString(): a test's expectation is computed, not
# copied — a copied number is a number that goes stale.
target_link_libraries(subedit_e2e_test PRIVATE subedit::core Catch2::Catch2WithMain)

# The exact path of the binary, resolved by CMake, as SUBEDIT_TEST_DATA_DIR is
# for the corpus: a test must not depend on the directory it is started
# from.
#
# SUBEDIT_SOURCE_DIR for the build files themselves, versioned: the test of
# `--version` reads their `find_package` calls (#716).
target_compile_definitions(
    subedit_e2e_test PRIVATE SUBEDIT_CLI_BINARY="$<TARGET_FILE:subedit-cli>"
                             SUBEDIT_GUI_BINARY="$<TARGET_FILE:subedit-gui>"
                             SUBEDIT_PATTERNS_DIR="${CMAKE_SOURCE_DIR}/packaging/patterns"
                             SUBEDIT_PATTERN_CATALOGUE_BINARY="$<TARGET_FILE:subedit_pattern_catalogue>"
                             SUBEDIT_FAKE_TRANSCRIBER_BINARY="$<TARGET_FILE:subedit_fake_transcriber>"
                             SUBEDIT_TEST_DATA_DIR="${CMAKE_SOURCE_DIR}/src/test/data"
                             SUBEDIT_SOURCE_DIR="${CMAKE_SOURCE_DIR}")

# $<TARGET_FILE:…> gives a path, not a build order. Without this line,
# nothing guarantees that the binary exists when the test starts it.
add_dependencies(subedit_e2e_test subedit-cli subedit-gui subedit_pattern_catalogue subedit_fake_transcriber)

# **The command line under a second language** (#663, spec 15 D4): a prefix of the
# tests' own, laid out as an installation is — `bin/subedit-cli`, a copy of the
# binary, and `share/subedit/locale/ru/LC_MESSAGES/subedit.mo`, compiled from the
# versioned fixture `src/test/data/i18n/cli.po`. The binary finds its catalogues from
# where it runs, as it does installed, so nothing in it is there for the tests.
#
# **A copy, and not the build tree's own binary**: a fixture catalogue next to it
# would be a language the build tree offers. The patterns are linked in, as they are
# for the build tree (`cmake/Installation.cmake`), so the copy reads what the
# original reads.
set(subedit_translated_prefix "${CMAKE_CURRENT_BINARY_DIR}/translated")
set(subedit_translated_cli "${subedit_translated_prefix}/bin/subedit-cli")
set(subedit_translated_mo "${subedit_translated_prefix}/share/subedit/locale/ru/LC_MESSAGES/subedit.mo")
file(MAKE_DIRECTORY "${subedit_translated_prefix}/share/subedit")
file(
    CREATE_LINK "${CMAKE_SOURCE_DIR}/packaging/patterns"
    "${subedit_translated_prefix}/share/subedit/patterns"
    COPY_ON_ERROR SYMBOLIC)
add_custom_command(
    OUTPUT "${subedit_translated_cli}"
    COMMAND "${CMAKE_COMMAND}" -E copy "$<TARGET_FILE:subedit-cli>" "${subedit_translated_cli}"
    DEPENDS subedit-cli
    COMMENT "subedit-cli, copied next to the fixture catalogue"
    VERBATIM)
add_custom_command(
    OUTPUT "${subedit_translated_mo}"
    COMMAND "${CMAKE_COMMAND}" -E make_directory "${subedit_translated_prefix}/share/subedit/locale/ru/LC_MESSAGES"
    COMMAND "${SUBEDIT_MSGFMT}" --output-file "${subedit_translated_mo}" "${CMAKE_SOURCE_DIR}/src/test/data/i18n/cli.po"
    DEPENDS "${CMAKE_SOURCE_DIR}/src/test/data/i18n/cli.po"
    COMMENT "msgfmt cli.po"
    VERBATIM)
add_custom_target(subedit_translated_cli DEPENDS "${subedit_translated_cli}" "${subedit_translated_mo}")
add_dependencies(subedit_e2e_test subedit_translated_cli)
target_compile_definitions(subedit_e2e_test
                           PRIVATE SUBEDIT_TRANSLATED_CLI_BINARY="${subedit_translated_cli}")

# **The man pages the build generates** (#683): read where the build lays them out, as an
# installation does (`cmake/ManPages.cmake`), and rendered by the machine's `man`, found here
# rather than in the test's `PATH`. Missing, it is an empty path, and the test that renders says
# why it cannot.
find_program(SUBEDIT_MAN_PROGRAM man)
if(NOT SUBEDIT_MAN_PROGRAM)
    set(SUBEDIT_MAN_PROGRAM "")
endif()
target_compile_definitions(subedit_e2e_test PRIVATE SUBEDIT_MAN_DIR="${SUBEDIT_MAN_BUILD_DIR}"
                                                    SUBEDIT_MAN_PROGRAM="${SUBEDIT_MAN_PROGRAM}")
add_dependencies(subedit_e2e_test subedit_man_pages)

# **The languages that ship** (#668, #720): the build tree's binary reads the catalogues the build
# compiles beside it, as an installed one does, so a test of them needs them built first.
add_dependencies(subedit_e2e_test subedit_locale)

# **Which languages ship, on the two files that read it alone** — the lines of `src/po/LINGUAS`,
# as `cmake/Translations.cmake` reads them (`SUBEDIT_LANGUAGES`), and where their catalogues are
# compiled. On the sources and not on the target, as `version.cpp` carries the version (#686): a
# language added to LINGUAS changes the command line of these two files, and of no other test.
string(JOIN "," subedit_shipped_languages ${SUBEDIT_LANGUAGES})
set_source_files_properties(
    cli/shipped_languages_test.cpp cli/man_pages_test.cpp
    PROPERTIES COMPILE_DEFINITIONS
               "SUBEDIT_SHIPPED_LANGUAGES=\"${subedit_shipped_languages}\";SUBEDIT_SHIPPED_LOCALE_DIR=\"${SUBEDIT_LOCALE_BUILD_DIR}\"")

subedit_set_warnings(subedit_e2e_test)

# The harness registers with CTest only under the asan and release presets,
# through the cache option SUBEDIT_REGISTER_E2E (declared in CMakeLists.txt) —
# OFF by default, set to ON explicitly by CMakePresets.json in those two
# presets and nowhere else.
#
#   - dev and asan are both Debug builds; asan is strictly more informative
#     (AddressSanitizer, UndefinedBehaviorSanitizer, LeakSanitizer), so also
#     registering under dev would bring nothing — one more run, with no new
#     signal.
#   - release is the mode that ships: it is the binary a user actually runs.
#   - coverage stays excluded: under that preset, subedit-cli is instrumented
#     too, and each invocation would inflate the coverage of src/lib without
#     any unit test having been written for that code, just before #50 made it
#     a ratchet. See `docs/adr/0014-registre-d-exigences.md`.
#
# The target builds in all four presets — its warnings are ours everywhere —
# only the run is reserved for asan and release.
if(SUBEDIT_REGISTER_E2E)
    # CTest's default is 1500 s: a stuck binary would tie the CI up for
    # twenty-five minutes before saying why. The LABELS e2e label lets
    # `make e2e` filter these tests with `ctest -L e2e`, without depending on
    # their names — a unit test with a similar name would not fool it.
    # **QT_QPA_PLATFORM=offscreen, and it is indispensable.** Measured: in an
    # environment without a display, constructing a QApplication aborts on
    # "could not connect to display" — a runtime failure, which no compilation
    # can announce. The CI has no graphical server.
    #
    # It is set here, on CTest, rather than in each binary: a setting made by
    # the code would have to be made again for each new binary, and forgetting
    # it would show only on the machine that has no display — never on the one
    # where it is written.
    catch_discover_tests(
        subedit_e2e_test
        PROPERTIES TIMEOUT 60 LABELS e2e ENVIRONMENT QT_QPA_PLATFORM=offscreen)
endif()
