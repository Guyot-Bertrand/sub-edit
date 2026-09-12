// The tag-aware parser, held to the cases that were written before it.
//
// `src/test/data/textes/recherche.cas` and its brace sibling are the
// specification — issue #372 wrote them as questions put to whoever decides,
// and this is the day they become verifications. Where the code and the corpus
// disagree, the corpus is right.

#include <subedit/core/model/subtitle_format.hpp>
#include <subedit/core/text/markup_parser.hpp>

#include <catch2/catch_test_macros.hpp>

#include <string>
#include <text_cases.hpp>
#include <utility>
#include <vector>

namespace {

using subedit::core::MarkupParser;
using subedit::core::replacingAll;
using subedit::core::SubtitleFormat;
using subedit::test::checkReplacementCases;
using subedit::test::replacementCasesOf;

} // namespace

TEST_CASE("the cases written in the HTML vocabulary pass", "[text][parser]") {
    checkReplacementCases(
        replacementCasesOf("textes/recherche.cas"),
        [](const std::string& text, const std::string& pattern, const std::string& replacement) {
            return replacingAll(text, pattern, replacement, SubtitleFormat::SubRip);
        });
}

TEST_CASE("the cases written in braces pass too", "[text][parser]") {
    checkReplacementCases(
        replacementCasesOf("textes/recherche-accolades.cas"),
        [](const std::string& text, const std::string& pattern, const std::string& replacement) {
            return replacingAll(
                text, pattern, replacement, SubtitleFormat::AdvancedSubStationAlpha);
        });
}

TEST_CASE("a format that writes no style has nothing to hold aside", "[text][parser]") {
    // An LRC line opens with a `/` that is a slash and nothing else.
    const MarkupParser parser{"/Bonjour <i>Marie</i>", SubtitleFormat::Lrc};

    CHECK(parser.visible() == "/Bonjour <i>Marie</i>");
    CHECK(replacingAll("/Bonjour", "Bonjour", "Salut", SubtitleFormat::Lrc) == "/Salut");
}

TEST_CASE("a text nothing touched comes back as the bytes it arrived as", "[text][parser]") {
    // The reassembly is not the identity — it cannot be, having moved tags —
    // so a search that finds nothing must not go through it. Otherwise looking
    // for something absent would tidy a file behind the user's back.
    constexpr const char* kOdd = "<i >Bonjour</I >  <b>Marie";

    CHECK(replacingAll(kOdd, "Sophie", "Claire", SubtitleFormat::SubRip) == kOdd);
    CHECK(MarkupParser{kOdd, SubtitleFormat::SubRip}.text() == kOdd);
}

TEST_CASE("an empty pattern matches nothing rather than everything", "[text][parser]") {
    CHECK(replacingAll("Bonjour", "", "X", SubtitleFormat::SubRip) == "Bonjour");
}

TEST_CASE("what only looks like a tag is text, and stays put", "[text][parser]") {
    // Four shapes the vocabularies refuse, and none of them may be swallowed:
    // whatever the parser does not understand, it carries.
    const auto kept =
        [](const char* text, const char* pattern, const char* with, SubtitleFormat format) {
            return replacingAll(text, pattern, with, format);
        };

    // An empty pair of angle brackets names nothing.
    CHECK(kept("<>Bonjour", "Bonjour", "Salut", SubtitleFormat::SubRip) == "<>Salut");
    // A closing tag that shuts nothing is text the document carries.
    CHECK(kept("Bonjour</i> Marie", "Marie", "Sophie", SubtitleFormat::SubRip) ==
          "Bonjour</i> Sophie");
    // A brace block that is not an override at all.
    CHECK(kept("{une note}Bonjour", "Bonjour", "Salut", SubtitleFormat::AdvancedSubStationAlpha) ==
          "{une note}Salut");
    // A block that opens and closes at once pairs with nothing.
    CHECK(kept(R"({\i1\b0}Bonjour)", "Bonjour", "Salut", SubtitleFormat::AdvancedSubStationAlpha) ==
          R"({\i1\b0}Salut)");
    // An override whose value is not a number.
    CHECK(kept(R"({\ix}Bonjour)", "Bonjour", "Salut", SubtitleFormat::AdvancedSubStationAlpha) ==
          R"({\ix}Salut)");
    // A brace nobody closed.
    CHECK(kept("{Bonjour", "Bonjour", "Salut", SubtitleFormat::AdvancedSubStationAlpha) ==
          "{Salut");
    // The reset, which shuts everything and pairs with nothing.
    CHECK(kept(R"({\r}Bonjour)", "Bonjour", "Salut", SubtitleFormat::AdvancedSubStationAlpha) ==
          R"({\r}Salut)");
}

TEST_CASE("a closing tag shuts its own style and not the one inside it", "[text][parser]") {
    // `</i>` here has a `<b>` opened after it and still open: it must walk past
    // it rather than shut the wrong one.
    CHECK(replacingAll("<i><b>Bonjour</i> Marie", "Marie", "Sophie", SubtitleFormat::SubRip) ==
          "<i><b>Bonjour</i> Sophie");
}

TEST_CASE("a replacement may carry a tag that opens nothing", "[text][parser]") {
    CHECK(replacingAll("Bonjour tout le monde",
                       "Bonjour",
                       R"({\pos(1,2)}Salut)",
                       SubtitleFormat::AdvancedSubStationAlpha) ==
          R"({\pos(1,2)}Salut tout le monde)");
}

TEST_CASE("the two vocabularies that never close carry their tags whole", "[text][parser]") {
    // MicroDVD opens a style to the end of the subtitle and MPL2 to the end of
    // its line; neither has a closer, so neither has a span to widen. What they
    // have is a place, and a replacement leaves it alone.
    CHECK(replacingAll("{Y:i}Bonjour Marie", "Marie", "Sophie", SubtitleFormat::MicroDvd) ==
          "{Y:i}Bonjour Sophie");
    CHECK(replacingAll("/Bonjour\n/Marie", "Marie", "Sophie", SubtitleFormat::Mpl2) ==
          "/Bonjour\n/Sophie");

    // A marker is only a marker at the head of a line.
    CHECK(replacingAll("Bonjour/Marie", "Marie", "Sophie", SubtitleFormat::Mpl2) ==
          "Bonjour/Sophie");
}

TEST_CASE("a tag taken inside a replacement comes out before it", "[text][parser]") {
    // It survives a text that has gone, and there is nothing else to say of it.
    CHECK(replacingAll("Bonjour {Y:i}Marie", "Bonjour Marie", "Salut", SubtitleFormat::MicroDvd) ==
          "{Y:i}Salut");
}

TEST_CASE("an accented letter is never a word boundary", "[text][parser]") {
    // The bytes of a UTF-8 sequence must count as part of a word, or a tag
    // falling between them would look like a boundary and stay there.
    CHECK(replacingAll("<i>ét</i>é", "été", "hiver", SubtitleFormat::SubRip) == "<i>hiver</i>");
}

TEST_CASE("the parser moves", "[text][parser]") {
    MarkupParser parser{"<i>Bonjour</i>", SubtitleFormat::SubRip};
    MarkupParser moved{std::move(parser)};
    CHECK(moved.visible() == "Bonjour");

    MarkupParser other{"Marie", SubtitleFormat::SubRip};
    other = std::move(moved);
    CHECK(other.visible() == "Bonjour");
}
