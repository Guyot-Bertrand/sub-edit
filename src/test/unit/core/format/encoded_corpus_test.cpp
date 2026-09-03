// The nine encoding fixtures, each read in the encoding it was written in.
//
// They are built by `src/scripts/encoding-fixtures.py` from a table that says,
// for each one, its encoding, its mark, its line ending and its lines; `make
// fixtures` rebuilds them and compares byte for byte. **A file in CP1250 does
// not say what it is** — no diff can be read, and no test can assume. What
// follows therefore names the encoding of each, and asserts on the text that
// comes out.

#include <subedit/core/format/read_error.hpp>
#include <subedit/core/format/read_result.hpp>
#include <subedit/core/format/subtitle_file.hpp>
#include <subedit/core/format/subtitle_writer.hpp>
#include <subedit/core/format/write_error.hpp>
#include <subedit/core/io/real_file_system.hpp>
#include <subedit/core/model/encoding.hpp>
#include <subedit/core/model/subtitle.hpp>

#include <catch2/catch_test_macros.hpp>

#include <array>
#include <expected>
#include <filesystem>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace {

using subedit::core::ByteOrderMark;
using subedit::core::Encoding;
using subedit::core::Newline;
using subedit::core::ReadError;
using subedit::core::ReadErrorKind;
using subedit::core::ReadResult;
using subedit::core::readSubtitles;
using subedit::core::RealFileSystem;
using subedit::core::Subtitle;
using subedit::core::WriteError;
using subedit::core::WriteErrorKind;
using subedit::core::WriteRequest;
using subedit::core::writeSubtitles;

/// What one fixture is: its file, the encoding it was written in, whether it
/// carries a mark, and the first line it should give back.
struct Fixture {
    std::string_view file;
    std::string_view encoding;
    ByteOrderMark mark;
    Newline newline;
    std::string_view firstLine;
};

/// The three sets of lines the fixtures share, one per script they need.
constexpr std::string_view kLatin = "Le port était vide à cette heure-là.";
constexpr std::string_view kCentral = "Přišel jsem pozdě.";
constexpr std::string_view kCyrillic = "Порт был пуст в этот час.";

constexpr std::array<Fixture, 9> kFixtures = {
    Fixture{.file = "utf-8-lf.srt",
            .encoding = "utf-8",
            .mark = ByteOrderMark::Absent,
            .newline = Newline::Lf,
            .firstLine = kLatin},
    Fixture{.file = "utf-8-bom-crlf.srt",
            .encoding = "utf-8",
            .mark = ByteOrderMark::Present,
            .newline = Newline::CrLf,
            .firstLine = kLatin},
    Fixture{.file = "utf-16-le-bom.srt",
            .encoding = "utf-16-le",
            .mark = ByteOrderMark::Present,
            .newline = Newline::CrLf,
            .firstLine = kLatin},
    Fixture{.file = "utf-16-be-bom.srt",
            .encoding = "utf-16-be",
            .mark = ByteOrderMark::Present,
            .newline = Newline::CrLf,
            .firstLine = kLatin},
    Fixture{.file = "latin1.srt",
            .encoding = "iso-8859-1",
            .mark = ByteOrderMark::Absent,
            .newline = Newline::Lf,
            .firstLine = kLatin},
    Fixture{.file = "cp1252.srt",
            .encoding = "cp1252",
            .mark = ByteOrderMark::Absent,
            .newline = Newline::Lf,
            .firstLine = kLatin},
    Fixture{.file = "cp1250.srt",
            .encoding = "cp1250",
            .mark = ByteOrderMark::Absent,
            .newline = Newline::Lf,
            .firstLine = kCentral},
    Fixture{.file = "koi8-r.srt",
            .encoding = "koi8-r",
            .mark = ByteOrderMark::Absent,
            .newline = Newline::Lf,
            .firstLine = kCyrillic},
    Fixture{.file = "cr-mac.srt",
            .encoding = "utf-8",
            .mark = ByteOrderMark::Absent,
            .newline = Newline::Cr,
            .firstLine = kLatin},
};

[[nodiscard]] std::string bytesOf(std::string_view file) {
    const std::filesystem::path path =
        std::filesystem::path{SUBEDIT_TEST_DATA_DIR} / "encodages" / file;
    const RealFileSystem files;
    std::expected<std::string, subedit::core::FileError> content = files.readFile(path);
    if (!content.has_value()) {
        FAIL("fixture d'encodage introuvable : " + path.string());
        return {};
    }
    return *std::move(content);
}

[[nodiscard]] Encoding named(std::string_view name, ByteOrderMark mark) {
    const std::optional<Encoding> encoding = Encoding::create(name, mark);
    if (!encoding.has_value()) {
        FAIL("ICU ne connaît pas l'encodage " + std::string{name});
        return Encoding::utf8(ByteOrderMark::Absent);
    }
    return *encoding;
}

} // namespace

TEST_CASE("every encoding fixture reads in its own encoding", "[format][encoding][corpus]") {
    for (const Fixture& fixture : kFixtures) {
        INFO("fixture : " << fixture.file);

        // The mark is not asked for: the caller names a charset, the file says
        // whether a mark is there. Asking without one is what a command line
        // giving `--encoding utf-16-le` will do.
        const Encoding asked = named(fixture.encoding, ByteOrderMark::Absent);
        const std::expected<ReadResult, ReadError> result =
            readSubtitles(bytesOf(fixture.file), asked);

        REQUIRE(result.has_value());
        CHECK(result->encoding == asked.withByteOrderMark(fixture.mark));
        CHECK(result->newline == fixture.newline);
        REQUIRE_FALSE(result->subtitles.empty());
        CHECK(result->subtitles[0].mainText == fixture.firstLine);
    }
}

