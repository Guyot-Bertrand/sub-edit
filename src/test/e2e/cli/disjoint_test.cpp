// `disjoint`, through the real binary: overlapping subtitles cut so that no
// range overlaps another.
//
// The expected file is worked out by hand from the rule — cut at every start and
// end, merge what shows together, line by line — and never read back from the
// program.

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include <string>

#include "cli_run.hpp"

using Catch::Matchers::ContainsSubstring;
using subedit::e2e::CliRun;
using subedit::e2e::contentOf;
using subedit::e2e::corpus;
using subedit::e2e::invoke;
using subedit::e2e::MatchesFile;
using subedit::e2e::Scratch;
using subedit::e2e::writeFile;

namespace {

/// The same two subtitles, the later one first.
const std::string kUnordered = "1\n00:00:03,000 --> 00:00:08,000\nB\n\n"
                               "2\n00:00:00,000 --> 00:00:05,000\nA\n\n";

const std::string kOverlap = "1\n00:00:00,000 --> 00:00:05,000\nA\n\n"
                             "2\n00:00:03,000 --> 00:00:08,000\nB\n\n";

} // namespace

TEST_CASE("overlapping subtitles come out cut, merged and disjoint", "[e2e][CLI-DISJOINT-01]") {
    const Scratch scratch;
    const std::string out = scratch.of("out/superpositions.srt");

    const CliRun run =
        invoke({"disjoint", "--output", out, corpus("malformes/superpositions.srt")});

    CHECK(run.exitCode == 0);
    CHECK_THAT(contentOf(out), MatchesFile(corpus("attendus/disjoint/superpositions.srt")));
    CHECK_THAT(run.errors,
               ContainsSubstring("4 subtitles split, 5 merged; 1 shown in windows for holding too "
                                 "many lines, 1 very short taken over by a neighbour"));
}

TEST_CASE("a second run changes nothing", "[e2e][CLI-DISJOINT-01]") {
    const Scratch scratch;
    const std::string once = scratch.of("once/a.srt");
    const std::string twice = scratch.of("twice/a.srt");
    const std::string input = writeFile(scratch, "in/a.srt", kOverlap);

    CHECK(invoke({"disjoint", "--output", once, input}).exitCode == 0);
    const CliRun again = invoke({"disjoint", "--output", twice, once});

    CHECK(again.exitCode == 0);
    CHECK(contentOf(twice) == contentOf(once));
    CHECK_THAT(again.errors, ContainsSubstring("already disjoint"));
}

TEST_CASE("a dry run says what would be done and writes nothing", "[e2e][CLI-DISJOINT-01]") {
    const Scratch scratch;
    const std::string input = writeFile(scratch, "in/a.srt", kOverlap);

    const CliRun run = invoke({"disjoint", "--dry-run", input});

    CHECK(run.exitCode == 0);
    CHECK_THAT(run.errors, ContainsSubstring("2 subtitles split, 1 merged"));
    CHECK_THAT(run.errors, ContainsSubstring("dry run, nothing written"));
}

TEST_CASE("a file out of order is refused, and --sort puts it right first",
          "[e2e][CLI-DISJOINT-02]") {
    const Scratch scratch;
    const std::string input = writeFile(scratch, "in/a.srt", kUnordered);
    const std::string refused = scratch.of("refused/a.srt");
    const std::string sorted = scratch.of("sorted/a.srt");

    const CliRun no = invoke({"disjoint", "--output", refused, input});
    CHECK(no.exitCode != 0);
    CHECK_THAT(no.errors, ContainsSubstring("sort them first"));
    CHECK(contentOf(refused).empty());

    const CliRun yes = invoke({"disjoint", "--sort", "--output", sorted, input});
    CHECK(yes.exitCode == 0);
    CHECK_THAT(contentOf(sorted), ContainsSubstring("00:00:03,000 --> 00:00:05,000\nA\nB\n"));
}

TEST_CASE("the settings change what is cut", "[e2e][CLI-DISJOINT-03]") {
    const Scratch scratch;
    const std::string input = writeFile(scratch, "in/a.srt", kOverlap);
    const std::string out = scratch.of("out/a.srt");

    const CliRun run = invoke(
        {"disjoint", "--dashes", "--max-lines", "0", "--minimum", "0", "--output", out, input});

    CHECK(run.exitCode == 0);
    CHECK_THAT(contentOf(out), ContainsSubstring("- A\n- B\n"));
}

TEST_CASE("a setting that makes no sense is a usage error", "[e2e][CLI-DISJOINT-03]") {
    const Scratch scratch;
    const std::string input = writeFile(scratch, "in/a.srt", kOverlap);

    const CliRun minimum = invoke({"disjoint", "--minimum", "soon", "--dry-run", input});
    CHECK(minimum.exitCode == 1);
    CHECK_THAT(minimum.errors, ContainsSubstring("--minimum"));

    const CliRun lines = invoke({"disjoint", "--max-lines", "two", "--dry-run", input});
    CHECK(lines.exitCode == 1);
    CHECK_THAT(lines.errors, ContainsSubstring("--max-lines"));
}

TEST_CASE("the disjoint option is one of convert, append and split-file",
          "[e2e][CLI-DISJOINT-04]") {
    const Scratch scratch;
    const std::string input = writeFile(scratch, "in/a.srt", kOverlap);
    const std::string next =
        writeFile(scratch, "in/b.srt", "1\n00:00:00,000 --> 00:00:01,000\nC\n\n");

    const std::string converted = scratch.of("out/a.vtt");
    const CliRun convert =
        invoke({"convert", "--to", "vtt", "--disjoint", "--output", converted, input});
    CHECK(convert.exitCode == 0);
    CHECK_THAT(contentOf(converted), ContainsSubstring("00:03.000 --> 00:05.000\nA\nB\n"));
    CHECK_THAT(convert.errors, ContainsSubstring("2 subtitles split, 1 merged"));

    const std::string appended = scratch.of("out/all.srt");
    const CliRun append = invoke({"append", "--disjoint", "--output", appended, input, next});
    CHECK(append.exitCode == 0);
    CHECK_THAT(contentOf(appended), ContainsSubstring("A\nB\n"));

    const CliRun split = invoke({"split-file",
                                 "--at",
                                 "4",
                                 "--disjoint",
                                 "--head",
                                 scratch.of("out/head.srt"),
                                 "--tail",
                                 scratch.of("out/tail.srt"),
                                 appended});
    CHECK(split.exitCode == 0);
    CHECK_THAT(contentOf(scratch.of("out/head.srt")), ContainsSubstring("A\nB\n"));
}

TEST_CASE("the disjoint option refuses a file out of order, on each command",
          "[e2e][CLI-DISJOINT-04]") {
    const Scratch scratch;
    const std::string input = writeFile(scratch, "in/a.srt", kUnordered);

    const CliRun convert =
        invoke({"convert", "--to", "vtt", "--disjoint", "--output", scratch.of("o.vtt"), input});
    CHECK(convert.exitCode != 0);
    CHECK_THAT(convert.errors, ContainsSubstring("sort them first"));

    const CliRun sorted = invoke(
        {"convert", "--to", "vtt", "--sort", "--disjoint", "--output", scratch.of("p.vtt"), input});
    CHECK(sorted.exitCode == 0);
}
