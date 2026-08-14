#include <subedit/cli/frame_rate_grammar.hpp>
#include <subedit/core/time/frame_rate.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include <string>

using Catch::Matchers::ContainsSubstring;
using subedit::cli::parseFrameRate;
using subedit::core::FrameRate;
using subedit::core::StandardFrameRate;

namespace {

FrameRate rateOf(const std::string& text) {
    return parseFrameRate(text).value();
}

} // namespace

TEST_CASE("a whole number of frames per second is read", "[cli][framerate]") {
    CHECK(rateOf("25") == FrameRate{StandardFrameRate::Fps25});
    CHECK(rateOf("24") == FrameRate{StandardFrameRate::Fps24});
    CHECK(rateOf("30") == FrameRate{StandardFrameRate::Fps30});
    CHECK(rateOf("50") == FrameRate{StandardFrameRate::Fps50});
    CHECK(rateOf("60") == FrameRate{StandardFrameRate::Fps60});
}

TEST_CASE("the three NTSC spellings name their exact rate", "[cli][framerate]") {
    // 23.976 is a label, not a value: the rate it names is 24000/1001, in this
    // tool as in every other. Reading it as 23976/1000 would make the command
    // line disagree with the window of phase 5 on the same words.
    CHECK(rateOf("23.976") == FrameRate{StandardFrameRate::Fps23976});
    CHECK(rateOf("29.97") == FrameRate{StandardFrameRate::Fps29970});
    CHECK(rateOf("59.94") == FrameRate{StandardFrameRate::Fps59940});
}

TEST_CASE("an NTSC spelling is recognised by its value, not its letters", "[cli][framerate]") {
    // Trailing zeros write the same number, so they name the same rate.
    CHECK(rateOf("23.9760") == FrameRate{StandardFrameRate::Fps23976});
    CHECK(rateOf("29.970") == FrameRate{StandardFrameRate::Fps29970});
}

TEST_CASE("any other decimal is taken at face value", "[cli][framerate]") {
    // 23.9 is nobody's standard, so there is nothing to read into it.
    CHECK(rateOf("23.9").framesPerSecond().numerator() == 239);
    CHECK(rateOf("23.9").framesPerSecond().denominator() == 10);
    CHECK(rateOf("12.5").framesPerSecond().numerator() == 25);
    CHECK(rateOf("12.5").framesPerSecond().denominator() == 2);
}

TEST_CASE("a rate of zero is refused", "[cli][framerate]") {
    const auto refused = parseFrameRate("0");

    REQUIRE_FALSE(refused.has_value());
    CHECK_THAT(refused.error(), ContainsSubstring("strictly positive"));
}

TEST_CASE("a negative rate is refused", "[cli][framerate]") {
    CHECK_THAT(parseFrameRate("-25").error(), ContainsSubstring("strictly positive"));
    CHECK_THAT(parseFrameRate("-0.5").error(), ContainsSubstring("strictly positive"));
}

TEST_CASE("zero written with decimals is still zero", "[cli][framerate]") {
    CHECK_THAT(parseFrameRate("0.000").error(), ContainsSubstring("strictly positive"));
}

TEST_CASE("the comma is refused rather than guessed at", "[cli][framerate]") {
    // The same rule as everywhere else on this command line, and for the same
    // reason: half the world writes 23,976.
    CHECK_THAT(parseFrameRate("23,976").error(), ContainsSubstring("decimal point"));
}

TEST_CASE("what is not a number at all is refused", "[cli][framerate]") {
    CHECK_FALSE(parseFrameRate("").has_value());
    CHECK_FALSE(parseFrameRate("pal").has_value());
    CHECK_FALSE(parseFrameRate("25fps").has_value());
    CHECK_FALSE(parseFrameRate("2.5.0").has_value());
    CHECK_FALSE(parseFrameRate("-").has_value());
    CHECK_FALSE(parseFrameRate("25.").has_value());
}

TEST_CASE("a refusal names what was given", "[cli][framerate]") {
    CHECK_THAT(parseFrameRate("pal").error(), ContainsSubstring("pal"));
}

TEST_CASE("a rate no video runs at is refused", "[cli][framerate]") {
    // The core bounds both terms at a billion, which is what keeps every
    // product of a conversion inside an int64.
    CHECK_FALSE(parseFrameRate("1000000001").has_value());
    CHECK_FALSE(parseFrameRate("99999999999999999999999").has_value());
    // Finer than a billionth of a frame: the denominator passes the bound.
    CHECK_FALSE(parseFrameRate("1.0000000001").has_value());
}

TEST_CASE("a rate written with more decimals than an integer holds is refused",
          "[cli][framerate]") {
    // Refused by the arithmetic and not by the core's bound: twenty-three
    // decimals overflow the accumulation long before a frame rate is built. The
    // guard is what keeps a wrapped value from naming some other rate.
    CHECK_FALSE(parseFrameRate("1.00000000000000000000000").has_value());
    CHECK_FALSE(parseFrameRate("1.99999999999999999999999").has_value());
}

TEST_CASE("a plus sign says nothing a rate does not already say", "[cli][framerate]") {
    CHECK(rateOf("+25") == FrameRate{StandardFrameRate::Fps25});
}
