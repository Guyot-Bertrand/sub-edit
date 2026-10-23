// Running a program and reading what it writes while it runs.

#include <subedit/core/process/start_process.hpp>

#include <catch2/catch_test_macros.hpp>

#include <chrono>
#include <expected>
#include <filesystem>
#include <string>
#include <sys/resource.h>
#include <unistd.h>
#include <utility>
#include <vector>

namespace {

using subedit::core::LaunchErrorKind;
using subedit::core::runStreaming;
using subedit::core::StreamedLine;
using subedit::core::StreamName;
using subedit::core::StreamObserver;

const std::filesystem::path kFakeProgram{SUBEDIT_FAKE_TRANSCRIBER_BINARY};

// The cases that use up the descriptors cannot run under the sanitizers: their
// runtime opens files and forks to report, and with no descriptor left it
// reports a corrupted object instead of the refusal under test. The coverage
// build, which has no sanitizer, runs them — and it is the one that counts the
// lines they exist for.
#if defined(__SANITIZE_ADDRESS__)
constexpr bool kUnderSanitizer = true;
#elif defined(__has_feature)
#    if __has_feature(address_sanitizer)
constexpr bool kUnderSanitizer = true;
#    else
constexpr bool kUnderSanitizer = false;
#    endif
#else
constexpr bool kUnderSanitizer = false;
#endif

constexpr std::string_view kProgress = "whisper_print_progress_callback: progress = ";

/// Runs the fake with `arguments`, collecting every line it delivers.
struct Collected {
    std::vector<StreamedLine> lines;
    std::expected<subedit::core::StreamedRun, subedit::core::LaunchError> run;
};

StreamedLine lineOf(StreamName stream, std::string text) {
    return StreamedLine{.stream = stream, .text = std::move(text)};
}

Collected collect(const std::vector<std::string>& arguments, StreamObserver observer = {}) {
    Collected collected{.lines = {}, .run = subedit::core::StreamedRun{}};
    observer.onLine = [&collected](const StreamedLine& line) { collected.lines.push_back(line); };
    collected.run = runStreaming(kFakeProgram, arguments, observer);
    return collected;
}

/// Lets a test have room for only `spare` more descriptors, so that the pipes of
/// a launch can be made to fail on purpose. Restored when it goes out of scope.
class DescriptorLimit {

public:
    explicit DescriptorLimit(rlim_t spare) {
        getrlimit(RLIMIT_NOFILE, &m_before);
        // The lowest descriptor that is free is the count of those in use.
        const int probe = ::dup(STDIN_FILENO);
        ::close(probe);
        rlimit limited = m_before;
        limited.rlim_cur = static_cast<rlim_t>(probe) + spare;
        setrlimit(RLIMIT_NOFILE, &limited);
    }

    DescriptorLimit(const DescriptorLimit&) = delete;
    DescriptorLimit(DescriptorLimit&&) = delete;
    DescriptorLimit& operator=(const DescriptorLimit&) = delete;
    DescriptorLimit& operator=(DescriptorLimit&&) = delete;

    ~DescriptorLimit() { setrlimit(RLIMIT_NOFILE, &m_before); }

private:
    rlimit m_before{};
};

} // namespace

TEST_CASE("each line a program writes comes back with the stream it was written on",
          "[process][streaming]") {
    const auto collected = collect({"--progress=2", "-m", "model.bin"});

    REQUIRE(collected.run.has_value());
    CHECK(collected.run->code == 0);
    CHECK_FALSE(collected.run->cancelled);

    const std::vector<StreamedLine> expected{
        lineOf(StreamName::Output, "--progress=2"),
        lineOf(StreamName::Output, "-m"),
        lineOf(StreamName::Output, "model.bin"),
        lineOf(StreamName::Error, std::string{kProgress} + "50%"),
        lineOf(StreamName::Error, std::string{kProgress} + "100%"),
    };
    // The two streams are read as they come, so only the order within each is fixed.
    std::vector<StreamedLine> outputs;
    std::vector<StreamedLine> errors;
    for (const StreamedLine& line : collected.lines)
        (line.stream == StreamName::Output ? outputs : errors).push_back(line);
    CHECK(outputs == std::vector<StreamedLine>(expected.begin(), expected.begin() + 3));
    CHECK(errors == std::vector<StreamedLine>(expected.begin() + 3, expected.end()));
}

