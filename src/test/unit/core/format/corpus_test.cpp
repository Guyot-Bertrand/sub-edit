// The corpus, walked one file at a time.
//
// Everything else in this module tests a reader against a string written for
// the occasion. Here the files exist on disk, they are read through the real
// file system, and what is asserted is what a user would see on opening them.

#include <subedit/core/analysis/anomaly.hpp>
#include <subedit/core/format/diagnostic.hpp>
#include <subedit/core/format/read_error.hpp>
#include <subedit/core/format/read_result.hpp>
#include <subedit/core/format/subtitle_file.hpp>
#include <subedit/core/format/subtitle_writer.hpp>
#include <subedit/core/io/real_file_system.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/model/subtitle_format.hpp>

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <expected>
#include <filesystem>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace {

using subedit::core::Anomaly;
using subedit::core::AnomalyKind;
using subedit::core::DiagnosticKind;
using subedit::core::Project;
using subedit::core::ReadError;
using subedit::core::ReadErrorKind;
using subedit::core::ReadResult;
using subedit::core::readSubtitles;
using subedit::core::RealFileSystem;
using subedit::core::scanAnomalies;
using subedit::core::WriteRequest;
using subedit::core::writeSubtitles;

std::filesystem::path corpus(std::string_view relative) {
    return std::filesystem::path{SUBEDIT_TEST_DATA_DIR} / relative;
}

std::string bytesOf(const std::filesystem::path& path) {
    const RealFileSystem files;
    std::expected<std::string, subedit::core::FileError> content = files.readFile(path);
    if (!content.has_value()) {
        FAIL("fichier du corpus introuvable : " + path.string());
        return {};
    }
    return *std::move(content);
}

/// Reads then writes, putting back what the file arrived with.
std::string roundTrip(const ReadResult& result) {
    return writeSubtitles(result.format,
                          WriteRequest{
                              .subtitles = result.subtitles,
                              .newline = result.newline,
                              .encoding = result.encoding,
                              .header = result.header,
                          });
}

bool hasDiagnostic(const ReadResult& result, DiagnosticKind kind) {
    return std::ranges::any_of(result.diagnostics,
                               [kind](const auto& one) { return one.kind == kind; });
}

/// The files that must open, and come back out unchanged.
///
/// **Enumerated, never listed** — issue #289. This used to be nine names
/// written by hand, and a fixture added without being written in ran through
/// nothing: the tests stayed green and proved nothing about it. It is the
/// defect `check-installation.sh` names for the manual — "a list written by
/// hand goes stale at the first chapter added, in silence" — and it is about
/// to matter, because phase 8 adds fixtures by the handful.
///
/// The directory is the list. A file dropped into `valides/` is opened, and
/// comes back out byte for byte, without anyone having to remember.
[[nodiscard]] std::vector<std::filesystem::path> validFiles() {
    std::vector<std::filesystem::path> found;

    for (const std::filesystem::directory_entry& entry :
         std::filesystem::directory_iterator{corpus("valides")}) {
        if (entry.is_regular_file())
            found.push_back(entry.path());
    }

    // Sorted, so that a failure names the same file from one run to the next:
    // the order a directory hands back is the file system's business.
    std::ranges::sort(found);
    return found;
}

} // namespace

TEST_CASE("every valid file of the corpus opens", "[format][corpus]") {
    const std::vector<std::filesystem::path> files = validFiles();
    REQUIRE(files.size() >= 9);

    for (const std::filesystem::path& path : files) {
        const std::expected<ReadResult, ReadError> result = readSubtitles(bytesOf(path));

        INFO("fichier : " << path.filename().string());
        REQUIRE(result.has_value());
        CHECK_FALSE(result->subtitles.empty());
        CHECK(result->diagnostics.empty());
    }
}

TEST_CASE("every valid file of the corpus comes back byte for byte",
          "[format][corpus][roundtrip]") {
    // The promise made to the user: opening a file and saving it back changes
    // nothing. The numbering of SubRip is regenerated, which is why these
    // files carry the numbering the writer produces.
    for (const std::filesystem::path& path : validFiles()) {
        const std::string original = bytesOf(path);
        const std::expected<ReadResult, ReadError> result = readSubtitles(original);

        INFO("fichier : " << path.filename().string());
        REQUIRE(result.has_value());
        CHECK(roundTrip(*result) == original);
    }
}

TEST_CASE("a file saved under Windows keeps its endings and its mark",
          "[format][corpus][roundtrip]") {
    // Two invisible properties, and the ones most likely to be lost: a diff
    // covering every line of the file is what losing them looks like.
    const std::string original = bytesOf(corpus("valides/crlf-bom.srt"));
    const std::expected<ReadResult, ReadError> result = readSubtitles(original);

    REQUIRE(result.has_value());
    CHECK(result->encoding.byteOrderMark() == subedit::core::ByteOrderMark::Present);
    CHECK(result->newline == subedit::core::Newline::CrLf);
    CHECK(roundTrip(*result) == original);
}

TEST_CASE("an empty file is refused", "[format][corpus]") {
    const std::expected<ReadResult, ReadError> result =
        readSubtitles(bytesOf(corpus("malformes/vide.srt")));

    REQUIRE_FALSE(result.has_value());
    CHECK(result.error().kind == ReadErrorKind::UnknownFormat);
}

