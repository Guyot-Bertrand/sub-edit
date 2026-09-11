// Italic tags put on and taken off, vocabulary by vocabulary — issue #365.
//
// **What is checked at every turn is what is left alone.** Putting a text in
// italics is three characters; doing it without disturbing a colour, a position
// override or a comment is the whole of the work.

#include <subedit/core/model/subtitle_format.hpp>
#include <subedit/core/text/italics.hpp>

#include <catch2/catch_test_macros.hpp>

#include <string>

namespace {

using subedit::core::inItalics;
using subedit::core::opensInItalics;
using subedit::core::SubtitleFormat;
using subedit::core::withoutItalics;

} // namespace

TEST_CASE("the HTML vocabulary wraps a text in its own tags", "[text][italics]") {
    CHECK(inItalics("Bonjour.", SubtitleFormat::SubRip) == "<i>Bonjour.</i>");
    CHECK(inItalics("Bonjour.", SubtitleFormat::WebVtt) == "<i>Bonjour.</i>");
    CHECK(inItalics("Bonjour.", SubtitleFormat::SubViewer2) == "<i>Bonjour.</i>");
}

TEST_CASE("the two brace vocabularies write what their format writes", "[text][italics]") {
    CHECK(inItalics("Bonjour.", SubtitleFormat::SubStationAlpha) == R"({\i1}Bonjour.{\i0})");
    CHECK(inItalics("Bonjour.", SubtitleFormat::AdvancedSubStationAlpha) ==
          R"({\i1}Bonjour.{\i0})");
    // Opened and never closed: a capital tag reaches the end of the subtitle.
    CHECK(inItalics("Bonjour.", SubtitleFormat::MicroDvd) == "{Y:i}Bonjour.");
    CHECK(inItalics("Bonjour.", SubtitleFormat::Mpl2) == "/Bonjour.");
}

TEST_CASE("MPL2 marks every line, since its marker only reaches the end of one",
          "[text][italics]") {
    CHECK(inItalics("Bonjour.\nAu revoir.", SubtitleFormat::Mpl2) == "/Bonjour.\n/Au revoir.");
}

TEST_CASE("a format that writes no style is handed its text back", "[text][italics]") {
    CHECK(inItalics("Bonjour.", SubtitleFormat::TMPlayer) == "Bonjour.");
    CHECK(inItalics("Bonjour.", SubtitleFormat::Lrc) == "Bonjour.");
    CHECK(withoutItalics("/Bonjour.", SubtitleFormat::Lrc) == "/Bonjour.");
    CHECK_FALSE(opensInItalics("/Bonjour.", SubtitleFormat::Lrc));
}

TEST_CASE("putting a text in italics twice wraps it once", "[text][italics]") {
    const std::string once = inItalics("Bonjour.", SubtitleFormat::SubRip);
    CHECK(inItalics(once, SubtitleFormat::SubRip) == once);

    const std::string mpl2 = inItalics("Bonjour.\nAu revoir.", SubtitleFormat::Mpl2);
    CHECK(inItalics(mpl2, SubtitleFormat::Mpl2) == mpl2);
}

TEST_CASE("a text with nothing in it gains no tags", "[text][italics]") {
    CHECK(inItalics("", SubtitleFormat::SubRip).empty());
    CHECK(inItalics("<i></i>", SubtitleFormat::SubRip).empty());
}

TEST_CASE("taking italics out leaves every other tag where it was", "[text][italics]") {
    CHECK(withoutItalics("<b><i>Bonjour.</i></b>", SubtitleFormat::SubRip) == "<b>Bonjour.</b>");
    CHECK(withoutItalics(R"({\pos(10,20)}{\i1}Bonjour.{\i0})",
                         SubtitleFormat::AdvancedSubStationAlpha) == R"({\pos(10,20)}Bonjour.)");
    CHECK(withoutItalics("{C:$0000ff}{Y:i}Bonjour.", SubtitleFormat::MicroDvd) ==
          "{C:$0000ff}Bonjour.");
    CHECK(withoutItalics("/\\Bonjour.", SubtitleFormat::Mpl2) == "\\Bonjour.");
}

TEST_CASE("a tag that says more than italics keeps the rest", "[text][italics]") {
    // Where Gaupol matches one whole tag at a time and hands back a text still
    // in italics after being asked to take them out.
    CHECK(withoutItalics(R"({\b1\i1}Bonjour.)", SubtitleFormat::SubStationAlpha) ==
          R"({\b1}Bonjour.)");
    CHECK(withoutItalics("{Y:bi}Bonjour.", SubtitleFormat::MicroDvd) == "{Y:b}Bonjour.");
}

TEST_CASE("a brace block that said nothing but italics goes with them", "[text][italics]") {
    CHECK(withoutItalics(R"({\i1}Bonjour.{\i0})", SubtitleFormat::SubStationAlpha) == "Bonjour.");
    CHECK(withoutItalics("{y:i}Bonjour.", SubtitleFormat::MicroDvd) == "Bonjour.");
}

TEST_CASE("what is not a tag at all is not rewritten", "[text][italics]") {
    CHECK(withoutItalics("{une note}Bonjour.", SubtitleFormat::SubStationAlpha) ==
          "{une note}Bonjour.");
    CHECK(withoutItalics("{une note}Bonjour.", SubtitleFormat::MicroDvd) == "{une note}Bonjour.");
    // An opening brace nobody closed is text, and stays text — in both
    // vocabularies that write braces.
    CHECK(withoutItalics("{\\i1 Bonjour.", SubtitleFormat::SubStationAlpha) == "{\\i1 Bonjour.");
    CHECK(withoutItalics("{Y:i Bonjour.", SubtitleFormat::MicroDvd) == "{Y:i Bonjour.");
    CHECK(withoutItalics("{Y:i}Un{Y:i Deux", SubtitleFormat::MicroDvd) == "Un{Y:i Deux");
}

TEST_CASE("an italic tag in upper case is taken out too", "[text][italics]") {
    // Our own reader lowers what it reads, so an `<I>` left behind would be a
    // text still in italics after the italics came out.
    CHECK(withoutItalics("<I>Bonjour.</I>", SubtitleFormat::SubRip) == "Bonjour.");
}

TEST_CASE("a text opens in italics when its first visible character does", "[text][italics]") {
    CHECK(opensInItalics("<i>Bonjour.</i>", SubtitleFormat::SubRip));
    CHECK(opensInItalics(R"({\pos(1,2)}{\i1}Bonjour.)", SubtitleFormat::AdvancedSubStationAlpha));
    CHECK(opensInItalics("{Y:i}Bonjour.", SubtitleFormat::MicroDvd));
    CHECK(opensInItalics("/Bonjour.", SubtitleFormat::Mpl2));

    CHECK_FALSE(opensInItalics("Bonjour.", SubtitleFormat::SubRip));
    CHECK_FALSE(opensInItalics("Bonjour <i>Marie</i>", SubtitleFormat::SubRip));
    CHECK_FALSE(opensInItalics("", SubtitleFormat::SubRip));
}
