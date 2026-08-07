#include <subedit/core/format/subtitle_writer.hpp>
#include <subedit/core/format/web_vtt_writer.hpp>
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
using subedit::core::Subtitle;
using subedit::core::Timestamp;
using subedit::core::WebVttExtras;
using subedit::core::WebVttWriter;
using subedit::core::WriteRequest;

Subtitle at(int startMs, int endMs, std::string text) {
    return Subtitle{
        .start = Timestamp::fromMilliseconds(startMs),
        .end = Timestamp::fromMilliseconds(endMs),
        .mainText = std::move(text),
    };
}

std::string written(const std::vector<Subtitle>& subtitles,
                    std::string_view header = {},
                    Document document = Document::Main,
                    Newline newline = Newline::Lf) {
    const WebVttWriter writer;
    return writer.write(WriteRequest{
        .subtitles = subtitles,
        .document = document,
        .newline = newline,
        .header = header,
    });
}

} // namespace

TEST_CASE("a file begins with the signature", "[format][webvtt]") {
    const std::vector<Subtitle> subtitles = {at(1000, 2000, "Bonjour.")};

    CHECK(written(subtitles) == "WEBVTT\n\n00:01.000 --> 00:02.000\nBonjour.\n");
}

TEST_CASE("the header of the file read is put back", "[format][webvtt]") {
    const std::vector<Subtitle> subtitles = {at(1000, 2000, "Bonjour.")};

    CHECK(written(subtitles, "WEBVTT - Dialogue").starts_with("WEBVTT - Dialogue\n"));
}

TEST_CASE("the decimal mark of WebVTT is the period", "[format][webvtt]") {
    const std::vector<Subtitle> subtitles = {at(1500, 2000, "Bonjour.")};

    CHECK(written(subtitles).contains("00:01.500"));
}

TEST_CASE("the hours are left out when no cue reaches one hour", "[format][webvtt]") {
    const std::vector<Subtitle> subtitles = {at(1000, 2000, "Bonjour.")};

    CHECK(written(subtitles).contains("00:01.000 --> 00:02.000"));
}

TEST_CASE("the hours come back as soon as one cue reaches an hour", "[format][webvtt]") {
    // All the timestamps of a file are written the same way; mixing the two
    // shapes within one file would be legal but unreadable.
    const std::vector<Subtitle> subtitles = {
        at(1000, 2000, "Court."),
        at(3600000, 3601000, "Long."),
    };

    const std::string text = written(subtitles);

    CHECK(text.contains("00:00:01.000 --> 00:00:02.000"));
    CHECK(text.contains("01:00:00.000 --> 01:00:01.000"));
}

TEST_CASE("the decision looks at every cue, not only the last", "[format][webvtt]") {
    // Gaupol looks at the last subtitle, which assumes the file is sorted. A
    // file that is not would come out with minutes past fifty-nine.
    const std::vector<Subtitle> subtitles = {
        at(3600000, 3601000, "Long, et placé en premier."),
        at(1000, 2000, "Court."),
    };

    CHECK(written(subtitles).contains("01:00:00.000"));
    CHECK(written(subtitles).contains("00:00:01.000"));
}

TEST_CASE("the cue identifier is written above the timestamps", "[format][webvtt]") {
    std::vector<Subtitle> subtitles = {at(1000, 2000, "Bonjour.")};
    subtitles[0].extras = WebVttExtras{.id = "chapitre-1"};

    CHECK(written(subtitles).contains("\nchapitre-1\n00:01.000 --> 00:02.000\n"));
}

TEST_CASE("the settings are written on the timestamp line", "[format][webvtt]") {
    std::vector<Subtitle> subtitles = {at(1000, 2000, "Bonjour.")};
    subtitles[0].extras = WebVttExtras{.settings = "align:start position:10%"};

    CHECK(written(subtitles).contains("00:01.000 --> 00:02.000 align:start position:10%\n"));
}

TEST_CASE("a style block is written before the cue it belongs to", "[format][webvtt]") {
    std::vector<Subtitle> subtitles = {at(1000, 2000, "Bonjour.")};
    subtitles[0].extras = WebVttExtras{.style = "STYLE\n::cue { color: yellow }"};

    CHECK(written(subtitles) == "WEBVTT\n"
                                "\n"
                                "STYLE\n"
                                "::cue { color: yellow }\n"
                                "\n"
                                "00:01.000 --> 00:02.000\n"
                                "Bonjour.\n");
}

TEST_CASE("a comment block is written before the cue it belongs to", "[format][webvtt]") {
    std::vector<Subtitle> subtitles = {at(1000, 2000, "Bonjour.")};
    subtitles[0].extras = WebVttExtras{.comment = "NOTE à revoir"};

    CHECK(written(subtitles).contains("\nNOTE à revoir\n\n00:01.000"));
}

TEST_CASE("the tags of the text are written as they stand", "[format][webvtt]") {
    const std::vector<Subtitle> subtitles = {at(1000, 2000, "<v Marie>Bonjour.</v>")};

    CHECK(written(subtitles).contains("<v Marie>Bonjour.</v>"));
}

TEST_CASE("the translation is written when it is the one asked for", "[format][webvtt]") {
    std::vector<Subtitle> subtitles = {at(1000, 2000, "Bonjour.")};
    subtitles[0].translationText = "Hello.";

    CHECK(written(subtitles, {}, Document::Translation).contains("Hello."));
}

TEST_CASE("the line ending is the one asked for", "[format][webvtt]") {
    const std::vector<Subtitle> subtitles = {at(1000, 2000, "Bonjour.")};

    CHECK(written(subtitles, {}, Document::Main, Newline::CrLf) ==
          "WEBVTT\r\n\r\n00:01.000 --> 00:02.000\r\nBonjour.\r\n");
}

TEST_CASE("a file without a single cue is still a WebVTT file", "[format][webvtt]") {
    // The signature alone is a valid file, and writing nothing at all would
    // produce something no reader would accept.
    CHECK(written({}) == "WEBVTT\n");
}
