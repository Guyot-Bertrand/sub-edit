#include "cli_run.hpp"

#include <array>
#include <cerrno>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <optional>
#include <poll.h>
#include <spawn.h>
#include <sstream>
#include <string>
#include <string_view>
#include <sys/wait.h>
#include <system_error>
#include <unistd.h>
#include <utility>
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

namespace {

/// The name of the variable every standard configuration location hangs off.
///
/// One variable moves the lot, and it crosses the fork: a child reads it from
/// the environment it was handed, so redirecting it is the only mechanism that
/// works on a binary we do not compile into the test. Qt's own
/// `QStandardPaths::setTestModeEnabled` moves the locations of the process that
/// calls it, and a launched binary never calls it.
constexpr std::string_view kConfigHomeVariable = "XDG_CONFIG_HOME=";

/// A configuration home of this process's own, removed when it ends.
class PrivateConfigHome {
public:
    PrivateConfigHome() {
        m_path = std::filesystem::temp_directory_path() /
                 ("subedit-e2e-config-" + std::to_string(::getpid()));
        std::filesystem::remove_all(m_path);
        std::filesystem::create_directories(m_path);
    }

    PrivateConfigHome(const PrivateConfigHome&) = delete;
    PrivateConfigHome& operator=(const PrivateConfigHome&) = delete;
    PrivateConfigHome(PrivateConfigHome&&) = delete;
    PrivateConfigHome& operator=(PrivateConfigHome&&) = delete;

    /// **Never throws**, for the reason `Scratch`'s destructor does not.
    ~PrivateConfigHome() {
        std::error_code ignored;
        std::filesystem::remove_all(m_path, ignored);
    }

    [[nodiscard]] const std::filesystem::path& path() const { return m_path; }

private:
    std::filesystem::path m_path;
};

/// Made on first use and destroyed when the process ends, which is what removes
/// the directory.
const PrivateConfigHome& privateConfigHome() {
    static const PrivateConfigHome home;
    return home;
}

/// Starts `binary` with `args` and waits for it. The only difference between
/// the two exported runners is which path lands in argv[0].
CliRun run(const char* binary, const std::vector<std::string>& args) {
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
    owned.emplace_back(binary);
    owned.insert(owned.end(), args.begin(), args.end());

    std::vector<char*> argv;
    argv.reserve(owned.size() + 1);
    for (std::string& argument : owned) {
        argv.push_back(argument.data());
    }
    argv.push_back(nullptr);

    // The child's environment rather than ours: everything this process has,
    // with the configuration home pointed somewhere it can do no harm.
    std::vector<std::string> ownedEnvironment = childEnvironment();

    std::vector<char*> envp;
    envp.reserve(ownedEnvironment.size() + 1);
    for (std::string& variable : ownedEnvironment) {
        envp.push_back(variable.data());
    }
    envp.push_back(nullptr);

    pid_t child = -1;
    const int spawned = ::posix_spawn(
        &child, owned.front().c_str(), actions.get(), nullptr, argv.data(), envp.data());
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

} // namespace

} // namespace subedit::e2e

