// Whether a pattern is enabled, decision D2 — promoted out of
// `correction_run.cpp`'s private `isEnabled` so the pattern-list widget of
// the correction assistant (issue #505) can open its boxes on the same rule.

#include <subedit/core/config/correction_settings.hpp>
#include <subedit/core/text/correction_pattern.hpp>

#include <catch2/catch_test_macros.hpp>

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
