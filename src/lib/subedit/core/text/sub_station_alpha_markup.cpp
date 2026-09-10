#include <subedit/core/text/markup.hpp>
#include <subedit/core/text/markup_codec.hpp>
#include <subedit/core/text/markup_vocabulary.hpp>
#include <subedit/core/text/sub_station_alpha_markup.hpp>

#include <algorithm>
#include <cstddef>
#include <optional>
#include <string>
#include <string_view>

namespace subedit::core {

namespace {

constexpr std::string_view kColourPrefix = "c&H";
constexpr std::string_view kFontPrefix = "fn";
constexpr std::string_view kSizePrefix = "fs";
constexpr int kDecimalBase = 10;

/// Reads a run of decimal digits as a number, or nothing if there is none.
[[nodiscard]] std::optional<int> wholeNumber(std::string_view text) {
    if (text.empty())
        return std::nullopt;
    int value = 0;
    for (const char digit : text) {
        if (digit < '0' || digit > '9')
            return std::nullopt;
        value = (value * kDecimalBase) + (digit - '0');
    }
    return value;
}

/// Applies one override to `style`, or says it meant nothing here.
///
/// `{\b700}` is bold and `{\b0}` is not: Advanced SSA writes a weight where Sub
/// Station Alpha writes a flag, and any weight above zero is bold.
[[nodiscard]] bool applyOverride(Style& style, std::string_view override) {
    if (override.empty())
        return false;

    const char letter = override.front();
    const std::string_view rest = override.substr(1);

    if (letter == 'r') {
        // The reset, with or without a style name: everything closes at once.
        style = Style{};
        return true;
    }

    if ((letter == 'b' || letter == 'i' || letter == 'u') && !rest.empty()) {
        const std::optional<int> weight = wholeNumber(rest);
        if (!weight.has_value())
            return false;
        const bool on = *weight > 0;
        if (letter == 'b')
            style.bold = on;
        else if (letter == 'i')
            style.italic = on;
        else
            style.underline = on;
        return true;
    }

    if (override.starts_with(kColourPrefix) && override.ends_with('&')) {
        const std::string_view digits =
            override.substr(kColourPrefix.size(), override.size() - kColourPrefix.size() - 1);
        const std::optional<Colour> colour = Colour::parseReversed(digits);
        if (!colour.has_value())
            return false;
        style.colour = colour;
        return true;
    }

    if (override.starts_with(kFontPrefix) && override.size() > kFontPrefix.size()) {
        style.font = std::string{override.substr(kFontPrefix.size())};
        return true;
    }

    if (override.starts_with(kSizePrefix)) {
        const std::optional<int> points = wholeNumber(override.substr(kSizePrefix.size()));
        if (!points.has_value())
            return false;
        style.size = points;
        return true;
    }

    return false;
}

/// Appends the override that turns `attribute` on, reading it from `style`.
void appendOn(std::string& out, StyleAttribute attribute, const Style& style) {
    switch (attribute) {
    case StyleAttribute::Bold:
        out += R"({\b1})";
        break;
    case StyleAttribute::Italic:
        out += R"({\i1})";
        break;
    case StyleAttribute::Underline:
        out += R"({\u1})";
        break;
    // **The checks are what read the values, not guards against them.** The
    // caller only asks for an override the style carries, and nothing in the
    // type says so; asking here costs three branches nobody takes and makes
    // the function total rather than conditional on its caller's manners.
    case StyleAttribute::Colour:
        if (style.colour.has_value()) {
            out += R"({\c&H)";
            out += style.colour->toReversedString();
            out += "&}";
        }
        break;
    case StyleAttribute::Font:
        if (style.font.has_value()) {
            out += R"({\fn)";
            out += *style.font;
            out += '}';
        }
        break;
    case StyleAttribute::Size:
        if (style.size.has_value()) {
            out += R"({\fs)";
            out += std::to_string(*style.size);
            out += '}';
        }
        break;
    }
}

/// Tells whether `attribute` can be turned off without resetting everything.
[[nodiscard]] bool closesOnItsOwn(StyleAttribute attribute) {
    return attribute == StyleAttribute::Bold || attribute == StyleAttribute::Italic ||
           attribute == StyleAttribute::Underline;
}

/// Appends the override that turns `attribute` off.
///
/// **Only the three `closesOnItsOwn` names get here**, and the caller is what
/// keeps it so: a colour, a font or a size goes away with `{\r}` and with
/// nothing else, so there is no fourth override to write and no arm here that
/// no text can reach.
void appendOff(std::string& out, StyleAttribute attribute) {
    out += R"({\)";
    if (attribute == StyleAttribute::Bold)
        out += 'b';
    else if (attribute == StyleAttribute::Italic)
        out += 'i';
    else
        out += 'u';
    out += "0}";
}

/// Tells whether going from `current` to `wanted` needs a `{\r}`.
[[nodiscard]] bool needsReset(const Style& current, const Style& wanted) {
    return std::ranges::any_of(kStyleAttributes, [&](StyleAttribute attribute) {
        return !closesOnItsOwn(attribute) && current.carries(attribute) &&
               !wanted.carries(attribute);
    });
}

/// Tells whether two styles say the same thing about `attribute`.
[[nodiscard]] bool agree(const Style& left, const Style& right, StyleAttribute attribute) {
    switch (attribute) {
    case StyleAttribute::Colour:
        return left.colour == right.colour;
    case StyleAttribute::Font:
        return left.font == right.font;
    case StyleAttribute::Size:
        return left.size == right.size;
    default:
        return left.carries(attribute) == right.carries(attribute);
    }
}

} // namespace

DecodedMarkup decodeSubStationAlphaMarkup(std::string_view text) {
    DecodedMarkup read;
    Style current;
    std::size_t start = 0;

    while (start < text.size()) {
        const std::size_t opening = text.find('{', start);
        if (opening == std::string_view::npos)
            break;
        const std::size_t closing = text.find('}', opening);
        if (closing == std::string_view::npos)
            break;

        appendRun(read.runs, text.substr(start, opening - start), current);

        // **A block holds as many overrides as it likes**, and `{\b1\i1}` says
        // two things. Splitting on the backslash is what Gaupol does first too.
        std::string_view block = text.substr(opening + 1, closing - opening - 1);
        while (!block.empty()) {
            if (block.front() != '\\') {
                // Not an override at all: `{some note}` is a comment in the wild.
                ++read.unknown;
                break;
            }
            block.remove_prefix(1);
            const std::size_t next = block.find('\\');
            const std::string_view override = block.substr(0, next);
            if (!applyOverride(current, override))
                ++read.unknown;
            block = next == std::string_view::npos ? std::string_view{} : block.substr(next);
        }

        start = closing + 1;
    }

    appendRun(read.runs, text.substr(start), current);
    return read;
}

EncodedMarkup encodeSubStationAlphaMarkup(const StyledText& runs, const StyleAbilities& abilities) {
    const WritableMarkup writable = keepWritable(runs, abilities);

    EncodedMarkup written{.text = {}, .dropped = writable.dropped};
    Style current;

    for (const StyledRun& run : writable.runs) {
        if (needsReset(current, run.style)) {
            written.text += R"({\r})";
            current = Style{};
        }

        for (const StyleAttribute attribute : kStyleAttributes) {
            if (agree(current, run.style, attribute))
                continue;
            if (run.style.carries(attribute))
                appendOn(written.text, attribute, run.style);
            else if (closesOnItsOwn(attribute))
                appendOff(written.text, attribute);
        }

        current = run.style;
        written.text += run.text;
    }

    // **What can be closed is closed, even at the very end.** Nothing would
    // read the difference — the subtitle stops there — but Gaupol writes the
    // pair, and a `{\i1}…{\i0}` that came back as `{\i1}…` would be a file
    // that changed for no reason anyone could name.
    for (const StyleAttribute attribute : kStyleAttributes) {
        if (closesOnItsOwn(attribute) && current.carries(attribute))
            appendOff(written.text, attribute);
    }

    return written;
}

} // namespace subedit::core
