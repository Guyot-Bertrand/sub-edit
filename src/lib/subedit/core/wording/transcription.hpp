#pragma once

// What both surfaces say of a transcription — why it did not happen, which tracks a
// video has, what it came to. Shared wording of `core/wording/`; see `formats.hpp` for
// why it lives in the core. Phase 22, #777.

#include <subedit/core/transcription/audio_source.hpp>
#include <subedit/core/transcription/transcription_error.hpp>

#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

namespace subedit::core {

/// Why a transcription did not happen, as a sentence. The causes that are the user's to
/// put right by installing something end with the pointer to the guide.
[[nodiscard]] std::string transcriptionErrorText(const TranscriptionError& error);

/// Why the audio could not be prepared, as a sentence, with the pointer to the guide
/// when the tool is missing.
[[nodiscard]] std::string audioPreparationErrorText(const AudioPreparationError& error);

/// The audio tracks of a video, one per line: what `--audio-track` takes, and what the
/// window offers. A video with none says so.
[[nodiscard]] std::string audioTracksText(const std::vector<ContainerAudioTrack>& tracks);

/// There is no engine of that name.
[[nodiscard]] std::string noSuchTranscriptionEngine(std::string_view name);

/// What a transcription came to: how many subtitles, in which language.
[[nodiscard]] std::string noticeOfTranscription(std::size_t subtitles, std::string_view language);

/// Where the engine has got to, in percent.
[[nodiscard]] std::string transcriptionProgressText(int percent);

/// Where to read how to install what the transcription needs.
[[nodiscard]] std::string installationGuidePointer();

} // namespace subedit::core
