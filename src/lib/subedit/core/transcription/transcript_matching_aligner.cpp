#include <subedit/core/transcription/transcript_matching_aligner.hpp>

#include <expected>
#include <memory>
#include <utility>

namespace subedit::core {

namespace {

[[nodiscard]] std::vector<TimedWord> wordsOf(const Transcription& heard) {
    std::vector<TimedWord> words;
    for (const TranscribedSegment& segment : heard.segments)
        words.insert(words.end(), segment.words.begin(), segment.words.end());
    return words;
}

} // namespace

TranscriptMatchingAligner::TranscriptMatchingAligner(const TranscriptionEngine& transcriber,
                                                     double minimumMatch)
    : m_transcriber(&transcriber), m_minimumMatch(minimumMatch) {}

std::string_view TranscriptMatchingAligner::name() const {
    return kDefaultAlignmentEngine;
}

std::expected<EngineCapabilities, TranscriptionError> TranscriptMatchingAligner::capabilitiesFor(
    const std::optional<std::filesystem::path>& model) const {
    return m_transcriber->capabilitiesFor(model);
}

std::expected<std::vector<LineMatch>, TranscriptionError>
TranscriptMatchingAligner::match(const AlignmentRequest& request,
                                 const TranscriptionObserver& observer) const {
    const auto capabilities = m_transcriber->capabilitiesFor(request.model);
    if (!capabilities)
        return std::unexpected(capabilities.error());
    if (request.lines.empty())
        return std::vector<LineMatch>{};
    // The matching is built on the moments of the words: an engine that gives none cannot align.
    if (!capabilities->timedWords)
        return std::unexpected(TranscriptionError{
            .kind = TranscriptionErrorKind::Failed,
            .detail = "the engine gives no timed words, which the alignment is built on"});

    const auto heard = m_transcriber->transcribe({.audio = request.audio,
                                                  .language = request.language,
                                                  .model = request.model,
                                                  .vadModel = request.vadModel,
                                                  .start = request.start,
                                                  .end = request.end},
                                                 observer);
    if (!heard)
        return std::unexpected(heard.error());

    return matchLinesToWords(request.lines, wordsOf(*heard), m_minimumMatch);
}

std::expected<std::vector<AlignedLine>, TranscriptionError>
TranscriptMatchingAligner::align(const AlignmentRequest& request,
                                 const TranscriptionObserver& observer) const {
    const auto matches = match(request, observer);
    if (!matches)
        return std::unexpected(matches.error());

    std::vector<AlignedLine> places;
    places.reserve(matches->size());
    for (const LineMatch& line : *matches)
        places.push_back(line.place);
    return places;
}

std::unique_ptr<AlignmentEngine> makeAlignmentEngine(std::string_view name,
                                                     const TranscriptionEngine& transcriber,
                                                     double minimumMatch) {
    if (name == kDefaultAlignmentEngine)
        return std::make_unique<TranscriptMatchingAligner>(transcriber, minimumMatch);
    return nullptr;
}

} // namespace subedit::core
