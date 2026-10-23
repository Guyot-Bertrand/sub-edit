#include <subedit/core/i18n/messages.hpp>
#include <subedit/core/process/start_process.hpp>

#include <array>
#include <cerrno>
#include <chrono>
#include <csignal>
#include <cstddef>
#include <expected>
#include <fcntl.h>
#include <filesystem>
#include <functional>
#include <optional>
#include <poll.h>
#include <span>
#include <spawn.h>
#include <string>
#include <string_view>
#include <sys/wait.h>
#include <utility>
#include <vector>

// `environ` is declared by unistd.h on this platform, with _GNU_SOURCE or
// _DEFAULT_SOURCE; a second extern declaration here would be redundant.
#include <unistd.h>

namespace subedit::core {

namespace {

/// The permissions a freshly created output file gets, before the umask.
constexpr int kOutputPermissions = 0644;

/// What the shells add to a signal number to make an exit code of it.
constexpr int kSignalledExitBase = 128;

/// `posix_spawn_file_actions_t` is a resource: without the matching destroy,
/// every launch leaks whatever the actions allocated.
class SpawnActions {

public:
    // `m_actions` is declared before `m_valid`, so it is zeroed by its own
    // default initializer before this line reads its address.
    SpawnActions() : m_valid(posix_spawn_file_actions_init(&m_actions) == 0) {}

    SpawnActions(const SpawnActions&) = delete;
    SpawnActions(SpawnActions&&) = delete;
    SpawnActions& operator=(const SpawnActions&) = delete;
    SpawnActions& operator=(SpawnActions&&) = delete;

    ~SpawnActions() {
        if (m_valid)
            posix_spawn_file_actions_destroy(&m_actions);
    }

    [[nodiscard]] bool valid() const { return m_valid; }

    /// Records an error the first time one happens, and keeps the first.
    void addOpen(int descriptor, const char* path, int flags, int permissions) {
        record(posix_spawn_file_actions_addopen(
            &m_actions, descriptor, path, flags, static_cast<mode_t>(permissions)));
    }

    void addDuplicate(int from, int to) {
        record(posix_spawn_file_actions_adddup2(&m_actions, from, to));
    }

    /// Closes a descriptor in the child, and only there.
    ///
    /// What keeps the reading end of a pipe out of the program being asked: as
    /// long as somebody holds a writing end open, the reader waits — and a
    /// child holding one it never writes to would make the answer never end.
    void addClose(int descriptor) {
        record(posix_spawn_file_actions_addclose(&m_actions, descriptor));
    }

    [[nodiscard]] int error() const { return m_error; }

    [[nodiscard]] const posix_spawn_file_actions_t* get() const { return &m_actions; }

private:
    void record(int status) {
        if (status != 0 && m_error == 0)
            m_error = status;
    }

    posix_spawn_file_actions_t m_actions{};
    bool m_valid = false;
    int m_error = 0;
};

[[nodiscard]] LaunchErrorKind launchErrorKindOf(int code) {
    if (code == ENOENT)
        return LaunchErrorKind::NotFound;
    if (code == EACCES || code == EPERM)
        return LaunchErrorKind::PermissionDenied;
    return LaunchErrorKind::Failed;
}

[[nodiscard]] std::unexpected<LaunchError>
failure(int code, const std::filesystem::path& program, std::string_view reason) {
    return std::unexpected(LaunchError{
        .kind = launchErrorKindOf(code),
        .detail = program.string() + " : " + std::string{reason},
    });
}

/// A file descriptor, closed exactly once.
///
/// A pipe is two of them, and both have to be let go on every path out —
/// including the ones a refusal takes. Counting them by hand is how a program
/// runs out of descriptors after a few thousand refusals.
class Descriptor {

public:
    explicit Descriptor(int descriptor = -1) : m_descriptor(descriptor) {}

    Descriptor(const Descriptor&) = delete;
    Descriptor(Descriptor&&) = delete;
    Descriptor& operator=(const Descriptor&) = delete;
    Descriptor& operator=(Descriptor&&) = delete;

    ~Descriptor() { close(); }

    [[nodiscard]] int get() const { return m_descriptor; }