namespace subedit::e2e {

namespace {

/// A number no two live scratch directories share.
int nextScratch() {
    static int next = 0;
    return ++next;
}

} // namespace

std::string configHome() {
    return privateConfigHome().path().string();
}

std::vector<std::string> childEnvironment() {
    std::vector<std::string> variables;

    for (char** entry = environ; *entry != nullptr; ++entry) {
        const std::string_view variable{*entry};
        // Dropped rather than left in place: `posix_spawn` hands the array over
        // as it is, and which of two definitions of the same name wins is the
        // child's business, not ours.
        if (variable.starts_with(kConfigHomeVariable))
            continue;

        variables.emplace_back(variable);
    }

    variables.emplace_back(std::string{kConfigHomeVariable} + configHome());
    return variables;
}

std::string corpus(const std::string& relative) {
    return (std::filesystem::path{SUBEDIT_TEST_DATA_DIR} / relative).string();
}

std::string contentOf(const std::filesystem::path& path) {
    const std::ifstream file{path, std::ios::binary};
    std::ostringstream all;
    all << file.rdbuf();
    return all.str();
}

namespace {

/// Removes the first line of `text` and returns it, terminator included.
[[nodiscard]] std::string_view takeLine(std::string_view& text) {
    const std::size_t end = text.find('\n');
    const std::size_t length = end == std::string_view::npos ? text.size() : end + 1;
    const std::string_view line = text.substr(0, length);
    text.remove_prefix(length);
    return line;
}

/// `line` in double quotes, with what is invisible spelled out.
[[nodiscard]] std::string visible(std::string_view line) {
    constexpr std::string_view kByteOrderMark = "\xEF\xBB\xBF";
    constexpr std::string_view kDigits = "0123456789ABCDEF";

    std::string shown = "\"";
    while (!line.empty()) {
        if (line.starts_with(kByteOrderMark)) {
            shown += R"(\xEF\xBB\xBF)";
            line.remove_prefix(kByteOrderMark.size());
            continue;
        }
        const auto byte = static_cast<unsigned char>(line.front());
        line.remove_prefix(1);
        if (byte == '\r')
            shown += "\\r";
        else if (byte == '\n')
            shown += "\\n";
        else if (byte == '\t')
            shown += "\\t";
        else if (byte == '"' || byte == '\\')
            shown += {'\\', static_cast<char>(byte)};
        else if (byte < 0x20 || byte == 0x7F)
            shown += {'\\', 'x', kDigits[byte >> 4], kDigits[byte & 0xF]};
        else
            shown += static_cast<char>(byte);
    }
    return shown + "\"";
}

} // namespace

std::optional<std::string> firstDifference(std::string_view actual, std::string_view expected) {
    if (actual == expected)
        return std::nullopt;

    for (std::size_t number = 1;; ++number) {
        const bool actualEnded = actual.empty();
        const bool expectedEnded = expected.empty();
        const std::string_view got = takeLine(actual);
        const std::string_view wanted = takeLine(expected);
        if (got == wanted)
            continue;

        return "line " + std::to_string(number) + " differs\n  expected: " +
               (expectedEnded ? std::string{"<end of text>"} : visible(wanted)) +
               "\n  actual:   " + (actualEnded ? std::string{"<end of text>"} : visible(got));
    }
}

MatchesFile::MatchesFile(std::filesystem::path expected) : m_expected(std::move(expected)) {}

bool MatchesFile::match(const std::string& actual) const {
    std::error_code error;
    if (!std::filesystem::is_regular_file(m_expected, error)) {
        m_difference = "expected file not found: " + m_expected.string();
        return false;
    }
    const std::optional<std::string> difference = firstDifference(actual, contentOf(m_expected));
    m_difference = difference.value_or(std::string{});
    return !difference.has_value();
}

std::string MatchesFile::describe() const {
    return "is the content of " + m_expected.string() + "\n" + m_difference;
}

Scratch::Scratch() {
    const std::filesystem::path root = std::filesystem::temp_directory_path();
    m_path = root / ("subedit-e2e-" + std::to_string(std::filesystem::hash_value(root)) + "-" +
                     std::to_string(nextScratch()));
    std::filesystem::remove_all(m_path);
    std::filesystem::create_directories(m_path);
}

Scratch::~Scratch() {
    std::error_code ignored;
    std::filesystem::remove_all(m_path, ignored);
}

std::string Scratch::of(const std::string& name) const {
    return (m_path / name).string();
}

std::string Scratch::path() const {
    return m_path.string();
}

std::string srtText(const std::string& tag, int cues) {
    std::string text;
    for (int cue = 1; cue <= cues; ++cue) {
        const std::string second = std::to_string(cue);
        const std::string stamp = "00:00:0" + second;
        text += second;
        text += '\n';
        text += stamp;
        text += ",000 --> ";
        text += stamp;
        text += ",500\n";
        text += tag;
        text += ' ';
        text += second;
        text += "\n\n";
    }
    return text;
}

std::string writeFile(const Scratch& scratch, const std::string& name, const std::string& text) {
    const std::string path = scratch.of(name);
    std::filesystem::create_directories(std::filesystem::path{path}.parent_path());
    std::ofstream file{path, std::ios::binary};
    file << text;
    return path;
}

std::string writeSrt(const Scratch& scratch, const std::string& name, int cues) {
    return writeFile(scratch, name, srtText(name, cues));
}

std::vector<std::string>
writeSrtBatch(const Scratch& scratch, int count, const std::string& directory) {
    std::vector<std::string> paths;
    for (int index = 1; index <= count; ++index)
        paths.push_back(writeSrt(scratch, directory + "/file-" + std::to_string(index) + ".srt"));
    return paths;
}

std::string writeUnreadable(const Scratch& scratch, const std::string& name) {
    return writeFile(scratch, name, contentOf(corpus("malformes/vide.srt")));
}

CliRun invoke(const std::vector<std::string>& args) {
    return run(SUBEDIT_CLI_BINARY, args);
}

CliRun invokeGui(const std::vector<std::string>& args) {
    return run(SUBEDIT_GUI_BINARY, args);
}

} // namespace subedit::e2e
