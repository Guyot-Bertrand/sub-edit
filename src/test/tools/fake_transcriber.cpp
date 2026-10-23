// A transcriber that transcribes nothing, and says what it was asked to do.
//
// Phase 22 runs whisper.cpp and an extraction tool as subprocesses. What has to
// be proven is **that the right command line is built, that its output is read
// and that its failures are told** — not that whisper can decode a sentence.
// Installing the real tools would prove the second and cost a model of several
// hundred megabytes; it would also make the test pass on the machine that has
// them and fail on every other, the defect #88 ruled out for the corpus and
// #117 for the screen. `fake_player.cpp` made the same choice for the video
// player, and this program follows it.
//
// So: a program the project owns, that ends by itself, and whose whole
// behaviour is visible from the file it was told to write and the streams it
// wrote to.
//
//   - every argument it received is written to standard output, one per line,
//     in order — `argv[0]` excluded;
//   - `--progress=N` writes N progress lines to standard error, in the form
//     `whisper-cli -pp` uses, the last one at 100 %;
//   - `--linger=N` makes it stay alive N seconds before going on, which is what
//     a test needs to ask after a run that has not finished, or to cancel one;
//   - `--fail-with=N` makes it complain on standard error and exit with N,
//     **before writing any result**, as the real tool does when it fails;
//   - `--output-from=FILE` makes it write FILE as its result: to `PREFIX.json`
//     when the call carries `-of PREFIX`, as `whisper-cli` does, and to the last
//     argument that is not a knob otherwise, as an extraction tool names its
//     output;
//   - `--truncate-output` cuts that result to its first half, which is what a
//     run killed in the middle of a write leaves behind;
//   - `--unfinished-line` ends standard output with a line that has no newline;
//   - `--detach-streams` closes both standard streams and goes on, which is
//     a program that is still running and says nothing more;
//   - `--ignore-term` makes it ignore `SIGTERM`, so that only `SIGKILL` ends it;
//   - `--no-echo` keeps it from writing its arguments, and `--stdout-from=FILE`
//     makes it write FILE to standard output instead: that is `ffprobe`, whose
//     answer is the only thing it says.
//
// It decodes no audio and reads no model: whatever the call names is neither
// opened nor checked. That is deliberate — a test that needs the adapter to
// read real output uses the versioned samples of #770, and one that needs the
// real tool is run by hand.
//
// **The knobs travel through argv and not through the environment**, for the
// reason `fake_player.cpp` gives: an argument is local to one launch, an
// environment variable is global state shared by every test of the binary.

#include <algorithm>
#include <chrono>
#include <csignal>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <optional>
#include <ranges>
#include <string>
#include <string_view>
#include <system_error>
#include <thread>
#include <unistd.h>
#include <vector>

namespace {

constexpr std::string_view kFailFlag = "--fail-with=";
constexpr std::string_view kLingerFlag = "--linger=";
constexpr std::string_view kProgressFlag = "--progress=";
constexpr std::string_view kOutputFlag = "--output-from=";
constexpr std::string_view kTruncateFlag = "--truncate-output";
constexpr std::string_view kUnfinishedFlag = "--unfinished-line";
constexpr std::string_view kDetachFlag = "--detach-streams";
constexpr std::string_view kIgnoreTermFlag = "--ignore-term";
constexpr std::string_view kNoEchoFlag = "--no-echo";
constexpr std::string_view kStdoutFlag = "--stdout-from=";
constexpr std::string_view kPrefixFlag = "-of";

/// What a transcriber says when it cannot go on. The text matters little; that
/// it reaches the caller through standard error matters.
constexpr std::string_view kComplaint = "fake transcriber: refusing to transcribe";

/// What `whisper-cli -pp` writes before each percentage.
constexpr std::string_view kProgressLine = "whisper_print_progress_callback: progress = ";

using Arguments = std::vector<std::string>;

/// The value of the first argument that starts with `flag`, or nothing.
[[nodiscard]] std::optional<std::string> valueOf(const Arguments& arguments,
                                                 std::string_view flag) {
    for (const std::string& argument : arguments)
        if (argument.starts_with(flag))
            return argument.substr(flag.size());
    return std::nullopt;
}

[[nodiscard]] bool has(const Arguments& arguments, std::string_view flag) {
    return valueOf(arguments, flag).has_value();
}

/// Where the result goes: `PREFIX.json` after `-of PREFIX`, else the last
/// argument that is not one of the knobs above. Nothing when the call names no
/// place at all.
[[nodiscard]] std::optional<std::filesystem::path> resultPath(const Arguments& arguments) {
    const auto prefix = std::ranges::find(arguments, kPrefixFlag);
    if (prefix != arguments.end() && std::next(prefix) != arguments.end())
        return std::filesystem::path{*std::next(prefix) + ".json"};
    for (const std::string& argument : arguments | std::views::reverse)
        if (!argument.starts_with("--"))
            return std::filesystem::path{argument};
    return std::nullopt;
}

[[nodiscard]] int fail(int code) {
    std::cerr << kComplaint << '\n';
    std::cerr.flush();
    return code;
}

void reportProgress(int steps) {
    for (int step = 1; step <= steps; ++step) {
        std::cerr << kProgressLine << (100 * step / steps) << "%\n";
        std::cerr.flush();
    }
}

/// Writes `source` as the result, whole or cut in half. Whether it worked.
[[nodiscard]] bool writeResult(const std::filesystem::path& source,
                               const std::filesystem::path& destination,
                               bool truncate) {
    std::error_code error;
    std::filesystem::copy_file(
        source, destination, std::filesystem::copy_options::overwrite_existing, error);
    if (error)
        return false;
    if (!truncate)
        return true;
    const auto size = std::filesystem::file_size(destination, error);
    if (error)
        return false;
    std::filesystem::resize_file(destination, size / 2, error);
    return !error;
}

} // namespace

int main(int argc, char** argv) {
    const Arguments arguments(argv + (argc > 0 ? 1 : 0), argv + argc);

    if (has(arguments, kIgnoreTermFlag))
        std::signal(SIGTERM, SIG_IGN);

    if (!has(arguments, kNoEchoFlag))
        for (const std::string& argument : arguments)
            std::cout << argument << '\n';
    if (const auto source = valueOf(arguments, kStdoutFlag)) {
        const std::ifstream file{*source, std::ios::binary};
        std::cout << file.rdbuf();
    }
    std::cout.flush();

    if (const auto steps = valueOf(arguments, kProgressFlag))
        reportProgress(std::atoi(steps->c_str()));

    if (has(arguments, kUnfinishedFlag)) {
        std::cout << "unfinished";
        std::cout.flush();
    }

    if (has(arguments, kDetachFlag)) {
        ::close(STDOUT_FILENO);
        ::close(STDERR_FILENO);
    }

    if (const auto seconds = valueOf(arguments, kLingerFlag))
        std::this_thread::sleep_for(std::chrono::seconds(std::atoi(seconds->c_str())));

    if (const auto code = valueOf(arguments, kFailFlag))
        return fail(std::atoi(code->c_str()));

    if (const auto source = valueOf(arguments, kOutputFlag)) {
        const std::optional<std::filesystem::path> destination = resultPath(arguments);
        if (!destination || !writeResult(*source, *destination, has(arguments, kTruncateFlag)))
            return fail(1);
    }

    return 0;
}
