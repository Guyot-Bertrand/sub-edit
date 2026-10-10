// `sort`, through the real binary: the subtitles in the order of their start,
// ties left as the file gave them.
//
// The expected files are worked out by hand — sort the starts, keep the ties in
// their order — and never read back from the program.

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include <cstddef>
#include <filesystem>
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

/// Out of order: the third subtitle in the file starts second.
const std::string kDisorder = "1\n00:00:05,000 --> 00:00:06,000\ncinq\n\n"
                              "2\n00:00:01,000 --> 00:00:02,000\nun\n\n"
                              "3\n00:00:03,000 --> 00:00:04,000\ntrois\n\n";

/// Two subtitles start together, at two seconds, and a third starts before them.
const std::string kTies = "1\n00:00:02,000 --> 00:00:03,000\nb-first\n\n"
                          "2\n00:00:01,000 --> 00:00:02,000\na\n\n"
                          "3\n00:00:02,000 --> 00:00:04,000\nb-second\n\n";

/// Already in order.
const std::string kOrdered = "1\n00:00:01,000 --> 00:00:02,000\nun\n\n"
                             "2\n00:00:03,000 --> 00:00:04,000\ntrois\n\n"
                             "3\n00:00:05,000 --> 00:00:06,000\ncinq\n\n";

std::string anonymised(std::string text, const std::string& what, const std::string& placeholder) {
    for (std::size_t at = text.find(what); at != std::string::npos;
         at = text.find(what, at + placeholder.size())) {
        text.replace(at, what.size(), placeholder);
    }
    return text;
}

} // namespace

TEST_CASE("a file out of order comes out in the order of its starts", "[e2e][CLI-SORT-01]") {
    const Scratch scratch;
    const std::string input = writeFile(scratch, "in/a.srt", kDisorder);
    const std::string out = scratch.of("out/a.srt");

    const CliRun run = invoke({"sort", "--output", out, input});

    CHECK(run.exitCode == 0);
    // Renumbered from one, texts moved with their positions.
    CHECK_THAT(contentOf(out), MatchesFile(corpus("attendus/sort/desordre.srt")));
    CHECK_THAT(run.errors, ContainsSubstring(": 3 subtitles moved -> "));
}

TEST_CASE("two subtitles that start together keep the order the file gave them",
          "[e2e][CLI-SORT-01]") {
    const Scratch scratch;
    const std::string input = writeFile(scratch, "in/b.srt", kTies);
    const std::string out = scratch.of("out/b.srt");

    const CliRun run = invoke({"sort", "--output", out, input});

    CHECK(run.exitCode == 0);
    // « b-first » stays before « b-second »: neither precedes the other, so
    // moving them would be a decision nobody asked for.
    CHECK_THAT(contentOf(out), MatchesFile(corpus("attendus/sort/egalite.srt")));
    CHECK_THAT(run.errors, ContainsSubstring(": 2 subtitles moved -> "));
}

TEST_CASE("a file already in order is written all the same, and says so", "[e2e][CLI-SORT-02]") {
    const Scratch scratch;
    const std::string input = writeFile(scratch, "in/c.srt", kOrdered);
    const std::string out = scratch.of("out/c.srt");

    const CliRun run = invoke({"sort", "--output", out, input});

    CHECK(run.exitCode == 0);
    CHECK_THAT(run.errors, ContainsSubstring(": already in order -> "));
    CHECK(contentOf(out) == kOrdered);
}

TEST_CASE("a batch says its destination and how many moved, file by file",
          "[e2e][CLI-SORT-02][CLI-JSON-09]") {
    const Scratch scratch;
    const std::string first = writeFile(scratch, "in/a.srt", kDisorder);
    const std::string second = writeFile(scratch, "in/b.srt", kTies);
    const std::string third = writeFile(scratch, "in/c.srt", kOrdered);

    const CliRun run = invoke(
        {"--format", "json", "sort", "--output-dir", scratch.of("out"), first, second, third});

    CHECK(run.exitCode == 0);
    CHECK_THAT(anonymised(run.output, scratch.path(), "<scratch>"),
               MatchesFile(corpus("attendus/json/sort.jsonl")));
}

