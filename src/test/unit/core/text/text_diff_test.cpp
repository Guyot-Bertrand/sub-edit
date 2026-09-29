#include <subedit/core/text/text_diff.hpp>

#include <catch2/catch_test_macros.hpp>

#include <string>

namespace {
using subedit::core::diffTexts;
} // namespace

TEST_CASE("identical texts answer as one unchanged span each", "[text][diff]") {
    const auto diff = diffTexts("Bonjour", "Bonjour");

    REQUIRE(diff.original.size() == 1);
    CHECK_FALSE(diff.original[0].changed);
    CHECK(diff.original[0].text == "Bonjour");
    REQUIRE(diff.proposed.size() == 1);
    CHECK_FALSE(diff.proposed[0].changed);
    CHECK(diff.proposed[0].text == "Bonjour");
}

TEST_CASE("two texts with nothing in common are each one changed span", "[text][diff]") {
    const auto diff = diffTexts("abc", "xyz");

    REQUIRE(diff.original.size() == 1);
    CHECK(diff.original[0].changed);
    CHECK(diff.original[0].text == "abc");
    REQUIRE(diff.proposed.size() == 1);
    CHECK(diff.proposed[0].changed);
    CHECK(diff.proposed[0].text == "xyz");
}

TEST_CASE("text appended at the end is its own changed span, and keeps a "
          "multi-byte character whole",
          "[text][diff]") {
    const auto diff = diffTexts("café", "cafés");

    REQUIRE(diff.original.size() == 1);
    CHECK_FALSE(diff.original[0].changed);
    CHECK(diff.original[0].text == "café");

    REQUIRE(diff.proposed.size() == 2);
    CHECK_FALSE(diff.proposed[0].changed);
    CHECK(diff.proposed[0].text == "café");
    CHECK(diff.proposed[1].changed);
    CHECK(diff.proposed[1].text == "s");
}

TEST_CASE("text removed from the end is its own changed span, on the original only",
          "[text][diff]") {
    const auto diff = diffTexts("Bonjour!", "Bonjour");

    REQUIRE(diff.original.size() == 2);
    CHECK_FALSE(diff.original[0].changed);
    CHECK(diff.original[0].text == "Bonjour");
    CHECK(diff.original[1].changed);
    CHECK(diff.original[1].text == "!");

    REQUIRE(diff.proposed.size() == 1);
    CHECK_FALSE(diff.proposed[0].changed);
    CHECK(diff.proposed[0].text == "Bonjour");
}

TEST_CASE("three- and four-byte codepoints are kept whole, not split at a byte", "[text][diff]") {
    // "…" is three UTF-8 bytes (U+2026), "🎬" is four (U+1F3AC) — the two
    // widths `codepointsOf` decodes beyond the two-byte "é" the tests above
    // already cover.
    const auto ellipsis = diffTexts("Bonjour…", "Bonjour");
    REQUIRE(ellipsis.original.size() == 2);
    CHECK(ellipsis.original[1].changed);
    CHECK(ellipsis.original[1].text == "…");

    const auto emoji = diffTexts("Bonjour🎬", "Bonjour");
    REQUIRE(emoji.original.size() == 2);
    CHECK(emoji.original[1].changed);
    CHECK(emoji.original[1].text == "🎬");
}

TEST_CASE("an empty text on one side answers as no spans on that side", "[text][diff]") {
    const auto diff = diffTexts("", "Bonjour");

    CHECK(diff.original.empty());
    REQUIRE(diff.proposed.size() == 1);
    CHECK(diff.proposed[0].changed);
    CHECK(diff.proposed[0].text == "Bonjour");
}
