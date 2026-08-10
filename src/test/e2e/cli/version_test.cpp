#include <subedit/core/version.hpp>

#include <catch2/catch_test_macros.hpp>

#include <string>

#include "cli_run.hpp"

using subedit::e2e::CliRun;
using subedit::e2e::invoke;

TEST_CASE("invoking with no argument writes the version", "[e2e]") {
    const CliRun run = invoke({});

    CHECK(run.exitCode == 0);
    CHECK(run.output == "subedit " + subedit::core::versionString() + "\n");
    CHECK(run.errors.empty());
}
