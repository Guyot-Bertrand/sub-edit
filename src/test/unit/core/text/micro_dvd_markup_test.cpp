// The braced tags of MicroDVD, and the head-of-line markers of MPL2.
//
// Neither vocabulary has a tag that stops: the case says how far a MicroDVD tag
// reaches, and an MPL2 marker reaches the end of its line. That is what makes a
// style covering part of a line impossible to write here, and it is the one
// place the pivot has to give something up on the way out.

#include <subedit/core/model/subtitle_format.hpp>
#include <subedit/core/text/markup.hpp>
#include <subedit/core/text/markup_codec.hpp>
#include <subedit/core/text/markup_vocabulary.hpp>
#include <subedit/core/text/micro_dvd_markup.hpp>

#include <catch2/catch_test_macros.hpp>

#include <string>
#include <string_view>

namespace {

using subedit::core::abilitiesOf;
using subedit::core::appendRun;
using subedit::core::Colour;
using subedit::core::DecodedMarkup;
using subedit::core::decodeMicroDvdMarkup;
using subedit::core::decodeMpl2Markup;
using subedit::core::EncodedMarkup;
using subedit::core::encodeMicroDvdMarkup;
using subedit::core::encodeMpl2Markup;
using subedit::core::Style;
using subedit::core::SubtitleFormat;

[[nodiscard]] EncodedMarkup asMicroDvd(std::string_view text) {
    return encodeMicroDvdMarkup(decodeMicroDvdMarkup(text).runs,
                                abilitiesOf(SubtitleFormat::MicroDvd));
}

[[nodiscard]] EncodedMarkup asMpl2(std::string_view text) {
    return encodeMpl2Markup(decodeMpl2Markup(text).runs, abilitiesOf(SubtitleFormat::Mpl2));
}

} // namespace

TEST_CASE("a capital tag reaches the end of the subtitle, a lower-case one its line",
          "[markup][microdvd]") {
    const DecodedMarkup read =
        decodeMicroDvdMarkup("{Y:i}penché\net encore\n{y:b}gras ici\net plus");

    REQUIRE(read.runs.size() == 3);
    CHECK(read.runs[0].text == "penché\net encore\n");
    CHECK(read.runs[0].style == Style{.italic = true});
    CHECK(read.runs[1].style == Style{.bold = true, .italic = true});
    // The bold went with the line ending; the italic did not.
    CHECK(read.runs[2].style == Style{.italic = true});
}

TEST_CASE("one style tag holds a bag of letters", "[markup][microdvd]") {
    const DecodedMarkup read = decodeMicroDvdMarkup("{Y:biu}les trois");

    REQUIRE(read.runs.size() == 1);
    CHECK(read.runs.front().style == Style{.bold = true, .italic = true, .underline = true});
}

TEST_CASE("a colour is written the other way round, and read back the same", "[markup][microdvd]") {
    const DecodedMarkup read = decodeMicroDvdMarkup("{C:$00ffff}jaune");

    REQUIRE(read.runs.size() == 1);
    CHECK(read.runs.front().style.colour == Colour::parse("ffff00"));
}

TEST_CASE("a brace that says nothing this format knows is dropped, and counted",
          "[markup][microdvd]") {
    const DecodedMarkup read = decodeMicroDvdMarkup("{H:12}en haut");

    REQUIRE(read.runs.size() == 1);
    CHECK(read.runs.front().text == "en haut");
    CHECK(read.unknown == 1);
}

TEST_CASE("a style shared by every line is written once, in capitals",
          "[markup][microdvd][roundtrip]") {
    // What the MicroDVD rendering of the scene carries, and what it has to come
    // back as: one tag at the head, and nothing on the second line.
    const EncodedMarkup written = asMicroDvd("{Y:i}A gull turns once above the mast\nand settles.");

    CHECK(written.text == "{Y:i}A gull turns once above the mast\nand settles.");
    CHECK(written.dropped == 0);
}

TEST_CASE("a style covering one line of two is written on that line",
          "[markup][microdvd][roundtrip]") {
    const EncodedMarkup written = asMicroDvd("{y:i}penché\ndroit");

    CHECK(written.text == "{y:i}penché\ndroit");
    CHECK(written.dropped == 0);
}

