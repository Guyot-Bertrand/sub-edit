// Where the command line finds its patterns, through a real executable.
//
// The resolution is of the executable's own directory and of the environment, so
// only a launched program proves it: a unit test would resolve the test binary's
// directory, which is not the layout under test. The program is the probe of
// `tools/pattern_catalogue.cpp`, which resolves as `subedit-cli` will.

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include <filesystem>
#include <fstream>
#include <string>

#include "cli_run.hpp"

using Catch::Matchers::ContainsSubstring;
using subedit::e2e::CliRun;
using subedit::e2e::dataHome;
using subedit::e2e::invokePatternCatalogue;

namespace {

/// A directory removed when it goes out of scope, and the file in it: what a
/// test drops in the user's patterns must not outlive the test, or the next
/// case would read it.
class DroppedPatterns {
public:
    DroppedPatterns(const std::string& name, const std::string& text)
        : m_directory{std::filesystem::path{dataHome()} / "subedit" / "patterns"} {
        std::filesystem::create_directories(m_directory);
        std::ofstream{m_directory / name} << text;
    }

    DroppedPatterns(const DroppedPatterns&) = delete;
    DroppedPatterns& operator=(const DroppedPatterns&) = delete;
    DroppedPatterns(DroppedPatterns&&) = delete;
    DroppedPatterns& operator=(DroppedPatterns&&) = delete;

    ~DroppedPatterns() {
        std::error_code ignored;
        std::filesystem::remove_all(std::filesystem::path{dataHome()} / "subedit", ignored);
    }

private:
    std::filesystem::path m_directory;
};

[[nodiscard]] int patternsIn(const std::string& output) {
    const std::string key = "patterns: ";
    const std::size_t at = output.find("\n" + key);
    return std::stoi(output.substr(at + 1 + key.size()));
}

} // namespace

TEST_CASE("the shipped patterns are found from the executable, in the build tree",
          "[e2e][patterns]") {
    const CliRun run = invokePatternCatalogue();

    REQUIRE(run.exitCode == 0);
    // The layout of an installation, reproduced beside `bin/`: the link CMake
    // lays, not an option the shipped program would have to carry.
    CHECK_THAT(run.output, ContainsSubstring("share/subedit/patterns\n"));
    CHECK(patternsIn(run.output) > 50);
    CHECK_THAT(run.output, ContainsSubstring("Zyyy: "));
    CHECK_THAT(run.output, ContainsSubstring("diagnostics: 0\n"));
}

TEST_CASE("the user's directory is the one the harness moved, and holds nothing by default",
          "[e2e][patterns]") {
    const CliRun run = invokePatternCatalogue();

    REQUIRE(run.exitCode == 0);
    CHECK_THAT(run.output, ContainsSubstring("user: " + dataHome() + "/subedit/patterns\n"));
    CHECK_FALSE(std::filesystem::exists(std::filesystem::path{dataHome()} / "subedit"));
}

TEST_CASE("a file of patterns dropped in the user's directory is added to the catalogue",
          "[e2e][patterns]") {
    const int shipped = patternsIn(invokePatternCatalogue().output);

    const DroppedPatterns dropped{"Latn-xx.common-error",
                                  "[Common Error Pattern]\n"
                                  "Name=A pattern of the test\n"
                                  "Description=Dropped by the harness\n"
                                  "Classes=OCR;\n"
                                  "Pattern=zzz\n"
                                  "Flags=DOTALL;MULTILINE;\n"
                                  "Replacement=yyy\n"
                                  "Repeat=False\n"};
    const CliRun run = invokePatternCatalogue();

    REQUIRE(run.exitCode == 0);
    CHECK(patternsIn(run.output) == shipped + 1);
    CHECK_THAT(run.output, ContainsSubstring("Latn-xx: A pattern of the test\n"));
    CHECK_THAT(run.output, ContainsSubstring("diagnostics: 0\n"));
}
