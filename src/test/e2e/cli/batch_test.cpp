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

// The four cases below are about a batch that WRITES, on `shift` which stands
// for every subcommand that does. **They record what the tool does today; they
// decide nothing.** Whether a collision should be refused, whether the output
// directory should be created, whether an existing destination should be
// overwritten: those are the framing's to settle (#542). Each case is named for
// what it observes, so that changing the behaviour is a visible diff here.

TEST_CASE("two inputs of one base name in --output-dir: the last one overwrites the first",
          "[e2e][CLI-BATCH-01]") {
    const Scratch scratch;
    const std::string first = writeSrt(scratch, "a/film.srt", 1);
    const std::string second = writeSrt(scratch, "b/film.srt", 2);
    std::filesystem::create_directories(scratch.of("out"));

    const CliRun run =
        invoke({"shift", "--by", "1", "--output-dir", scratch.of("out"), first, second});

    // Both are reported as written, to the same path; no warning, code 0.
    CHECK(run.exitCode == 0);
    CHECK_THAT(run.errors, ContainsSubstring("2 of 2 files shifted\n"));
    CHECK_THAT(run.errors, !ContainsSubstring("overwrit"));
    // Only the second survives, shifted by one second, as written by hand.
    CHECK(contentOf(scratch.of("out/film.srt")) ==
          "1\n00:00:02,000 --> 00:00:02,500\nb/film.srt 1\n\n"
          "2\n00:00:03,000 --> 00:00:03,500\nb/film.srt 2\n\n");
}

TEST_CASE("an output directory that does not exist is not created: nothing is written, code 2",
          "[e2e][CLI-BATCH-02]") {
    const Scratch scratch;
    const std::string input = writeSrt(scratch, "in/film.srt");
    const std::string missing = scratch.of("absent/deeper");

    const CliRun run = invoke({"shift", "--by", "1", "--output-dir", missing, input});

    // The directory is not created. The failure is worded as "cannot be read",
    // which is the wording of an input that cannot be opened, not of an output.
    CHECK(run.exitCode == 2);
    CHECK_THAT(run.errors, ContainsSubstring(missing + "/film.srt: cannot be read"));
    // A single input has no summary line: the failure line is all there is.
    CHECK_THAT(run.errors, !ContainsSubstring("files shifted"));
    CHECK(!std::filesystem::exists(scratch.of("absent")));
}

TEST_CASE("an existing destination is overwritten, without a question or an option",
          "[e2e][CLI-BATCH-01]") {
    const Scratch scratch;
    const std::string input = writeSrt(scratch, "in/film.srt", 1);
    const std::string destination = writeFile(scratch, "out/film.srt", "precious\n");

    const CliRun run = invoke({"shift", "--by", "1", "--output-dir", scratch.of("out"), input});

    CHECK(run.exitCode == 0);
    CHECK(contentOf(destination) == "1\n00:00:02,000 --> 00:00:02,500\nin/film.srt 1\n\n");
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
