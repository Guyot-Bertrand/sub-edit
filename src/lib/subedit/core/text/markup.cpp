#include <subedit/core/text/markup.hpp>

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <utility>

namespace subedit::core {

namespace {

constexpr std::size_t kChannelDigits = 2;
constexpr std::size_t kColourDigits = 6;
constexpr int kHexBase = 16;
constexpr int kTen = 10;

/// The value of one hexadecimal digit, or nothing if it is not one.
[[nodiscard]] std::optional<int> valueOfDigit(char digit) {
    if (digit >= '0' && digit <= '9')
        return digit - '0';
    if (digit >= 'a' && digit <= 'f')
        return digit - 'a' + kTen;
    if (digit >= 'A' && digit <= 'F')
        return digit - 'A' + kTen;
    return std::nullopt;
}

/// Reads two hexadecimal digits at `index` as one channel.
[[nodiscard]] std::optional<std::uint8_t> channelAt(std::string_view text, std::size_t index) {
    const std::optional<int> high = valueOfDigit(text[index]);
    const std::optional<int> low = valueOfDigit(text[index + 1]);
    if (!high.has_value() || !low.has_value())
        return std::nullopt;
    return static_cast<std::uint8_t>((*high * kHexBase) + *low);
}

/// Appends `value` as exactly two hexadecimal digits, in lower case.
void appendChannel(std::string& text, std::uint8_t value) {
    constexpr std::string_view kDigits = "0123456789abcdef";
    // The two halves of a byte are never negative; the cast says so where the
    // promotion to `int` would otherwise leave the compiler to wonder.
    text += kDigits[static_cast<std::size_t>(value / kHexBase)];
    text += kDigits[static_cast<std::size_t>(value % kHexBase)];
}

} // namespace

std::optional<Colour> Colour::parse(std::string_view text) {
    if (text.size() != kColourDigits)
        return std::nullopt;

    const std::optional<std::uint8_t> red = channelAt(text, 0);
    const std::optional<std::uint8_t> green = channelAt(text, kChannelDigits);
    const std::optional<std::uint8_t> blue = channelAt(text, 2 * kChannelDigits);
    if (!red.has_value() || !green.has_value() || !blue.has_value())
        return std::nullopt;

    return Colour{*red, *green, *blue};
}

std::optional<Colour> Colour::parseReversed(std::string_view text) {
    if (text.size() > kColourDigits)
        return std::nullopt;

    // Padded on the left, which is what a file that omitted its leading zeroes
    // meant: `ff00` is `00ff00`, green, and not `ff0000`, red.
    std::string padded(kColourDigits - text.size(), '0');
    padded += text;

    const std::optional<Colour> reversed = parse(padded);
    if (!reversed.has_value())
        return std::nullopt;
    return Colour{reversed->m_blue, reversed->m_green, reversed->m_red};
}

std::string Colour::toString() const {
    std::string text;
    appendChannel(text, m_red);
    appendChannel(text, m_green);
    appendChannel(text, m_blue);
    return text;
}

std::string Colour::toReversedString() const {
    std::string text;
    appendChannel(text, m_blue);
    appendChannel(text, m_green);
    appendChannel(text, m_red);
    return text;
}

bool Style::carries(StyleAttribute attribute) const {
    switch (attribute) {
    case StyleAttribute::Bold:
        return bold;
    case StyleAttribute::Italic:
        return italic;
    case StyleAttribute::Underline:
        return underline;
    case StyleAttribute::Colour:
        return colour.has_value();
    case StyleAttribute::Font:
        return font.has_value();
    case StyleAttribute::Size:
        return size.has_value();
    }
    std::unreachable();
}

Style Style::without(StyleAttribute attribute) const {
    Style rest = *this;
    switch (attribute) {
    case StyleAttribute::Bold:
        rest.bold = false;
        break;
    case StyleAttribute::Italic:
        rest.italic = false;
        break;
    case StyleAttribute::Underline:
        rest.underline = false;
        break;
    case StyleAttribute::Colour:
        rest.colour.reset();
        break;
    case StyleAttribute::Font:
        rest.font.reset();
        break;
    case StyleAttribute::Size:
        rest.size.reset();
        break;
    }
    return rest;
}

std::size_t Style::count() const {
    std::size_t said = 0;
    for (const StyleAttribute attribute : kStyleAttributes) {
        if (carries(attribute))
            ++said;
    }
    return said;
}

void appendRun(StyledText& runs, std::string_view text, const Style& style) {
    if (text.empty())
        return;

    if (!runs.empty() && runs.back().style == style) {
        runs.back().text += text;
        return;
    }
    runs.push_back(StyledRun{.text = std::string{text}, .style = style});
}

std::string plainTextOf(const StyledText& runs) {
    std::string text;
    for (const StyledRun& run : runs)
        text += run.text;
    return text;
}

} // namespace subedit::core
