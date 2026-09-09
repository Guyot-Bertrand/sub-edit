#include <subedit/core/format/tm_player_syntax.hpp>
#include <subedit/core/time/timestamp.hpp>

#include <cstddef>
#include <optional>
#include <string>
#include <string_view>

namespace subedit::core {

namespace {

constexpr std::size_t kFieldDigits = 2;

/// Counts the decimal digits `line` holds from `index` on.
[[nodiscard]] std::size_t digitRun(std::string_view line, std::size_t index) {
    std::size_t digits = 0;
    while (index + digits < line.size() && line[index + digits] >= '0' &&
           line[index + digits] <= '9')
        ++digits;
    return digits;
}

/// Reads a field of exactly `digits` digits followed by a colon.
[[nodiscard]] bool skipField(std::string_view line, std::size_t& index, std::size_t digits) {
    if (digitRun(line, index) != digits)
        return false;
    index += digits;
    if (index >= line.size() || line[index] != ':')
        return false;
    ++index;
    return true;
}

} // namespace

std::optional<TMPlayerTimeLine> parseTMPlayerTimeLine(std::string_view line) {
    std::size_t index = 0;
    if (index < line.size() && line[index] == '-')
        ++index;

    // **The hour field is one digit or two, and which one is the whole of what
    // the file declares about its own shape.** Gaupol matches the two forms
    // with two patterns; here one walk answers both, and says which it saw.
    const std::size_t hourDigits = digitRun(line, index);
    if (hourDigits < 1 || hourDigits > kFieldDigits)
        return std::nullopt;

    index += hourDigits;
    if (index >= line.size() || line[index] != ':')
        return std::nullopt;
    ++index;

    if (!skipField(line, index, kFieldDigits) || !skipField(line, index, kFieldDigits))
        return std::nullopt;

    // The trailing colon is the format's separator, not part of the timestamp:
    // `Timestamp::parse` is given the three fields and nothing else, and it is
    // what refuses a sixty-first minute.
    const std::optional<Timestamp> position = Timestamp::parse(line.substr(0, index - 1));
    if (!position.has_value())
        return std::nullopt;

    return TMPlayerTimeLine{
        .start = *position,
        .twoDigitHour = hourDigits == kFieldDigits,
        .textAt = index,
    };
}

std::string tmPlayerTimeOf(Timestamp position, bool twoDigitHour) {
    // The decimal mark is asked for and never written: `Decimals::Seconds`
    // keeps no digits, so it keeps no separator either.
    return position.format(DecimalMark::Period,
                           twoDigitHour ? HourField::Always : HourField::Unpadded,
                           Decimals::Seconds);
}

} // namespace subedit::core
