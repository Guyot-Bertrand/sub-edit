// `append`, through the real binary: files one after another into a single one,
// each shifted from the end of what precedes it.
//
// The expected files are worked out by hand — the end of the last subtitle is the
// shift of the next file — and never read back from the program.

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

/// Two subtitles; the last ends at four seconds.
const std::string kFirst = "1\n00:00:01,000 --> 00:00:02,000\nun\n\n"
                           "2\n00:00:03,000 --> 00:00:04,000\ndeux\n\n";

/// One subtitle, italic, starting at half a second.
const std::string kSecond = "1\n00:00:00,500 --> 00:00:01,500\n<i>trois</i>\n\n";

/// One subtitle in another format, starting at one second.
const std::string kThird = "WEBVTT\n\n00:00:01.000 --> 00:00:02.000\nquatre\n";

/// One subtitle in Advanced SSA, with a position tag SubRip cannot hold.
const std::string kAss =
    "[Script Info]\nScriptType: v4.00+\n\n"
    "[V4+ Styles]\nFormat: Name, Fontname, Fontsize\nStyle: Default,Arial,20\n\n"
    "[Events]\nFormat: Layer, Start, End, Style, Name, MarginL, MarginR, "
    "MarginV, Effect, Text\n"
    "Dialogue: 0,0:00:01.00,0:00:02.00,Default,,0,0,0,,{\\pos(10,10)}cinq\\Nsix\n";

std::string anonymised(std::string text, const std::string& what, const std::string& placeholder) {
    for (std::size_t at = text.find(what); at != std::string::npos;
         at = text.find(what, at + placeholder.size())) {
        text.replace(at, what.size(), placeholder);
    }
    return text;
}

} // namespace

TEST_CASE("two files come out one after the other, the second shifted by the end of the first",
          "[e2e][CLI-APPEND-01]") {
    const Scratch scratch;
    const std::string first = writeFile(scratch, "in/a.srt", kFirst);
    const std::string second = writeFile(scratch, "in/b.srt", kSecond);
    const std::string out = scratch.of("out/all.srt");

    const CliRun run = invoke({"append", first, second, "--output", out});

    CHECK(run.exitCode == 0);
    CHECK_THAT(contentOf(out), MatchesFile(corpus("attendus/append/deux-fichiers.srt")));
    CHECK_THAT(run.errors, ContainsSubstring("b.srt: appended 1 subtitle"));
    CHECK_THAT(run.errors, ContainsSubstring("3 subtitles from 2 files -> "));
}

TEST_CASE("three files in two formats come out in the format of the first",
          "[e2e][CLI-APPEND-01]") {
    const Scratch scratch;
    const std::string first = writeFile(scratch, "in/a.srt", kFirst);
    const std::string second = writeFile(scratch, "in/b.srt", kSecond);
    const std::string third = writeFile(scratch, "in/c.vtt", kThird);
    const std::string out = scratch.of("out/all.srt");

    const CliRun run = invoke({"append", first, second, third, "--output", out});

    CHECK(run.exitCode == 0);
    // The third starts one second into its own file, and the second ended at 5.5.
    CHECK_THAT(contentOf(out), MatchesFile(corpus("attendus/append/trois-fichiers.srt")));
}

TEST_CASE("what crossing into the format of the first costs is said, file by file",
          "[e2e][CLI-APPEND-01]") {
    const Scratch scratch;
    const std::string first = writeFile(scratch, "in/a.srt", kFirst);
    const std::string second = writeFile(scratch, "in/d.ass", kAss);
    const std::string out = scratch.of("out/all.srt");

    const CliRun run = invoke({"append", first, second, "--output", out});

    CHECK(run.exitCode == 0);
    CHECK_THAT(run.errors,
               ContainsSubstring("d.ass: appended 1 subtitle; Advanced SSA into SubRip: "
                                 "1 tag dropped, the header was dropped"));
}

TEST_CASE("the record is one object for the whole run, with what each file cost",
          "[e2e][CLI-APPEND-01][CLI-JSON-09]") {
    const Scratch scratch;
    const std::string first = writeFile(scratch, "in/a.srt", kFirst);
    const std::string second = writeFile(scratch, "in/b.srt", kSecond);
    const std::string third = writeFile(scratch, "in/d.ass", kAss);

    const CliRun run = invoke({"--format",
                               "json",
                               "append",
                               first,
                               second,
                               third,
                               "--output",
                               scratch.of("out/all.srt")});

    CHECK(run.exitCode == 0);
    CHECK_THAT(anonymised(run.output, scratch.path(), "<scratch>"),
               MatchesFile(corpus("attendus/json/append.jsonl")));
}

