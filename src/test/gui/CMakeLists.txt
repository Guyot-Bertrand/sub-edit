# The interface test harness.
#
# Catch2 as everywhere else, and of Qt only its driving functions: that is what
# keeps the requirement tags visible to check-requirements.sh, which queries
# Catch2 binaries.
#
# Qt6::Test is linked for QTest::keyClicks and QSignalSpy, without the binary
# being a QTest binary: no QTEST_MAIN, no private slots, no moc.
set(CMAKE_AUTOMOC ON)

add_executable(
    subedit_gui_test window_harness_test.cpp subtitle_table_model_test.cpp
    cell_delegates_test.cpp timestamp_edit_test.cpp command_label_test.cpp
    window_files_test.cpp
    window_formats_test.cpp qt_prompts_test.cpp target_test.cpp save_shape_test.cpp
    operation_dialogs_test.cpp window_operations_test.cpp
    window_hearing_impaired_test.cpp window_italics_test.cpp window_text_test.cpp window_insert_test.cpp window_merge_test.cpp window_clipboard_test.cpp window_durations_test.cpp window_search_test.cpp window_search_projects_test.cpp anomaly_marking_test.cpp window_translation_test.cpp window_translation_files_test.cpp window_translation_search_test.cpp translation_dialogs_test.cpp window_append_test.cpp window_split_project_test.cpp window_tabs_test.cpp window_columns_test.cpp window_drop_test.cpp table_columns_test.cpp subtitle_table_test.cpp project_search_test.cpp project_files_test.cpp project_page_test.cpp window_actions_test.cpp video_pane_test.cpp status_line_test.cpp project_operations_test.cpp window_marks_real_test.cpp window_frames_test.cpp window_sort_test.cpp window_disjoint_test.cpp window_transcribe_test.cpp
    manual_window_test.cpp real_manual_test.cpp ems_line_measure_test.cpp
    correction_target_test.cpp correction_diff_view_test.cpp pattern_code_test.cpp
    pattern_code_selector_test.cpp pattern_list_test.cpp correction_task_page_test.cpp join_split_page_test.cpp spell_check_settings_dialog_test.cpp spell_check_dialog_test.cpp spell_check_controller_test.cpp window_spell_check_test.cpp
    correction_target_page_test.cpp correction_progress_page_test.cpp
    correction_result_model_test.cpp correction_confirmation_page_test.cpp
    correction_wizard_test.cpp correction_controller_test.cpp window_correction_test.cpp
    main_window_test.cpp mpv_player_test.cpp window_video_test.cpp
    window_player_test.cpp player_factory_test.cpp video_surface_test.cpp play_bar_test.cpp audio_track_menu_test.cpp invocation_test.cpp
    window_video_bounds_test.cpp window_grid_test.cpp window_commit_edit_test.cpp
    window_appearance_test.cpp settings_path_test.cpp window_settings_test.cpp theme_test.cpp
    window_language_test.cpp dialog_language_test.cpp shipped_languages_test.cpp
    ${CMAKE_SOURCE_DIR}/src/test/unit/grid_fixtures.cpp)

target_link_libraries(
    subedit_gui_test PRIVATE subedit::gui subedit::core Qt6::Widgets Qt6::Test Catch2::Catch2)

subedit_set_warnings(subedit_gui_test)

# The versioned corpus, for the only test that opens a real file through a
# real file system. The others go through InMemoryFileSystem, which is the
# right choice to put a window to the test — but left `openProject` without
# proof on a file that exists.
target_compile_definitions(subedit_gui_test
                           PRIVATE SUBEDIT_TEST_DATA_DIR="${CMAKE_SOURCE_DIR}/src/test/data"
                                   SUBEDIT_FAKE_TRANSCRIBER_BINARY="$<TARGET_FILE:subedit_fake_transcriber>")

