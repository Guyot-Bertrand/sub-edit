#include <subedit/core/format/micro_dvd_syntax.hpp>
#include <subedit/core/time/frame.hpp>

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string_view>

namespace subedit::core {

namespace {

constexpr std::string_view kHeaderKeyword = "{DEFAULT}";
constexpr std::int64_t kDecimalBase = 10;

/// Wide enough for any frame count a real film carries — ten digits is three
/// months at sixty frames a second — and narrow enough not to overflow.
constexpr std::size_t kMaxDigits = 10;

/// Reads `{25}` at `index`, and moves it past the closing brace.
[[nodiscard]] std::optional<std::int64_t> readBraced(std::string_view line, std::size_t& index) {
    if (index >= line.size() || line[index] != '{')
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

    if (index == first || index >= line.size() || line[index] != '}')
        return std::nullopt;
    ++index;

    return negative ? -value : value;
}

} // namespace

std::optional<MicroDvdFrameLine> parseMicroDvdFrameLine(std::string_view line) {
    std::size_t index = 0;
    const std::optional<std::int64_t> start = readBraced(line, index);
    if (!start.has_value())
        return std::nullopt;

    const std::optional<std::int64_t> end = readBraced(line, index);
    if (!end.has_value())
        return std::nullopt;

    return MicroDvdFrameLine{
        .start = Frame::fromNumber(*start),
        .end = Frame::fromNumber(*end),
        .textAt = index,
    };
}

bool isMicroDvdHeader(std::string_view line) {
    return line.starts_with(kHeaderKeyword);
}

} // namespace subedit::core