TEST_CASE("every encoding fixture comes back byte for byte", "[format][encoding][corpus]") {
    // **The property that carries the phase.** It does not check on the text —
    // two encodings carry the same text — so it checks on the bytes: a file
    // read and written back with nothing asked of it is the same file. Losing
    // it means a user who corrected one subtitle finds every line of their
    // file in the diff.
    for (const Fixture& fixture : kFixtures) {
        INFO("fixture : " << fixture.file);
        const std::string original = bytesOf(fixture.file);
        const std::expected<ReadResult, ReadError> result = readSubtitles(original);

        REQUIRE(result.has_value());
        const std::expected<std::string, WriteError> written =
            writeSubtitles(result->format,
                           WriteRequest{
                               .subtitles = result->subtitles,
                               .newline = result->newline,
                               .encoding = result->encoding,
                               .header = result->header,
                           });

        REQUIRE(written.has_value());
        CHECK(*written == original);
    }
}

TEST_CASE("a character the encoding cannot write is named, not replaced",
          "[format][encoding][corpus]") {
    // ICU writes `?` for what it cannot map unless it is told to stop, and a
    // `?` written over the file it came from is text lost under the user's
    // eyes. `ł` has no place in Latin-1, and no right answer either.
    const std::expected<ReadResult, ReadError> result = readSubtitles(bytesOf("latin1.srt"));
    REQUIRE(result.has_value());

    std::vector<Subtitle> subtitles = result->subtitles;
    subtitles[0].mainText = "Przyszedł późno.";

    const std::expected<std::string, WriteError> written = writeSubtitles(
        result->format, WriteRequest{.subtitles = subtitles, .encoding = result->encoding});

    REQUIRE_FALSE(written.has_value());
    CHECK(written.error().kind == WriteErrorKind::Unencodable);
    CHECK(written.error().detail == "ł");
}

TEST_CASE("a mark is taken off rather than left at the head of the first line",
          "[format][encoding][corpus]") {
    // The invisible character that would otherwise open the file. In UTF-16 it
    // is also the two bytes that say the byte order, which is why it cannot
    // simply be handed to the converter.
    for (const std::string_view file : {"utf-8-bom-crlf.srt", "utf-16-le-bom.srt"}) {
        INFO("fixture : " << file);
        const Encoding asked =
            named(file.starts_with("utf-8") ? "utf-8" : "utf-16-le", ByteOrderMark::Absent);

        const std::expected<ReadResult, ReadError> result = readSubtitles(bytesOf(file), asked);

        REQUIRE(result.has_value());
        CHECK(result->encoding.byteOrderMark() == ByteOrderMark::Present);
        REQUIRE_FALSE(result->subtitles.empty());
        CHECK(result->subtitles[0].mainText.starts_with("Le port"));
    }
}

TEST_CASE("a file read in the wrong encoding is refused rather than mangled",
          "[format][encoding][corpus]") {
    // What every one of these fixtures is for: read as UTF-8, a Latin-1 file
    // used to come back as text with nonsense where its accents were. It is
    // refused instead, and the reason names the encoding tried. Given here
    // rather than detected — left to itself, the reading proposes Latin-1 and
    // succeeds, which is the case just below.
    const std::expected<ReadResult, ReadError> result =
        readSubtitles(bytesOf("latin1.srt"), Encoding::utf8(ByteOrderMark::Absent));

    REQUIRE_FALSE(result.has_value());
    CHECK(result.error().kind == ReadErrorKind::Undecodable);
    CHECK(result.error().detail == "UTF-8");
}

TEST_CASE("every encoding fixture reads with nobody naming its encoding",
          "[format][encoding][corpus]") {
    // The same nine files, opened the way a user opens one: without saying
    // anything. What the detection proposes has to be what they were written
    // in, and `score-encoding-detection.py` is what measures that on the whole
    // corpus — this is the same claim, held by the gate.
    for (const Fixture& fixture : kFixtures) {
        INFO("fixture : " << fixture.file);
        const std::expected<ReadResult, ReadError> result = readSubtitles(bytesOf(fixture.file));

        REQUIRE(result.has_value());
        CHECK(result->encoding == named(fixture.encoding, fixture.mark));
        REQUIRE_FALSE(result->subtitles.empty());
        CHECK(result->subtitles[0].mainText == fixture.firstLine);
    }
}

TEST_CASE("two single-byte encodings both decode, and say different things",
          "[format][encoding][corpus]") {
    // The reason a detection is a proposal and never a certainty — ADR 0027.
    // Nothing in these bytes says which of the two answers is the right one;
    // only one of them is French.
    const std::string bytes = bytesOf("latin1.srt");

    const std::expected<ReadResult, ReadError> western =
        readSubtitles(bytes, named("iso-8859-1", ByteOrderMark::Absent));
    const std::expected<ReadResult, ReadError> cyrillic =
        readSubtitles(bytes, named("koi8-r", ByteOrderMark::Absent));

    REQUIRE(western.has_value());
    REQUIRE(cyrillic.has_value());
    CHECK(western->subtitles[0].mainText == kLatin);
    CHECK(cyrillic->subtitles[0].mainText != kLatin);
}
