// The pivot, seen from the only place it is used: a conversion between formats.
//
// **Five vocabularies make twenty ordered pairs**, and this file does not walk
// them one by one. What it walks is what the issue promises: each of the six
// attributes, from every format that can write it to every format that can, and
// back. A table of pairs would say the same thing at four times the length.

#include <subedit/core/model/subtitle_format.hpp>
#include <subedit/core/text/markup_conversion.hpp>
#include <subedit/core/wording.hpp>

#include <catch2/catch_test_macros.hpp>

#include <array>
#include <span>
#include <string>
#include <string_view>

namespace {

using subedit::core::ConvertedMarkup;
using subedit::core::convertMarkup;
using subedit::core::nameOf;
using subedit::core::SubtitleFormat;

/// How one attribute is written in each of the vocabularies that has it.
///
/// **Written out rather than derived.** These strings are the whole subject:
/// deriving them from the encoder would compare it to itself, and the point of
/// the pivot is that a human can read what each format says.
struct Rendering {
    SubtitleFormat format;
    std::string_view text;
};

/// The formats that carry nothing, and give back the text alone.
constexpr std::array<SubtitleFormat, 2> kSilent = {SubtitleFormat::TMPlayer, SubtitleFormat::Lrc};

/// Asserts that every rendering of one thing crosses into every other.
void crossesBoth(const std::span<const Rendering> renderings, std::string_view plain) {
    for (const Rendering& from : renderings) {
        for (const Rendering& to : renderings) {
            INFO("de " << nameOf(from.format) << " vers " << nameOf(to.format));
            const ConvertedMarkup carried = convertMarkup(from.text, from.format, to.format);
            CHECK(carried.text == to.text);
            CHECK(carried.dropped == 0);
        }

        for (const SubtitleFormat silent : kSilent) {
            INFO("de " << nameOf(from.format) << " vers " << nameOf(silent));
            const ConvertedMarkup carried = convertMarkup(from.text, from.format, silent);
            CHECK(carried.text == plain);
            CHECK(carried.dropped == 1);
        }
    }
}

} // namespace

TEST_CASE("italics cross every vocabulary that has them", "[markup][conversion]") {
    constexpr std::array<Rendering, 5> kRenderings = {
        Rendering{.format = SubtitleFormat::SubRip, .text = "<i>penché</i>"},
        Rendering{.format = SubtitleFormat::SubViewer2, .text = "<i>penché</i>"},
        Rendering{.format = SubtitleFormat::AdvancedSubStationAlpha, .text = R"({\i1}penché{\i0})"},
        Rendering{.format = SubtitleFormat::MicroDvd, .text = "{Y:i}penché"},
        Rendering{.format = SubtitleFormat::Mpl2, .text = "/penché"},
    };
    crossesBoth(kRenderings, "penché");
}

TEST_CASE("bold crosses every vocabulary that has it", "[markup][conversion]") {
    constexpr std::array<Rendering, 4> kRenderings = {
        Rendering{.format = SubtitleFormat::SubRip, .text = "<b>gras</b>"},
        Rendering{.format = SubtitleFormat::AdvancedSubStationAlpha, .text = R"({\b1}gras{\b0})"},
        Rendering{.format = SubtitleFormat::MicroDvd, .text = "{Y:b}gras"},
        Rendering{.format = SubtitleFormat::Mpl2, .text = "\\gras"},
    };
    crossesBoth(kRenderings, "gras");
}

TEST_CASE("underline crosses every vocabulary that has it", "[markup][conversion]") {
    // Sub Station Alpha is missing from this list on purpose: `{\u1}` is
    // Advanced SSA's, and Gaupol writes it for one of the two formats only.
    constexpr std::array<Rendering, 4> kRenderings = {
        Rendering{.format = SubtitleFormat::SubRip, .text = "<u>souligné</u>"},
        Rendering{.format = SubtitleFormat::AdvancedSubStationAlpha,
                  .text = R"({\u1}souligné{\u0})"},
        Rendering{.format = SubtitleFormat::MicroDvd, .text = "{Y:u}souligné"},
        Rendering{.format = SubtitleFormat::Mpl2, .text = "_souligné"},
    };
    crossesBoth(kRenderings, "souligné");
}

