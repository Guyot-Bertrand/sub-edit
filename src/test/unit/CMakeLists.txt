# Un binaire de test par bibliothèque, chaque cas étant enregistré
# individuellement dans CTest par catch_discover_tests.

add_executable(
    subedit_core_test
    text_cases.cpp
    await_process.cpp
    text_cases_test.cpp
    core/version_test.cpp
    core/wording_test.cpp
    core/edit/shift_limits_test.cpp
    core/time/duration_test.cpp
    core/time/frame_test.cpp
    core/time/frame_rate_test.cpp
    core/time/ratio_test.cpp
    core/time/scaling_test.cpp
    core/time/timestamp_test.cpp
    core/time/timestamp_conversion_test.cpp
    core/time/timestamp_format_test.cpp
    core/time/timestamp_parse_test.cpp
    core/text/hearing_impaired_test.cpp
    core/model/format_extras_test.cpp
    core/model/subtitle_test.cpp
    core/model/project_test.cpp
    core/model/subtitle_index_test.cpp
    core/model/project_access_test.cpp
    core/model/anomaly_test.cpp
    core/model/project_edit_test.cpp
    core/model/selection_test.cpp
    core/edit/session_test.cpp
    core/edit/set_text_command_test.cpp
    core/edit/hearing_impaired_removal_test.cpp
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
    core/io/in_memory_file_system_test.cpp
    core/io/atomic_write_test.cpp
    core/io/find_executable_test.cpp
    core/io/real_file_system_test.cpp
    core/process/start_process_test.cpp
    core/io/file_error_test.cpp
    core/text/lines_test.cpp
    core/format/sub_rip_reader_test.cpp
    core/format/sub_rip_writer_test.cpp
    core/format/sub_rip_round_trip_test.cpp
    core/format/web_vtt_reader_test.cpp
    core/format/web_vtt_writer_test.cpp
    core/format/web_vtt_round_trip_test.cpp
    core/format/format_detection_test.cpp
    core/text/encoding_test.cpp
    core/format/subtitle_file_test.cpp
    core/format/corpus_test.cpp
    core/format/local_corpus_test.cpp)

target_link_libraries(subedit_core_test PRIVATE subedit::core Catch2::Catch2WithMain)

# Le harnais de cas de texte vit à la racine de unit/. Sans ce chemin
# d'inclusion, un test rangé dans un sous-répertoire le désignerait par une
# suite de « ../ » qui changerait à chaque déplacement de fichier.
target_include_directories(subedit_core_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# Le corpus vit sur le disque et se lit à l'exécution. Le chemin est passé à la
# compilation plutôt que déduit du répertoire courant : un test ne doit pas
# dépendre de l'endroit d'où on le lance.
target_compile_definitions(
    subedit_core_test
    PRIVATE SUBEDIT_TEST_DATA_DIR="${CMAKE_SOURCE_DIR}/src/test/data"
            SUBEDIT_LOCAL_DATA_DIR="${CMAKE_SOURCE_DIR}/src/data"
            SUBEDIT_FAKE_PLAYER_BINARY="$<TARGET_FILE:subedit_fake_player>")

# $<TARGET_FILE:…> donne un chemin, pas un ordre de construction. Sans cette
# ligne, rien ne garantit que le faux lecteur existe quand un test le lance.
add_dependencies(subedit_core_test subedit_fake_player)

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
    cli/digits_test.cpp
    cli/frame_rate_conversion_test.cpp
    cli/frame_rate_grammar_test.cpp
    cli/hearing_impaired_test.cpp
    cli/index_grammar_test.cpp
    cli/inspection_test.cpp
    cli/reporter_test.cpp
    cli/shifting_test.cpp
    cli/time_grammar_test.cpp
    cli/transforming_test.cpp
    cli/verbosity_test.cpp)

target_link_libraries(subedit_cli_test PRIVATE subedit::cli Catch2::Catch2WithMain)

subedit_set_warnings(subedit_cli_test)

catch_discover_tests(subedit_cli_test)
