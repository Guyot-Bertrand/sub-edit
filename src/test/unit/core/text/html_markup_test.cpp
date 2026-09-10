// The HTML tags of SubRip, WebVTT and SubViewer 2.
//
// `<b>`, `<i>`, `<u>` and `<font color="#RRGGBB">`. Everything else these
// formats may carry is layout, which ADR 0031 leaves outside the pivot.

#include <subedit/core/model/subtitle_format.hpp>
#include <subedit/core/text/html_markup.hpp>
#include <subedit/core/text/markup.hpp>
#include <subedit/core/text/markup_codec.hpp>
#include <subedit/core/text/markup_vocabulary.hpp>

#include <catch2/catch_test_macros.hpp>

#include <string>
#include <string_view>

namespace {

using subedit::core::abilitiesOf;
using subedit::core::appendRun;
using subedit::core::Colour;
using subedit::core::DecodedMarkup;
using subedit::core::decodeHtmlMarkup;
using subedit::core::EncodedMarkup;
using subedit::core::encodeHtmlMarkup;
using subedit::core::Style;
using subedit::core::StyleAbilities;
using subedit::core::StyledText;
using subedit::core::SubtitleFormat;

/// The text written back out as SubRip, which of the three says the most.
[[nodiscard]] EncodedMarkup asSubRip(std::string_view text) {
    return encodeHtmlMarkup(decodeHtmlMarkup(text).runs, abilitiesOf(SubtitleFormat::SubRip));
}

} // namespace

TEST_CASE("the three one-letter tags are read, in either case", "[markup][html]") {
    const DecodedMarkup read = decodeHtmlMarkup("<B>gras</B> <i>penché</i> <U>souligné</U>");

    REQUIRE(read.runs.size() == 5);
    CHECK(read.runs[0].style == Style{.bold = true});
    CHECK(read.runs[1].style == Style{});
    CHECK(read.runs[2].style == Style{.italic = true});
    CHECK(read.runs[4].style == Style{.underline = true});
    CHECK(read.unknown == 0);
}

TEST_CASE("a font tag carries a colour and nothing else", "[markup][html]") {
    const DecodedMarkup read = decodeHtmlMarkup(R"(<font color="#ffff00">zone interdite</font>)");

    REQUIRE(read.runs.size() == 1);
    CHECK(read.runs.front().style.colour == Colour::parse("ffff00"));
}

TEST_CASE("nested tags give one run that says both things", "[markup][html]") {
    const DecodedMarkup read = decodeHtmlMarkup("<b><i>les deux</i></b>");

    REQUIRE(read.runs.size() == 1);
    CHECK(read.runs.front().style == Style{.bold = true, .italic = true});
}

TEST_CASE("a tag the pivot has no room for is dropped, and counted once", "[markup][html]") {
    // **The closing tag is not a second loss.** `<v Marie>…</v>` is one thing
    // gone, and a reader told two would go looking for the other.
    const DecodedMarkup read = decodeHtmlMarkup("<v Marie>Le canot dérive.</v>");

    REQUIRE(read.runs.size() == 1);
    CHECK(read.runs.front().text == "Le canot dérive.");
    CHECK(read.runs.front().style.isPlain());
    CHECK(read.unknown == 1);
}

TEST_CASE("an unterminated tag stops the reading rather than eating the text", "[markup][html]") {
    const DecodedMarkup read = decodeHtmlMarkup("le vent <i tombe");

    CHECK(subedit::core::plainTextOf(read.runs) == "le vent <i tombe");
}

TEST_CASE("what was read comes back as it was written", "[markup][html][roundtrip]") {
    for (const std::string_view text : {"<i>Le vent tomba d'un coup.</i>",
                                        R"(<b>ATTENTION</b> — <font color="#ffff00">zone</font>)",
                                        "<b><i>les deux</i></b>",
                                        "rien du tout"}) {
        INFO("texte : " << text);
        const EncodedMarkup written = asSubRip(text);
        CHECK(written.text == text);
        CHECK(written.dropped == 0);
    }
}

TEST_CASE("tags are closed in the order they were opened", "[markup][html]") {
    // A nesting that opens `<b>` then `<i>` cannot close `<b>` first, and the
    // encoder keeps its own stack rather than trusting the run order.
    const EncodedMarkup written = asSubRip("<b>gras <i>et penché</i> encore</b>");
    CHECK(written.text == "<b>gras <i>et penché</i> encore</b>");
}

TEST_CASE("a colour WebVTT cannot write is dropped, and counted", "[markup][html]") {
    const EncodedMarkup written = encodeHtmlMarkup(
        decodeHtmlMarkup(R"(<i>penché</i> et <font color="#ff0000">rouge</font>)").runs,
        abilitiesOf(SubtitleFormat::WebVtt));

    CHECK(written.text == "<i>penché</i> et rouge");
    CHECK(written.dropped == 1);
}

TEST_CASE("a run left with nothing to say opens no tag at all", "[markup][html]") {
    // The other half of dropping before composing: what is left has to look
    // like a text nobody styled, not like an empty pair of tags.
    const EncodedMarkup written =
        encodeHtmlMarkup(decodeHtmlMarkup(R"(<font color="#ff0000">rouge</font>)").runs,
                         abilitiesOf(SubtitleFormat::WebVtt));

    CHECK(written.text == "rouge");
    CHECK(written.dropped == 1);
}

TEST_CASE("a font tag that carries something else is not a colour", "[markup][html]") {
    // **The prefix has to match to the letter.** `<font size="3">` is as long
    // as `<font color="#`, and reading it as a colour would give a run painted
    // whatever the size happened to look like.
    const DecodedMarkup read = decodeHtmlMarkup(R"(<font size="3">grand</font>)");

    REQUIRE(read.runs.size() == 1);
    CHECK(read.runs.front().text == "grand");
    CHECK(read.runs.front().style.isPlain());
    CHECK(read.unknown == 1);
}

TEST_CASE("two colours in a row close and reopen the font tag", "[markup][html]") {
    // The two runs both carry a colour, so the open tag is still wanted — and
    // it is still the wrong one. What decides is the value, not the attribute.
    const std::string_view text =
        R"(<font color="#ff0000">rouge</font><font color="#00ff00">vert</font>)";
    const EncodedMarkup written = asSubRip(text);

    CHECK(written.text == text);
    CHECK(written.dropped == 0);
}

TEST_CASE("HTML writes no font and no size, even when allowed to keep them", "[markup][html]") {
    // **The vocabulary has no tag for either**, which is why no format that
    // speaks it is given the ability. Asked anyway, it writes the text and
    // nothing else — the loss is `abilitiesOf`'s to report, not this encoder's.
    StyledText runs;
    appendRun(runs, "titre", Style{.font = "Arial", .size = 20});

    const EncodedMarkup written = encodeHtmlMarkup(runs,
                                                   StyleAbilities{.bold = true,
                                                                  .italic = true,
                                                                  .underline = true,
                                                                  .colour = true,
                                                                  .font = true,
                                                                  .size = true});

    CHECK(written.text == "titre");
    CHECK(written.dropped == 0);
}
