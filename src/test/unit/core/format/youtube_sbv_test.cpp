// YouTube SBV — the first format of phase 17, and the first Gaupol does not read.
//
// Blocks of a time line and a text, a blank line between them. No header, no
// numbering, no markup. No specification is published: what is asserted here is
// what the platform's help page describes (spec 17, D7), written by hand.

#include <subedit/core/format/diagnostic.hpp>
#include <subedit/core/format/read_error.hpp>
#include <subedit/core/format/read_result.hpp>
#include <subedit/core/format/subtitle_writer.hpp>
#include <subedit/core/format/youtube_sbv_reader.hpp>
#include <subedit/core/format/youtube_sbv_syntax.hpp>
#include <subedit/core/format/youtube_sbv_writer.hpp>
#include <subedit/core/model/source_file.hpp>
#include <subedit/core/model/subtitle.hpp>
#include <subedit/core/model/subtitle_format.hpp>
#include <subedit/core/time/timestamp.hpp>

#include <catch2/catch_test_macros.hpp>

#include <cstdint>
#include <expected>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace {

using subedit::core::DiagnosticKind;
using subedit::core::Newline;
using subedit::core::parseYouTubeSbvTimeLine;
using subedit::core::ReadError;
using subedit::core::ReadErrorKind;
using subedit::core::ReadResult;
using subedit::core::Subtitle;
using subedit::core::SubtitleFormat;
using subedit::core::Timestamp;
using subedit::core::WriteRequest;
using subedit::core::YouTubeSbvReader;
using subedit::core::YouTubeSbvTimeLine;
using subedit::core::YouTubeSbvWriter;

[[nodiscard]] ReadResult read(std::string_view content) {
    std::expected<ReadResult, ReadError> result = YouTubeSbvReader{}.read(content);
    if (!result.has_value()) {
        FAIL("the reading was meant to succeed");
        return {};
    }
    return *std::move(result);
}

[[nodiscard]] Subtitle at(std::int64_t start, std::int64_t end, std::string text) {
    return Subtitle{
        .start = Timestamp::fromMilliseconds(start),
        .end = Timestamp::fromMilliseconds(end),
        .mainText = std::move(text),
    };
}

[[nodiscard]] std::string written(const std::vector<Subtitle>& subtitles,
                                  Newline newline = Newline::Lf) {
    return YouTubeSbvWriter{}.write(WriteRequest{.subtitles = subtitles, .newline = newline});
}

} // namespace

