#pragma once

#include <chrono>
#include <expected>
#include <filesystem>
#include <functional>
#include <optional>
#include <span>
#include <string>

namespace subedit::core {

/// A process that was started and has not been collected yet.
struct ProcessHandle {
    /// What the system calls it. A negative value names no process.
    int id = -1;

    friend bool operator==(const ProcessHandle&, const ProcessHandle&) = default;
};

/// How a process ended.
struct ProcessOutcome {
    /// Its exit code, or 128 plus the signal that killed it — the convention
    /// of the shells, so that a caller reading the number reads what it would
    /// have read from one.
    int code = 0;

    friend bool operator==(const ProcessOutcome&, const ProcessOutcome&) = default;
};

/// Why a process did not start.
enum class LaunchErrorKind {
    NotFound,         ///< nothing at that path
    PermissionDenied, ///< there, but not ours to run
    Failed,           ///< anything the system refused for another reason
};

struct LaunchError {
    LaunchErrorKind kind;

    /// Free context. May be empty.
    std::string detail;

    friend bool operator==(const LaunchError&, const LaunchError&) = default;
};

/// Starts `program` with `arguments`, and returns at once.
///
/// **The opposite of the end-to-end runner**, which starts a binary, waits for
/// it, and reads what it wrote. A video player outlives the call that started
/// it: it runs until somebody closes the window, its output is read later, and
/// its exit code arrives whenever it arrives.
///
/// Free functions and not a class. There is nothing to hold between two
/// launches — a launcher object would be a namespace with a constructor. Nor
/// an interface: the project introduces an abstraction where the variation is
/// real and named, and there is only one way to start a process here. What a
/// test needs is not a second implementation but **a program it controls**,
/// which is why the path is a parameter. `src/test/tools/fake_player.cpp` is
/// that program, and it makes a better test than a fake launcher would, since
/// it exercises the launch itself.
///
/// `arguments` are what follows the program name; the name itself lands in
/// `argv[0]`, as the convention requires. Standard output and standard error
/// both go to `output`, truncated — one stream, as Gaupol does it, because
/// what one wants of a player that failed is what it said, not which of the
/// two channels it said it on.
///
/// Standard input comes from `/dev/null`. Without it a player started from a
/// terminal reads the keyboard behind the back of the program that started it
/// — the reason Gaupol appends `< /dev/null` to its MPlayer command line.
[[nodiscard]] std::expected<ProcessHandle, LaunchError>
startProcess(const std::filesystem::path& program,
             std::span<const std::string> arguments,
             const std::filesystem::path& output);

/// What a program wrote, and how it ended.
struct ProgramOutput {
    /// Its exit code, read as `ProcessOutcome::code` is.
    int code = 0;

    /// Everything it wrote on standard output, to the last byte.
    std::string output;

    friend bool operator==(const ProgramOutput&, const ProgramOutput&) = default;
};

/// Runs `program` with `arguments`, **waits for it**, and returns what it said.
///
/// The sibling of `startProcess`, and its opposite in every way that matters.
/// A video player outlives the call that started it, so that one returns at
/// once and leaves the answer for later. A program one asks a question of —
/// `ffprobe`, and what a container declares — answers in milliseconds and is
/// then done: waiting for it is the whole point, and there is nothing an
/// interface could usefully do in between.
///
/// Which is why this reads through a pipe rather than through a file. There is
/// no temporary to name, to place on a writable device, to remove afterwards,
/// and no two runs of the program to keep from writing over one another.
///
/// **Standard error is discarded**, and that is a decision rather than an
/// oversight: what comes back is parsed, and a complaint mixed into the answer
/// would be read as part of it. A caller that needs to know why a program
/// failed has its exit code.
///
/// Standard input comes from `/dev/null`, for the reason `startProcess` gives.
[[nodiscard]] std::expected<ProgramOutput, LaunchError>
runAndCapture(const std::filesystem::path& program, std::span<const std::string> arguments);

/// Which of the two streams of a program a line came from.
enum class StreamName {
    Output, ///< standard output
    Error,  ///< standard error
};

/// One line a program wrote, without its newline.
struct StreamedLine {
    StreamName stream;
    std::string text;

    friend bool operator==(const StreamedLine&, const StreamedLine&) = default;
};

/// How a streamed run is watched and stopped. Both are called from the thread
/// that runs the program.
/// How long a program asked to stop is given before it is killed.
inline constexpr std::chrono::milliseconds kDefaultStopGrace{2000};

struct StreamObserver {
    /// Called for each line, **as soon as the program has written it**, in the
    /// order they arrive. A last line with no newline is delivered when the
    /// program closes the stream.
    std::function<void(const StreamedLine&)> onLine{};

    /// Polled about every fifty milliseconds. When it answers true the program
    /// is asked to end (`SIGTERM`), then killed (`SIGKILL`) if it has not ended
    /// within `grace`.
    std::function<bool()> cancelled{};

    std::chrono::milliseconds grace = kDefaultStopGrace;
};

/// How a streamed run ended.
struct StreamedRun {
    /// Read as `ProcessOutcome::code` is: 128 plus the signal for a program
    /// that was stopped.
    int code = 0;

    /// Whether the observer asked to stop. A program that ended by itself at the
    /// same moment is still reported as cancelled: the caller asked.
    bool cancelled = false;
};

/// Runs `program` with `arguments`, **delivers what it writes line by line while
/// it runs**, and can stop it.
///
/// The third way to run a program, for the one the two others do not fit:
/// `startProcess` returns at once and sends the output to a file, so nothing is
/// seen while the program runs; `runAndCapture` waits and **discards standard
/// error**, which is where a transcription engine prints its progress. This one
/// waits too, but reads both streams as they come, and a caller that wants to
/// stop gets the program ended and collected, with no zombie left behind.
///
/// Standard input comes from `/dev/null`, for the reason `startProcess` gives.
[[nodiscard]] std::expected<StreamedRun, LaunchError>
runStreaming(const std::filesystem::path& program,
             std::span<const std::string> arguments,
             const StreamObserver& observer);

/// How the process ended, or nothing while it is still running.
///
/// Never blocks. **Collects the process when it answers**, so the answer comes
/// once and once only; asking again afterwards gives nothing.
///
/// A caller that never asks leaves a zombie behind. That is the price of not
/// blocking, and it is the caller's to pay — the same one Gaupol pays with a
/// timer that polls every second.
[[nodiscard]] std::optional<ProcessOutcome> outcomeOf(ProcessHandle handle);

} // namespace subedit::core
