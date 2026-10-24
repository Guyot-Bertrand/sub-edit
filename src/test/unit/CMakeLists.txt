# One test binary per library, each case being registered individually with
# CTest by catch_discover_tests.

add_executable(
    subedit_core_test
    text_cases.cpp
    grid_fixtures.cpp
    await_process.cpp
    text_cases_test.cpp
    core/analysis/anomaly_test.cpp
    core/analysis/frame_rate_deduction_test.cpp
    core/analysis/grid_correction_test.cpp
    core/analysis/grid_repair_test.cpp
    core/version_test.cpp
    core/libraries_test.cpp
    core/wording_language_test.cpp
    core/wording_test.cpp
    core/i18n/catalogue_test.cpp
    core/i18n/language_test.cpp
    core/i18n/messages_test.cpp
    core/i18n/plural_expression_test.cpp
    core/edit/shift_limits_test.cpp
    core/edit/video_bounds_test.cpp
    core/time/duration_test.cpp
    core/time/frame_test.cpp
    core/time/frame_rate_test.cpp
    core/time/ratio_test.cpp
    core/time/scaling_test.cpp
    core/time/timestamp_test.cpp
    core/time/timestamp_conversion_test.cpp
    core/time/timestamp_format_test.cpp
    core/time/timestamp_parse_test.cpp
    core/text/break_marker_test.cpp
    core/text/capitalization_test.cpp
    core/text/hearing_impaired_test.cpp
    core/text/hearing_impaired_correction_test.cpp
    core/text/common_errors_test.cpp
    core/text/correction_run_test.cpp
    core/text/text_diff_test.cpp
    core/text/line_breaking_test.cpp
    core/text/line_lengths_test.cpp
    core/text/line_measure_test.cpp
    core/text/join_split_words_test.cpp
    core/text/spell_check_navigator_test.cpp
    core/text/spell_check_walk_test.cpp
    core/text/spell_checker_test.cpp
    core/text/spell_ranges_test.cpp
    core/text/spell_replacements_test.cpp
    core/text/spell_tokenizer_test.cpp
    core/text/enchant_spell_provider_test.cpp
    core/text/icu_pattern_engine_test.cpp
    core/text/pattern_catalogue_test.cpp
    core/text/pattern_paths_test.cpp
    core/text/python_regex_syntax_test.cpp
    core/text/replacement_template_test.cpp
    core/text/dialogue_dashes_test.cpp
    core/text/italics_test.cpp
    core/text/letter_case_test.cpp
    core/model/encoding_test.cpp
    core/model/format_extras_test.cpp
    core/model/file_extras_test.cpp
    core/model/subtitle_test.cpp
    core/model/project_test.cpp
    core/model/project_video_test.cpp
    core/model/project_documents_test.cpp
    core/model/video_file_test.cpp
    core/model/subtitle_index_test.cpp
    core/model/project_access_test.cpp
    core/model/project_edit_test.cpp
    core/model/selection_test.cpp
    core/edit/session_test.cpp
    core/edit/append_test.cpp
    core/edit/split_project_test.cpp
    core/edit/set_text_command_test.cpp
    core/edit/hearing_impaired_removal_test.cpp
    core/edit/italics_command_test.cpp
    core/edit/document_dialect_test.cpp
    core/edit/translation_test.cpp
    core/edit/translation_drift_test.cpp
    core/edit/rewrite_texts_test.cpp
    core/edit/text_commands_test.cpp
    core/edit/insert_command_test.cpp
    core/edit/merge_split_command_test.cpp
    core/edit/clipboard_test.cpp
    core/edit/duration_adjustment_test.cpp
    core/edit/search_test.cpp
    core/edit/shift_command_test.cpp
    core/edit/transform_command_test.cpp
    core/edit/convert_frame_rate_command_test.cpp
    core/edit/remove_command_test.cpp
    core/edit/set_position_command_test.cpp
    core/edit/set_duration_command_test.cpp
    core/edit/snap_command_test.cpp
    core/edit/command_preview_test.cpp
    core/edit/disjoint_ranges_test.cpp
    core/edit/join_repeats_test.cpp
    core/edit/sort_command_test.cpp
    core/edit/order_policy_test.cpp
    core/command/change_test.cpp
    core/command/composite_command_test.cpp
    core/command/history_test.cpp
    core/format/read_result_test.cpp
    core/io/in_memory_file_system_test.cpp
    core/io/atomic_write_test.cpp
    core/io/find_executable_test.cpp
    core/io/find_video_test.cpp
    core/video/declared_frame_rate_test.cpp
    core/video/frame_step_test.cpp
    core/video/replica_test.cpp
    core/video/seeking_test.cpp
    core/video/showing_test.cpp
    core/io/real_file_system_test.cpp
    core/process/start_process_test.cpp
    core/process/run_and_capture_test.cpp
    core/process/fake_transcriber_test.cpp
    core/process/run_streaming_test.cpp
    core/transcription/transcription_engine_test.cpp
    core/transcription/audio_source_test.cpp
    core/transcription/json_reader_test.cpp
    core/transcription/subtitle_assembly_test.cpp
    core/transcription/transcription_wording_test.cpp
    core/transcription/whisper_cpp_engine_test.cpp
    core/transcription/whisper_cpp_output_test.cpp
    core/io/file_error_test.cpp
    core/text/lines_test.cpp
    core/text/markup_test.cpp
    core/text/html_markup_test.cpp
    core/text/sub_station_alpha_markup_test.cpp
    core/text/micro_dvd_markup_test.cpp
    core/text/markup_conversion_test.cpp
    core/text/markup_parser_test.cpp
    core/text/markup_reader_test.cpp
    core/text/utf8_test.cpp
    core/format/sub_rip_reader_test.cpp
    core/format/sub_rip_writer_test.cpp
    core/format/sub_rip_round_trip_test.cpp
    core/format/micro_dvd_test.cpp
    core/format/mpl2_test.cpp
    core/format/tm_player_test.cpp
    core/format/lrc_test.cpp
    core/format/deduced_ends_test.cpp
    core/format/sub_station_alpha_reader_test.cpp
    core/format/sub_station_alpha_writer_test.cpp
    core/format/sub_station_alpha_round_trip_test.cpp
    core/format/sub_station_alpha_syntax_test.cpp
    core/format/sub_viewer2_reader_test.cpp
    core/format/sub_viewer2_writer_test.cpp
    core/format/sub_viewer2_round_trip_test.cpp
    core/format/web_vtt_reader_test.cpp
    core/format/web_vtt_writer_test.cpp
    core/format/web_vtt_round_trip_test.cpp
    core/format/format_detection_test.cpp
    core/format/degradation_test.cpp
    core/text/encoding_test.cpp
    core/format/subtitle_file_test.cpp
    core/config/settings_test.cpp
    core/config/settings_location_test.cpp
    core/config/operation_settings_test.cpp
    core/config/duration_adjustment_settings_test.cpp
    core/config/pattern_activation_test.cpp
    core/format/open_project_test.cpp
    core/format/save_project_test.cpp
    core/format/corpus_test.cpp
    core/format/format_corpus_test.cpp
    core/format/pairs_corpus_test.cpp
    core/format/encoded_corpus_test.cpp
    core/format/local_corpus_test.cpp)

