// What the tool says while it works, and on which stream it says it.

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include <algorithm>
#include <filesystem>
#include <sstream>
#include <string>
#include <vector>

#include "cli_run.hpp"

using Catch::Matchers::ContainsSubstring;
using subedit::e2e::CliRun;
using subedit::e2e::invoke;

namespace {

std::string corpus(const std::string& relative) {
    return (std::filesystem::path{SUBEDIT_TEST_DATA_DIR} / relative).string();
}

std::vector<std::string> lines(const std::string& text) {
    std::vector<std::string> result;
    std::istringstream stream{text};
    std::string line;
    while (std::getline(stream, line)) {
        result.push_back(line);
    }
    return result;
}

/// Every line of `narrower` appears, identical, in `wider`.
///
/// Compared rather than written out three times: three expectations written by
/// hand would drift apart without anything saying so.
bool contains(const std::string& wider, const std::string& narrower) {
    const std::vector<std::string> haystack = lines(wider);
    return std::ranges::all_of(lines(narrower), [&haystack](const std::string& line) {
        return std::ranges::find(haystack, line) != haystack.end();
    });
}

const std::string kGood = corpus("valides/minimal.srt");
const std::string kOther = corpus("valides/minimal.vtt");
const std::string kUnreadable = corpus("malformes/vide.srt");

} // namespace

TEST_CASE("the report goes to standard output and the narration to standard error",
          "[e2e][CLI-OUTPUT-01]") {
    const CliRun run = invoke({"inspect", kGood});

    CHECK_THAT(run.output, ContainsSubstring("  format: SubRip\n"));
    // Not one line of the report leaks onto the other stream, and not one line
    // of narration onto the one a caller pipes.
    CHECK_THAT(run.errors, !ContainsSubstring("format:"));
    CHECK_THAT(run.output, !ContainsSubstring("subtitles\n"));
}

TEST_CASE("the default level accounts for each file", "[e2e][CLI-OUTPUT-04]") {
    const CliRun run = invoke({"inspect", kGood});

    CHECK(run.exitCode == 0);
    CHECK_THAT(run.errors, ContainsSubstring(kGood + ": 2 subtitles\n"));
}

TEST_CASE("silence keeps the errors", "[e2e][CLI-OUTPUT-02]") {
    const CliRun quiet = invoke({"--quiet", "inspect", kUnreadable});

    CHECK_THAT(quiet.errors, ContainsSubstring(kUnreadable));
    // Nothing else: what raises the alarm survives, what merely recounts does
    // not.
    CHECK(lines(quiet.errors).size() == 1);
}

TEST_CASE("silence drops the narration", "[e2e][CLI-OUTPUT-02]") {
    CHECK(invoke({"--quiet", "inspect", kGood}).errors.empty());
}

TEST_CASE("each level keeps every line of the one below", "[e2e][CLI-OUTPUT-03]") {
    const std::string one = invoke({"inspect", kGood}).errors;
    const std::string two = invoke({"-vv", "inspect", kGood}).errors;
    const std::string three = invoke({"-vvv", "inspect", kGood}).errors;

    CHECK(contains(two, one));
    CHECK(contains(three, two));
    // And each really does add something, or the nesting would hold trivially.
    CHECK(lines(two).size() > lines(one).size());
    CHECK(lines(three).size() > lines(two).size());
}

TEST_CASE("a single -v is the default level", "[e2e][CLI-OUTPUT-04]") {
    CHECK(invoke({"-v", "inspect", kGood}).errors == invoke({"inspect", kGood}).errors);
}

TEST_CASE("the second level says what was recognised", "[e2e][CLI-OUTPUT-03]") {
    const CliRun run = invoke({"-vv", "inspect", kGood});

    CHECK_THAT(run.errors, ContainsSubstring(kGood + ": SubRip, UTF-8, no BOM, LF line endings\n"));
}

TEST_CASE("a single file gets no summary", "[e2e][CLI-OUTPUT-05]") {
    // "1 of 1 files inspected" would repeat the line just above it.
    CHECK_THAT(invoke({"inspect", kGood}).errors, !ContainsSubstring("of 1 file"));
}

TEST_CASE("two files get a summary", "[e2e][CLI-OUTPUT-05]") {
    CHECK_THAT(invoke({"inspect", kGood, kOther}).errors,
               ContainsSubstring("2 of 2 files inspected\n"));
}
