// `join-repeats` and `repair`, through the real binary: the subtitles that
// repeat each other joined, then — for `repair` — the order, then the ranges.
//
// The expected files are worked out by hand and never read back from the program.

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

TEST_CASE("the pieces of one text come out as one subtitle", "[e2e][CLI-JOIN-01]") {
    const Scratch scratch;
    const std::string out = scratch.of("out/segments.vtt");

    const CliRun run =
        invoke({"join-repeats", "--output", out, corpus("malformes/segments-repetes.vtt")});

    CHECK(run.exitCode == 0);
    CHECK_THAT(contentOf(out), MatchesFile(corpus("attendus/join/segments-repetes.vtt")));
    CHECK_THAT(run.errors, ContainsSubstring(": 2 repeated subtitles joined -> "));
}

TEST_CASE("a second run joins nothing", "[e2e][CLI-JOIN-01]") {
    const Scratch scratch;
    const std::string once = scratch.of("once/a.srt");
    const std::string input =
        writeFile(scratch,
                  "in/a.srt",
                  "1\n00:00:00,000 --> 00:00:01,000\nA\n\n2\n00:00:01,000 --> 00:00:02,000\nA\n\n");

    CHECK(invoke({"join-repeats", "--output", once, input}).exitCode == 0);
    const CliRun again = invoke({"join-repeats", "--output", scratch.of("twice/a.srt"), once});

    CHECK(again.exitCode == 0);
    CHECK_THAT(again.errors, ContainsSubstring("no repeated subtitles"));
}

TEST_CASE("repair does the three in one go", "[e2e][CLI-JOIN-02]") {
    const Scratch scratch;
    const std::string out = scratch.of("out/segments.vtt");

    const CliRun run =
        invoke({"repair", "--output", out, corpus("malformes/segments-repetes.vtt")});

    CHECK(run.exitCode == 0);
    CHECK_THAT(contentOf(out), MatchesFile(corpus("attendus/repair/segments-repetes.vtt")));
    CHECK_THAT(run.errors,
               ContainsSubstring("2 repeated subtitles joined; already in order; 0 subtitles "
                                 "split, 1 merged; 1 merged with styles or positions that "
                                 "differ, the first kept"));
}

TEST_CASE("repair takes the settings of the cut", "[e2e][CLI-JOIN-02]") {
    const Scratch scratch;
    const std::string input = writeFile(scratch,
                                        "in/a.srt",
                                        "1\n00:00:00,000 --> 00:00:05,000\nA\n\n"
                                        "2\n00:00:03,000 --> 00:00:08,000\nB\n\n");
    const std::string out = scratch.of("out/a.srt");

    const CliRun run = invoke({"repair", "--dashes", "--output", out, input});
    CHECK(run.exitCode == 0);
    CHECK_THAT(contentOf(out), ContainsSubstring("- A\n- B\n"));

    const CliRun refused = invoke({"repair", "--max-lines", "two", "--dry-run", input});
    CHECK(refused.exitCode == 1);
    CHECK_THAT(refused.errors, ContainsSubstring("--max-lines"));
}

TEST_CASE("convert takes the same corrections as options", "[e2e][CLI-JOIN-03]") {
    const Scratch scratch;
    const std::string joined = scratch.of("out/joined.vtt");
    const std::string repaired = scratch.of("out/repaired.vtt");

    const CliRun join = invoke({"convert",
                                "--to",
                                "vtt",
                                "--join-repeats",
                                "--output",
                                joined,
                                corpus("malformes/segments-repetes.vtt")});
    CHECK(join.exitCode == 0);
    CHECK_THAT(contentOf(joined), MatchesFile(corpus("attendus/join/segments-repetes.vtt")));

    const CliRun repair = invoke({"convert",
                                  "--to",
                                  "vtt",
                                  "--repair",
                                  "--output",
                                  repaired,
                                  corpus("malformes/segments-repetes.vtt")});
    CHECK(repair.exitCode == 0);
    CHECK_THAT(contentOf(repaired), MatchesFile(corpus("attendus/repair/segments-repetes.vtt")));
    CHECK_THAT(repair.errors, ContainsSubstring("2 repeated subtitles joined"));
}
