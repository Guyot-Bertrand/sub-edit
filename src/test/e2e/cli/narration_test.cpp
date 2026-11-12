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
using subedit::e2e::corpus;
using subedit::e2e::invoke;
using subedit::e2e::Scratch;

namespace {

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

/// What that one diagnostic is owed, in full: under the key `diagnostic`, the file named by the
/// element's `source` line.
///
/// Line 6 and not 5, where the « 7 » sits: a diagnostic about a block is
/// anchored on the block, that is on its timing line. Only the diagnostics
/// about a single line — an unreadable timestamp, text before the first one —
/// point at themselves.
const std::string kNumberingTold =
    "] diagnostic: line 6: SubRip numbers that do not follow (\"7\"), settled by the reader\n";

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
    CHECK_THAT(run.errors, ContainsSubstring("[1/1] source: " + kGood + "\n"));
    CHECK_THAT(run.errors, ContainsSubstring("[1/1] result: 2 subtitles\n"));
}

TEST_CASE("silence keeps the errors", "[e2e][CLI-OUTPUT-02]") {
    const CliRun quiet = invoke({"--quiet", "inspect", kUnreadable});

    // The error, and the `source` line it never comes without: what raises the alarm survives,
    // what merely recounts does not.
    CHECK_THAT(quiet.errors, ContainsSubstring("[1/1] source: " + kUnreadable + "\n"));
    CHECK_THAT(quiet.errors, ContainsSubstring("[1/1] error: "));
    CHECK(lines(quiet.errors).size() == 2);
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
    const Scratch scratch;
    const std::string out = scratch.path();

    const std::string one = invoke({"shift", "--by", "1", "--output-dir", out, kGood}).errors;
    const std::string two =
        invoke({"-vv", "shift", "--by", "1", "--output-dir", out, kGood}).errors;
    const std::string three =
        invoke({"-vvv", "shift", "--by", "1", "--output-dir", out, kGood}).errors;

    CHECK(contains(two, one));
    CHECK(contains(three, two));
    CHECK(lines(two).size() > lines(one).size());
    CHECK(lines(three).size() > lines(two).size());
}

TEST_CASE("a subcommand that writes says nothing on standard output", "[e2e][CLI-OUTPUT-01]") {
    // Its result is the file. At the loudest level, where a stray line is most
    // likely, standard output must still be empty.
    const Scratch scratch;

    const CliRun run =
        invoke({"-vvv", "shift", "--by", "1", "--output-dir", scratch.path(), kGood});

    CHECK(run.exitCode == 0);
    CHECK(run.output.empty());
    CHECK_FALSE(run.errors.empty());
}

TEST_CASE("a single -v is the default level", "[e2e][CLI-OUTPUT-04]") {
    CHECK(invoke({"-v", "inspect", kGood}).errors == invoke({"inspect", kGood}).errors);
}

TEST_CASE("the second level says what was recognised", "[e2e][CLI-OUTPUT-03]") {
    const CliRun run = invoke({"-vv", "inspect", kGood});

    CHECK_THAT(run.errors,
               ContainsSubstring("[subedit] [inspect] [1/1] format: SubRip\n"
                                 "[subedit] [inspect] [1/1] encoding: UTF-8\n"
                                 "[subedit] [inspect] [1/1] bom: absent\n"
                                 "[subedit] [inspect] [1/1] line-endings: LF\n"));
}

TEST_CASE("the third level names each diagnostic, not only how many", "[e2e][CLI-OUTPUT-06]") {
    // « 1 diagnostic while reading » left the reader with a number and no way
    // to learn what it was. Each one is named, a line each; the count is the
    // number of those lines.
    const CliRun run = invoke({"-vvv", "inspect", kNumbering});

    CHECK_THAT(run.errors, ContainsSubstring("[1/1] source: " + kNumbering + "\n"));
    CHECK_THAT(run.errors, ContainsSubstring(kNumberingTold));
}

TEST_CASE("the diagnostics stay at the level that details", "[e2e][CLI-OUTPUT-06]") {
    // They are informative, never a failure: the file was read and the command
    // succeeded. Below the third level they would bury what actually happened.
    CHECK_THAT(invoke({"-vv", "inspect", kNumbering}).errors, !ContainsSubstring("do not follow"));
    CHECK(invoke({"inspect", kNumbering}).exitCode == 0);
}

TEST_CASE("a subcommand that writes reports what the reader had to decide",
          "[e2e][CLI-OUTPUT-06]") {
    const Scratch scratch;
    // ADR 0008 has the core read at best effort and say what it settled. A
    // promise kept by one subcommand out of seven is not kept: shift rewrites
    // the same file and owes the same account.
    //
    // Seven and no longer five: `hearing-impaired` came with phase 4 and `snap`
    // with phase 16, and both report as this case asks. The requirement's
    // wording followed at the phase 7 review, where the count was measured
    // rather than carried over.
    const CliRun run =
        invoke({"-vvv", "shift", "--by", "1", "--output-dir", scratch.path(), kNumbering});

    CHECK(run.exitCode == 0);
    CHECK_THAT(run.errors, ContainsSubstring("[1/1] source: " + kNumbering + "\n"));
    CHECK_THAT(run.errors, ContainsSubstring(kNumberingTold));
}

TEST_CASE("converting reports them too", "[e2e][CLI-OUTPUT-06]") {
    const Scratch scratch;
    const CliRun run =
        invoke({"-vvv", "convert", "--to", "vtt", "--output-dir", scratch.path(), kNumbering});

    CHECK(run.exitCode == 0);
    CHECK_THAT(run.errors, ContainsSubstring(kNumberingTold));
}

TEST_CASE("the second level says the byte order mark, whichever subcommand",
          "[e2e][CLI-OUTPUT-03]") {
    const Scratch scratch;
    // One shape for the three, in one order: format, encoding, mark, endings.
    // The mark is the property most easily lost and the least visible; saying
    // it for two subcommands out of three was the worst of both.
    // One fact a line, the words as names.
    for (const std::vector<std::string>& asked :
         {std::vector<std::string>{"inspect", kGood},
          std::vector<std::string>{"shift", "--by", "1", "--output-dir", scratch.path(), kGood},
          std::vector<std::string>{
              "convert", "--to", "vtt", "--output-dir", scratch.path(), kGood}}) {
        std::vector<std::string> loud{"-vv"};
        loud.insert(loud.end(), asked.begin(), asked.end());
        const std::string prefix = "[subedit] [" + asked.front() + "] [1/1] ";
        std::string expected;
        for (const char* fact :
             {"format: SubRip\n", "encoding: UTF-8\n", "bom: absent\n", "line-endings: LF\n"}) {
            expected += prefix;
            expected += fact;
        }
        CHECK_THAT(invoke(loud).errors, ContainsSubstring(expected));
    }
}

TEST_CASE("a single file gets no summary", "[e2e][CLI-OUTPUT-05]") {
    // "1 of 1 files inspected" would repeat the line just above it.
    CHECK_THAT(invoke({"inspect", kGood}).errors, !ContainsSubstring("of 1 file"));
}

TEST_CASE("two files get a summary", "[e2e][CLI-OUTPUT-05]") {
    CHECK_THAT(invoke({"inspect", kGood, kOther}).errors,
               ContainsSubstring("2 of 2 files inspected\n"));
}
