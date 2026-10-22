// The program that stands in for whisper-cli and for the extraction tool.
//
// A fake that is wrong makes every test built on it wrong in the same way, so
// each knob it offers is held here, once, and the tests of the adapter and of
// the extraction can rely on it.

#include <subedit/core/process/start_process.hpp>

#include <catch2/catch_test_macros.hpp>

#include <chrono>
#include <expected>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <optional>
#include <string>
#include <vector>

#include "await_process.hpp"

namespace {

using subedit::core::LaunchError;
using subedit::core::ProcessHandle;
using subedit::core::ProcessOutcome;
using subedit::core::runAndCapture;
using subedit::core::startProcess;
using subedit::test::RunningProcess;

const std::filesystem::path kFakeTranscriber{SUBEDIT_FAKE_TRANSCRIBER_BINARY};

/// A directory that removes itself, so that a failing assertion cannot leave
/// anything behind in the temporary directory of the machine.
class ScratchDirectory {

public:
    ScratchDirectory() {
        const auto stamp = std::chrono::steady_clock::now().time_since_epoch().count();
        m_path =
            std::filesystem::temp_directory_path() /
            ("subedit-transcriber-" + std::to_string(stamp) + "-" + std::to_string(nextSerial()));
        std::filesystem::create_directories(m_path);
    }

    ScratchDirectory(const ScratchDirectory&) = delete;
    ScratchDirectory(ScratchDirectory&&) = delete;
    ScratchDirectory& operator=(const ScratchDirectory&) = delete;
    ScratchDirectory& operator=(ScratchDirectory&&) = delete;

    ~ScratchDirectory() {
        std::error_code ignored;
        std::filesystem::remove_all(m_path, ignored);
    }

    [[nodiscard]] std::filesystem::path file(const std::string& name) const {
        return m_path / name;
    }

private:
    [[nodiscard]] static int nextSerial() {
        static int serial = 0;
        return ++serial;
    }

    std::filesystem::path m_path;
};

[[nodiscard]] std::string contentOf(const std::filesystem::path& path) {
    std::ifstream file{path, std::ios::binary};
    return {std::istreambuf_iterator<char>{file}, std::istreambuf_iterator<char>{}};
}

void write(const std::filesystem::path& path, const std::string& content) {
    std::ofstream file{path, std::ios::binary};
    file << content;
}

[[nodiscard]] std::vector<std::string> linesOf(const std::string& text) {
    std::vector<std::string> lines;
    std::size_t start = 0;
    while (start < text.size()) {
        const std::size_t end = text.find('\n', start);
        lines.push_back(text.substr(start, end - start));
        if (end == std::string::npos)
            break;
        start = end + 1;
    }
    return lines;
}

/// Runs the fake with both streams sent to `output`, and returns how it ended.
[[nodiscard]] std::optional<ProcessOutcome> runInto(const std::filesystem::path& output,
                                                    const std::vector<std::string>& arguments) {
    const std::expected<ProcessHandle, LaunchError> started =
        startProcess(kFakeTranscriber, arguments, output);
    REQUIRE(started.has_value());
    RunningProcess running{*started};
    const std::optional<ProcessOutcome> outcome = running.await();
    REQUIRE(outcome.has_value());
    return outcome;
}

constexpr const char* kResult = "{\"result\": {\"language\": \"en\"}}\n";

} // namespace

TEST_CASE("the fake transcriber writes back the arguments it received", "[process][fake]") {
    const std::vector<std::string> arguments{
        "-m", "ggml-tiny.bin", "-f", "audio.wav", "-l", "auto"};

    const auto answered = runAndCapture(kFakeTranscriber, arguments);

    REQUIRE(answered.has_value());
    CHECK(answered->code == 0);
    CHECK(linesOf(answered->output) == arguments);
}

TEST_CASE("the fake transcriber reports the progress it is asked for", "[process][fake]") {
    const ScratchDirectory scratch;
    const std::filesystem::path output = scratch.file("output");

    const auto outcome = runInto(output, {"--progress=4"});

    CHECK(outcome == ProcessOutcome{.code = 0});
    std::vector<std::string> progress;
    for (const std::string& line : linesOf(contentOf(output)))
        if (line.starts_with("whisper_print_progress_callback"))
            progress.push_back(line);
    const std::string prefix = "whisper_print_progress_callback: progress = ";
    CHECK(progress == std::vector<std::string>{
                          prefix + "25%", prefix + "50%", prefix + "75%", prefix + "100%"});
}

