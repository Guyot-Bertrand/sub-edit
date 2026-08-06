#include <subedit/core/format/sub_rip_writer.hpp>
#include <subedit/core/format/subtitle_writer.hpp>
#include <subedit/core/model/document.hpp>
#include <subedit/core/model/format_extras.hpp>
#include <subedit/core/model/source_file.hpp>
#include <subedit/core/model/subtitle.hpp>
#include <subedit/core/time/timestamp.hpp>

#include <catch2/catch_test_macros.hpp>

#include <string>
#include <vector>

namespace {

using subedit::core::Document;
using subedit::core::Newline;
using subedit::core::Rectangle;
using subedit::core::SubRipExtras;
using subedit::core::SubRipWriter;
using subedit::core::Subtitle;
using subedit::core::Timestamp;
using subedit::core::WriteRequest;

Subtitle at(int startMs, int endMs, std::string text) {
    return Subtitle{
        .start = Timestamp::fromMilliseconds(startMs),
        .end = Timestamp::fromMilliseconds(endMs),
        .mainText = std::move(text),
    };
}

std::string written(const std::vector<Subtitle>& subtitles,
                    Document document = Document::Main,
                    Newline newline = Newline::Lf) {
    const SubRipWriter writer;
    return writer.write(WriteRequest{
        .subtitles = subtitles,
        .document = document,
        .newline = newline,
    });
}

} // namespace

TEST_CASE("a subtitle is written as number, timestamps, text and a blank line",
          "[format][subrip]") {
    const std::vector<Subtitle> subtitles = {at(1000, 2500, "Bonjour.")};

    CHECK(written(subtitles) == "1\n00:00:01,000 --> 00:00:02,500\nBonjour.\n\n");
}

TEST_CASE("the decimal mark of SubRip is the comma", "[format][subrip]") {
    const std::vector<Subtitle> subtitles = {at(1500, 2000, "Bonjour.")};

    CHECK(written(subtitles).contains("00:00:01,500"));
}

TEST_CASE("the numbering is regenerated, whatever the file said", "[format][subrip]") {
    // Which is why a file whose numbers do not follow heals itself by being
    // opened and saved.
    const std::vector<Subtitle> subtitles = {
        at(1000, 2000, "Un."),
        at(3000, 4000, "Deux."),
        at(5000, 6000, "Trois."),
    };

    const std::string text = written(subtitles);

    CHECK(text.starts_with("1\n"));
    CHECK(text.contains("\n2\n"));
    CHECK(text.contains("\n3\n"));
}

TEST_CASE("several lines of text stay several lines", "[format][subrip]") {
    const std::vector<Subtitle> subtitles = {at(1000, 2000, "Première\nDeuxième")};

    CHECK(written(subtitles) == "1\n00:00:01,000 --> 00:00:02,000\nPremière\nDeuxième\n\n");
}

TEST_CASE("the translation is written when it is the one asked for", "[format][subrip]") {
    std::vector<Subtitle> subtitles = {at(1000, 2000, "Bonjour.")};
    subtitles[0].translationText = "Hello.";

    CHECK(written(subtitles, Document::Translation).contains("Hello."));
    CHECK_FALSE(written(subtitles, Document::Translation).contains("Bonjour."));
}

TEST_CASE("the extended coordinates follow the timestamps when they exist", "[format][subrip]") {
    std::vector<Subtitle> subtitles = {at(1000, 2000, "Bonjour.")};
    subtitles[0].extras = SubRipExtras{
        .coordinates = Rectangle{.x1 = 40, .x2 = 600, .y1 = 20, .y2 = 460},
    };

    CHECK(written(subtitles).contains("00:00:02,000  X1:040 X2:600 Y1:020 Y2:460\n"));
}

TEST_CASE("coordinates all at zero are not written", "[format][subrip]") {
    // Gaupol leaves them out, and a file that never had them must not gain
    // them by being saved.
    std::vector<Subtitle> subtitles = {at(1000, 2000, "Bonjour.")};
    subtitles[0].extras = SubRipExtras{.coordinates = Rectangle{}};

    CHECK_FALSE(written(subtitles).contains("X1:"));
}

TEST_CASE("absent coordinates are not written", "[format][subrip]") {
    std::vector<Subtitle> subtitles = {at(1000, 2000, "Bonjour.")};
    subtitles[0].extras = SubRipExtras{};

    CHECK_FALSE(written(subtitles).contains("X1:"));
}

TEST_CASE("a position beyond ninety-nine hours saturates on writing", "[format][subrip]") {
    // A constraint of the format, not of the model: the position itself is
    // untouched.
    const std::vector<Subtitle> subtitles = {at(360000000, 360000000, "Loin.")};

    CHECK(written(subtitles).contains("99:59:59,999 --> 99:59:59,999"));
}

TEST_CASE("the line ending is the one asked for", "[format][subrip]") {
    const std::vector<Subtitle> subtitles = {at(1000, 2000, "Bonjour.")};

    CHECK(written(subtitles, Document::Main, Newline::CrLf) ==
          "1\r\n00:00:01,000 --> 00:00:02,000\r\nBonjour.\r\n\r\n");
}

TEST_CASE("no subtitle gives no file", "[format][subrip]") {
    CHECK(written({}).empty());
}
