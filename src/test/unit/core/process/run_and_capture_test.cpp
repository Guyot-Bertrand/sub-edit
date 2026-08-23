// Running a program one asks a question of, and reading its answer.

#include <subedit/core/process/start_process.hpp>

#include <catch2/catch_test_macros.hpp>

#include <expected>
#include <filesystem>
#include <string>
#include <vector>

namespace {

using subedit::core::LaunchErrorKind;
using subedit::core::runAndCapture;

/// The program the tests ask: it writes back what it was given, one argument
/// per line, and can be told to fail.
const std::filesystem::path kFakeProgram{SUBEDIT_FAKE_PLAYER_BINARY};

} // namespace

TEST_CASE("a program's answer comes back whole", "[process]") {
    const std::vector<std::string> arguments{"-show_entries", "stream=r_frame_rate"};

    const auto answered = runAndCapture(kFakeProgram, arguments);

    REQUIRE(answered.has_value());
    CHECK(answered->code == 0);
    CHECK(answered->output == "-show_entries\nstream=r_frame_rate\n");
}

TEST_CASE("a program asked nothing answers nothing", "[process]") {
    const auto answered = runAndCapture(kFakeProgram, {});

    REQUIRE(answered.has_value());
    CHECK(answered->code == 0);
    CHECK(answered->output.empty());
}

// The call waits, so an answer is an answer and not « ask again later ». What
// proves it is that the output is complete by the time the call returns.
TEST_CASE("the exit code of a program that failed comes back with it", "[process]") {
    const std::vector<std::string> arguments{"--fail-with=3"};

    const auto answered = runAndCapture(kFakeProgram, arguments);

    REQUIRE(answered.has_value());
    CHECK(answered->code == 3);
    CHECK(answered->output == "--fail-with=3\n");
}

// Standard error is dropped on purpose: what comes back here is parsed, and a
// complaint mixed into the answer would be read as part of it. The fake
// program complains on standard error whenever it is told to fail, so the case
// above already carries the proof — this one names it.
TEST_CASE("what a program says on standard error stays out of the answer", "[process]") {
    const std::vector<std::string> arguments{"--fail-with=1"};

    const auto answered = runAndCapture(kFakeProgram, arguments);

    REQUIRE(answered.has_value());
    CHECK(answered->output.find("refusing") == std::string::npos);
}

TEST_CASE("a program that is not there is refused, not run", "[process]") {
    const auto answered = runAndCapture("/nulle/part/ffprobe", {});

    REQUIRE_FALSE(answered.has_value());
    CHECK(answered.error().kind == LaunchErrorKind::NotFound);
}
