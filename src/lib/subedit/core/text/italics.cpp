#include <subedit/core/text/italics.hpp>
#include <subedit/core/text/markup.hpp>
#include <subedit/core/text/markup_codec.hpp>
#include <subedit/core/text/markup_reader.hpp>
#include <subedit/core/text/markup_vocabulary.hpp>

#include <cstddef>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace subedit::core {

namespace {

/// The text without its `<i>` and `</i>`, whatever their case or spacing.
///
/// **Any tag the reader names `i`**, where Gaupol matches `<i>` letter for
/// letter. `<I>` and `<i >` are italic tags the rest of the core reads as such,
/// so one left behind would be a text still in italics after the italics came
/// out — the defect this function exists to avoid, and issue #403's.
[[nodiscard]] std::string htmlWithoutItalics(std::string_view text) {
    std::string out;
    out.reserve(text.size());
    for (const MarkupPiece& piece : piecesOf(text, MarkupVocabulary::Html)) {
        if (piece.kind == MarkupPiece::Kind::Tag && htmlTagOf(piece.text).name == "i")
            continue;
        out += piece.text;
    }
    return out;
}

/// The block `tag` without its italic overrides.
///
/// **A block that said nothing but italics goes with them**: leaving `{}`
/// behind would be a tag the reader counts as one it cannot make sense of,
/// which is exactly the noise this avoids. A block with no italic in it is not
/// the toggle's to touch, and comes back as it was — a comment, or an empty
/// pair the file already held.
[[nodiscard]] std::string blockWithoutItalics(std::string_view tag) {
    const std::optional<std::vector<std::string_view>> overrides = overridesOf(tag);
    if (!overrides.has_value())
        return std::string{tag};

    std::string kept;
    bool italic = false;
    for (const std::string_view override : *overrides) {
        const std::optional<FlagOverride> flag = flagOverrideOf(override);
        if (flag.has_value() && flag->letter == 'i') {
            italic = true;
            continue;
        }
        kept += '\\';
        kept += override;
    }

    if (!italic)
        return std::string{tag};
    return kept.empty() ? std::string{} : "{" + kept + "}";
}

/// The text without the italic overrides of its brace blocks.
[[nodiscard]] std::string ssaWithoutItalics(std::string_view text) {
    std::string out;
    out.reserve(text.size());
    for (const MarkupPiece& piece : piecesOf(text, MarkupVocabulary::SubStationAlpha)) {
        if (piece.kind == MarkupPiece::Kind::Tag)
            out += blockWithoutItalics(piece.text);
        else
            out += piece.text;
    }
    return out;
}

/// The tag `tag` without the `i` of its style letters, gone when that was all.
[[nodiscard]] std::string scopedWithoutItalics(std::string_view tag) {
    const std::optional<ScopedTag> scoped = scopedTagOf(tag);
    if (!scoped.has_value() || scoped->letter != 'y' || !scoped->value.contains('i'))
        return std::string{tag};

    std::string kept;
    for (const char letter : scoped->value) {
        if (letter != 'i')
            kept += letter;
    }
    // `{Y:` or `{y:` — the opening three bytes, which say the scope.
    constexpr std::size_t kScopeLength = 3;
    return kept.empty() ? std::string{} : std::string{tag.substr(0, kScopeLength)} + kept + "}";
}

/// The text without the `i` of its `{Y:…}` tags and, in MPL2, without its `/`.
///
/// **One pass for the two ways MPL2 says italics**: it is MicroDVD plus a
/// marker at the head of a line, and a text may carry both.
[[nodiscard]] std::string bracedWithoutItalics(std::string_view text, MarkupVocabulary vocabulary) {
    std::string out;
    out.reserve(text.size());
    for (const MarkupPiece& piece : piecesOf(text, vocabulary)) {
        switch (piece.kind) {
        case MarkupPiece::Kind::Marker:
            if (piece.text != "/")
                out += piece.text;
            break;
        case MarkupPiece::Kind::Tag:
            out += scopedWithoutItalics(piece.text);
            break;
        case MarkupPiece::Kind::Text:
            out += piece.text;
            break;
        }
    }
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
    case MarkupVocabulary::Mpl2:
        return bracedWithoutItalics(text, vocabularyOf(format));
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
