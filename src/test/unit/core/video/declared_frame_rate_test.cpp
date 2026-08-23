// What the container declares, read with `ffprobe`, and the two ways of
// getting nothing that are not errors.

#include <subedit/core/io/in_memory_file_system.hpp>
#include <subedit/core/io/real_file_system.hpp>
#include <subedit/core/time/frame_rate.hpp>
#include <subedit/core/video/declared_frame_rate.hpp>

#include <catch2/catch_test_macros.hpp>

#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <optional>
#include <string>

namespace {

using subedit::core::FrameRate;
using subedit::core::InMemoryFileSystem;
using subedit::core::parseDeclaredFrameRate;
using subedit::core::readDeclaredFrameRate;
using subedit::core::RealFileSystem;
using subedit::core::StandardFrameRate;

[[nodiscard]] std::filesystem::path fixture(const std::string& name) {
    return std::filesystem::path{SUBEDIT_TEST_DATA_DIR} / name;
}

/// The search path of the machine running the tests.
///
/// **The one place a test asks the machine anything**, and it asks the same
/// question a user's session would. `ffprobe` comes with `ffmpeg`, which
/// `setup-toolchain.sh` installs and which the continuous integration restores
/// from its package cache: this is a demand made of a development machine, in
/// the same breath as `gcovr` and `clang-tidy`.
[[nodiscard]] std::string machineSearchPath() {
    const char* path = std::getenv("PATH");
    return path == nullptr ? std::string{} : std::string{path};
}

/// A directory that removes itself, for the one test that has to plant a
/// program of its own on a search path.
class ScratchDirectory {

public:
    ScratchDirectory() {
        const auto stamp = std::chrono::steady_clock::now().time_since_epoch().count();
        m_path =
            std::filesystem::temp_directory_path() / ("subedit-probe-" + std::to_string(stamp));
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

    [[nodiscard]] std::string searchPath() const { return m_path.string(); }

private:
    std::filesystem::path m_path;
};

} // namespace

TEST_CASE("the rational ffprobe writes is read exactly", "[video][framerate]") {
    CHECK(parseDeclaredFrameRate("24000/1001\n") == FrameRate{StandardFrameRate::Fps23976});
    CHECK(parseDeclaredFrameRate("25/1\n") == FrameRate{StandardFrameRate::Fps25});
    // The last line of a program's output may have no newline of its own.
    CHECK(parseDeclaredFrameRate("30000/1001") == FrameRate{StandardFrameRate::Fps29970});
}

// What `ffprobe` writes when it has nothing to say, in each of its forms. None
// of them is an error: a file that is not a video is a perfectly ordinary
// thing to be handed.
TEST_CASE("what is not a rate reads as nothing", "[video][framerate]") {
    CHECK_FALSE(parseDeclaredFrameRate("").has_value());
    CHECK_FALSE(parseDeclaredFrameRate("\n").has_value());
    CHECK_FALSE(parseDeclaredFrameRate("0/0\n").has_value());
    CHECK_FALSE(parseDeclaredFrameRate("N/A\n").has_value());
    CHECK_FALSE(parseDeclaredFrameRate("24000\n").has_value());
    CHECK_FALSE(parseDeclaredFrameRate("24000/\n").has_value());
    CHECK_FALSE(parseDeclaredFrameRate("/1001\n").has_value());
    CHECK_FALSE(parseDeclaredFrameRate("24000/1001 fps\n").has_value());
    CHECK_FALSE(parseDeclaredFrameRate("-24000/1001\n").has_value());
}

// The number ADR 0020 keeps `ffprobe` for: `container-fps` of libmpv answers
// 23.976025 to this same file, and a `FrameRate` built from that would not be
// the rate the film was timed at.
TEST_CASE("a container timed at 24000/1001 declares that rational", "[video][framerate]") {
    const RealFileSystem files;

    const std::optional<FrameRate> declared =
        readDeclaredFrameRate(files, machineSearchPath(), fixture("videos/cadence-23-976.mp4"));

    // Written as the two terms rather than as `Fps23976`, so that what the
    // case claims is the rational itself and not a name for it.
    CHECK(declared == FrameRate::create(24000, 1001));
}

TEST_CASE("a container timed at a whole rate declares it too", "[video][framerate]") {
    const RealFileSystem files;

    const std::optional<FrameRate> declared =
        readDeclaredFrameRate(files, machineSearchPath(), fixture("videos/cadence-25.mp4"));

    CHECK(declared == FrameRate{StandardFrameRate::Fps25});
}

// The first of the two ways of getting nothing without an error, and the one
// the project promises a user: `ffprobe` comes with `ffmpeg`, which subedit
// asks of a development machine and never of whoever runs it.
TEST_CASE("a machine without ffprobe declares nothing, and does not fail", "[video][framerate]") {
    const InMemoryFileSystem bareMachine;

    CHECK_FALSE(readDeclaredFrameRate(
                    bareMachine, "/usr/local/bin:/usr/bin:/bin", fixture("videos/cadence-25.mp4"))
                    .has_value());
}

// The second: `ffprobe` answers, and has nothing to declare. It writes not one
// line and ends well, having been asked about a video stream that is not there.
TEST_CASE("a file that is not a video declares nothing", "[video][framerate]") {
    const RealFileSystem files;

    CHECK_FALSE(readDeclaredFrameRate(files, machineSearchPath(), fixture("valides/minimal.srt"))
                    .has_value());
}

TEST_CASE("a file that is not there declares nothing", "[video][framerate]") {
    const RealFileSystem files;

    CHECK_FALSE(readDeclaredFrameRate(files, machineSearchPath(), fixture("videos/absent.mp4"))
                    .has_value());
}

// A program named `ffprobe` that no system would run. The search finds it —
// it carries the execute bit — and the launch is what refuses. Nothing, again,
// and again without an error: subedit has no opinion about what else lives on
// a user's search path.
TEST_CASE("something named ffprobe that is not a program declares nothing", "[video][framerate]") {
    const ScratchDirectory scratch;
    const RealFileSystem files;

    const std::filesystem::path impostor = scratch.file("ffprobe");
    std::ofstream{impostor} << "ceci n'est pas un programme\n";
    std::filesystem::permissions(
        impostor, std::filesystem::perms::owner_exec, std::filesystem::perm_options::add);

    CHECK_FALSE(readDeclaredFrameRate(files, scratch.searchPath(), fixture("videos/cadence-25.mp4"))
                    .has_value());
}
