// What the tool does with its own command line, before any file is touched.

#include <subedit/core/version.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include <filesystem>
#include <string>

#include "cli_run.hpp"

using Catch::Matchers::ContainsSubstring;
using subedit::e2e::CliRun;
using subedit::e2e::corpus;
using subedit::e2e::invoke;

TEST_CASE("invoking with no argument writes the help", "[e2e][CLI-USAGE-01]") {
    const CliRun run = invoke({});

    CHECK(run.exitCode == 0);
    // The wording is the parser's, and asserting it whole would break on every
    // reformatting. What matters is that the reader is told how to call the
    // tool. Which subcommands it lists is asserted where they are implemented:
    // a help that announced a subcommand before it worked would be the tool
    // lying about itself.
    CHECK_THAT(run.output, ContainsSubstring("Usage"));
}

TEST_CASE("asking for the version writes it", "[e2e][CLI-VERSION-04]") {
    const CliRun run = invoke({"--version"});

    CHECK(run.exitCode == 0);
    // Computed, never copied: a version number written by hand goes stale.
    CHECK(run.output == "subedit " + subedit::core::versionString() + "\n");
}

TEST_CASE("a successful invocation stays silent on standard error", "[e2e][CLI-VERSION-05]") {
    CHECK(invoke({"--version"}).errors.empty());
    CHECK(invoke({}).errors.empty());
}

TEST_CASE("an unknown option is refused", "[e2e][CLI-USAGE-02]") {
    const CliRun run = invoke({"--inexistant"});

    CHECK(run.exitCode == 1);
    // Nothing on standard output: a caller piping the result must not receive
    // a complaint where it expected data.
    CHECK(run.output.empty());
    CHECK_FALSE(run.errors.empty());
}

TEST_CASE("an unknown subcommand is refused", "[e2e][CLI-USAGE-02]") {
    const CliRun run = invoke({"inspecte"});

    CHECK(run.exitCode == 1);
    CHECK(run.output.empty());
    CHECK_FALSE(run.errors.empty());
}

TEST_CASE("asking for silence and for detail at once is refused", "[e2e][CLI-USAGE-04]") {
    // Otherwise valid on purpose: with no file to work on, the parser would
    // complain about that first, and the test would prove nothing about the
    // conflict it is named after.
    const CliRun run = invoke({"--quiet", "-v", "inspect", corpus("valides/minimal.srt")});

    CHECK(run.exitCode == 1);
    CHECK(run.output.empty());
    // Two opposite intentions are not arbitrated in favour of the last one
    // written: saying so is what tells the caller they were misunderstood.
    CHECK_THAT(run.errors, ContainsSubstring("--quiet"));
}
