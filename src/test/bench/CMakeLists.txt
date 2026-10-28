# The benchmarks are a separate executable, not registered with CTest: they
# are measured in Release, not during an instrumented test pass.
#
# **A single binary, core and interface together.** `record-bench.sh` reads
# only one XML output, and two binaries would require either merging it, or
# two readings in the log for the same version. Qt is required to build the
# project anyway.
set(CMAKE_AUTOMOC ON)

add_executable(
    subedit_bench
    main.cpp
    full_length_project.cpp
    cli/batch_bench.cpp
    core/analysis/frame_rate_deduction_bench.cpp
    core/edit/operations_bench.cpp
    core/format/subtitle_file_bench.cpp
    core/i18n/catalogue_bench.cpp
    core/text/common_errors_bench.cpp
    core/text/line_breaking_bench.cpp
    core/time/timestamp_bench.cpp
    core/transcription/subtitle_assembly_bench.cpp
    core/transcription/word_matching_bench.cpp
    core/version_bench.cpp
    gui/ems_line_breaking_bench.cpp
    gui/subtitle_table_model_bench.cpp
    gui/video_bench.cpp)

# Catch2 without its main — see main.cpp: the QApplication must live on its
# stack, as in subedit_gui_test.
target_link_libraries(
    subedit_bench PRIVATE subedit::gui subedit::cli subedit::core Qt6::Widgets Catch2::Catch2)

# The shared fixture lives at the root of bench/, as text_cases.hpp lives at
# that of unit/. Without this include path, a benchmark kept in a subdirectory
# would designate it by a chain of "../" that would change with every file
# move.
target_include_directories(subedit_bench PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# The reading of the numbered image, shared with the interface tests (#610): the bench of the
# jump and the step checks the gesture it measures with the same oracle as the tests, and not
# with a second reading that could differ from it.
target_include_directories(subedit_bench PRIVATE ${CMAKE_SOURCE_DIR}/src/test/gui)

# The video fixtures of #163, for the only measurement that opens a real file:
# what opening and seeking cost, that is what happens at every change of
# line.
target_compile_definitions(
    subedit_bench PRIVATE SUBEDIT_TEST_DATA_DIR="${CMAKE_SOURCE_DIR}/src/test/data"
                          SUBEDIT_PATTERNS_DIR="${CMAKE_SOURCE_DIR}/packaging/patterns"
                          SUBEDIT_SHIPPED_LOCALE_DIR="${SUBEDIT_LOCALE_BUILD_DIR}")

# The catalogues the bench of catalogue loading reads (#684).
add_dependencies(subedit_bench subedit_locale)

subedit_set_warnings(subedit_bench)
