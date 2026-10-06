// Several files in one invocation, and what the exit code says about them.

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include <filesystem>
#include <string>

#include "cli_run.hpp"

using Catch::Matchers::ContainsSubstring;
using subedit::e2e::CliRun;
using subedit::e2e::contentOf;
using subedit::e2e::corpus;
using subedit::e2e::invoke;
using subedit::e2e::Scratch;
using subedit::e2e::srtText;
using subedit::e2e::writeFile;
using subedit::e2e::writeSrt;
using subedit::e2e::writeUnreadable;

namespace {

const std::string kGood = corpus("valides/minimal.srt");
const std::string kOther = corpus("valides/minimal.vtt");
const std::string kUnreadable = corpus("malformes/vide.srt");
const std::string kMissing = corpus("valides/rien-du-tout.srt");

} // namespace

TEST_CASE("every file is processed, whatever happened to the ones before", "[e2e][CLI-BATCH-01]") {
    const CliRun run = invoke({"inspect", kUnreadable, kGood});

    // The failure of the first does not stop the second: the point of taking a
    // batch is to learn about all of it in one go.
    CHECK_THAT(run.output, ContainsSubstring(kGood + "\n"));
    CHECK_THAT(run.errors, ContainsSubstring(kUnreadable));
}

TEST_CASE("all files succeeding is code 0", "[e2e][CLI-BATCH-02]") {
    CHECK(invoke({"inspect", kGood, kOther}).exitCode == 0);
}

TEST_CASE("no file surviving is code 2", "[e2e][CLI-BATCH-02]") {
    CHECK(invoke({"inspect", kUnreadable, kMissing}).exitCode == 2);
}

TEST_CASE("some files surviving is code 3", "[e2e][CLI-BATCH-02]") {
    // Told apart from code 2 on purpose: a script must be able to act on
    // "nothing worked" and on "one is missing" without reading the output.
    CHECK(invoke({"inspect", kGood, kUnreadable}).exitCode == 3);
}

TEST_CASE("a missing file is named rather than counted", "[e2e][CLI-BATCH-01]") {
    const CliRun run = invoke({"inspect", kMissing});

    CHECK(run.exitCode == 2);
    CHECK_THAT(run.errors, ContainsSubstring(kMissing));
    CHECK_THAT(run.errors, ContainsSubstring("does not exist"));
}

TEST_CASE("the summary counts the failures", "[e2e][CLI-OUTPUT-05]") {
    CHECK_THAT(invoke({"inspect", kGood, kUnreadable}).errors,
               ContainsSubstring("1 of 2 files inspected, 1 failed\n"));
}

TEST_CASE("a usage error stops before any file is touched", "[e2e][CLI-USAGE-03]") {
    const CliRun run = invoke({"inspect", "--inexistant", kGood});

    CHECK(run.exitCode == 1);
    // Not a single report: the command line is judged whole, before the first
    // file is opened.
    CHECK(run.output.empty());
}

// The cases below are about a batch that WRITES, on `shift` which stands for
// every subcommand that does. ADR 0039 settled what the first four of them only
// recorded (#544): a collision is refused, an input is not written over without
// `--in-place`, the output directory is created, and an existing destination is
// overwritten. Each case is named for what it observes.

TEST_CASE("two inputs of one base name in --output-dir are refused, and nothing is written",
          "[e2e][CLI-BATCH-03]") {
    const Scratch scratch;
    const std::string first = writeSrt(scratch, "a/film.srt", 1);
    const std::string second = writeSrt(scratch, "b/film.srt", 2);

    const std::string firstBefore = contentOf(first);
    const std::string secondBefore = contentOf(second);

    const CliRun run =
        invoke({"shift", "--by", "1", "--output-dir", scratch.of("out"), first, second});

    // A usage error, code 1, naming the destination and both inputs; nothing on
    // standard output, and the inputs as they were.
    CHECK(run.exitCode == 1);
    CHECK(run.output.empty());
    CHECK(contentOf(first) == firstBefore);
    CHECK(contentOf(second) == secondBefore);
    CHECK_THAT(run.errors,
               ContainsSubstring(scratch.of("out/film.srt") + ": would be written by both " +
                                 first + " and " + second));
    CHECK_THAT(run.errors, !ContainsSubstring("files shifted"));
    // Not one file, and not even the directory: it is made after the validation.
    CHECK(!std::filesystem::exists(scratch.of("out")));
}

