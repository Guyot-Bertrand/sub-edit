#include <subedit/core/format/format_detection.hpp>
#include <subedit/core/model/subtitle_format.hpp>

#include <catch2/catch_test_macros.hpp>

#include <optional>
#include <string_view>

namespace {

using subedit::core::detectFormat;
using subedit::core::SubtitleFormat;

} // namespace

TEST_CASE("a SubRip file is recognised by its timestamps", "[format][detection]") {
    CHECK(detectFormat("1\n"
                       "00:00:01,000 --> 00:00:02,000\n"
                       "Bonjour.\n") == SubtitleFormat::SubRip);
}

TEST_CASE("a SubRip file without its numbering is still recognised", "[format][detection]") {
    CHECK(detectFormat("00:00:01,000 --> 00:00:02,000\nBonjour.\n") == SubtitleFormat::SubRip);
}

TEST_CASE("a WebVTT file is recognised by its signature", "[format][detection]") {
    CHECK(detectFormat("WEBVTT\n"
                       "\n"
                       "00:01.000 --> 00:02.000\n"
                       "Bonjour.\n") == SubtitleFormat::WebVtt);
}

TEST_CASE("the signature wins over anything that follows", "[format][detection]") {
    // The most specific format wins: a WebVTT file whose timestamps were
    // written with commas is a malformed WebVTT file, not a SubRip one.
    CHECK(detectFormat("WEBVTT\n"
                       "\n"
                       "00:00:01,000 --> 00:00:02,000\n"
                       "Bonjour.\n") == SubtitleFormat::WebVtt);
}

TEST_CASE("blank lines before the signature do not hide it", "[format][detection]") {
    CHECK(detectFormat("\n\nWEBVTT\n\n00:01.000 --> 00:02.000\nBonjour.\n") ==
          SubtitleFormat::WebVtt);
}

TEST_CASE("the signature is uppercase, as the format requires", "[format][detection]") {
    // Deliberately strict, and consistent with the reader: recognising
    // « webvtt » would promise a format the reader then refuses, and produce a
    // file no browser accepts.
    CHECK(detectFormat("webvtt\n\n00:01.000 --> 00:02.000\nBonjour.\n") == std::nullopt);
}

TEST_CASE("timestamps with a period and no signature are not guessed", "[format][detection]") {
    // Neither format claims it: not WebVTT for want of a signature, not SubRip
    // for want of a comma. Refusing beats picking one at random.
    CHECK(detectFormat("00:00:01.000 --> 00:00:02.000\nBonjour.\n") == std::nullopt);
}

TEST_CASE("an arrow between two things that are not times proves nothing", "[format][detection]") {
    // A comma and an arrow are not enough; the fields on either side have to
    // read as timestamps, or a line of prose would pass for a subtitle file.
    CHECK(detectFormat("avant, ici --> après, là\n") == std::nullopt);
}

TEST_CASE("a file of anything else is not a subtitle file", "[format][detection]") {
    CHECK(detectFormat("du texte ordinaire\nsur deux lignes\n") == std::nullopt);
    CHECK(detectFormat("") == std::nullopt);
}
