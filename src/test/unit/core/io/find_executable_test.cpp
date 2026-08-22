#include <subedit/core/io/find_executable.hpp>
#include <subedit/core/io/in_memory_file_system.hpp>

#include <catch2/catch_test_macros.hpp>

#include <filesystem>
#include <optional>
#include <string_view>

namespace {

using subedit::core::findExecutable;
using subedit::core::InMemoryFileSystem;

constexpr std::string_view kSearchPath = "/usr/local/bin:/usr/bin:/bin";

/// A machine where `ffprobe` is installed where it usually is.
[[nodiscard]] InMemoryFileSystem machineWithFfprobe() {
    InMemoryFileSystem files;
    files.addExecutable("/usr/bin/ffprobe");
    return files;
}

} // namespace

TEST_CASE("a program on the search path is found", "[filesystem][executable]") {
    const InMemoryFileSystem files = machineWithFfprobe();

    CHECK(findExecutable(files, "ffprobe", kSearchPath) ==
          std::filesystem::path{"/usr/bin/ffprobe"});
}

TEST_CASE("a program that is not installed is not found", "[filesystem][executable]") {
    const InMemoryFileSystem files = machineWithFfprobe();

    CHECK_FALSE(findExecutable(files, "mpv", kSearchPath).has_value());
}

// The branch this whole search exists to make reachable: `ffprobe` comes with
// `ffmpeg`, which the project cannot demand of a user, so the code carries a
// path for its absence — and a path nobody can walk on purpose is a path that
// rots.
TEST_CASE("a machine without ffprobe answers that it has none", "[filesystem][executable]") {
    const InMemoryFileSystem bareMachine;

    CHECK_FALSE(findExecutable(bareMachine, "ffprobe", kSearchPath).has_value());
}

TEST_CASE("entries are read left to right, and the first match wins", "[filesystem][executable]") {
    InMemoryFileSystem files;
    files.addExecutable("/usr/local/bin/ffprobe");
    files.addExecutable("/usr/bin/ffprobe");

    CHECK(findExecutable(files, "ffprobe", kSearchPath) ==
          std::filesystem::path{"/usr/local/bin/ffprobe"});
}

TEST_CASE("a file that is there but cannot be run is not a program", "[filesystem][executable]") {
    InMemoryFileSystem files;
    files.addFile("/usr/bin/ffprobe", "not a program at all");

    CHECK_FALSE(findExecutable(files, "ffprobe", kSearchPath).has_value());
}

TEST_CASE("an empty search path finds nothing", "[filesystem][executable]") {
    const InMemoryFileSystem files = machineWithFfprobe();

    CHECK_FALSE(findExecutable(files, "ffprobe", "").has_value());
}

TEST_CASE("an empty entry is skipped rather than read as the current directory",
          "[filesystem][executable]") {
    InMemoryFileSystem files;
    files.addExecutable("ffprobe");

    CHECK_FALSE(findExecutable(files, "ffprobe", ":/usr/bin:").has_value());
}

TEST_CASE("a name carrying a separator is a path, and is not searched for",
          "[filesystem][executable]") {
    InMemoryFileSystem files;
    files.addExecutable("/opt/ffmpeg/bin/ffprobe");
    files.addExecutable("/usr/bin/ffprobe");

    CHECK(findExecutable(files, "/opt/ffmpeg/bin/ffprobe", kSearchPath) ==
          std::filesystem::path{"/opt/ffmpeg/bin/ffprobe"});
}

TEST_CASE("a path that names nothing runnable is not found either", "[filesystem][executable]") {
    const InMemoryFileSystem files = machineWithFfprobe();

    CHECK_FALSE(findExecutable(files, "/opt/ffmpeg/bin/ffprobe", kSearchPath).has_value());
}

TEST_CASE("an empty name is not a program", "[filesystem][executable]") {
    const InMemoryFileSystem files = machineWithFfprobe();

    CHECK_FALSE(findExecutable(files, "", kSearchPath).has_value());
}
