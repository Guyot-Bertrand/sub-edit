// Writing a project back to a file — issue #131, moved to the core by #154.
//
// **The byte-for-byte round trip is the criterion**, and it is not decorative:
// a file that arrived with a byte order mark and CRLF line endings, rewritten
// without them, would show a diff on every one of its lines where the user
// expected one corrected subtitle.
//
// **The requirement tags are gone, and deliberately.** `GUI-SAVE-01` and
// `GUI-SAVE-02` promise what the *window* does; `check-requirements.sh` reads
// them from the window and end-to-end binaries, so a tag written here would be
// invisible to it — decoration rather than traceability. What the window does
// is tested where the window is, in `window_files_test.cpp`, and those tags
// are there.

#include <subedit/core/format/project_file.hpp>
#include <subedit/core/format/write_error.hpp>
#include <subedit/core/io/in_memory_file_system.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/model/subtitle_format.hpp>
#include <subedit/core/model/subtitle_index.hpp>
#include <subedit/core/wording.hpp>

#include <catch2/catch_test_macros.hpp>

#include <expected>
#include <string>
#include <variant>

namespace {

using subedit::core::FileErrorKind;
using subedit::core::InMemoryFileSystem;
using subedit::core::openProject;
using subedit::core::Project;
using subedit::core::SaveError;
using subedit::core::saveProject;
using subedit::core::SubtitleFormat;
using subedit::core::SubtitleIndex;
using subedit::core::WriteError;

/// In the layout the project writes — closing blank line included, after the
/// last block too. That is the one the round trip is faithful to byte for byte;
/// the phase 1 spec puts it that way.
constexpr const char* kSubRip = "1\n"
                                "00:00:01,000 --> 00:00:02,000\n"
                                "Un.\n"
                                "\n"
                                "2\n"
                                "00:00:03,000 --> 00:00:04,000\n"
                                "Deux.\n"
                                "\n";

/// The same without its final blank line: readable, common, and outside the
/// layout.
constexpr const char* kSubRipUnclosed = "1\n"
                                        "00:00:01,000 --> 00:00:02,000\n"
                                        "Un.\n"
                                        "\n"
                                        "2\n"
                                        "00:00:03,000 --> 00:00:04,000\n"
                                        "Deux.\n";

/// The same, as a Windows editor would have left it: byte order mark and
/// CRLF.
[[nodiscard]] std::string withBomAndCrLf(const std::string& content) {
    std::string out = "\xEF\xBB\xBF";
    for (const char letter : content) {
        if (letter == '\n')
            out += '\r';
        out += letter;
    }
    return out;
}

[[nodiscard]] Project opened(const InMemoryFileSystem& files, const char* path) {
    const auto result = openProject(files, path);
    REQUIRE(result.has_value());
    return result->project;
}

} // namespace

TEST_CASE("a file already in the layout the project writes comes back identical byte for byte",
          "[format][save]") {
    const std::string original = withBomAndCrLf(kSubRip);
    InMemoryFileSystem files;
    files.addFile("film.srt", original);

    const Project project = opened(files, "film.srt");
    const std::expected<void, SaveError> saved =
        saveProject(files, project, "film.srt", SubtitleFormat::SubRip);

    REQUIRE(saved.has_value());
    CHECK(files.contentOf("film.srt").value_or("") == original);
}

TEST_CASE("saving keeps the line endings and the byte order mark of the file it came from",
          "[format][save]") {
    // Each separately, so that a failure says which of the two was lost.
    InMemoryFileSystem files;
    files.addFile("unix.srt", kSubRip);

    const Project project = opened(files, "unix.srt");
    REQUIRE(saveProject(files, project, "unix.srt", SubtitleFormat::SubRip).has_value());

    const std::string written = files.contentOf("unix.srt").value_or("");
    CHECK(written.find('\r') == std::string::npos);
    CHECK_FALSE(written.starts_with("\xEF\xBB\xBF"));
}

TEST_CASE("saving under another name leaves the first file alone", "[format][save]") {
    InMemoryFileSystem files;
    files.addFile("film.srt", kSubRip);

    const Project project = opened(files, "film.srt");
    REQUIRE(saveProject(files, project, "copie.srt", SubtitleFormat::SubRip).has_value());

    CHECK(files.contentOf("film.srt").value_or("") == kSubRip);
    CHECK(files.contentOf("copie.srt").value_or("") == kSubRip);
}

TEST_CASE("saving in the other format writes that format", "[format][save]") {
    InMemoryFileSystem files;
    files.addFile("film.srt", kSubRip);

    const Project project = opened(files, "film.srt");
    REQUIRE(saveProject(files, project, "film.vtt", SubtitleFormat::WebVtt).has_value());

    const std::string written = files.contentOf("film.vtt").value_or("");
    CHECK(written.starts_with("WEBVTT"));
    CHECK(written.find(",000") == std::string::npos);
    // A period for a comma, and the hours left out below one hour: both are
    // what WebVTT writes, and both tell the file produced from the one that was
    // read.
    CHECK(written.find("00:01.000 --> 00:02.000") != std::string::npos);
}

TEST_CASE("a file that is not UTF-8 comes back in its own encoding, byte for byte",
          "[format][save]") {
    // The encoding is put back like the line endings and the mark: the project
    // kept what it was read in, and saving writes it there again. Latin-1, a
    // lone 0xE9 where UTF-8 would need a second byte.
    const std::string original = "1\n00:00:01,000 --> 00:00:02,000\nUn caf\xE9.\n\n";
    InMemoryFileSystem files;
    files.addFile("film.srt", original);

    const Project project = opened(files, "film.srt");
    const std::expected<void, SaveError> saved =
        saveProject(files, project, "film.srt", SubtitleFormat::SubRip);

    REQUIRE(saved.has_value());
    CHECK(files.contentOf("film.srt").value_or("") == original);
}