TEST_CASE("the progress goes to standard error and not to the answer", "[process][fake]") {
    const auto answered = runAndCapture(kFakeTranscriber, std::vector<std::string>{"--progress=2"});

    REQUIRE(answered.has_value());
    CHECK(answered->output == "--progress=2\n");
}

TEST_CASE("the fake transcriber writes its result where -of names it", "[process][fake]") {
    const ScratchDirectory scratch;
    const std::filesystem::path sample = scratch.file("sample.json");
    write(sample, kResult);
    const std::filesystem::path prefix = scratch.file("out");

    const auto outcome = runInto(scratch.file("output"),
                                 {"--output-from=" + sample.string(), "-of", prefix.string()});

    CHECK(outcome == ProcessOutcome{.code = 0});
    CHECK(contentOf(scratch.file("out.json")) == kResult);
}

TEST_CASE("without -of the result goes to the last argument that is not a knob",
          "[process][fake]") {
    const ScratchDirectory scratch;
    const std::filesystem::path sample = scratch.file("sample.wav");
    write(sample, "audio");
    const std::filesystem::path destination = scratch.file("extracted.wav");

    const auto outcome = runInto(scratch.file("output"),
                                 {"-i",
                                  "film.mkv",
                                  "--output-from=" + sample.string(),
                                  destination.string(),
                                  "--progress=1"});

    CHECK(outcome == ProcessOutcome{.code = 0});
    CHECK(contentOf(destination) == "audio");
}

TEST_CASE("a truncated result is cut to its first half", "[process][fake]") {
    const ScratchDirectory scratch;
    const std::filesystem::path sample = scratch.file("sample.json");
    write(sample, "0123456789");
    const std::filesystem::path prefix = scratch.file("out");

    const auto outcome =
        runInto(scratch.file("output"),
                {"--output-from=" + sample.string(), "--truncate-output", "-of", prefix.string()});

    CHECK(outcome == ProcessOutcome{.code = 0});
    CHECK(contentOf(scratch.file("out.json")) == "01234");
}

TEST_CASE("a result that cannot be read is a failure, not an empty file", "[process][fake]") {
    const ScratchDirectory scratch;
    const std::filesystem::path prefix = scratch.file("out");

    const auto outcome =
        runInto(scratch.file("output"),
                {"--output-from=" + scratch.file("absent").string(), "-of", prefix.string()});

    CHECK(outcome == ProcessOutcome{.code = 1});
    CHECK_FALSE(std::filesystem::exists(scratch.file("out.json")));
}

TEST_CASE("a failure leaves no result and says why on standard error", "[process][fake]") {
    const ScratchDirectory scratch;
    const std::filesystem::path sample = scratch.file("sample.json");
    write(sample, kResult);
    const std::filesystem::path prefix = scratch.file("out");
    const std::filesystem::path output = scratch.file("output");

    const auto outcome = runInto(
        output, {"--output-from=" + sample.string(), "-of", prefix.string(), "--fail-with=3"});

    CHECK(outcome == ProcessOutcome{.code = 3});
    CHECK_FALSE(std::filesystem::exists(scratch.file("out.json")));
    CHECK(contentOf(output).find("refusing to transcribe") != std::string::npos);
}

TEST_CASE("a lingering transcriber is still running, and writes nothing, until it ends",
          "[process][fake]") {
    const ScratchDirectory scratch;
    const std::filesystem::path sample = scratch.file("sample.json");
    write(sample, kResult);
    const std::filesystem::path prefix = scratch.file("out");

    const std::vector<std::string> arguments{
        "--linger=30", "--output-from=" + sample.string(), "-of", prefix.string()};
    const std::expected<ProcessHandle, LaunchError> started =
        startProcess(kFakeTranscriber, arguments, scratch.file("output"));

    REQUIRE(started.has_value());
    // The destructor ends the process, which is what a cancellation does: no
    // result is left behind, as with the real tool.
    RunningProcess running{*started};
    CHECK_FALSE(running.await(std::chrono::milliseconds{300}).has_value());
    CHECK_FALSE(std::filesystem::exists(scratch.file("out.json")));
}
