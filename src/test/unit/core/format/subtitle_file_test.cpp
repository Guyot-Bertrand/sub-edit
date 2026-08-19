#include <subedit/core/format/diagnostic.hpp>
#include <subedit/core/format/read_error.hpp>
#include <subedit/core/format/read_result.hpp>
#include <subedit/core/format/subtitle_file.hpp>
#include <subedit/core/format/subtitle_writer.hpp>
#include <subedit/core/model/source_file.hpp>
#include <subedit/core/model/subtitle_format.hpp>

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <expected>
#include <string>
#include <string_view>

namespace {

using subedit::core::DiagnosticKind;
using subedit::core::Newline;
using subedit::core::ReadError;
using subedit::core::ReadErrorKind;
using subedit::core::ReadResult;
using subedit::core::readSubtitles;
using subedit::core::SubtitleFormat;
using subedit::core::Utf8Bom;
using subedit::core::WriteRequest;
using subedit::core::writeSubtitles;

constexpr std::string_view kBom = "\xEF\xBB\xBF";

constexpr std::string_view kSubRip = "1\n"
                                     "00:00:01,000 --> 00:00:02,000\n"
                                     "Bonjour.\n"
                                     "\n";

constexpr std::string_view kWebVtt = "WEBVTT\n"
                                     "\n"
                                     "00:01.000 --> 00:02.000\n"
                                     "Bonjour.\n";

ReadResult readOrFail(std::string_view content) {
    std::expected<ReadResult, ReadError> result = readSubtitles(content);
    if (!result.has_value()) {
        FAIL("la lecture a échoué alors qu'elle devait aboutir");
        return {};
    }
    return *std::move(result);
}

bool hasDiagnostic(const ReadResult& result, DiagnosticKind kind) {
    return std::ranges::any_of(result.diagnostics,
                               [kind](const auto& one) { return one.kind == kind; });
}

} // namespace

TEST_CASE("a SubRip file is read without being told what it is", "[format][file]") {
    const ReadResult result = readOrFail(kSubRip);

    CHECK(result.format == SubtitleFormat::SubRip);
    REQUIRE(result.subtitles.size() == 1);
    CHECK(result.subtitles[0].mainText == "Bonjour.");
}

TEST_CASE("a WebVTT file is read without being told what it is", "[format][file]") {
    const ReadResult result = readOrFail(kWebVtt);

    CHECK(result.format == SubtitleFormat::WebVtt);
    REQUIRE(result.subtitles.size() == 1);
    CHECK(result.header == "WEBVTT");
}

TEST_CASE("a file of an unknown format is refused, not guessed", "[format][file]") {
    const std::expected<ReadResult, ReadError> result = readSubtitles("du texte ordinaire\n");

    REQUIRE_FALSE(result.has_value());
    CHECK(result.error().kind == ReadErrorKind::UnknownFormat);
}

TEST_CASE("bytes that are not UTF-8 are refused, not mangled", "[format][file]") {
    // Reading Latin-1 as UTF-8 does not fail, it replaces the accents with
    // nonsense — and the user finds out once the file has been saved over.
    const std::expected<ReadResult, ReadError> result =
        readSubtitles("1\n00:00:01,000 --> 00:00:02,000\n\xE9t\xE9\n");

    REQUIRE_FALSE(result.has_value());
    CHECK(result.error().kind == ReadErrorKind::InvalidUtf8);
}

TEST_CASE("a file recognised but unreadable says which, not « unknown »", "[format][file]") {
    // The signature settles the format; that the file then holds no cue is a
    // different problem, and telling them apart is what lets a caller say
    // something useful to the user.
    const std::expected<ReadResult, ReadError> result = readSubtitles("WEBVTT\n");

    REQUIRE_FALSE(result.has_value());
    CHECK(result.error().kind == ReadErrorKind::NoSubtitleFound);
}

TEST_CASE("a byte order mark is noticed and kept out of the text", "[format][file]") {
    const ReadResult result = readOrFail(std::string{kBom} + std::string{kSubRip});

    CHECK(result.hadUtf8Bom);
    REQUIRE(result.subtitles.size() == 1);
    CHECK(result.subtitles[0].mainText == "Bonjour.");
}

TEST_CASE("a byte order mark does not hide the format", "[format][file]") {
    const ReadResult result = readOrFail(std::string{kBom} + std::string{kWebVtt});

    CHECK(result.format == SubtitleFormat::WebVtt);
    CHECK(result.hadUtf8Bom);
}

TEST_CASE("a file without a mark says so", "[format][file]") {
    CHECK_FALSE(readOrFail(kSubRip).hadUtf8Bom);
}

TEST_CASE("the line ending of the file is noticed", "[format][file]") {
    const ReadResult result = readOrFail("1\r\n"
                                         "00:00:01,000 --> 00:00:02,000\r\n"
                                         "Bonjour.\r\n");

    CHECK(result.newline == Newline::CrLf);
}

TEST_CASE("a file mixing its endings is read and reported", "[format][file]") {
    const ReadResult result = readOrFail("1\r\n"
                                         "00:00:01,000 --> 00:00:02,000\r\n"
                                         "Bonjour.\n");

    CHECK(result.newline == Newline::CrLf);
    CHECK(hasDiagnostic(result, DiagnosticKind::MixedNewlines));
    REQUIRE(result.subtitles.size() == 1);
}

TEST_CASE("writing puts the byte order mark back when there was one", "[format][file]") {
    const ReadResult result = readOrFail(std::string{kBom} + std::string{kSubRip});

    const std::string written = writeSubtitles(
        result.format, WriteRequest{.subtitles = result.subtitles}, Utf8Bom::Present);

    CHECK(written.starts_with(kBom));
}

TEST_CASE("writing adds no mark when the file had none", "[format][file]") {
    const std::string written =
        writeSubtitles(SubtitleFormat::SubRip, WriteRequest{}, Utf8Bom::Absent);

    CHECK_FALSE(written.starts_with(kBom));
}

TEST_CASE("a file with a mark comes back with its mark and nothing else changed",
          "[format][file]") {
    // The whole point: an invisible three-byte prefix must survive being
    // opened and saved, or every such file grows a spurious diff.
    const std::string original = std::string{kBom} + std::string{kSubRip};
    const ReadResult result = readOrFail(original);

    const std::string written =
        writeSubtitles(result.format,
                       WriteRequest{.subtitles = result.subtitles, .newline = result.newline},
                       result.hadUtf8Bom ? Utf8Bom::Present : Utf8Bom::Absent);

    CHECK(written == original);
}

TEST_CASE("a WebVTT file comes back with its header", "[format][file]") {
    const ReadResult result = readOrFail(kWebVtt);

    const std::string written =
        writeSubtitles(result.format,
                       WriteRequest{.subtitles = result.subtitles, .header = result.header},
                       Utf8Bom::Absent);

    CHECK(written == kWebVtt);
}
