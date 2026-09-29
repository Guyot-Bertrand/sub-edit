#include <subedit/core/text/pattern_paths.hpp>
#include <subedit/gui/patterns_path.hpp>

#include <catch2/catch_test_macros.hpp>

#include <cstdlib>
#include <string>

TEST_CASE("the installed patterns directory sits beside the manual, under the executable",
          "[gui][patterns]") {
    const std::filesystem::path installed = subedit::gui::installedPatternsPath();

    // `<bin>/../share/subedit/patterns` — the layout GNUInstallDirs produces,
    // the same one `installedManualPath()` already reaches for the manual.
    CHECK(installed.filename() == "patterns");
    CHECK(installed.parent_path().filename() == "subedit");
    CHECK(installed.parent_path().parent_path().filename() == "share");
}

TEST_CASE("the user's patterns directory follows XDG_DATA_HOME when it is set", "[gui][patterns]") {
    // `qputenv`/`qgetenv` would move the *process* environment, which a test
    // run in parallel with others must not do — `core::userPatternsPath` is a
    // pure function of the strings it is given, so this test reads the
    // process's actual variables through it without setting anything.
    const char* const xdgEnv = std::getenv("XDG_DATA_HOME");
    const char* const homeEnv = std::getenv("HOME");
    const std::string xdg = xdgEnv != nullptr ? xdgEnv : "";
    const std::string home = homeEnv != nullptr ? homeEnv : "";
    const std::filesystem::path expected = subedit::core::userPatternsPath(xdg, home);

    CHECK(subedit::gui::resolvedUserPatternsPath() == expected);
}
