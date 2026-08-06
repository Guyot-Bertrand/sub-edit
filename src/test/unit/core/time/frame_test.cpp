#include <subedit/core/time/frame.hpp>

#include <catch2/catch_test_macros.hpp>

#include <cstdint>
#include <type_traits>

#include "arithmetic_concepts.hpp"

namespace {

using subedit::core::Frame;

} // namespace

TEST_CASE("a frame carries the number it was built from", "[time][frame]") {
    CHECK(Frame::fromNumber(42).number() == 42);
    CHECK(Frame::fromNumber(-42).number() == -42);
    CHECK(Frame::zero().number() == 0);
}

TEST_CASE("frames add and subtract", "[time][frame]") {
    const Frame first = Frame::fromNumber(100);
    const Frame second = Frame::fromNumber(30);

    CHECK((first + second).number() == 130);
    CHECK((first - second).number() == 70);
    CHECK((second - first).number() == -70);
    CHECK((-first).number() == -100);
}

TEST_CASE("frames compare by their number", "[time][frame]") {
    CHECK(Frame::fromNumber(1) < Frame::fromNumber(2));
    CHECK(Frame::fromNumber(2) > Frame::fromNumber(1));
    CHECK(Frame::fromNumber(7) == Frame::fromNumber(7));
    CHECK(Frame::fromNumber(7) != Frame::fromNumber(8));
}

TEST_CASE("frames accumulate in place", "[time][frame]") {
    Frame count = Frame::zero();
    count += Frame::fromNumber(24);
    count -= Frame::fromNumber(4);

    CHECK(count.number() == 20);
}

TEST_CASE("a frame is not an integer in disguise", "[time][frame]") {
    static_assert(!std::is_convertible_v<Frame, std::int64_t>);
    static_assert(!std::is_convertible_v<std::int64_t, Frame>);
    static_assert(!std::is_constructible_v<Frame, std::int64_t>);
    static_assert(!subedit::test::Addable<Frame, std::int64_t>);
    static_assert(!subedit::test::Ordered<Frame, std::int64_t>);
    static_assert(std::is_trivially_copyable_v<Frame>);
}
