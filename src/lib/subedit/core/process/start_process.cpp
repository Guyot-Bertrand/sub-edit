#include <subedit/core/process/start_process.hpp>

#include <cerrno>
#include <expected>
#include <fcntl.h>
#include <filesystem>
#include <optional>
#include <span>
#include <spawn.h>
#include <string>
#include <sys/wait.h>
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
failure(int code, const std::filesystem::path& program, const std::string& reason) {
    return std::unexpected(LaunchError{
        .kind = launchErrorKindOf(code),
        .detail = program.string() + " : " + reason,
    });
}

} // namespace

std::expected<ProcessHandle, LaunchError> startProcess(const std::filesystem::path& program,
                                                       std::span<const std::string> arguments,
                                                       const std::filesystem::path& output) {
    SpawnActions actions;
    if (!actions.valid())
        return failure(errno, program, "could not prepare the launch");

    actions.addOpen(STDIN_FILENO, "/dev/null", O_RDONLY, 0);
    actions.addOpen(
        STDOUT_FILENO, output.c_str(), O_WRONLY | O_CREAT | O_TRUNC, kOutputPermissions);
    actions.addDuplicate(STDOUT_FILENO, STDERR_FILENO);
    if (actions.error() != 0)
        return failure(actions.error(), program, "could not redirect the output");

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
        return failure(spawned, program, "could not be started");

    return ProcessHandle{.id = child};
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

    // Without WUNTRACED or WCONTINUED, `waitpid` only answers for a process
    // that has ended: the two cases below are exhaustive, and a third branch
    // here would be a line no test could ever reach.
    if (WIFSIGNALED(status))
        return ProcessOutcome{.code = kSignalledExitBase + WTERMSIG(status)};
    return ProcessOutcome{.code = WEXITSTATUS(status)};
}

} // namespace subedit::core