TEST_CASE("the exit code of a program that failed comes back with its complaint",
          "[process][streaming]") {
    const auto collected = collect({"--fail-with=3"});

    REQUIRE(collected.run.has_value());
    CHECK(collected.run->code == 3);
    CHECK_FALSE(collected.run->cancelled);
    REQUIRE_FALSE(collected.lines.empty());
    CHECK(collected.lines.back().stream == StreamName::Error);
    CHECK(collected.lines.back().text.find("refusing to transcribe") != std::string::npos);
}

TEST_CASE("a last line with no newline is delivered when the stream closes",
          "[process][streaming]") {
    const auto collected = collect({"--unfinished-line"});

    REQUIRE(collected.run.has_value());
    REQUIRE(collected.lines.size() == 2);
    CHECK(collected.lines.back() == lineOf(StreamName::Output, "unfinished"));
}

TEST_CASE("lines arrive while the program is still running", "[process][streaming]") {
    // The program lingers for thirty seconds after its progress. Stopping on the
    // last progress line proves it was delivered long before the program ended.
    int progressLines = 0;
    StreamObserver observer;
    observer.cancelled = [&progressLines] { return progressLines >= 2; };
    std::vector<StreamedLine> seen;
    observer.onLine = [&](const StreamedLine& line) {
        seen.push_back(line);
        if (line.stream == StreamName::Error)
            ++progressLines;
    };

    const std::vector<std::string> arguments{"--progress=2", "--linger=30"};
    const auto started = std::chrono::steady_clock::now();
    const auto run = runStreaming(kFakeProgram, arguments, observer);

    REQUIRE(run.has_value());
    CHECK(run->cancelled);
    CHECK(run->code == 128 + 15);
    CHECK(std::chrono::steady_clock::now() - started < std::chrono::seconds(20));
    CHECK(progressLines == 2);
}

TEST_CASE("a program that has closed its streams can still be stopped", "[process][streaming]") {
    int polls = 0;
    StreamObserver observer;
    observer.cancelled = [&polls] { return ++polls > 3; };

    const std::vector<std::string> arguments{"--detach-streams", "--linger=30"};
    const auto run = runStreaming(kFakeProgram, arguments, observer);

    REQUIRE(run.has_value());
    CHECK(run->cancelled);
    CHECK(run->code == 128 + 15);
}

TEST_CASE("a program that ignores the request to stop is killed", "[process][streaming]") {
    int polls = 0;
    StreamObserver observer;
    observer.cancelled = [&polls] { return ++polls > 2; };
    observer.grace = std::chrono::milliseconds{200};

    const std::vector<std::string> arguments{"--ignore-term", "--linger=30"};
    const auto run = runStreaming(kFakeProgram, arguments, observer);

    REQUIRE(run.has_value());
    CHECK(run->cancelled);
    CHECK(run->code == 128 + 9);
}

TEST_CASE("a program that is not there is refused, not run", "[process][streaming]") {
    const std::vector<std::string> none;
    const auto run = runStreaming("/nulle/part/whisper-cli", none, {});

    REQUIRE_FALSE(run.has_value());
    CHECK(run.error().kind == LaunchErrorKind::NotFound);
}

// A pipe that cannot be made is the only way a launch fails before the program
// is started; the descriptor limit is how a test reaches it.
TEST_CASE("a launch that cannot make its first pipe is refused", "[process][streaming]") {
    if (kUnderSanitizer)
        SKIP("the sanitizers need a descriptor to report");
    const DescriptorLimit limit{1};

    const std::vector<std::string> none;
    const auto run = runStreaming(kFakeProgram, none, {});

    REQUIRE_FALSE(run.has_value());
    CHECK(run.error().kind == LaunchErrorKind::Failed);
}

TEST_CASE("a launch that cannot make its second pipe is refused", "[process][streaming]") {
    if (kUnderSanitizer)
        SKIP("the sanitizers need a descriptor to report");
    const DescriptorLimit limit{2};

    const std::vector<std::string> none;
    const auto run = runStreaming(kFakeProgram, none, {});

    REQUIRE_FALSE(run.has_value());
    CHECK(run.error().kind == LaunchErrorKind::Failed);
}

TEST_CASE("a launch that cannot make its pipe is refused, whichever way it runs",
          "[process][streaming]") {
    if (kUnderSanitizer)
        SKIP("the sanitizers need a descriptor to report");
    const DescriptorLimit limit{1};

    const std::vector<std::string> none;
    const auto answered = subedit::core::runAndCapture(kFakeProgram, none);

    REQUIRE_FALSE(answered.has_value());
    CHECK(answered.error().kind == LaunchErrorKind::Failed);
}
