// Whether a pattern is enabled, decision D2 — promoted out of
// `correction_run.cpp`'s private `isEnabled` so the pattern-list widget of
// the correction assistant (issue #505) can open its boxes on the same rule.

#include <subedit/core/config/correction_settings.hpp>
#include <subedit/core/text/correction_pattern.hpp>

#include <catch2/catch_test_macros.hpp>

#include <string>
#include <utility>
#include <vector>

namespace {
using subedit::core::CorrectionPattern;
using subedit::core::CorrectionSettings;
using subedit::core::PatternActivation;
using subedit::core::patternEnabled;
using subedit::core::PatternKind;
} // namespace

TEST_CASE("a pattern with no override keeps its shipped default", "[core][pattern-activation]") {
    CorrectionPattern pattern;
    pattern.code = "Zyyy";
    pattern.name = "Letter I";
    pattern.enabled = false;

    CHECK_FALSE(patternEnabled(pattern, CorrectionSettings{}));
}

TEST_CASE("an override by kind, code and name wins over the shipped default",
          "[core][pattern-activation]") {
    CorrectionPattern pattern;
    pattern.code = "Zyyy";
    pattern.name = "Letter I";
    pattern.fields = subedit::core::CommonErrorFields{};
    pattern.enabled = false;

    CorrectionSettings settings;
    settings.patternActivations.push_back(PatternActivation{
        .kind = PatternKind::CommonError, .code = "Zyyy", .name = "Letter I", .enabled = true});

    CHECK(patternEnabled(pattern, settings));
}

TEST_CASE("an override naming a different code does not apply", "[core][pattern-activation]") {
    CorrectionPattern pattern;
    pattern.code = "Latn-en";
    pattern.name = "Letter I";
    pattern.fields = subedit::core::CommonErrorFields{};
    pattern.enabled = true;

    CorrectionSettings settings;
    settings.patternActivations.push_back(PatternActivation{
        .kind = PatternKind::CommonError, .code = "Zyyy", .name = "Letter I", .enabled = false});

    CHECK(patternEnabled(pattern, settings));
}

// `foldActivations` — issue #505's final review: an override must be able to
// go away again, not only to be written.

namespace {

[[nodiscard]] CorrectionPattern commonError(std::string code, std::string name, bool enabled) {
    CorrectionPattern pattern;
    pattern.code = std::move(code);
    pattern.name = std::move(name);
    pattern.fields = subedit::core::CommonErrorFields{};
    pattern.enabled = enabled;
    return pattern;
}

} // namespace

TEST_CASE("folding a shown record back to its default erases its old override",
          "[core][pattern-activation]") {
    const CorrectionPattern letterI = commonError("Zyyy", "Letter I", true);
    std::vector<PatternActivation> activations{PatternActivation{
        .kind = PatternKind::CommonError, .code = "Zyyy", .name = "Letter I", .enabled = false}};
    const std::vector<const CorrectionPattern*> shown{&letterI};

    subedit::core::foldActivations(activations, shown, {}); // box back on: no override reported

    CHECK(activations.empty());
}

TEST_CASE("folding replaces a shown record's override rather than adding a second",
          "[core][pattern-activation]") {
    const CorrectionPattern letterI = commonError("Zyyy", "Letter I", false);
    std::vector<PatternActivation> activations{PatternActivation{
        .kind = PatternKind::CommonError, .code = "Zyyy", .name = "Letter I", .enabled = false}};
    const std::vector<const CorrectionPattern*> shown{&letterI};
    const std::vector<PatternActivation> overrides{PatternActivation{
        .kind = PatternKind::CommonError, .code = "Zyyy", .name = "Letter I", .enabled = true}};

    subedit::core::foldActivations(activations, shown, overrides);

    CHECK(activations == overrides);
}

TEST_CASE("folding leaves the overrides of records not shown alone", "[core][pattern-activation]") {
    const CorrectionPattern shownRecord = commonError("Latn-en", "Letter I", true);
    const PatternActivation otherCode{
        .kind = PatternKind::CommonError, .code = "Zyyy", .name = "Letter I", .enabled = false};
    const PatternActivation otherKind{.kind = PatternKind::Capitalization,
                                      .code = "Latn-en",
                                      .name = "Letter I",
                                      .enabled = false};
    std::vector<PatternActivation> activations{otherCode, otherKind};
    const std::vector<const CorrectionPattern*> shown{&shownRecord};

    subedit::core::foldActivations(activations, shown, {});

    CHECK(activations == std::vector<PatternActivation>{otherCode, otherKind});
}
