// What the overlay of the video is handed — issue #408, GUI-REPLICA-01.
//
// A subtitle is held as its file wrote it, and the picture used to draw that text as it stood: a
// `<i>` was drawn as a `<i>`. These cases say what `replicaOf` makes of it instead — the tags
// understood through the pivot of ADR 0031 and written in the Sub Station Alpha vocabulary, and
// **a tag with no equivalent on screen removed, never drawn as it stands**.

#include <subedit/core/model/subtitle_format.hpp>
#include <subedit/core/video/replica.hpp>

#include <catch2/catch_test_macros.hpp>

#include <string>

namespace {

using subedit::core::replicaOf;
using subedit::core::SubtitleFormat;

} // namespace

TEST_CASE("a text with no tag is left as it is", "[video][replica][GUI-REPLICA-01]") {
    CHECK(replicaOf("Un.", SubtitleFormat::SubRip) == "Un.");
    CHECK(replicaOf("Un.\nDeux.", SubtitleFormat::SubRip) == "Un.\nDeux.");
    CHECK(replicaOf("", SubtitleFormat::SubRip).empty());
}

// The case the issue opens with: the picture showed `<i>` and `</i>` as letters.
TEST_CASE("an italic is drawn italic, and its tags are not drawn",
          "[video][replica][GUI-REPLICA-01]") {
    const std::string drawn = replicaOf("Il a dit <i>non</i>.", SubtitleFormat::SubRip);

    CHECK(drawn == "Il a dit {\\i1}non{\\i0}.");
    CHECK(drawn.find('<') == std::string::npos);
}

TEST_CASE("bold, underline and colour are drawn too", "[video][replica][GUI-REPLICA-01]") {
    CHECK(replicaOf("<b>Fort</b>", SubtitleFormat::SubRip) == "{\\b1}Fort{\\b0}");
    CHECK(replicaOf("<u>Lié</u>", SubtitleFormat::SubRip) == "{\\u1}Lié{\\u0}");

    // A colour: HTML writes it as RRGGBB and the overlay as BBGGRR, and it is the pivot that
    // turns one into the other.
    const std::string red =
        replicaOf("<font color=\"#ff0000\">Rouge</font>", SubtitleFormat::WebVtt);
    CHECK(red.find("Rouge") != std::string::npos);
    CHECK(red.find("<font") == std::string::npos);
    CHECK(red.find("\\c&H0000ff&") != std::string::npos);
}

// Every vocabulary reads in its own way, and they all end in the same one.
TEST_CASE("each vocabulary is read in its own way", "[video][replica][GUI-REPLICA-01]") {
    CHECK(replicaOf("{\\i1}Un.{\\i0}", SubtitleFormat::AdvancedSubStationAlpha) ==
          "{\\i1}Un.{\\i0}");
    CHECK(replicaOf("{y:i}Un.", SubtitleFormat::MicroDvd) == "{\\i1}Un.{\\i0}");
    CHECK(replicaOf("/Un.", SubtitleFormat::Mpl2) == "{\\i1}Un.{\\i0}");
    // Two of the nine have no way to say anything: what they hold is text.
    CHECK(replicaOf("Un.", SubtitleFormat::TMPlayer) == "Un.");
    CHECK(replicaOf("Un.", SubtitleFormat::Lrc) == "Un.");
}

// The rule of the decision, and the one that was broken: nothing is drawn as it stands.
TEST_CASE("a tag with no equivalent on screen is removed, not drawn",
          "[video][replica][GUI-REPLICA-01]") {
    // Layout: a position and an alignment of Advanced SSA.
    CHECK(replicaOf("{\\an8}En haut.", SubtitleFormat::AdvancedSubStationAlpha) == "En haut.");
    CHECK(replicaOf("{\\pos(100,200)}Ici.", SubtitleFormat::AdvancedSubStationAlpha) == "Ici.");
    // A speaker of WebVTT.
    CHECK(replicaOf("<v Marie>Bonjour.", SubtitleFormat::WebVtt).find("<v") == std::string::npos);
    CHECK(replicaOf("<v Marie>Bonjour.", SubtitleFormat::WebVtt).find("Bonjour.") !=
          std::string::npos);
}

// The overlay speaks the same vocabulary as the file would: a size or a font from the file is
// not one the overlay can honour, and drawing it would make a replica nobody can read.
TEST_CASE("a font or a size is not drawn", "[video][replica][GUI-REPLICA-01]") {
    const std::string drawn = replicaOf("<font size=\"3\">Petit</font>", SubtitleFormat::SubRip);

    CHECK(drawn == "Petit");
}

// The case that made the player escape braces in the first place (#176): a subtitle saying
// « {laughs} » says it. Now it is the replica that escapes, because it is the replica that
// writes the tags a brace would be taken for.
TEST_CASE("the braces of the visible text are escaped", "[video][replica][GUI-REPLICA-01]") {
    CHECK(replicaOf("{rires}", SubtitleFormat::SubRip) == "\\{rires\\}");
    CHECK(replicaOf("<i>{rires}</i>", SubtitleFormat::SubRip) == "{\\i1}\\{rires\\}{\\i0}");
}