TEST_CASE("a save the encoding cannot carry fails, and nothing is written", "[format][save]") {
    // Two failures meet in one answer, and they are not the same: the disk
    // refusing the bytes, and the bytes not existing. Here it is the second —
    // `ł` has no place in Latin-1 — and the file on disk keeps what it had.
    const std::string original = "1\n00:00:01,000 --> 00:00:02,000\nUn caf\xE9.\n\n";
    InMemoryFileSystem files;
    files.addFile("film.srt", original);

    Project project = opened(files, "film.srt");
    project.subtitleAt(SubtitleIndex::fromValue(0)).mainText = "Przyszedł późno.";

    const std::expected<void, SaveError> saved =
        saveProject(files, project, "film.srt", SubtitleFormat::SubRip);

    REQUIRE_FALSE(saved.has_value());
    CHECK(std::holds_alternative<WriteError>(saved.error()));
    CHECK(files.contentOf("film.srt").value_or("") == original);
}

// The two causes of a failed save, as a user is told them. Saving is two steps,
// and each has its own sentence — the counterpart of what #154 did for opening.
TEST_CASE("each cause of a failed save has its own sentence", "[format][save]") {
    InMemoryFileSystem files;
    files.addFile("film.srt", "1\n00:00:01,000 --> 00:00:02,000\nUn caf\xE9.\n\n");
    Project project = opened(files, "film.srt");

    files.failNextWrite(FileErrorKind::PermissionDenied);
    const std::expected<void, SaveError> refused =
        saveProject(files, project, "film.srt", SubtitleFormat::SubRip);

    project.subtitleAt(SubtitleIndex::fromValue(0)).mainText = "Przyszedł późno.";
    const std::expected<void, SaveError> unwritable =
        saveProject(files, project, "film.srt", SubtitleFormat::SubRip);

    REQUIRE_FALSE(refused.has_value());
    REQUIRE_FALSE(unwritable.has_value());
    CHECK(subedit::core::reasonOf(refused.error()) == "cannot be opened: permission denied");
    CHECK(subedit::core::reasonOf(unwritable.error()) ==
          "holds a character the chosen encoding cannot write");
}

TEST_CASE("a save that cannot be written says so", "[format][save]") {
    InMemoryFileSystem files;
    files.addFile("film.srt", kSubRip);
    const Project project = opened(files, "film.srt");
    files.failNextWrite(FileErrorKind::PermissionDenied);

    const std::expected<void, SaveError> saved =
        saveProject(files, project, "film.srt", SubtitleFormat::SubRip);

    CHECK_FALSE(saved.has_value());
}

TEST_CASE("a file outside that layout is normalised once and never again", "[format][save]") {
    // The second half of the guarantee, as the phase 1 spec states it: the
    // first save closes the last block with the blank line the layout calls
    // for, and no save afterwards touches anything. The file moves once, not at
    // every save.
    InMemoryFileSystem files;
    files.addFile("film.srt", kSubRipUnclosed);

    const Project first = opened(files, "film.srt");
    REQUIRE(saveProject(files, first, "film.srt", SubtitleFormat::SubRip).has_value());
    const std::string once = files.contentOf("film.srt").value_or("");

    CHECK(once != kSubRipUnclosed);
    CHECK(once == kSubRip);

    const Project second = opened(files, "film.srt");
    REQUIRE(saveProject(files, second, "film.srt", SubtitleFormat::SubRip).has_value());

    CHECK(files.contentOf("film.srt").value_or("") == once);
}

TEST_CASE("changing format leaves the other variant's extras out of the file", "[format][save]") {
    // **The point the ticket asked to look at rather than assume.** A project
    // read as WebVTT carries `WebVttExtras` — a cue identifier, placement
    // settings — which mean nothing in SubRip, and the other way round for
    // SubRip's coordinates.
    //
    // The writers query the variant through `std::get_if` and fall back on
    // their default values when it belongs to the other format. Nothing to
    // change, then, but nothing that said so: this test says it.
    InMemoryFileSystem files;
    files.addFile("film.vtt",
                  "WEBVTT\n"
                  "\n"
                  "chapitre-1\n"
                  "00:01.000 --> 00:02.000 align:start position:10%\n"
                  "Un.\n"
                  "\n");

    const Project project = opened(files, "film.vtt");
    REQUIRE(saveProject(files, project, "film.srt", SubtitleFormat::SubRip).has_value());

    const std::string written = files.contentOf("film.srt").value_or("");
    CHECK(written.find("chapitre-1") == std::string::npos);
    CHECK(written.find("align:start") == std::string::npos);
    // What matters is there, in SubRip's own shape.
    CHECK(written.find("00:00:01,000 --> 00:00:02,000") != std::string::npos);
    CHECK(written.find("Un.") != std::string::npos);
}

TEST_CASE("the extras of the format written are kept", "[format][save]") {
    // The counterpart of the previous one: what is left out is left out because
    // it belongs to the other format, and not because the extras would be
    // lost.
    InMemoryFileSystem files;
    files.addFile("film.vtt",
                  "WEBVTT\n"
                  "\n"
                  "chapitre-1\n"
                  "00:01.000 --> 00:02.000 align:start position:10%\n"
                  "Un.\n"
                  "\n");

    const Project project = opened(files, "film.vtt");
    REQUIRE(saveProject(files, project, "copie.vtt", SubtitleFormat::WebVtt).has_value());

    const std::string written = files.contentOf("copie.vtt").value_or("");
    CHECK(written.find("chapitre-1") != std::string::npos);
    CHECK(written.find("align:start position:10%") != std::string::npos);
}