TEST_CASE("what inspect reports as out of order, sort repairs", "[e2e][CLI-SORT-01]") {
    const Scratch scratch;
    const std::string input = writeFile(scratch, "in/a.srt", kDisorder);
    const std::string out = scratch.of("out/a.srt");

    const CliRun before = invoke({"inspect", input});
    REQUIRE(invoke({"--quiet", "sort", "--output", out, input}).exitCode == 0);
    const CliRun after = invoke({"inspect", out});

    CHECK_THAT(before.output, ContainsSubstring("starts before the previous one starts"));
    CHECK_THAT(after.output, ContainsSubstring("anomalies: none"));
}

TEST_CASE("a dry run counts and writes nothing", "[e2e][CLI-SORT-02][CLI-DRYRUN-01]") {
    const Scratch scratch;
    const std::string input = writeFile(scratch, "in/a.srt", kDisorder);

    const CliRun run = invoke({"sort", "--dry-run", input});

    CHECK(run.exitCode == 0);
    CHECK_THAT(run.errors, ContainsSubstring(": 3 subtitles moved (dry run, nothing written)"));
    CHECK(contentOf(input) == kDisorder);
}

TEST_CASE("sort needs a destination like every subcommand that writes", "[e2e][CLI-SORT-02]") {
    const Scratch scratch;
    const std::string input = writeFile(scratch, "in/a.srt", kDisorder);

    const CliRun run = invoke({"sort", input});

    CHECK(run.exitCode == 1);
    CHECK(contentOf(input) == kDisorder);
}

TEST_CASE("convert, append and split-file sort on request", "[e2e][CLI-SORT-03]") {
    const Scratch scratch;
    const std::string input = writeFile(scratch, "in/a.srt", kDisorder);

    SECTION("convert writes in order, and says how many moved") {
        const std::string out = scratch.of("out/a.vtt");

        const CliRun run = invoke({"convert", "--to", "vtt", "--sort", "--output", out, input});

        CHECK(run.exitCode == 0);
        CHECK_THAT(run.errors, ContainsSubstring("3 subtitles moved"));
        const std::string written = contentOf(out);
        CHECK(written.find("un") < written.find("trois"));
        CHECK(written.find("trois") < written.find("cinq"));
    }

    SECTION("convert without the option keeps the order of the file") {
        const std::string out = scratch.of("out/a.vtt");

        REQUIRE(invoke({"convert", "--to", "vtt", "--output", out, input}).exitCode == 0);

        const std::string written = contentOf(out);
        CHECK(written.find("cinq") < written.find("un"));
    }

    SECTION("append writes the result in order") {
        const std::string other = writeFile(scratch, "in/b.srt", kOrdered);
        const std::string out = scratch.of("out/all.srt");

        const CliRun run = invoke({"append", "--sort", "--output", out, input, other});

        CHECK(run.exitCode == 0);
        CHECK_THAT(run.errors, ContainsSubstring("subtitles moved"));
        const std::string written = contentOf(out);
        CHECK(written.find("un") < written.find("cinq"));
    }

    SECTION("split-file cuts in the time order") {
        const std::string head = scratch.of("out/head.srt");
        const std::string tail = scratch.of("out/tail.srt");

        const CliRun run =
            invoke({"split-file", "--sort", "--at", "3", "--head", head, "--tail", tail, input});

        CHECK(run.exitCode == 0);
        // In time order the head holds the two earliest subtitles, the tail the latest.
        CHECK_THAT(contentOf(head), ContainsSubstring("trois"));
        CHECK_THAT(contentOf(tail), ContainsSubstring("cinq"));
    }
}
