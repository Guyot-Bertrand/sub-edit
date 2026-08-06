#include <subedit/core/time/timestamp.hpp>

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>

namespace subedit::core {

namespace {

constexpr std::int64_t kDecimalBase = 10;
constexpr std::size_t kFieldDigits = 2;
constexpr std::size_t kFractionDigits = 3;
constexpr std::size_t kMaxFields = 3;
constexpr std::size_t kMinFields = 2;
constexpr std::string_view kBlanks = " \t\r\n";
constexpr std::string_view kDecimalMarks = ",.";

/// Appends `value` as exactly `width` digits, padding with zeroes.
void appendDigits(std::string& text, std::int64_t value, std::size_t width) {
    std::array<char, kFractionDigits> digits{};
    std::int64_t rest = value;
    for (std::size_t index = width; index > 0; --index) {
        digits[index - 1] = static_cast<char>('0' + (rest % kDecimalBase));
        rest /= kDecimalBase;
    }
    text.append(digits.data(), width);
}

/// Strips the blanks around `text`.
std::string_view trimmed(std::string_view text) {
    const std::size_t first = text.find_first_not_of(kBlanks);
    if (first == std::string_view::npos)
        return {};
    return text.substr(first, text.find_last_not_of(kBlanks) - first + 1);
}

/// Reads a run of at most `maxDigits` decimal digits, and nothing else.
std::optional<std::int64_t> readInteger(std::string_view field, std::size_t maxDigits) {
    if (field.empty() || field.size() > maxDigits)
        return std::nullopt;

    std::int64_t value = 0;
    for (const char character : field) {
        if (character < '0' || character > '9')
            return std::nullopt;
        value = (value * kDecimalBase) + (character - '0');
    }
    return value;
}

/// Reads the decimals of the seconds as a millisecond count: `4` is 400 ms,
/// `45` is 450 ms, `456` is 456 ms.
std::optional<std::int64_t> readFraction(std::string_view digits) {
    const std::optional<std::int64_t> value = readInteger(digits, kFractionDigits);
    if (!value.has_value())
        return std::nullopt;

    std::int64_t milliseconds = *value;
    for (std::size_t written = digits.size(); written < kFractionDigits; ++written)
        milliseconds *= kDecimalBase;
    return milliseconds;
}

} // namespace

std::string Timestamp::format(DecimalMark mark, HourField hours) const {
    // Clamping before taking the magnitude, rather than after, keeps the most
    // negative representable value from overflowing on negation.
    std::int64_t magnitude = m_milliseconds;
    if (magnitude < 0)
        magnitude =
            -(magnitude < -kMaxWritableMilliseconds ? -kMaxWritableMilliseconds : magnitude);
    else if (magnitude > kMaxWritableMilliseconds)
        magnitude = kMaxWritableMilliseconds;

    const std::int64_t hourCount = magnitude / kMillisecondsPerHour;

    std::string text;
    if (m_milliseconds < 0)
        text += '-';
    // Below one hour the hours may be dropped, as WebVTT allows. Above it they
    // come back: minutes past fifty-nine would not read back.
    if (hours == HourField::Always || hourCount > 0) {
        appendDigits(text, hourCount, kFieldDigits);
        text += ':';
    }
    appendDigits(text, magnitude / kMillisecondsPerMinute % kMinutesPerHour, kFieldDigits);
    text += ':';
    appendDigits(text, magnitude / kMillisecondsPerSecond % kSecondsPerMinute, kFieldDigits);
    text += mark == DecimalMark::Comma ? ',' : '.';
    appendDigits(text, magnitude % kMillisecondsPerSecond, kFractionDigits);
    return text;
}

std::optional<Timestamp> Timestamp::parse(std::string_view text) {
    std::string_view rest = trimmed(text);
    if (rest.empty())
        return std::nullopt;

    const bool negative = rest.front() == '-';
    if (negative)
        rest.remove_prefix(1);

    std::int64_t fraction = 0;
    if (const std::size_t mark = rest.find_first_of(kDecimalMarks);
        mark != std::string_view::npos) {
        const std::optional<std::int64_t> decimals = readFraction(rest.substr(mark + 1));
        if (!decimals.has_value())
            return std::nullopt;
        fraction = *decimals;
        rest = rest.substr(0, mark);
    }

    std::array<std::int64_t, kMaxFields> fields{};
    std::size_t fieldCount = 0;
    while (!rest.empty()) {
        if (fieldCount == kMaxFields)
            return std::nullopt;

        const std::size_t separator = rest.find(':');
        const std::optional<std::int64_t> field =
            readInteger(rest.substr(0, separator), kFieldDigits);
        if (!field.has_value())
            return std::nullopt;
        fields[fieldCount] = *field;
        ++fieldCount;

        if (separator == std::string_view::npos)
            break;
        rest = rest.substr(separator + 1);
        if (rest.empty())
            return std::nullopt;
    }

    if (fieldCount < kMinFields)
        return std::nullopt;

    // Two fields mean the hours were left out, three that they were written.
    const std::int64_t hours = fieldCount == kMaxFields ? fields.front() : 0;
    const std::int64_t minutes = fields[fieldCount - 2];
    const std::int64_t seconds = fields[fieldCount - 1];
    if (minutes >= kMinutesPerHour || seconds >= kSecondsPerMinute)
        return std::nullopt;

    const std::int64_t total = (hours * kMillisecondsPerHour) + (minutes * kMillisecondsPerMinute) +
                               (seconds * kMillisecondsPerSecond) + fraction;
    return Timestamp{negative ? -total : total};
}

} // namespace subedit::core