TEST_CASE("a file that is not UTF-8 opens, and says in what it was read", "[format][corpus]") {
    // Refused through seven phases, and read since the eighth. The file lives
    // in `malformes/` because that is where it was put when it could not be
    // read; what makes it worth keeping there is that nothing declares its
    // encoding.
    const std::expected<ReadResult, ReadError> result =
        readSubtitles(bytesOf(corpus("malformes/latin1.srt")));

    REQUIRE(result.has_value());
    CHECK_FALSE(result->subtitles.empty());
    CHECK(hasDiagnostic(*result, DiagnosticKind::GuessedEncoding));
}

TEST_CASE("a mark contradicted by the bytes is refused", "[format][corpus]") {
    // The one reading that still fails: a mark saying UTF-8 over bytes that are
    // not. The mark wins over every heuristic — it is the only declaration a
    // subtitle file carries — so no other encoding is tried.
    const std::expected<ReadResult, ReadError> result =
        readSubtitles(bytesOf(corpus("malformes/bom-utf8-menteur.srt")));

    REQUIRE_FALSE(result.has_value());
    CHECK(result.error().kind == ReadErrorKind::Undecodable);
}

TEST_CASE("a WebVTT file without its signature is not guessed", "[format][corpus]") {
    const std::expected<ReadResult, ReadError> result =
        readSubtitles(bytesOf(corpus("malformes/sans-signature.vtt")));

    REQUIRE_FALSE(result.has_value());
    CHECK(result.error().kind == ReadErrorKind::UnknownFormat);
}

TEST_CASE("the malformed files open, and say what is wrong with them", "[format][corpus]") {
    struct Expectation {
        std::string_view name;
        DiagnosticKind kind;
    };

    // Each of these opens: the anomaly is reported, never fatal. That is the
    // whole of ADR 0008, checked against files rather than against strings.
    const std::vector<Expectation> expectations = {
        {.name = "malformes/fins-de-ligne-melangees.srt", .kind = DiagnosticKind::MixedNewlines},
        {.name = "malformes/numerotation-absente.srt", .kind = DiagnosticKind::MissingNumbering},
        {.name = "malformes/numerotation-incoherente.srt",
         .kind = DiagnosticKind::InconsistentNumbering},
        {.name = "malformes/texte-avant-horodatage.srt",
         .kind = DiagnosticKind::TextBeforeAnyTimestamp},
        {.name = "malformes/bloc-inconnu.vtt", .kind = DiagnosticKind::UnknownBlock},
    };

    for (const Expectation& expected : expectations) {
        const std::expected<ReadResult, ReadError> result =
            readSubtitles(bytesOf(corpus(expected.name)));

        INFO("fichier : " << expected.name);
        REQUIRE(result.has_value());
        CHECK_FALSE(result->subtitles.empty());
        CHECK(hasDiagnostic(*result, expected.kind));
    }
}

TEST_CASE("timestamps without their padding are read without a word", "[format][corpus]") {
    // Accepted in silence on purpose: writing normalises them, so reporting
    // every field of every line would flood a list meant to be read.
    const std::expected<ReadResult, ReadError> result =
        readSubtitles(bytesOf(corpus("malformes/horodatage-court.srt")));

    REQUIRE(result.has_value());
    REQUIRE(result->subtitles.size() == 1);
    CHECK(result->subtitles[0].start.milliseconds() == 1500);
    CHECK(result->subtitles[0].end.milliseconds() == 3250);
    CHECK(result->diagnostics.empty());
}

TEST_CASE("an unclosed tag goes through untouched", "[format][corpus]") {
    // ADR 0009: the text is a raw string. Judging a tag needs the parser of
    // phase 4; until then, what the user typed comes back as they typed it.
    const std::string original = bytesOf(corpus("malformes/balise-non-fermee.srt"));
    const std::expected<ReadResult, ReadError> result = readSubtitles(original);

    REQUIRE(result.has_value());
    REQUIRE(result->subtitles.size() == 1);
    CHECK(result->subtitles[0].mainText == "<i>La balise ne se referme jamais.");
    CHECK(result->diagnostics.empty());
}

TEST_CASE("the files whose positions are wrong open, and the document says so",
          "[format][corpus]") {
    // The counterpart of the case above, on the other side of the line ADR 0018
    // draws: these three files are not malformed as *text*, so a reading has
    // nothing to say about them. What is wrong with them is what they hold —
    // and `scanAnomalies` still says it once they have been edited.
    struct Expectation {
        std::string_view name;
        AnomalyKind kind;
    };

    const std::vector<Expectation> expectations = {
        {.name = "malformes/fin-avant-debut.srt", .kind = AnomalyKind::EndBeforeStart},
        {.name = "malformes/chevauchement.srt", .kind = AnomalyKind::OverlappingSubtitles},
        {.name = "malformes/desordre.srt", .kind = AnomalyKind::OutOfOrder},
    };

    for (const Expectation& expected : expectations) {
        const std::expected<ReadResult, ReadError> result =
            readSubtitles(bytesOf(corpus(expected.name)));

        INFO("fichier : " << expected.name);
        REQUIRE(result.has_value());
        CHECK(result->diagnostics.empty());

        Project project;
        project.setSubtitles(result->subtitles);

        const std::vector<Anomaly> found = scanAnomalies(project);
        const bool named = std::ranges::any_of(
            found, [&expected](const Anomaly& anomaly) { return anomaly.kind == expected.kind; });
        CHECK(named);
    }
}
