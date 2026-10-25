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

/// Cuts `piece` into subtitles that fit, at word boundaries.
void split(const Piece& piece, const AssemblySettings& settings, Assembly& out) {
    std::string text;
    Timestamp start = piece.start;
    Timestamp end = piece.start;

    const auto flush = [&] {
        out.subtitles.push_back(Subtitle{.start = start, .end = end, .mainText = std::move(text)});
        out.sources.push_back(piece.sources);
        text.clear();
    };

    for (const Token& token : tokensOf(piece)) {
        const bool tooLong = !text.empty() && (token.end - start > settings.maxDuration ||
                                               charactersOf(text) + 1 + charactersOf(token.text) >
                                                   settings.maxCharacters);
        if (tooLong)
            flush();
        if (text.empty())
            start = token.start;
        else
            text += ' ';
        text += token.text;
        end = token.end;
    }
    if (!text.empty())
        flush();
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