TEST_CASE("an SBV time line is two stamps of three decimals", "[format][sbv]") {
    const std::optional<YouTubeSbvTimeLine> line =
        parseYouTubeSbvTimeLine("0:01:02.345,1:02:03.456");
    REQUIRE(line.has_value());
    if (line) { // the analyser does not know REQUIRE leaves the case
        CHECK(line->start.milliseconds() == 62'345);
        CHECK(line->end.milliseconds() == 3'723'456);
    }

    // Two hour digits are read, a sign is read, blanks around are not the line.
    CHECK(parseYouTubeSbvTimeLine("  00:00:01.000,00:00:02.000 ").has_value());
    CHECK(parseYouTubeSbvTimeLine("-0:00:01.000,0:00:02.000").has_value());

    // SubViewer 2's line, and shapes near it, are not this one.
    CHECK_FALSE(parseYouTubeSbvTimeLine("00:00:01.00,00:00:02.00").has_value());
    CHECK_FALSE(parseYouTubeSbvTimeLine("0:00:01.000;0:00:02.000").has_value());
    CHECK_FALSE(parseYouTubeSbvTimeLine("0:0:01.000,0:00:02.000").has_value());
    CHECK_FALSE(parseYouTubeSbvTimeLine("000:00:01.000,0:00:02.000").has_value());
    CHECK_FALSE(parseYouTubeSbvTimeLine("0:00:01.000,0:00:02.0000").has_value());
    CHECK_FALSE(parseYouTubeSbvTimeLine("0:00:01,000,0:00:02.000").has_value());
    CHECK_FALSE(parseYouTubeSbvTimeLine("a:00:01.000,0:00:02.000").has_value());
    CHECK_FALSE(parseYouTubeSbvTimeLine("0:00:01.000").has_value());
    CHECK_FALSE(parseYouTubeSbvTimeLine("0:60:01.000,0:00:02.000").has_value());
}

TEST_CASE("an SBV block's text runs to the blank line", "[format][sbv]") {
    const ReadResult result = read("0:00:01.000,0:00:03.000\n"
                                   "First line\n"
                                   "second line\n"
                                   "third line\n"
                                   "\n"
                                   "0:00:04.000,0:00:05.500\n"
                                   "<i>not markup</i>\n");

    CHECK(result.format == SubtitleFormat::YouTubeSbv);
    REQUIRE(result.subtitles.size() == 2);
    CHECK(result.subtitles[0].mainText == "First line\nsecond line\nthird line");
    CHECK(result.subtitles[1].start.milliseconds() == 4000);
    CHECK(result.subtitles[1].end.milliseconds() == 5500);
    // The format has no markup: a tag is text, held as it was written.
    CHECK(result.subtitles[1].mainText == "<i>not markup</i>");
    CHECK(result.diagnostics.empty());
}

TEST_CASE("an SBV time line followed by a blank one is a subtitle with no text", "[format][sbv]") {
    const ReadResult result = read("0:00:01.000,0:00:02.000\n"
                                   "\n"
                                   "0:00:03.000,0:00:04.000\n"
                                   "Said.\n");
    REQUIRE(result.subtitles.size() == 2);
    CHECK(result.subtitles[0].mainText.empty());
    CHECK(result.subtitles[1].mainText == "Said.");
}

TEST_CASE("a line outside any SBV block is reported and dropped", "[format][sbv]") {
    const ReadResult result = read("A stray line\n"
                                   "\n"
                                   "0:00:01.000,0:00:02.000\n"
                                   "Said.\n");
    REQUIRE(result.subtitles.size() == 1);
    REQUIRE(result.diagnostics.size() == 1);
    CHECK(result.diagnostics[0].kind == DiagnosticKind::IgnoredLine);
    CHECK(result.diagnostics[0].line == 1);
    CHECK(result.diagnostics[0].detail == "A stray line");
}

TEST_CASE("an SBV file without a single time line is refused", "[format][sbv]") {
    const std::expected<ReadResult, ReadError> result = YouTubeSbvReader{}.read("Nothing timed.\n");
    REQUIRE_FALSE(result.has_value());
    CHECK(result.error().kind == ReadErrorKind::NoSubtitleFound);
}

TEST_CASE("SBV is written as YouTube exports it", "[format][sbv]") {
    const std::vector<Subtitle> subtitles = {
        at(1000, 3000, "First\nsecond"),
        at(3'723'456, 3'724'000, "Past the hour"),
    };
    CHECK(written(subtitles) == "0:00:01.000,0:00:03.000\n"
                                "First\n"
                                "second\n"
                                "\n"
                                "1:02:03.456,1:02:04.000\n"
                                "Past the hour\n");

    // The file's endings are the caller's, inside a text as between blocks.
    CHECK(written(subtitles, Newline::CrLf) == "0:00:01.000,0:00:03.000\r\n"
                                               "First\r\n"
                                               "second\r\n"
                                               "\r\n"
                                               "1:02:03.456,1:02:04.000\r\n"
                                               "Past the hour\r\n");
}

TEST_CASE("an SBV writing reads back as itself", "[format][sbv]") {
    const std::vector<Subtitle> subtitles = {
        at(-500, 1000, "Before the start"),
        at(1000, 2000, ""),
        at(2000, 3000, "Last"),
    };
    const std::string first = written(subtitles);
    const ReadResult again = read(first);
    REQUIRE(again.subtitles.size() == 3);
    CHECK(again.subtitles[0].start.milliseconds() == -500);
    CHECK(again.subtitles[1].mainText.empty());
    CHECK(written(again.subtitles) == first);
}
