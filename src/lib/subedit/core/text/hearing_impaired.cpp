// The rule of phase 4, and nothing else: a text goes in, a text comes out.
//
// **A scan, and no regular expression** — ADR 0017. The rule is local to the
// site of the removal: « exactly one space between what surrounded it ». A
// substitution pass does not know where the mention was, so it can only apply
// the global rule — « two spaces become one » — which also rewrites the ones
// the file already had. The two are different rules, not two spellings of one.
//
// The scan therefore builds its output as it goes, deciding each seam where it
// happens. The seam is decided **late**, when real text turns up after it:
// until then, nothing says whether anything will follow it on the line.

#include <subedit/core/model/subtitle_format.hpp>
#include <subedit/core/text/hearing_impaired.hpp>
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

constexpr char kDialogueDash = '-';
constexpr std::size_t kNowhere = std::string_view::npos;

[[nodiscard]] bool isBlank(char letter) {
    return letter == ' ' || letter == '\t';
}

/// Whether nothing of `text` would show on screen — blanks, and the tags of
/// `vocabulary`.
///
/// **The tags are why this exists.** Nine subtitles of the corpus are written
/// `<i>[PEOPLE SCREAMING]</i>`, and without ignoring the markup the removal
/// would leave `<i></i>`: an empty subtitle nothing would take away. ADR 0009
/// keeps the text raw, tags included, so it is the judgement that ignores them
/// and never the text that loses them.
///
/// **The tags of the format, and not `<…>` whatever the format** — issue #403.
/// A `{\i1}[SOUPIR]{\i0}` of Advanced SSA is emptied as the `<i>` of SubRip
/// is, and a `<i>` on a TMPlayer line is three characters a viewer sees. Where a
/// tag starts and ends is the one reader's to say; a `<` nothing closes on its
/// line is text, which is what leaves a damaged file alone.
[[nodiscard]] bool showsNothing(std::string_view text, MarkupVocabulary vocabulary) {
    return std::ranges::all_of(piecesOf(text, vocabulary), [](const MarkupPiece& piece) {
        return piece.kind != MarkupPiece::Kind::Text ||
               std::ranges::all_of(piece.text,
                                   [](char letter) { return isBlank(letter) || letter == '\n'; });
    });
}

/// The vocabulary to read a text with when it starts in the middle of a line.
///
/// **MPL2 without its markers**: a marker is one only at the head of a line,
/// and the rest of a line, or a line with its dash off, does not start there.
[[nodiscard]] MarkupVocabulary midLine(MarkupVocabulary vocabulary) {
    return vocabulary == MarkupVocabulary::Mpl2 ? MarkupVocabulary::MicroDvd : vocabulary;
}

/// Whether the rest of the line from `at` shows nothing.
[[nodiscard]] bool
restOfLineShowsNothing(std::string_view text, std::size_t at, MarkupVocabulary vocabulary) {
    const std::size_t lineEnd = text.find('\n', at);
    return showsNothing(text.substr(at, lineEnd == kNowhere ? kNowhere : lineEnd - at),
                        midLine(vocabulary));
}

/// Whether what sits between the delimiters is a reference rather than a sound.
///
/// **Purely numeric**: digits only, blanks tolerated on either side, none
/// between them, no sign and no letter. The criterion is deliberately narrow —
/// it does not try to recognise a reference as a human would, only to refrain
/// from eating what no mention ever looks like. `[1]` has never named a noise.
[[nodiscard]] bool isReference(std::string_view content) {
    std::size_t index = 0;
    while (index < content.size() && isBlank(content[index]))
        ++index;

    const std::size_t firstDigit = index;
    while (index < content.size() && content[index] >= '0' && content[index] <= '9')
        ++index;
    if (index == firstDigit)
        return false;

    while (index < content.size() && isBlank(content[index]))
        ++index;
    return index == content.size();
}

/// Returns the index just past a mention opening at `at`, or `kNowhere`.
///
/// `kNowhere` covers the three ways nothing is removed: `at` opens nothing, the
/// delimiter is never closed, or what it holds is a reference. An unclosed
/// delimiter is not a decision — it is what a scan does when it does not find
/// its end, and no real file has ever produced one.
[[nodiscard]] std::size_t mentionEnd(std::string_view text, std::size_t at) {
    const char opening = text[at];
    if (opening != '[' && opening != '(')
        return kNowhere;

    const std::size_t closing = text.find(opening == '[' ? ']' : ')', at + 1);
    if (closing == kNowhere)
        return kNowhere;
    if (isReference(text.substr(at + 1, closing - at - 1)))
        return kNowhere;
    return closing + 1;
}

void trimTrailingBlanks(std::string& text) {
    while (!text.empty() && isBlank(text.back()))
        text.pop_back();
}

/// One line of the text being rebuilt, and whether a mention left it.
///
/// The flag is what keeps the rule local: a line already blank in the file is
/// left alone, only one the removal emptied disappears.
struct Line {
    std::string text;
    bool touched = false;
};

/// Whether the line opens a voice of a dialogue.
///
/// **The dash is the first character, with nothing before it.** Of the 97 059
/// text lines of the corpus, 26 begin with a blank and not one of them puts it
/// before a dash. Tolerating the blank would be writing code for a case no file
/// produces, which is the rule this project holds itself to elsewhere.
[[nodiscard]] bool opensWithDash(std::string_view text) {
    return text.starts_with(kDialogueDash);
}

