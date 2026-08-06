// The one test that touches a real disk, because it is the one place where
// the disk is the subject. Everything else goes through InMemoryFileSystem.

#include <subedit/core/format/atomic_write.hpp>
#include <subedit/core/format/file_system.hpp>
#include <subedit/core/format/real_file_system.hpp>

#include <catch2/catch_test_macros.hpp>

#include <chrono>
#include <expected>
#include <filesystem>
#include <string>

namespace {

using subedit::core::FileError;
using subedit::core::FileErrorKind;
using subedit::core::RealFileSystem;
using subedit::core::writeAtomically;

/// A directory that removes itself, so that a failing assertion cannot leave
/// anything behind in the temporary directory of the machine.
class ScratchDirectory {

public:
    ScratchDirectory() {
        const auto stamp = std::chrono::steady_clock::now().time_since_epoch().count();
        m_path = std::filesystem::temp_directory_path() /
                 ("subedit-test-" + std::to_string(stamp) + "-" + std::to_string(nextSerial()));
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
    /// The clock alone could hand out the same value twice; the serial makes
    /// two directories created in the same instant distinct anyway.
    [[nodiscard]] static int nextSerial() {
        static int serial = 0;
        return ++serial;
    }

    std::filesystem::path m_path;
};

} // namespace

TEST_CASE("a file written to disk reads back byte for byte", "[format][filesystem][disk]") {
    const ScratchDirectory scratch;
    RealFileSystem files;
    const std::filesystem::path path = scratch.file("dialogue.srt");
    // A null byte and a CR, to prove the stream is not translating anything.
    const std::string content{"ligne\r\nsuite\0fin", 16};

    REQUIRE(files.writeFile(path, content).has_value());

    const std::expected<std::string, FileError> read = files.readFile(path);
    REQUIRE(read.has_value());
    CHECK(*read == content);
}

TEST_CASE("reading a file that is not on disk fails", "[format][filesystem][disk]") {
    const ScratchDirectory scratch;
    const RealFileSystem files;

    const std::expected<std::string, FileError> read = files.readFile(scratch.file("absent.srt"));

    REQUIRE_FALSE(read.has_value());
    CHECK(read.error().kind == FileErrorKind::NotFound);
}

TEST_CASE("existence, renaming and removal follow the disk", "[format][filesystem][disk]") {
    const ScratchDirectory scratch;
    RealFileSystem files;
    const std::filesystem::path path = scratch.file("un.srt");
    const std::filesystem::path other = scratch.file("deux.srt");

    CHECK_FALSE(files.exists(path));
    REQUIRE(files.writeFile(path, "contenu").has_value());
    CHECK(files.exists(path));

    REQUIRE(files.rename(path, other).has_value());
    CHECK_FALSE(files.exists(path));
    CHECK(files.exists(other));

    REQUIRE(files.remove(other).has_value());
    CHECK_FALSE(files.exists(other));
}

TEST_CASE("renaming a file that is not on disk fails", "[format][filesystem][disk]") {
    const ScratchDirectory scratch;
    RealFileSystem files;

    const std::expected<void, FileError> renamed =
        files.rename(scratch.file("absent.srt"), scratch.file("autre.srt"));

    REQUIRE_FALSE(renamed.has_value());
    CHECK(renamed.error().kind == FileErrorKind::NotFound);
}

TEST_CASE("removing a directory that still holds something fails", "[format][filesystem][disk]") {
    // The path where the system answers with an error code rather than with a
    // plain « nothing removed ».
    const ScratchDirectory scratch;
    RealFileSystem files;
    const std::filesystem::path directory = scratch.file("plein");
    std::filesystem::create_directories(directory);
    REQUIRE(files.writeFile(directory / "dedans.srt", "contenu").has_value());

    const std::expected<void, FileError> removed = files.remove(directory);

    REQUIRE_FALSE(removed.has_value());
    CHECK(removed.error().kind == FileErrorKind::Io);
}

TEST_CASE("removing what is not on disk fails", "[format][filesystem][disk]") {
    const ScratchDirectory scratch;
    RealFileSystem files;

    const std::expected<void, FileError> removed = files.remove(scratch.file("absent.srt"));

    REQUIRE_FALSE(removed.has_value());
    CHECK(removed.error().kind == FileErrorKind::NotFound);
}

TEST_CASE("an atomic write on disk replaces the file and leaves nothing beside it",
          "[format][filesystem][disk]") {
    const ScratchDirectory scratch;
    RealFileSystem files;
    const std::filesystem::path path = scratch.file("dialogue.srt");
    REQUIRE(files.writeFile(path, "ancien").has_value());

    REQUIRE(writeAtomically(files, path, "nouveau").has_value());

    const std::expected<std::string, FileError> read = files.readFile(path);
    REQUIRE(read.has_value());
    CHECK(*read == "nouveau");
    CHECK_FALSE(files.exists(scratch.file("dialogue.srt.subedit-tmp")));
}

TEST_CASE("writing into a directory that does not exist fails without a crash",
          "[format][filesystem][disk]") {
    const ScratchDirectory scratch;
    RealFileSystem files;

    const std::expected<void, FileError> written =
        files.writeFile(scratch.file("absent/dialogue.srt"), "contenu");

    REQUIRE_FALSE(written.has_value());
    CHECK(written.error().kind == FileErrorKind::Io);
}
