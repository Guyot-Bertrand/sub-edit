cmake_minimum_required(VERSION 3.28)

project(
    subedit
    VERSION 1.1.5
    DESCRIPTION "Subtitle editor"
    HOMEPAGE_URL "https://github.com/Guyot-Bertrand/sub-edit"
    LANGUAGES CXX)

# This project is never built as a subproject of another.
if(NOT CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    message(FATAL_ERROR "subedit is built on its own, not as a subproject.")
endif()

# C++23 for std::expected — see docs/adr/0007-cpp23-et-std-expected.md.
# Requires clang-tidy >= 19: version 18 does not see <expected>.
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# GNU extensions would make the code non-portable without anyone noticing.
set(CMAKE_CXX_EXTENSIONS OFF)

# Consumed by clang-tidy and by editors.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

option(SUBEDIT_BUILD_TESTS "Build the tests and the benchmarks" ON)
option(SUBEDIT_SANITIZERS "Enable AddressSanitizer and UndefinedBehaviorSanitizer" OFF)
option(SUBEDIT_COVERAGE "Instrument for coverage measurement" OFF)
# OFF by default; only the asan and release presets set it to ON. A
# declarative flag reads better six months later than an if() composed over
# CMAKE_BUILD_TYPE and SUBEDIT_SANITIZERS — see src/test/e2e/CMakeLists.txt
# for the reason for this choice of presets.
option(SUBEDIT_REGISTER_E2E "Register the end-to-end tests with CTest" OFF)

# Build date: the commit's, not the clock's. Two builds of the same source
# then give the same binary — which the clock would forbid, and which ccache
# would pay for in useless recompilations. The date is refreshed at
# configuration, which is enough for its use: saying which state of the source
# a binary comes from.
set(SUBEDIT_BUILD_DATE "")
# Qt is required since phase 5, and no longer optional: the window is the
# heart of what the project delivers.
#
# Test comes along with Widgets: it carries the functions that simulate a
# click or a keystroke, which the interface harness uses from Catch2 — without
# being a QTest binary for all that.
#
# **Multimedia used to be declared here, and no longer is.** It had been "for
# the phase 6 preview"; ADR 0020 settled the video backend in favour of libmpv,
# and nobody ever linked it. A dependency that nothing uses is not free: it is
# in the CI's package cache, and the line that declared it announced a use
# that will not exist.
find_package(Qt6 REQUIRED COMPONENTS Widgets Test Concurrent)

# libmpv — the engine of the embedded player, see
# `docs/adr/0020-libmpv-pour-le-lecteur-integre.md`.
#
# Through pkg-config and not find_package: libmpv provides no CMake
# configuration file, only an `mpv.pc`. `IMPORTED_TARGET` derives
# `PkgConfig::mpv` from it, which carries its include paths and its link flags
# as a Qt target would — so nothing to copy by hand into the targets that use
# it.
#
# Declared at the root, like Qt, so that its absence shows at configuration: a
# missing library discovered in the middle of a build costs the time of the
# build.
find_package(PkgConfig REQUIRED)
pkg_check_modules(mpv REQUIRED IMPORTED_TARGET mpv)

# ICU — the conversion of encodings, and their detection. See
# `docs/adr/0027-icu-pour-les-encodages.md`.
#
# **The first external dependency of the core**, and therefore of
# `subedit-cli`, which had none: that is the written price of ADR 0027, paid
# for the ninety-seven encodings that iso-functionality requires and that are
# not written by hand.
#
# Two components, each for a written use: `ucnv` converts, and lives in `uc`;
# `ucsdet` detects, and lives in `i18n`. The second was declared only with the
# detection that uses it, rather than announced in advance — that is what
# Qt::Multimedia cost us to learn.
find_package(ICU REQUIRED COMPONENTS uc i18n)

# Enchant 2 — the spell checker, D6 of docs/specs/12-correction.md.
#
# **The only dependency of phase 12**, and the second of the core, hence of
# `subedit-cli`: the spell checker is in the core, and phase 13 will join and
# split words on the command line. Enchant keeps the personal word list in its
# own directory, shared by every application that uses it — Gaupol's
# included —, and finds the system's dictionaries, whatever their engine.
#
# Through pkg-config, like libmpv: Enchant provides only an `enchant-2.pc`.
pkg_check_modules(enchant REQUIRED IMPORTED_TARGET enchant-2)

find_package(Git QUIET)
if(GIT_FOUND)
    execute_process(
        COMMAND "${GIT_EXECUTABLE}" log -1 --format=%cs
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        OUTPUT_VARIABLE SUBEDIT_COMMIT_DATE
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
        RESULT_VARIABLE SUBEDIT_GIT_STATUS)
    if(SUBEDIT_GIT_STATUS EQUAL 0)
        set(SUBEDIT_BUILD_DATE "${SUBEDIT_COMMIT_DATE}")
    endif()
endif()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(ToolchainSanity)
include(Ccache)
include(Warnings)
include(Sanitizers)
include(Coverage)
include(Lto)

# Before the targets, and never after: `CMAKE_CXX_CLANG_TIDY` is read when a
# target is added. Inert outside the `tidy` preset. Issue #269.
include(Tidy)

add_subdirectory(src/lib)
add_subdirectory(src/exe)

# After the targets: `install(TARGETS)` can only name what exists.
include(Installation)
include(Translations)
# After the translations: it generates a set of pages per installed language.
include(ManPages)

# After the installation, and never before: CPack packages what the
# `install()` rules lay down, so it has nothing to package until they are
# read. ADR 0023.
include(Packaging)

if(SUBEDIT_BUILD_TESTS)
    include(CTest)
    include(Dependencies)
    add_subdirectory(src/test)
endif()

message(STATUS "subedit ${PROJECT_VERSION} — ${CMAKE_BUILD_TYPE}")
message(STATUS "  tests       : ${SUBEDIT_BUILD_TESTS}")
message(STATUS "  sanitizers  : ${SUBEDIT_SANITIZERS}")
message(STATUS "  coverage    : ${SUBEDIT_COVERAGE}")
