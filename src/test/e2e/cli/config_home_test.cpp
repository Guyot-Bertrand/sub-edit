// What the harness gives a binary instead of the developer's settings.
//
// **The end-to-end tests are the one place a real configuration location is
// reachable.** Everywhere else the seam of ADR 0022 closes the question by
// construction: the settings receive a path, so a test gives one, and nothing
// resolves a standard location behind a test's back. Here a whole process is
// started, and that process resolves its own — it has to, it is the program a
// user runs.
//
// So the redirection happens where the process is started, and these cases hold
// it to it: the home handed over is the harness's, and what would have been
// handed over without it is the user's.

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <string>
#include <vector>

#include "cli_run.hpp"

namespace {

/// The configuration home this test process itself has — the one a child would
/// inherit if nothing were substituted.
[[nodiscard]] std::string inheritedConfigHome() {
    if (const char* named = std::getenv("XDG_CONFIG_HOME"); named != nullptr && *named != '\0')
        return named;

    const char* home = std::getenv("HOME");
    return home == nullptr ? std::string{} : std::string{home} + "/.config";
}

/// How many entries of `environment` set the configuration home.
[[nodiscard]] std::ptrdiff_t timesConfigHomeIsSet(const std::vector<std::string>& environment) {
    return std::ranges::count_if(environment, [](const std::string& variable) {
        return variable.starts_with("XDG_CONFIG_HOME=");
    });
}

} // namespace

TEST_CASE("the harness has a configuration home of its own", "[e2e][harness]") {
    const std::string home = subedit::e2e::configHome();

    REQUIRE_FALSE(home.empty());
    CHECK(std::filesystem::is_directory(home));
    CHECK(std::filesystem::path{home}.is_absolute());
}

TEST_CASE("a launched binary is given that home, and it is set once", "[e2e][harness]") {
    const std::vector<std::string> environment = subedit::e2e::childEnvironment();

    CHECK(timesConfigHomeIsSet(environment) == 1);
    CHECK(std::ranges::count(environment, "XDG_CONFIG_HOME=" + subedit::e2e::configHome()) == 1);
}

// The demonstration asked for by #238. Nothing is written and nothing is
// launched: what is shown is that the value the harness replaces is the real
// one, so that removing the substitution would point a running binary straight
// at the settings of whoever ran the tests.
TEST_CASE("without the harness, a launched binary would inherit the user's own", "[e2e][harness]") {
    REQUIRE_FALSE(inheritedConfigHome().empty());

    CHECK(inheritedConfigHome() != subedit::e2e::configHome());
    CHECK(std::ranges::count(subedit::e2e::childEnvironment(),
                             "XDG_CONFIG_HOME=" + inheritedConfigHome()) == 0);
}

// The runner is one for both binaries, and so is the substitution: a
// configuration home given to the command line and forgotten for the window
// would leave the half of the program that actually keeps preferences
// unprotected.
TEST_CASE("both binaries run under the harness, and say nothing about it", "[e2e][harness]") {
    CHECK(subedit::e2e::invoke({"--version"}).exitCode == 0);
    CHECK(subedit::e2e::invokeGui({"--version"}).exitCode == 0);

    // Nothing was put there: the binaries do not keep preferences yet, and the
    // day they do this is the case that will say where those preferences went.
    CHECK(std::filesystem::is_empty(subedit::e2e::configHome()));
}