# The scratch directory and the stand-ins of the transcription tests are the unit tests'.
target_include_directories(subedit_gui_test PRIVATE "${CMAKE_SOURCE_DIR}/src/test/unit/core/transcription")
add_dependencies(subedit_gui_test subedit_fake_transcriber)

# The repository's manual, for the only test that opens it whole in a
# `ManualWindow` — #268. It is given as it is to the screenshot program:
# `docs/manual/` is versioned, present on any freshly cloned machine and in
# continuous integration, and the installed one is its page-for-page copy.
target_compile_definitions(subedit_gui_test
                           PRIVATE SUBEDIT_MANUAL_DIR="${CMAKE_SOURCE_DIR}/docs/manual")

# The shipped patterns, for the wizard tests that need the real catalogue —
# two records of different codes that share a name exist only in the real one,
# and that is where a rewrite of settings went wrong (#505). Versioned, hence
# present everywhere, as for subedit_core_test.
target_compile_definitions(subedit_gui_test
                           PRIVATE SUBEDIT_PATTERNS_DIR="${CMAKE_SOURCE_DIR}/packaging/patterns")

# The window under a second language (#664): the Russian fixture `data/i18n/window.po`, compiled
# by msgfmt into a locale directory of this binary's own, laid out as an installation is, so that
# the window finds it the way `main` finds an installed catalogue.
set(subedit_gui_locale "${CMAKE_CURRENT_BINARY_DIR}/locale")
set(subedit_gui_catalogue "${subedit_gui_locale}/ru/LC_MESSAGES/subedit.mo")
add_custom_command(
    OUTPUT "${subedit_gui_catalogue}"
    COMMAND "${CMAKE_COMMAND}" -E make_directory "${subedit_gui_locale}/ru/LC_MESSAGES"
    COMMAND "${SUBEDIT_MSGFMT}" --output-file "${subedit_gui_catalogue}"
            "${CMAKE_SOURCE_DIR}/src/test/data/i18n/window.po"
    DEPENDS "${CMAKE_SOURCE_DIR}/src/test/data/i18n/window.po"
    COMMENT "msgfmt window.po"
    VERBATIM)
add_custom_target(subedit_gui_test_catalogue DEPENDS "${subedit_gui_catalogue}")
add_dependencies(subedit_gui_test subedit_gui_test_catalogue)
target_compile_definitions(subedit_gui_test PRIVATE SUBEDIT_TEST_LOCALE_DIR="${subedit_gui_locale}")

# The languages that ship (#668, #720): the catalogues of the build tree, compiled from `src/po/`
# and laid out as `cmake/Translations.cmake` installs them, read by one test as the window reads
# them.
add_dependencies(subedit_gui_test subedit_locale)
target_compile_definitions(subedit_gui_test
                           PRIVATE SUBEDIT_SHIPPED_LOCALE_DIR="${SUBEDIT_LOCALE_BUILD_DIR}")

# **Which languages ship** (#720): the lines of `src/po/LINGUAS`, on the one file that reads them
# and not on the target, so that a language added changes the command line of this file alone.
string(JOIN "," subedit_shipped_languages ${SUBEDIT_LANGUAGES})
set_source_files_properties(
    shipped_languages_test.cpp PROPERTIES COMPILE_DEFINITIONS
                                          "SUBEDIT_SHIPPED_LANGUAGES=\"${subedit_shipped_languages}\"")

# The grid fixtures, shared with the core's tests and those of the command
# line: there is only one set of files, and only one way to open it.
target_include_directories(subedit_gui_test PRIVATE ${CMAKE_SOURCE_DIR}/src/test/unit)

# Catch2 without its main: the binary provides its own, because the
# QApplication must live on main's stack. Destroyed among the process exit
# handlers — which is what a static does — it crashes, and ASan showed it.
#
# The no-display setting is made by the binary itself, and not here: it is also
# started outside CTest, by check-requirements.sh, which asks it for its tags.
catch_discover_tests(subedit_gui_test PROPERTIES LABELS gui)
