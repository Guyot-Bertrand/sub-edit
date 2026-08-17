// The chain under the window, through the real binary.
//
// It checks nothing of what the interface does — nothing exists yet. It checks
// that the interface **can run at all where there is no screen**: Qt is found,
// a `QApplication` constructs, and the platform plugin loads.
//
// That last point is the only reason this test exists. A missing plugin fails
// at construction and never at compilation, so no build can warn about it —
// and it would fail on the machine that has no display, which is exactly the
// one nobody watches. `QT_QPA_PLATFORM=offscreen` is set by CTest, and this is
// what proves the setting reaches the binary.

#include <subedit/core/version.hpp>

#include <catch2/catch_test_macros.hpp>

#include <string>

#include "../cli/cli_run.hpp"

TEST_CASE("the window binary runs where there is no screen", "[e2e][gui][GUI-VERSION-01]") {
    const subedit::e2e::CliRun run = subedit::e2e::invokeGui({"--version"});

    CHECK(run.exitCode == 0);
    CHECK(run.output == "subedit " + subedit::core::versionString() + "\n");
}
