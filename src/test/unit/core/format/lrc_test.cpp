// LRC — the lyrics format, the last of the nine, and the other one with no end.
//
// `[00:12.34]text`, one line per subtitle, counted in hundredths. It is also
// the only one of the nine that cannot hold a line break at all: two lines come
// back as one, joined by a space, and that loss does not come back.

#include <subedit/core/format/diagnostic.hpp>
#include <subedit/core/format/lrc_reader.hpp>
#include <subedit/core/format/lrc_syntax.hpp>
#include <subedit/core/format/lrc_writer.hpp>
#include <subedit/core/format/read_error.hpp>
#include <subedit/core/format/read_result.hpp>
#include <subedit/core/format/subtitle_file.hpp>
#include <subedit/core/format/subtitle_writer.hpp>
#include <subedit/core/format/write_error.hpp>
#include <subedit/core/model/subtitle.hpp>
#include <subedit/core/model/subtitle_format.hpp>
#include <subedit/core/time/timestamp.hpp>

#include <catch2/catch_test_macros.hpp>

#include <array>
#include <cstdint>
#include <expected>
#include <string>
#include <string_view>
#include <utility>

namespace {

using subedit::core::DiagnosticKind;
using subedit::core::LrcReader;
using subedit::core::lrcTimeOf;
using subedit::core::LrcWriter;
using subedit::core::ReadError;
using subedit::core::ReadErrorKind;
using subedit::core::ReadResult;
using subedit::core::readSubtitles;
using subedit::core::Subtitle;
using subedit::core::SubtitleFormat;
using subedit::core::Timestamp;
using subedit::core::WriteError;
using subedit::core::WriteRequest;
using subedit::core::writeSubtitles;

[[nodiscard]] ReadResult read(std::string_view content) {
    const std::expected<ReadResult, ReadError> result = LrcReader{}.read(content);
    if (!result.has_value()) {
        FAIL("the reading was meant to succeed");
        return {};
    }
    return *result;
}

[[nodiscard]] Subtitle at(std::int64_t start, std::int64_t end, std::string text) {
    return Subtitle{
        .start = Timestamp::fromMilliseconds(start),
        .end = Timestamp::fromMilliseconds(end),
        .mainText = std::move(text),
    };
}

} // namespace

TEST_CASE("an LRC line counts in hundredths of a second", "[format][lrc]") {
    const ReadResult result = read("[00:01.00]Le vent se lève.\n[01:05.25]Et retombe.\n");

    CHECK(result.format == SubtitleFormat::Lrc);
    REQUIRE(result.subtitles.size() == 2);
    CHECK(result.subtitles[0].start.milliseconds() == 1000);
    CHECK(result.subtitles[1].start.milliseconds() == 65250);
    CHECK(result.subtitles[0].mainText == "Le vent se lève.");
}

TEST_CASE("the minutes carry the hours, past sixty and past a hundred", "[format][lrc]") {
    // **This format has no hours field**, so an hour and two minutes is written
    // `[62:03.00]`. Gaupol's pattern takes exactly two digits, which is why
    // nothing past `[99:59.99]` reads back there.
    const ReadResult result = read("[62:03.00]Une.\n[120:00.00]Deux.\n");

    REQUIRE(result.subtitles.size() == 2);
    CHECK(result.subtitles[0].start.milliseconds() == 3723000);
    CHECK(result.subtitles[1].start.milliseconds() == 7200000);
}

TEST_CASE("an LRC reading works its ends out the same way", "[format][lrc][ends]") {
    const ReadResult result = read("[00:01.00]Une.\n[00:04.00]Deux.\n[00:08.00]Trois.\n");

    REQUIRE(result.subtitles.size() == 3);
    CHECK(result.subtitles[0].end.milliseconds() == 4000);
    CHECK(result.subtitles[1].end.milliseconds() == 8000);
    CHECK(result.subtitles[2].end.milliseconds() == 13000);
}

TEST_CASE("a reading that worked the ends out says so, here too", "[format][lrc][ends]") {
    const ReadResult result = read("[00:01.00]Une.\n");

    REQUIRE_FALSE(result.diagnostics.empty());
    CHECK(result.diagnostics.front().kind == DiagnosticKind::DeducedEnds);
    CHECK(result.diagnostics.front().line == subedit::core::kWholeFile);
}

TEST_CASE("everything before the first timed line is the header", "[format][lrc]") {
    // The `[ar:…]` and `[ti:…]` tags a player reads. They look like timed lines
    // and are not, which is why the parse is the thing that separates them.
    const ReadResult result = read("[ar:Un artiste]\n[ti:Un titre]\n\n[00:01.00]Une.\n");

    CHECK(result.header == "[ar:Un artiste]\n[ti:Un titre]");
    REQUIRE(result.subtitles.size() == 1);
}

