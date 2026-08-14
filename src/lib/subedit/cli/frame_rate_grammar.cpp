#include <subedit/cli/frame_rate_grammar.hpp>

#include <algorithm>
#include <array>
#include <cstdint>
#include <limits>
#include <optional>

namespace subedit::cli {

namespace {

using core::FrameRate;
using core::StandardFrameRate;

constexpr std::int64_t kBase = 10;
constexpr std::int64_t kLargest = std::numeric_limits<std::int64_t>::max();

std::string refusal(std::string_view text, std::string_view why) {
    return "\"" + std::string{text} + "\" is not a frame rate: " + std::string{why};
}

/// A decimal that is the customary name of a rate it cannot write exactly.
///
/// Held as the **reduced** fraction of the decimal, because that is what the
/// parse produces and what makes `23.9760` the same entry as `23.976`.
struct Label {
    std::int64_t numerator;
    std::int64_t denominator;
    StandardFrameRate names;
};

/// The three NTSC rates, and nothing else: those are the only standard rates
/// whose usual spelling is not their value. The five others — 24, 25, 30, 50,
/// 60 — write themselves exactly, so reading them literally already lands on
/// the right rate and no entry is owed here.
constexpr std::array<Label, 3> kLabels = {
    Label{.numerator = 2997, .denominator = 125, .names = StandardFrameRate::Fps23976}, // 23.976
    Label{.numerator = 2997, .denominator = 100, .names = StandardFrameRate::Fps29970}, // 29.97
    Label{.numerator = 2997, .denominator = 50, .names = StandardFrameRate::Fps59940},  // 59.94
};

/// A decimal read into a fraction: `23.976` becomes 23976 over 1000.
struct Decimal {
    std::int64_t numerator;
    std::int64_t denominator;
    bool negative;
};

/// Multiplies by ten and adds `digit`, or nothing if that would overflow.
///
/// Checked before it happens rather than detected after: a wrapped value would
/// name some other rate, which is worse than a refusal.
std::optional<std::int64_t> appended(std::int64_t value, std::int64_t digit) {
    if (value > (kLargest - digit) / kBase) {
        return std::nullopt;
    }
    return (value * kBase) + digit;
}

/// Reads `[±]digits[.digits]`, or nothing when the text is not of that shape.
///
/// Written out rather than handed to `std::from_chars` on a double for the
/// reason ADR 0013 gives: a rate is kept as an exact rational, and going
/// through a binary float to build one would throw away the exactness on the
/// way in.
std::optional<Decimal> decimalOf(std::string_view text) {
    Decimal read{.numerator = 0, .denominator = 1, .negative = false};
    if (text.starts_with('-')) {
        read.negative = true;
        text.remove_prefix(1);
    } else if (text.starts_with('+')) {
        text.remove_prefix(1);
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

    for (const char digit : whole) {
        const std::optional<std::int64_t> grown = appended(read.numerator, digit - '0');
        if (!grown.has_value()) {
            return std::nullopt;
        }
        read.numerator = *grown;
    }
    for (const char digit : decimals) {
        const std::optional<std::int64_t> grown = appended(read.numerator, digit - '0');
        const std::optional<std::int64_t> scaled = appended(read.denominator, 0);
        if (!grown.has_value() || !scaled.has_value()) {
            return std::nullopt;
        }
        read.numerator = *grown;
        read.denominator = *scaled;
    }
    return read;
}

/// Returns the standard rate `rate` is the customary name of, if it is one.
std::optional<FrameRate> labelled(FrameRate rate) {
    for (const Label& label : kLabels) {
        if (rate.numerator() == label.numerator && rate.denominator() == label.denominator) {
            return FrameRate{label.names};
        }
    }
    return std::nullopt;
}

} // namespace

std::expected<FrameRate, std::string> parseFrameRate(std::string_view text) {
    if (text.empty()) {
        return std::unexpected{refusal(text, "it is empty")};
    }
    if (text.contains(',')) {
        return std::unexpected{
            refusal(text, "use a decimal point and not a comma, the command line being English")};
    }

    const std::optional<Decimal> read = decimalOf(text);
    if (!read.has_value()) {
        return std::unexpected{refusal(text, "expected frames per second, like 25 or 23.976")};
    }
    if (read->negative || read->numerator == 0) {
        return std::unexpected{
            refusal(text, "a frame rate must be strictly positive; nothing runs at that speed")};
    }

    const std::optional<FrameRate> rate = FrameRate::create(read->numerator, read->denominator);
    if (!rate.has_value()) {
        return std::unexpected{refusal(text, "no video runs at that many frames per second")};
    }

    // The label wins over the letters: see the header for why.
    return labelled(*rate).value_or(*rate);
}

} // namespace subedit::cli
