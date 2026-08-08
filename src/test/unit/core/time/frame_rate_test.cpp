#include <subedit/core/time/frame_rate.hpp>
#include <subedit/core/time/ratio.hpp>

#include <catch2/catch_test_macros.hpp>

#include <array>
#include <cstdint>
#include <optional>
#include <type_traits>

namespace {

using subedit::core::FrameRate;
using subedit::core::kStandardFrameRates;
using subedit::core::Ratio;
using subedit::core::StandardFrameRate;

} // namespace

TEST_CASE("the eight standard frame rates are exact rationals", "[time][framerate]") {
    struct Expectation {
        StandardFrameRate standard;
        std::int64_t numerator;
        std::int64_t denominator;
    };

    // The three fractional rates are the ones a floating-point frame rate
    // would get wrong: 24 / 1.001 has no exact binary representation.
    const std::array<Expectation, 8> expectations = {
        Expectation{
            .standard = StandardFrameRate::Fps23976, .numerator = 24000, .denominator = 1001},
        Expectation{.standard = StandardFrameRate::Fps24, .numerator = 24, .denominator = 1},
        Expectation{.standard = StandardFrameRate::Fps25, .numerator = 25, .denominator = 1},
        Expectation{
            .standard = StandardFrameRate::Fps29970, .numerator = 30000, .denominator = 1001},
        Expectation{.standard = StandardFrameRate::Fps30, .numerator = 30, .denominator = 1},
        Expectation{.standard = StandardFrameRate::Fps50, .numerator = 50, .denominator = 1},
        Expectation{
            .standard = StandardFrameRate::Fps59940, .numerator = 60000, .denominator = 1001},
        Expectation{.standard = StandardFrameRate::Fps60, .numerator = 60, .denominator = 1},
    };

    for (const Expectation& expected : expectations) {
        const FrameRate rate{expected.standard};

        CHECK(rate.numerator() == expected.numerator);
        CHECK(rate.denominator() == expected.denominator);
    }
}

TEST_CASE("the standard frame rates are listed once each", "[time][framerate]") {
    CHECK(kStandardFrameRates.size() == 8);

    for (const StandardFrameRate standard : kStandardFrameRates) {
        const FrameRate rate{standard};
        CHECK(rate.numerator() > 0);
        CHECK(rate.denominator() > 0);
    }
}

TEST_CASE("a custom frame rate is accepted", "[time][framerate]") {
    const std::optional<FrameRate> created = FrameRate::create(48, 1);
    if (!created.has_value()) {
        FAIL("a frame rate with two positive terms must be accepted");
        return;
    }
    const FrameRate rate = *created;

    CHECK(rate.numerator() == 48);
    CHECK(rate.denominator() == 1);
}

TEST_CASE("a custom frame rate is reduced to lowest terms", "[time][framerate]") {
    const std::optional<FrameRate> created = FrameRate::create(48000, 2002);
    if (!created.has_value()) {
        FAIL("a frame rate with two positive terms must be accepted");
        return;
    }
    const FrameRate rate = *created;

    CHECK(rate.numerator() == 24000);
    CHECK(rate.denominator() == 1001);
    CHECK(rate == FrameRate{StandardFrameRate::Fps23976});
}

TEST_CASE("a frame rate must be strictly positive", "[time][framerate]") {
    CHECK_FALSE(FrameRate::create(0, 1).has_value());
    CHECK_FALSE(FrameRate::create(25, 0).has_value());
    CHECK_FALSE(FrameRate::create(-25, 1).has_value());
    CHECK_FALSE(FrameRate::create(25, -1).has_value());
}

TEST_CASE("a frame rate refuses terms no frame rate could have", "[time][framerate]") {
    // The bound is what makes the conversions below total: a rate whose terms
    // stay under a billion can be turned into milliseconds per frame, or
    // divided by another rate, without any product leaving `std::int64_t`.
    CHECK(FrameRate::create(1000000000, 1).has_value());
    CHECK(FrameRate::create(1, 1000000000).has_value());
    CHECK_FALSE(FrameRate::create(1000000001, 1).has_value());
    CHECK_FALSE(FrameRate::create(1, 1000000001).has_value());
}

TEST_CASE("a frame rate is a rational number of frames per second", "[time][framerate]") {
    const FrameRate ntscFilm{StandardFrameRate::Fps23976};

    CHECK(ntscFilm.framesPerSecond().numerator() == 24000);
    CHECK(ntscFilm.framesPerSecond().denominator() == 1001);
}

TEST_CASE("a frame rate says how long a frame lasts, exactly", "[time][framerate]") {
    const FrameRate pal{StandardFrameRate::Fps25};
    const FrameRate ntscFilm{StandardFrameRate::Fps23976};

    // 40 ms at 25 fps; 1001/24 ms — not a whole number — at 24000/1001.
    CHECK(pal.millisecondsPerFrame() == Ratio::create(40, 1));
    CHECK(ntscFilm.millisecondsPerFrame() == Ratio::create(1001, 24));
    CHECK(pal.framesPerMillisecond() == Ratio::create(1, 40));
    CHECK(ntscFilm.framesPerMillisecond() == Ratio::create(24, 1001));
}

TEST_CASE("the conversion factor between two rates is their exact quotient", "[time][framerate]") {
    // A position timed at one rate is multiplied by this factor to be timed at
    // the other. Naming the direction is the point: the inverse factor is just
    // as valid a ratio, and tells nothing apart from the result being wrong.
    const FrameRate pal{StandardFrameRate::Fps25};
    const FrameRate ntscFilm{StandardFrameRate::Fps23976};

    CHECK(pal.conversionTo(ntscFilm) == Ratio::create(std::int64_t{25} * 1001, 24000));
    CHECK(ntscFilm.conversionTo(pal) == Ratio::create(24000, std::int64_t{25} * 1001));
    CHECK(pal.conversionTo(pal) == Ratio::one());
}

TEST_CASE("frame rates compare by value, not by representation", "[time][framerate]") {
    CHECK(FrameRate::create(50, 2) == FrameRate::create(25, 1));
    CHECK(FrameRate{StandardFrameRate::Fps25} != FrameRate{StandardFrameRate::Fps30});
}

TEST_CASE("a frame rate is not a number in disguise", "[time][framerate]") {
    static_assert(!std::is_convertible_v<FrameRate, double>);
    static_assert(!std::is_convertible_v<StandardFrameRate, FrameRate>);
    static_assert(!std::is_default_constructible_v<FrameRate>);
    static_assert(std::is_trivially_copyable_v<FrameRate>);
}
