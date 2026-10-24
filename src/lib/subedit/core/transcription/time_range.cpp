#include <subedit/core/transcription/time_range.hpp>

#include <charconv>
#include <cmath>
#include <cstdint>
#include <string>

namespace subedit::core {

namespace {

constexpr double kMillisecondsPerSecond = 1000.0;

/// Seconds as a number, nothing else: no sign, no exponent, no trailing text.
[[nodiscard]] std::optional<Timestamp> readSeconds(std::string_view text) {
    if (text.empty() || text.find_first_not_of("0123456789.") != std::string_view::npos)
        return std::nullopt;
    double seconds = 0;
    const std::from_chars_result parsed =
        std::from_chars(text.data(), text.data() + text.size(), seconds);
    if (parsed.ec != std::errc{} || parsed.ptr != text.data() + text.size())
        return std::nullopt;
    return timestampFromSeconds(seconds);
}

} // namespace

Timestamp timestampFromSeconds(double seconds) {
    const auto milliseconds = std::llround(seconds * kMillisecondsPerSecond);
    return Timestamp::fromMilliseconds(static_cast<std::int64_t>(milliseconds));
}

std::optional<Timestamp> readRangeBound(std::string_view text) {
    const std::size_t first = text.find_first_not_of(' ');
    if (first == std::string_view::npos)
        return std::nullopt;
    text = text.substr(first, text.find_last_not_of(' ') - first + 1);
    if (text.find(':') == std::string_view::npos)
        return readSeconds(text);
    if (text.starts_with('-') || text.starts_with('+') || text.find(',') != std::string_view::npos)
        return std::nullopt;
    return Timestamp::parse(text);
}

bool isRangePossible(const std::optional<Timestamp>& start, const std::optional<Timestamp>& end) {
    return !start || !end || *start < *end;
}

} // namespace subedit::core