TEST_CASE("a collision is judged on the extension the destination ends up with",
          "[e2e][CLI-BATCH-03]") {
    const Scratch scratch;
    const std::string srt = writeSrt(scratch, "a/film.srt", 1);
    const std::string vtt =
        writeFile(scratch, "b/film.vtt", "WEBVTT\n\n00:01.000 --> 00:02.000\nhi\n");

    const std::string srtBefore = contentOf(srt);
    const std::string vttBefore = contentOf(vtt);

    // Both become out/film.vtt: neither input name says so.
    const CliRun run =
        invoke({"convert", "--to", "vtt", "--output-dir", scratch.of("out"), srt, vtt});

    CHECK(run.exitCode == 1);
    CHECK(run.output.empty());
    CHECK_THAT(run.errors,
               ContainsSubstring(scratch.of("out/film.vtt") + ": would be written by both"));
    CHECK(!std::filesystem::exists(scratch.of("out")));
    CHECK(contentOf(srt) == srtBefore);
    CHECK(contentOf(vtt) == vttBefore);
}

TEST_CASE("an input is not written over without --in-place, however the destination is spelled",
          "[e2e][CLI-BATCH-04]") {
    const Scratch scratch;
    const std::string input = writeSrt(scratch, "in/film.srt", 1);
    const std::string before = contentOf(input);

    // The directory the input lies in, then the file itself.
    const CliRun directory =
        invoke({"shift", "--by", "1", "--output-dir", scratch.of("in"), input});
    const CliRun file = invoke({"shift", "--by", "1", "--output", input, input});
    // The same file under another spelling.
    const CliRun spelled =
        invoke({"shift", "--by", "1", "--output-dir", scratch.of("in/../in"), input});

    for (const CliRun* run : {&directory, &file, &spelled}) {
        CHECK(run->exitCode == 1);
        CHECK_THAT(run->errors, ContainsSubstring("would be written over the input " + input));
    }
    CHECK(contentOf(input) == before);
}

TEST_CASE("a symbolic link to the input's directory does not hide that it is the input",
          "[e2e][CLI-BATCH-04]") {
    const Scratch scratch;
    const std::string input = writeSrt(scratch, "in/film.srt", 1);
    const std::string before = contentOf(input);
    std::filesystem::create_directory_symlink(scratch.of("in"), scratch.of("link"));

    const CliRun run = invoke({"shift", "--by", "1", "--output-dir", scratch.of("link"), input});

    // The two spellings differ, the file does not: only the system can say so.
    CHECK(run.exitCode == 1);
    CHECK_THAT(run.errors, ContainsSubstring("would be written over the input " + input));
    CHECK(contentOf(input) == before);
}

TEST_CASE("a file named on the command line is never filtered by its extension",
          "[e2e][CLI-BATCH-12]") {
    const Scratch scratch;
    // SubRip under a name no format uses, and a notes file nobody would walk into.
    const std::string odd = writeFile(scratch, "in/film.dat", srtText("odd", 1));
    const std::string notes = writeFile(scratch, "in/notes.txt", srtText("notes", 1));

    const CliRun run =
        invoke({"shift", "--by", "1", "--output-dir", scratch.of("out"), odd, notes});

    CHECK(run.exitCode == 0);
    CHECK(contentOf(scratch.of("out/film.dat")) == "1\n00:00:02,000 --> 00:00:02,500\nodd 1\n\n");
    CHECK(contentOf(scratch.of("out/notes.txt")) ==
          "1\n00:00:02,000 --> 00:00:02,500\nnotes 1\n\n");
}

