#include <subedit/cli/time_grammar.hpp>
#include <subedit/core/time/timestamp.hpp>

#include <algorithm>
#include <cstdint>
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

/// Reads `[-]S[.mmm]`, or nothing when the text is not of that shape.
///
/// Written out rather than handed to `std::from_chars` on a double: seconds
/// with three decimals are exactly what a binary float cannot hold, and
/// `7.001` would come back as a hair under seven thousand and one
/// milliseconds. Counting the digits keeps the arithmetic in integers.
std::optional<std::int64_t> secondsFormOf(std::string_view text) {
    std::int64_t sign = 1;
    if (text.starts_with('-')) {
        sign = -1;
        text.remove_prefix(1);
    } else if (text.starts_with('+')) {
        text.remove_prefix(1);
    }
    if (text.empty()) {
        return std::nullopt;
    }

    const std::size_t dot = text.find('.');
    const std::string_view whole = text.substr(0, dot);
    const std::string_view decimals = dot == std::string_view::npos ? "" : text.substr(dot + 1);

    const auto isDigit = [](char c) { return c >= '0' && c <= '9'; };
    if (whole.empty() || !std::ranges::all_of(whole, isDigit)) {
        return std::nullopt;
    }
    if (dot != std::string_view::npos &&
        (decimals.empty() || !std::ranges::all_of(decimals, isDigit))) {
        return std::nullopt;
    }

    std::int64_t total = 0;
    for (const char digit : whole) {
        total = total * kBase + (digit - '0');
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

    if (const std::optional<std::int64_t> seconds = secondsFormOf(text); seconds.has_value()) {
        return Duration::fromMilliseconds(*seconds);
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
