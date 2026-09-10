// The override blocks of Sub Station Alpha and Advanced SSA.
//
// `{\i1}…{\i0}` closes, `{\c&HBBGGRR&}` does not, and `{\r}` closes everything
// at once. A block is read as a change of state, which is what the format means.

#include <subedit/core/model/subtitle_format.hpp>
#include <subedit/core/text/markup.hpp>
#include <subedit/core/text/markup_codec.hpp>
#include <subedit/core/text/markup_vocabulary.hpp>
#include <subedit/core/text/sub_station_alpha_markup.hpp>

#include <catch2/catch_test_macros.hpp>

#include <string>
#include <string_view>

namespace {

using subedit::core::abilitiesOf;
using subedit::core::Colour;
using subedit::core::DecodedMarkup;
using subedit::core::decodeSubStationAlphaMarkup;
using subedit::core::EncodedMarkup;
using subedit::core::encodeSubStationAlphaMarkup;
using subedit::core::Style;
using subedit::core::SubtitleFormat;

[[nodiscard]] EncodedMarkup asAdvanced(std::string_view text) {
    return encodeSubStationAlphaMarkup(decodeSubStationAlphaMarkup(text).runs,
                                       abilitiesOf(SubtitleFormat::AdvancedSubStationAlpha));
}

} // namespace

TEST_CASE("an override turns something on until another turns it off", "[markup][ssa]") {
    const DecodedMarkup read = decodeSubStationAlphaMarkup(R"({\i1}penché{\i0} droit)");

    REQUIRE(read.runs.size() == 2);
    CHECK(read.runs[0].style == Style{.italic = true});
    CHECK(read.runs[1].style.isPlain());
    CHECK(read.unknown == 0);
}

TEST_CASE("a weight above zero is bold, and zero is not", "[markup][ssa]") {
    // Advanced SSA writes `{\b700}` where Sub Station Alpha writes `{\b1}`.
    const DecodedMarkup read = decodeSubStationAlphaMarkup(R"({\b700}lourd{\b0}léger)");

    REQUIRE(read.runs.size() == 2);
    CHECK(read.runs[0].style.bold);
    CHECK_FALSE(read.runs[1].style.bold);
}

TEST_CASE("one block may say several things at once", "[markup][ssa]") {
    const DecodedMarkup read = decodeSubStationAlphaMarkup(R"({\b1\i1}les deux)");

    REQUIRE(read.runs.size() == 1);
    CHECK(read.runs.front().style == Style{.bold = true, .italic = true});
}

TEST_CASE("a colour runs to the end, having no override that closes it", "[markup][ssa]") {
    const DecodedMarkup read = decodeSubStationAlphaMarkup(R"({\c&H00ffff&}jaune, et encore)");

    REQUIRE(read.runs.size() == 1);
    CHECK(read.runs.front().style.colour == Colour::parse("ffff00"));
}

TEST_CASE("the reset closes everything, colour included", "[markup][ssa]") {
    const DecodedMarkup read =
        decodeSubStationAlphaMarkup(R"({\i1}{\c&H00ffff&}les deux{\r}plus rien)");

    REQUIRE(read.runs.size() == 2);
    CHECK(read.runs[0].style.italic);
    CHECK(read.runs[0].style.colour.has_value());
    CHECK(read.runs[1].style.isPlain());
}

TEST_CASE("a font and a size are read from their own overrides", "[markup][ssa]") {
    const DecodedMarkup read = decodeSubStationAlphaMarkup(R"({\fnArial}{\fs24}écrit)");

    REQUIRE(read.runs.size() == 1);
    CHECK(read.runs.front().style.font == std::string{"Arial"});
    CHECK(read.runs.front().style.size == 24);
}

TEST_CASE("an override the pivot has no room for is dropped, and counted", "[markup][ssa]") {
    // The layout ADR 0031 leaves out, and it is the commonest thing in a real
    // Advanced SSA file.
    const DecodedMarkup read = decodeSubStationAlphaMarkup(R"({\an8}{\pos(320,20)}en haut)");

    REQUIRE(read.runs.size() == 1);
    CHECK(read.runs.front().text == "en haut");
    CHECK(read.unknown == 2);
}

TEST_CASE("what was read comes back as it was written", "[markup][ssa][roundtrip]") {
    for (const std::string_view text : {R"({\i1}penché{\i0})",
                                        R"({\b1}gras{\b0} — {\c&H00ffff&}jaune)",
                                        R"({\fnArial}{\fs24}écrit)",
                                        "rien du tout"}) {
        INFO("texte : " << text);
        const EncodedMarkup written = asAdvanced(text);
        CHECK(written.text == text);
        CHECK(written.dropped == 0);
    }
}

TEST_CASE("an underline is written by Advanced SSA and dropped by the other", "[markup][ssa]") {
    const DecodedMarkup read = decodeSubStationAlphaMarkup(R"({\u1}souligné{\u0})");

    CHECK(
        encodeSubStationAlphaMarkup(read.runs, abilitiesOf(SubtitleFormat::AdvancedSubStationAlpha))
            .text == R"({\u1}souligné{\u0})");

    const EncodedMarkup plainer =
        encodeSubStationAlphaMarkup(read.runs, abilitiesOf(SubtitleFormat::SubStationAlpha));
    CHECK(plainer.text == "souligné");
    CHECK(plainer.dropped == 1);
}

TEST_CASE("a colour that has to go takes a reset with it", "[markup][ssa]") {
    // **There is no override that unsets a colour**, so the only way back to
    // plain text is to close everything and say again what is still wanted.
    const EncodedMarkup written = asAdvanced(R"({\i1}{\c&H00ffff&}jaune{\r}{\i1}penché{\i0})");

    CHECK(written.text == R"({\i1}{\c&H00ffff&}jaune{\r}{\i1}penché{\i0})");
}

TEST_CASE("an override whose value is not a value says nothing", "[markup][ssa]") {
    // **Each of these has the shape of an override and no meaning**, so each is
    // one thing lost and counted. A `{\fs}` that silently became a size would
    // be a number invented where the file wrote none.
    for (const std::string_view text :
         {R"({\})", R"({\b1x})", R"({\c&Hzz&})", R"({\fs})", R"({\fs12a})"}) {
        INFO("texte : " << text);
        const DecodedMarkup read = decodeSubStationAlphaMarkup(std::string{text} + "le vent");

        CHECK(read.unknown == 1);
        REQUIRE(read.runs.size() == 1);
        CHECK(read.runs.front().style.isPlain());
    }
}

TEST_CASE("a block that is not an override at all is one thing lost", "[markup][ssa]") {
    // `{some note}` is what a hand-edited file carries, and it is not a series
    // of overrides: the block is abandoned whole rather than read letter by
    // letter.
    const DecodedMarkup read = decodeSubStationAlphaMarkup("{une note}le vent");

    CHECK(read.unknown == 1);
    REQUIRE(read.runs.size() == 1);
    CHECK(read.runs.front().text == "le vent");
}

TEST_CASE("an unterminated block stops the reading rather than eating the text", "[markup][ssa]") {
    const DecodedMarkup read = decodeSubStationAlphaMarkup(R"(le vent {\i1 tombe)");

    CHECK(subedit::core::plainTextOf(read.runs) == R"(le vent {\i1 tombe)");
    CHECK(read.unknown == 0);
}
