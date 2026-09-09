// TMPlayer — the poorest of the nine, and one of the two that hold no end.
//
// One line per subtitle, `00:00:12:text`, counted in whole seconds. It has no
// vocabulary for italics, writes its breaks as `|`, and says one thing about
// itself that nothing else says: whether its hour is written on one digit or
// on two.

#include <subedit/core/format/deduced_ends.hpp>
#include <subedit/core/format/diagnostic.hpp>
#include <subedit/core/format/read_error.hpp>
#include <subedit/core/format/read_result.hpp>
#include <subedit/core/format/subtitle_file.hpp>
#include <subedit/core/format/subtitle_writer.hpp>
#include <subedit/core/format/tm_player_reader.hpp>
#include <subedit/core/format/tm_player_syntax.hpp>
#include <subedit/core/format/tm_player_writer.hpp>
#include <subedit/core/format/write_error.hpp>
#include <subedit/core/model/file_extras.hpp>
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
#include <variant>

namespace {

using subedit::core::DiagnosticKind;
using subedit::core::ReadError;
using subedit::core::ReadErrorKind;
using subedit::core::ReadResult;
using subedit::core::readSubtitles;
using subedit::core::Subtitle;
using subedit::core::SubtitleFormat;
using subedit::core::Timestamp;
using subedit::core::TMPlayerFile;
using subedit::core::TMPlayerReader;
using subedit::core::tmPlayerTimeOf;
using subedit::core::TMPlayerWriter;
using subedit::core::WriteError;
using subedit::core::WriteRequest;
using subedit::core::writeSubtitles;

[[nodiscard]] ReadResult read(std::string_view content) {
    const std::expected<ReadResult, ReadError> result = TMPlayerReader{}.read(content);
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

TEST_CASE("a TMPlayer line counts in whole seconds", "[format][tmplayer]") {
    const ReadResult result = read("00:00:01:Le vent se lève.\n00:01:05:Et retombe.\n");

    CHECK(result.format == SubtitleFormat::TMPlayer);
    REQUIRE(result.subtitles.size() == 2);
    CHECK(result.subtitles[0].start.milliseconds() == 1000);
    CHECK(result.subtitles[1].start.milliseconds() == 65000);
    CHECK(result.subtitles[0].mainText == "Le vent se lève.");
    CHECK(result.header.empty());
}

TEST_CASE("the ends are the next starts, and the last one gets five seconds",
          "[format][tmplayer][ends]") {
    // **The one thing worth asserting here**, and the reason the byte round
    // trip below proves nothing on its own: not one of these numbers is in the
    // file. ADR 0029.
    const ReadResult result = read("00:00:01:Une.\n00:00:04:Deux.\n00:00:08:Trois.\n");

    REQUIRE(result.subtitles.size() == 3);
    CHECK(result.subtitles[0].end.milliseconds() == 4000);
    CHECK(result.subtitles[1].end.milliseconds() == 8000);
    CHECK(result.subtitles[2].end.milliseconds() == 13000);
}

TEST_CASE("a reading that worked the ends out says so", "[format][tmplayer][ends]") {
    // Gaupol deduces in silence. A user looking at a filled End column would
    // otherwise have no way to know that none of it came from their file.
    const ReadResult result = read("00:00:01:Une.\n");

    REQUIRE_FALSE(result.diagnostics.empty());
    CHECK(result.diagnostics.front().kind == DiagnosticKind::DeducedEnds);
    CHECK(result.diagnostics.front().line == subedit::core::kWholeFile);
}

TEST_CASE("the shape of the hour is kept, and put back", "[format][tmplayer]") {
    // Both forms are TMPlayer, and nothing else in the file says which is in
    // use. Forgetting it would rewrite every line of the file — ADR 0030.
    const ReadResult one = read("0:00:01:Une.\n");
    const auto* shape = std::get_if<TMPlayerFile>(&one.extras);
    REQUIRE(shape != nullptr);
    CHECK_FALSE(shape->twoDigitHour);

    const ReadResult two = read("00:00:01:Une.\n");
    const auto* other = std::get_if<TMPlayerFile>(&two.extras);
    REQUIRE(other != nullptr);
    CHECK(other->twoDigitHour);
}

TEST_CASE("a position before the origin carries its sign", "[format][tmplayer]") {
    const ReadResult result = read("-0:00:01:Avant le début.\n");

    REQUIRE(result.subtitles.size() == 1);
    CHECK(result.subtitles[0].start.milliseconds() == -1000);
}

TEST_CASE("a pipe becomes a real line break", "[format][tmplayer]") {
    const ReadResult result = read("00:00:01:La côte est loin|et le vent tombe.\n");

    REQUIRE(result.subtitles.size() == 1);
    CHECK(result.subtitles[0].mainText == "La côte est loin\net le vent tombe.");
}

TEST_CASE("a line that is not a subtitle is reported", "[format][tmplayer]") {
    const ReadResult result = read("00:00:01:Une réplique.\nune ligne qui traîne\n");

    REQUIRE(result.subtitles.size() == 1);
    REQUIRE(result.diagnostics.size() == 2);
    // The deduced ends come first, being about the file; the stray line points
    // at a place in it.
    CHECK(result.diagnostics[1].kind == DiagnosticKind::IgnoredLine);
    CHECK(result.diagnostics[1].line == 2);
}

TEST_CASE("a TMPlayer file with no timed line is refused", "[format][tmplayer]") {
    const std::expected<ReadResult, ReadError> refused =
        TMPlayerReader{}.read("du texte\net rien\n");

    REQUIRE_FALSE(refused.has_value());
    CHECK(refused.error().kind == ReadErrorKind::NoSubtitleFound);
}

TEST_CASE("a shape that only looks like a timed line is not one", "[format][tmplayer]") {
    // Each of these opens like TMPlayer and is not: the fields are counted, and
    // a minute past fifty-nine is refused rather than folded.
    for (const std::string_view line : {"000:00:01:Une.\n",
                                        "00:0:01:Une.\n",
                                        "00.00:01:Une.\n",
                                        "00:00:01 Une.\n",
                                        "00:60:01:Une.\n",
                                        "00:00:61:Une.\n"}) {
        INFO("ligne : " << line);
        CHECK_FALSE(TMPlayerReader{}.read(line).has_value());
    }
}

TEST_CASE("a position between two seconds lands on one", "[format][tmplayer]") {
    // Halves away from zero, the rule the rest of the project follows, and no
    // decimal mark left behind by the digits that are not written.
    CHECK(tmPlayerTimeOf(Timestamp::fromMilliseconds(1499), true) == "00:00:01");
    CHECK(tmPlayerTimeOf(Timestamp::fromMilliseconds(1500), true) == "00:00:02");
    CHECK(tmPlayerTimeOf(Timestamp::fromMilliseconds(-1500), true) == "-00:00:02");
    CHECK(tmPlayerTimeOf(Timestamp::fromMilliseconds(3600000), false) == "1:00:00");
}

TEST_CASE("writing puts one subtitle on one line, and no end anywhere", "[format][tmplayer]") {
    const std::array<Subtitle, 2> subtitles = {at(1000, 3000, "Une\nsur deux lignes."),
                                               at(4000, 6000, "Deux.")};

    const std::string written = TMPlayerWriter{}.write(WriteRequest{.subtitles = subtitles});

    CHECK(written == "00:00:01:Une|sur deux lignes.\n00:00:04:Deux.\n");
}

TEST_CASE("a document that came from nowhere is written on two hour digits", "[format][tmplayer]") {
    // The commoner of the two forms, and the one a file that says nothing gets.
    const std::array<Subtitle, 1> subtitles = {at(1000, 3000, "Une.")};

    const std::string written = TMPlayerWriter{}.write(WriteRequest{.subtitles = subtitles});

    CHECK(written == "00:00:01:Une.\n");
}

TEST_CASE("a TMPlayer file comes back byte for byte", "[format][tmplayer][roundtrip]") {
    // **And on its own this proves nothing** — the writing puts no end back, so
    // the bytes would return whatever the reading had invented. What it does
    // prove is the rest: the hour shape, the pipes, the whole seconds.
    constexpr std::string_view kFile = "0:00:01:Une réplique|sur deux lignes.\n"
                                       "0:00:04:Une autre.\n"
                                       "0:01:05:Et la dernière.\n";

    const std::expected<ReadResult, ReadError> read = readSubtitles(kFile);
    REQUIRE(read.has_value());
    CHECK(read->format == SubtitleFormat::TMPlayer);

    const std::expected<std::string, WriteError> written =
        writeSubtitles(read->format,
                       WriteRequest{
                           .subtitles = read->subtitles,
                           .newline = read->newline,
                           .extras = read->extras,
                       });
    REQUIRE(written.has_value());
    CHECK(*written == kFile);
}
