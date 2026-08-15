// Moving a file in time, through the real binary.

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

TEST_CASE("shifting moves every position", "[e2e][CLI-SHIFT-01]") {
    const Scratch scratch;
    const std::string out = scratch.of("forward.srt");

    const CliRun run =
        invoke({"shift", "--by", "2.999", "--output", out, corpus("valides/minimal.srt")});

    CHECK(run.exitCode == 0);
    CHECK_THAT(contentOf(out), ContainsSubstring("00:00:03,999 --> 00:00:06,499"));
}

TEST_CASE("shifting backwards moves every position", "[e2e][CLI-SHIFT-01]") {
    const Scratch scratch;
    const std::string out = scratch.of("backward.srt");

    const CliRun run =
        invoke({"shift", "--by", "-0.500", "--output", out, corpus("valides/minimal.srt")});

    CHECK(run.exitCode == 0);
    CHECK_THAT(contentOf(out), ContainsSubstring("00:00:00,500 --> 00:00:03,000"));
}

TEST_CASE("a timestamp says the same thing as a count of seconds", "[e2e][CLI-SHIFT-01]") {
    const Scratch scratch;
    const std::string one = scratch.of("seconds.srt");
    const std::string other = scratch.of("stamp.srt");

    CHECK(invoke(
              {"--quiet", "shift", "--by", "7.001", "--output", one, corpus("valides/minimal.srt")})
              .exitCode == 0);
    CHECK(invoke({"--quiet",
                  "shift",
                  "--by",
                  "00:00:07.001",
                  "--output",
                  other,
                  corpus("valides/minimal.srt")})
              .exitCode == 0);

    CHECK(contentOf(one) == contentOf(other));
}

TEST_CASE("a shift before the origin is refused, naming the subtitle", "[e2e][CLI-SHIFT-02]") {
    const Scratch scratch;
    const std::string out = scratch.of("refused.srt");

    const CliRun run =
        invoke({"shift", "--by", "-7.001", "--output", out, corpus("valides/minimal.srt")});

    CHECK(run.exitCode == 2);
    CHECK_FALSE(std::filesystem::exists(out));
    CHECK_THAT(run.errors, ContainsSubstring("subtitle 1"));
    CHECK_THAT(run.errors, ContainsSubstring("before the origin"));
}

TEST_CASE("a comma in the amount is refused", "[e2e][CLI-USAGE-02]") {
    const Scratch scratch;
    const CliRun run = invoke(
        {"shift", "--by", "-7,001", "--output-dir", scratch.path(), corpus("valides/minimal.srt")});

    CHECK(run.exitCode == 1);
    CHECK_THAT(run.errors, ContainsSubstring("decimal point"));
}

TEST_CASE("an amount finer than a millisecond is refused", "[e2e][CLI-USAGE-02]") {
    const Scratch scratch;
    const CliRun run = invoke(
        {"shift", "--by", "1.0005", "--output-dir", scratch.path(), corpus("valides/minimal.srt")});

    CHECK(run.exitCode == 1);
    CHECK_THAT(run.errors, ContainsSubstring("millisecond"));
}
