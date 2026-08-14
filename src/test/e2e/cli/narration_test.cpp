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

/// Readable, but not without the reader having to decide something: its second
/// block is numbered 7 where 2 was due.
const std::string kNumbering = corpus("malformes/numerotation-incoherente.srt");

/// What that one diagnostic is owed, in full.
///
/// Line 6 and not 5, where the « 7 » sits: a diagnostic about a block is
/// anchored on the block, that is on its timing line. Only the diagnostics
/// about a single line — an unreadable timestamp, text before the first one —
/// point at themselves.
const std::string kNumberingSaid =
    ": line 6: SubRip numbers that do not follow (\"7\"), settled by the reader\n";

std::string scratch(const std::string& name) {
    const std::filesystem::path directory = std::filesystem::temp_directory_path() / name;
    std::filesystem::remove_all(directory);
    std::filesystem::create_directories(directory);
    return directory.string();
}

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

TEST_CASE("the levels nest on a subcommand that writes, too", "[e2e][CLI-OUTPUT-03]") {
    // Proved above on `inspect`, which writes no file, and here on one that
    // does. The two narrations are built by different code — inspection.cpp and
    // rewriting.cpp — and only the first was ever compared level to level.
    const std::filesystem::path directory =
        std::filesystem::temp_directory_path() / "subedit-narration-e2e";
    std::filesystem::create_directories(directory);
    const std::string out = directory.string();

    const std::string one = invoke({"shift", "--by", "1", "--output-dir", out, kGood}).errors;
    const std::string two =
        invoke({"-vv", "shift", "--by", "1", "--output-dir", out, kGood}).errors;
    const std::string three =
        invoke({"-vvv", "shift", "--by", "1", "--output-dir", out, kGood}).errors;

    CHECK(contains(two, one));
    CHECK(contains(three, two));
    CHECK(lines(two).size() > lines(one).size());
    CHECK(lines(three).size() > lines(two).size());

    std::filesystem::remove_all(directory);
}

TEST_CASE("a subcommand that writes says nothing on standard output", "[e2e][CLI-OUTPUT-01]") {
    // Its result is the file. At the loudest level, where a stray line is most
    // likely, standard output must still be empty.
    const std::filesystem::path directory =
        std::filesystem::temp_directory_path() / "subedit-narration-e2e-quiet";
    std::filesystem::create_directories(directory);

    const CliRun run =
        invoke({"-vvv", "shift", "--by", "1", "--output-dir", directory.string(), kGood});

    CHECK(run.exitCode == 0);
    CHECK(run.output.empty());
    CHECK_FALSE(run.errors.empty());

    std::filesystem::remove_all(directory);
}

TEST_CASE("a single -v is the default level", "[e2e][CLI-OUTPUT-04]") {
    CHECK(invoke({"-v", "inspect", kGood}).errors == invoke({"inspect", kGood}).errors);
}

TEST_CASE("the second level says what was recognised", "[e2e][CLI-OUTPUT-03]") {
    const CliRun run = invoke({"-vv", "inspect", kGood});

    CHECK_THAT(run.errors, ContainsSubstring(kGood + ": SubRip, UTF-8, no BOM, LF line endings\n"));
}

TEST_CASE("the third level names each diagnostic, not only how many", "[e2e][CLI-OUTPUT-06]") {
    // « 1 diagnostic while reading » leaves the reader with a number and no way
    // to learn what it was. The count stays — it is the summary — and each one
    // is now named under it.
    const CliRun run = invoke({"-vvv", "inspect", kNumbering});

    CHECK_THAT(run.errors, ContainsSubstring(kNumbering + ": 1 diagnostic while reading\n"));
    CHECK_THAT(run.errors, ContainsSubstring(kNumbering + kNumberingSaid));
}

TEST_CASE("the diagnostics stay at the level that details", "[e2e][CLI-OUTPUT-06]") {
    // They are informative, never a failure: the file was read and the command
    // succeeded. Below the third level they would bury what actually happened.
    CHECK_THAT(invoke({"-vv", "inspect", kNumbering}).errors, !ContainsSubstring("do not follow"));
    CHECK(invoke({"inspect", kNumbering}).exitCode == 0);
}

TEST_CASE("a subcommand that writes reports what the reader had to decide",
          "[e2e][CLI-OUTPUT-06]") {
    // ADR 0008 has the core read at best effort and say what it settled. A
    // promise kept by one subcommand out of five is not kept: shift rewrites
    // the same file and owes the same account.
    const CliRun run = invoke(
        {"-vvv", "shift", "--by", "1", "--output-dir", scratch("subedit-diag-e2e"), kNumbering});

    CHECK(run.exitCode == 0);
    CHECK_THAT(run.errors, ContainsSubstring(kNumbering + kNumberingSaid));
}

TEST_CASE("converting reports them too", "[e2e][CLI-OUTPUT-06]") {
    const CliRun run = invoke({"-vvv",
                               "convert",
                               "--to",
                               "vtt",
                               "--output-dir",
                               scratch("subedit-diag-convert-e2e"),
                               kNumbering});

    CHECK(run.exitCode == 0);
    CHECK_THAT(run.errors, ContainsSubstring(kNumbering + kNumberingSaid));
}

TEST_CASE("the second level says the byte order mark, whichever subcommand",
          "[e2e][CLI-OUTPUT-03]") {
    // One shape for the three, in one order: format, encoding, mark, endings.
    // The mark is the property most easily lost and the least visible; saying
    // it for two subcommands out of three was the worst of both.
    CHECK_THAT(invoke({"-vv", "inspect", kGood}).errors,
               ContainsSubstring(kGood + ": SubRip, UTF-8, no BOM, LF line endings\n"));

    CHECK_THAT(
        invoke({"-vv", "shift", "--by", "1", "--output-dir", scratch("subedit-bom-e2e"), kGood})
            .errors,
        ContainsSubstring(kGood + ": SubRip, UTF-8, no BOM, LF line endings kept\n"));

    CHECK_THAT(invoke({"-vv",
                       "convert",
                       "--to",
                       "vtt",
                       "--output-dir",
                       scratch("subedit-bom-convert-e2e"),
                       kGood})
                   .errors,
               ContainsSubstring(kGood + ": SubRip -> WebVTT, UTF-8, no BOM, LF line endings\n"));
}

TEST_CASE("a single file gets no summary", "[e2e][CLI-OUTPUT-05]") {
    // "1 of 1 files inspected" would repeat the line just above it.
    CHECK_THAT(invoke({"inspect", kGood}).errors, !ContainsSubstring("of 1 file"));
}

TEST_CASE("two files get a summary", "[e2e][CLI-OUTPUT-05]") {
    CHECK_THAT(invoke({"inspect", kGood, kOther}).errors,
               ContainsSubstring("2 of 2 files inspected\n"));
}
