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
using subedit::e2e::invoke;

namespace {

std::string corpus(const std::string& relative) {
    return (std::filesystem::path{SUBEDIT_TEST_DATA_DIR} / relative).string();
}

/// A directory of its own per test, removed with everything in it.
class Scratch {

public:
    Scratch() {
        m_path =
            std::filesystem::temp_directory_path() /
            ("subedit-e2e-" +
             std::to_string(std::filesystem::hash_value(std::filesystem::temp_directory_path())) +
             "-" + std::to_string(counter()));
        std::filesystem::remove_all(m_path);
        std::filesystem::create_directories(m_path);
    }

    Scratch(const Scratch&) = delete;
    Scratch& operator=(const Scratch&) = delete;
    Scratch(Scratch&&) = delete;
    Scratch& operator=(Scratch&&) = delete;

    ~Scratch() { std::filesystem::remove_all(m_path); }

    [[nodiscard]] std::string of(const std::string& name) const { return (m_path / name).string(); }

    [[nodiscard]] std::string path() const { return m_path.string(); }

private:
    static int counter() {
        static int next = 0;
        return ++next;
    }

    std::filesystem::path m_path;
};

std::string contentOf(const std::string& path) {
    const std::ifstream file{path, std::ios::binary};
    std::ostringstream all;
    all << file.rdbuf();
    return all.str();
}

} // namespace

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
