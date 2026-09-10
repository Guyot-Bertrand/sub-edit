#include <subedit/core/text/html_markup.hpp>
#include <subedit/core/text/markup.hpp>
#include <subedit/core/text/markup_codec.hpp>
#include <subedit/core/text/markup_vocabulary.hpp>

#include <algorithm>
#include <cstddef>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace subedit::core {

namespace {

constexpr std::string_view kFontColour = "font color=\"#";
constexpr std::size_t kColourDigits = 6;

/// The same letter, in lower case.
[[nodiscard]] char lowered(char letter) {
    return letter >= 'A' && letter <= 'Z' ? static_cast<char>(letter - 'A' + 'a') : letter;
}

/// Tells whether `text` opens with `prefix`, whatever the case.
[[nodiscard]] bool startsWithLoosely(std::string_view text, std::string_view prefix) {
    if (text.size() < prefix.size())
        return false;
    for (std::size_t index = 0; index < prefix.size(); ++index) {
        if (lowered(text[index]) != lowered(prefix[index]))
            return false;
    }
    return true;
}

/// The attribute a one-letter tag stands for, or nothing for another letter.
[[nodiscard]] std::optional<StyleAttribute> attributeOfLetter(char letter) {
    switch (lowered(letter)) {
    case 'b':
        return StyleAttribute::Bold;
    case 'i':
        return StyleAttribute::Italic;
    case 'u':
        return StyleAttribute::Underline;
    default:
        return std::nullopt;
    }
}

/// Sets `attribute` on `style`, with `colour` when that is what it takes.
///
/// **Four of the six, and the two missing are missing from the language.**
/// `readTag` never names a font or a size, because no tag of this vocabulary
/// carries either; a switch over all six would carry two arms no text can
/// reach, and an arm nothing reaches is a promise nothing verifies.
void applyOpening(Style& style, StyleAttribute attribute, const std::optional<Colour>& colour) {
    if (attribute == StyleAttribute::Colour)
        style.colour = colour;
    else if (attribute == StyleAttribute::Bold)
        style.bold = true;
    else if (attribute == StyleAttribute::Italic)
        style.italic = true;
    else
        style.underline = true;
}

/// What one tag between angle brackets says, once its brackets are off.
struct Tag {
    /// What it opens or closes, or nothing when the pivot has no room for it.
    std::optional<StyleAttribute> attribute{};
    std::optional<Colour> colour{};
    bool closing = false;
};

[[nodiscard]] Tag readTag(std::string_view inside) {
    Tag tag;
    tag.closing = inside.starts_with('/');
    const std::string_view body = tag.closing ? inside.substr(1) : inside;

    if (body.size() == 1) {
        tag.attribute = attributeOfLetter(body.front());
        return tag;
    }

    if (tag.closing) {
        if (startsWithLoosely(body, "font"))
            tag.attribute = StyleAttribute::Colour;
        return tag;
    }

    if (startsWithLoosely(body, kFontColour)) {
        const std::string_view rest = body.substr(kFontColour.size());
        if (rest.size() == kColourDigits + 1 && rest.back() == '"') {
            if (const std::optional<Colour> colour = Colour::parse(rest.substr(0, kColourDigits))) {
                tag.attribute = StyleAttribute::Colour;
                tag.colour = colour;
            }
        }
    }
    return tag;
}

/// Appends the tag that opens `attribute`, reading its value from `style`.
void appendOpening(std::string& out, StyleAttribute attribute, const Style& style) {
    switch (attribute) {
    case StyleAttribute::Bold:
        out += "<b>";
        break;
    case StyleAttribute::Italic:
        out += "<i>";
        break;
    case StyleAttribute::Underline:
        out += "<u>";
        break;
    case StyleAttribute::Colour:
        // **The check is what reads the value, not a guard against it.** The
        // caller only asks for a tag the style carries, and nothing in the type
        // says so; asking here costs a branch nobody takes and makes the
        // function total rather than conditional on its caller's manners.
        if (style.colour.has_value()) {
            out += "<font color=\"#";
            out += style.colour->toString();
            out += "\">";
        }
        break;
    case StyleAttribute::Font:
    case StyleAttribute::Size:
        break;
    }
}

/// Appends the tag that closes `attribute`.
void appendClosing(std::string& out, StyleAttribute attribute) {
    switch (attribute) {
    case StyleAttribute::Bold:
        out += "</b>";
        break;
    case StyleAttribute::Italic:
        out += "</i>";
        break;
    case StyleAttribute::Underline:
        out += "</u>";
        break;
    case StyleAttribute::Colour:
        out += "</font>";
        break;
    case StyleAttribute::Font:
    case StyleAttribute::Size:
        break;
    }
}

/// Tells whether two styles say the same thing about `attribute`.
[[nodiscard]] bool agree(const Style& left, const Style& right, StyleAttribute attribute) {
    if (attribute == StyleAttribute::Colour)
        return left.colour == right.colour;
    return left.carries(attribute) == right.carries(attribute);
}

} // namespace

DecodedMarkup decodeHtmlMarkup(std::string_view text) {
    DecodedMarkup read;
    Style current;
    std::size_t start = 0;

    while (start < text.size()) {
        const std::size_t opening = text.find('<', start);
        if (opening == std::string_view::npos)
            break;
        const std::size_t closing = text.find('>', opening);
        if (closing == std::string_view::npos)
            break;

        appendRun(read.runs, text.substr(start, opening - start), current);

        const Tag tag = readTag(text.substr(opening + 1, closing - opening - 1));
        if (!tag.attribute.has_value()) {
            // **A closing tag of an unknown pair is not counted a second time.**
            // What was lost is the thing `<c.loud>` said, and it was said once.
            if (!tag.closing)
                ++read.unknown;
        } else if (tag.closing) {
            current = current.without(*tag.attribute);
        } else {
            applyOpening(current, *tag.attribute, tag.colour);
        }

        start = closing + 1;
    }

    appendRun(read.runs, text.substr(start), current);
    return read;
}

EncodedMarkup encodeHtmlMarkup(const StyledText& runs, const StyleAbilities& abilities) {
    const WritableMarkup writable = keepWritable(runs, abilities);

    EncodedMarkup written{.text = {}, .dropped = writable.dropped};
    std::vector<StyleAttribute> openTags;
    Style current;

    for (const StyledRun& run : writable.runs) {
        // **The tags still wanted, in the order they were opened.** Closing
        // stops at the first one that has to go, since a tag cannot be closed
        // before the ones nested inside it.
        std::size_t kept = 0;
        while (kept < openTags.size() && run.style.carries(openTags[kept]) &&
               agree(current, run.style, openTags[kept]))
            ++kept;

        for (std::size_t index = openTags.size(); index > kept; --index)
            appendClosing(written.text, openTags[index - 1]);
        openTags.resize(kept);

        for (const StyleAttribute attribute : kStyleAttributes) {
            if (!run.style.carries(attribute))
                continue;
            if (std::ranges::find(openTags, attribute) != openTags.end())
                continue;
            appendOpening(written.text, attribute, run.style);
            openTags.push_back(attribute);
        }

        current = run.style;
        written.text += run.text;
    }

    for (std::size_t index = openTags.size(); index > 0; --index)
        appendClosing(written.text, openTags[index - 1]);

    return written;
}

} // namespace subedit::core
