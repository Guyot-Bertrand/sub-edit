#include <subedit/core/text/markup.hpp>
#include <subedit/core/text/markup_codec.hpp>
#include <subedit/core/text/markup_reader.hpp>
#include <subedit/core/text/markup_vocabulary.hpp>
#include <subedit/core/text/micro_dvd_markup.hpp>

#include <algorithm>
#include <cstddef>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace subedit::core {

namespace {

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

/// What the line scope says, over what the subtitle scope said.
///
/// A capital tag is the ground and a lower-case one is written on top of it:
/// `{Y:i}` then `{y:b}` gives a bold italic line, and the italic survives the
/// line ending where the bold does not.
[[nodiscard]] Style layered(const Style& subtitle, const Style& line) {
    Style effective = subtitle;
    effective.bold = effective.bold || line.bold;
    effective.italic = effective.italic || line.italic;
    effective.underline = effective.underline || line.underline;
    if (line.colour.has_value())
        effective.colour = line.colour;
    if (line.font.has_value())
        effective.font = line.font;
    if (line.size.has_value())
        effective.size = line.size;
    return effective;
}

/// Applies a MicroDVD tag to `style`, or says it meant nothing here.
[[nodiscard]] bool applyTag(Style& style, const ScopedTag& tag) {
    const std::string_view value = tag.value;

    if (tag.letter == 'y') {
        const bool bold = value.contains('b');
        const bool italic = value.contains('i');
        const bool underline = value.contains('u');
        if (!bold && !italic && !underline)
            return false;
        style.bold = style.bold || bold;
        style.italic = style.italic || italic;
        style.underline = style.underline || underline;
        return true;
    }

    if (tag.letter == 'c') {
        if (!value.starts_with('$'))
            return false;
        const std::optional<Colour> colour = Colour::parseReversed(value.substr(1));
        if (!colour.has_value())
            return false;
        style.colour = colour;
        return true;
    }

    if (tag.letter == 'f') {
        if (value.empty())
            return false;
        style.font = std::string{value};
        return true;
    }

    const std::optional<int> points = wholeNumber(value);
    if (!points.has_value())
        return false;
    style.size = points;
    return true;
}

/// Appends `text` under the style its two scopes make, closing the line scope
/// at every line ending.
void appendLines(std::string_view text,
                 Style& lineScope,
                 const Style& subtitleScope,
                 DecodedMarkup& read) {
    while (!text.empty()) {
        const std::size_t ending = text.find('\n');
        if (ending == std::string_view::npos) {
            appendRun(read.runs, text, layered(subtitleScope, lineScope));
            return;
        }
        // The line ending belongs to the line it ends, and so does its style.
        appendRun(read.runs, text.substr(0, ending + 1), layered(subtitleScope, lineScope));
        lineScope = Style{};
        text.remove_prefix(ending + 1);
    }
}

/// Reads the pieces of `text`, the one reader of tags having cut them.
///
/// Both scopes are written to: which one a tag lands in is decided by its case,
/// and the subtitle scope is what survives the line ending.
[[nodiscard]] DecodedMarkup decodeBraced(std::string_view text, MarkupVocabulary vocabulary) {
    DecodedMarkup read;
    Style subtitleScope;
    Style lineScope;

    for (const MarkupPiece& piece : piecesOf(text, vocabulary)) {
        switch (piece.kind) {
        case MarkupPiece::Kind::Marker:
            // MPL2's markers, read as a set: `\/` is bold and italic.
            lineScope.bold = lineScope.bold || piece.text == "\\";
            lineScope.italic = lineScope.italic || piece.text == "/";
            lineScope.underline = lineScope.underline || piece.text == "_";
            break;
        case MarkupPiece::Kind::Tag: {
            // **The case decides where the tag goes**, and nothing else does: the
            // same four letters, capitalised, reach the end of the subtitle.
            const std::optional<ScopedTag> tag = scopedTagOf(piece.text);
            if (!tag.has_value() || !applyTag(tag->wholeSubtitle ? subtitleScope : lineScope, *tag))
                ++read.unknown;
            break;
        }
        case MarkupPiece::Kind::Text:
            appendLines(piece.text, lineScope, subtitleScope, read);
            break;
        }
    }

    return read;
}

/// One line of an encoding: its text, and the one style it can carry.
struct EncodedLine {
    std::string text{};
    Style style{};
};

/// The lines of a styled text, and what cutting it into them cost.
struct EncodedLines {
    std::vector<EncodedLine> lines{};
    std::size_t dropped = 0;
};

/// Cuts `runs` into lines, keeping a style only where it covers a whole one.
[[nodiscard]] EncodedLines intoLines(const StyledText& runs) {
    std::vector<std::vector<StyledRun>> gathered(1);
    for (const StyledRun& run : runs) {
        const std::string_view whole = run.text;
        std::size_t start = 0;
        while (true) {
            const std::size_t ending = whole.find('\n', start);
            const std::string_view piece = ending == std::string_view::npos
                                               ? whole.substr(start)
                                               : whole.substr(start, ending - start);
            if (!piece.empty())
                gathered.back().push_back(
                    StyledRun{.text = std::string{piece}, .style = run.style});
            if (ending == std::string_view::npos)
                break;
            gathered.emplace_back();
            start = ending + 1;
        }
    }

    EncodedLines cut;
    cut.lines.reserve(gathered.size());
    for (const std::vector<StyledRun>& pieces : gathered) {
        EncodedLine line;
        for (const StyledRun& run : pieces)
            line.text += run.text;

        const bool uniform =
            !pieces.empty() && std::ranges::all_of(pieces, [&pieces](const StyledRun& run) {
                return run.style == pieces.front().style;
            });

        if (uniform) {
            line.style = pieces.front().style;
        } else {
            // **A line whose runs disagree is written plain**, and everything
            // any of them said is lost. There is no tag here that stops.
            for (const StyledRun& run : pieces)
                cut.dropped += run.style.count();
        }

        cut.lines.push_back(std::move(line));
    }
    return cut;
}

/// Appends the braced tags of `style`, in the case `whole` asks for.
///
/// `letters` says whether bold, italic and underline are written here. MPL2
/// writes those as head-of-line markers instead, and leaves the braces to the
/// three attributes it has no marker for.
void appendBraces(std::string& out, const Style& style, bool whole, bool letters) {
    if (letters) {
        const char scope = whole ? 'Y' : 'y';
        std::string flags;
        if (style.bold)
            flags += 'b';
        if (style.italic)
            flags += 'i';
        if (style.underline)
            flags += 'u';
        for (const char flag : flags) {
            out += '{';
            out += scope;
            out += ':';
            out += flag;
            out += '}';
        }
    }

    if (style.colour.has_value()) {
        out += '{';
        out += whole ? 'C' : 'c';
        out += ":$";
        out += style.colour->toReversedString();
        out += '}';
    }
    if (style.font.has_value()) {
        out += '{';
        out += whole ? 'F' : 'f';
        out += ':';
        out += *style.font;
        out += '}';
    }
    if (style.size.has_value()) {
        out += '{';
        out += whole ? 'S' : 's';
        out += ':';
        out += std::to_string(*style.size);
        out += '}';
    }
}

/// Appends the head-of-line markers MPL2 writes for bold, italic and underline.
void appendMarkers(std::string& out, const Style& style) {
    if (style.bold)
        out += '\\';
    if (style.italic)
        out += '/';
    if (style.underline)
        out += '_';
}

/// Tells whether every line says the same thing, and says something.
[[nodiscard]] bool sharedByAll(const std::vector<EncodedLine>& lines) {
    return !lines.empty() && !lines.front().style.isPlain() &&
           std::ranges::all_of(lines, [&lines](const EncodedLine& line) {
               return line.style == lines.front().style;
           });
}

/// Whether a line's braces are still owed, once the shared tag has been written.
[[nodiscard]] bool bracesOwed(const Style& style, bool letters) {
    return letters ? !style.isPlain()
                   : style.colour.has_value() || style.font.has_value() || style.size.has_value();
}

[[nodiscard]] EncodedMarkup
encodeBraced(const StyledText& runs, const StyleAbilities& abilities, bool markers) {
    const WritableMarkup writable = keepWritable(runs, abilities);
    EncodedLines cut = intoLines(writable.runs);

    EncodedMarkup written{.text = {}, .dropped = writable.dropped + cut.dropped};

    // **One capital tag beats a lower-case one on every line**, and it is what
    // a real file carries: the MicroDVD rendering of the scene opens on `{Y:i}`
    // and says nothing more. MPL2 has no capital form for its markers and
    // repeats them, but its braces follow the same rule.
    const bool shared = sharedByAll(cut.lines);
    const bool braceLetters = !markers;

    for (std::size_t index = 0; index < cut.lines.size(); ++index) {
        if (index > 0)
            written.text += '\n';

        const EncodedLine& line = cut.lines[index];
        // **The markers come first, and that is not a style choice.** MPL2
        // recognises them at the head of a line and nowhere else, so a brace
        // written before them would take their place and they would be text.
        if (markers)
            appendMarkers(written.text, line.style);

        const bool owed = bracesOwed(line.style, braceLetters);
        if (shared && index == 0 && owed)
            appendBraces(written.text, line.style, true, braceLetters);
        else if (!shared && owed)
            appendBraces(written.text, line.style, false, braceLetters);

        written.text += line.text;
    }

    return written;
}

} // namespace

DecodedMarkup decodeMicroDvdMarkup(std::string_view text) {
    return decodeBraced(text, MarkupVocabulary::MicroDvd);
}

EncodedMarkup encodeMicroDvdMarkup(const StyledText& runs, const StyleAbilities& abilities) {
    return encodeBraced(runs, abilities, false);
}

DecodedMarkup decodeMpl2Markup(std::string_view text) {
    return decodeBraced(text, MarkupVocabulary::Mpl2);
}

EncodedMarkup encodeMpl2Markup(const StyledText& runs, const StyleAbilities& abilities) {
    return encodeBraced(runs, abilities, true);
}

} // namespace subedit::core
