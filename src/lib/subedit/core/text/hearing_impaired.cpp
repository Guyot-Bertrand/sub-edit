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

#include <subedit/core/text/hearing_impaired.hpp>

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

/// Returns the index just past the format tag opening at `at`, or `kNowhere`.
///
/// A tag is `<` up to the first `>` of the same line. Nothing more is needed:
/// what this has to answer is « does anything show here », not « what does this
/// markup mean ».
///
/// A `<` that nothing closes is not a tag, and is therefore text like any
/// other. No file of the corpus holds one — of 8 347 lines opening with `<`,
/// every one closes — but a scan has to answer the question all the same, and
/// answering it « it is text » is what leaves a damaged file alone.
[[nodiscard]] std::size_t tagEnd(std::string_view text, std::size_t at) {
    if (at >= text.size() || text[at] != '<')
        return kNowhere;

    const std::size_t closing = text.find_first_of(">\n", at + 1);
    if (closing == kNowhere || text[closing] == '\n')
        return kNowhere;
    return closing + 1;
}

/// Whether nothing of `text` would show on screen — blanks and format tags only.
///
/// **The tags are why this exists.** Nine subtitles of the corpus are written
/// `<i>[PEOPLE SCREAMING]</i>`, and without ignoring the markup the removal
/// would leave `<i></i>`: an empty subtitle nothing would take away. ADR 0009
/// keeps the text raw, tags included, so it is the judgement that ignores them
/// and never the text that loses them.
[[nodiscard]] bool showsNothing(std::string_view text) {
    for (std::size_t index = 0; index < text.size();) {
        if (isBlank(text[index]) || text[index] == '\n') {
            ++index;
            continue;
        }
        const std::size_t tag = tagEnd(text, index);
        if (tag == kNowhere)
            return false;
        index = tag;
    }
    return true;
}

/// Whether the rest of the line from `at` shows nothing.
[[nodiscard]] bool restOfLineShowsNothing(std::string_view text, std::size_t at) {
    const std::size_t lineEnd = text.find('\n', at);
    return showsNothing(text.substr(at, lineEnd == kNowhere ? kNowhere : lineEnd - at));
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
[[nodiscard]] bool showsNothingButItsDash(std::string_view text) {
    return showsNothing(opensWithDash(text) ? text.substr(1) : text);
}

/// Takes the dialogue dash off a line, with the blanks that followed it.
void takeDashOff(std::string& text) {
    std::size_t index = 1;
    while (index < text.size() && isBlank(text[index]))
        ++index;
    text.erase(0, index);
}

/// Removes every mention, and says on which lines it removed one.
[[nodiscard]] std::vector<Line> scanned(std::string_view text) {
    std::vector<Line> lines{Line{}};
    bool seamPending = false;
    bool seamIsLineBreak = false;

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

        const std::size_t mention = mentionEnd(text, index);
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
            const bool atEdge =
                showsNothing(lines.back().text) || restOfLineShowsNothing(text, index);
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

        lines.back().text += text[index];
        ++index;
    }

    trimTrailingBlanks(lines.back().text);
    return lines;
}

} // namespace

std::optional<std::string> withoutHearingImpaired(std::string_view text) {
    std::vector<Line> lines = scanned(text);

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
        if (line.touched && showsNothingButItsDash(line.text)) {
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

    if (showsNothing(joined))
        return std::nullopt;
    return joined;
}

} // namespace subedit::core
