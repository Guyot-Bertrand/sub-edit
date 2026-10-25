#include <subedit/core/transcription/subtitle_assembly.hpp>

#include <unicode/uchar.h>
#include <unicode/unistr.h>

#include <algorithm>
#include <cmath>
#include <string>
#include <string_view>
#include <utility>

namespace subedit::core {

namespace {

/// A piece of speech on its way to becoming a subtitle.
struct Piece {
    std::string text;
    Timestamp start;
    Timestamp end;
    std::optional<std::string> speaker;
    std::vector<TimedWord> words;

    /// The segments (by index) this piece is made of.
    std::vector<std::size_t> sources;
};

/// A unit a piece is cut at: a word, with the moment it is said.
struct Token {
    std::string text;
    Timestamp start;
    Timestamp end;
};

/// The characters of `text`, not its bytes: a continuation byte of UTF-8 begins
/// no character.
[[nodiscard]] std::size_t charactersOf(std::string_view text) {
    return static_cast<std::size_t>(icu::UnicodeString::fromUTF8(text).countChar32());
}

/// Whether `text` has a letter or a digit anywhere — `u_isalnum` over the whole of Unicode, so that
/// a Greek or a Chinese text has them too.
[[nodiscard]] bool hasLetterOrDigit(std::string_view text) {
    const icu::UnicodeString characters = icu::UnicodeString::fromUTF8(text);
    for (int32_t at = 0; at < characters.length(); at = characters.moveIndex32(at, 1))
        if (u_isalnum(characters.char32At(at)) != 0)
            return true;
    return false;
}

[[nodiscard]] bool endsSentence(std::string_view text) {
    return text.ends_with('.') || text.ends_with('!') || text.ends_with('?') ||
           text.ends_with("\xE2\x80\xA6"); // the ellipsis
}

[[nodiscard]] bool fits(const Piece& piece, const AssemblySettings& settings) {
    return piece.end - piece.start <= settings.maxDuration &&
           charactersOf(piece.text) <= settings.maxCharacters;
}

[[nodiscard]] bool
canMerge(const Piece& before, const Piece& after, const AssemblySettings& settings) {
    if (before.speaker != after.speaker || endsSentence(before.text))
        return false;
    if (after.start - before.end > settings.maxGap)
        return false;
    const Piece joined{.text = before.text + " " + after.text,
                       .start = before.start,
                       .end = std::max(before.end, after.end),
                       .speaker = {},
                       .words = {},
                       .sources = {}};
    return fits(joined, settings);
}

void merge(Piece& into, Piece next) {
    into.text += " " + next.text;
    into.end = std::max(into.end, next.end);
    into.words.insert(into.words.end(),
                      std::make_move_iterator(next.words.begin()),
                      std::make_move_iterator(next.words.end()));
    into.sources.insert(into.sources.end(), next.sources.begin(), next.sources.end());
}

/// The words of a piece with their moments: the engine's when it gave them,
/// else the words of the text spread over the piece in proportion to their
/// letters.
[[nodiscard]] std::vector<Token> tokensOf(const Piece& piece) {
    std::vector<Token> tokens;
    if (!piece.words.empty()) {
        for (const TimedWord& word : piece.words)
            tokens.push_back({word.text, word.start, word.end});
        return tokens;
    }

    std::vector<std::string> words;
    std::size_t start = 0;
    while (start < piece.text.size()) {
        const std::size_t end = std::min(piece.text.find(' ', start), piece.text.size());
        if (end > start)
            words.push_back(piece.text.substr(start, end - start));
        start = end + 1;
    }

    std::size_t total = 0;
    for (const std::string& word : words)
        total += charactersOf(word);
    const auto span = static_cast<double>((piece.end - piece.start).milliseconds());
    const auto allLetters = static_cast<double>(std::max<std::size_t>(total, 1));
    // The share of the piece before a word, as a fraction: the product of
    // two large integers would overflow, a fraction of a double cannot.
    const auto moment = [&](std::size_t letters) {
        return piece.start + Duration::fromMilliseconds(
                                 std::llround(span * static_cast<double>(letters) / allLetters));
    };
    std::size_t before = 0;
    for (std::string& word : words) {
        const auto from = moment(before);
        before += charactersOf(word);
        tokens.push_back({std::move(word), from, moment(before)});
    }
    return tokens;
}

/// How good a place to cut a punctuation mark makes the end of a word: the lower, the better.
enum class Cut { Sentence = 0, Clause = 1, Plain = 2 };

/// The last character of `text` that is not a closing quote or bracket, so that `urge."` ends a
/// sentence as `urge.` does.
[[nodiscard]] UChar32 lastSignificant(std::string_view text) {
    const icu::UnicodeString characters = icu::UnicodeString::fromUTF8(text);
    for (int32_t at = characters.length(); at > 0;) {
        at = characters.moveIndex32(at, -1);
        const UChar32 character = characters.char32At(at);
        if (u_charType(character) != U_END_PUNCTUATION &&
            u_charType(character) != U_FINAL_PUNCTUATION && character != '"' && character != '\'')
            return character;
    }
    return 0;
}

// The marks a cut is ranked by, besides the ASCII ones: the ellipsis, and the ideographic and
// full-width forms that Chinese and Japanese write.
constexpr UChar32 kEllipsis = 0x2026;
constexpr UChar32 kIdeographicFullStop = 0x3002;
constexpr UChar32 kFullWidthExclamation = 0xFF01;
constexpr UChar32 kFullWidthQuestion = 0xFF1F;
constexpr UChar32 kIdeographicComma = 0x3001;
constexpr UChar32 kFullWidthComma = 0xFF0C;
constexpr UChar32 kFullWidthSemicolon = 0xFF1B;
constexpr UChar32 kFullWidthColon = 0xFF1A;

[[nodiscard]] Cut cutAfter(std::string_view word) {
    const UChar32 mark = lastSignificant(word);
    for (const UChar32 sentence : {UChar32{'.'},
                                   UChar32{'!'},
                                   UChar32{'?'},
                                   kEllipsis,
                                   kIdeographicFullStop,
                                   kFullWidthExclamation,
                                   kFullWidthQuestion})
        if (mark == sentence)
            return Cut::Sentence;
    for (const UChar32 clause : {UChar32{','},
                                 UChar32{';'},
                                 UChar32{':'},
                                 kIdeographicComma,
                                 kFullWidthComma,
                                 kFullWidthSemicolon,
                                 kFullWidthColon})
        if (mark == clause)
            return Cut::Clause;
    return Cut::Plain;
}

/// The words `first` to `last` (excluded) of a piece, being cut into subtitles.
struct Cutter {
    const Piece& piece;
    const std::vector<Token>& tokens;
    const AssemblySettings& settings;
    Assembly& out;

