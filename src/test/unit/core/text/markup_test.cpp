// The pivot itself: a colour, a style, and the runs a text is cut into.
//
// Nothing here reads or writes a tag. What is asserted is the vocabulary the
// five codecs share, and the two tables that say which format speaks which —
// ADR 0031.

#include <subedit/core/model/subtitle_format.hpp>
#include <subedit/core/text/markup.hpp>
#include <subedit/core/text/markup_codec.hpp>
#include <subedit/core/text/markup_vocabulary.hpp>
#include <subedit/core/wording.hpp>

#include <catch2/catch_test_macros.hpp>

#include <optional>
#include <string>
#include <string_view>

namespace {

using subedit::core::abilitiesOf;
using subedit::core::appendRun;
using subedit::core::Colour;
using subedit::core::keepWritable;
using subedit::core::kStyleAttributes;
using subedit::core::MarkupVocabulary;
using subedit::core::plainTextOf;
using subedit::core::Style;
using subedit::core::StyleAbilities;
using subedit::core::StyleAttribute;
using subedit::core::StyledText;
using subedit::core::SubtitleFormat;
using subedit::core::vocabularyOf;

/// The colour `text` reads as, or a failure that names it.
///
/// **A helper rather than a dereference after `REQUIRE`.** Catch2's macro does
/// stop the case, and nothing in the type says so; going through a function
/// that returns early is what makes the reading safe to look at.
[[nodiscard]] Colour readColour(std::string_view text) {
    const std::optional<Colour> read = Colour::parse(text);
    if (!read.has_value()) {
        FAIL("la couleur était censée se lire");
        return Colour::fromChannels(0, 0, 0);
    }
    return *read;
}

/// The same, for the order the two brace vocabularies write.
[[nodiscard]] Colour readReversedColour(std::string_view text) {
    const std::optional<Colour> read = Colour::parseReversed(text);
    if (!read.has_value()) {
        FAIL("la couleur était censée se lire");
        return Colour::fromChannels(0, 0, 0);
    }
    return *read;
}

} // namespace

TEST_CASE("a colour reads and writes in the order it was asked for", "[markup]") {
    const Colour yellow = readColour("ffff00");

    CHECK(yellow.toString() == "ffff00");
    // The two brace vocabularies write the channels the other way round, which
    // is a property of the writing and not of the colour.
    CHECK(yellow.toReversedString() == "00ffff");
}

TEST_CASE("a reversed colour is padded on the left, never on the right", "[markup]") {
    // `{\c&Hff00&}` is green. Padded the other way it would be red, and a file
    // that omitted its leading zeroes would come back a different colour.
    CHECK(readReversedColour("ff00").toString() == "00ff00");
}

TEST_CASE("what is not six hexadecimal digits is not a colour", "[markup]") {
    CHECK_FALSE(Colour::parse("ffff0").has_value());
    CHECK_FALSE(Colour::parse("ffff000").has_value());
    CHECK_FALSE(Colour::parse("gggggg").has_value());
    CHECK_FALSE(Colour::parseReversed("1234567").has_value());
}

TEST_CASE("a style counts what it says, and drops it one at a time", "[markup]") {
    const Style loud{.bold = true, .italic = true, .colour = Colour::parse("ff0000")};

    CHECK(loud.count() == 3);
    CHECK_FALSE(loud.isPlain());
    CHECK(loud.carries(StyleAttribute::Bold));
    CHECK_FALSE(loud.carries(StyleAttribute::Underline));

    const Style quieter = loud.without(StyleAttribute::Colour);
    CHECK(quieter.count() == 2);
    CHECK_FALSE(quieter.carries(StyleAttribute::Colour));
}

TEST_CASE("a plain style says nothing about any of the six", "[markup]") {
    const Style plain;
    CHECK(plain.isPlain());
    for (const StyleAttribute attribute : kStyleAttributes) {
        INFO("attribut : " << static_cast<int>(attribute));
        CHECK_FALSE(plain.carries(attribute));
    }
}

TEST_CASE("two runs that say the same thing become one", "[markup]") {
    // **What the encoders are spared.** Two runs in a row asking for italics
    // would be written as two italic tags, and the file would have grown for
    // no reason a reader could name.
    StyledText runs;
    appendRun(runs, "Le vent ", Style{.italic = true});
    appendRun(runs, "tombe.", Style{.italic = true});

    REQUIRE(runs.size() == 1);
    CHECK(runs.front().text == "Le vent tombe.");
}

TEST_CASE("a run of no text is not a run", "[markup]") {
    StyledText runs;
    appendRun(runs, "", Style{.italic = true});
    CHECK(runs.empty());
}

