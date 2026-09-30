// The em-based `LineMeasure` — issue #503, decision D5 of the spec of
// phase 12.
//
// **Authoritative only under DejaVu Sans** — ADR 0024's own rule for
// anything a font renders: a length measured under a stand-in font is a
// length nobody else's machine would reproduce.

#include <subedit/core/text/correction_pattern.hpp>
#include <subedit/core/text/icu_pattern_engine.hpp>
#include <subedit/core/text/line_breaking.hpp>
#include <subedit/core/text/line_measure.hpp>
#include <subedit/gui/ems_line_measure.hpp>

#include <QFont>
#include <QFontInfo>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <memory>
#include <string>
#include <vector>

namespace {

using subedit::core::BrokenTexts;
using subedit::core::CharacterLineMeasure;
using subedit::core::CorrectionPattern;
using subedit::core::IcuPatternEngine;
using subedit::gui::EmsLineMeasure;

/// DejaVu Sans 10pt, refused rather than measured under a stand-in — the
/// same font and the same refusal `screenshots.cpp` uses.
QFont dejaVuSansOrSkip() {
    const QFont font{QStringLiteral("DejaVu Sans"), 10};
    if (QFontInfo{font}.family() != QStringLiteral("DejaVu Sans"))
        SKIP("la police DejaVu Sans est absente (paquet fonts-dejavu-core)");
    return font;
}

} // namespace

TEST_CASE("the lowercase alphabet is 14.3 em by calibration", "[gui][line-break]") {
    const EmsLineMeasure measure{dejaVuSansOrSkip()};
    CHECK_THAT(measure.lengthOf("abcdefghijklmnopqrstuvwxyz"),
               Catch::Matchers::WithinAbs(14.3, 0.01));
}

TEST_CASE("known lengths are rendered, under DejaVu Sans 10pt", "[gui][line-break]") {
    const EmsLineMeasure measure{dejaVuSansOrSkip()};

    CHECK_THAT(measure.lengthOf(""), Catch::Matchers::WithinAbs(0.0, 0.01));
    CHECK_THAT(measure.lengthOf("Hello there"), Catch::Matchers::WithinAbs(5.3672, 0.01));
    CHECK_THAT(measure.lengthOf("iiii"), Catch::Matchers::WithinAbs(1.0852, 0.01));
    CHECK_THAT(measure.lengthOf("MMMM"), Catch::Matchers::WithinAbs(3.3683, 0.01));
}

TEST_CASE("iiii and MMMM differ in ems, and not in characters", "[gui][line-break]") {
    const EmsLineMeasure ems{dejaVuSansOrSkip()};

    // Four narrow letters against four wide ones: a character count could
    // never tell them apart, and a real font never confuses them.
    CHECK(ems.lengthOf("iiii") < ems.lengthOf("MMMM"));
}

TEST_CASE("breaking a text in ems chooses a different split than in characters",
          "[gui][line-break]") {
    // Six words of four letters each, so a character count sees them as
    // identical boxes; three are narrow, three are wide, so ems does not.
    const std::vector<std::string> texts{"iiii iiii iiii MMMM MMMM MMMM"};
    const std::vector<const CorrectionPattern*> noPatterns;
    const IcuPatternEngine engine;

    const CharacterLineMeasure characters;
    const BrokenTexts byCharacters =
        subedit::core::breakLines(engine, noPatterns, texts, characters, 15, 2);

    const EmsLineMeasure ems{dejaVuSansOrSkip()};
    const BrokenTexts byEms = subedit::core::breakLines(engine, noPatterns, texts, ems, 15, 2);

    CHECK(byCharacters.texts != byEms.texts);
}

TEST_CASE("the assistant measures in ems through a cache, in characters without one",
          "[gui][line-break]") {
    // The measure the assistant hands to the line-breaker is the one that
    // must remember: a `dynamic_cast` and a count of what it holds are how a
    // test tells it from a bare `EmsLineMeasure`.
    const QFont font = dejaVuSansOrSkip();

    const std::shared_ptr<const subedit::core::LineMeasure> ems =
        subedit::gui::assistantLineMeasure(true, font);
    const auto* cached = dynamic_cast<const subedit::gui::CachedEmsLineMeasure*>(ems.get());
    REQUIRE(cached != nullptr);
    CHECK(cached->cachedCount() == 0);

    const double first = ems->lengthOf("Hello there");
    const double again = ems->lengthOf("Hello there");
    CHECK(cached->cachedCount() == 1);
    CHECK(first == again);
    CHECK_THAT(first, Catch::Matchers::WithinAbs(5.3672, 0.01));

    CHECK(ems->lengthOf("Another line") > 0.0);
    CHECK(cached->cachedCount() == 2);

    const std::shared_ptr<const subedit::core::LineMeasure> characters =
        subedit::gui::assistantLineMeasure(false, font);
    CHECK(dynamic_cast<const CharacterLineMeasure*>(characters.get()) != nullptr);
    CHECK(characters->lengthOf("Hello") == 5.0);
}
