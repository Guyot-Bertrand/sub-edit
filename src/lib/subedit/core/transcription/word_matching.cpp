#include <subedit/core/transcription/word_matching.hpp>

#include <unicode/normalizer2.h>
#include <unicode/uchar.h>
#include <unicode/unistr.h>
#include <unicode/utypes.h>

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

namespace subedit::core {

namespace {

constexpr std::size_t kNone = std::numeric_limits<std::size_t>::max();

/// A word needs this many bytes to be taken for a slip of the ear: a short word one letter off is
/// another word.
constexpr std::size_t kNearMinimumLength = 5;
constexpr std::size_t kNearLongLength = 9;
constexpr std::size_t kNearShortDistance = 1;
constexpr std::size_t kNearLongDistance = 2;

/// A word must be this long to anchor the alignment: short words come back too often to say where
/// they are.
constexpr std::size_t kAnchorMinimumLength = 4;

/// The most cells the alignment of one gap may fill; a gap beyond is left unmatched, which is
/// honest (the lines in it are not found) and bounds the memory at this many bytes.
constexpr std::size_t kMaximumCells = std::size_t{64} * 1024 * 1024;

constexpr int kScoreEqual = 2;
constexpr int kScoreNear = 1;
constexpr int kScoreOther = -1;
constexpr int kScoreGap = -1;

enum class Step : std::uint8_t { Diagonal, Up, Left };

/// A word of one side, once normalised, and where it comes from.
struct Unit {
    std::string text;
    std::size_t owner = 0;
};

/// Removes what is not said: styles, ASS overrides and bracketed sound notes.
[[nodiscard]] std::string withoutMarkup(std::string_view text) {
    std::string kept;
    kept.reserve(text.size());
    char closing = '\0';
    for (const char c : text) {
        if (closing != '\0') {
            if (c == closing)
                closing = '\0';
            continue;
        }
        if (c == '<')
            closing = '>';
        else if (c == '{')
            closing = '}';
        else if (c == '[')
            closing = ']';
        else
            kept.push_back(c);
    }
    return kept;
}

/// The words of a text: lower case, no accent, letters and digits only.
[[nodiscard]] std::vector<std::string> wordsOf(std::string_view text) {
    UErrorCode status = U_ZERO_ERROR;
    const icu::Normalizer2* decomposition = icu::Normalizer2::getNFDInstance(status);
    icu::UnicodeString folded = icu::UnicodeString::fromUTF8(withoutMarkup(text));
    folded.foldCase();
    const icu::UnicodeString plain =
        status <= U_ZERO_ERROR ? decomposition->normalize(folded, status) : folded;

    std::vector<std::string> words;
    icu::UnicodeString current;
    const auto flush = [&words, &current] {
        if (current.isEmpty())
            return;
        std::string utf8;
        current.toUTF8String(utf8);
        words.push_back(std::move(utf8));
        current.remove();
    };
    for (std::int32_t at = 0; at < plain.length();) {
        const UChar32 c = plain.char32At(at);
        at += U16_LENGTH(c);
        if (u_charType(c) == U_NON_SPACING_MARK)
            continue;
        if (u_isalnum(c) != 0)
            current.append(c);
        else
            flush();
    }
    flush();
    return words;
}

[[nodiscard]] std::vector<Unit> unitsOfLines(const std::vector<std::string>& lines) {
    std::vector<Unit> units;
    for (std::size_t line = 0; line < lines.size(); ++line)
        for (std::string& word : wordsOf(lines[line]))
            units.push_back({.text = std::move(word), .owner = line});
    return units;
}

[[nodiscard]] std::vector<Unit> unitsOfHeard(std::span<const TimedWord> heard) {
    std::vector<Unit> units;
    units.reserve(heard.size());
    for (std::size_t at = 0; at < heard.size(); ++at)
        for (std::string& word : wordsOf(heard[at].text))
            units.push_back({.text = std::move(word), .owner = at});
    return units;
}

[[nodiscard]] std::size_t distanceBetween(std::string_view a, std::string_view b) {
    std::vector<std::size_t> row(b.size() + 1);
    for (std::size_t j = 0; j <= b.size(); ++j)
        row[j] = j;
    for (std::size_t i = 1; i <= a.size(); ++i) {
        std::size_t diagonal = row[0];
        row[0] = i;
        for (std::size_t j = 1; j <= b.size(); ++j) {
            const std::size_t above = row[j];
            const std::size_t substitution = diagonal + (a[i - 1] == b[j - 1] ? 0U : 1U);
            row[j] = std::min({row[j - 1] + 1, above + 1, substitution});
            diagonal = above;
        }
    }
    return row[b.size()];
}

[[nodiscard]] bool isNear(std::string_view a, std::string_view b) {
    const std::size_t shortest = std::min(a.size(), b.size());
    if (shortest < kNearMinimumLength)
        return false;
    const std::size_t allowed =
        shortest >= kNearLongLength ? kNearLongDistance : kNearShortDistance;
    const std::size_t gap = std::max(a.size(), b.size()) - shortest;
    return gap <= allowed && distanceBetween(a, b) <= allowed;
}

[[nodiscard]] int scoreOf(const std::string& a, const std::string& b) {
    if (a == b)
        return kScoreEqual;
    return isNear(a, b) ? kScoreNear : kScoreOther;
}

[[nodiscard]] bool heardAlike(const std::string& a, const std::string& b) {
    return a == b || isNear(a, b);
}

struct Pair {
    std::size_t text = 0;
    std::size_t heard = 0;
};

/// The words that occur once on each side, in the order of the text, whose places in the heard
/// words never go back: the points the alignment is pinned to.
[[nodiscard]] std::vector<Pair> anchorsOf(const std::vector<Unit>& text,
                                          const std::vector<Unit>& heard) {
    std::unordered_map<std::string, std::size_t> textCount;
    std::unordered_map<std::string, std::size_t> heardCount;
    std::unordered_map<std::string, std::size_t> heardAt;
    for (const Unit& unit : text)
        ++textCount[unit.text];
    for (std::size_t at = 0; at < heard.size(); ++at) {
        ++heardCount[heard[at].text];
        heardAt[heard[at].text] = at;
    }

    std::vector<Pair> candidates;
    for (std::size_t at = 0; at < text.size(); ++at) {
        const std::string& word = text[at].text;
        if (word.size() < kAnchorMinimumLength || textCount[word] != 1 || heardCount[word] != 1)
            continue;
        candidates.push_back({.text = at, .heard = heardAt[word]});
    }

    // The longest run of candidates whose heard places increase, by patience.
    std::vector<std::size_t> tails;
    std::vector<std::size_t> previous(candidates.size(), kNone);
    for (std::size_t at = 0; at < candidates.size(); ++at) {
        const auto place = std::ranges::lower_bound(
            tails, candidates[at].heard, [&candidates](std::size_t tail, std::size_t value) {
                return candidates[tail].heard < value;
            });
        const auto rank = static_cast<std::size_t>(place - tails.begin());
        previous[at] = rank == 0 ? kNone : tails[rank - 1];
        if (place == tails.end())
            tails.push_back(at);
        else
            *place = at;
    }
    std::vector<Pair> anchors;
    for (std::size_t at = tails.empty() ? kNone : tails.back(); at != kNone; at = previous[at])
        anchors.push_back(candidates[at]);
    std::ranges::reverse(anchors);
    return anchors;
}

/// Aligns `text[textBegin, textEnd)` with `heard[heardBegin, heardEnd)` and records, for each text
/// word heard in the gap, the heard word it is.
struct Gap {
    std::size_t textBegin = 0;
    std::size_t textEnd = 0;
    std::size_t heardBegin = 0;
    std::size_t heardEnd = 0;
};

void alignGap(const std::vector<Unit>& text,
              const std::vector<Unit>& heard,
              const Gap& gap,
              std::vector<std::size_t>& heardOf) {
    const std::size_t rows = gap.textEnd - gap.textBegin;
    const std::size_t columns = gap.heardEnd - gap.heardBegin;
    if (rows == 0 || columns == 0 || rows > kMaximumCells / (columns + 1))
        return;

    const std::size_t width = columns + 1;
    std::vector<Step> steps((rows + 1) * width, Step::Diagonal);
    std::vector<int> previous(width);
    std::vector<int> current(width);
    for (std::size_t j = 0; j <= columns; ++j) {
        previous[j] = static_cast<int>(j) * kScoreGap;
        steps[j] = Step::Left;
    }
    for (std::size_t i = 1; i <= rows; ++i) {
        current[0] = static_cast<int>(i) * kScoreGap;
        steps[i * width] = Step::Up;
        const std::string& word = text[gap.textBegin + i - 1].text;
        for (std::size_t j = 1; j <= columns; ++j) {
            const int diagonal =
                previous[j - 1] + scoreOf(word, heard[gap.heardBegin + j - 1].text);
            const int up = previous[j] + kScoreGap;
            const int left = current[j - 1] + kScoreGap;
            current[j] = diagonal;
            steps[(i * width) + j] = Step::Diagonal;
            if (up > current[j]) {
                current[j] = up;
                steps[(i * width) + j] = Step::Up;
            }
            if (left > current[j]) {
                current[j] = left;
                steps[(i * width) + j] = Step::Left;
            }
        }
        std::swap(previous, current);
    }

    std::size_t i = rows;
    std::size_t j = columns;
    while (i > 0 || j > 0) {
        const Step step = steps[(i * width) + j];
        if (step == Step::Diagonal) {
            const std::size_t at = gap.textBegin + i - 1;
            const std::size_t heardAt = gap.heardBegin + j - 1;
            if (heardAlike(text[at].text, heard[heardAt].text))
                heardOf[at] = heardAt;
            --i;
            --j;
        } else if (step == Step::Up) {
            --i;
        } else {
            --j;
        }
    }
}

/// For each text word, the heard word it is, or `kNone`.
[[nodiscard]] std::vector<std::size_t> alignedWords(const std::vector<Unit>& text,
                                                    const std::vector<Unit>& heard) {
    std::vector<std::size_t> heardOf(text.size(), kNone);
    std::size_t textFrom = 0;
    std::size_t heardFrom = 0;
    for (const Pair& anchor : anchorsOf(text, heard)) {
        const Gap before{.textBegin = textFrom,
                         .textEnd = anchor.text,
                         .heardBegin = heardFrom,
                         .heardEnd = anchor.heard};
        alignGap(text, heard, before, heardOf);
        heardOf[anchor.text] = anchor.heard;
        textFrom = anchor.text + 1;
        heardFrom = anchor.heard + 1;
    }
    const Gap rest{.textBegin = textFrom,
                   .textEnd = text.size(),
                   .heardBegin = heardFrom,
                   .heardEnd = heard.size()};
    alignGap(text, heard, rest, heardOf);
    return heardOf;
}

} // namespace

std::vector<LineMatch> matchLinesToWords(const std::vector<std::string>& lines,
                                         std::span<const TimedWord> heard,
                                         double minimumMatch) {
    const std::vector<Unit> text = unitsOfLines(lines);
    const std::vector<Unit> sounds = unitsOfHeard(heard);
    const std::vector<std::size_t> heardOf = alignedWords(text, sounds);

    std::vector<LineMatch> matches(lines.size());
    std::vector<std::size_t> first(lines.size(), kNone);
    std::vector<std::size_t> last(lines.size(), kNone);
    for (std::size_t at = 0; at < text.size(); ++at) {
        const std::size_t line = text[at].owner;
        ++matches[line].words;
        if (heardOf[at] == kNone)
            continue;
        const std::size_t word = sounds[heardOf[at]].owner;
        ++matches[line].matched;
        if (first[line] == kNone)
            first[line] = word;
        last[line] = word;
    }

    for (std::size_t line = 0; line < lines.size(); ++line) {
        LineMatch& match = matches[line];
        const double needed =
            std::max(1.0, std::ceil(minimumMatch * static_cast<double>(match.words)));
        if (match.matched == 0 || static_cast<double>(match.matched) < needed)
            continue;
        const Timestamp start = heard[first[line]].start;
        match.place = {.start = start, .end = std::max(start, heard[last[line]].end)};
    }
    return matches;
}

} // namespace subedit::core
