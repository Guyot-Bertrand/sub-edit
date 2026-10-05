// Where the platform says things are — what does not need a launched program.
//
// The executable directory is the test binary's own, which is not the layout the
// patterns are looked for in; what is checked here is that the answer is a real
// directory and that the two resolved locations are the pure functions of the core
// applied to what the process has. The layout itself is proved by
// `e2e/cli/pattern_catalogue_test.cpp`, through a launched program.

#include <subedit/core/text/pattern_paths.hpp>
#include <subedit/platform/locations.hpp>

#include <catch2/catch_test_macros.hpp>

#include <cstdlib>
#include <filesystem>
#include <string>

TEST_CASE("the executable directory is a real directory holding the running binary",
          "[platform][patterns]") {
    const std::filesystem::path directory = subedit::platform::executableDirectory();

    REQUIRE_FALSE(directory.empty());
    CHECK(directory.is_absolute());
    CHECK(std::filesystem::is_directory(directory));
}

TEST_CASE("the installed patterns directory is the shipped one, seen from the executable",
          "[platform][patterns]") {
    const std::filesystem::path installed = subedit::platform::installedPatternsPath();

    // `<bin>/../share/subedit/patterns` — the layout GNUInstallDirs produces,
    // the one `installedManualPath()` reaches for the manual.
    CHECK(installed ==
          subedit::core::shippedPatternsPath(subedit::platform::executableDirectory()));
    CHECK(installed.filename() == "patterns");
    CHECK(installed.parent_path().filename() == "subedit");
    CHECK(installed.parent_path().parent_path().filename() == "share");
}

TEST_CASE("the user's patterns directory follows the variables the process has",
          "[platform][patterns]") {
    // Nothing is set here: the process's own variables are read through the pure
    // function, which a test running beside others may do and may not undo.
    const char* const xdgEnv = std::getenv("XDG_DATA_HOME");
    const char* const homeEnv = std::getenv("HOME");
    const std::string xdg = xdgEnv != nullptr ? xdgEnv : "";
    const std::string home = homeEnv != nullptr ? homeEnv : "";

    CHECK(subedit::platform::resolvedUserPatternsPath() ==
          subedit::core::userPatternsPath(xdg, home));
}
