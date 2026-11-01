#pragma once

#include <subedit/core/transcription/speech_correction.hpp>
#include <subedit/core/transcription/speech_detector.hpp>
#include <subedit/core/transcription/transcribe_media.hpp>
#include <subedit/core/transcription/word_matching.hpp>

#include <expected>
#include <string_view>
#include <vector>

namespace subedit::core {

/// The rate of the audio the voice detector reads, in hertz.
inline constexpr int kSpeechSampleRate = 16000;

/// What laying a subtitle file on the sound of a film is asked to do. `media` carries the audio
/// and the part of the film (its `vadModel` is the voice-activity model); `lines` are the times of
/// the subtitles to move, which are all the sound is asked about.
struct SpeechAlignmentRequest {
    MediaTranscriptionRequest media;
    std::vector<LineTimes> lines;
};

/// What it came to: the correction found, and each line where it ends up.
struct SpeechAlignment {
    SpeechCorrection correction;

    /// One per line of the request, in order; shaped as the matching of words answers so that both
    /// methods share what comes after. A line is always placed, and counts as one word heard.
    std::vector<LineMatch> matches;
};

/// Prepares the audio, finds where there is speech, and puts the lines on it.
///
/// **No transcription, no model of a language**: the sound is only asked where somebody speaks, so
/// it works whatever the language of the subtitles and of the film, and takes seconds where a
/// transcription takes minutes. What it cannot do is look at one line: it corrects the file as a
/// whole. When the sound and the file have nothing in common, nothing is corrected, the matches
/// are all empty, and `SpeechAlignment::correction` says so.
[[nodiscard]] std::expected<SpeechAlignment, MediaTranscriptionError>
alignMediaOnSpeech(FileSystem& files,
                   const SpeechDetector& detector,
                   std::string_view searchPath,
                   const SpeechAlignmentRequest& request,
                   const TranscriptionObserver& observer = {});

} // namespace subedit::core