TEST_CASE("a colour crosses, channels reversed where that is the custom", "[markup][conversion]") {
    constexpr std::array<Rendering, 4> kRenderings = {
        Rendering{.format = SubtitleFormat::SubRip,
                  .text = R"(<font color="#ffff00">jaune</font>)"},
        Rendering{.format = SubtitleFormat::AdvancedSubStationAlpha,
                  .text = R"({\c&H00ffff&}jaune)"},
        Rendering{.format = SubtitleFormat::MicroDvd, .text = "{C:$00ffff}jaune"},
        Rendering{.format = SubtitleFormat::Mpl2, .text = "{C:$00ffff}jaune"},
    };
    crossesBoth(kRenderings, "jaune");
}

TEST_CASE("a font and a size cross the two vocabularies that have them", "[markup][conversion]") {
    // **Only the braces carry either**, which is why this pair is shorter than
    // the others: SubRip's `<font>` holds a colour and nothing else here.
    constexpr std::array<Rendering, 3> kFonts = {
        Rendering{.format = SubtitleFormat::AdvancedSubStationAlpha, .text = R"({\fnArial}écrit)"},
        Rendering{.format = SubtitleFormat::MicroDvd, .text = "{F:Arial}écrit"},
        Rendering{.format = SubtitleFormat::Mpl2, .text = "{F:Arial}écrit"},
    };
    crossesBoth(kFonts, "écrit");

    constexpr std::array<Rendering, 3> kSizes = {
        Rendering{.format = SubtitleFormat::AdvancedSubStationAlpha, .text = R"({\fs24}écrit)"},
        Rendering{.format = SubtitleFormat::MicroDvd, .text = "{S:24}écrit"},
        Rendering{.format = SubtitleFormat::Mpl2, .text = "{S:24}écrit"},
    };
    crossesBoth(kSizes, "écrit");
}

TEST_CASE("a text going nowhere is not touched", "[markup][conversion]") {
    // **ADR 0009 and ADR 0031 meet here.** Saving a file in its own format is
    // not a conversion, so the raw string stays raw — tags nobody understands
    // included, which is what makes `convert --to srt` on a `.srt` exact.
    constexpr std::string_view kOpaque = "<v Marie>{\\pos(3,4)}intact";

    const ConvertedMarkup carried =
        convertMarkup(kOpaque, SubtitleFormat::SubRip, SubtitleFormat::SubRip);

    CHECK(carried.text == kOpaque);
    CHECK(carried.dropped == 0);
}

TEST_CASE("what the arriving format cannot write is dropped, and counted", "[markup][conversion]") {
    // The pair the abilities exist for: same tags, one colour fewer.
    const ConvertedMarkup carried =
        convertMarkup(R"(<i>penché</i> et <font color="#ff0000">rouge</font>)",
                      SubtitleFormat::SubRip,
                      SubtitleFormat::WebVtt);

    CHECK(carried.text == "<i>penché</i> et rouge");
    CHECK(carried.dropped == 1);
}

TEST_CASE("layout is dropped on the way in, and counted there", "[markup][conversion]") {
    const ConvertedMarkup carried = convertMarkup(R"({\an8}{\i1}en haut{\i0})",
                                                  SubtitleFormat::AdvancedSubStationAlpha,
                                                  SubtitleFormat::SubRip);

    CHECK(carried.text == "<i>en haut</i>");
    CHECK(carried.dropped == 1);
}

TEST_CASE("a brace never reaches a file that would show it as text", "[markup][conversion]") {
    // **The sentence that decided the ADR**, asserted rather than quoted: an
    // SSA converted to SubRip must not carry `{\i1}` into a file that will
    // never interpret it. That is not a loss, it is text the user would see.
    const ConvertedMarkup carried = convertMarkup(
        R"({\i1}Il ne dit rien.{\i0})", SubtitleFormat::SubStationAlpha, SubtitleFormat::SubRip);

    CHECK(carried.text == "<i>Il ne dit rien.</i>");
    CHECK_FALSE(carried.text.contains("{"));
}

TEST_CASE("a format with no vocabulary carries its text out whole", "[markup][conversion]") {
    // **Nothing in an LRC line is a tag**, so nothing in it is read as one: the
    // `/` that MPL2 would take for an italic marker is a slash, and the angle
    // brackets a hand-written file carries are angle brackets. Coming out of
    // silence, a conversion loses nothing because there was nothing to lose.
    for (const SubtitleFormat from : kSilent) {
        INFO("de " << nameOf(from));
        const ConvertedMarkup carried =
            convertMarkup("/deux <i> trois", from, SubtitleFormat::SubRip);

        CHECK(carried.text == "/deux <i> trois");
        CHECK(carried.dropped == 0);
    }
}
