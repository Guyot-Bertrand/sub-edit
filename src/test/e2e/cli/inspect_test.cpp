// What `inspect` reports about a file, without touching it.

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include <filesystem>
#include <string>

#include "cli_run.hpp"

using Catch::Matchers::ContainsSubstring;
using subedit::e2e::CliRun;
using subedit::e2e::invoke;

namespace {

// Resolved by CMake, like the corpus of the unit tests: a test must not depend
// on the directory it was launched from.
std::string corpus(const std::string& relative) {
    return (std::filesystem::path{SUBEDIT_TEST_DATA_DIR} / relative).string();
}

} // namespace

TEST_CASE("the help lists the subcommand", "[e2e][CLI-USAGE-01]") {
    const std::string help = invoke({}).output;

    // The listing, not the prose: the description sentence also contains the
    // word, and asserting on it would pass before the subcommand existed.
    CHECK_THAT(help, ContainsSubstring("Subcommands:"));
    CHECK_THAT(help, ContainsSubstring("\n  inspect"));
}

TEST_CASE("inspect reports what a file is made of", "[e2e][CLI-INSPECT-01]") {
    const CliRun run = invoke({"inspect", corpus("valides/minimal.srt")});

    CHECK(run.exitCode == 0);
    CHECK_THAT(run.output, ContainsSubstring("  format: SubRip\n"));
    CHECK_THAT(run.output, ContainsSubstring("  encoding: UTF-8\n"));
    CHECK_THAT(run.output, ContainsSubstring("  byte order mark: absent\n"));
    CHECK_THAT(run.output, ContainsSubstring("  line endings: LF\n"));
    CHECK_THAT(run.output, ContainsSubstring("  subtitles: 2\n"));
    CHECK_THAT(run.output, ContainsSubstring("  span: 00:00:01.000 -> 00:00:06.200\n"));
}

TEST_CASE("inspect names the file it is reporting on", "[e2e][CLI-INSPECT-01]") {
    const std::string path = corpus("valides/minimal.srt");

    CHECK_THAT(invoke({"inspect", path}).output, ContainsSubstring(path + "\n"));
}

TEST_CASE("inspect sees a byte order mark and Windows line endings", "[e2e][CLI-INSPECT-01]") {
    const CliRun run = invoke({"inspect", corpus("valides/crlf-bom.srt")});

    CHECK(run.exitCode == 0);
    CHECK_THAT(run.output, ContainsSubstring("  byte order mark: present\n"));
    CHECK_THAT(run.output, ContainsSubstring("  line endings: CRLF\n"));
}

TEST_CASE("inspect recognises WebVTT", "[e2e][CLI-INSPECT-01]") {
    CHECK_THAT(invoke({"inspect", corpus("valides/minimal.vtt")}).output,
               ContainsSubstring("  format: WebVTT\n"));
}

TEST_CASE("inspect says a file is in order", "[e2e][CLI-INSPECT-01]") {
    CHECK_THAT(invoke({"inspect", corpus("valides/minimal.srt")}).output,
               ContainsSubstring("  order: in order\n"));
}

TEST_CASE("inspect names the line that breaks the order", "[e2e][CLI-INSPECT-01]") {
    const CliRun run = invoke({"inspect", corpus("malformes/desordre.srt")});

    CHECK(run.exitCode == 0);
    // The second subtitle starts before the first one does. Counted from one,
    // as it is shown.
    CHECK_THAT(run.output, ContainsSubstring("  order: line 2 breaks the order\n"));
}

TEST_CASE("inspect signals mixed line endings with their line", "[e2e][CLI-INSPECT-02]") {
    const CliRun run = invoke({"inspect", corpus("malformes/fins-de-ligne-melangees.srt")});

    CHECK(run.exitCode == 0);
    CHECK_THAT(run.output, ContainsSubstring("  line endings: CRLF, mixed from line 5\n"));
}

TEST_CASE("a file that cannot be read is named, and nothing is reported on it",
          "[e2e][CLI-BATCH-01]") {
    const std::string path = corpus("malformes/vide.srt");
    const CliRun run = invoke({"inspect", path});

    // An empty file matches no format, and ADR 0008 makes that a failure
    // rather than an empty result: losing a file without noticing is the
    // outcome that rule exists to prevent.
    CHECK(run.output.empty());
    CHECK_THAT(run.errors, ContainsSubstring(path));
}

TEST_CASE("inspect writes its report even when asked for silence", "[e2e][CLI-OUTPUT-02]") {
    const CliRun run = invoke({"--quiet", "inspect", corpus("valides/minimal.srt")});

    CHECK(run.exitCode == 0);
    // The report is the result, not narration: silencing it would leave the
    // subcommand with nothing to say.
    CHECK_THAT(run.output, ContainsSubstring("  format: SubRip\n"));
    CHECK(run.errors.empty());
}
