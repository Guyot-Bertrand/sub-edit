// Reading a path into a project — the one recipe of issue #154.
//
// **What is proved here is that the cause survives the trip.** The recipe used
// to be written four times, and the copy that served the window translated
// every `FileError` into `ReadErrorKind::NoSubtitleFound`: a file that was
// absent, one the system refused, one that was not UTF-8 and one that was a
// Word document all came out saying « holds nothing recognisable as a
// subtitle », and the sentence was true of the last only.
//
// So each of the failures gets a case, and each case names the cause rather
// than merely checking that opening failed. A test that only asserted
// `has_value() == false` would have passed against the defect.

#include <subedit/core/format/open_error.hpp>
#include <subedit/core/format/project_file.hpp>
#include <subedit/core/io/in_memory_file_system.hpp>
#include <subedit/core/model/source_file.hpp>
#include <subedit/core/model/subtitle_format.hpp>
#include <subedit/core/wording.hpp>

#include <catch2/catch_test_macros.hpp>

#include <expected>
#include <string>
#include <variant>

namespace {

using subedit::core::FileError;
using subedit::core::FileErrorKind;
using subedit::core::InMemoryFileSystem;
using subedit::core::OpenedFile;
using subedit::core::OpenError;
using subedit::core::openProject;
using subedit::core::ReadError;
using subedit::core::ReadErrorKind;
using subedit::core::SubtitleFormat;

constexpr const char* kTwo = "1\n"
                             "00:00:01,000 --> 00:00:02,000\n"
                             "Un.\n"
                             "\n"
                             "2\n"
                             "00:00:03,000 --> 00:00:04,000\n"
                             "Deux.\n"
                             "\n";

/// Why the opening failed, when it failed for the file system's own reason.
[[nodiscard]] bool refusedWith(const OpenError& error, FileErrorKind kind) {
    const auto* refused = std::get_if<FileError>(&error);
    return refused != nullptr && refused->kind == kind;
}

/// Why the opening failed, when the bytes arrived but said nothing.
[[nodiscard]] bool unreadableWith(const OpenError& error, ReadErrorKind kind) {
    const auto* unread = std::get_if<ReadError>(&error);
    return unread != nullptr && unread->kind == kind;
}

} // namespace

TEST_CASE("a file that opens gives its subtitles, its shape and its weight", "[format][open]") {
    InMemoryFileSystem files;
    files.addFile("film.srt", kTwo);

    const std::expected<OpenedFile, OpenError> opened = openProject(files, "film.srt");

    REQUIRE(opened.has_value());
    CHECK(opened->project.count() == 2U);
    CHECK(opened->diagnostics.empty());
    // The shape the project needs to be written back as it was found — ADR 0018.
    CHECK(opened->project.sourceFile().format == SubtitleFormat::SubRip);
    CHECK(opened->project.sourceFile().path == "film.srt");
    // What `subedit-cli -vvv` prints, and the one thing that would have been
    // lost by handing the reading over.
    CHECK(opened->bytes == std::string{kTwo}.size());
}

TEST_CASE("a file the system does not have says exactly that", "[format][open]") {
    const InMemoryFileSystem files;

    const std::expected<OpenedFile, OpenError> opened = openProject(files, "absent.srt");

    REQUIRE_FALSE(opened.has_value());
    CHECK(refusedWith(opened.error(), FileErrorKind::NotFound));
}

TEST_CASE("a file the system refuses says that, and not something else", "[format][open]") {
    InMemoryFileSystem files;
    files.addFile("film.srt", kTwo);
    files.failNextRead(FileErrorKind::PermissionDenied);

    const std::expected<OpenedFile, OpenError> opened = openProject(files, "film.srt");

    REQUIRE_FALSE(opened.has_value());
    CHECK(refusedWith(opened.error(), FileErrorKind::PermissionDenied));
}

TEST_CASE("bytes that are not UTF-8 are a reader's failure, not the system's", "[format][open]") {
    InMemoryFileSystem files;
    // Latin-1: a lone 0xE9 where UTF-8 would need a second byte.
    files.addFile("film.srt", "1\n00:00:01,000 --> 00:00:02,000\nCaf\xE9.\n\n");

    const std::expected<OpenedFile, OpenError> opened = openProject(files, "film.srt");

    REQUIRE_FALSE(opened.has_value());
    CHECK(unreadableWith(opened.error(), ReadErrorKind::Undecodable));
}

TEST_CASE("a file in no format this tool knows says so", "[format][open]") {
    InMemoryFileSystem files;
    files.addFile("notes.txt", "rien de reconnaissable\n");

    const std::expected<OpenedFile, OpenError> opened = openProject(files, "notes.txt");

    REQUIRE_FALSE(opened.has_value());
    CHECK(unreadableWith(opened.error(), ReadErrorKind::UnknownFormat));
}

// The four causes, as a reader is told them. This is the visible half of #154:
// the words already existed and only the window could not reach them.
TEST_CASE("each cause has its own sentence", "[format][open]") {
    const InMemoryFileSystem empty;
    InMemoryFileSystem unknown;
    unknown.addFile("notes.txt", "rien de reconnaissable\n");

    CHECK(subedit::core::reasonOf(openProject(empty, "absent.srt").error()) == "does not exist");
    CHECK(subedit::core::reasonOf(openProject(unknown, "notes.txt").error()) ==
          "is in no format this tool knows");
}

TEST_CASE("what the reading recovered from is carried out", "[format][open]") {
    InMemoryFileSystem files;
    // Numbering the reader puts back: it recovers, and says it did.
    files.addFile("film.srt",
                  "7\n"
                  "00:00:01,000 --> 00:00:02,000\n"
                  "Un.\n"
                  "\n");

    const std::expected<OpenedFile, OpenError> opened = openProject(files, "film.srt");

    REQUIRE(opened.has_value());
    CHECK(opened->project.count() == 1U);
    CHECK_FALSE(opened->diagnostics.empty());
}
