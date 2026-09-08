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

TEST_CASE("a SubViewer 2 file is recognised by its timestamp line", "[format][detection]") {
    // **The header is not what settles it.** A header is a promise a file makes
    // about itself; a timestamp line is the format being spoken, and it is what
    // Gaupol looks for too.
    CHECK(detectFormat("[INFORMATION]\n[END INFORMATION]\n"
                       "\n00:00:01.00,00:00:03.00\nUne réplique.\n") == SubtitleFormat::SubViewer2);
    CHECK(detectFormat("00:00:01.00,00:00:03.00\nUne réplique.\n") == SubtitleFormat::SubViewer2);
}

TEST_CASE("a bracketed header alone claims nothing", "[format][detection]") {
    // Nothing rather than a guess: the sections could open anything, and this
    // one has no subtitle in it.
    CHECK_FALSE(detectFormat("[INFORMATION]\n[TITLE]Le port\n[END INFORMATION]\n").has_value());
}

TEST_CASE("the two formats written to the thousandth are not taken for SubViewer 2",
          "[format][detection]") {
    // The separator and the number of decimals are what tell them apart, and
    // the check on the shape is what keeps a permissive timestamp reader from
    // blurring the three.
    CHECK(detectFormat("1\n00:00:01,000 --> 00:00:03,000\nUne.\n") == SubtitleFormat::SubRip);
    CHECK(detectFormat("WEBVTT\n\n00:01.000 --> 00:03.000\nUne.\n") == SubtitleFormat::WebVtt);
}

TEST_CASE("the two Sub Station Alpha formats are told apart by their version",
          "[format][detection]") {
    // **The `+` is the whole difference**, and Gaupol settles it the same way.
    // A declaration beats a shape: a file saying what it is answers the
    // question, where a line written a certain way only suggests it.
    CHECK(detectFormat("[Script Info]\nScriptType: v4.00\n"
                       "[Events]\nDialogue: Marked=0,0:00:01.00,0:00:03.00,Une.\n") ==
          SubtitleFormat::SubStationAlpha);
    CHECK(detectFormat("[Script Info]\nScriptType: v4.00+\n"
                       "[Events]\nDialogue: 0,0:00:01.00,0:00:03.00,Une.\n") ==
          SubtitleFormat::AdvancedSubStationAlpha);

    // Case and blanks around the version are what real files vary on.
    CHECK(detectFormat("ScriptType:   V4.00+  \n") == SubtitleFormat::AdvancedSubStationAlpha);
}

TEST_CASE("a script type of another version claims nothing", "[format][detection]") {
    CHECK_FALSE(detectFormat("[Script Info]\nScriptType: v3.00\n").has_value());
}

TEST_CASE("MPL2 is recognised by two bracketed numbers opening a line", "[format][detection]") {
    CHECK(detectFormat("[10][30]Une réplique.\n") == SubtitleFormat::Mpl2);

    // The three that open on a bracket or a brace and are not MPL2. Only the
    // first two exist as formats today; the third is what MicroDVD looks like,
    // and it must not be claimed by anyone before its reader lands.
    CHECK_FALSE(detectFormat("[00:12.34]Une réplique.\n").has_value());
    CHECK_FALSE(detectFormat("[INFORMATION]\n[TITLE]Le port\n").has_value());
    CHECK_FALSE(detectFormat("{25}{75}Une réplique.\n").has_value());
}
