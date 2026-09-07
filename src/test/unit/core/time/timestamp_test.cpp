#include <subedit/core/time/duration.hpp>
#include <subedit/core/time/frame.hpp>
#include <subedit/core/time/timestamp.hpp>

#include <catch2/catch_test_macros.hpp>

#include <cstdint>
#include <optional>
#include <string_view>
#include <type_traits>

#include "arithmetic_concepts.hpp"

namespace {

using subedit::core::DecimalMark;
using subedit::core::Decimals;
using subedit::core::Duration;
using subedit::core::Frame;
using subedit::core::HourField;
using subedit::core::Timestamp;

/// A position read from its written form, unwrapped where the test can say so.
[[nodiscard]] Timestamp positionOf(std::string_view text) {
    const std::optional<Timestamp> parsed = Timestamp::parse(text);
    if (!parsed.has_value()) {
        FAIL("a timestamp written in full must be readable");
        return Timestamp::origin();
    }
    return *parsed;
}

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

TEST_CASE("a position written to the hundredth rounds the whole of it", "[time][timestamp]") {
    // **Not the fraction alone.** 3 999 ms is four seconds at this precision,
    // and rounding only what follows the mark would write `00:00:03.100` — a
    // string no reader would take back.
    const auto at = [](std::int64_t milliseconds) {
        return Timestamp::fromMilliseconds(milliseconds)
            .format(DecimalMark::Period, HourField::Always, Decimals::Centiseconds);
    };

    CHECK(at(1000) == "00:00:01.00");
    CHECK(at(1234) == "00:00:01.23");
    CHECK(at(3999) == "00:00:04.00");
    CHECK(at(59995) == "00:01:00.00");

    // Halves away from zero, the rule `fromFrame` already follows.
    CHECK(at(1005) == "00:00:01.01");
    CHECK(at(-1005) == "-00:00:01.01");
}

TEST_CASE("writing to the hundredth leaves the position itself alone", "[time][timestamp]") {
    // The rounding is a property of the writing, not of the value: the same
    // position written to the millisecond keeps all three digits.
    const Timestamp position = Timestamp::fromMilliseconds(1234);

    CHECK(position.format(DecimalMark::Period, HourField::Always, Decimals::Centiseconds) ==
          "00:00:01.23");
    CHECK(position.format(DecimalMark::Comma) == "00:00:01,234");
    CHECK(position.milliseconds() == 1234);
}

TEST_CASE("rounding up at the very edge stays inside what can be written", "[time][timestamp]") {
    // The magnitude saturates at `99:59:59,999`, and rounding that to the
    // hundredth would ask for a hundredth hour. It comes back to the last
    // position the field can hold instead.
    // Read rather than computed: the largest position the two-digit hours field
    // can hold is easier to recognise written out than as a sum of factors.
    const Timestamp edge = positionOf("99:59:59,999");

    CHECK(edge.format(DecimalMark::Comma) == "99:59:59,999");
    CHECK(edge.format(DecimalMark::Period, HourField::Always, Decimals::Centiseconds) ==
          "99:59:59.99");
}
