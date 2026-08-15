// Re-timing a file from one frame rate to another, through the real binary.

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>

#include "cli_run.hpp"

using Catch::Matchers::ContainsSubstring;
using subedit::e2e::CliRun;
using subedit::e2e::contentOf;
using subedit::e2e::corpus;
using subedit::e2e::invoke;
using subedit::e2e::Scratch;

TEST_CASE("25 towards 23.976 retimes the whole file", "[e2e][CLI-FRAMERATE-01]") {
    // The factor is 25 ÷ 24000/1001 = 1001/960, and every position follows:
    //      1010 -> 1053        2020 -> 2106
    //   3600017 -> 3753768  3602000 -> 3755835
    const Scratch scratch;
    const std::string out = scratch.of("ntsc.srt");

    const CliRun run = invoke({"framerate",
                               "--from",
                               "25",
                               "--to",
                               "23.976",
                               "--output",
                               out,
                               corpus("valides/cadence.srt")});

    CHECK(run.exitCode == 0);
    const std::string written = contentOf(out);
    CHECK_THAT(written, ContainsSubstring("00:00:01,053 --> 00:00:02,106"));
    CHECK_THAT(written, ContainsSubstring("01:02:33,768 --> 01:02:35,835"));
}

TEST_CASE("the rounding happens once, and never on the frame grid", "[e2e][CLI-FRAMERATE-01]") {
    // Going through the frames answers 1043 and 3753750 — up to half a frame
    // away from the exact result. ADR 0013 exists for this, and the proof that
    // it holds is that neither number reaches the file.
    const Scratch scratch;
    const std::string out = scratch.of("once.srt");

    const CliRun run = invoke({"framerate",
                               "--from",
                               "25",
                               "--to",
                               "23.976",
                               "--output",
                               out,
                               corpus("valides/cadence.srt")});

    REQUIRE(run.exitCode == 0);
    const std::string written = contentOf(out);
    CHECK_THAT(written, !ContainsSubstring("00:00:01,043"));
    CHECK_THAT(written, !ContainsSubstring("01:02:33,750"));
}

TEST_CASE("the narration names the exact rate and not the label", "[e2e][CLI-FRAMERATE-01]") {
    const Scratch scratch;
    const std::string out = scratch.of("narrated.srt");

    const CliRun run = invoke({"framerate",
                               "--from",
                               "25",
                               "--to",
                               "23.976",
                               "--output",
                               out,
                               corpus("valides/cadence.srt")});

    // "23.976" is a label for 24000/1001, and the line reporting what happened
    // says which rate was used.
    CHECK_THAT(run.errors, ContainsSubstring("retimed from 25 to 24000/1001 fps"));
    CHECK(run.output.empty());
}

TEST_CASE("a rate of zero is refused", "[e2e][CLI-FRAMERATE-02]") {
    const Scratch scratch;
    const std::string out = scratch.of("zero.srt");

    const CliRun run = invoke(
        {"framerate", "--from", "0", "--to", "25", "--output", out, corpus("valides/cadence.srt")});

    CHECK(run.exitCode == 1);
    CHECK(run.output.empty());
    CHECK_FALSE(std::filesystem::exists(out));
    CHECK_THAT(run.errors, ContainsSubstring("strictly positive"));
}

TEST_CASE("a negative rate is refused", "[e2e][CLI-FRAMERATE-02]") {
    const Scratch scratch;
    const std::string out = scratch.of("negative.srt");

    const CliRun run = invoke(
        {"framerate", "--from", "25", "--to=-24", "--output", out, corpus("valides/cadence.srt")});

    CHECK(run.exitCode == 1);
    CHECK_FALSE(std::filesystem::exists(out));
    CHECK_THAT(run.errors, ContainsSubstring("strictly positive"));
}

TEST_CASE("a comma in a rate is refused", "[e2e][CLI-USAGE-02]") {
    const Scratch scratch;
    const CliRun run = invoke({"framerate",
                               "--from",
                               "25",
                               "--to",
                               "23,976",
                               "--output-dir",
                               scratch.path(),
                               corpus("valides/cadence.srt")});

    CHECK(run.exitCode == 1);
    CHECK_THAT(run.errors, ContainsSubstring("decimal point"));
}

TEST_CASE("a retiming without a destination writes nothing", "[e2e][CLI-CONVERT-03]") {
    const CliRun run =
        invoke({"framerate", "--from", "25", "--to", "23.976", corpus("valides/cadence.srt")});

    CHECK(run.exitCode == 1);
    CHECK(run.output.empty());
}
