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
using subedit::e2e::invoke;

namespace {

std::string corpus(const std::string& relative) {
    return (std::filesystem::path{SUBEDIT_TEST_DATA_DIR} / relative).string();
}

std::filesystem::path scratch(const std::string& name) {
    const std::filesystem::path directory =
        std::filesystem::temp_directory_path() / "subedit-shift-e2e";
    std::filesystem::create_directories(directory);
    return directory / name;
}

std::string contentOf(const std::filesystem::path& path) {
    const std::ifstream file{path, std::ios::binary};
    std::ostringstream all;
    all << file.rdbuf();
    return all.str();
}

} // namespace

TEST_CASE("shifting moves every position", "[e2e][CLI-SHIFT-01]") {
    const std::filesystem::path out = scratch("forward.srt");
    std::filesystem::remove(out);

    const CliRun run =
        invoke({"shift", "--by", "2.999", "--output", out.string(), corpus("valides/minimal.srt")});

    CHECK(run.exitCode == 0);
    CHECK_THAT(contentOf(out), ContainsSubstring("00:00:03,999 --> 00:00:06,499"));
    std::filesystem::remove(out);
}

TEST_CASE("shifting backwards moves every position", "[e2e][CLI-SHIFT-01]") {
    const std::filesystem::path out = scratch("backward.srt");
    std::filesystem::remove(out);

    const CliRun run = invoke(
        {"shift", "--by", "-0.500", "--output", out.string(), corpus("valides/minimal.srt")});

    CHECK(run.exitCode == 0);
    CHECK_THAT(contentOf(out), ContainsSubstring("00:00:00,500 --> 00:00:03,000"));
    std::filesystem::remove(out);
}

TEST_CASE("a timestamp says the same thing as a count of seconds", "[e2e][CLI-SHIFT-01]") {
    const std::filesystem::path one = scratch("seconds.srt");
    const std::filesystem::path other = scratch("stamp.srt");

    CHECK(invoke({"--quiet",
                  "shift",
                  "--by",
                  "7.001",
                  "--output",
                  one.string(),
                  corpus("valides/minimal.srt")})
              .exitCode == 0);
    CHECK(invoke({"--quiet",
                  "shift",
                  "--by",
                  "00:00:07.001",
                  "--output",
                  other.string(),
                  corpus("valides/minimal.srt")})
              .exitCode == 0);

    CHECK(contentOf(one) == contentOf(other));
    std::filesystem::remove(one);
    std::filesystem::remove(other);
}

TEST_CASE("a shift before the origin is refused, naming the subtitle", "[e2e][CLI-SHIFT-02]") {
    const std::filesystem::path out = scratch("refused.srt");
    std::filesystem::remove(out);

    const CliRun run = invoke(
        {"shift", "--by", "-7.001", "--output", out.string(), corpus("valides/minimal.srt")});

    CHECK(run.exitCode == 2);
    CHECK_FALSE(std::filesystem::exists(out));
    CHECK_THAT(run.errors, ContainsSubstring("subtitle 1"));
    CHECK_THAT(run.errors, ContainsSubstring("before the origin"));
}

TEST_CASE("a comma in the amount is refused", "[e2e][CLI-USAGE-02]") {
    const CliRun run = invoke({"shift",
                               "--by",
                               "-7,001",
                               "--output-dir",
                               scratch("").parent_path().string(),
                               corpus("valides/minimal.srt")});

    CHECK(run.exitCode == 1);
    CHECK_THAT(run.errors, ContainsSubstring("decimal point"));
}

TEST_CASE("an amount finer than a millisecond is refused", "[e2e][CLI-USAGE-02]") {
    const CliRun run = invoke({"shift",
                               "--by",
                               "1.0005",
                               "--output-dir",
                               scratch("").parent_path().string(),
                               corpus("valides/minimal.srt")});

    CHECK(run.exitCode == 1);
    CHECK_THAT(run.errors, ContainsSubstring("millisecond"));
}