    [[nodiscard]] Timestamp startOf(std::size_t first) const {
        return first == 0 ? piece.start : tokens[first].start;
    }

    [[nodiscard]] Timestamp endOf(std::size_t last) const {
        return last == tokens.size() ? piece.end : tokens[last - 1].end;
    }

    [[nodiscard]] std::size_t charactersIn(std::size_t first, std::size_t last) const {
        std::size_t count = 0;
        for (std::size_t at = first; at < last; ++at)
            count += charactersOf(tokens[at].text) + (at > first ? 1 : 0);
        return count;
    }

    [[nodiscard]] bool fitsIn(std::size_t first, std::size_t last) const {
        return endOf(last) - startOf(first) <= settings.maxDuration &&
               charactersIn(first, last) <= settings.maxCharacters;
    }

    [[nodiscard]] bool tooSmall(std::size_t first, std::size_t last) const {
        return endOf(last) - startOf(first) < settings.minDuration ||
               charactersIn(first, last) < settings.minCharacters;
    }

    /// Where to cut `first` to `last`: at the best punctuation, else in the middle of a sentence;
    /// among equals, where the two halves are the most alike. A cut that leaves a half too small
    /// is only taken when there is no other.
    [[nodiscard]] std::size_t cutOf(std::size_t first, std::size_t last) const {
        std::size_t best = 0;
        for (const bool avoidSmall : {true, false}) {
            Cut bestKind = Cut::Plain;
            std::size_t bestGap = 0;
            for (std::size_t at = first + 1; at < last; ++at) {
                if (avoidSmall && (tooSmall(first, at) || tooSmall(at, last)))
                    continue;
                const Cut kind = cutAfter(tokens[at - 1].text);
                const std::size_t left = charactersIn(first, at);
                const std::size_t right = charactersIn(at, last);
                const std::size_t gap = left > right ? left - right : right - left;
                if (best == 0 || kind < bestKind || (kind == bestKind && gap < bestGap)) {
                    best = at;
                    bestKind = kind;
                    bestGap = gap;
                }
            }
            if (best != 0)
                break;
        }
        return best;
    }

    void run(std::size_t first, std::size_t last) {
        const std::size_t at = last - first > 1 && !fitsIn(first, last) ? cutOf(first, last) : 0;
        if (at == 0) {
            std::string text;
            for (std::size_t index = first; index < last; ++index)
                text += (index > first ? " " : "") + tokens[index].text;
            out.subtitles.push_back(
                Subtitle{.start = startOf(first), .end = endOf(last), .mainText = std::move(text)});
            out.sources.push_back(piece.sources);
            return;
        }
        run(first, at);
        run(at, last);
    }
};

/// Cuts `piece` into subtitles that fit, at word boundaries: **a subtitle that begins the piece
/// keeps its start, and one that ends it keeps its end** — the timed words are only trusted for
/// the cuts inside it.
void split(const Piece& piece, const AssemblySettings& settings, Assembly& out) {
    const std::vector<Token> tokens = tokensOf(piece);
    Cutter{.piece = piece, .tokens = tokens, .settings = settings, .out = out}.run(0,
                                                                                   tokens.size());
}

} // namespace

AssemblySettings assemblyOf(const TranscriptionSettings& settings) {
    return {.maxGap = Duration::fromMilliseconds(settings.mergeGapMilliseconds),
            .maxDuration = Duration::fromMilliseconds(settings.maxDurationMilliseconds),
            .maxCharacters = static_cast<std::size_t>(settings.maxCharacters)};
}

Assembly assemble(std::span<const TranscribedSegment> segments, const AssemblySettings& settings) {
    std::vector<Piece> pieces;
    for (std::size_t index = 0; index < segments.size(); ++index) {
        const TranscribedSegment& segment = segments[index];
        if (!hasLetterOrDigit(segment.text))
            continue;
        Piece next{.text = segment.text,
                   .start = segment.start,
                   .end = segment.end,
                   .speaker = segment.speaker,
                   .words = segment.words,
                   .sources = {index}};
        if (!pieces.empty() && canMerge(pieces.back(), next, settings))
            merge(pieces.back(), std::move(next));
        else
            pieces.push_back(std::move(next));
    }

    Assembly made;
    for (const Piece& piece : pieces) {
        if (fits(piece, settings)) {
            made.subtitles.push_back(
                Subtitle{.start = piece.start, .end = piece.end, .mainText = piece.text});
            made.sources.push_back(piece.sources);
        } else {
            split(piece, settings, made);
        }
    }
    return made;
}

std::vector<Subtitle> assembleSubtitles(std::span<const TranscribedSegment> segments,
                                        const AssemblySettings& settings) {
    return assemble(segments, settings).subtitles;
}

} // namespace subedit::core
