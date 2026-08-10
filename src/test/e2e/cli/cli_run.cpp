#include "cli_run.hpp"

#include <array>
#include <cerrno>
#include <cstddef>
#include <poll.h>
#include <spawn.h>
#include <string>
#include <sys/wait.h>
#include <system_error>
#include <unistd.h>
#include <vector>

// unistd.h already declares `environ` (with _GNU_SOURCE / _DEFAULT_SOURCE, on
// this platform); a second extern declaration here would be redundant.

namespace subedit::e2e {
namespace {

[[noreturn]] void fail(int code, const std::string& what) {
    throw std::system_error(code, std::generic_category(), what);
}

/// An owned file descriptor, closed exactly once.
class Descriptor {
public:
    Descriptor() = default;

    explicit Descriptor(int descriptor) : m_descriptor(descriptor) {}

    Descriptor(const Descriptor&) = delete;
    Descriptor& operator=(const Descriptor&) = delete;

    Descriptor(Descriptor&& other) noexcept : m_descriptor(other.release()) {}

    Descriptor& operator=(Descriptor&& other) noexcept {
        if (this != &other) {
            reset();
            m_descriptor = other.release();
        }
        return *this;
    }

    ~Descriptor() { reset(); }

    [[nodiscard]] int get() const { return m_descriptor; }

    int release() {
        const int held = m_descriptor;
        m_descriptor = -1;
        return held;
    }

    void reset() {
        if (m_descriptor >= 0) {
            ::close(m_descriptor);
            m_descriptor = -1;
        }
    }

private:
    int m_descriptor = -1;
};

/// `posix_spawn_file_actions_t` is a resource: without the matching destroy,
/// LeakSanitizer reports a leak on every single invocation.
class SpawnActions {
public:
    SpawnActions() {
        const int status = posix_spawn_file_actions_init(&m_actions);
        if (status != 0) {
            fail(status, "posix_spawn_file_actions_init");
        }
    }

    SpawnActions(const SpawnActions&) = delete;
    SpawnActions& operator=(const SpawnActions&) = delete;
    SpawnActions(SpawnActions&&) = delete;
    SpawnActions& operator=(SpawnActions&&) = delete;

    ~SpawnActions() { posix_spawn_file_actions_destroy(&m_actions); }

    void redirect(int from, int to) {
        const int status = posix_spawn_file_actions_adddup2(&m_actions, from, to);
        if (status != 0) {
            fail(status, "posix_spawn_file_actions_adddup2");
        }
    }

    void closeInChild(int descriptor) {
        const int status = posix_spawn_file_actions_addclose(&m_actions, descriptor);
        if (status != 0) {
            fail(status, "posix_spawn_file_actions_addclose");
        }
    }

    posix_spawn_file_actions_t* get() { return &m_actions; }

private:
    posix_spawn_file_actions_t m_actions{};
};

struct Channel {
    Descriptor readEnd;
    Descriptor writeEnd;
};

Channel makeChannel() {
    std::array<int, 2> ends{-1, -1};
    if (::pipe(ends.data()) != 0) {
        fail(errno, "pipe");
    }
    return Channel{.readEnd = Descriptor(ends[0]), .writeEnd = Descriptor(ends[1])};
}

/// Reads both pipes at once until each reaches end of file.
///
/// Draining one before the other would deadlock as soon as the child filled
/// the pipe nobody is reading: it blocks writing, we block reading, and
/// neither ever moves. Invisible with today's one-line output, fatal the day
/// `--help` grows past a pipe buffer.
void drain(Descriptor& outEnd, Descriptor& errEnd, std::string& output, std::string& errors) {
    // POLLIN is an int macro and `events` is a short. The cast is explicit
    // because -Wconversion is an error here.
    constexpr auto kReadable = static_cast<short>(POLLIN);

    const std::array<Descriptor*, 2> ends{&outEnd, &errEnd};
    const std::array<std::string*, 2> targets{&output, &errors};

    std::size_t remaining = ends.size();
    while (remaining > 0) {
        std::array<pollfd, 2> watched{};
        for (std::size_t index = 0; index < ends.size(); ++index) {
            // A negative descriptor is ignored by poll, which is exactly what
            // a closed end should be.
            watched[index].fd = ends[index]->get();
            watched[index].events = kReadable;
            watched[index].revents = 0;
        }

        if (::poll(watched.data(), watched.size(), -1) < 0) {
            if (errno == EINTR) {
                continue;
            }
            fail(errno, "poll");
        }

        for (std::size_t index = 0; index < watched.size(); ++index) {
            if (watched[index].fd < 0 || watched[index].revents == 0) {
                continue;
            }

            std::array<char, 4096> buffer{};
            const ssize_t count = ::read(watched[index].fd, buffer.data(), buffer.size());
            if (count > 0) {
                targets[index]->append(buffer.data(), static_cast<std::size_t>(count));
                continue;
            }
            if (count < 0) {
                if (errno == EINTR) {
                    continue;
                }
                fail(errno, "read");
            }
            // count == 0: a clean end of file. This end has nothing more to
            // say.
            ends[index]->reset();
            --remaining;
        }
    }
}

} // namespace

CliRun invoke(const std::vector<std::string>& args) {
    Channel out = makeChannel();
    Channel err = makeChannel();

    SpawnActions actions;
    actions.redirect(out.writeEnd.get(), STDOUT_FILENO);
    actions.redirect(err.writeEnd.get(), STDERR_FILENO);
    // The child must not inherit the reading ends: while it holds them, the
    // parent never sees end of file and drain() waits forever.
    actions.closeInChild(out.readEnd.get());
    actions.closeInChild(err.readEnd.get());
    // dup2 above leaves the original write-end descriptor numbers open too.
    // Harmless for subedit-cli today, but a live write end would follow any
    // subprocess it ever spawned, and drain() would then wait forever on the
    // very deadlock its own comment warns about.
    actions.closeInChild(out.writeEnd.get());
    actions.closeInChild(err.writeEnd.get());

    // argv[0] is the program itself, by convention and by necessity.
    std::vector<std::string> owned;
    owned.reserve(args.size() + 1);
    owned.emplace_back(SUBEDIT_CLI_BINARY);
    owned.insert(owned.end(), args.begin(), args.end());

    std::vector<char*> argv;
    argv.reserve(owned.size() + 1);
    for (std::string& argument : owned) {
        argv.push_back(argument.data());
    }
    argv.push_back(nullptr);

    pid_t child = -1;
    const int spawned =
        ::posix_spawn(&child, owned.front().c_str(), actions.get(), nullptr, argv.data(), environ);
    if (spawned != 0) {
        fail(spawned, "posix_spawn " + owned.front());
    }

    // The parent writes nothing. Holding these open would keep end of file
    // from ever arriving.
    out.writeEnd.reset();
    err.writeEnd.reset();

    CliRun run;
    try {
        drain(out.readEnd, err.readEnd, run.output, run.errors);
    } catch (...) {
        // A failed read must not also leave a zombie behind. Best-effort: if
        // the reap itself fails here, the exception already in flight is the
        // one that matters, so its error is not chased any further.
        int reapStatus = 0;
        while (::waitpid(child, &reapStatus, 0) < 0 && errno == EINTR) {
        }
        throw;
    }

    int status = 0;
    while (::waitpid(child, &status, 0) < 0) {
        if (errno != EINTR) {
            fail(errno, "waitpid");
        }
    }
    run.exitCode = WIFEXITED(status) ? WEXITSTATUS(status) : 128 + WTERMSIG(status);
    return run;
}

} // namespace subedit::e2e
