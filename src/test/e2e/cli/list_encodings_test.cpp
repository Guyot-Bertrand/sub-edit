// `--list-encodings`: what the tool can read and write, one name a line.
//
// **No test fixes the size of the list, nor all of its names**: the set is what the
// ICU installed on the machine converts, and a count written here would be true
// of one machine. What is stable is a handful of names that every ICU has, and
// the two ICU knows and the model refuses.

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include <algorithm>
#include <cstddef>
#include <sstream>
#include <string>
#include <vector>

#include "cli_run.hpp"

using Catch::Matchers::ContainsSubstring;
using subedit::e2e::CliRun;
using subedit::e2e::invoke;

namespace {

std::vector<std::string> linesOf(const std::string& text) {
    std::vector<std::string> lines;
    std::istringstream stream{text};
    for (std::string line; std::getline(stream, line);) {
        lines.push_back(line);
    }
    return lines;
}

bool contains(const std::vector<std::string>& names, const std::string& name) {
    return std::ranges::find(names, name) != names.end();
}

} // namespace

TEST_CASE("the list holds the encodings every ICU converts, one a line", "[e2e][CLI-LISTENC-01]") {
    const CliRun run = invoke({"--list-encodings"});

    CHECK(run.exitCode == 0);
    CHECK(run.errors.empty());
    const std::vector<std::string> names = linesOf(run.output);
    CHECK(names.size() > 50);
    CHECK(contains(names, "UTF-8"));
    CHECK(contains(names, "windows-1252"));
    CHECK(contains(names, "UTF-16LE"));
}

TEST_CASE("the two names whose converter writes its own mark are not in it",
          "[e2e][CLI-LISTENC-01]") {
    const std::vector<std::string> names = linesOf(invoke({"--list-encodings"}).output);

    // ICU knows them, the model refuses them (`--encoding UTF-16` says why), and a
    // list one picks from must not offer what the next field would refuse.
    CHECK_FALSE(contains(names, "UTF-16"));
    CHECK_FALSE(contains(names, "UTF-32"));
}

TEST_CASE("the list is sorted by name and has no repeats", "[e2e][CLI-LISTENC-01]") {
    const std::vector<std::string> names = linesOf(invoke({"--list-encodings"}).output);

    CHECK(std::ranges::is_sorted(names));
    CHECK(std::ranges::adjacent_find(names) == names.end());
}

TEST_CASE("every name the list gives is one --encoding accepts", "[e2e][CLI-LISTENC-01]") {
    const std::vector<std::string> names = linesOf(invoke({"--list-encodings"}).output);
    REQUIRE_FALSE(names.empty());

    // The ends and the middle: all two hundred would cost a process each.
    for (const std::string& name : {names.front(), names[names.size() / 2], names.back()}) {
        const CliRun run = invoke({"--encoding", name, "inspect", "/nonexistent/file.srt"});

        // The file is absent, which is the failure we expect — and not a refusal
        // of the encoding, which would be a usage error before any file is read.
        INFO(name);
        CHECK(run.exitCode == 2);
    }
}

TEST_CASE("in json it is one object per encoding, and the narration level does not change it",
          "[e2e][CLI-LISTENC-01][CLI-JSON-01]") {
    const CliRun plain = invoke({"--format", "json", "--list-encodings"});
    const CliRun quiet = invoke({"-q", "--format", "json", "--list-encodings"});

    CHECK(plain.exitCode == 0);
    const std::vector<std::string> lines = linesOf(plain.output);
    REQUIRE_FALSE(lines.empty());
    CHECK(lines.size() == linesOf(invoke({"--list-encodings"}).output).size());
    for (const std::string& line : lines) {
        CHECK_THAT(line,
                   ContainsSubstring("{\"schema\":1,\"command\":\"list-encodings\",\"name\":\""));
    }
    CHECK(std::ranges::find(lines,
                            "{\"schema\":1,\"command\":\"list-encodings\",\"name\":\"UTF-8\"}") !=
          lines.end());
    // A result, not narration: silence does not take it away.
    CHECK(quiet.output == plain.output);
}

TEST_CASE("it stops after the list, whatever else was asked", "[e2e][CLI-LISTENC-01]") {
    const CliRun run = invoke({"--list-encodings", "inspect", "/nonexistent/file.srt"});

    CHECK(run.exitCode == 0);
    CHECK(run.errors.empty());
    CHECK(contains(linesOf(run.output), "UTF-8"));
}
