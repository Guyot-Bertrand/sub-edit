# Un binaire de test par bibliothèque, chaque cas étant enregistré
# individuellement dans CTest par catch_discover_tests.

add_executable(
    subedit_core_test
    core/version_test.cpp
    core/time/duration_test.cpp
    core/time/frame_test.cpp
    core/time/frame_rate_test.cpp
    core/time/timestamp_test.cpp
    core/time/timestamp_conversion_test.cpp
    core/time/timestamp_format_test.cpp
    core/time/timestamp_parse_test.cpp
    core/model/format_extras_test.cpp
    core/model/subtitle_test.cpp
    core/model/project_test.cpp
    core/model/subtitle_index_test.cpp
    core/model/project_access_test.cpp
    core/command/change_test.cpp
    core/command/composite_command_test.cpp
    core/command/history_test.cpp)

target_link_libraries(subedit_core_test PRIVATE subedit::core Catch2::Catch2WithMain)

subedit_set_warnings(subedit_core_test)

catch_discover_tests(subedit_core_test)
