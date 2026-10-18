// `--version`: the program's own version, then the libraries it runs against — issue #716.
//
// **No version of a library is written here**: they are the machine's, and a number written in a
// test is true of one machine. What is stable is the first line, the names, the shape of a
// version, and that the build's every library is named by one binary or the other.

#include <subedit/core/version.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <map>
#include <regex>
#include <set>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

#include "cli_run.hpp"

using Catch::Matchers::ContainsSubstring;
using Catch::Matchers::StartsWith;
using subedit::e2e::CliRun;
using subedit::e2e::invoke;
using subedit::e2e::invokeGui;
using subedit::e2e::invokeIn;
using subedit::e2e::russian;

namespace {

[[nodiscard]] std::vector<std::string> linesOf(const std::string& text) {
    std::vector<std::string> lines;
    std::istringstream stream{text};
    for (std::string line; std::getline(stream, line);)
        lines.push_back(line);
    return lines;
}

/// The names of the libraries a text `--version` lists: the first word of every indented line
/// that ends with a version built against.
[[nodiscard]] std::set<std::string> namesInText(const std::string& output) {
    static const std::regex kLibrary{R"(^  (\S+) +[0-9][0-9.]* \(.*[0-9]\)$)"};
    std::set<std::string> names;
    for (const std::string& line : linesOf(output))
        if (std::smatch found; std::regex_match(line, found, kLibrary))
            names.insert(found[1].str());
    return names;
}

/// The names of the libraries a JSON `--version` lists.
[[nodiscard]] std::set<std::string> namesInJson(const std::string& output) {
    static const std::regex kName{
        R"re(\{"name":"([^"]+)","loaded":"[0-9][0-9.]*","built":"[0-9][0-9.]*"\})re"};
    std::set<std::string> names;
    for (auto found = std::sregex_iterator{output.begin(), output.end(), kName};
         found != std::sregex_iterator{};
         ++found)
        names.insert((*found)[1].str());
    return names;
}

/// The packages the build looks for, by the name CMake gives them: the first argument of every
/// `find_package` and `pkg_check_modules` of the build files, comments left out.
[[nodiscard]] std::set<std::string> requiredByTheBuild() {
    const std::filesystem::path root{SUBEDIT_SOURCE_DIR};
    std::vector<std::filesystem::path> files{root / "CMakeLists.txt"};
    for (const std::filesystem::path& directory : {root / "cmake", root / "src"}) {
        for (auto entry = std::filesystem::recursive_directory_iterator{directory};
             entry != std::filesystem::recursive_directory_iterator{};
             ++entry) {
            // The private corpus is each machine's own, and scratch is nobody's.
            if (entry->is_directory() &&
                (entry->path() == root / "src" / "data" || entry->path() == root / "src" / "tmp")) {
                entry.disable_recursion_pending();
                continue;
            }
            const std::string name = entry->path().filename().string();
            if (entry->is_regular_file() && (name == "CMakeLists.txt" || name.ends_with(".cmake")))
                files.push_back(entry->path());
        }
    }

    static const std::regex kCall{R"((?:find_package|pkg_check_modules)\s*\(\s*([A-Za-z0-9_]+))"};
    std::set<std::string> packages;
    for (const std::filesystem::path& file : files) {
        std::ifstream stream{file};
        for (std::string line; std::getline(stream, line);) {
            line = line.substr(0, line.find('#'));
            for (auto found = std::sregex_iterator{line.begin(), line.end(), kCall};
                 found != std::sregex_iterator{};
                 ++found)
                packages.insert((*found)[1].str());
        }
    }
    return packages;
}

} // namespace

TEST_CASE("asking for the version writes it", "[e2e][CLI-VERSION-04]") {
    const CliRun run = invoke({"--version"});

    CHECK(run.exitCode == 0);
    // Computed, never copied: a version number written by hand goes stale. The first line alone,
    // and unchanged: it is the one scripts read.
    REQUIRE_FALSE(linesOf(run.output).empty());
    CHECK(linesOf(run.output).front() == "subedit " + subedit::core::versionString());
}

TEST_CASE("the version names the libraries the command line runs against, and the languages",
          "[e2e][CLI-VERSION-06]") {
    const CliRun run = invoke({"--version"});

    CHECK(run.exitCode == 0);
    CHECK(run.errors.empty());
    CHECK(namesInText(run.output) == std::set<std::string>{"ICU", "Enchant", "CLI11"});
    // Neither Qt nor libmpv: this binary links neither.
    CHECK_THAT(linesOf(run.output).back(), StartsWith("  Languages: en"));
}

TEST_CASE("the version follows the language around its names and numbers",
          "[e2e][CLI-VERSION-06]") {
    const CliRun english = invoke({"--version"});
    const CliRun run = invokeIn(russian(), {"--version"});

    CHECK(run.exitCode == 0);
    const std::vector<std::string> lines = linesOf(run.output);
    REQUIRE(lines.size() == linesOf(english.output).size());
    // The first line is never translated.
    CHECK(lines.front() == linesOf(english.output).front());
    CHECK(namesInText(run.output) == namesInText(english.output));
    CHECK_THAT(run.output, ContainsSubstring("(собрана с "));
    // The translated copy has the fixture catalogue beside it, and nothing else.
    CHECK(lines.back() == "  Языки: en, ru");
}

TEST_CASE("the version in json is one object, wherever --format is written",
          "[e2e][CLI-VERSION-07]") {
    const CliRun after = invoke({"--version", "--format", "json"});
    const CliRun before = invoke({"--format", "json", "--version"});

    CHECK(after.exitCode == 0);
    CHECK(after.errors.empty());
    CHECK(after.output == before.output);
    REQUIRE(linesOf(after.output).size() == 1);
    CHECK_THAT(after.output,
               StartsWith(R"({"schema":1,"command":"version","version":")" +
                          subedit::core::versionString() + R"(","libraries":[)"));
    CHECK_THAT(after.output, ContainsSubstring(R"(],"languages":["en")"));
    CHECK(namesInJson(after.output) == namesInText(invoke({"--version"}).output));

    // Nothing in it is translated: the same libraries under Russian, only the languages
    // installed beside the copy differ.
    const std::string translated = invokeIn(russian(), {"--version", "--format", "json"}).output;
    CHECK(translated.substr(0, translated.find("\"languages\"")) ==
          after.output.substr(0, after.output.find("\"languages\"")));
}

TEST_CASE("every library the build requires is named by one --version or the other",
          "[e2e][gui][CLI-VERSION-08]") {
    // How each package the build looks for is named in the report. **A package the build gains
    // and that is in neither list fails here**: it needs a line in the report of the layer that
    // links it (`core/libraries.cpp`, `gui/libraries.cpp`, `exe/cli/application.cpp`), then one
    // here.
    const std::map<std::string, std::string, std::less<>> reportedAs{
        {"Qt6", "Qt"},
        {"mpv", "libmpv"},
        {"ICU", "ICU"},
        {"enchant", "Enchant"},
        {"CLI11", "CLI11"},
    };
    // What the build looks for and the binaries do not link.
    const std::set<std::string, std::less<>> notLinked{
        "PkgConfig", // a tool of the configuration, which finds libmpv and Enchant
        "Git",       // a tool of the configuration, which dates the commit
    };

    std::set<std::string> named = namesInJson(invoke({"--version", "--format", "json"}).output);
    const CliRun window = invokeGui({"--version"});
    REQUIRE(window.exitCode == 0);
    std::ranges::copy(namesInText(window.output), std::inserter(named, named.end()));

    const std::set<std::string> packages = requiredByTheBuild();
    // The parse found something: an empty set would prove nothing.
    REQUIRE(packages.contains("ICU"));
    for (const std::string& package : packages) {
        INFO(package);
        if (notLinked.contains(package))
            continue;
        const auto reported = reportedAs.find(package);
        REQUIRE(reported != reportedAs.end());
        CHECK(named.contains(reported->second));
    }
}
