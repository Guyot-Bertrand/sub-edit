// Correcting a file from two known-good points, through the real binary.

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

TEST_CASE("the two references land exactly where they were asked to", "[e2e][CLI-TRANSFORM-01]") {
    const Scratch scratch;
    const std::string out = scratch.of("references.srt");

    const CliRun run = invoke({"transform",
                               "--first",
                               "1=00:00:01.000",
                               "--last",
                               "3=00:00:10.000",
                               "--output",
                               out,
                               corpus("valides/trois.srt")});

    CHECK(run.exitCode == 0);
    CHECK_THAT(contentOf(out), ContainsSubstring("00:00:01,000 --> "));
    CHECK_THAT(contentOf(out), ContainsSubstring("00:00:10,000 --> "));
}

TEST_CASE("every other position follows the two references", "[e2e][CLI-TRANSFORM-01]") {
    // Worked out by hand, on the corpus file of three subtitles.
    // r = (10000 - 1000) / (9000 - 1000) = 9/8, and t' = 1000 + (t - 1000) × 9/8:
    //
    //    3000 -> 1000 + 2000 × 9/8  = 3250
    //    5001 -> 1000 + 4001 × 9/8  = 1000 + 4501.125 -> 5501
    //    7000 -> 1000 + 6000 × 9/8  = 7750
    //   11000 -> 1000 + 10000 × 9/8 = 12250
    const Scratch scratch;
    const std::string out = scratch.of("between.srt");

    const CliRun run = invoke({"transform",
                               "--first",
                               "1=00:00:01.000",
                               "--last",
                               "3=00:00:10.000",
                               "--output",
                               out,
                               corpus("valides/trois.srt")});

    REQUIRE(run.exitCode == 0);
    const std::string written = contentOf(out);
    CHECK_THAT(written, ContainsSubstring("00:00:01,000 --> 00:00:03,250"));
    CHECK_THAT(written, ContainsSubstring("00:00:05,501 --> 00:00:07,750"));
    CHECK_THAT(written, ContainsSubstring("00:00:10,000 --> 00:00:12,250"));
}

TEST_CASE("a reference in seconds says the same thing as one in a timestamp",
          "[e2e][CLI-TRANSFORM-01]") {
    const Scratch scratch;
    const std::string one = scratch.of("seconds.srt");
    const std::string other = scratch.of("stamp.srt");

    CHECK(invoke({"--quiet",
                  "transform",
                  "--first",
                  "1=1",
                  "--last",
                  "3=10",
                  "--output",
                  one,
                  corpus("valides/trois.srt")})
              .exitCode == 0);
    CHECK(invoke({"--quiet",
                  "transform",
                  "--first",
                  "1=00:00:01.000",
                  "--last",
                  "3=00:00:10.000",
                  "--output",
                  other,
                  corpus("valides/trois.srt")})
              .exitCode == 0);

    CHECK(contentOf(one) == contentOf(other));
}

TEST_CASE("two references on one subtitle are refused before anything is read",
          "[e2e][CLI-TRANSFORM-02]") {
    const Scratch scratch;
    const std::string out = scratch.of("confounded.srt");

    const CliRun run = invoke({"transform",
                               "--first",
                               "2=00:00:01.000",
                               "--last",
                               "2=00:00:04.000",
                               "--output",
                               out,
                               corpus("valides/trois.srt")});

    // A usage error, and not a failure to process: the two options are wrong
    // whatever the files are, so one message is owed and not one per file.
    CHECK(run.exitCode == 1);
    CHECK(run.output.empty());
    CHECK_FALSE(std::filesystem::exists(out));
    CHECK_THAT(run.errors, ContainsSubstring("subtitle 2"));
}

TEST_CASE("a subtitle past the end is refused, naming the bound", "[e2e][CLI-TRANSFORM-02]") {
    const Scratch scratch;
    const std::string out = scratch.of("beyond.srt");

    const CliRun run = invoke({"transform",
                               "--first",
                               "1=00:00:01.000",
                               "--last",
                               "9=00:00:10.000",
                               "--output",
                               out,
                               corpus("valides/trois.srt")});

    // The file is what settles it, so this one is a processing failure.
    CHECK(run.exitCode == 2);
    CHECK_FALSE(std::filesystem::exists(out));
    CHECK_THAT(run.errors, ContainsSubstring("subtitle 9"));
    CHECK_THAT(run.errors, ContainsSubstring("3 subtitles"));
}

TEST_CASE("a transform that would go before the origin is refused", "[e2e][CLI-TRANSFORM-01]") {
    // r = (1000 - 100) / (9000 - 5001) = 300/1333, and the first subtitle sits
    // before the first reference: 100 + (1000 - 5001) × 300/1333 = -800.
    const Scratch scratch;
    const std::string out = scratch.of("negative.srt");

    const CliRun run = invoke({"transform",
                               "--first",
                               "2=00:00:00.100",
                               "--last",
                               "3=00:00:01.000",
                               "--output",
                               out,
                               corpus("valides/trois.srt")});

    CHECK(run.exitCode == 2);
    CHECK_FALSE(std::filesystem::exists(out));
    CHECK_THAT(run.errors, ContainsSubstring("subtitle 1"));
    CHECK_THAT(run.errors, ContainsSubstring("before the origin"));
}

TEST_CASE("a reference without an equals sign is refused", "[e2e][CLI-USAGE-02]") {
    const Scratch scratch;
    const CliRun run = invoke({"transform",
                               "--first",
                               "1",
                               "--last",
                               "3=00:00:10.000",
                               "--output-dir",
                               scratch.path(),
                               corpus("valides/trois.srt")});

    CHECK(run.exitCode == 1);
    CHECK(run.output.empty());
    CHECK_THAT(run.errors, ContainsSubstring("<index>=<time>"));
}

TEST_CASE("a subtitle numbered from zero is refused", "[e2e][CLI-USAGE-02]") {
    const Scratch scratch;
    const CliRun run = invoke({"transform",
                               "--first",
                               "0=00:00:01.000",
                               "--last",
                               "3=00:00:10.000",
                               "--output-dir",
                               scratch.path(),
                               corpus("valides/trois.srt")});

    CHECK(run.exitCode == 1);
    CHECK_THAT(run.errors, ContainsSubstring("counted from 1"));
}

TEST_CASE("a transform without a destination writes nothing", "[e2e][CLI-CONVERT-03]") {
    const CliRun run = invoke({"transform",
                               "--first",
                               "1=00:00:01.000",
                               "--last",
                               "3=00:00:10.000",
                               corpus("valides/trois.srt")});

    CHECK(run.exitCode == 1);
    CHECK(run.output.empty());
}
