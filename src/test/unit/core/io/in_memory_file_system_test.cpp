#include <subedit/core/io/file_system.hpp>
#include <subedit/core/io/in_memory_file_system.hpp>

#include <catch2/catch_test_macros.hpp>

#include <expected>
#include <filesystem>
#include <optional>
#include <string>

namespace {

using subedit::core::FileError;
using subedit::core::FileErrorKind;
using subedit::core::InMemoryFileSystem;

const std::filesystem::path kPath{"/films/dialogue.srt"};

} // namespace

TEST_CASE("a file put in memory can be read back", "[format][filesystem]") {
    InMemoryFileSystem files;
    files.addFile(kPath, "1\n00:00:01,000 --> 00:00:02,000\nBonjour.\n");

    const std::expected<std::string, FileError> content = files.readFile(kPath);

    REQUIRE(content.has_value());
    CHECK(content->starts_with("1\n"));
}

TEST_CASE("reading a file that is not there fails", "[format][filesystem]") {
    const InMemoryFileSystem files;

    const std::expected<std::string, FileError> content = files.readFile(kPath);

    REQUIRE_FALSE(content.has_value());
    CHECK(content.error().kind == FileErrorKind::NotFound);
}

TEST_CASE("asking for the content of a file that is not there gives nothing",
          "[format][filesystem]") {
    const InMemoryFileSystem files;

    CHECK(files.contentOf(kPath) == std::nullopt);
}

TEST_CASE("writing creates the file and overwrites what was there", "[format][filesystem]") {
    InMemoryFileSystem files;

    REQUIRE(files.writeFile(kPath, "premier").has_value());
    CHECK(files.contentOf(kPath) == "premier");

    REQUIRE(files.writeFile(kPath, "second").has_value());
    CHECK(files.contentOf(kPath) == "second");
}

TEST_CASE("existence follows what was written and removed", "[format][filesystem]") {
    InMemoryFileSystem files;
    CHECK_FALSE(files.exists(kPath));

    REQUIRE(files.writeFile(kPath, "quelque chose").has_value());
    CHECK(files.exists(kPath));

    REQUIRE(files.remove(kPath).has_value());
    CHECK_FALSE(files.exists(kPath));
}

TEST_CASE("removing a file that is not there fails", "[format][filesystem]") {
    InMemoryFileSystem files;

    const std::expected<void, FileError> removed = files.remove(kPath);

    REQUIRE_FALSE(removed.has_value());
    CHECK(removed.error().kind == FileErrorKind::NotFound);
}

TEST_CASE("renaming moves the content and leaves nothing behind", "[format][filesystem]") {
    InMemoryFileSystem files;
    files.addFile(kPath, "contenu");
    const std::filesystem::path destination{"/films/autre.srt"};

    REQUIRE(files.rename(kPath, destination).has_value());

    CHECK_FALSE(files.exists(kPath));
    CHECK(files.contentOf(destination) == "contenu");
}

TEST_CASE("renaming what is not there fails", "[format][filesystem]") {
    InMemoryFileSystem files;

    const std::expected<void, FileError> renamed = files.rename(kPath, "/films/autre.srt");

    REQUIRE_FALSE(renamed.has_value());
    CHECK(renamed.error().kind == FileErrorKind::NotFound);
}

TEST_CASE("a write can be made to fail on demand", "[format][filesystem]") {
    // This is what lets a test interrupt a save without unplugging anything.
    InMemoryFileSystem files;
    files.failNextWrite(FileErrorKind::PermissionDenied);

    const std::expected<void, FileError> written = files.writeFile(kPath, "contenu");

    REQUIRE_FALSE(written.has_value());
    CHECK(written.error().kind == FileErrorKind::PermissionDenied);
    CHECK_FALSE(files.exists(kPath));
    // Only the next one fails; the one after works.
    CHECK(files.writeFile(kPath, "contenu").has_value());
}

TEST_CASE("a rename can be made to fail on demand", "[format][filesystem]") {
    InMemoryFileSystem files;
    files.addFile(kPath, "contenu");
    files.failNextRename(FileErrorKind::Io);

    const std::expected<void, FileError> renamed = files.rename(kPath, "/films/autre.srt");

    REQUIRE_FALSE(renamed.has_value());
    CHECK(renamed.error().kind == FileErrorKind::Io);
    CHECK(files.exists(kPath));
}

TEST_CASE("a program put in memory is runnable, and an ordinary file is not",
          "[filesystem][executable]") {
    InMemoryFileSystem files;
    files.addExecutable("/usr/bin/ffprobe");
    files.addFile(kPath, "contenu");

    CHECK(files.exists("/usr/bin/ffprobe"));
    CHECK(files.isExecutable("/usr/bin/ffprobe"));
    CHECK_FALSE(files.isExecutable(kPath));
    CHECK_FALSE(files.isExecutable("/usr/bin/absent"));
}

TEST_CASE("renaming a program carries its mode over", "[filesystem][executable]") {
    InMemoryFileSystem files;
    files.addExecutable("/usr/bin/ffprobe");

    REQUIRE(files.rename("/usr/bin/ffprobe", "/usr/bin/ffprobe.old").has_value());

    CHECK(files.isExecutable("/usr/bin/ffprobe.old"));
    CHECK_FALSE(files.isExecutable("/usr/bin/ffprobe"));
}

TEST_CASE("removing a program leaves nothing runnable behind", "[filesystem][executable]") {
    InMemoryFileSystem files;
    files.addExecutable("/usr/bin/ffprobe");

    REQUIRE(files.remove("/usr/bin/ffprobe").has_value());

    CHECK_FALSE(files.isExecutable("/usr/bin/ffprobe"));
}