TEST_CASE("a dry run says what would be written and writes nothing",
          "[e2e][CLI-APPEND-01][CLI-DRYRUN-01]") {
    const Scratch scratch;
    const std::string first = writeFile(scratch, "in/a.srt", kFirst);
    const std::string second = writeFile(scratch, "in/b.srt", kSecond);
    const std::string out = scratch.of("out/all.srt");

    // With a destination, and without one: a dry run has nowhere to write to.
    const CliRun named = invoke({"append", "--dry-run", first, second, "--output", out});
    const CliRun bare = invoke({"append", "--dry-run", first, second});

    CHECK(named.exitCode == 0);
    CHECK(bare.exitCode == 0);
    CHECK_THAT(named.errors,
               ContainsSubstring("3 subtitles from 2 files (dry run, nothing written)"));
    CHECK_THAT(bare.errors, ContainsSubstring("(dry run, nothing written)"));
    CHECK_FALSE(std::filesystem::exists(out));
    CHECK_FALSE(std::filesystem::exists(scratch.of("out")));
}

TEST_CASE("a file that cannot be read stops the run, and nothing is written",
          "[e2e][CLI-APPEND-01]") {
    const Scratch scratch;
    const std::string first = writeFile(scratch, "in/a.srt", kFirst);
    // A WebVTT with its header and no cue: no reader accepts a file holding no
    // subtitle, so the middle file of a film in three parts cannot be an empty one.
    const std::string empty = writeFile(scratch, "in/empty.vtt", "WEBVTT\n");
    const std::string third = writeFile(scratch, "in/c.vtt", kThird);
    const std::string out = scratch.of("out/all.srt");

    const CliRun run = invoke({"append", first, empty, third, "--output", out});

    CHECK(run.exitCode == 2);
    CHECK_THAT(run.errors,
               ContainsSubstring("empty.vtt: holds nothing recognisable as a subtitle"));
    CHECK_FALSE(std::filesystem::exists(out));
}

TEST_CASE("a missing file stops the run with the code of a file that failed",
          "[e2e][CLI-APPEND-01]") {
    const Scratch scratch;
    const std::string first = writeFile(scratch, "in/a.srt", kFirst);

    const CliRun run =
        invoke({"append", first, scratch.of("in/absent.srt"), "--output", scratch.of("all.srt")});

    CHECK(run.exitCode == 2);
    CHECK_THAT(run.errors, ContainsSubstring("absent.srt: does not exist"));
    CHECK_FALSE(std::filesystem::exists(scratch.of("all.srt")));
}

TEST_CASE("a destination that is one of the inputs is refused before anything is read",
          "[e2e][CLI-APPEND-01]") {
    const Scratch scratch;
    const std::string first = writeFile(scratch, "in/a.srt", kFirst);
    const std::string second = writeFile(scratch, "in/b.srt", kSecond);

    const CliRun overBase = invoke({"append", first, second, "--output", first});
    const CliRun overOther = invoke({"append", first, second, "--output", second});

    CHECK(overBase.exitCode == 1);
    CHECK(overOther.exitCode == 1);
    CHECK_THAT(overOther.errors, ContainsSubstring("would be written over the input"));
    CHECK(contentOf(first) == kFirst);
    CHECK(contentOf(second) == kSecond);
}

TEST_CASE("append takes one output, and asks for it", "[e2e][CLI-APPEND-01]") {
    const Scratch scratch;
    const std::string first = writeFile(scratch, "in/a.srt", kFirst);
    const std::string second = writeFile(scratch, "in/b.srt", kSecond);

    const CliRun none = invoke({"append", first, second});
    const CliRun directory = invoke({"append", first, second, "--output-dir", scratch.of("out")});
    const CliRun inPlace = invoke({"append", first, second, "--in-place"});
    const CliRun alone = invoke({"append", first, "--output", scratch.of("out/all.srt")});

    CHECK(none.exitCode == 1);
    CHECK_THAT(none.errors, ContainsSubstring("append writes one file, use --output"));
    CHECK(directory.exitCode == 1);
    CHECK(inPlace.exitCode == 1);
    CHECK(alone.exitCode == 1);
    CHECK_THAT(alone.errors, ContainsSubstring("at least one file to put after it"));
    CHECK(contentOf(first) == kFirst);
    CHECK_FALSE(std::filesystem::exists(scratch.of("out")));
}
