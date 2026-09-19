#include <subedit/core/text/markup_reader.hpp>
#include <subedit/core/text/markup_vocabulary.hpp>

#include <algorithm>
#include <cstddef>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace subedit::core {

namespace {

/// The three characters MPL2 writes at the head of a line.
constexpr std::string_view kMpl2Markers = "\\/_";

/// The four letters a MicroDVD tag may open with, in both cases.
constexpr std::string_view kScopedLetters = "cfsyCFSY";

[[nodiscard]] char lowered(char letter) {
    return letter >= 'A' && letter <= 'Z' ? static_cast<char>(letter - 'A' + 'a') : letter;
}

/// What sits between a tag's two delimiters.
[[nodiscard]] std::string_view insideOf(std::string_view tag) {
    return tag.substr(1, tag.size() - 2);
}

/// Appends `piece`, joining it to the text before it when both are text.
void append(std::vector<MarkupPiece>& pieces, const MarkupPiece& piece, std::string_view text) {
    if (piece.kind == MarkupPiece::Kind::Text && !pieces.empty() &&
        pieces.back().kind == MarkupPiece::Kind::Text) {
        MarkupPiece& before = pieces.back();
        before.text = text.substr(before.at, before.text.size() + piece.text.size());
        return;
    }
    pieces.push_back(piece);
}

/// The character that opens a tag in `vocabulary`, when it has one.
[[nodiscard]] std::optional<char> openerOf(MarkupVocabulary vocabulary) {
    switch (vocabulary) {
    case MarkupVocabulary::None:
        return std::nullopt;
    case MarkupVocabulary::Html:
        return '<';
    case MarkupVocabulary::SubStationAlpha:
    case MarkupVocabulary::MicroDvd:
    case MarkupVocabulary::Mpl2:
        return '{';
    }
    std::unreachable();
}

/// Where the run of text that includes `text[at]` ends: at the next character
/// that could start something else.
///
/// A subtitle is mostly text, and this is the loop the whole core reads its tags
/// through, so a run is appended as one piece rather than a character at a time.
/// In MPL2 a marker follows a line break, so a break ends the run there; nothing
/// else can follow a character of text.
[[nodiscard]] std::size_t
endOfTextRun(std::string_view text, std::size_t at, MarkupVocabulary vocabulary) {
    const std::size_t from = at + 1;
    std::size_t end = std::string_view::npos;
    if (vocabulary == MarkupVocabulary::Mpl2) {
        if (text[at] == '\n')
            return from;
        end = text.find_first_of("{\n", from);
    } else if (const std::optional<char> opener = openerOf(vocabulary)) {
        end = text.find(*opener, from);
    }
    return end == std::string_view::npos ? text.size() : end;
}

} // namespace

std::vector<MarkupPiece> piecesOf(std::string_view text, MarkupVocabulary vocabulary) {
    const bool angles = vocabulary == MarkupVocabulary::Html;
    const bool braces = vocabulary == MarkupVocabulary::SubStationAlpha ||
                        vocabulary == MarkupVocabulary::MicroDvd ||
                        vocabulary == MarkupVocabulary::Mpl2;
    const bool markers = vocabulary == MarkupVocabulary::Mpl2;
    const char opening = angles ? '<' : '{';
    const char closing = angles ? '>' : '}';
    // What ends a search for the closer: the closer, the end of the line, or a
    // second opener — before which the first one opened nothing.
    const std::string_view stops = angles ? "<>\n" : "{}\n";

    std::vector<MarkupPiece> pieces;
    bool atLineHead = true;
    for (std::size_t at = 0; at < text.size();) {
        if (markers && atLineHead && kMpl2Markers.contains(text[at])) {
            append(pieces,
                   MarkupPiece{
                       .kind = MarkupPiece::Kind::Marker, .text = text.substr(at, 1), .at = at},
                   text);
            ++at;
            continue;
        }
        atLineHead = false;

        if ((angles || braces) && text[at] == opening) {
            const std::size_t ending = text.find_first_of(stops, at + 1);
            if (ending != std::string_view::npos && text[ending] == closing) {
                append(pieces,
                       MarkupPiece{.kind = MarkupPiece::Kind::Tag,
                                   .text = text.substr(at, ending - at + 1),
                                   .at = at},
                       text);
                at = ending + 1;
                continue;
            }
        }

        atLineHead = text[at] == '\n';

        const std::size_t end = endOfTextRun(text, at, vocabulary);
        append(pieces,
               MarkupPiece{
                   .kind = MarkupPiece::Kind::Text, .text = text.substr(at, end - at), .at = at},
               text);
        at = end;
    }
    return pieces;
}

bool mayHoldMarkup(std::string_view text, MarkupVocabulary vocabulary) {
    if (vocabulary == MarkupVocabulary::Mpl2)
        return true;
    const std::optional<char> opener = openerOf(vocabulary);
    return opener.has_value() && text.contains(*opener);
}

HtmlTag htmlTagOf(std::string_view tag) {
    HtmlTag read;
    std::string_view body = insideOf(tag);
    read.closing = body.starts_with('/');
    if (read.closing)
        body.remove_prefix(1);

    // **A self-closing tag names nothing**: `<i/>` opens no italic and shuts
    // none, and read as `<i>` it put the rest of the subtitle in italics.
    if (body.ends_with('/'))
        return read;

    const auto separates = [](char letter) { return letter == ' ' || letter == '\t'; };
    std::size_t end = 0;
    while (end < body.size() && !separates(body[end])) {
        read.name += lowered(body[end]);
        ++end;
    }
    std::string_view rest = body.substr(end);
    while (!rest.empty() && separates(rest.front()))
        rest.remove_prefix(1);
    read.attributes = rest;
    return read;
}

std::optional<std::vector<std::string_view>> overridesOf(std::string_view tag) {
    std::string_view block = insideOf(tag);
    std::vector<std::string_view> overrides;
    while (!block.empty()) {
        if (block.front() != '\\')
            return std::nullopt;
        block.remove_prefix(1);
        const std::size_t next = block.find('\\');
        overrides.push_back(block.substr(0, next));
        block = next == std::string_view::npos ? std::string_view{} : block.substr(next);
    }
    return overrides;
}

std::optional<FlagOverride> flagOverrideOf(std::string_view override) {
    if (override.size() < 2)
        return std::nullopt;
    const char letter = override.front();
    if (letter != 'b' && letter != 'i' && letter != 'u')
        return std::nullopt;

    const std::string_view number = override.substr(1);
    if (!std::ranges::all_of(number, [](char digit) { return digit >= '0' && digit <= '9'; }))
        return std::nullopt;
    const bool on = std::ranges::any_of(number, [](char digit) { return digit != '0'; });
    return FlagOverride{.letter = letter, .on = on};
}

std::optional<ScopedTag> scopedTagOf(std::string_view tag) {
    const std::string_view inside = insideOf(tag);
    if (inside.size() < 2 || inside[1] != ':' || !kScopedLetters.contains(inside.front()))
        return std::nullopt;
    return ScopedTag{.letter = lowered(inside.front()),
                     .wholeSubtitle = inside.front() >= 'A' && inside.front() <= 'Z',
                     .value = inside.substr(2)};
}

} // namespace subedit::core
