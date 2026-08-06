#include <subedit/core/time/duration.hpp>
#include <subedit/core/time/frame.hpp>
#include <subedit/core/time/timestamp.hpp>

#include <catch2/catch_test_macros.hpp>

#include <cstdint>
#include <type_traits>

#include "arithmetic_concepts.hpp"

namespace {

using subedit::core::Duration;
using subedit::core::Frame;
using subedit::core::Timestamp;

} // namespace

TEST_CASE("a timestamp carries the millisecond count it was built from", "[time][timestamp]") {
    CHECK(Timestamp::fromMilliseconds(1500).milliseconds() == 1500);
    CHECK(Timestamp::fromMilliseconds(-1500).milliseconds() == -1500);
    CHECK(Timestamp::origin().milliseconds() == 0);
}

TEST_CASE("subtracting two timestamps yields a duration", "[time][timestamp]") {
    const Timestamp start = Timestamp::fromMilliseconds(1000);
    const Timestamp end = Timestamp::fromMilliseconds(4500);

    CHECK(end - start == Duration::fromMilliseconds(3500));
    CHECK(start - end == Duration::fromMilliseconds(-3500));
}

TEST_CASE("shifting a timestamp by a duration yields a timestamp", "[time][timestamp]") {
    const Timestamp start = Timestamp::fromMilliseconds(1000);
    const Duration shift = Duration::fromMilliseconds(250);

    CHECK((start + shift).milliseconds() == 1250);
    CHECK((shift + start).milliseconds() == 1250);
    CHECK((start - shift).milliseconds() == 750);
}

TEST_CASE("a timestamp shifts in place", "[time][timestamp]") {
    Timestamp position = Timestamp::origin();
    position += Duration::fromMilliseconds(2000);
    position -= Duration::fromMilliseconds(500);

    CHECK(position.milliseconds() == 1500);
}

TEST_CASE("a shift can move a timestamp before the origin", "[time][timestamp]") {
    const Timestamp position = Timestamp::fromMilliseconds(400);

    CHECK((position - Duration::fromMilliseconds(1000)).milliseconds() == -600);
}

TEST_CASE("timestamps compare by position", "[time][timestamp]") {
    const Timestamp earlier = Timestamp::fromMilliseconds(-100);
    const Timestamp later = Timestamp::fromMilliseconds(100);

    CHECK(earlier < later);
    CHECK(later > earlier);
    CHECK(earlier == Timestamp::fromMilliseconds(-100));
    CHECK(earlier != later);
}

TEST_CASE("positions and durations are not interchangeable", "[time][timestamp]") {
    static_assert(!std::is_convertible_v<Timestamp, std::int64_t>);
    static_assert(!std::is_convertible_v<Timestamp, Duration>);
    static_assert(!std::is_convertible_v<Duration, Timestamp>);
    static_assert(!std::is_convertible_v<Frame, Timestamp>);

    // Adding two positions is the mistake the type system exists to catch:
    // the sum of two points in time is not a point in time.
    static_assert(!subedit::test::Addable<Timestamp, Timestamp>);
    static_assert(!subedit::test::Addable<Timestamp, std::int64_t>);
    static_assert(!subedit::test::Subtractable<Timestamp, Frame>);
    static_assert(!subedit::test::Multipliable<Timestamp, std::int64_t>);
    static_assert(!subedit::test::Ordered<Timestamp, Duration>);

    // What is allowed, stated in the same place, so the two lists read together.
    static_assert(subedit::test::Subtractable<Timestamp, Timestamp>);
    static_assert(subedit::test::Addable<Timestamp, Duration>);
    static_assert(subedit::test::Subtractable<Timestamp, Duration>);
    static_assert(std::is_same_v<decltype(Timestamp::origin() - Timestamp::origin()), Duration>);
    static_assert(std::is_same_v<decltype(Timestamp::origin() + Duration::zero()), Timestamp>);
}
