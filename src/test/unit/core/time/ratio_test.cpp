#include <subedit/core/time/ratio.hpp>

#include <catch2/catch_test_macros.hpp>

#include <cstdint>
#include <limits>
#include <optional>
#include <type_traits>

#include "arithmetic_concepts.hpp"

namespace {

using subedit::core::Ratio;

constexpr std::int64_t kLargest = std::numeric_limits<std::int64_t>::max();
constexpr std::int64_t kSmallest = std::numeric_limits<std::int64_t>::min();

/// Builds a ratio whose terms are known to be valid.
///
/// The tests below are about what a ratio computes, not about whether it was
/// built; unwrapping in each of them would bury that under a check.
[[nodiscard]] Ratio ratioOf(std::int64_t numerator, std::int64_t denominator) {
    const std::optional<Ratio> created = Ratio::create(numerator, denominator);
    if (!created.has_value()) {
        FAIL("these terms make a valid ratio");
        return Ratio::one();
    }
    return *created;
}

} // namespace

TEST_CASE("a ratio with a null denominator is refused", "[time][ratio]") {
    CHECK_FALSE(Ratio::create(1, 0).has_value());
    CHECK_FALSE(Ratio::create(0, 0).has_value());
    CHECK(Ratio::create(0, 1).has_value());
}

TEST_CASE("a term that has no opposite is refused", "[time][ratio]") {
    // The smallest `std::int64_t` has no opposite in its own type. Reducing
    // and moving the sign to the numerator both need one, so such a term is
    // refused at construction rather than silently overflowed afterwards.
    CHECK_FALSE(Ratio::create(kSmallest, 1).has_value());
    CHECK_FALSE(Ratio::create(1, kSmallest).has_value());
}

TEST_CASE("a ratio is stored reduced", "[time][ratio]") {
    // Two ratios written differently but equal in value must compare equal,
    // which only holds if the terms are reduced at construction.
    const Ratio half = ratioOf(50, 100);

    CHECK(half.numerator() == 1);
    CHECK(half.denominator() == 2);
    CHECK(half == ratioOf(1, 2));
    CHECK(ratioOf(0, 7).denominator() == 1);
}

TEST_CASE("the sign of a ratio is carried by its numerator", "[time][ratio]") {
    // A single place for the sign: the denominator is then strictly positive
    // for every ratio, which is what the rounding relies on.
    CHECK(ratioOf(1, -2).numerator() == -1);
    CHECK(ratioOf(1, -2).denominator() == 2);
    CHECK(ratioOf(-1, -2) == ratioOf(1, 2));
    CHECK(ratioOf(-1, 2) == ratioOf(1, -2));
}

TEST_CASE("the neutral ratio leaves a value untouched", "[time][ratio]") {
    CHECK(Ratio::one() == ratioOf(1, 1));
    CHECK(Ratio::one().scale(1234) == 1234);
    CHECK(Ratio::one().scale(-1234) == -1234);
    CHECK(Ratio::one().scale(kLargest) == kLargest);
    CHECK(Ratio::one().scale(kSmallest) == kSmallest);
}

TEST_CASE("scaling multiplies by the exact rational and rounds once", "[time][ratio]") {
    // 25 / (24000/1001), the factor of a conversion from 25 to 23.976 fps.
    // 1010 × 25025/24000 is 1053.135…, so 1053. Going through the frames
    // would answer 1043 — the error this type exists to avoid.
    const Ratio factor = ratioOf(std::int64_t{25} * 1001, 24000);

    CHECK(factor.scale(1010) == 1053);
    CHECK(factor.scale(1020) == 1064);
    CHECK(factor.scale(3600017) == 3753768);
}

TEST_CASE("scaling rounds halves away from zero", "[time][ratio]") {
    const Ratio half = ratioOf(1, 2);

    CHECK(half.scale(3) == 2);
    CHECK(half.scale(-3) == -2);
    CHECK(half.scale(5) == 3);
    CHECK(half.scale(-5) == -3);
}

TEST_CASE("the scaling of an opposite value is the opposite scaling", "[time][ratio]") {
    // Symmetry around zero, the property that lets an operation give matching
    // results on either side of the origin.
    const Ratio factor = ratioOf(std::int64_t{25} * 1001, 24000);

    for (const std::int64_t value : {0, 1, 7, 500, 1010, 3600017, 123456789}) {
        CHECK(factor.scale(-value) == -factor.scale(value));
    }
}

TEST_CASE("a negative ratio reverses the direction of the value", "[time][ratio]") {
    const Ratio opposite = ratioOf(-1, 1);

    CHECK(opposite.scale(1000) == -1000);
    CHECK(opposite.scale(-1000) == 1000);
}

TEST_CASE("a scaling that leaves the representable range saturates", "[time][ratio]") {
    // Beyond `std::int64_t` there is no position to answer with, and wrapping
    // around would be undefined behaviour. The result stops at the bound
    // instead, which is defined, ordered like the exact result, and absurd in
    // the same way its input was: the bound is some 292 million years.
    const Ratio doubled = ratioOf(2, 1);
    const Ratio opposite = ratioOf(-1, 1);

    CHECK(doubled.scale(kLargest) == kLargest);
    CHECK(doubled.scale(kSmallest) == kSmallest);
    CHECK(opposite.scale(kSmallest) == kLargest);
    CHECK(ratioOf(3, 1).scale(kLargest / 2) == kLargest);
}

TEST_CASE("a huge denominator does not break the rounding", "[time][ratio]") {
    // The halving test used to double the remainder, which overflowed as soon
    // as the denominator passed half the range. Ratios that large are absurd,
    // but the arithmetic must stay defined for them.
    const Ratio tiny = ratioOf(1, kLargest);

    CHECK(tiny.scale(kLargest) == 1);
    CHECK(tiny.scale((kLargest / 2) + 1) == 1); // just past half, rounds away
    CHECK(tiny.scale(kLargest / 2) == 0);       // just short of half, rounds to zero
    CHECK(tiny.scale(-kLargest) == -1);
}

TEST_CASE("scaling by a ratio and by its inverse comes back to the value", "[time][ratio]") {
    // Only when nothing is rounded away: the point is that the exact rational
    // path is exact, not that scaling is invertible in general.
    const Ratio factor = ratioOf(3, 4);
    const Ratio inverse = ratioOf(4, 3);

    CHECK(inverse.scale(factor.scale(4000)) == 4000);
    CHECK(inverse.scale(factor.scale(-4000)) == -4000);
}

TEST_CASE("a ratio is not an integer in disguise", "[time][ratio]") {
    static_assert(!std::is_convertible_v<Ratio, std::int64_t>);
    static_assert(!std::is_convertible_v<std::int64_t, Ratio>);
    static_assert(!std::is_constructible_v<Ratio, std::int64_t>);
    static_assert(!std::is_default_constructible_v<Ratio>);
    static_assert(!subedit::test::Addable<Ratio, std::int64_t>);
    static_assert(!subedit::test::Ordered<Ratio, std::int64_t>);
    // Scaling is a rounding decision, and ADR 0006 wants none of them taken
    // implicitly: `ratio * value` must not compile, `scale` must be written.
    static_assert(!subedit::test::Multipliable<Ratio, std::int64_t>);
    static_assert(!subedit::test::Multipliable<std::int64_t, Ratio>);
    static_assert(std::is_trivially_copyable_v<Ratio>);
}
