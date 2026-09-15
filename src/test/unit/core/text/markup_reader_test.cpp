// The one reader of tags that every piece of the core goes through — #403.

#include <subedit/core/text/markup_reader.hpp>
#include <subedit/core/text/markup_vocabulary.hpp>

#include <catch2/catch_test_macros.hpp>

#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace {

using subedit::core::flagOverrideOf;
using subedit::core::htmlTagOf;
using subedit::core::MarkupPiece;
using subedit::core::MarkupVocabulary;
using subedit::core::overridesOf;
using subedit::core::piecesOf;
using subedit::core::scopedTagOf;

using Spelled = std::vector<std::string>;

/// The pieces of `text`, each written as its kind's letter and its bytes.
///
/// `T` for text, `B` for a tag — a bracket or a brace — and `M` for a marker.
[[nodiscard]] Spelled spelled(std::string_view text, MarkupVocabulary vocabulary) {
    Spelled out;
    for (const MarkupPiece& piece : piecesOf(text, vocabulary)) {
        const char letter = piece.kind == MarkupPiece::Kind::Text  ? 'T'
                            : piece.kind == MarkupPiece::Kind::Tag ? 'B'
                                                                   : 'M';
        out.push_back(std::string{letter} + ':' + std::string{piece.text});
    }
    return out;
}

} // namespace

TEST_CASE("a text is cut into what shows and what marks it up", "[text][reader]") {
    CHECK(spelled("le <i>vent</i>", MarkupVocabulary::Html) ==
          Spelled{"T:le ", "B:<i>", "T:vent", "B:</i>"});
    CHECK(spelled(R"({\i1}le vent)", MarkupVocabulary::SubStationAlpha) ==
          Spelled{R"(B:{\i1})", "T:le vent"});
    CHECK(spelled("{Y:i}le vent", MarkupVocabulary::MicroDvd) == Spelled{"B:{Y:i}", "T:le vent"});
}

TEST_CASE("each vocabulary reads its own delimiters and no other", "[text][reader]") {
    CHECK(spelled("{i}le <i>vent", MarkupVocabulary::Html) ==
          Spelled{"T:{i}le ", "B:<i>", "T:vent"});
    CHECK(spelled("<i>le {i}vent", MarkupVocabulary::MicroDvd) ==
          Spelled{"T:<i>le ", "B:{i}", "T:vent"});
    CHECK(spelled("/le <i>{vent}", MarkupVocabulary::None) == Spelled{"T:/le <i>{vent}"});
}

TEST_CASE("an opener closes on its own line, or is text", "[text][reader]") {
    CHECK(spelled("le <i\n>vent", MarkupVocabulary::Html) == Spelled{"T:le <i\n>vent"});
    CHECK(spelled("le {Y:i vent", MarkupVocabulary::MicroDvd) == Spelled{"T:le {Y:i vent"});
    CHECK(spelled("<>vent", MarkupVocabulary::Html) == Spelled{"B:<>", "T:vent"});
}

TEST_CASE("an MPL2 marker is a marker at the head of a line only", "[text][reader]") {
    CHECK(spelled("/\\Bonjour\n_Marie/", MarkupVocabulary::Mpl2) ==
          Spelled{"M:/", "M:\\", "T:Bonjour\n", "M:_", "T:Marie/"});
    CHECK(spelled("{y:b}/Bonjour", MarkupVocabulary::Mpl2) == Spelled{"B:{y:b}", "T:/Bonjour"});
    CHECK(spelled("/Bonjour", MarkupVocabulary::MicroDvd) == Spelled{"T:/Bonjour"});
}

TEST_CASE("each piece knows where it starts", "[text][reader]") {
    const std::vector<MarkupPiece> pieces = piecesOf("ab<i>c", MarkupVocabulary::Html);

    REQUIRE(pieces.size() == 3);
    CHECK(pieces[0].at == 0);
    CHECK(pieces[1].at == 2);
    CHECK(pieces[2].at == 5);
}

TEST_CASE("an HTML tag gives its name lower-cased, and what follows it", "[text][reader]") {
    CHECK(htmlTagOf("<I >").name == "i");
    CHECK_FALSE(htmlTagOf("<I >").closing);

    const auto font = htmlTagOf(R"(<font  color="#ff0000">)");
    CHECK(font.name == "font");
    CHECK(font.attributes == R"(color="#ff0000")");

    CHECK(htmlTagOf("</i >").name == "i");
    CHECK(htmlTagOf("</i >").closing);
    CHECK(htmlTagOf("<>").name.empty());
}

TEST_CASE("a brace block is a run of overrides, or not one at all", "[text][reader]") {
    const auto two = overridesOf(R"({\b1\i1})");
    REQUIRE(two.has_value());
    CHECK(*two == std::vector<std::string_view>{"b1", "i1"});

    const auto none = overridesOf("{}");
    REQUIRE(none.has_value());
    CHECK(none->empty());

    CHECK_FALSE(overridesOf("{une note}").has_value());
}

TEST_CASE("the three flags are read with their number", "[text][reader]") {
    const auto on = flagOverrideOf("i1");
    REQUIRE(on.has_value());
    CHECK(on->letter == 'i');
    CHECK(on->on);

    CHECK(flagOverrideOf("b700")->on);
    CHECK_FALSE(flagOverrideOf("u0")->on);

    for (const std::string_view other : {"i", "i1x", "fs12", "pos(1,2)"}) {
        INFO("surcharge : " << other);
        CHECK_FALSE(flagOverrideOf(other).has_value());
    }
}

TEST_CASE("a MicroDVD tag is a letter, a scope and a value", "[text][reader]") {
    const auto style = scopedTagOf("{Y:bi}");
    REQUIRE(style.has_value());
    CHECK(style->letter == 'y');
    CHECK(style->wholeSubtitle);
    CHECK(style->value == "bi");

    const auto colour = scopedTagOf("{c:$0000ff}");
    REQUIRE(colour.has_value());
    CHECK(colour->letter == 'c');
    CHECK_FALSE(colour->wholeSubtitle);
    CHECK(colour->value == "$0000ff");

    CHECK_FALSE(scopedTagOf("{une note}").has_value());
    CHECK_FALSE(scopedTagOf("{x:1}").has_value());
}
