// The one reader of tags that every piece of the core goes through — #403.

#include <subedit/core/text/markup_reader.hpp>
#include <subedit/core/text/markup_vocabulary.hpp>

#include <catch2/catch_test_macros.hpp>

#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace {

using subedit::core::FlagOverride;
using subedit::core::flagOverrideOf;
using subedit::core::htmlTagOf;
using subedit::core::MarkupPiece;
using subedit::core::MarkupVocabulary;
using subedit::core::mayHoldMarkup;
using subedit::core::overridesOf;
using subedit::core::piecesOf;
using subedit::core::ScopedTag;
using subedit::core::scopedTagOf;

using Spelled = std::vector<std::string>;

/// `T` for text, `B` for a tag — a bracket or a brace — and `M` for a marker.
[[nodiscard]] char letterOf(MarkupPiece::Kind kind) {
    switch (kind) {
    case MarkupPiece::Kind::Text:
        return 'T';
    case MarkupPiece::Kind::Tag:
        return 'B';
    case MarkupPiece::Kind::Marker:
        return 'M';
    }
    return '?';
}

/// The pieces of `text`, each written as its kind's letter and its bytes.
[[nodiscard]] Spelled spelled(std::string_view text, MarkupVocabulary vocabulary) {
    Spelled out;
    for (const MarkupPiece& piece : piecesOf(text, vocabulary))
        out.push_back(std::string{letterOf(piece.kind)} + ':' + std::string{piece.text});
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

TEST_CASE("a second opener before the closer makes the first one text", "[text][reader]") {
    // Otherwise a lone `<` swallows the tag after it: `< b <i>` read as one tag
    // hid an italic from the pivot, the parser and the toggle alike.
    CHECK(spelled("a < b <i>c</i>", MarkupVocabulary::Html) ==
          Spelled{"T:a < b ", "B:<i>", "T:c", "B:</i>"});
    CHECK(spelled("a { b {Y:i}c", MarkupVocabulary::MicroDvd) ==
          Spelled{"T:a { b ", "B:{Y:i}", "T:c"});
}

TEST_CASE("an MPL2 marker is a marker at the head of a line only", "[text][reader]") {
    CHECK(spelled("/\\Bonjour\n_Marie/", MarkupVocabulary::Mpl2) ==
          Spelled{"M:/", "M:\\", "T:Bonjour\n", "M:_", "T:Marie/"});
    CHECK(spelled("{y:b}/Bonjour", MarkupVocabulary::Mpl2) == Spelled{"B:{y:b}", "T:/Bonjour"});
    CHECK(spelled("/Bonjour", MarkupVocabulary::MicroDvd) == Spelled{"T:/Bonjour"});
}

TEST_CASE("a text without the opener of a vocabulary holds none of its markup", "[text][reader]") {
    CHECK_FALSE(mayHoldMarkup("le vent", MarkupVocabulary::Html));
    CHECK(mayHoldMarkup("le <i>vent", MarkupVocabulary::Html));
    CHECK_FALSE(mayHoldMarkup("le {i}vent", MarkupVocabulary::Html));
    CHECK_FALSE(mayHoldMarkup("le <i>vent", MarkupVocabulary::MicroDvd));
    CHECK(mayHoldMarkup("le {Y:i}vent", MarkupVocabulary::MicroDvd));
    CHECK(mayHoldMarkup(R"({\i1}le vent)", MarkupVocabulary::SubStationAlpha));
    CHECK_FALSE(mayHoldMarkup("<i>{i}", MarkupVocabulary::None));

    // A marker is not found by an opener, so MPL2 never rules a text out.
    CHECK(mayHoldMarkup("/le vent", MarkupVocabulary::Mpl2));
}

TEST_CASE("a text the reader says holds no markup comes back as text alone", "[text][reader]") {
    // What makes it safe to skip reading such a text: the answer no is never
    // wrong, whatever the text and whichever vocabulary reads it.
    const std::vector<std::string_view> texts = {
        "",
        "le vent",
        "le <i>vent</i>",
        "{Y:i}le vent",
        R"({\i1}le vent{\i0})",
        "a < b > c",
        "un\ndeux",
        "/un\n_deux",
        "[bruit] (tout)",
        "a } b",
        "{",
        "<",
        "  \t ",
        "-\n-",
    };
    const std::vector<MarkupVocabulary> vocabularies = {
        MarkupVocabulary::None,
        MarkupVocabulary::Html,
        MarkupVocabulary::SubStationAlpha,
        MarkupVocabulary::MicroDvd,
        MarkupVocabulary::Mpl2,
    };
    std::size_t examined = 0;
    for (const std::string_view text : texts) {
        for (const MarkupVocabulary vocabulary : vocabularies) {
            if (mayHoldMarkup(text, vocabulary))
                continue;
            ++examined;

            // Text alone, and all of it: a fast path that skipped the read must
            // find in the text exactly what the read would have shown.
            std::string joined;
            for (const MarkupPiece& piece : piecesOf(text, vocabulary)) {
                CHECK(piece.kind == MarkupPiece::Kind::Text);
                joined += piece.text;
            }
            CHECK(joined == text);
        }
    }
    // A predicate that answered yes to everything would leave the loop empty
    // and the case green.
    CHECK(examined > 0);
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

    // A tab separates a name as a space does.
    CHECK(htmlTagOf("<i\tclass=\"x\">").name == "i");
}

TEST_CASE("a self-closing tag names nothing", "[text][reader]") {
    // `<i/>` opens nothing and closes nothing; read as `<i>`, it put the rest
    // of the subtitle in italics.
    CHECK(htmlTagOf("<i/>").name.empty());
    CHECK(htmlTagOf("<br />").name.empty());
}

TEST_CASE("a brace block is a run of overrides, or not one at all", "[text][reader]") {
    using Overrides = std::vector<std::string_view>;
    const Overrides absent{"<absent>"};

    CHECK(overridesOf(R"({\b1\i1})").value_or(absent) == Overrides{"b1", "i1"});
    CHECK(overridesOf("{}").value_or(absent).empty());
    CHECK_FALSE(overridesOf("{une note}").has_value());
}

TEST_CASE("the three flags are read with their number", "[text][reader]") {
    // A letter no flag has, so that an absent answer cannot pass for one.
    const FlagOverride absent{.letter = 'x', .on = false};

    const FlagOverride on = flagOverrideOf("i1").value_or(absent);
    CHECK(on.letter == 'i');
    CHECK(on.on);

    CHECK(flagOverrideOf("b700").value_or(absent).on);
    CHECK(flagOverrideOf("u0").value_or(absent).letter == 'u');
    CHECK_FALSE(flagOverrideOf("u0").value_or(absent).on);

    for (const std::string_view other : {"i", "i1x", "fs12", "pos(1,2)"}) {
        INFO("override: " << other);
        CHECK_FALSE(flagOverrideOf(other).has_value());
    }
}

TEST_CASE("a MicroDVD tag is a letter, a scope and a value", "[text][reader]") {
    const ScopedTag absent{.letter = 'x', .wholeSubtitle = false, .value = "<absent>"};

    const ScopedTag style = scopedTagOf("{Y:bi}").value_or(absent);
    CHECK(style.letter == 'y');
    CHECK(style.wholeSubtitle);
    CHECK(style.value == "bi");

    const ScopedTag colour = scopedTagOf("{c:$0000ff}").value_or(absent);
    CHECK(colour.letter == 'c');
    CHECK_FALSE(colour.wholeSubtitle);
    CHECK(colour.value == "$0000ff");

    CHECK_FALSE(scopedTagOf("{une note}").has_value());
    CHECK_FALSE(scopedTagOf("{x:1}").has_value());
}
