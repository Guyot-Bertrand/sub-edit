#include <subedit/core/process/start_process.hpp>

#include <catch2/catch_test_macros.hpp>

#include <chrono>
#include <csignal>
#include <expected>
#include <filesystem>
#include <fstream>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

#include "await_process.hpp"

namespace {

using subedit::core::LaunchError;
using subedit::core::LaunchErrorKind;
using subedit::core::outcomeOf;
using subedit::core::ProcessHandle;
using subedit::core::ProcessOutcome;
using subedit::core::startProcess;
using subedit::test::RunningProcess;

/// The program the tests launch: one the project owns, that ends by itself.
const std::filesystem::path kFakePlayer{SUBEDIT_FAKE_PLAYER_BINARY};

/// A directory that removes itself, so that a failing assertion cannot leave
/// anything behind in the temporary directory of the machine.
class ScratchDirectory {

public:
    ScratchDirectory() {
        const auto stamp = std::chrono::steady_clock::now().time_since_epoch().count();
        m_path = std::filesystem::temp_directory_path() /
                 ("subedit-launch-" + std::to_string(stamp) + "-" + std::to_string(nextSerial()));
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

[[nodiscard]] std::vector<std::string> linesOf(const std::filesystem::path& path) {
    std::ifstream file{path, std::ios::binary};
    std::vector<std::string> lines;
    std::string line;
    while (std::getline(file, line))
        lines.push_back(line);
    return lines;
}

} // namespace

TEST_CASE("the arguments a launched program received are the ones it was given",
          "[process][launch]") {
    const ScratchDirectory scratch;
    const std::filesystem::path output = scratch.file("sortie");
    const std::vector<std::string> arguments{
        "--osd-level=2", "--start=12.480", "--sub-file=/tmp/dialogue.srt", "/films/film.mkv"};

    const std::expected<ProcessHandle, LaunchError> started =
        startProcess(kFakePlayer, arguments, output);

    REQUIRE(started.has_value());
    RunningProcess running{*started};
    const std::optional<ProcessOutcome> outcome = running.await();
    REQUIRE(outcome.has_value());
    CHECK(outcome == ProcessOutcome{.code = 0});
    CHECK(linesOf(output) == arguments);
}

TEST_CASE("a program can be told to fail, and its complaint comes back", "[process][launch]") {
    const ScratchDirectory scratch;
    const std::filesystem::path output = scratch.file("sortie");
    const std::vector<std::string> arguments{"/films/film.mkv", "--fail-with=3"};

    const std::expected<ProcessHandle, LaunchError> started =
        startProcess(kFakePlayer, arguments, output);

    REQUIRE(started.has_value());
    RunningProcess running{*started};
    const std::optional<ProcessOutcome> outcome = running.await();
    REQUIRE(outcome.has_value());
    CHECK(outcome == ProcessOutcome{.code = 3});

    // Both streams land in the same file, as they do for Gaupol: what one
    // wants of a player that failed is what it said, not which channel it
    // said it on.
    const std::vector<std::string> written = linesOf(output);
    REQUIRE(written.size() == arguments.size() + 1);
    CHECK(written.back().find("refusing to play") != std::string::npos);
}

TEST_CASE("starting a program that is not there fails, and says so", "[process][launch]") {
    const ScratchDirectory scratch;

    const std::expected<ProcessHandle, LaunchError> started =
        startProcess(scratch.file("absent"), {}, scratch.file("sortie"));

    REQUIRE_FALSE(started.has_value());
    CHECK(started.error().kind == LaunchErrorKind::NotFound);
    CHECK_FALSE(started.error().detail.empty());
}

TEST_CASE("starting a program that cannot be run fails on the permission", "[process][launch]") {
    const ScratchDirectory scratch;
    const std::filesystem::path plain = scratch.file("pas-un-programme");
    {
        std::ofstream file{plain};
        file << "ceci n'est pas un programme\n";
    }

    const std::expected<ProcessHandle, LaunchError> started =
        startProcess(plain, {}, scratch.file("sortie"));

    REQUIRE_FALSE(started.has_value());
    CHECK(started.error().kind == LaunchErrorKind::PermissionDenied);
}

// The third refusal, and the one that says the mapping is a mapping and not
// two special cases: a file that is executable but is not a program at all.
// The system answers ENOEXEC, which is neither « not there » nor « not yours ».
TEST_CASE("a file that is executable but is not a program fails on neither of the two",
          "[process][launch]") {
    const ScratchDirectory scratch;
    const std::filesystem::path impostor = scratch.file("faux-programme");
    {
        std::ofstream file{impostor};
        file << "ceci n'est pas un programme\n";
    }
    std::filesystem::permissions(
        impostor, std::filesystem::perms::owner_exec, std::filesystem::perm_options::add);

    const std::expected<ProcessHandle, LaunchError> started =
        startProcess(impostor, {}, scratch.file("sortie"));

    REQUIRE_FALSE(started.has_value());
    CHECK(started.error().kind == LaunchErrorKind::Failed);
}

TEST_CASE("an output that cannot be written fails the launch rather than the program",
          "[process][launch]") {
    const ScratchDirectory scratch;

    const std::expected<ProcessHandle, LaunchError> started =
        startProcess(kFakePlayer, {}, scratch.file("absent/sortie"));

    REQUIRE_FALSE(started.has_value());
    CHECK(started.error().kind == LaunchErrorKind::NotFound);
}

TEST_CASE("asking after a process that is still running answers nothing", "[process][launch]") {
    const ScratchDirectory scratch;
    // A program that will not have ended when we ask. The destructor of
    // RunningProcess is what makes this test safe to write, and what it is
    // there for.
    const std::vector<std::string> arguments{"--linger=30"};

    const std::expected<ProcessHandle, LaunchError> started =
        startProcess(kFakePlayer, arguments, scratch.file("sortie"));

    REQUIRE(started.has_value());
    const RunningProcess running{*started};
    CHECK_FALSE(outcomeOf(*started).has_value());
}

TEST_CASE("a bound that is reached gives no outcome", "[process][launch]") {
    const ScratchDirectory scratch;

    const std::vector<std::string> arguments{"--linger=30"};

    const std::expected<ProcessHandle, LaunchError> started =
        startProcess(kFakePlayer, arguments, scratch.file("sortie"));

    REQUIRE(started.has_value());
    RunningProcess running{*started};

    CHECK_FALSE(running.await(std::chrono::milliseconds{20}).has_value());
}

TEST_CASE("a process is collected once, and asking again gives nothing", "[process][launch]") {
    const ScratchDirectory scratch;

    const std::expected<ProcessHandle, LaunchError> started =
        startProcess(kFakePlayer, {}, scratch.file("sortie"));

    REQUIRE(started.has_value());
    RunningProcess running{*started};
    REQUIRE(running.await().has_value());

    CHECK_FALSE(outcomeOf(*started).has_value());
}

TEST_CASE("a handle that names no process answers nothing", "[process][launch]") {

    CHECK_FALSE(outcomeOf(ProcessHandle{}).has_value());
}

// The criterion this whole class exists for, stated as an assertion rather
// than as a habit: whatever a test does, it does not leave a player running
// behind it.
TEST_CASE("a process that outlives its test is killed and collected", "[process][launch]") {
    const ScratchDirectory scratch;
    const std::vector<std::string> arguments{"--linger=30"};
    ProcessHandle handle;

    {
        const std::expected<ProcessHandle, LaunchError> started =
            startProcess(kFakePlayer, arguments, scratch.file("sortie"));
        REQUIRE(started.has_value());
        handle = *started;
        const RunningProcess running{*started};
    }

    // Signal zero asks « does this exist » without sending anything. The
    // answer could in theory be yes for a number the system has handed out
    // again since; in a test that just released it, it is not.
    CHECK(::kill(handle.id, 0) != 0);
}

// What a player killed from outside looks like — closed by a window manager,
// stopped by the person who started it. The shells report it as 128 plus the
// signal, and so does this.
TEST_CASE("a process killed by a signal reports it as the shells do", "[process][launch]") {
    const ScratchDirectory scratch;
    const std::vector<std::string> arguments{"--linger=30"};

    const std::expected<ProcessHandle, LaunchError> started =
        startProcess(kFakePlayer, arguments, scratch.file("sortie"));

    REQUIRE(started.has_value());
    RunningProcess running{*started};
    REQUIRE(::kill(started->id, SIGTERM) == 0);

    CHECK(running.await() == ProcessOutcome{.code = 128 + SIGTERM});
}
