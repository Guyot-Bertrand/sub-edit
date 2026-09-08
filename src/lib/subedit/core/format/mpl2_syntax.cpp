#include <subedit/core/format/mpl2_syntax.hpp>
#include <subedit/core/time/timestamp.hpp>

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>

namespace subedit::core {

namespace {

constexpr std::int64_t kMillisecondsPerTenth = 100;
constexpr std::int64_t kDecimalBase = 10;

/// Wide enough for any position a real file carries — a tenth of a second times
/// ten digits is three years — and narrow enough not to overflow.
constexpr std::size_t kMaxDigits = 10;

/// Reads `[-68]` at `index`, and moves it past the closing bracket.
[[nodiscard]] std::optional<std::int64_t> readBracketed(std::string_view line, std::size_t& index) {
    if (index >= line.size() || line[index] != '[')
        return std::nullopt;
    ++index;

    const bool negative = index < line.size() && line[index] == '-';
    if (negative)
        ++index;

    const std::size_t first = index;
    std::int64_t value = 0;
    while (index < line.size() && line[index] >= '0' && line[index] <= '9') {
        if (index - first == kMaxDigits)
            return std::nullopt;
        value = (value * kDecimalBase) + (line[index] - '0');
        ++index;
    }

    if (index == first || index >= line.size() || line[index] != ']')
        return std::nullopt;
    ++index;

    return negative ? -value : value;
}

} // namespace

std::optional<Mpl2TimeLine> parseMpl2TimeLine(std::string_view line) {
    std::size_t index = 0;
    const std::optional<std::int64_t> start = readBracketed(line, index);
    if (!start.has_value())
        return std::nullopt;

    const std::optional<std::int64_t> end = readBracketed(line, index);
    if (!end.has_value())
        return std::nullopt;

    return Mpl2TimeLine{
        .start = Timestamp::fromMilliseconds(*start * kMillisecondsPerTenth),
        .end = Timestamp::fromMilliseconds(*end * kMillisecondsPerTenth),
        .textAt = index,
    };
}

std::string tenthsOf(Timestamp position) {
    const std::int64_t milliseconds = position.milliseconds();
    const std::int64_t half = kMillisecondsPerTenth / 2;
    const std::int64_t rounded = milliseconds < 0 ? (milliseconds - half) / kMillisecondsPerTenth
                                                  : (milliseconds + half) / kMillisecondsPerTenth;
    return std::to_string(rounded);
}

} // namespace subedit::core