TEST_CASE("a style that covers part of a line cannot be written here", "[markup][microdvd]") {
    // **The one loss this vocabulary imposes on its own**, and Gaupol takes the
    // same way out: there is no tag that stops, so a tag put in the middle
    // would say more than the source did. The line goes out plain.
    subedit::core::StyledText runs;
    appendRun(runs, "ATTENTION", Style{.bold = true});
    appendRun(runs, " — zone interdite", Style{});

    const EncodedMarkup written = encodeMicroDvdMarkup(runs, abilitiesOf(SubtitleFormat::MicroDvd));

    CHECK(written.text == "ATTENTION — zone interdite");
    CHECK(written.dropped == 1);
}

TEST_CASE("a style covering one whole line of two survives beside a bare one",
          "[markup][microdvd]") {
    // The rule is « whole lines », not « the whole subtitle »: the first line
    // keeps its italics because nothing on it disagrees.
    subedit::core::StyledText runs;
    appendRun(runs, "penché\n", Style{.italic = true});
    appendRun(runs, "droit", Style{});

    const EncodedMarkup written = encodeMicroDvdMarkup(runs, abilitiesOf(SubtitleFormat::MicroDvd));

    CHECK(written.text == "{y:i}penché\ndroit");
    CHECK(written.dropped == 0);
}

TEST_CASE("MPL2 repeats its marker on every line it covers", "[markup][mpl2][roundtrip]") {
    const EncodedMarkup written = asMpl2("/A gull turns once above the mast\n/and settles.");

    CHECK(written.text == "/A gull turns once above the mast\n/and settles.");
    CHECK(written.dropped == 0);
}

TEST_CASE("MPL2 markers pile up, and are read as a set", "[markup][mpl2]") {
    const DecodedMarkup read = decodeMpl2Markup("/_penché et souligné");

    REQUIRE(read.runs.size() == 1);
    CHECK(read.runs.front().text == "penché et souligné");
    CHECK(read.runs.front().style == Style{.italic = true, .underline = true});
}

TEST_CASE("MPL2 keeps MicroDVD's braces for what it has no marker for",
          "[markup][mpl2][roundtrip]") {
    // The marker stays at the head of the line, where MPL2 looks for it, and
    // the brace follows. A single line is the whole subtitle, so the brace is
    // the capital one.
    const EncodedMarkup written = asMpl2("/{C:$00ffff}jaune et penché");

    CHECK(written.text == "/{C:$00ffff}jaune et penché");
    CHECK(written.dropped == 0);
}

TEST_CASE("two lines that disagree each carry their own brace", "[markup][mpl2][roundtrip]") {
    const EncodedMarkup written = asMpl2("{c:$00ffff}jaune\n{c:$0000ff}rouge");

    CHECK(written.text == "{c:$00ffff}jaune\n{c:$0000ff}rouge");
    CHECK(written.dropped == 0);
}

TEST_CASE("a lower-case tag reaches the end of its line and no further", "[markup][microdvd]") {
    // **The line scope, on the three attributes that carry a value.** The
    // capital form is what survives the line ending; this one is written over
    // the subtitle scope for one line and then forgotten.
    const DecodedMarkup read = decodeMicroDvdMarkup("{f:Arial}{s:20}{c:$0000ff}titre\nsuite");

    REQUIRE(read.runs.size() == 2);
    CHECK(read.runs.front().text == "titre\n");
    CHECK(read.runs.front().style.font == std::string{"Arial"});
    CHECK(read.runs.front().style.size == 20);
    CHECK(read.runs.front().style.colour == Colour::parse("ff0000"));
    CHECK(read.runs.back().text == "suite");
    CHECK(read.runs.back().style.isPlain());
    CHECK(read.unknown == 0);
}

TEST_CASE("a braced tag whose value is not a value says nothing", "[markup][microdvd]") {
    // Each has the shape of a tag and no meaning: a `{y:}` naming none of the
    // three flags, a colour without its dollar or without its digits, a font
    // with no name, a size that is not a number.
    for (const std::string_view text :
         {"{y:x}", "{c:0000ff}", "{c:$zz}", "{f:}", "{s:}", "{s:2a}"}) {
        INFO("texte : " << text);
        const DecodedMarkup read = decodeMicroDvdMarkup(std::string{text} + "le vent");

        CHECK(read.unknown == 1);
        REQUIRE(read.runs.size() == 1);
        CHECK(read.runs.front().style.isPlain());
    }
}

TEST_CASE("an unterminated brace stops the reading rather than eating the text",
          "[markup][microdvd]") {
    const DecodedMarkup read = decodeMicroDvdMarkup("le vent {Y:i tombe");

    CHECK(subedit::core::plainTextOf(read.runs) == "le vent {Y:i tombe");
    CHECK(read.unknown == 0);
}
