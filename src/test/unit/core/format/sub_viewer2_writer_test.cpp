// Writing SubViewer 2.0.

#include <subedit/core/format/sub_viewer2_writer.hpp>
#include <subedit/core/format/subtitle_writer.hpp>
#include <subedit/core/model/source_file.hpp>
#include <subedit/core/model/subtitle.hpp>
#include <subedit/core/time/timestamp.hpp>

#include <catch2/catch_test_macros.hpp>

#include <array>
#include <string>
#include <vector>

namespace {

using subedit::core::Newline;
using subedit::core::Subtitle;
using subedit::core::SubViewer2Writer;
using subedit::core::Timestamp;
using subedit::core::WriteRequest;

[[nodiscard]] Subtitle at(std::int64_t start, std::int64_t end, std::string text) {
    return Subtitle{
        .start = Timestamp::fromMilliseconds(start),
        .end = Timestamp::fromMilliseconds(end),
        .mainText = std::move(text),
    };
}

} // namespace

TEST_CASE("a blank line opens each block, and none closes the file", "[format][subviewer2]") {
    const std::array<Subtitle, 2> subtitles = {at(1000, 3000, "Une."), at(4000, 6000, "Deux.")};

    const std::string written = SubViewer2Writer{}.write(WriteRequest{
        .subtitles = subtitles,
        .header = "[INFORMATION]\n[END INFORMATION]",
    });

    CHECK(written == "[INFORMATION]\n[END INFORMATION]\n"
                     "\n00:00:01.00,00:00:03.00\nUne.\n"
                     "\n00:00:04.00,00:00:06.00\nDeux.\n");
}

TEST_CASE("a document with no header gets the template one", "[format][subviewer2]") {
    // The promise `SubtitleWriter` makes for the formats that need a header. A
    // SubViewer 2 file without its `[INFORMATION]` block would still be read
    // back, but it would not look like one to anything else.
    const std::array<Subtitle, 1> subtitles = {at(1000, 3000, "Une.")};

    const std::string written = SubViewer2Writer{}.write(WriteRequest{.subtitles = subtitles});

    CHECK(written.starts_with("[INFORMATION]\n[TITLE]\n"));
    CHECK(written.contains("[COLF]&HFFFFFF,[STYLE]bd,[SIZE]18,[FONT]Sans\n"));
    CHECK(written.ends_with("\n00:00:01.00,00:00:03.00\nUne.\n"));
}

TEST_CASE("a line break is written as the marker the format has", "[format][subviewer2]") {
    const std::array<Subtitle, 1> subtitles = {
        at(1000, 3000, "La côte est loin\net le vent tombe.")};

    const std::string written =
        SubViewer2Writer{}.write(WriteRequest{.subtitles = subtitles, .header = "[SUBTITLE]"});

    CHECK(written.ends_with("La côte est loin[br]et le vent tombe.\n"));
}

TEST_CASE("a position finer than the hundredth is rounded, seconds included",
          "[format][subviewer2]") {
    // **The rounding is on the whole position.** 3 999 ms is four seconds at
    // this precision, and writing `00:00:03.100` would not be a timestamp.
    const std::array<Subtitle, 1> subtitles = {at(3999, 6004, "Une.")};

    const std::string written =
        SubViewer2Writer{}.write(WriteRequest{.subtitles = subtitles, .header = "[SUBTITLE]"});

    CHECK(written.contains("00:00:04.00,00:00:06.00\n"));
}

TEST_CASE("the line ending of the file is the one it asked for", "[format][subviewer2]") {
    const std::array<Subtitle, 1> subtitles = {at(1000, 3000, "Une.")};

    const std::string written = SubViewer2Writer{}.write(WriteRequest{
        .subtitles = subtitles,
        .newline = Newline::CrLf,
        .header = "[SUBTITLE]",
    });

    CHECK(written == "[SUBTITLE]\r\n\r\n00:00:01.00,00:00:03.00\r\nUne.\r\n");
}