# ICU directly, for a single test: the one that sets a default locale and
# checks that case does not follow it (#401). The core keeps ICU private; the
# test, for its part, must be able to touch what the core refuses to show.
target_link_libraries(subedit_core_test PRIVATE subedit::core Catch2::Catch2WithMain ICU::uc)

# The text case harness lives at the root of unit/. Without this include
# path, a test kept in a subdirectory would designate it by a chain of "../"
# that would change with every file move.
target_include_directories(subedit_core_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# The corpus lives on disk and is read at run time. The path is passed at
# compile time rather than deduced from the current directory: a test must not
# depend on where it is started from.
target_compile_definitions(
    subedit_core_test
    PRIVATE SUBEDIT_TEST_DATA_DIR="${CMAKE_SOURCE_DIR}/src/test/data"
            SUBEDIT_PATTERNS_DIR="${CMAKE_SOURCE_DIR}/packaging/patterns"
            SUBEDIT_COMPILED_I18N_DIR="${CMAKE_CURRENT_BINARY_DIR}/i18n"
            SUBEDIT_LOCAL_DATA_DIR="${CMAKE_SOURCE_DIR}/src/data"
            SUBEDIT_FAKE_PLAYER_BINARY="$<TARGET_FILE:subedit_fake_player>"
            SUBEDIT_FAKE_TRANSCRIBER_BINARY="$<TARGET_FILE:subedit_fake_transcriber>")

# The fixture catalogues (#658): the .po files are versioned, the .mo files are compiled at build
# time by msgfmt, exactly as the shipped translations will be. A test of the reader therefore
# reads what the translators' tool produces, and not a binary written by hand.
# `SUBEDIT_MSGFMT` comes from `cmake/Translations.cmake`, which requires the tool.
set(subedit_i18n_fixtures fr ru bad-plural wording cli)
set(subedit_i18n_catalogues "")
foreach(language IN LISTS subedit_i18n_fixtures)
    set(po "${CMAKE_SOURCE_DIR}/src/test/data/i18n/${language}.po")
    set(mo "${CMAKE_CURRENT_BINARY_DIR}/i18n/${language}.mo")
    add_custom_command(
        OUTPUT "${mo}"
        COMMAND "${CMAKE_COMMAND}" -E make_directory "${CMAKE_CURRENT_BINARY_DIR}/i18n"
        COMMAND "${SUBEDIT_MSGFMT}" --output-file "${mo}" "${po}"
        DEPENDS "${po}"
        COMMENT "msgfmt ${language}.po"
        VERBATIM)
    list(APPEND subedit_i18n_catalogues "${mo}")
endforeach()
add_custom_target(subedit_i18n_fixtures DEPENDS ${subedit_i18n_catalogues})
add_dependencies(subedit_core_test subedit_i18n_fixtures)

# $<TARGET_FILE:…> gives a path, not a build order. Without this line,
# nothing guarantees that the fake player exists when a test starts it.
add_dependencies(subedit_core_test subedit_fake_player subedit_fake_transcriber)

subedit_set_warnings(subedit_core_test)

# SKIP_RETURN_CODE tells CTest what Catch2 answers it: 4 means "skipped", not
# "failed". Without this line, a test that declares itself skipped for lack of
# its local corpus makes the gate fail — on the machine that does not have that
# corpus, hence never on the one where it is written.
catch_discover_tests(subedit_core_test PROPERTIES SKIP_RETURN_CODE 4)

# One binary per library: subedit_cli has its own, like subedit_core.
add_executable(
    subedit_cli_test
    grid_fixtures.cpp
    cli/adjusting_test.cpp
    cli/aligning_test.cpp
    cli/batch_test.cpp
    cli/changes_test.cpp
    cli/conversion_test.cpp
    cli/correcting_test.cpp
    cli/destination_test.cpp
    cli/expansion_test.cpp
    cli/json_test.cpp
    cli/encoding_grammar_test.cpp
    cli/encoding_listing_test.cpp
    cli/diagnostics_test.cpp
    cli/digits_test.cpp
    cli/frame_rate_conversion_test.cpp
    cli/frame_rate_grammar_test.cpp
    cli/hearing_impaired_test.cpp
    cli/index_grammar_test.cpp
    cli/inspection_test.cpp
    cli/inspection_json_test.cpp
    cli/pairing_test.cpp
    platform/locations_test.cpp
    cli/records_test.cpp
    cli/replacing_test.cpp
    cli/appending_test.cpp
    cli/options_test.cpp
    cli/rewriting_test.cpp
    cli/reporter_test.cpp
    cli/configuring_test.cpp
    cli/shared_settings_test.cpp
    cli/shifting_test.cpp
    cli/sorting_test.cpp
    cli/transcribing_test.cpp
    cli/disjoining_test.cpp
    cli/joining_test.cpp
    cli/language_test.cpp
    cli/splitting_test.cpp
    cli/text_rewriting_test.cpp
    cli/time_grammar_test.cpp
    cli/translation_writing_test.cpp
    cli/transforming_test.cpp
    cli/verbosity_test.cpp
    cli/version_report_test.cpp
    cli/writing_test.cpp)

target_link_libraries(subedit_cli_test PRIVATE subedit::cli Catch2::Catch2WithMain)

# The grid fixtures, and the root where they live: the inspection report reads
# them, like the core, and there is only one set of files for both.
target_include_directories(subedit_cli_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_definitions(
    subedit_cli_test PRIVATE SUBEDIT_TEST_DATA_DIR="${CMAKE_SOURCE_DIR}/src/test/data"
                             SUBEDIT_FAKE_TRANSCRIBER_BINARY="$<TARGET_FILE:subedit_fake_transcriber>"
                             SUBEDIT_COMPILED_I18N_DIR="${CMAKE_CURRENT_BINARY_DIR}/i18n")

# The fixture catalogue of the command line (#663), compiled with the others above.
add_dependencies(subedit_cli_test subedit_i18n_fixtures subedit_fake_transcriber)

subedit_set_warnings(subedit_cli_test)

catch_discover_tests(subedit_cli_test)
