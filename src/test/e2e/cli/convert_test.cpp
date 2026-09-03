// Converting between the two formats of the MVP, through the real binary.

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

TEST_CASE("converting writes the format asked for", "[e2e][CLI-CONVERT-01]") {
    const Scratch scratch;
    const CliRun run = invoke(
        {"convert", "--to", "vtt", "--output", scratch.of("a.vtt"), corpus("valides/minimal.srt")});

    CHECK(run.exitCode == 0);
    CHECK_THAT(contentOf(scratch.of("a.vtt")), ContainsSubstring("WEBVTT"));
}

TEST_CASE("a directory takes the extension of the format", "[e2e][CLI-CONVERT-01]") {
    const Scratch scratch;
    const CliRun run = invoke(
        {"convert", "--to", "vtt", "--output-dir", scratch.path(), corpus("valides/minimal.srt")});

    CHECK(run.exitCode == 0);
    CHECK(std::filesystem::exists(scratch.of("minimal.vtt")));
    CHECK_FALSE(std::filesystem::exists(scratch.of("minimal.srt")));
}

TEST_CASE("a file rewritten without a word comes back byte for byte", "[e2e][CLI-ENC-05]") {
    // **The property the phase carries**, through the real binary: a file that
    // is asked for nothing gives back the same bytes — its encoding among them.
    // Latin-1 here, which nothing declares and everything has to keep.
    const Scratch scratch;
    const std::string path = corpus("encodages/latin1.srt");
    const CliRun run =
        invoke({"convert", "--to", "srt", "--output", scratch.of("copie.srt"), path});

    CHECK(run.exitCode == 0);
    CHECK(contentOf(scratch.of("copie.srt")) == contentOf(path));
}

TEST_CASE("a file in UTF-16 with its mark comes back byte for byte", "[e2e][CLI-ENC-05]") {
    // The mark is two bytes and carries the byte order; the text is two bytes
    // per character. Nothing of either survives by accident.
    const Scratch scratch;
    const std::string path = corpus("encodages/utf-16-le-bom.srt");
    const CliRun run =
        invoke({"convert", "--to", "srt", "--output", scratch.of("copie.srt"), path});

    CHECK(run.exitCode == 0);
    CHECK(contentOf(scratch.of("copie.srt")) == contentOf(path));
}

TEST_CASE("nothing is written without a destination", "[e2e][CLI-CONVERT-03]") {
    const CliRun run = invoke({"convert", "--to", "vtt", corpus("valides/minimal.srt")});

    CHECK(run.exitCode == 1);
    CHECK(run.output.empty());
    CHECK_THAT(run.errors, ContainsSubstring("--output"));
}

TEST_CASE("two destinations at once are refused", "[e2e][CLI-CONVERT-03]") {
    const Scratch scratch;
    const CliRun run = invoke({"convert",
                               "--to",
                               "vtt",
                               "--output",
                               scratch.of("a.vtt"),
                               "--in-place",
                               corpus("valides/minimal.srt")});

    CHECK(run.exitCode == 1);
}

TEST_CASE("converting in place is refused", "[e2e][CLI-CONVERT-03]") {
    const Scratch scratch;
    const std::string copy = scratch.of("a.srt");
    std::filesystem::copy_file(corpus("valides/minimal.srt"), copy);
    const std::string before = contentOf(copy);

    const CliRun run = invoke({"convert", "--to", "vtt", "--in-place", copy});

    CHECK(run.exitCode == 1);
    // Writing WebVTT into a file named .srt would leave a file that lies about
    // itself, and in place there is no second name to tell the truth.
    CHECK(contentOf(copy) == before);
    CHECK_THAT(run.errors, ContainsSubstring("--in-place"));
}

TEST_CASE("in place is allowed when the format does not change", "[e2e][CLI-CONVERT-02]") {
    const Scratch scratch;
    const std::string copy = scratch.of("a.srt");
    std::filesystem::copy_file(corpus("valides/minimal.srt"), copy);

    const CliRun run =
        invoke({"convert", "--to", "srt", "--line-endings", "windows", "--in-place", copy});

    CHECK(run.exitCode == 0);
    CHECK_THAT(contentOf(copy), ContainsSubstring("\r\n"));
}

TEST_CASE("line endings and the byte order mark are settable", "[e2e][CLI-CONVERT-02]") {
    const Scratch scratch;
    const CliRun run = invoke({"convert",
                               "--to",
                               "srt",
                               "--line-endings",
                               "mac",
                               "--bom",
                               "--output",
                               scratch.of("a.srt"),
                               corpus("valides/minimal.srt")});

    CHECK(run.exitCode == 0);
    const std::string written = contentOf(scratch.of("a.srt"));
    CHECK(written.starts_with("\xEF\xBB\xBF"));
    CHECK(written.contains('\r'));
    CHECK_FALSE(written.contains('\n'));
}

TEST_CASE("a single output file cannot take a batch", "[e2e][CLI-CONVERT-03]") {
    const Scratch scratch;
    const CliRun run = invoke({"convert",
                               "--to",
                               "vtt",
                               "--output",
                               scratch.of("a.vtt"),
                               corpus("valides/minimal.srt"),
                               corpus("valides/minimal.vtt")});

    CHECK(run.exitCode == 1);
    CHECK_THAT(run.errors, ContainsSubstring("--output-dir"));
}

TEST_CASE("a batch converts what it can and counts the rest", "[e2e][CLI-BATCH-02]") {
    const Scratch scratch;
    const CliRun run = invoke({"convert",
                               "--to",
                               "vtt",
                               "--output-dir",
                               scratch.path(),
                               corpus("valides/minimal.srt"),
                               corpus("malformes/vide.srt")});

    CHECK(run.exitCode == 3);
    CHECK(std::filesystem::exists(scratch.of("minimal.vtt")));
}
