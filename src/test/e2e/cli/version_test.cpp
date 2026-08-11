#include <subedit/core/version.hpp>

#include <catch2/catch_test_macros.hpp>

#include <string>

#include "cli_run.hpp"

using subedit::e2e::CliRun;
using subedit::e2e::invoke;

namespace {

// Computed, never copied: a version number written by hand is a version
// number that goes stale.
std::string expectedLine() {
    return "subedit " + subedit::core::versionString() + "\n";
}

} // namespace

TEST_CASE("invoking with no argument writes the version", "[e2e][CLI-VERSION-01]") {
    const CliRun run = invoke({});

    CHECK(run.exitCode == 0);
    CHECK(run.output == expectedLine());
}

TEST_CASE("invoking with no argument writes nothing to standard error", "[e2e][CLI-VERSION-02]") {
    const CliRun run = invoke({});

    CHECK(run.errors.empty());
}

TEST_CASE("every argument is ignored and the exit code stays zero", "[e2e][CLI-VERSION-03]") {
    const CliRun run = invoke({"--help", "--version", "fichier.srt"});

    CHECK(run.exitCode == 0);
    CHECK(run.output == expectedLine());
    CHECK(run.errors.empty());
}
