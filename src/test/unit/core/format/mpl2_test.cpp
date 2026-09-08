// MPL2 — the simplest of the nine, and one of the two hiding behind a `.txt`.
//
// No header, no numbering, one line per subtitle. The tenth of a second is the
// coarsest grain of any of the time formats, and it is the whole of what this
// format loses.

#include <subedit/core/format/mpl2_reader.hpp>
#include <subedit/core/format/mpl2_syntax.hpp>
#include <subedit/core/format/mpl2_writer.hpp>
#include <subedit/core/format/read_error.hpp>
#include <subedit/core/format/read_result.hpp>
#include <subedit/core/format/subtitle_file.hpp>
#include <subedit/core/format/subtitle_writer.hpp>
#include <subedit/core/format/write_error.hpp>
#include <subedit/core/model/subtitle.hpp>
#include <subedit/core/model/subtitle_format.hpp>
#include <subedit/core/time/timestamp.hpp>

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <array>
#include <cstdint>
#include <expected>
#include <string>
#include <string_view>

namespace {

using subedit::core::DiagnosticKind;
using subedit::core::Mpl2Reader;
using subedit::core::Mpl2Writer;
using subedit::core::ReadError;
using subedit::core::ReadErrorKind;
using subedit::core::ReadResult;
using subedit::core::readSubtitles;
using subedit::core::Subtitle;
using subedit::core::SubtitleFormat;
using subedit::core::tenthsOf;
using subedit::core::Timestamp;
using subedit::core::WriteError;
using subedit::core::WriteRequest;
using subedit::core::writeSubtitles;

[[nodiscard]] ReadResult read(std::string_view content) {
    const std::expected<ReadResult, ReadError> result = Mpl2Reader{}.read(content);
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

TEST_CASE("an MPL2 line counts in tenths of a second", "[format][mpl2]") {
    const ReadResult result = read("[10][30]Le vent se lève.\n[45][60]Et retombe.\n");

    CHECK(result.format == SubtitleFormat::Mpl2);
    REQUIRE(result.subtitles.size() == 2);
    CHECK(result.subtitles[0].start.milliseconds() == 1000);
    CHECK(result.subtitles[0].end.milliseconds() == 3000);
    CHECK(result.subtitles[1].start.milliseconds() == 4500);
    CHECK(result.subtitles[0].mainText == "Le vent se lève.");
    CHECK(result.diagnostics.empty());
    CHECK(result.header.empty());
}

TEST_CASE("a position before the origin carries its sign", "[format][mpl2]") {
    // Gaupol's own sample opens on two of them.
    const ReadResult result = read("[-68][-28]Avant le début.\n");

    REQUIRE(result.subtitles.size() == 1);
    CHECK(result.subtitles[0].start.milliseconds() == -6800);
    CHECK(result.subtitles[0].end.milliseconds() == -2800);
}

TEST_CASE("a pipe becomes a real line break, and the italic stays as it is", "[format][mpl2]") {
    // The `/` opening a line is how this format writes italics. ADR 0009: the
    // text is the raw string, tags included, and nothing decodes it here.
    const ReadResult result = read("[10][30]/La côte est loin|/et le vent tombe.\n");

    REQUIRE(result.subtitles.size() == 1);
    CHECK(result.subtitles[0].mainText == "/La côte est loin\n/et le vent tombe.");
}

TEST_CASE("a line that is not a subtitle is reported", "[format][mpl2]") {
    // One line is one subtitle, so a line that is not one belongs to nothing.
    const ReadResult result = read("[10][30]Une réplique.\nune ligne qui traîne\n");

    REQUIRE(result.subtitles.size() == 1);
    REQUIRE_FALSE(result.diagnostics.empty());
    CHECK(result.diagnostics[0].kind == DiagnosticKind::IgnoredLine);
    CHECK(result.diagnostics[0].line == 2);
}

TEST_CASE("a file with no bracketed line is refused", "[format][mpl2]") {
    const std::expected<ReadResult, ReadError> refused = Mpl2Reader{}.read("du texte\net rien\n");

    REQUIRE_FALSE(refused.has_value());
    CHECK(refused.error().kind == ReadErrorKind::NoSubtitleFound);
}

TEST_CASE("brackets that hold no number, or no second pair, are not a subtitle", "[format][mpl2]") {
    // Each of these opens like MPL2 and is not: the shape is checked whole.
    for (const std::string_view line : {"[][30]Une.\n",
                                        "[10]Une.\n",
                                        "[10][30Une.\n",
                                        "[10)[30]Une.\n",
                                        "[12345678901][30]Une.\n"}) {
        INFO("ligne : " << line);
        CHECK_FALSE(Mpl2Reader{}.read(line).has_value());
    }
}

TEST_CASE("a position between two tenths lands on one", "[format][mpl2]") {
    // Halves away from zero, the rule the rest of the project follows.
    CHECK(tenthsOf(Timestamp::fromMilliseconds(1000)) == "10");
    CHECK(tenthsOf(Timestamp::fromMilliseconds(1049)) == "10");
    CHECK(tenthsOf(Timestamp::fromMilliseconds(1050)) == "11");
    CHECK(tenthsOf(Timestamp::fromMilliseconds(-1050)) == "-11");
}

TEST_CASE("writing puts one subtitle on one line, and no separator between them",
          "[format][mpl2]") {
    const std::array<Subtitle, 2> subtitles = {at(1000, 3000, "Une\nsur deux lignes."),
                                               at(4000, 6000, "Deux.")};

    const std::string written = Mpl2Writer{}.write(WriteRequest{.subtitles = subtitles});

    CHECK(written == "[10][30]Une|sur deux lignes.\n[40][60]Deux.\n");
}

TEST_CASE("an MPL2 file comes back byte for byte", "[format][mpl2][roundtrip]") {
    constexpr std::string_view kFile = "[-68][-28]Avant le début|sur deux lignes.\n"
                                       "[10][30]/Une réplique en italique.\n"
                                       "[45][60]Une autre, avec une virgule.\n";

    const std::expected<ReadResult, ReadError> read = readSubtitles(kFile);
    REQUIRE(read.has_value());
    CHECK(read->format == SubtitleFormat::Mpl2);

    const std::expected<std::string, WriteError> written = writeSubtitles(
        read->format, WriteRequest{.subtitles = read->subtitles, .newline = read->newline});
    REQUIRE(written.has_value());
    CHECK(*written == kFile);
}
