#include <subedit/core/time/frame.hpp>
#include <subedit/core/time/frame_rate.hpp>
#include <subedit/core/time/timestamp.hpp>

#include <catch2/catch_test_macros.hpp>

#include <array>
#include <cstdint>

namespace {

using subedit::core::Frame;
using subedit::core::FrameRate;
using subedit::core::kStandardFrameRates;
using subedit::core::StandardFrameRate;
using subedit::core::Timestamp;

} // namespace

TEST_CASE("a frame converts to the millisecond it starts at", "[time][conversion]") {
    const FrameRate pal{StandardFrameRate::Fps25};

    CHECK(Timestamp::fromFrame(Frame::zero(), pal) == Timestamp::origin());
    CHECK(Timestamp::fromFrame(Frame::fromNumber(1), pal).milliseconds() == 40);
    CHECK(Timestamp::fromFrame(Frame::fromNumber(25), pal).milliseconds() == 1000);
}

TEST_CASE("a fractional frame rate converts without drifting", "[time][conversion]") {
    const FrameRate ntscFilm{StandardFrameRate::Fps23976};

    // 24 frames at 24000/1001 last exactly 1001 ms. A frame rate held as a
    // double would land a millisecond off after a few hours; a rational does
    // not drift at all.
    const std::int64_t framesInAnHour = std::int64_t{24} * 3600;
    const std::int64_t millisecondsInAnHour = std::int64_t{1001} * 3600;

    CHECK(Timestamp::fromFrame(Frame::fromNumber(24), ntscFilm).milliseconds() == 1001);
    CHECK(Timestamp::fromFrame(Frame::fromNumber(framesInAnHour), ntscFilm).milliseconds() ==
          millisecondsInAnHour);
}

TEST_CASE("conversion rounds to the nearest millisecond, half away from zero",
          "[time][conversion]") {
    const FrameRate ntscFilm{StandardFrameRate::Fps23976};

    // Frame 12 lasts exactly 500.5 ms. Round-half-to-even, which Gaupol
    // inherits from Python, would answer 500 here and −501 below: not
    // symmetric around the origin.
    CHECK(Timestamp::fromFrame(Frame::fromNumber(12), ntscFilm).milliseconds() == 501);
    CHECK(Timestamp::fromFrame(Frame::fromNumber(-12), ntscFilm).milliseconds() == -501);
}

TEST_CASE("a millisecond converts to the frame it falls in", "[time][conversion]") {
    const FrameRate pal{StandardFrameRate::Fps25};

    CHECK(Timestamp::fromMilliseconds(0).toFrame(pal) == Frame::zero());
    CHECK(Timestamp::fromMilliseconds(19).toFrame(pal) == Frame::zero());
    CHECK(Timestamp::fromMilliseconds(21).toFrame(pal) == Frame::fromNumber(1));
    CHECK(Timestamp::fromMilliseconds(1000).toFrame(pal) == Frame::fromNumber(25));
}

TEST_CASE("the rounding of a negative position mirrors the positive one", "[time][conversion]") {
    const FrameRate pal{StandardFrameRate::Fps25};

    // 20 ms is exactly half a frame at 25 fps.
    CHECK(Timestamp::fromMilliseconds(20).toFrame(pal) == Frame::fromNumber(1));
    CHECK(Timestamp::fromMilliseconds(-20).toFrame(pal) == Frame::fromNumber(-1));
    CHECK(Timestamp::fromMilliseconds(60).toFrame(pal) == Frame::fromNumber(2));
    CHECK(Timestamp::fromMilliseconds(-60).toFrame(pal) == Frame::fromNumber(-2));
}

TEST_CASE("frame to millisecond to frame is exact at every standard rate", "[time][conversion]") {
    // The guarantee that makes milliseconds a safe internal representation for
    // frame-based formats: up to 60 fps a frame lasts more than 16 ms, so the
    // millisecond grid is finer than the frame grid and nothing is lost.
    const std::array<std::int64_t, 11> samples = {
        0, 1, 2, 12, 23, 24, 25, 1000, 86399, 123456, 5184000};

    for (const StandardFrameRate standard : kStandardFrameRates) {
        const FrameRate rate{standard};

        for (const std::int64_t number : samples) {
            for (const std::int64_t signedNumber : {number, -number}) {
                const Frame original = Frame::fromNumber(signedNumber);

                CHECK(Timestamp::fromFrame(original, rate).toFrame(rate) == original);
            }
        }
    }
}

TEST_CASE("millisecond to frame to millisecond loses the sub-frame precision",
          "[time][conversion]") {
    // The opposite round trip is not the identity, and cannot be: a frame is
    // 40 ms at 25 fps, so 39 of every 40 positions have no frame of their own.
    // Documented here as expected behaviour rather than discovered later.
    const FrameRate pal{StandardFrameRate::Fps25};
    const Timestamp original = Timestamp::fromMilliseconds(1021);

    const Timestamp roundTripped = Timestamp::fromFrame(original.toFrame(pal), pal);

    CHECK(roundTripped.milliseconds() == 1040);
    CHECK(roundTripped != original);
}
