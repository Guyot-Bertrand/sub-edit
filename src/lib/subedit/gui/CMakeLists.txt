# subedit_gui — the window, its models and its dialogs.
#
# Structuring constraint, symmetrical to the core's: everything that is not
# wiring lives here and not in src/exe, because check-architecture.sh refuses
# a main that defines a class. That is what makes the interface testable.
set(CMAKE_AUTOMOC ON)

add_library(
    subedit_gui STATIC subtitle_table.cpp subtitle_table_model.cpp cell_delegates.cpp subtitle_editor.cpp line_length_display.cpp command_label.cpp messages.cpp interface_language.cpp
                       invocation.cpp libraries.cpp settings_path.cpp theme.cpp preferences_dialog.cpp
                       manual_path.cpp manual_window.cpp
                       diagnostics_button.cpp qt_prompts.cpp target.cpp ems_line_measure.cpp
                       correction_target.cpp pattern_code.cpp pattern_code_selector.cpp
                       pattern_list.cpp correction_task_page.cpp
                       correction_target_page.cpp
                       correction_progress_page.cpp
                       correction_diff_view.cpp
                       correction_result_model.cpp
                       correction_confirmation_page.cpp
                       correction_wizard.cpp
                       join_split_page.cpp spell_highlighter.cpp spell_check_settings_dialog.cpp spell_check_dialog.cpp
                       correction_controller.cpp spell_check_controller.cpp
                       operation_dialog.cpp shift_dialog.cpp transform_dialog.cpp
                       insert_dialog.cpp split_project_dialog.cpp save_shape.cpp
                       frame_rate_box.cpp frame_rate_dialog.cpp hearing_impaired_dialog.cpp
                       grid_analysis_dialog.cpp snap_dialog.cpp fixes_proposal_dialog.cpp disjoint_dialog.cpp
    duration_adjust_dialog.cpp
    search_dialog.cpp about_dialog.cpp
    open_translation_dialog.cpp unsaved_documents_dialog.cpp
                       mpv_player.cpp video_surface.cpp play_bar.cpp audio_track_menu.cpp player_factory.cpp project_page.cpp table_columns.cpp project_search.cpp project_files.cpp window_actions.cpp video_pane.cpp status_line.cpp project_operations.cpp main_window.cpp)
add_library(subedit::gui ALIAS subedit_gui)

target_include_directories(subedit_gui PUBLIC "${CMAKE_SOURCE_DIR}/src/lib")

target_link_libraries(subedit_gui PUBLIC subedit::core subedit::platform Qt6::Widgets Qt6::Concurrent)

# libmpv, behind the `core::VideoPlayer` seam — see docs/adr/0020.
#
# **The implementation is here and not in the core**, although neither it nor
# libmpv knows Qt. The player is an interface element: it exists only for the
# window, receives the window's own through the `wid` property, and nothing
# else will call it. Bringing libmpv into `subedit_core` would have given the
# domain an infrastructure dependency for the mere convenience of a lighter
# test harness — which is not an architectural reason.
#
# What remains in the core is the interface, and it alone: `core::VideoPlayer`
# names neither libmpv nor Qt, and that is what lets the rest of the core reason
# about a player without knowing one.
#
# PRIVATE: `mpv_player.hpp` only declares `mpv_handle` and `mpv_render_context`, so that no file
# that includes the library needs libmpv's headers.
target_link_libraries(subedit_gui PRIVATE PkgConfig::mpv)

subedit_set_warnings(subedit_gui)
