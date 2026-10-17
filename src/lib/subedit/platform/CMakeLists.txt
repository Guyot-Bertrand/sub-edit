# subedit_platform — where the executable and the environment say things are.
#
# Neither the core, which reads no variable and asks no executable where it
# lives (ADR 0037), nor an interface: what answers once, without Qt, for the
# command line as for the window. Issue #566.

add_library(subedit_platform STATIC locations.cpp)
add_library(subedit::platform ALIAS subedit_platform)

target_include_directories(subedit_platform PUBLIC "${CMAKE_SOURCE_DIR}/src/lib")

target_link_libraries(subedit_platform PUBLIC subedit::core)

subedit_set_warnings(subedit_platform)