TEST_CASE("the plain text of a styled text is its runs, in order", "[markup]") {
    StyledText runs;
    appendRun(runs, "Le vent ", Style{.bold = true});
    appendRun(runs, "tombe.", Style{});

    CHECK(plainTextOf(runs) == "Le vent tombe.");
}

TEST_CASE("each of the nine formats speaks one of the five vocabularies", "[markup]") {
    CHECK(vocabularyOf(SubtitleFormat::SubRip) == MarkupVocabulary::Html);
    CHECK(vocabularyOf(SubtitleFormat::WebVtt) == MarkupVocabulary::Html);
    CHECK(vocabularyOf(SubtitleFormat::SubViewer2) == MarkupVocabulary::Html);
    CHECK(vocabularyOf(SubtitleFormat::SubStationAlpha) == MarkupVocabulary::SubStationAlpha);
    CHECK(vocabularyOf(SubtitleFormat::AdvancedSubStationAlpha) ==
          MarkupVocabulary::SubStationAlpha);
    CHECK(vocabularyOf(SubtitleFormat::MicroDvd) == MarkupVocabulary::MicroDvd);
    CHECK(vocabularyOf(SubtitleFormat::Mpl2) == MarkupVocabulary::Mpl2);
    CHECK(vocabularyOf(SubtitleFormat::TMPlayer) == MarkupVocabulary::None);
    CHECK(vocabularyOf(SubtitleFormat::Lrc) == MarkupVocabulary::None);
}

TEST_CASE("a shared vocabulary is not a shared set of abilities", "[markup]") {
    // **The pair that makes abilities a question of their own.** SubRip and
    // WebVTT spell italics the same way and disagree about colour, so a colour
    // is lost between them without a single tag changing shape.
    CHECK(vocabularyOf(SubtitleFormat::SubRip) == vocabularyOf(SubtitleFormat::WebVtt));
    CHECK(abilitiesOf(SubtitleFormat::SubRip).colour);
    CHECK_FALSE(abilitiesOf(SubtitleFormat::WebVtt).colour);

    // And the other pair, inside the braces: `{\u1}` is Advanced SSA's, and
    // Gaupol neither reads nor writes it for Sub Station Alpha.
    CHECK_FALSE(abilitiesOf(SubtitleFormat::SubStationAlpha).underline);
    CHECK(abilitiesOf(SubtitleFormat::AdvancedSubStationAlpha).underline);
}

TEST_CASE("the two formats with no vocabulary can write nothing", "[markup]") {
    for (const SubtitleFormat format : {SubtitleFormat::TMPlayer, SubtitleFormat::Lrc}) {
        INFO("format : " << subedit::core::nameOf(format));
        const StyleAbilities abilities = abilitiesOf(format);
        for (const StyleAttribute attribute : kStyleAttributes)
            CHECK_FALSE(abilities.can(attribute));
    }
}

TEST_CASE("what a format cannot write is dropped before a tag is composed", "[markup]") {
    // **Counted here, and merged here.** Two runs differing only by a colour
    // the target cannot write are the same run once it is gone.
    StyledText runs;
    appendRun(runs, "Le vent ", Style{.italic = true, .colour = Colour::parse("ff0000")});
    appendRun(runs, "tombe.", Style{.italic = true});

    const subedit::core::WritableMarkup kept =
        keepWritable(runs, abilitiesOf(SubtitleFormat::WebVtt));

    CHECK(kept.dropped == 1);
    REQUIRE(kept.runs.size() == 1);
    CHECK(kept.runs.front().text == "Le vent tombe.");
    CHECK(kept.runs.front().style == Style{.italic = true});
}

TEST_CASE("a colour built from its channels is the colour that text reads", "[markup]") {
    // **The two ways in, and they have to agree.** `fromChannels` is what a
    // caller holding three bytes uses; `parse` is what a file gives. A pivot
    // where the two disagreed would turn a colour on a round trip.
    CHECK(Colour::fromChannels(255, 136, 0) == readColour("ff8800"));
    CHECK(Colour::fromChannels(0, 255, 0) == readReversedColour("ff00"));
    CHECK(Colour::fromChannels(0, 0, 0).toString() == "000000");
}

TEST_CASE("hexadecimal digits read in either case", "[markup]") {
    // A file writes `#FFFF00` as readily as `#ffff00`, and both are the same
    // yellow. What comes back out is lower case, which is the one shape.
    CHECK(readColour("FFFF00") == readColour("ffff00"));
    CHECK(readColour("AbCdEf").toString() == "abcdef");
}

TEST_CASE("a reversed colour short enough to pad still has to be digits", "[markup]") {
    // Padding happens first, so what fails here is the reading of the padded
    // text and not its length: `zz` is `0000zz`, and `zz` is not a channel.
    CHECK_FALSE(Colour::parseReversed("zz").has_value());
}
