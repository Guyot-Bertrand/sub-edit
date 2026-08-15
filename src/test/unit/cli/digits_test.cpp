#include <subedit/cli/digits.hpp>

#include <catch2/catch_test_macros.hpp>

#include <cstdint>
#include <limits>
#include <optional>

using subedit::cli::appendedDigit;

namespace {

constexpr std::int64_t kLargest = std::numeric_limits<std::int64_t>::max();

/// The largest value another digit can still be appended to, and the largest
/// digit it takes: 9223372036854775807 is 922337203685477580 then a 7.
constexpr std::int64_t kAlmostLargest = kLargest / 10;

/// The value, unwrapped where the test can say what was expected.
std::int64_t after(std::int64_t value, char digit) {
    const std::optional<std::int64_t> grown = appendedDigit(value, digit);
    if (!grown.has_value()) {
        FAIL("appending " << digit << " to " << value << " must hold");
        return 0;
    }
    return *grown;
}

} // namespace

TEST_CASE("a digit is read into the number being built", "[cli][digits]") {
    CHECK(after(0, '7') == 7);
    CHECK(after(12, '3') == 123);
}

TEST_CASE("appending a zero is how a decimal place is made", "[cli][digits]") {
    // What the frame rate grammar does to its denominator: ten times as many
    // parts for every decimal read.
    CHECK(after(1, '0') == 10);
    CHECK(after(50, '0') == 500);
}

TEST_CASE("the last digit that fits is accepted", "[cli][digits]") {
    CHECK(after(kAlmostLargest, '7') == kLargest);
}

TEST_CASE("the first digit that does not fit is refused", "[cli][digits]") {
    // Refused before the multiplication, not detected after it: after is too
    // late, the overflow of a signed integer being undefined behaviour and not
    // a value one can look at.
    CHECK_FALSE(appendedDigit(kAlmostLargest, '8').has_value());
    CHECK_FALSE(appendedDigit(kAlmostLargest + 1, '0').has_value());
    CHECK_FALSE(appendedDigit(kLargest, '0').has_value());
}