TEST_CASE("writing in place overwrites the input, and nothing refuses it", "[e2e][CLI-BATCH-04]") {
    const Scratch scratch;
    const std::string input = writeSrt(scratch, "in/film.srt", 1);

    const CliRun run = invoke({"shift", "--by", "1", "--in-place", input});

    CHECK(run.exitCode == 0);
    CHECK(contentOf(input) == "1\n00:00:02,000 --> 00:00:02,500\nin/film.srt 1\n\n");
}

TEST_CASE("an output directory that does not exist is created, parents included",
          "[e2e][CLI-BATCH-05]") {
    const Scratch scratch;
    const std::string input = writeSrt(scratch, "in/film.srt", 1);

    const CliRun run =
        invoke({"shift", "--by", "1", "--output-dir", scratch.of("absent/deeper"), input});

    CHECK(run.exitCode == 0);
    CHECK(contentOf(scratch.of("absent/deeper/film.srt")) ==
          "1\n00:00:02,000 --> 00:00:02,500\nin/film.srt 1\n\n");
}

TEST_CASE("the directory --output writes into is created too", "[e2e][CLI-BATCH-05]") {
    const Scratch scratch;
    const std::string input = writeSrt(scratch, "in/film.srt", 1);

    const CliRun run =
        invoke({"shift", "--by", "1", "--output", scratch.of("x/y/renamed.srt"), input});

    CHECK(run.exitCode == 0);
    CHECK(std::filesystem::exists(scratch.of("x/y/renamed.srt")));
}

TEST_CASE("an output directory that cannot be made is said once, and no file is read",
          "[e2e][CLI-BATCH-05]") {
    const Scratch scratch;
    const std::string first = writeSrt(scratch, "in/one.srt", 1);
    const std::string second = writeSrt(scratch, "in/two.srt", 1);
    // A file where the directory should go: it cannot be made.
    const std::string blocker = writeFile(scratch, "blocker", "not a directory");

    const CliRun run =
        invoke({"shift", "--by", "1", "--output-dir", blocker + "/out", first, second});

    CHECK(run.exitCode == 2);
    CHECK_THAT(run.errors, ContainsSubstring(blocker + "/out: cannot be created"));
    // Once, whatever the number of inputs, and before any of them is touched.
    CHECK(run.errors.find("cannot be created") == run.errors.rfind("cannot be created"));
    CHECK_THAT(run.errors, !ContainsSubstring(first));
    CHECK_THAT(run.errors, !ContainsSubstring("files shifted"));
}

TEST_CASE("an existing destination is overwritten, and the manual says so", "[e2e][CLI-BATCH-07]") {
    const Scratch scratch;
    const std::string input = writeSrt(scratch, "in/film.srt", 1);
    const std::string destination = writeFile(scratch, "out/film.srt", "precious\n");

    const CliRun run = invoke({"shift", "--by", "1", "--output-dir", scratch.of("out"), input});

    CHECK(run.exitCode == 0);
    CHECK(contentOf(destination) == "1\n00:00:02,000 --> 00:00:02,500\nin/film.srt 1\n\n");
    // Atomically: the temporary the write goes through is gone.
    CHECK(!std::filesystem::exists(destination + ".subedit-tmp"));
}

