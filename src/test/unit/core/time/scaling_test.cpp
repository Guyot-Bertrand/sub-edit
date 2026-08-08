#include <subedit/core/time/duration.hpp>
#include <subedit/core/time/frame_rate.hpp>
#include <subedit/core/time/ratio.hpp>
#include <subedit/core/time/timestamp.hpp>

#include <catch2/catch_test_macros.hpp>

#include <array>
#include <cstdint>
#include <optional>

#include "arithmetic_concepts.hpp"

namespace {

using subedit::core::Duration;
using subedit::core::FrameRate;
using subedit::core::Ratio;
using subedit::core::StandardFrameRate;
using subedit::core::Timestamp;

/// The factor of a conversion from 25 to 23.976 frames per second.
///
/// A conversion multiplies by the quotient of the two rates, and nothing else:
/// the frames themselves never enter the calculation.
[[nodiscard]] Ratio conversionFrom25To23976() {
    return FrameRate{StandardFrameRate::Fps25}.conversionTo(FrameRate{StandardFrameRate::Fps23976});
}

/// Builds a ratio whose terms are known to be valid.
[[nodiscard]] Ratio ratioOf(std::int64_t numerator, std::int64_t denominator) {
    const std::optional<Ratio> created = Ratio::create(numerator, denominator);
    if (!created.has_value()) {
        FAIL("these terms make a valid ratio");
        return Ratio::one();
    }
    return *created;
}

} // namespace

TEST_CASE("a position scales by an exact ratio", "[time][scaling]") {
    const Ratio doubled = ratioOf(2, 1);

    CHECK(Timestamp::fromMilliseconds(1500).scaledBy(doubled).milliseconds() == 3000);
    CHECK(Timestamp::fromMilliseconds(-1500).scaledBy(doubled).milliseconds() == -3000);
    CHECK(Timestamp::origin().scaledBy(doubled) == Timestamp::origin());
}

TEST_CASE("a duration scales by an exact ratio", "[time][scaling]") {
    const Ratio doubled = ratioOf(2, 1);

    CHECK(Duration::fromMilliseconds(1500).scaledBy(doubled).milliseconds() == 3000);
    CHECK(Duration::fromMilliseconds(-1500).scaledBy(doubled).milliseconds() == -3000);
    CHECK(Duration::zero().scaledBy(doubled) == Duration::zero());
}

TEST_CASE("the neutral ratio leaves a position and a duration untouched", "[time][scaling]") {
    const Timestamp position = Timestamp::fromMilliseconds(3600017);
    const Duration length = Duration::fromMilliseconds(-1234);

    CHECK(position.scaledBy(Ratio::one()) == position);
    CHECK(length.scaledBy(Ratio::one()) == length);
}

TEST_CASE("scaling a position rounds once, and not on the frame grid", "[time][scaling]") {
    // The measurement ADR 0013 rests on, asserted rather than alluded to: the
    // round trip through the frames quantises each position on the input grid
    // and lands up to half a frame away. Both columns are checked, so a change
    // to either path shows up here instead of passing on an inequality.
    const FrameRate input{StandardFrameRate::Fps25};
    const FrameRate output{StandardFrameRate::Fps23976};
    const Ratio factor = input.conversionTo(output);

    struct Expectation {
        std::int64_t milliseconds;
        std::int64_t exact;
        std::int64_t throughFrames;
    };

    const std::array<Expectation, 3> expectations = {
        Expectation{.milliseconds = 1010, .exact = 1053, .throughFrames = 1043},
        Expectation{.milliseconds = 1020, .exact = 1064, .throughFrames = 1084},
        Expectation{.milliseconds = 3600017, .exact = 3753768, .throughFrames = 3753750},
    };

    for (const auto& [milliseconds, exact, throughFrames] : expectations) {
        const Timestamp position = Timestamp::fromMilliseconds(milliseconds);
        const Timestamp quantised = Timestamp::fromFrame(position.toFrame(input), output);

        CHECK(position.scaledBy(factor).milliseconds() == exact);
        CHECK(quantised.milliseconds() == throughFrames);
    }
}

TEST_CASE("scaling a position stays symmetric around the origin", "[time][scaling]") {
    const Ratio factor = conversionFrom25To23976();

    for (const std::int64_t milliseconds : {1, 7, 500, 1010, 3600017}) {
        const Timestamp position = Timestamp::fromMilliseconds(milliseconds);
        const Timestamp opposite = Timestamp::fromMilliseconds(-milliseconds);

        CHECK(opposite.scaledBy(factor).milliseconds() ==
              -position.scaledBy(factor).milliseconds());
    }
}

TEST_CASE("scaling is a named decision, not an operator", "[time][scaling]") {
    // Multiplying a position by a factor would take a rounding decision behind
    // the caller's back. The compiler refuses it; `scaledBy` says it out loud.
    static_assert(!subedit::test::Multipliable<Timestamp, Ratio>);
    static_assert(!subedit::test::Multipliable<Duration, Ratio>);
    static_assert(!subedit::test::Multipliable<Ratio, Timestamp>);
    static_assert(!subedit::test::Multipliable<Ratio, Duration>);
}
