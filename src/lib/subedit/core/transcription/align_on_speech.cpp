#include <subedit/core/transcription/align_on_speech.hpp>

#include <algorithm>
#include <functional>
#include <utility>

namespace subedit::core {

namespace {

/// Below this, the lines and the sound have too little to do with one another for the correction
/// to be believed: the wrong film, or one with hardly any speech. Settled on the observations of
/// #820, on a few episodes of a series: a file laid on its own film agrees at about 0.3, one laid
/// on another film at less than 0.1.
constexpr double kLeastAgreement = 0.15;

/// The stretches of `spans` inside the part of the film asked for.
[[nodiscard]] std::vector<SpeechSpan> within(std::vector<SpeechSpan> spans,
                                             const std::optional<Timestamp>& start,
                                             const std::optional<Timestamp>& end) {
    std::erase_if(spans, [&](const SpeechSpan& span) {
        return (start && span.end < *start) || (end && span.start > *end);
    });
    return spans;
}

/// Has `detector` find the speech in `audio`, leaving it in `heard`.
std::expected<void, TranscriptionError> listen(const SpeechDetector& detector,
                                               const SpeechAlignmentRequest& request,
                                               const TranscriptionObserver& observer,
                                               std::vector<SpeechSpan>& heard,
                                               const std::filesystem::path& audio) {
    auto spans = detector.detect(audio, request.media.vadModel, observer);
    if (!spans)
        return std::unexpected(spans.error());
    heard = std::move(*spans);
    return {};
}

[[nodiscard]] SpeechAlignment laidOn(const SpeechCorrection& correction,
                                     std::span<const LineTimes> lines) {
    SpeechAlignment alignment{.correction = correction, .matches = {}};
    alignment.matches.reserve(lines.size());
    for (const LineTimes& line : lines) {
        LineMatch match;
        if (correction.fitted()) {
            match.place = {.start = correction.corrected(line.start),
                           .end = correction.corrected(line.end)};
            match.words = 1;
            match.matched = 1;
        }
        alignment.matches.push_back(match);
    }
    return alignment;
}

} // namespace

std::expected<SpeechAlignment, MediaTranscriptionError>
alignMediaOnSpeech(FileSystem& files,
                   const SpeechDetector& detector,
                   std::string_view searchPath,
                   const SpeechAlignmentRequest& request,
                   const TranscriptionObserver& observer) {
    // The voice detector reads 16 kHz, and asks for no language and no model of speech: saying it
    // detects the language is how the check that precedes the extraction is told not to ask for
    // one.
    const EngineCapabilities capabilities{.languageDetection = true,
                                          .voiceActivity = true,
                                          .sampleRate = kSpeechSampleRate,
                                          .needsModel = false};
    std::vector<SpeechSpan> heard;
    const auto prepared = withPreparedAudio(
        files,
        capabilities,
        searchPath,
        request.media,
        observer,
        std::bind_front(
            listen, std::cref(detector), std::cref(request), std::cref(observer), std::ref(heard)));
    if (!prepared)
        return std::unexpected(prepared.error());

    heard = within(std::move(heard), request.media.start, request.media.end);
    SpeechCorrection correction = estimateSpeechCorrection(heard, request.lines);
    if (correction.agreement < kLeastAgreement)
        correction.shifts.clear();
    return laidOn(correction, request.lines);
}

} // namespace subedit::core
