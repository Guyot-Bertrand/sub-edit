// Dialogue dashes: the corpus for putting them on, and the tests for the rest.
//
// The `.cas` format says what a text becomes, which fits the putting on. Which
// of the two ways the single entry goes, and what taking them off gives, need a
// second question the format does not carry — so they live here.

#include <subedit/core/model/subtitle_format.hpp>
#include <subedit/core/text/dialogue_dashes.hpp>

#include <catch2/catch_test_macros.hpp>

#include <optional>
#include <string>
#include <text_cases.hpp>

namespace {

using subedit::core::SubtitleFormat;
using subedit::core::wantsDialogueDashes;
using subedit::core::withDialogueDashes;
using subedit::core::withoutDialogueDashes;
using subedit::test::checkTextCases;
using subedit::test::textCasesOf;

} // namespace

TEST_CASE("the corpus of putting dashes on passes", "[text][dashes]") {
    checkTextCases(textCasesOf("textes/tirets.cas"), [](const std::string& text) {
        return std::optional<std::string>{withDialogueDashes(text, SubtitleFormat::SubRip)};
    });
}

TEST_CASE("one line without a dash sends the whole text into them", "[text][dashes]") {
    // Gaupol's rule, and the one the italic toggle already follows: a single
    // entry usable in both directions needs the target to go one way whole.
    CHECK(wantsDialogueDashes("- Bonjour\nMarie", SubtitleFormat::SubRip));
    CHECK(wantsDialogueDashes("Bonjour\nMarie", SubtitleFormat::SubRip));
    CHECK_FALSE(wantsDialogueDashes("- Bonjour\n- Marie", SubtitleFormat::SubRip));

    // A tag says nothing about it: what opens the line is the visible text.
    CHECK_FALSE(wantsDialogueDashes("<i>- Bonjour</i>", SubtitleFormat::SubRip));
    CHECK(wantsDialogueDashes("<i>Bonjour</i>", SubtitleFormat::SubRip));

    // An empty line is no replica at all, and asks for nothing.
    CHECK_FALSE(wantsDialogueDashes("- Bonjour\n", SubtitleFormat::SubRip));
}

TEST_CASE("taking the dashes off takes their spaces with them", "[text][dashes]") {
    CHECK(withoutDialogueDashes("- Bonjour\n- Marie", SubtitleFormat::SubRip) == "Bonjour\nMarie");
    CHECK(withoutDialogueDashes("—   Bonjour", SubtitleFormat::SubRip) == "Bonjour");
    CHECK(withoutDialogueDashes("<i>- Bonjour</i>", SubtitleFormat::SubRip) == "<i>Bonjour</i>");
}

TEST_CASE("pressing twice leaves the text as it was found", "[text][dashes]") {
    constexpr const char* kPlain = "Bonjour\nMarie";
    const std::string on = withDialogueDashes(kPlain, SubtitleFormat::SubRip);

    CHECK(on == "- Bonjour\n- Marie");
    CHECK(withoutDialogueDashes(on, SubtitleFormat::SubRip) == kPlain);
}

TEST_CASE("a text nothing changed comes back as the bytes it was", "[text][dashes]") {
    constexpr const char* kOdd = "<i >- Bonjour</I >";

    CHECK(withDialogueDashes(kOdd, SubtitleFormat::SubRip) == kOdd);
    CHECK(withoutDialogueDashes("Bonjour", SubtitleFormat::SubRip) == "Bonjour");
}
