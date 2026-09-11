#include <subedit/core/text/italics.hpp>
#include <subedit/core/text/markup.hpp>
#include <subedit/core/text/markup_codec.hpp>
#include <subedit/core/text/markup_vocabulary.hpp>

#include <algorithm>
#include <cstddef>
#include <string>
#include <string_view>
#include <utility>

namespace subedit::core {

namespace {

/// The three characters MPL2 writes at the head of a line, italics among them.
constexpr std::string_view kMpl2Markers = "\\/_";

/// The same letter, in lower case.
[[nodiscard]] char lowered(char letter) {
    return letter >= 'A' && letter <= 'Z' ? static_cast<char>(letter - 'A' + 'a') : letter;
}

/// Tells whether `text` carries `literal` at `at`, whatever the case.
///
/// `literal` is written in lower case, being the side that is not compared.
[[nodiscard]] bool matchesAt(std::string_view text, std::size_t at, std::string_view literal) {
    if (text.size() - at < literal.size())
        return false;
    for (std::size_t index = 0; index < literal.size(); ++index) {
        if (lowered(text[at + index]) != literal[index])
            return false;
    }
    return true;
}

/// The text without its `<i>` and `</i>`, in either case.
///
/// **Either case, where Gaupol matches the lower one only.** Our own reader
/// lowers what it reads, so a `<I>` left behind would be a text still in italics
/// after the italics came out — the defect this function exists to avoid.
[[nodiscard]] std::string htmlWithoutItalics(std::string_view text) {
    std::string out;
    out.reserve(text.size());

    std::size_t at = 0;
    while (at < text.size()) {
        if (matchesAt(text, at, "<i>")) {
            at += 3;
            continue;
        }
        if (matchesAt(text, at, "</i>")) {
            at += 4;
            continue;
        }
        out += text[at];
        ++at;
    }
    return out;
}

/// Tells whether one Sub Station Alpha override, backslash off, is the italic.
///
/// `i` followed by a number, which is what the reader accepts: Advanced SSA
/// writes a weight where Sub Station Alpha writes a flag, and both turn the
/// same thing on.
[[nodiscard]] bool isItalicOverride(std::string_view override) {
    if (override.size() < 2 || override.front() != 'i')
        return false;
    return std::ranges::all_of(override.substr(1),
                               [](char digit) { return digit >= '0' && digit <= '9'; });
}

/// The text without the italic overrides of its brace blocks.
[[nodiscard]] std::string ssaWithoutItalics(std::string_view text) {
    std::string out;
    out.reserve(text.size());

    std::size_t start = 0;
    while (start < text.size()) {
        const std::size_t opening = text.find('{', start);
        if (opening == std::string_view::npos)
            break;
        const std::size_t closing = text.find('}', opening);
        if (closing == std::string_view::npos)
            break;

        out += text.substr(start, opening - start);

        std::string_view block = text.substr(opening + 1, closing - opening - 1);
        std::string kept;
        while (!block.empty()) {
            if (block.front() != '\\') {
                // Not an override at all — `{some note}` is a comment in the
                // wild, and a comment is not ours to rewrite.
                kept += block;
                break;
            }
            block.remove_prefix(1);
            const std::size_t next = block.find('\\');
            const std::string_view override = block.substr(0, next);
            if (!isItalicOverride(override)) {
                kept += '\\';
                kept += override;
            }
            block = next == std::string_view::npos ? std::string_view{} : block.substr(next);
        }

        // A block that said nothing but italics goes with them. Leaving `{}`
        // behind would be a tag the reader counts as one it cannot make sense
        // of, which is exactly the noise this avoids.
        if (!kept.empty()) {
            out += '{';
            out += kept;
            out += '}';
        }

        start = closing + 1;
    }

    out += text.substr(start);
    return out;
}

/// The text without the `i` of its `{Y:…}` tags, and without a tag left empty.
[[nodiscard]] std::string bracedWithoutItalics(std::string_view text) {
    std::string out;
    out.reserve(text.size());

    std::size_t start = 0;
    while (start < text.size()) {
        const std::size_t opening = text.find('{', start);
        if (opening == std::string_view::npos)
            break;
        const std::size_t closing = text.find('}', opening);
        if (closing == std::string_view::npos)
            break;

        out += text.substr(start, opening - start);

        const std::string_view inside = text.substr(opening + 1, closing - opening - 1);
        const bool style = inside.size() > 2 && lowered(inside.front()) == 'y' && inside[1] == ':';
        if (!style) {
            out += text.substr(opening, closing - opening + 1);
        } else {
            std::string kept;
            for (const char letter : inside.substr(2)) {
                if (letter != 'i')
                    kept += letter;
            }
            if (!kept.empty()) {
                out += '{';
                out += inside.substr(0, 2);
                out += kept;
                out += '}';
            }
        }

        start = closing + 1;
    }

    out += text.substr(start);
    return out;
}

/// Walks the lines of `text`, handing each to `visit` with its ending.
template<typename Visit>
void overLines(std::string_view text, Visit visit) {
    std::size_t start = 0;
    while (true) {
        const std::size_t ending = text.find('\n', start);
        visit(ending == std::string_view::npos ? text.substr(start)
                                               : text.substr(start, ending - start),
              ending != std::string_view::npos);
        if (ending == std::string_view::npos)
            return;
        start = ending + 1;
    }
}

/// The text without the `/` of its leading markers, line by line.
[[nodiscard]] std::string withoutMpl2Markers(std::string_view text) {
    std::string out;
    out.reserve(text.size());

    overLines(text, [&out](std::string_view line, bool more) {
        const std::size_t body = line.find_first_not_of(kMpl2Markers);
        const std::string_view markers =
            body == std::string_view::npos ? line : line.substr(0, body);
        for (const char marker : markers) {
            if (marker != '/')
                out += marker;
        }
        if (body != std::string_view::npos)
            out += line.substr(body);
        if (more)
            out += '\n';
    });

    return out;
}

/// The text with MPL2's italic marker at the head of every one of its lines.
[[nodiscard]] std::string withMpl2Marker(std::string_view text) {
    std::string out;
    out.reserve(text.size() + 2);

    overLines(text, [&out](std::string_view line, bool more) {
        out += '/';
        out += line;
        if (more)
            out += '\n';
    });

    return out;
}

} // namespace

bool opensInItalics(std::string_view text, SubtitleFormat format) {
    // Read through the reader of the vocabulary, which is what already knows
    // that `{Y:i}` reaches the end of the subtitle and that `/` reaches the end
    // of its line. Reading is safe where rewriting would not be: what the pivot
    // drops, it drops on the way out, and nothing goes out here.
    const DecodedMarkup read = decodeAs(text, vocabularyOf(format));
    for (const StyledRun& run : read.runs) {
        if (!run.text.empty())
            return run.style.italic;
    }
    return false;
}

std::string withoutItalics(std::string_view text, SubtitleFormat format) {
    switch (vocabularyOf(format)) {
    case MarkupVocabulary::None:
        return std::string{text};
    case MarkupVocabulary::Html:
        return htmlWithoutItalics(text);
    case MarkupVocabulary::SubStationAlpha:
        return ssaWithoutItalics(text);
    case MarkupVocabulary::MicroDvd:
        return bracedWithoutItalics(text);
    case MarkupVocabulary::Mpl2:
        // Two ways of saying the same thing, and a text may carry both: MPL2
        // is MicroDVD plus a marker at the head of a line.
        return bracedWithoutItalics(withoutMpl2Markers(text));
    }
    std::unreachable();
}

std::string inItalics(std::string_view text, SubtitleFormat format) {
    std::string plain = withoutItalics(text, format);
    if (plain.empty())
        return plain;

    switch (vocabularyOf(format)) {
    case MarkupVocabulary::None:
        return plain;
    case MarkupVocabulary::Html:
        return "<i>" + plain + "</i>";
    case MarkupVocabulary::SubStationAlpha:
        return R"({\i1})" + plain + R"({\i0})";
    // **Opened and never closed**, which is the format's own way: a capital tag
    // reaches the end of the subtitle, so there is nothing to shut.
    case MarkupVocabulary::MicroDvd:
        return "{Y:i}" + plain;
    case MarkupVocabulary::Mpl2:
        return withMpl2Marker(plain);
    }
    std::unreachable();
}

} // namespace subedit::core