TEST_CASE("a write that fails in the middle of a batch is worded as a write, and is code 3",
          "[e2e][CLI-BATCH-02][CLI-BATCH-06]") {
    const Scratch scratch;
    const std::string before = writeSrt(scratch, "in/before.srt", 1);
    const std::string middle = writeSrt(scratch, "in/middle.srt", 1);
    const std::string after = writeSrt(scratch, "in/after.srt", 1);
    // A directory where the destination of the second should be: the system
    // refuses to put a file over it.
    std::filesystem::create_directories(scratch.of("out/middle.srt/inside"));

    const CliRun run =
        invoke({"shift", "--by", "1", "--output-dir", scratch.of("out"), before, middle, after});

    CHECK(run.exitCode == 3);
    CHECK_THAT(
        run.errors,
        ContainsSubstring(middle + ": " + scratch.of("out/middle.srt") + ": cannot be written"));
    CHECK_THAT(run.errors, !ContainsSubstring("cannot be read"));
    CHECK_THAT(run.errors, ContainsSubstring("2 of 3 files shifted, 1 failed\n"));
    CHECK(std::filesystem::exists(scratch.of("out/before.srt")));
    CHECK(std::filesystem::exists(scratch.of("out/after.srt")));
    // And nothing was left next to the refused destination.
    CHECK(!std::filesystem::exists(scratch.of("out/middle.srt.subedit-tmp")));
}

TEST_CASE(
    "an unreadable file in the middle of a writing batch: the others are written, it is named",
    "[e2e][CLI-BATCH-01][CLI-BATCH-02]") {
    const Scratch scratch;
    const std::string before = writeSrt(scratch, "in/before.srt", 1);
    const std::string broken = writeUnreadable(scratch, "in/broken.srt");
    const std::string after = writeSrt(scratch, "in/after.srt", 1);
    std::filesystem::create_directories(scratch.of("out"));

    const CliRun run =
        invoke({"shift", "--by", "1", "--output-dir", scratch.of("out"), before, broken, after});

    CHECK(run.exitCode == 3);
    CHECK_THAT(run.errors, ContainsSubstring(broken + ": is in no format this tool knows"));
    CHECK_THAT(run.errors, ContainsSubstring("2 of 3 files shifted, 1 failed\n"));
    CHECK(contentOf(scratch.of("out/before.srt")) ==
          "1\n00:00:02,000 --> 00:00:02,500\nin/before.srt 1\n\n");
    CHECK(contentOf(scratch.of("out/after.srt")) ==
          "1\n00:00:02,000 --> 00:00:02,500\nin/after.srt 1\n\n");
    CHECK(!std::filesystem::exists(scratch.of("out/broken.srt")));

    // And when nothing survives, code 2 and the same account.
    const std::string other = writeUnreadable(scratch, "in/other.srt");
    const CliRun none =
        invoke({"shift", "--by", "1", "--output-dir", scratch.of("out"), broken, other});
    CHECK(none.exitCode == 2);
    CHECK_THAT(none.errors, ContainsSubstring("0 of 2 files shifted, 2 failed\n"));
}

TEST_CASE("when several things are wrong the same one is said first, whatever the subcommand",
          "[e2e][CLI-USAGE-03]") {
    // One command line with four mistakes, mended one at a time: the range, then the
    // inputs, then the translation, then the destination — the order `prepare` reads
    // them in for every subcommand, and not the one each subcommand once chose.
    const Scratch scratch;
    const std::string directory = scratch.of("films");
    std::filesystem::create_directories(directory);
    const std::string file =
        writeFile(scratch, "films/a.srt", "1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n");

    const CliRun range = invoke({"replace", "--range", "5-2", "-t", file, "a", "b", directory});
    const CliRun inputs = invoke({"replace", "--range", "1-2", "-t", file, "a", "b", directory});
    const CliRun pairing = invoke({"replace", "--range", "1-2", "-t", file, "a", "b", file});
    const CliRun destination = invoke({"replace",
                                       "--range",
                                       "1-2",
                                       "-t",
                                       file,
                                       "--document",
                                       "translation",
                                       "a",
                                       "b",
                                       scratch.of("films/b.srt")});

    for (const CliRun* run : {&range, &inputs, &pairing, &destination}) {
        CHECK(run->exitCode == 1);
        CHECK(run->output.empty());
    }
    CHECK_THAT(range.errors, ContainsSubstring("--range"));
    CHECK_THAT(inputs.errors, ContainsSubstring("is a directory"));
    CHECK_THAT(pairing.errors, ContainsSubstring("use --document translation"));
    CHECK_THAT(destination.errors, ContainsSubstring("no destination given"));
}
