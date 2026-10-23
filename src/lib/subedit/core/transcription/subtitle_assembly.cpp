#include <subedit/core/transcription/subtitle_assembly.hpp>

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
                       .words = {}};
    return fits(joined, settings);
}

void merge(Piece& into, Piece next) {
    into.text += " " + next.text;
    into.end = std::max(into.end, next.end);
    into.words.insert(into.words.end(),
                      std::make_move_iterator(next.words.begin()),
                      std::make_move_iterator(next.words.end()));
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
void split(const Piece& piece, const AssemblySettings& settings, std::vector<Subtitle>& out) {
    std::string text;
    Timestamp start = piece.start;
    Timestamp end = piece.start;

    const auto flush = [&] {
        out.push_back(Subtitle{.start = start, .end = end, .mainText = std::move(text)});
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

std::vector<Subtitle> assembleSubtitles(std::span<const TranscribedSegment> segments,
                                        const AssemblySettings& settings) {
    std::vector<Piece> pieces;
    for (const TranscribedSegment& segment : segments) {
        Piece next{.text = segment.text,
                   .start = segment.start,
                   .end = segment.end,
                   .speaker = segment.speaker,
                   .words = segment.words};
        if (!pieces.empty() && canMerge(pieces.back(), next, settings))
            merge(pieces.back(), std::move(next));
        else
            pieces.push_back(std::move(next));
    }

    std::vector<Subtitle> subtitles;
    for (const Piece& piece : pieces) {
        if (fits(piece, settings))
            subtitles.push_back(
                Subtitle{.start = piece.start, .end = piece.end, .mainText = piece.text});
        else
            split(piece, settings, subtitles);
    }
    return subtitles;
}

} // namespace subedit::core