    void close() {
        if (m_descriptor >= 0)
            ::close(m_descriptor);
        m_descriptor = -1;
    }

private:
    int m_descriptor;
};

/// How much one read asks for. A frame rate is twelve bytes; a program with
/// more to say is read in as many turns as it takes.
constexpr std::size_t kReadBlockSize = 4096;

/// Turns what `waitpid` filled in into the number a caller reads.
///
/// Without `WUNTRACED` or `WCONTINUED`, `waitpid` only answers for a process
/// that has ended: the two cases are exhaustive, and a third branch would be a
/// line no test could ever reach.
[[nodiscard]] int exitCodeOf(int status) {
    if (WIFSIGNALED(status))
        return kSignalledExitBase + WTERMSIG(status);
    return WEXITSTATUS(status);
}

/// Starts `program` under `actions`, or says why it could not.
///
/// Shared by the two ways of running a program: everything but the
/// redirections is the same, and the refusals — a launch that could not be
/// prepared, redirections the system would not record, a program that is not
/// there — are the same three whichever way one runs it.
[[nodiscard]] std::expected<pid_t, LaunchError> spawnWith(const SpawnActions& actions,
                                                          const std::filesystem::path& program,
                                                          std::span<const std::string> arguments) {
    if (!actions.valid())
        return failure(errno, program, translate("could not prepare the launch"));
    if (actions.error() != 0)
        return failure(actions.error(), program, translate("could not redirect the output"));

    // argv[0] is the program itself, by convention and by necessity. The
    // strings are copied because posix_spawn wants writable pointers, and the
    // caller's span is not ours to write into.
    std::vector<std::string> owned;
    owned.reserve(arguments.size() + 1);
    owned.emplace_back(program.string());
    owned.insert(owned.end(), arguments.begin(), arguments.end());

    std::vector<char*> argv;
    argv.reserve(owned.size() + 1);
    for (std::string& argument : owned)
        argv.push_back(argument.data());
    argv.push_back(nullptr);

    pid_t child = -1;
    const int spawned =
        ::posix_spawn(&child, owned.front().c_str(), actions.get(), nullptr, argv.data(), environ);
    if (spawned != 0)
        return failure(spawned, program, translate("could not be started"));

    return child;
}

/// How often a streamed run looks at the cancellation while nothing is written.
constexpr int kPollMilliseconds = 50;

/// How often it looks at a program that has closed its streams but not ended.
constexpr int kReapMilliseconds = 10;

/// Cuts what arrives in chunks into lines, keeping the unfinished one.
class LineBuffer {

public:
    /// Hands each complete line of `chunk` to `sink`, without its newline.
    template<typename Sink>
    void feed(std::string_view chunk, Sink&& sink) {
        m_pending.append(chunk);
        std::size_t start = 0;
        for (std::size_t end = m_pending.find('\n'); end != std::string::npos;
             end = m_pending.find('\n', start)) {
            sink(m_pending.substr(start, end - start));
            start = end + 1;
        }
        m_pending.erase(0, start);
    }

