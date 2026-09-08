// Writing Sub Station Alpha and Advanced SSA.

#include <subedit/core/format/sub_station_alpha_writer.hpp>
#include <subedit/core/format/subtitle_writer.hpp>
#include <subedit/core/model/file_extras.hpp>
#include <subedit/core/model/format_extras.hpp>
#include <subedit/core/model/subtitle.hpp>
#include <subedit/core/model/subtitle_format.hpp>
#include <subedit/core/time/timestamp.hpp>

#include <catch2/catch_test_macros.hpp>

#include <array>
#include <cstdint>
#include <string>
#include <vector>

namespace {

using subedit::core::SubStationAlphaExtras;
using subedit::core::SubStationAlphaFile;
using subedit::core::SubStationAlphaWriter;
using subedit::core::Subtitle;
using subedit::core::SubtitleFormat;
using subedit::core::Timestamp;
using subedit::core::WriteRequest;

[[nodiscard]] Subtitle at(std::int64_t start, std::int64_t end, std::string text) {
    return Subtitle{
        .start = Timestamp::fromMilliseconds(start),
        .end = Timestamp::fromMilliseconds(end),
        .mainText = std::move(text),
    };
}

[[nodiscard]] std::vector<std::string> columns() {
    return {"Marked",
            "Start",
            "End",
            "Style",
            "Name",
            "MarginL",
            "MarginR",
            "MarginV",
            "Effect",
            "Text"};
}

} // namespace

TEST_CASE("the events section follows the header, one blank line apart", "[format][ssa]") {
    const std::array<Subtitle, 1> subtitles = {at(1000, 3000, "Bonjour.")};

    const std::string written =
        SubStationAlphaWriter{SubtitleFormat::SubStationAlpha}.write(WriteRequest{
            .subtitles = subtitles,
            .header = "[Script Info]\nScriptType: v4.00",
            .extras = SubStationAlphaFile{.eventFields = columns()},
        });

    CHECK(written ==
          "[Script Info]\nScriptType: v4.00\n"
          "\n"
          "[Events]\n"
          "Format: Marked, Start, End, Style, Name, MarginL, MarginR, MarginV, Effect, Text\n"
          "Dialogue: Marked=0,0:00:01.00,0:00:03.00,Default,,0000,0000,0000,,Bonjour.\n");
}

TEST_CASE("a subtitle from another format is written under the style every file has",
          "[format][ssa]") {
    // `Default` and not empty: a Sub Station Alpha file declares a style by
    // that name, so a subtitle that came from a SubRip has somewhere to point.
    const std::array<Subtitle, 1> subtitles = {at(1000, 3000, "Bonjour.")};

    const std::string written =
        SubStationAlphaWriter{SubtitleFormat::SubStationAlpha}.write(WriteRequest{
            .subtitles = subtitles,
            .header = "[Script Info]",
            .extras = SubStationAlphaFile{.eventFields = columns()},
        });

    CHECK(written.contains(",Default,,0000,0000,0000,,Bonjour.\n"));
}

TEST_CASE("a document with no header gets the one its format is recognised by", "[format][ssa]") {
    // The detection reads `ScriptType`. A file without it is not this format at
    // all, so it would not come back.
    const std::array<Subtitle, 1> subtitles = {at(1000, 3000, "Bonjour.")};

    const std::string ssa = SubStationAlphaWriter{SubtitleFormat::SubStationAlpha}.write(
        WriteRequest{.subtitles = subtitles});
    const std::string ass = SubStationAlphaWriter{SubtitleFormat::AdvancedSubStationAlpha}.write(
        WriteRequest{.subtitles = subtitles});

    CHECK(ssa.contains("ScriptType: v4.00\n"));
    CHECK(ssa.contains("[V4 Styles]"));
    CHECK(ssa.contains("Format: Marked, Start"));

    CHECK(ass.contains("ScriptType: v4.00+\n"));
    CHECK(ass.contains("[V4+ Styles]"));
    CHECK(ass.contains("Format: Layer, Start"));
}

TEST_CASE("the columns are written in the order the file declared", "[format][ssa]") {
    const std::array<Subtitle, 1> subtitles = {at(1000, 3000, "Bonjour.")};

    const std::string written =
        SubStationAlphaWriter{SubtitleFormat::SubStationAlpha}.write(WriteRequest{
            .subtitles = subtitles,
            .header = "[Script Info]",
            .extras = SubStationAlphaFile{.eventFields = {"Start", "End", "Text"}},
        });

    CHECK(written.contains("Format: Start, End, Text\n"));
    CHECK(written.contains("Dialogue: 0:00:01.00,0:00:03.00,Bonjour.\n"));
}

TEST_CASE("an hour is written without padding, and keeps its tens", "[format][ssa]") {
    // Gaupol cuts the first character off a padded string, which would eat the
    // tens digit of a file past ten hours. Writing the count says the same
    // thing for the hours a file really has, and keeps saying it beyond.
    constexpr std::int64_t kTwelveHours = std::int64_t{12} * 60 * 60 * 1000;
    const std::array<Subtitle, 2> subtitles = {
        at(1000, 3000, "Tôt."),
        at(kTwelveHours, kTwelveHours + 2000, "Tard."),
    };

    const std::string written =
        SubStationAlphaWriter{SubtitleFormat::SubStationAlpha}.write(WriteRequest{
            .subtitles = subtitles,
            .header = "[Script Info]",
            .extras = SubStationAlphaFile{.eventFields = {"Start", "End", "Text"}},
        });

    CHECK(written.contains("0:00:01.00,0:00:03.00,Tôt.\n"));
    CHECK(written.contains("12:00:00.00,12:00:02.00,Tard.\n"));
}

TEST_CASE("a line break is written as the marker the format has", "[format][ssa]") {
    const std::array<Subtitle, 1> subtitles = {at(1000, 3000, "Une ligne\nune autre.")};

    const std::string written =
        SubStationAlphaWriter{SubtitleFormat::SubStationAlpha}.write(WriteRequest{
            .subtitles = subtitles,
            .header = "[Script Info]",
            .extras = SubStationAlphaFile{.eventFields = {"Start", "End", "Text"}},
        });

    CHECK(written.contains("Une ligne\\Nune autre.\n"));
}

TEST_CASE("what a subtitle carries of this format is written back", "[format][ssa]") {
    std::array<Subtitle, 1> subtitles = {at(1000, 3000, "Bonjour.")};
    subtitles[0].extras = SubStationAlphaExtras{
        .marked = 1,
        .style = "Titre",
        .name = "Marie",
        .marginLeft = 30,
        .marginVertical = 10,
        .effect = "fade",
    };

    const std::string written =
        SubStationAlphaWriter{SubtitleFormat::SubStationAlpha}.write(WriteRequest{
            .subtitles = subtitles,
            .header = "[Script Info]",
            .extras = SubStationAlphaFile{.eventFields = columns()},
        });

    CHECK(written.contains(
        "Dialogue: Marked=1,0:00:01.00,0:00:03.00,Titre,Marie,0030,0000,0010,fade,Bonjour.\n"));
}
