# Uses ccache if it is installed, ignores it otherwise.
#
# Not a dependency: ccache speeds rebuilds up, it conditions none of them.

find_program(CCACHE_EXECUTABLE ccache)

if(CCACHE_EXECUTABLE)
    set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_EXECUTABLE}")
    message(STATUS "ccache: ${CCACHE_EXECUTABLE}")
endif()
