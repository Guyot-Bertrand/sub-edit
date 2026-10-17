# Static analysis, hooked to the compilation of each source.
#
# **This module is active only under the `tidy` preset**, and does nothing
# elsewhere: `SUBEDIT_TIDY` is `OFF` everywhere else, so the development loop
# never pays the six seconds per file that clang-tidy costs.
#
# ## Why the compilation, and not a script that chooses
#
# The scope of the analysis used to be computed by `tidy-scope.sh`, from a git
# diff: what changed since a base, plus the closure of headers, plus a list of
# "governing files" that made everything start over. It approximated, with two
# defects that measurement showed:
#
#   * the header closure was done by a `grep` of the **file name** in the
#     sources — a header mentioned in a comment counted;
#   * a comment added to the `Makefile` took the analysis from 2 files to 223,
#     that is 72 s against 827. Issue #269.
#
# Yet clang-tidy never reads the `Makefile`. What the result of a translation
# unit depends on is exactly: its source, its transitive headers, its compile
# command line, the `.clang-tidy` configuration, and the clang-tidy binary. The
# first three, **the build system already knows**, exactly and without
# heuristics: Ninja replays a rule when the source changes, when a header of
# its dependency file changes, or when the command line changes.
#
# What remains is to teach it the last two, which the key below does.

option(SUBEDIT_TIDY "clang-tidy analysis on the compilation of each source" OFF)

if(NOT SUBEDIT_TIDY)
    return()
endif()

# libstdc++ keeps <expected> behind __cpp_concepts >= 202002L, a value that
# Clang 18 does not declare: it then does not see std::expected. We therefore
# take the most recent version available.
find_program(
    SUBEDIT_CLANG_TIDY
    NAMES clang-tidy-20 clang-tidy-19 clang-tidy
    DOC "clang-tidy binary used by the tidy preset")

if(NOT SUBEDIT_CLANG_TIDY)
    message(
        FATAL_ERROR
        "clang-tidy cannot be found, and the tidy preset has nothing to do without it.\n"
        "  install it with: ./src/scripts/setup-toolchain.sh")
endif()

# ## The two inputs Ninja does not see, and the key that shows them to it
#
# Ninja replays a rule when its command line changes. The `.clang-tidy`
# configuration is not on it — clang-tidy reads it itself, on the fly —, and
# neither is the binary, of which only the path appears. Changing the one or
# updating the other would therefore replay **nothing**, and the analysis would
# stay green on a stale answer.
#
# **That is the worst failure mode possible**: quieter than the one it
# replaces, since the old one erred by analysing too much. We close it by
# putting the fingerprint of both in the **compile flags**, as a definition
# that nobody uses.
#
# In the flags and not in clang-tidy's arguments, and the nuance decides: "the
# command line changed, so I replay" is a property of **Ninja**, which keeps its
# fingerprint. The Makefile generator does not know it — but it makes each
# object depend on its `flags.make`, so a compile definition that changes
# recompiles it. Going through the flags makes the guarantee true with both,
# and the project does not fix its generator (see `CMAKE_GENERATOR` in the
# Makefile: Ninja is preferred, not required).
execute_process(
    COMMAND "${SUBEDIT_CLANG_TIDY}" --version
    OUTPUT_VARIABLE subedit_tidy_version
    ERROR_VARIABLE subedit_tidy_version
    OUTPUT_STRIP_TRAILING_WHITESPACE)

file(READ "${CMAKE_SOURCE_DIR}/.clang-tidy" subedit_tidy_config)
string(SHA256 subedit_tidy_key "${subedit_tidy_version}${subedit_tidy_config}")
string(SUBSTRING "${subedit_tidy_key}" 0 16 subedit_tidy_key)

# And for the key to be recomputed when the file moves, the configuration must
# rerun: without this line, CMake never looks at `.clang-tidy`.
set_property(
    DIRECTORY "${CMAKE_SOURCE_DIR}"
    APPEND
    PROPERTY CMAKE_CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/.clang-tidy")

# `compile_commands.json` carries GCC's flags, some of which do not exist in
# Clang: without -Wno-unknown-warning-option, clang-tidy fails on
# -Wuseless-cast and the like before analysing a single line.
#
# `WarningsAsErrors: '*'` is in `.clang-tidy`: a finding therefore makes the
# compilation fail, and the gate keeps its teeth.
#
# No `PARENT_SCOPE`: `include()` creates no scope, so what is set here is set
# in the including file, and every target added afterwards inherits it.
add_compile_definitions(__SUBEDIT_TIDY_KEY=${subedit_tidy_key})

set(CMAKE_CXX_CLANG_TIDY
    "${SUBEDIT_CLANG_TIDY}"
    "--quiet"
    "--extra-arg=-Wno-unknown-warning-option")

message(STATUS "clang-tidy   : ${SUBEDIT_CLANG_TIDY} (key ${subedit_tidy_key})")
