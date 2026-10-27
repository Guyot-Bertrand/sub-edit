#include <subedit/core/io/json_reader.hpp>
#include <subedit/core/transcription/time_range.hpp>
#include <subedit/core/transcription/whisper_cpp_output.hpp>

#include <algorithm>
#include <charconv>
#include <cmath>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

namespace subedit::core {

namespace {

constexpr std::string_view kSpecialTokenPrefix = "[_";
constexpr std::string_view kProgressMarker = "progress =";
constexpr std::string_view kModelMagic = "lmgg";
/// The furthest an offset can be from the origin: about thirty years, in
/// milliseconds. Far beyond any film, and far inside what an `int64_t` holds.
constexpr double kMaxOffsetMilliseconds = 1e12;

constexpr std::uint32_t kFirstMultilingualVocabulary = 51865;
constexpr std::size_t kMagicSize = 4;
constexpr std::size_t kVocabularyBytes = 4;
constexpr std::size_t kBitsPerByte = 8;

/// The `offsets` of an object as a pair of timestamps, or nothing.
[[nodiscard]] std::optional<std::pair<Timestamp, Timestamp>> offsetsOf(const JsonValue& object) {
    const JsonValue* offsets = object.find("offsets");
    const JsonValue* from = offsets != nullptr ? offsets->find("from") : nullptr;
    const JsonValue* to = offsets != nullptr ? offsets->find("to") : nullptr;
    if (from == nullptr || to == nullptr)
        return std::nullopt;
    const std::optional<double> fromValue = from->asNumber();
    const std::optional<double> toValue = to->asNumber();
    const auto sensible = [](double value) { return std::abs(value) <= kMaxOffsetMilliseconds; };
    if (!fromValue || !toValue || !sensible(*fromValue) || !sensible(*toValue))
        return std::nullopt;
    return std::pair{Timestamp::fromMilliseconds(static_cast<std::int64_t>(*fromValue)),
                     Timestamp::fromMilliseconds(static_cast<std::int64_t>(*toValue))};
}

/// A word being built from its tokens.
struct PendingWord {
    TimedWord word;
    double confidenceSum = 0;
    int tokens = 0;
};

void closeWord(std::optional<PendingWord>& pending, std::vector<TimedWord>& words) {
    if (!pending)
        return;
    if (pending->tokens > 0)
        pending->word.confidence = pending->confidenceSum / pending->tokens;
    words.push_back(std::move(pending->word));
    pending.reset();
}

/// The words of a segment, rebuilt from its tokens.
[[nodiscard]] std::vector<TimedWord> wordsOf(const JsonValue& segment) {
    std::vector<TimedWord> words;
    const JsonValue* tokens = segment.find("tokens");
    if (tokens == nullptr || tokens->asArray() == nullptr)
        return words;

    std::optional<PendingWord> pending;
    for (const JsonValue& token : *tokens->asArray()) {
        const JsonValue* text = token.find("text");
        const std::string* piece = text != nullptr ? text->asString() : nullptr;
        const auto offsets = offsetsOf(token);
        if (piece == nullptr || !offsets || piece->starts_with(kSpecialTokenPrefix))
            continue;

        const std::pair<Timestamp, Timestamp> span = *offsets;
        if (piece->starts_with(' ') || !pending) {
            closeWord(pending, words);
            pending = PendingWord{.word = {.text = {}, .start = span.first, .end = span.second}};
        }
        PendingWord& word = *pending;
        word.word.text += piece->starts_with(' ') ? piece->substr(1) : *piece;
        word.word.end = span.second;

        const JsonValue* probability = token.find("p");
        const std::optional<double> confidence =
            probability != nullptr ? probability->asNumber() : std::nullopt;
        if (confidence) {
            word.confidenceSum += *confidence;
            ++word.tokens;
        }
    }
    closeWord(pending, words);
    return words;
}

[[nodiscard]] std::string withoutLeadingSpace(const std::string& text) {
    const std::size_t first = text.find_first_not_of(' ');
    return first == std::string::npos ? std::string{} : text.substr(first);
}

} // namespace

std::optional<WhisperOutput> readWhisperJson(std::string_view text) {
    const std::optional<JsonValue> document = parseJson(text);
    if (!document)
        return std::nullopt;
    const JsonValue* segments = document->find("transcription");
    if (segments == nullptr || segments->asArray() == nullptr)
        return std::nullopt;

    WhisperOutput output;
    const JsonValue* model = document->find("model");
    const JsonValue* multilingual = model != nullptr ? model->find("multilingual") : nullptr;
    output.multilingual = multilingual != nullptr && multilingual->asBool().value_or(false);

    const JsonValue* result = document->find("result");
    const JsonValue* language = result != nullptr ? result->find("language") : nullptr;
    if (language != nullptr && language->asString() != nullptr)
        output.transcription.language = *language->asString();

    for (const JsonValue& segment : *segments->asArray()) {
        const JsonValue* spoken = segment.find("text");
        const auto offsets = offsetsOf(segment);
        if (spoken == nullptr || spoken->asString() == nullptr || !offsets)
            return std::nullopt;
        std::string words = withoutLeadingSpace(*spoken->asString());
        if (words.empty())
            continue;

        TranscribedSegment kept{
            .text = std::move(words), .start = offsets->first, .end = offsets->second};
        kept.words = wordsOf(segment);
        output.transcription.segments.push_back(std::move(kept));
    }
    return output;
}

std::optional<double> readWhisperProgress(std::string_view line) {
    const std::size_t marker = line.find(kProgressMarker);
    if (marker == std::string_view::npos)
        return std::nullopt;
    std::string_view rest = line.substr(marker + kProgressMarker.size());
    rest.remove_prefix(rest.find_first_not_of(' ') == std::string_view::npos
                           ? rest.size()
                           : rest.find_first_not_of(' '));

    int percent = 0;
    const std::from_chars_result parsed =
        std::from_chars(rest.data(), rest.data() + rest.size(), percent);
    if (parsed.ec != std::errc{} || parsed.ptr == rest.data() + rest.size() || *parsed.ptr != '%')
        return std::nullopt;
    return static_cast<double>(percent) / 100.0;
}

namespace {

constexpr std::string_view kSamplesMarker = " samples, ";

/// `HH:MM:SS.mmm` at the start of `text`, which is moved past it.
[[nodiscard]] std::optional<Timestamp> takeClock(std::string_view& text) {
    constexpr std::size_t kClockSize = 12;
    if (text.size() < kClockSize)
        return std::nullopt;
    const std::optional<Timestamp> clock = Timestamp::parse(text.substr(0, kClockSize));
    if (clock)
        text.remove_prefix(kClockSize);
    return clock;
}

} // namespace

std::optional<WhisperSegmentLine> readWhisperSegmentLine(std::string_view line) {
    constexpr std::string_view kArrow = " --> ";
    if (!line.starts_with('['))
        return std::nullopt;
    line.remove_prefix(1);
    const std::optional<Timestamp> start = takeClock(line);
    if (!start || !line.starts_with(kArrow))
        return std::nullopt;
    line.remove_prefix(kArrow.size());
    const std::optional<Timestamp> end = takeClock(line);
    if (!end || !line.starts_with(']'))
        return std::nullopt;
    line.remove_prefix(1);
    const std::size_t first = line.find_first_not_of(' ');
    return WhisperSegmentLine{
        .start = *start,
        .end = *end,
        .text =
            std::string{first == std::string_view::npos ? std::string_view{} : line.substr(first)}};
}

std::optional<Timestamp> readWhisperAudioLength(std::string_view line) {
    const std::size_t marker = line.rfind(kSamplesMarker);
    if (marker == std::string_view::npos || !line.starts_with("main: processing"))
        return std::nullopt;
    const std::string_view rest = line.substr(marker + kSamplesMarker.size());
    double seconds = 0;
    const std::from_chars_result parsed =
        std::from_chars(rest.data(), rest.data() + rest.size(), seconds);
    if (parsed.ec != std::errc{} ||
        !std::string_view{parsed.ptr, rest.data() + rest.size()}.starts_with(" sec"))
        return std::nullopt;
    return timestampFromSeconds(seconds);
}

std::optional<bool> readModelIsMultilingual(std::string_view header) {
    if (header.size() < kModelHeaderSize || !header.starts_with(kModelMagic))
        return std::nullopt;
    std::uint32_t vocabulary = 0;
    for (std::size_t index = 0; index < kVocabularyBytes; ++index)
        vocabulary |=
            static_cast<std::uint32_t>(static_cast<unsigned char>(header[kMagicSize + index]))
            << (kBitsPerByte * index);
    return vocabulary >= kFirstMultilingualVocabulary;
}

void putWordsOnTheSegments(std::vector<TranscribedSegment>& segments) {
    for (TranscribedSegment& segment : segments) {
        if (segment.words.empty())
            continue;
        const Duration shift = segment.start - segment.words.front().start;
        for (TimedWord& word : segment.words) {
            word.start = std::clamp(word.start + shift, segment.start, segment.end);
            word.end = std::clamp(word.end + shift, word.start, segment.end);
        }
    }
}

} // namespace subedit::core
