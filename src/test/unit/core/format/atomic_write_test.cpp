#include <subedit/core/format/atomic_write.hpp>
#include <subedit/core/format/file_system.hpp>
#include <subedit/core/format/in_memory_file_system.hpp>

#include <catch2/catch_test_macros.hpp>

#include <expected>
#include <filesystem>

namespace {

using subedit::core::FileError;
using subedit::core::FileErrorKind;
using subedit::core::InMemoryFileSystem;
using subedit::core::writeAtomically;

const std::filesystem::path kPath{"/films/dialogue.srt"};

} // namespace

TEST_CASE("an atomic write puts the content in place", "[format][atomic]") {
    InMemoryFileSystem files;

    const std::expected<void, FileError> written = writeAtomically(files, kPath, "contenu");

    REQUIRE(written.has_value());
    CHECK(files.contentOf(kPath) == "contenu");
}

TEST_CASE("an atomic write leaves no temporary behind", "[format][atomic]") {
    InMemoryFileSystem files;

    REQUIRE(writeAtomically(files, kPath, "contenu").has_value());

    CHECK(files.fileCount() == 1);
}

TEST_CASE("an atomic write replaces what was already there", "[format][atomic]") {
    InMemoryFileSystem files;
    files.addFile(kPath, "ancien");

    REQUIRE(writeAtomically(files, kPath, "nouveau").has_value());

    CHECK(files.contentOf(kPath) == "nouveau");
    CHECK(files.fileCount() == 1);
}

TEST_CASE("a write interrupted before the rename leaves the original intact", "[format][atomic]") {
    // The whole point of writing through a temporary: a save that fails must
    // not take the previous version of the file with it.
    InMemoryFileSystem files;
    files.addFile(kPath, "ancien");
    files.failNextWrite(FileErrorKind::Io);

    const std::expected<void, FileError> written = writeAtomically(files, kPath, "nouveau");

    REQUIRE_FALSE(written.has_value());
    CHECK(written.error().kind == FileErrorKind::Io);
    CHECK(files.contentOf(kPath) == "ancien");
    CHECK(files.fileCount() == 1);
}

TEST_CASE("a write interrupted during the rename leaves the original intact", "[format][atomic]") {
    // The temporary exists and holds the new content, but the swap did not
    // happen. The old file must still be the old file, and the temporary must
    // not be left lying next to it.
    InMemoryFileSystem files;
    files.addFile(kPath, "ancien");
    files.failNextRename(FileErrorKind::PermissionDenied);

    const std::expected<void, FileError> written = writeAtomically(files, kPath, "nouveau");

    REQUIRE_FALSE(written.has_value());
    CHECK(written.error().kind == FileErrorKind::PermissionDenied);
    CHECK(files.contentOf(kPath) == "ancien");
    CHECK(files.fileCount() == 1);
}

TEST_CASE("a failed write on a file that did not exist creates nothing", "[format][atomic]") {
    InMemoryFileSystem files;
    files.failNextWrite(FileErrorKind::PermissionDenied);

    REQUIRE_FALSE(writeAtomically(files, kPath, "nouveau").has_value());

    CHECK(files.fileCount() == 0);
}
