// The form of `Adjust Durations…` and the request it makes — issue #409.

#include <subedit/core/config/duration_adjustment_settings.hpp>
#include <subedit/core/edit/duration_adjustment.hpp>
#include <subedit/core/time/duration.hpp>

#include <catch2/catch_test_macros.hpp>

namespace {

using subedit::core::constraintsOf;
using subedit::core::Duration;
using subedit::core::DurationAdjustmentSettings;
using subedit::core::DurationConstraints;
using subedit::core::ReadingSpeed;

} // namespace

TEST_CASE("the form's defaults make Gaupol's request", "[config][durations]") {
    CHECK(constraintsOf(DurationAdjustmentSettings{}) == DurationConstraints{});
}

TEST_CASE("an unchecked case is absent from the request, and its value stays in the form",
          "[config][durations]") {
    const DurationAdjustmentSettings form{.charactersPerSecond = 22.5,
                                          .lengthen = false,
                                          .shorten = false,
                                          .minimumEnabled = false,
                                          .minimumMilliseconds = 2000,
                                          .maximumEnabled = true,
                                          .maximumMilliseconds = 4250,
                                          .gapEnabled = false,
                                          .gapMilliseconds = 80};

    const DurationConstraints asked = constraintsOf(form);

    CHECK_FALSE(asked.speed.has_value());
    CHECK_FALSE(asked.minimum.has_value());
    CHECK(asked.maximum == Duration::fromMilliseconds(4250));
    CHECK_FALSE(asked.gap.has_value());
}

TEST_CASE("a checked case with a value of zero asks for zero", "[config][durations]") {
    const DurationAdjustmentSettings form{
        .minimumEnabled = true, .minimumMilliseconds = 0, .gapEnabled = true, .gapMilliseconds = 0};

    const DurationConstraints asked = constraintsOf(form);

    CHECK(asked.minimum == Duration::zero());
    CHECK(asked.gap == Duration::zero());
}

TEST_CASE("the reading speed of the form carries both of its directions", "[config][durations]") {
    const DurationAdjustmentSettings form{
        .charactersPerSecond = 12.5, .lengthen = false, .shorten = true};

    CHECK(constraintsOf(form).speed == ReadingSpeed::create(12.5, false, true));
}

TEST_CASE("a speed that is not one is read as the speed being off", "[config][durations]") {
    const DurationAdjustmentSettings form{.charactersPerSecond = 0.0};

    CHECK_FALSE(constraintsOf(form).speed.has_value());
}
