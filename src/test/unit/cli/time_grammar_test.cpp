#include <subedit/cli/time_grammar.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

using Catch::Matchers::ContainsSubstring;
using subedit::cli::parseTime;

namespace {

std::int64_t msOf(const std::string& text) {
    return parseTime(text).value().milliseconds();
}

} // namespace

TEST_CASE("whole seconds are read", "[cli][time]") {
    CHECK(msOf("30") == 30'000);
    CHECK(msOf("0") == 0);
}

TEST_CASE("decimals are read down to the millisecond", "[cli][time]") {
    CHECK(msOf("2.999") == 2'999);
    CHECK(msOf("7.001") == 7'001);
    CHECK(msOf("1.5") == 1'500);
    CHECK(msOf("1.05") == 1'050);
}

TEST_CASE("the sign moves time backwards", "[cli][time]") {
    CHECK(msOf("-7.001") == -7'001);
    CHECK(msOf("+2.999") == 2'999);
}

TEST_CASE("a timestamp is read as well as a count of seconds", "[cli][time]") {
    CHECK(msOf("01:39:37.040") == 5'977'040);
    CHECK(msOf("00:00:07.001") == 7'001);
    CHECK(msOf("-00:00:07.001") == -7'001);
}

TEST_CASE("the comma is refused rather than guessed at", "[cli][time]") {
    const auto read = parseTime("-7,001");

    REQUIRE_FALSE(read.has_value());
    // Half the world writes 7,001 for seven thousand and one. Reading it as
    // seven seconds would be a guess, and a wrong one for that half.
    CHECK_THAT(read.error(), ContainsSubstring("decimal point"));
}

TEST_CASE("finer than a millisecond is refused", "[cli][time]") {
    const auto read = parseTime("1.0005");

    REQUIRE_FALSE(read.has_value());
    // Rounding it silently would give a result matching nothing that was asked
    // for.
    CHECK_THAT(read.error(), ContainsSubstring("millisecond"));
}

TEST_CASE("what is not a time at all is refused", "[cli][time]") {
    CHECK_FALSE(parseTime("").has_value());
    CHECK_FALSE(parseTime("bientôt").has_value());
    CHECK_FALSE(parseTime("1.2.3").has_value());
    CHECK_FALSE(parseTime("--7").has_value());
    CHECK_FALSE(parseTime("7s").has_value());
}

TEST_CASE("a refusal names what was given", "[cli][time]") {
    CHECK_THAT(parseTime("bientôt").error(), ContainsSubstring("bientôt"));
}

TEST_CASE("an amount too large to hold is refused", "[cli][time]") {
    // Unguarded, the accumulation overflows — undefined behaviour, and then a
    // shift by whatever the wrapping produced, reported as a success. A
    // position is a signed count of milliseconds and nothing may pretend to be
    // one from outside that range.
    CHECK_FALSE(parseTime("99999999999999999999").has_value());
    CHECK_FALSE(parseTime("-99999999999999999999").has_value());
    CHECK_FALSE(parseTime("9223372036854775808").has_value());
}

TEST_CASE("the multiplication into milliseconds is guarded too", "[cli][time]") {
    // 9223372036854776 seconds holds in an int64; a thousand times it does not.
    CHECK_FALSE(parseTime("9223372036854776").has_value());
    // One less thousand-fold is the largest that does.
    CHECK(parseTime("9223372036854775").value().milliseconds() == 9'223'372'036'854'775'000);
}

TEST_CASE("the decimals are weighed against what the seconds already take", "[cli][time]") {
    // The largest whole second that holds leaves exactly 807 milliseconds of
    // room. The boundary is pinned on both sides because it is the one place
    // where a value the two earlier guards let through still does not fit.
    CHECK(parseTime("9223372036854775.807").value().milliseconds() == 9'223'372'036'854'775'807);
    CHECK_FALSE(parseTime("9223372036854775.808").has_value());
}

TEST_CASE("a refusal for size says so", "[cli][time]") {
    CHECK_THAT(parseTime("99999999999999999999").error(), ContainsSubstring("too large"));
}

TEST_CASE("a timestamp too large to hold is refused as well", "[cli][time]") {
    CHECK_FALSE(parseTime("99999999999999999999:00:00.000").has_value());
}

TEST_CASE("a sign with nothing after it is not a time", "[cli][time]") {
    CHECK_FALSE(parseTime("-").has_value());
    CHECK_FALSE(parseTime("+").has_value());
}
