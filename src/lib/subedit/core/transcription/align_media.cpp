#include <subedit/core/transcription/align_media.hpp>

#include <filesystem>
#include <functional>
#include <utility>

namespace subedit::core {

namespace {

/// Has `aligner` match the lines on `audio`, leaving the result in `matches`.
std::expected<void, TranscriptionError> listen(const TranscriptMatchingAligner& aligner,
                                               const MediaAlignmentRequest& request,
                                               const TranscriptionObserver& observer,
                                               std::vector<LineMatch>& matches,
                                               const std::filesystem::path& audio) {
    auto matched = aligner.match({.audio = audio,
                                  .lines = request.lines,
                                  .language = request.media.language,
                                  .model = request.media.model,
                                  .vadModel = request.media.vadModel,
                                  .start = request.media.start,
                                  .end = request.media.end},
                                 observer);
    if (!matched)
        return std::unexpected(matched.error());
    matches = std::move(*matched);
    return {};
}

} // namespace

std::expected<std::vector<LineMatch>, MediaTranscriptionError>
alignMedia(FileSystem& files,
           const TranscriptMatchingAligner& aligner,
           std::string_view searchPath,
           const MediaAlignmentRequest& request,
           const TranscriptionObserver& observer) {
    const auto capabilities = aligner.capabilitiesFor(request.media.model);
    if (!capabilities)
        return std::unexpected(MediaTranscriptionError{.cause = capabilities.error()});

    std::vector<LineMatch> matches;
    const auto prepared = withPreparedAudio(files,
                                            *capabilities,
                                            searchPath,
                                            request.media,
                                            observer,
                                            std::bind_front(listen,
                                                            std::cref(aligner),
                                                            std::cref(request),
                                                            std::cref(observer),
                                                            std::ref(matches)));
    if (!prepared)
        return std::unexpected(prepared.error());
    return matches;
}

} // namespace subedit::core
