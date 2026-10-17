# Instrumentation for coverage measurement.
#
# The report is produced by gcovr, driven from the Makefile: CMake only
# instruments.

if(NOT SUBEDIT_COVERAGE)
    return()
endif()

if(NOT CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    message(FATAL_ERROR "Coverage is configured for GCC only.")
endif()

add_compile_options(--coverage -fprofile-abs-path -fno-inline -fno-elide-constructors)
add_link_options(--coverage)

message(STATUS "coverage instrumented")
