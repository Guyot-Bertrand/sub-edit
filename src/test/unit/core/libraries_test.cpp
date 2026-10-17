// Which libraries the core runs against, and how `--version` says it — issue #716.

#include <subedit/core/libraries.hpp>
#include <subedit/core/version.hpp>
#include <subedit/core/wording/version.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include <regex>
#include <sstream>
#include <string>
#include <vector>

using Catch::Matchers::StartsWith;
using subedit::core::Library;

namespace {

/// A version as the libraries write theirs: numbers and dots, nothing else.
[[nodiscard]] bool looksLikeAVersion(const std::string& text) {
    return std::regex_match(text, std::regex{R"([0-9]+(\.[0-9]+)*)"});
}

[[nodiscard]] std::vector<std::string> linesOf(const std::string& text) {
    std::vector<std::string> lines;
    std::istringstream stream{text};
    for (std::string line; std::getline(stream, line);)
        lines.push_back(line);
    return lines;
}

} // namespace

TEST_CASE("the core names ICU and Enchant, each with both of its versions", "[version]") {
    const std::vector<Library> libraries = subedit::core::coreLibraries();

    REQUIRE(libraries.size() == 2);
    CHECK(libraries.at(0).name == "ICU");
    CHECK(libraries.at(1).name == "Enchant");
    // Present and shaped like a version, never a fixed number: they are the machine's.
    for (const Library& library : libraries) {
        INFO(library.name);
        CHECK(looksLikeAVersion(library.loaded));
        CHECK(looksLikeAVersion(library.built));
    }
}

TEST_CASE("the report keeps the version alone on its first line", "[version]") {
    const std::string report =
        subedit::core::versionReport({{.name = "ICU", .loaded = "74.2", .built = "74.1"},
                                      {.name = "Enchant", .loaded = "2.3.3", .built = "2.3.3"}},
                                     {"en", "fr"});

    CHECK(linesOf(report) == std::vector<std::string>{
                                 "subedit " + subedit::core::versionString(),
                                 "  ICU      74.2 (built with 74.1)",
                                 "  Enchant  2.3.3 (built with 2.3.3)",
                                 "  Languages: en, fr",
                             });
    CHECK_THAT(report, StartsWith("subedit "));
    CHECK(report.back() == '\n');
}

TEST_CASE("the languages line lists the codes it is given", "[version]") {
    CHECK(subedit::core::languagesLine({"en"}) == "Languages: en");
    CHECK(subedit::core::languagesLine({"en", "fr", "ru"}) == "Languages: en, fr, ru");
}
