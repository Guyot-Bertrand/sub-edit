// `split-file`, through the real binary: a file cut in two, the tail brought back
// to the origin — the inverse of `append`.
//
// The expected files are worked out by hand — the tail moves back by the end of the
// last subtitle that stays — and never read back from the program.

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

/// Four subtitles ending at 2, 4, 7 and 9 seconds; cut at the third, the tail
/// moves back by four.
const std::string kFilm = "1\n00:00:01,000 --> 00:00:02,000\nun\n\n"
                          "2\n00:00:03,000 --> 00:00:04,000\ndeux\n\n"
                          "3\n00:00:06,000 --> 00:00:07,000\ntrois\n\n"
                          "4\n00:00:08,000 --> 00:00:09,000\nquatre\n\n";

/// The second subtitle ends after the third starts: cut at the third, the tail
/// would begin before the origin.
const std::string kOverlap = "1\n00:00:00,000 --> 00:00:10,000\na\n\n"
                             "2\n00:00:05,000 --> 00:00:06,000\nb\n\n"
                             "3\n00:00:05,500 --> 00:00:06,000\nc\n\n";

const std::string kFirst = "1\n00:00:01,000 --> 00:00:02,000\nun\n\n"
                           "2\n00:00:03,000 --> 00:00:04,000\ndeux\n\n";

const std::string kSecond = "1\n00:00:00,500 --> 00:00:01,500\n<i>trois</i>\n\n";

std::string anonymised(std::string text, const std::string& what, const std::string& placeholder) {
    for (std::size_t at = text.find(what); at != std::string::npos;
         at = text.find(what, at + placeholder.size())) {
        text.replace(at, what.size(), placeholder);
    }
    return text;
}

} // namespace

TEST_CASE("a file is cut at the subtitle given, the tail brought back to the origin",
          "[e2e][CLI-PSPLIT-01]") {
    const Scratch scratch;
    const std::string input = writeFile(scratch, "in/film.srt", kFilm);
    const std::string head = scratch.of("out/head.srt");
    const std::string tail = scratch.of("out/tail.srt");

    const CliRun run = invoke({"split-file", input, "--at", "3", "--head", head, "--tail", tail});

    CHECK(run.exitCode == 0);
    CHECK_THAT(contentOf(head), MatchesFile(corpus("attendus/split/coupe-tete.srt")));
    CHECK_THAT(contentOf(tail), MatchesFile(corpus("attendus/split/coupe-queue.srt")));
    CHECK_THAT(run.errors,
               ContainsSubstring("split at subtitle 3: 2 subtitles in the head, "
                                 "2 subtitles in the tail -> "));
}

TEST_CASE("append then split-file gives the files back", "[e2e][CLI-PSPLIT-01][CLI-APPEND-01]") {
    const Scratch scratch;
    const std::string first = writeFile(scratch, "in/a.srt", kFirst);
    const std::string second = writeFile(scratch, "in/b.srt", kSecond);
    const std::string whole = scratch.of("mid/all.srt");
    const std::string head = scratch.of("out/head.srt");
    const std::string tail = scratch.of("out/tail.srt");

    REQUIRE(invoke({"--quiet", "append", first, second, "--output", whole}).exitCode == 0);
    const CliRun run = invoke({"split-file", whole, "--at", "3", "--head", head, "--tail", tail});

    CHECK(run.exitCode == 0);
    // Byte for byte what went in: the shift forward and the shift back cancel.
    CHECK(contentOf(head) == kFirst);
    CHECK(contentOf(tail) == kSecond);
}

TEST_CASE("two halves that overlap are refused, naming the subtitle, and nothing is written",
          "[e2e][CLI-PSPLIT-01]") {
    const Scratch scratch;
    const std::string input = writeFile(scratch, "in/o.srt", kOverlap);
    const std::string head = scratch.of("out/head.srt");
    const std::string tail = scratch.of("out/tail.srt");

    const CliRun run = invoke({"split-file", input, "--at", "3", "--head", head, "--tail", tail});

    CHECK(run.exitCode == 2);
    CHECK_THAT(run.errors,
               ContainsSubstring("Cannot split at subtitle 3: subtitle 3 would fall before the "
                                 "start of the video. Cut somewhere else."));
    CHECK_FALSE(std::filesystem::exists(head));
    CHECK_FALSE(std::filesystem::exists(tail));
}

