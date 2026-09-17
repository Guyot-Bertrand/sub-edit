// The tag-aware parser, held to the cases that were written before it.
//
// `src/test/data/textes/recherche.cas` and its brace sibling are the
// specification — issue #372 wrote them as questions put to whoever decides,
// and this is the day they become verifications. Where the code and the corpus
// disagree, the corpus is right.

#include <subedit/core/model/subtitle_format.hpp>
#include <subedit/core/text/markup_parser.hpp>

#include <catch2/catch_test_macros.hpp>

#include <cstddef>
#include <string>
#include <string_view>
#include <text_cases.hpp>
#include <utility>
#include <vector>

namespace {

using subedit::core::MarkupParser;
using subedit::core::SubtitleFormat;
using subedit::test::checkReplacementCases;
using subedit::test::replacementCasesOf;

/// Replaces every occurrence of `pattern` in `text`, through `MarkupParser`
/// alone — the driver these cases need, now that production carries no
/// caller of its own for a "replace everywhere" over a literal pattern.
///
/// Nothing is rebuilt when nothing matches: the text comes back as the very
/// bytes it arrived as, which is what keeps a search that finds nothing from
/// tidying a file behind the user's back.
[[nodiscard]] std::string replacedAll(std::string_view text,
                                      std::string_view pattern,
                                      std::string_view replacement,
                                      SubtitleFormat format) {
    if (pattern.empty())
        return std::string{text};

    MarkupParser parser{text, format};
    bool found = false;
    std::size_t at = 0;
    while (true) {
        const std::size_t which = parser.visible().find(pattern, at);
        if (which == std::string_view::npos)
            break;
        parser.replace(which, pattern.size(), replacement);
        at = which + MarkupParser{replacement, format}.visible().size();
        found = true;
    }
    return found ? parser.text() : std::string{text};
}

} // namespace

TEST_CASE("the cases written in the HTML vocabulary pass", "[text][parser]") {
    checkReplacementCases(
        replacementCasesOf("textes/recherche.cas"),
        [](const std::string& text, const std::string& pattern, const std::string& replacement) {
            return replacedAll(text, pattern, replacement, SubtitleFormat::SubRip);
        });
}

TEST_CASE("the cases written in braces pass too", "[text][parser]") {
    checkReplacementCases(
        replacementCasesOf("textes/recherche-accolades.cas"),
        [](const std::string& text, const std::string& pattern, const std::string& replacement) {
            return replacedAll(text, pattern, replacement, SubtitleFormat::AdvancedSubStationAlpha);
        });
}

TEST_CASE("a format that writes no style has nothing to hold aside", "[text][parser]") {
    // An LRC line opens with a `/` that is a slash and nothing else.
    const MarkupParser parser{"/Bonjour <i>Marie</i>", SubtitleFormat::Lrc};

    CHECK(parser.visible() == "/Bonjour <i>Marie</i>");
    CHECK(replacedAll("/Bonjour", "Bonjour", "Salut", SubtitleFormat::Lrc) == "/Salut");
}

TEST_CASE("a text nothing touched comes back as the bytes it arrived as", "[text][parser]") {
    // The reassembly is not the identity — it cannot be, having moved tags —
    // so a search that finds nothing must not go through it. Otherwise looking
    // for something absent would tidy a file behind the user's back.
    constexpr const char* kOdd = "<i >Bonjour</I >  <b>Marie";

    CHECK(replacedAll(kOdd, "Sophie", "Claire", SubtitleFormat::SubRip) == kOdd);
    CHECK(MarkupParser{kOdd, SubtitleFormat::SubRip}.text() == kOdd);
}

TEST_CASE("an empty pattern matches nothing rather than everything", "[text][parser]") {
    CHECK(replacedAll("Bonjour", "", "X", SubtitleFormat::SubRip) == "Bonjour");
}

