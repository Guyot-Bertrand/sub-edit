# Un binaire de test par bibliothèque, chaque cas étant enregistré
# individuellement dans CTest par catch_discover_tests.

add_executable(
    subedit_core_test
    core/version_test.cpp
    core/time/duration_test.cpp
    core/time/frame_test.cpp
    core/time/frame_rate_test.cpp
    core/time/ratio_test.cpp
    core/time/scaling_test.cpp
    core/time/timestamp_test.cpp
    core/time/timestamp_conversion_test.cpp
    core/time/timestamp_format_test.cpp
    core/time/timestamp_parse_test.cpp
    core/model/format_extras_test.cpp
    core/model/subtitle_test.cpp
    core/model/project_test.cpp
    core/model/subtitle_index_test.cpp
    core/model/project_access_test.cpp
    core/model/project_edit_test.cpp
    core/model/selection_test.cpp
    core/edit/session_test.cpp
    core/edit/set_text_command_test.cpp
    core/edit/insert_command_test.cpp
    core/edit/shift_command_test.cpp
    core/edit/transform_command_test.cpp
    core/edit/convert_frame_rate_command_test.cpp
    core/edit/remove_command_test.cpp
    core/edit/set_position_command_test.cpp
    core/edit/sort_command_test.cpp
    core/edit/order_policy_test.cpp
    core/command/change_test.cpp
    core/command/composite_command_test.cpp
    core/command/history_test.cpp
    core/format/read_result_test.cpp
    core/format/in_memory_file_system_test.cpp
    core/format/atomic_write_test.cpp
    core/format/real_file_system_test.cpp
    core/format/file_error_test.cpp
    core/format/lines_test.cpp
    core/format/sub_rip_reader_test.cpp
    core/format/sub_rip_writer_test.cpp
    core/format/sub_rip_round_trip_test.cpp
    core/format/web_vtt_reader_test.cpp
    core/format/web_vtt_writer_test.cpp
    core/format/web_vtt_round_trip_test.cpp
    core/format/subtitle_format_test.cpp
    core/format/encoding_test.cpp
    core/format/subtitle_file_test.cpp
    core/format/corpus_test.cpp
    core/format/local_corpus_test.cpp)

target_link_libraries(subedit_core_test PRIVATE subedit::core Catch2::Catch2WithMain)

# Le corpus vit sur le disque et se lit à l'exécution. Le chemin est passé à la
# compilation plutôt que déduit du répertoire courant : un test ne doit pas
# dépendre de l'endroit d'où on le lance.
target_compile_definitions(
    subedit_core_test
    PRIVATE SUBEDIT_TEST_DATA_DIR="${CMAKE_SOURCE_DIR}/src/test/data"
            SUBEDIT_LOCAL_DATA_DIR="${CMAKE_SOURCE_DIR}/src/data")

subedit_set_warnings(subedit_core_test)

# SKIP_RETURN_CODE dit à CTest ce que Catch2 lui répond : 4 signifie « ignoré »,
# pas « échoué ». Sans cette ligne, un test qui se déclare ignoré faute de son
# corpus local fait échouer la porte — sur la machine qui n'a pas ce corpus,
# donc jamais sur celle où on l'écrit.
catch_discover_tests(subedit_core_test PROPERTIES SKIP_RETURN_CODE 4)

# Un binaire par bibliothèque : subedit_cli a le sien, comme subedit_core.
add_executable(
    subedit_cli_test
    cli/batch_test.cpp
    cli/conversion_test.cpp
    cli/destination_test.cpp
    cli/diagnostics_test.cpp
    cli/frame_rate_conversion_test.cpp
    cli/frame_rate_grammar_test.cpp
    cli/index_grammar_test.cpp
    cli/inspection_test.cpp
    cli/reporter_test.cpp
    cli/shifting_test.cpp
    cli/time_grammar_test.cpp
    cli/transforming_test.cpp
    cli/verbosity_test.cpp
    cli/wording_test.cpp)

target_link_libraries(subedit_cli_test PRIVATE subedit::cli Catch2::Catch2WithMain)

subedit_set_warnings(subedit_cli_test)

catch_discover_tests(subedit_cli_test)
