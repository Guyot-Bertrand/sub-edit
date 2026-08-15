// Several files in one invocation, and what the exit code says about them.

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include <filesystem>
#include <string>

#include "cli_run.hpp"

using Catch::Matchers::ContainsSubstring;
using subedit::e2e::CliRun;
using subedit::e2e::corpus;
using subedit::e2e::invoke;

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