TEST_CASE("a position before the origin carries its sign", "[format][lrc]") {
    // Gaupol's own pattern accepts the sign, and a subtitle shifted backwards
    // can legitimately land there.
    const ReadResult result = read("[-00:01.00]Avant le début.\n");

    REQUIRE(result.subtitles.size() == 1);
    CHECK(result.subtitles[0].start.milliseconds() == -1000);
}

TEST_CASE("a blank line between two lyrics is skipped, not reported", "[format][lrc]") {
    // Past the first timed line a blank is nothing, where before it it would
    // have been part of the header.
    const ReadResult result = read("[00:01.00]Une.\n\n[00:04.00]Deux.\n");

    REQUIRE(result.subtitles.size() == 2);
    REQUIRE(result.diagnostics.size() == 1);
    CHECK(result.diagnostics.front().kind == DiagnosticKind::DeducedEnds);
}

TEST_CASE("a stray line past the first subtitle is reported rather than kept", "[format][lrc]") {
    // Before the first timed line it would have been the header; after it,
    // there is nothing it can belong to.
    const ReadResult result = read("[00:01.00]Une.\nune ligne qui traîne\n");

    REQUIRE(result.subtitles.size() == 1);
    REQUIRE(result.diagnostics.size() == 2);
    CHECK(result.diagnostics[1].kind == DiagnosticKind::IgnoredLine);
    CHECK(result.diagnostics[1].line == 2);
}

TEST_CASE("an LRC file with no timed line is refused", "[format][lrc]") {
    const std::expected<ReadResult, ReadError> refused =
        LrcReader{}.read("[ar:Un artiste]\n[ti:Un titre]\n");

    REQUIRE_FALSE(refused.has_value());
    CHECK(refused.error().kind == ReadErrorKind::NoSubtitleFound);
}

TEST_CASE("a bracket that holds no time is not a timed line", "[format][lrc]") {
    for (const std::string_view line : {"[0:01.00]Une.\n",
                                        "[00:01]Une.\n",
                                        "[00:01.0]Une.\n",
                                        "[00:60.00]Une.\n",
                                        "[123456789:00.00]Une.\n",
                                        "[00:01.00Une.\n"}) {
        INFO("ligne : " << line);
        CHECK_FALSE(LrcReader{}.read(line).has_value());
    }
}

TEST_CASE("a position between two hundredths lands on one", "[format][lrc]") {
    CHECK(lrcTimeOf(Timestamp::fromMilliseconds(1000)) == "00:01.00");
    CHECK(lrcTimeOf(Timestamp::fromMilliseconds(1004)) == "00:01.00");
    CHECK(lrcTimeOf(Timestamp::fromMilliseconds(1005)) == "00:01.01");
    CHECK(lrcTimeOf(Timestamp::fromMilliseconds(-1005)) == "-00:01.01");
    CHECK(lrcTimeOf(Timestamp::fromMilliseconds(3723000)) == "62:03.00");
}

TEST_CASE("writing joins the lines of a subtitle with a space", "[format][lrc]") {
    // **The one loss of the nine that is visible to the naked eye.** It is one
    // way: nothing reading the result can tell a break was ever there.
    const std::array<Subtitle, 2> subtitles = {at(1000, 3000, "Une\nsur deux lignes."),
                                               at(4000, 6000, "Deux.")};

    const std::string written = LrcWriter{}.write(WriteRequest{.subtitles = subtitles});

    CHECK(written == "[00:01.00]Une sur deux lignes.\n[00:04.00]Deux.\n");
}

TEST_CASE("a header is written back, and a blank line after it", "[format][lrc]") {
    const std::array<Subtitle, 1> subtitles = {at(1000, 3000, "Une.")};

    const std::string written = LrcWriter{}.write(WriteRequest{
        .subtitles = subtitles,
        .header = "[ar:Un artiste]",
    });

    CHECK(written == "[ar:Un artiste]\n\n[00:01.00]Une.\n");
}

TEST_CASE("an LRC file comes back byte for byte", "[format][lrc][roundtrip]") {
    // **And on its own this proves nothing**, the writing putting no end back.
    // What it does prove is the header, the blank line under it, and the
    // hundredths.
    constexpr std::string_view kFile = "[ar:Un artiste]\n"
                                       "[ti:Un titre]\n"
                                       "\n"
                                       "[00:01.00]Une réplique.\n"
                                       "[00:04.25]Une autre.\n"
                                       "[62:03.00]Et la dernière.\n";

    const std::expected<ReadResult, ReadError> read = readSubtitles(kFile);
    REQUIRE(read.has_value());
    CHECK(read->format == SubtitleFormat::Lrc);

    const std::expected<std::string, WriteError> written =
        writeSubtitles(read->format,
                       WriteRequest{
                           .subtitles = read->subtitles,
                           .newline = read->newline,
                           .header = read->header,
                       });
    REQUIRE(written.has_value());
    CHECK(*written == kFile);
}
