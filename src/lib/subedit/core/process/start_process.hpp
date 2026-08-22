#pragma once

#include <expected>
#include <filesystem>
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
