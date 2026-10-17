# Caps the parallelism of cross-module optimisation.
#
# `CMAKE_INTERPROCEDURAL_OPTIMIZATION` turns LTO into `-flto=auto` with GCC,
# that is "as many processes as cores", **at every link**. That parallelism
# appears in no `-j`: it escapes the sober setting of the Makefile, which
# defaults to 2, and saturates a machine on which something else is being done
# — another project's tests, for instance, which then fail on timeouts they
# should not have exceeded.
#
# This file therefore gives LTO the same knob as the rest: low by default,
# raised explicitly by whoever knows what their machine can take.
#
#   cmake --preset release -DSUBEDIT_LTO_JOBS=8
#
# The flag is added after the one CMake sets; with GCC, the last `-flto=`
# wins.

set(SUBEDIT_LTO_JOBS
    1
    CACHE STRING "Parallel processes for cross-module optimisation")

if(CMAKE_INTERPROCEDURAL_OPTIMIZATION AND CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    add_link_options("-flto=${SUBEDIT_LTO_JOBS}")

    # **One link at a time.** `-flto=N` caps the processes of ONE link; with `-j2`, Ninja starts
    # two side by side, and the machine sees 2 × (1 + N) `lto1` processes — four to six for
    # `N = 2`, which a user observed. A pool of one link brings the peak down to `N` (plus the
    # neighbouring compilation). No effect with the Makefile generator, which knows no pools: the
    # `release` preset is Ninja.
    set_property(GLOBAL APPEND PROPERTY JOB_POOLS subedit_link=1)
    set(CMAKE_JOB_POOL_LINK subedit_link)
    message(STATUS "  LTO         : -flto=${SUBEDIT_LTO_JOBS}")
endif()
