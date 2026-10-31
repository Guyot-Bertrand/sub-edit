#pragma once

#include <subedit/core/transcription/transcribe_media.hpp>
#include <subedit/core/transcription/transcript_matching_aligner.hpp>
#include <subedit/core/transcription/word_matching.hpp>

#include <expected>
#include <string>
#include <string_view>
#include <vector>

namespace subedit::core {

/// What aligning a text on a video or an audio file is asked to do. `media` carries the audio and
/// the choices of the listener (language, models, range); `lines` are the lines of the text.
struct MediaAlignmentRequest {
    MediaTranscriptionRequest media;
    std::vector<std::string> lines;
};

/// Prepares the audio and has `aligner` match `request.lines` to what it hears: how each line
/// came out, with the counts the doubt is judged on.
///
/// The one road both surfaces take, as `transcribeMedia` is for transcription.
[[nodiscard]] std::expected<std::vector<LineMatch>, MediaTranscriptionError>
alignMedia(FileSystem& files,
           const TranscriptMatchingAligner& aligner,
           std::string_view searchPath,
           const MediaAlignmentRequest& request,
           const TranscriptionObserver& observer = {});

} // namespace subedit::core