TEST_CASE("what only looks like a tag is text, and stays put", "[text][parser]") {
    // Four shapes the vocabularies refuse, and none of them may be swallowed:
    // whatever the parser does not understand, it carries.
    const auto kept =
        [](const char* text, const char* pattern, const char* with, SubtitleFormat format) {
            return replacedAll(text, pattern, with, format);
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
    CHECK(replacedAll("<i><b>Bonjour</i> Marie", "Marie", "Sophie", SubtitleFormat::SubRip) ==
          "<i><b>Bonjour</i> Sophie");
}

TEST_CASE("a replacement may carry a tag that opens nothing", "[text][parser]") {
    CHECK(replacedAll("Bonjour tout le monde",
                      "Bonjour",
                      R"({\pos(1,2)}Salut)",
                      SubtitleFormat::AdvancedSubStationAlpha) ==
          R"({\pos(1,2)}Salut tout le monde)");
}

TEST_CASE("the two vocabularies that never close carry their tags whole", "[text][parser]") {
    // MicroDVD opens a style to the end of the subtitle and MPL2 to the end of
    // its line; neither has a closer, so neither has a span to widen. What they
    // have is a place, and a replacement leaves it alone.
    CHECK(replacedAll("{Y:i}Bonjour Marie", "Marie", "Sophie", SubtitleFormat::MicroDvd) ==
          "{Y:i}Bonjour Sophie");
    CHECK(replacedAll("/Bonjour\n/Marie", "Marie", "Sophie", SubtitleFormat::Mpl2) ==
          "/Bonjour\n/Sophie");

    // A marker is only a marker at the head of a line.
    CHECK(replacedAll("Bonjour/Marie", "Marie", "Sophie", SubtitleFormat::Mpl2) ==
          "Bonjour/Sophie");
}

TEST_CASE("an MPL2 marker after a brace is text", "[text][parser]") {
    // Issue #403: the parser kept a line « at its head » across a brace, and
    // read the `/` as a marker; the pivot and the italic toggle read text.
    // Searching for it found nothing, and a dash landed after it.
    const MarkupParser parser{"{y:b}/Bonjour", SubtitleFormat::Mpl2};

    CHECK(parser.visible() == "/Bonjour");
    CHECK(replacedAll("{y:b}/Bonjour", "/", "|", SubtitleFormat::Mpl2) == "{y:b}|Bonjour");
}

TEST_CASE("a tag taken inside a replacement comes out before it", "[text][parser]") {
    // It survives a text that has gone, and there is nothing else to say of it.
    CHECK(replacedAll("Bonjour {Y:i}Marie", "Bonjour Marie", "Salut", SubtitleFormat::MicroDvd) ==
          "{Y:i}Salut");
}

TEST_CASE("an accented letter is never a word boundary", "[text][parser]") {
    // A word is read in code points: an accented letter is a letter, or a tag
    // falling beside it would look like a boundary and stay there.
    CHECK(replacedAll("<i>ét</i>é", "été", "hiver", SubtitleFormat::SubRip) == "<i>hiver</i>");
    // And a letter outside the basic plane, four bytes long, is one too.
    CHECK(replacedAll("<i>a</i>𝒜b", "a𝒜b", "c", SubtitleFormat::SubRip) == "<i>c</i>");
}

TEST_CASE("a replacement leaves an empty pair elsewhere alone", "[text][parser]") {
    // Issue #402: only what the match reaches moves or goes. An empty pair the
    // file holds away from the match is the file's business.
    CHECK(replacedAll("a<i></i>b Marie", "Marie", "Sophie", SubtitleFormat::SubRip) ==
          "a<i></i>b Sophie");
}

TEST_CASE("the parser moves", "[text][parser]") {
    MarkupParser parser{"<i>Bonjour</i>", SubtitleFormat::SubRip};
    MarkupParser moved{std::move(parser)};
    CHECK(moved.visible() == "Bonjour");

    MarkupParser other{"Marie", SubtitleFormat::SubRip};
    other = std::move(moved);
    CHECK(other.visible() == "Bonjour");
}

TEST_CASE("a transformation lets no style spread over what it rewrote", "[text][parser]") {
    // The whole difference with a replacement, and the case operations of #379
    // live on it: putting a subtitle in capitals must not make its first word's
    // italic swallow the rest.
    MarkupParser parser{"<i>Bonjour</i> Marie", SubtitleFormat::SubRip};
    parser.transform(0, parser.visible().size(), "BONJOUR MARIE");

    CHECK(parser.text() == "<i>BONJOUR</i> MARIE");
}

TEST_CASE("a transformation that grows keeps the tags it pushed", "[text][parser]") {
    // Putting a dialogue dash at the head of a line: what was at the head stays
    // at the head, and the two characters go after it.
    MarkupParser parser{"<i>Bonjour</i>\n<i>Marie</i>", SubtitleFormat::SubRip};
    parser.transform(8, 0, "- ");
    parser.transform(0, 0, "- ");

    CHECK(parser.text() == "<i>- Bonjour</i>\n<i>- Marie</i>");
}

TEST_CASE("a transformation that shrinks brings the tags back with it", "[text][parser]") {
    MarkupParser parser{"Bonjour <i>Marie</i> !", SubtitleFormat::SubRip};
    parser.transform(0, 8, "");

    CHECK(parser.text() == "<i>Marie</i> !");
}

TEST_CASE("nothing is read as a tag in what a transformation writes", "[text][parser]") {
    // A transformation has no business inventing a tag: `<i>` written here is
    // three characters a user will see.
    MarkupParser parser{"Bonjour", SubtitleFormat::SubRip};
    parser.transform(0, 7, "<i>Salut");

    CHECK(parser.visible() == "<i>Salut");
    CHECK(parser.text() == "<i>Salut");
}

TEST_CASE("a transformation carries the tags it understands nothing of", "[text][parser]") {
    // A MicroDVD tag opens a style and never closes it, so the parser holds it
    // as a place rather than a span — and a transformation moves that place
    // like any other.
    MarkupParser parser{"Bonjour {Y:i}Marie", SubtitleFormat::MicroDvd};
    parser.transform(0, 7, "Salut");

    CHECK(parser.text() == "Salut {Y:i}Marie");
}

TEST_CASE("a block that shuts two styles is written once", "[text][parser]") {
    // `{\b0\i0}` closes the bold and the italic both. Each span kept the
    // block as its closer, and a transformation wrote it twice.
    MarkupParser parser{R"({\b1\i1}bonjour{\b0\i0})", SubtitleFormat::AdvancedSubStationAlpha};
    parser.transform(0, 7, "BONJOUR");

    CHECK(parser.text() == R"({\b1\i1}BONJOUR{\b0\i0})");
}

TEST_CASE("an empty pair stays inside the tag that held it", "[text][parser]") {
    // At the offset where the bold closes, the empty italic came out after it:
    // closers were written before anything else, and the pair wraps nothing.
    MarkupParser parser{"<b>bonjour<i></i></b>", SubtitleFormat::SubRip};
    parser.transform(0, 7, "BONJOUR");

    CHECK(parser.text() == "<b>BONJOUR<i></i></b>");
}

TEST_CASE("a combining mark belongs to its word", "[text][parser]") {
    // A decomposed `é` is an `e` and a mark, and a tag between them cuts the
    // word as surely as one between two letters: a match that starts on the
    // mark reaches the italic before it.
    CHECK(replacedAll("<i>e</i>\u0301t", "\u0301t", "x", SubtitleFormat::SubRip) == "<i>ex</i>");
}

TEST_CASE("an empty pair written after a closer stays after it", "[text][parser]") {
    // The other side of the one above: the pair came after the bold shut, and
    // it goes out after it.
    MarkupParser parser{"<b>bonjour</b><i></i>", SubtitleFormat::SubRip};
    parser.transform(0, 7, "BONJOUR");

    CHECK(parser.text() == "<b>BONJOUR</b><i></i>");
}
