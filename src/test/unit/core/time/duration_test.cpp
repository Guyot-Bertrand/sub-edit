#include <subedit/core/time/duration.hpp>

#include <catch2/catch_test_macros.hpp>

#include <cstdint>
#include <type_traits>

#include "arithmetic_concepts.hpp"

namespace {

using subedit::core::Duration;

} // namespace

TEST_CASE("a duration carries the millisecond count it was built from", "[time][duration]") {
    CHECK(Duration::fromMilliseconds(1500).milliseconds() == 1500);
    CHECK(Duration::fromMilliseconds(-1500).milliseconds() == -1500);
    CHECK(Duration::zero().milliseconds() == 0);
}

TEST_CASE("durations add and subtract", "[time][duration]") {
    const Duration one = Duration::fromMilliseconds(1000);
    const Duration half = Duration::fromMilliseconds(500);

    CHECK((one + half).milliseconds() == 1500);
    CHECK((one - half).milliseconds() == 500);
    CHECK((half - one).milliseconds() == -500);
    CHECK((-one).milliseconds() == -1000);
}

TEST_CASE("a duration scales by an integer factor", "[time][duration]") {
    const Duration one = Duration::fromMilliseconds(1000);

    CHECK((one * 3).milliseconds() == 3000);
    CHECK((3 * one).milliseconds() == 3000);
    CHECK((one * -2).milliseconds() == -2000);
}

TEST_CASE("durations compare by their millisecond count", "[time][duration]") {
    const Duration shorter = Duration::fromMilliseconds(500);
    const Duration longer = Duration::fromMilliseconds(1500);

    CHECK(shorter < longer);
    CHECK(longer > shorter);
    CHECK(shorter == Duration::fromMilliseconds(500));
    CHECK(shorter != longer);
    CHECK(Duration::fromMilliseconds(-1) < Duration::zero());
}

TEST_CASE("durations accumulate in place", "[time][duration]") {
    Duration total = Duration::zero();
    total += Duration::fromMilliseconds(700);
    total -= Duration::fromMilliseconds(200);

    CHECK(total.milliseconds() == 500);
}

// The compiler is the first test: these are the mistakes the type exists to
// make impossible. A `static_assert` proves the operation does not compile,
// which a runtime test could never do.
TEST_CASE("a duration is not an integer in disguise", "[time][duration]") {
    static_assert(!std::is_convertible_v<Duration, std::int64_t>);
    static_assert(!std::is_convertible_v<std::int64_t, Duration>);
    static_assert(!std::is_constructible_v<Duration, std::int64_t>);
    static_assert(!subedit::test::Addable<Duration, std::int64_t>);
    static_assert(!subedit::test::Subtractable<Duration, std::int64_t>);
    static_assert(!subedit::test::Ordered<Duration, std::int64_t>);
    static_assert(std::is_trivially_copyable_v<Duration>);
}
