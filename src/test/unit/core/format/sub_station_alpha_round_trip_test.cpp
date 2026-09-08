// A Sub Station Alpha file read and written back is the same file.

#include <subedit/core/format/read_error.hpp>
#include <subedit/core/format/read_result.hpp>
#include <subedit/core/format/subtitle_file.hpp>
#include <subedit/core/format/subtitle_writer.hpp>
#include <subedit/core/format/write_error.hpp>
#include <subedit/core/model/subtitle_format.hpp>

#include <catch2/catch_test_macros.hpp>

#include <expected>
#include <string>
#include <string_view>

namespace {

using subedit::core::ReadError;
using subedit::core::ReadResult;
using subedit::core::readSubtitles;
using subedit::core::SubtitleFormat;
using subedit::core::WriteError;
using subedit::core::WriteRequest;
using subedit::core::writeSubtitles;

[[nodiscard]] std::string roundTrip(std::string_view original) {
    const std::expected<ReadResult, ReadError> read = readSubtitles(original);
    if (!read.has_value()) {
        FAIL("the file was meant to open");
        return {};
    }

    const std::expected<std::string, WriteError> written =
        writeSubtitles(read->format,
                       WriteRequest{
                           .subtitles = read->subtitles,
                           .newline = read->newline,
                           .encoding = read->encoding,
                           .header = read->header,
                           .extras = read->extras,
                       });
    if (!written.has_value()) {
        FAIL("the writing was meant to succeed");
        return {};
    }
    return *written;
}

constexpr std::string_view kSsaFile =
    "[Script Info]\n"
    "Title: Le port\n"
    "ScriptType: v4.00\n"
    "Collisions: Normal\n"
    "\n"
    "[V4 Styles]\n"
    "Format: Name, Fontname, Fontsize\n"
    "Style: Default,Sans,18\n"
    "\n"
    "[Events]\n"
    "Format: Marked, Start, End, Style, Name, MarginL, MarginR, MarginV, Effect, Text\n"
    "Dialogue: Marked=0,0:00:01.00,0:00:03.00,Default,,0000,0000,0000,,"
    "Le vent se lève\\Nsur le port.\n"
    "Dialogue: Marked=1,0:00:04.50,0:00:06.25,Titre,Marie,0030,0040,0010,fade,"
    "{\\i1}Oui, bien sûr.{\\i0}\n";

constexpr std::string_view kAssFile =
    "[Script Info]\n"
    "ScriptType: v4.00+\n"
    "\n"
    "[V4+ Styles]\n"
    "Format: Name, Fontname\n"
    "Style: Default,Sans\n"
    "\n"
    "[Events]\n"
    "Format: Layer, Start, End, Style, Name, MarginL, MarginR, MarginV, Effect, Text\n"
    "Dialogue: 2,0:00:01.00,0:00:03.00,Default,,0000,0000,0000,,Une réplique.\n";

} // namespace

TEST_CASE("a Sub Station Alpha file comes back byte for byte", "[format][ssa][roundtrip]") {
    // A header of two sections, a column order, a mark, margins, an effect, a
    // break marker, a tag and a comma inside the text — each of them is
    // something the reading had to keep rather than understand.
    CHECK(roundTrip(kSsaFile) == kSsaFile);
}

TEST_CASE("an Advanced SSA file comes back byte for byte", "[format][ssa][roundtrip]") {
    CHECK(roundTrip(kAssFile) == kAssFile);
}

TEST_CASE("the two formats are told apart by the line that declares them",
          "[format][ssa][roundtrip]") {
    const std::expected<ReadResult, ReadError> ssa = readSubtitles(kSsaFile);
    const std::expected<ReadResult, ReadError> ass = readSubtitles(kAssFile);

    REQUIRE(ssa.has_value());
    REQUIRE(ass.has_value());
    CHECK(ssa->format == SubtitleFormat::SubStationAlpha);
    CHECK(ass->format == SubtitleFormat::AdvancedSubStationAlpha);
}
