#include <subedit/core/format/lrc_syntax.hpp>
#include <subedit/core/time/timestamp.hpp>

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>

namespace subedit::core {

namespace {

constexpr std::int64_t kDecimalBase = 10;
constexpr std::int64_t kMillisecondsPerHundredth = 10;
constexpr std::int64_t kMillisecondsPerSecond = 1000;
constexpr std::int64_t kSecondsPerMinute = 60;
constexpr std::int64_t kMillisecondsPerMinute = kMillisecondsPerSecond * kSecondsPerMinute;
constexpr std::size_t kFieldDigits = 2;

/// Wide enough for any position a real file carries — a hundred million
/// minutes is a hundred and ninety years — and narrow enough not to overflow.
constexpr std::size_t kMaxMinuteDigits = 8;

/// Appends `value` as exactly two digits.
void appendTwoDigits(std::string& text, std::int64_t value) {
    text += static_cast<char>('0' + (value / kDecimalBase));
    text += static_cast<char>('0' + (value % kDecimalBase));
}

/// Reads a run of decimal digits as a number, or nothing if the run is not
/// between `least` and `most` digits long.
[[nodiscard]] std::optional<std::int64_t>
readDigits(std::string_view line, std::size_t& index, std::size_t least, std::size_t most) {
    const std::size_t first = index;
    std::int64_t value = 0;
    while (index < line.size() && line[index] >= '0' && line[index] <= '9') {
        if (index - first == most)
            return std::nullopt;
        value = (value * kDecimalBase) + (line[index] - '0');
        ++index;
    }

    if (index - first < least)
        return std::nullopt;
    return value;
}

} // namespace

std::optional<LrcTimeLine> parseLrcTimeLine(std::string_view line) {
    std::size_t index = 0;
    if (index >= line.size() || line[index] != '[')
        return std::nullopt;
    ++index;

    const bool negative = index < line.size() && line[index] == '-';
    if (negative)
        ++index;

    const std::optional<std::int64_t> minutes =
        readDigits(line, index, kFieldDigits, kMaxMinuteDigits);
    if (!minutes.has_value() || index >= line.size() || line[index] != ':')
        return std::nullopt;
    ++index;

    const std::optional<std::int64_t> seconds = readDigits(line, index, kFieldDigits, kFieldDigits);
    if (!seconds.has_value() || *seconds >= kSecondsPerMinute || index >= line.size() ||
        line[index] != '.')
        return std::nullopt;
    ++index;

    const std::optional<std::int64_t> hundredths =
        readDigits(line, index, kFieldDigits, kFieldDigits);
    if (!hundredths.has_value() || index >= line.size() || line[index] != ']')
        return std::nullopt;
    ++index;

    const std::int64_t total = (*minutes * kMillisecondsPerMinute) +
                               (*seconds * kMillisecondsPerSecond) +
                               (*hundredths * kMillisecondsPerHundredth);
    return LrcTimeLine{
        .start = Timestamp::fromMilliseconds(negative ? -total : total),
        .textAt = index,
    };
}

std::string lrcTimeOf(Timestamp position) {
    const std::int64_t milliseconds = position.milliseconds();
    const std::int64_t half = kMillisecondsPerHundredth / 2;
    const std::int64_t rounded = milliseconds < 0
                                     ? (milliseconds - half) / kMillisecondsPerHundredth
                                     : (milliseconds + half) / kMillisecondsPerHundredth;

    const std::int64_t magnitude = rounded < 0 ? -rounded : rounded;
    const std::int64_t hundredthsPerMinute = kMillisecondsPerMinute / kMillisecondsPerHundredth;
    const std::int64_t hundredthsPerSecond = kMillisecondsPerSecond / kMillisecondsPerHundredth;

    std::string text;
    if (rounded < 0)
        text += '-';

    // **The minutes are padded to two digits and cut at none.** A file past an
    // hour writes `[62:03.00]`, which is what this format has instead of an
    // hours field.
    const std::string minutes = std::to_string(magnitude / hundredthsPerMinute);
    if (minutes.size() < kFieldDigits)
        text += '0';
    text += minutes;

    text += ':';
    appendTwoDigits(text, magnitude / hundredthsPerSecond % kSecondsPerMinute);
    text += '.';
    appendTwoDigits(text, magnitude % hundredthsPerSecond);
    return text;
}

} // namespace subedit::core