    /// Hands the unfinished line, if there is one, to `sink`.
    template<typename Sink>
    void finish(Sink&& sink) {
        if (!m_pending.empty())
            sink(std::move(m_pending));
        m_pending.clear();
    }

private:
    std::string m_pending;
};

/// Waits for a program that has closed its streams to end. Its wait status, or
/// nothing if the caller asked to stop first.
[[nodiscard]] std::optional<int> awaitEnd(pid_t child, const std::function<bool()>& stopRequested) {
    int status = 0;
    while (::waitpid(child, &status, WNOHANG) == 0) {
        if (stopRequested())
            return std::nullopt;
        ::poll(nullptr, 0, kReapMilliseconds);
    }
    return status;
}

/// Ends a program that was asked to stop: politely first, then for good.
/// Returns its wait status.
[[nodiscard]] int terminate(pid_t child, std::chrono::milliseconds grace) {
    ::kill(child, SIGTERM);
    const auto deadline = std::chrono::steady_clock::now() + grace;
    int status = 0;
    while (std::chrono::steady_clock::now() < deadline) {
        if (::waitpid(child, &status, WNOHANG) != 0)
            return status;
        ::poll(nullptr, 0, kReapMilliseconds);
    }
    ::kill(child, SIGKILL);
    while (::waitpid(child, &status, 0) < 0 && errno == EINTR) {
    }
    return status;
}

} // namespace

std::expected<ProcessHandle, LaunchError> startProcess(const std::filesystem::path& program,
                                                       std::span<const std::string> arguments,
                                                       const std::filesystem::path& output) {
    SpawnActions actions;
    actions.addOpen(STDIN_FILENO, "/dev/null", O_RDONLY, 0);
    actions.addOpen(
        STDOUT_FILENO, output.c_str(), O_WRONLY | O_CREAT | O_TRUNC, kOutputPermissions);
    actions.addDuplicate(STDOUT_FILENO, STDERR_FILENO);

    const std::expected<pid_t, LaunchError> child = spawnWith(actions, program, arguments);
    if (!child.has_value())
        return std::unexpected(child.error());

    return ProcessHandle{.id = *child};
}

std::expected<ProgramOutput, LaunchError> runAndCapture(const std::filesystem::path& program,
                                                        std::span<const std::string> arguments) {
    std::array<int, 2> ends{-1, -1};
    if (::pipe(ends.data()) != 0)
        return failure(errno, program, translate("could not open a pipe"));
    const Descriptor readEnd{ends[0]};
    Descriptor writeEnd{ends[1]};

    SpawnActions actions;
    actions.addOpen(STDIN_FILENO, "/dev/null", O_RDONLY, 0);
    actions.addDuplicate(writeEnd.get(), STDOUT_FILENO);
    actions.addOpen(STDERR_FILENO, "/dev/null", O_WRONLY, 0);
    // Neither end of the pipe is the child's to hold: it writes through the
    // standard output it was just given.
    actions.addClose(writeEnd.get());
    actions.addClose(readEnd.get());

    const std::expected<pid_t, LaunchError> child = spawnWith(actions, program, arguments);
    if (!child.has_value())
        return std::unexpected(child.error());

    // **Ours has to go before the reading starts.** End of file comes when the
    // last writing end closes, and this one is a writing end.
    writeEnd.close();

    ProgramOutput answer;
    std::array<char, kReadBlockSize> buffer{};
    while (true) {
        const ssize_t count = ::read(readEnd.get(), buffer.data(), buffer.size());
        // Nothing more to come, or a read that failed for a reason other than
        // having been interrupted — an interruption is not an answer, only an
        // interruption of the asking.
        if (count == 0 || (count < 0 && errno != EINTR))
            break;
        if (count > 0)
            answer.output.append(buffer.data(), static_cast<std::size_t>(count));
    }

    int status = 0;
    while (::waitpid(*child, &status, 0) < 0 && errno == EINTR) {
    }

    answer.code = exitCodeOf(status);
    return answer;
}

std::expected<StreamedRun, LaunchError> runStreaming(const std::filesystem::path& program,
                                                     std::span<const std::string> arguments,
                                                     const StreamObserver& observer) {
    std::array<int, 2> outputEnds{-1, -1};
    if (::pipe(outputEnds.data()) != 0)
        return failure(errno, program, translate("could not open a pipe"));
    Descriptor outputRead{outputEnds[0]};
    Descriptor outputWrite{outputEnds[1]};

    std::array<int, 2> errorEnds{-1, -1};
    if (::pipe(errorEnds.data()) != 0)
        return failure(errno, program, translate("could not open a pipe"));
    Descriptor errorRead{errorEnds[0]};
    Descriptor errorWrite{errorEnds[1]};

    SpawnActions actions;
    actions.addOpen(STDIN_FILENO, "/dev/null", O_RDONLY, 0);
    actions.addDuplicate(outputWrite.get(), STDOUT_FILENO);
    actions.addDuplicate(errorWrite.get(), STDERR_FILENO);
    // None of the four ends is the child's to hold: it writes through the two
    // standard descriptors it was just given.
    actions.addClose(outputRead.get());
    actions.addClose(outputWrite.get());
    actions.addClose(errorRead.get());
    actions.addClose(errorWrite.get());

    const std::expected<pid_t, LaunchError> child = spawnWith(actions, program, arguments);
    if (!child.has_value())
        return std::unexpected(child.error());

    // **Ours have to go before the reading starts**, for the reason `runAndCapture` gives.
    outputWrite.close();
    errorWrite.close();

    LineBuffer outputLines;
    LineBuffer errorLines;
    const auto deliver = [&observer](StreamName stream) {
        return [&observer, stream](std::string line) {
            if (observer.onLine)
                observer.onLine(StreamedLine{.stream = stream, .text = std::move(line)});
        };
    };

    // Reads what is there on one stream; lets it go when it has ended.
    const auto pump = [&](Descriptor& descriptor, LineBuffer& lines, StreamName stream) {
        std::array<char, kReadBlockSize> buffer{};
        const ssize_t count = ::read(descriptor.get(), buffer.data(), buffer.size());
        if (count > 0) {
            lines.feed(std::string_view{buffer.data(), static_cast<std::size_t>(count)},
                       deliver(stream));
        } else if (count == 0 || errno != EINTR) {
            lines.finish(deliver(stream));
            descriptor.close();
        }
    };

    const auto stopRequested = [&observer] { return observer.cancelled && observer.cancelled(); };

    bool cancelled = false;
    while (outputRead.get() >= 0 || errorRead.get() >= 0) {
        if (stopRequested()) {
            cancelled = true;
            break;
        }
        // A closed stream has descriptor -1, which poll leaves alone.
        std::array<pollfd, 2> watched{{{.fd = outputRead.get(), .events = POLLIN, .revents = 0},
                                       {.fd = errorRead.get(), .events = POLLIN, .revents = 0}}};
        ::poll(watched.data(), watched.size(), kPollMilliseconds);
        if (watched[0].revents != 0)
            pump(outputRead, outputLines, StreamName::Output);
        if (watched[1].revents != 0)
            pump(errorRead, errorLines, StreamName::Error);
    }

    // Both streams are closed, or the caller stopped: the program may still be
    // running, and may still be asked to stop.
    const std::optional<int> ended = cancelled ? std::nullopt : awaitEnd(*child, stopRequested);
    const int status = ended ? *ended : terminate(*child, observer.grace);

    return StreamedRun{.code = exitCodeOf(status), .cancelled = !ended.has_value()};
}

std::optional<ProcessOutcome> outcomeOf(ProcessHandle handle) {
    if (handle.id < 0)
        return std::nullopt;

    int status = 0;
    pid_t answered = 0;
    // WNOHANG is what makes this a question rather than a wait. EINTR is not
    // an answer, only an interruption of the asking.
    while ((answered = ::waitpid(handle.id, &status, WNOHANG)) < 0 && errno == EINTR) {
    }

    // 0 means alive; a negative value means it is no longer ours to collect,
    // which a caller asking twice would see. Neither is an outcome.
    if (answered <= 0)
        return std::nullopt;

    return ProcessOutcome{.code = exitCodeOf(status)};
}

} // namespace subedit::core