TEST_CASE("a cut outside the file is refused before anything is written", "[e2e][CLI-PSPLIT-01]") {
    const Scratch scratch;
    const std::string input = writeFile(scratch, "in/film.srt", kFilm);
    const std::string single = writeFile(scratch, "in/one.srt", kSecond);
    const std::string head = scratch.of("out/head.srt");
    const std::string tail = scratch.of("out/tail.srt");

    // The first subtitle cannot begin a tail, nor can one past the last.
    const CliRun first = invoke({"split-file", input, "--at", "1", "--head", head, "--tail", tail});
    const CliRun past = invoke({"split-file", input, "--at", "5", "--head", head, "--tail", tail});
    const CliRun lone = invoke({"split-file", single, "--at", "2", "--head", head, "--tail", tail});
    const CliRun negative =
        invoke({"split-file", input, "--at", "-1", "--head", head, "--tail", tail});

    CHECK(first.exitCode == 2);
    CHECK_THAT(first.errors,
               ContainsSubstring("--at 1: the file holds 4 subtitles, and the tail can start "
                                 "from 2 to 4"));
    CHECK(past.exitCode == 2);
    CHECK(lone.exitCode == 2);
    CHECK_THAT(lone.errors, ContainsSubstring("a file of 1 subtitle cannot be split"));
    CHECK(negative.exitCode == 1);
    CHECK_FALSE(std::filesystem::exists(head));
    CHECK_FALSE(std::filesystem::exists(tail));
}

TEST_CASE("the record is one object, the head as destination and the tail beside it",
          "[e2e][CLI-PSPLIT-01][CLI-JSON-09]") {
    const Scratch scratch;
    const std::string input = writeFile(scratch, "in/film.srt", kFilm);

    const CliRun run = invoke({"--format",
                               "json",
                               "split-file",
                               input,
                               "--at",
                               "3",
                               "--head",
                               scratch.of("out/head.srt"),
                               "--tail",
                               scratch.of("out/tail.srt")});

    CHECK(run.exitCode == 0);
    CHECK_THAT(anonymised(run.output, scratch.path(), "<scratch>"),
               MatchesFile(corpus("attendus/json/split-file.jsonl")));
}

TEST_CASE("a dry run says the cut and writes nothing", "[e2e][CLI-PSPLIT-01][CLI-DRYRUN-01]") {
    const Scratch scratch;
    const std::string input = writeFile(scratch, "in/film.srt", kFilm);
    const std::string head = scratch.of("out/head.srt");
    const std::string tail = scratch.of("out/tail.srt");

    const CliRun named =
        invoke({"split-file", "--dry-run", input, "--at", "3", "--head", head, "--tail", tail});
    const CliRun bare = invoke({"split-file", "--dry-run", input, "--at", "3"});
    const CliRun half = invoke({"split-file", "--dry-run", input, "--at", "3", "--head", head});

    CHECK(named.exitCode == 0);
    CHECK(bare.exitCode == 0);
    CHECK_THAT(bare.errors,
               ContainsSubstring("2 subtitles in the head, 2 subtitles in the tail "
                                 "(dry run, nothing written)"));
    CHECK(half.exitCode == 1);
    CHECK_FALSE(std::filesystem::exists(head));
    CHECK_FALSE(std::filesystem::exists(scratch.of("out")));
}

TEST_CASE("the two outputs are asked for, and are neither the input nor one another",
          "[e2e][CLI-PSPLIT-01]") {
    const Scratch scratch;
    const std::string input = writeFile(scratch, "in/film.srt", kFilm);
    const std::string head = scratch.of("out/head.srt");
    const std::string tail = scratch.of("out/tail.srt");

    const CliRun none = invoke({"split-file", input, "--at", "3"});
    const CliRun onlyHead = invoke({"split-file", input, "--at", "3", "--head", head});
    const CliRun same = invoke({"split-file", input, "--at", "3", "--head", head, "--tail", head});
    const CliRun overInput =
        invoke({"split-file", input, "--at", "3", "--head", input, "--tail", tail});
    const CliRun noCut = invoke({"split-file", input, "--head", head, "--tail", tail});
    const CliRun output = invoke({"split-file", input, "--at", "3", "--output", head});
    const CliRun inPlace = invoke({"split-file", input, "--at", "3", "--in-place"});

    CHECK(none.exitCode == 1);
    CHECK_THAT(none.errors, ContainsSubstring("use --head and --tail"));
    CHECK(onlyHead.exitCode == 1);
    CHECK(same.exitCode == 1);
    CHECK_THAT(same.errors, ContainsSubstring("named as both the head and the tail"));
    CHECK(overInput.exitCode == 1);
    CHECK_THAT(overInput.errors, ContainsSubstring("would be written over the input"));
    CHECK(noCut.exitCode == 1);
    CHECK(output.exitCode == 1);
    CHECK(inPlace.exitCode == 1);
    CHECK(contentOf(input) == kFilm);
    CHECK_FALSE(std::filesystem::exists(scratch.of("out")));
}

TEST_CASE("a file that cannot be read fails with the code of a file that failed",
          "[e2e][CLI-PSPLIT-01]") {
    const Scratch scratch;

    const CliRun run = invoke({"split-file",
                               scratch.of("absent.srt"),
                               "--at",
                               "2",
                               "--head",
                               scratch.of("h.srt"),
                               "--tail",
                               scratch.of("t.srt")});

    CHECK(run.exitCode == 2);
    CHECK_THAT(run.errors, ContainsSubstring("absent.srt: does not exist"));
}
