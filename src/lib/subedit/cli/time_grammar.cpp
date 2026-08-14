#include <subedit/cli/time_grammar.hpp>
#include <subedit/core/time/timestamp.hpp>

#include <algorithm>
#include <cstdint>
#include <expected>
#include <limits>
#include <optional>

namespace subedit::cli {

namespace {

using core::Duration;

/// Written out because the arithmetic below is decimal and stays in integers.
constexpr std::int64_t kBase = 10;
constexpr std::int64_t kMillisecondsPerSecond = 1000;
constexpr std::size_t kDecimalsHeld = 3;

std::string refusal(std::string_view text, std::string_view why) {
    return "\"" + std::string{text} + "\" is not a time: " + std::string{why};
}

/// Why a text is not the seconds form.
enum class NotSeconds {
    Shape,    ///< it is not of that shape at all; a timestamp may still fit
    TooLarge, ///< it is, but no position could hold the number
};

/// Multiplies by ten and adds `digit`, or nothing if that would overflow.
///
/// Checked before it happens rather than detected after. Left unguarded, the
/// accumulation is undefined behaviour, and what came out of the wrapping was
/// then written to a file as if it had been asked for.
std::optional<std::int64_t> appended(std::int64_t value, std::int64_t digit) {
    constexpr std::int64_t kLargest = std::numeric_limits<std::int64_t>::max();
    if (value > (kLargest - digit) / kBase) {
        return std::nullopt;
    }
    return (value * kBase) + digit;
}

/// Reads `[-]S[.mmm]`, or says why the text is not of that shape.
///
/// Written out rather than handed to `std::from_chars` on a double: seconds
/// with three decimals are exactly what a binary float cannot hold, and
/// `7.001` would come back as a hair under seven thousand and one
/// milliseconds. Counting the digits keeps the arithmetic in integers.
std::expected<std::int64_t, NotSeconds> secondsFormOf(std::string_view text) {
    std::int64_t sign = 1;
    if (text.starts_with('-')) {
        sign = -1;
        text.remove_prefix(1);
    } else if (text.starts_with('+')) {
        text.remove_prefix(1);
    }
    if (text.empty()) {
        return std::unexpected{NotSeconds::Shape};
    }

    const std::size_t dot = text.find('.');
    const std::string_view whole = text.substr(0, dot);
    const std::string_view decimals = dot == std::string_view::npos ? "" : text.substr(dot + 1);

    const auto isDigit = [](char c) { return c >= '0' && c <= '9'; };
    if (whole.empty() || !std::ranges::all_of(whole, isDigit)) {
        return std::unexpected{NotSeconds::Shape};
    }
    if (dot != std::string_view::npos &&
        (decimals.empty() || !std::ranges::all_of(decimals, isDigit))) {
        return std::unexpected{NotSeconds::Shape};
    }

    std::int64_t total = 0;
    for (const char digit : whole) {
        const std::optional<std::int64_t> grown = appended(total, digit - '0');
        if (!grown.has_value()) {
            return std::unexpected{NotSeconds::TooLarge};
        }
        total = *grown;
    }

    // Seconds become milliseconds, and a count that held may stop holding: the
    // guard covers the scaling as well as the reading.
    constexpr std::int64_t kLargest = std::numeric_limits<std::int64_t>::max();
    if (total > kLargest / kMillisecondsPerSecond) {
        return std::unexpected{NotSeconds::TooLarge};
    }
    total *= kMillisecondsPerSecond;

    // Padded to three, so that `.5` is five hundred milliseconds and not five.
    std::int64_t fraction = 0;
    std::size_t counted = 0;
    for (const char digit : decimals) {
        fraction = fraction * kBase + (digit - '0');
        ++counted;
    }
    for (; counted < kDecimalsHeld; ++counted) {
        fraction *= kBase;
    }

    // And the decimals on top of them, which the scaling guard above leaves
    // room for in every case but the very largest.
    if (total > kLargest - fraction) {
        return std::unexpected{NotSeconds::TooLarge};
    }
    return sign * (total + fraction);
}

std::size_t decimalCountOf(std::string_view text) {
    const std::size_t dot = text.find('.');
    return dot == std::string_view::npos ? 0 : text.size() - dot - 1;
}

} // namespace

std::expected<Duration, std::string> parseTime(std::string_view text) {
    if (text.empty()) {
        return std::unexpected{refusal(text, "it is empty")};
    }
    if (text.contains(',')) {
        return std::unexpected{
            refusal(text, "use a decimal point and not a comma, the command line being English")};
    }
    if (decimalCountOf(text) > kDecimalsHeld) {
        return std::unexpected{refusal(text, "positions are held to the millisecond, no finer")};
    }

    const std::expected<std::int64_t, NotSeconds> seconds = secondsFormOf(text);
    if (seconds.has_value()) {
        return Duration::fromMilliseconds(*seconds);
    }
    if (seconds.error() == NotSeconds::TooLarge) {
        return std::unexpected{
            refusal(text, "it is too large to hold, positions being counted in milliseconds")};
    }

    // A timestamp, then. The core reads those, and more permissively than this
    // grammar allows — which is why the decimal mark was checked above rather
    // than left to it.
    if (const std::optional<core::Timestamp> stamp = core::Timestamp::parse(text);
        stamp.has_value()) {
        return Duration::fromMilliseconds(stamp->milliseconds());
    }

    return std::unexpected{
        refusal(text, "expected seconds like 2.999, or a timestamp like 00:00:07.001")};
}

} // namespace subedit::cli