/// Whether the line only carried a dialogue dash, blanks and tags.
[[nodiscard]] bool showsNothingButItsDash(std::string_view text, MarkupVocabulary vocabulary) {
    return opensWithDash(text) ? showsNothing(text.substr(1), midLine(vocabulary))
                               : showsNothing(text, vocabulary);
}

/// Takes the dialogue dash off a line, with the blanks that followed it.
void takeDashOff(std::string& text) {
    std::size_t index = 1;
    while (index < text.size() && isBlank(text[index]))
        ++index;
    text.erase(0, index);
}

/// Where a tag of `vocabulary` starting at an offset ends, walked in order.
///
/// **What lets the scan step over a tag whole.** Looked for letter by letter,
/// the `(` of an Advanced SSA `\pos(320,50)` opened a mention, and a subtitle
/// with none lost its position.
class TagWalk {
public:
    TagWalk(std::string_view text, MarkupVocabulary vocabulary)
        : m_pieces(piecesOf(text, vocabulary)) {}

    /// The offset past the tag starting at `at`, or `kNowhere` when none does.
    ///
    /// `at` never goes backwards between two calls.
    [[nodiscard]] std::size_t endOfTagAt(std::size_t at) {
        while (m_next < m_pieces.size() && m_pieces[m_next].at < at)
            ++m_next;
        if (m_next == m_pieces.size() || m_pieces[m_next].at != at ||
            m_pieces[m_next].kind != MarkupPiece::Kind::Tag)
            return kNowhere;
        return at + m_pieces[m_next].text.size();
    }

private:
    std::vector<MarkupPiece> m_pieces;
    std::size_t m_next = 0;
};

/// Removes every mention, and says on which lines it removed one.
[[nodiscard]] std::vector<Line> scanned(std::string_view text, MarkupVocabulary vocabulary) {
    std::vector<Line> lines{Line{}};
    bool seamPending = false;
    bool seamIsLineBreak = false;
    TagWalk tags{text, vocabulary};

    std::size_t index = 0;
    while (index < text.size()) {
        if (text[index] == '\n') {
            // The line break replaces the seam: it says on its own that what
            // followed does not join what preceded.
            seamPending = false;
            seamIsLineBreak = false;
            trimTrailingBlanks(lines.back().text);
            lines.emplace_back();
            ++index;
            continue;
        }

        if (seamPending && isBlank(text[index])) {
            ++index;
            continue;
        }

        // A tag is copied whole, and nothing inside it is a mention.
        const std::size_t tag = tags.endOfTagAt(index);
        const std::size_t mention = tag == kNowhere ? mentionEnd(text, index) : kNowhere;
        if (mention != kNowhere) {
            trimTrailingBlanks(lines.back().text);
            lines.back().touched = true;
            seamIsLineBreak = seamIsLineBreak || text.substr(index, mention - index).contains('\n');
            seamPending = true;
            index = mention;
            continue;
        }

        if (seamPending) {
            // Real text is arriving, so the seam can be decided at last. It is
            // written only between two things that show: nothing at the edge of
            // a line, where a tag counts as no more than a blank.
            const bool atEdge = showsNothing(lines.back().text, vocabulary) ||
                                restOfLineShowsNothing(text, index, vocabulary);
            if (!atEdge) {
                if (seamIsLineBreak) {
                    lines.emplace_back();
                    lines.back().touched = true;
                } else {
                    lines.back().text += ' ';
                }
            }
            seamPending = false;
            seamIsLineBreak = false;
        }

        const std::size_t next = tag == kNowhere ? index + 1 : tag;
        lines.back().text += text.substr(index, next - index);
        index = next;
    }

    trimTrailingBlanks(lines.back().text);
    return lines;
}

} // namespace

std::optional<std::string> withoutHearingImpaired(std::string_view text, SubtitleFormat format) {
    const MarkupVocabulary vocabulary = vocabularyOf(format);
    std::vector<Line> lines = scanned(text, vocabulary);

    // Nothing bit, so nothing is decided: the text goes back out as it came,
    // whatever it holds. Judging the emptiness of a text no mention touched
    // would remove subtitles that were already empty — the very thing the
    // locality of the rule forbids.
    const bool bitten = std::ranges::any_of(lines, [](const Line& line) { return line.touched; });
    if (!bitten)
        return std::string{text};

    std::size_t voicesBefore = 0;
    for (const Line& line : lines)
        if (opensWithDash(line.text))
            ++voicesBefore;

    std::vector<Line> kept;
    std::size_t dropped = 0;
    for (Line& line : lines) {
        if (line.touched && showsNothingButItsDash(line.text, vocabulary)) {
            ++dropped;
            continue;
        }
        kept.push_back(std::move(line));
    }

    // A dialogue reduced to a single voice is no longer one, and its dash goes
    // with the voice that left. As long as two remain, the dashes stay. It is a
    // rule about dashes and not about mentions — it would hold for a line
    // removed by hand — and phase 12 inherits it.
    if (dropped > 0 && voicesBefore >= 2 && kept.size() == 1 && opensWithDash(kept.front().text))
        takeDashOff(kept.front().text);

    // Joined on the rank and not on « is what we have so far empty », which
    // would swallow a first line the file itself had left blank — precisely the
    // line the locality of the rule says not to touch.
    std::string joined;
    for (std::size_t rank = 0; rank < kept.size(); ++rank) {
        if (rank > 0)
            joined += '\n';
        joined += kept[rank].text;
    }

    if (showsNothing(joined, vocabulary))
        return std::nullopt;
    return joined;
}

} // namespace subedit::core
