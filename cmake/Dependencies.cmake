# External dependencies.
#
# Project rule: `find_package` on system packages, so that adopting Conan or
# vcpkg later stays almost free — both work by interposing themselves on that
# mechanism. See docs/adr/0004-gestion-des-dependances.md.
#
# Catch2 is the exception: it compiles in a few seconds and its version must be
# the same everywhere, lest a distribution update break the tests.

include(FetchContent)

set(CATCH2_VERSION "v3.5.2")

FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG ${CATCH2_VERSION}
    GIT_SHALLOW TRUE
    SYSTEM) # headers treated as system ones: their warnings are not ours

FetchContent_MakeAvailable(Catch2)

# **Catch2 is not our code, and the `tidy` preset does not lint it.**
# `CMAKE_CXX_CLANG_TIDY` is a directory variable: every target added after it
# inherits it, including those a dependency adds on its own account. Without
# this loop, the analysis would cover a superset of `src/` and report defects
# that nobody here can fix.
#
# The `SYSTEM` of the declaration above is not enough: it treats Catch2's
# headers as system headers for *our* translation units, and says nothing of
# its own.
foreach(subedit_foreign IN ITEMS Catch2 Catch2WithMain)
    if(TARGET ${subedit_foreign})
        set_target_properties(${subedit_foreign} PROPERTIES CXX_CLANG_TIDY "")
    endif()
endforeach()

list(APPEND CMAKE_MODULE_PATH "${catch2_SOURCE_DIR}/extras")
include(Catch)
