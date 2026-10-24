#pragma once

// What both surfaces say of a transcription — why it did not happen, which tracks a
// video has, what it came to. Shared wording of `core/wording/`; see `formats.hpp` for
// why it lives in the core. Phase 22, #777.

#include <subedit/core/transcription/audio_source.hpp>
#include <subedit/core/transcription/tools_status.hpp>
#include <subedit/core/transcription/transcription_error.hpp>
#include <subedit/core/transcription/transcription_progress.hpp>

#include <chrono>
#include <cstddef>
#include <optional>
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

/// What replacing the subtitles of the project takes away, for the question that asks.
[[nodiscard]] std::string noticeOfReplacement(std::size_t subtitles);

/// Where the engine has got to, in percent.
[[nodiscard]] std::string transcriptionProgressText(int percent);

/// What is wrong with a bound of the range that could not be read.
[[nodiscard]] std::string notATimeInRange(std::string_view text);

/// A transcription of which a good part is notes in brackets, the language used probably wrong:
/// says so and what to do. `language` is the one the engine reports.
[[nodiscard]] std::string
noticeOfNotes(std::size_t notes, std::size_t pieces, std::string_view language);

/// A model for English alone will write everything as English, whatever the film speaks.
[[nodiscard]] std::string noticeOfEnglishOnlyModel();

/// What is wrong with a language that could not be read as a code.
[[nodiscard]] std::string notALanguageCode(std::string_view text);

/// The end of the range does not come after its start.
[[nodiscard]] std::string emptyRange();

/// A position as a clock, `H:MM:SS`, for a person to read where the work stands.
[[nodiscard]] std::string clockText(Timestamp position);

/// How long is left, roughly: `about 12 min left`, `about 1 h 05 min left`, and
/// `less than a minute left` below a minute. **Never more precise than a minute**: the
/// figure moves, and seconds that jump are noise.
[[nodiscard]] std::string remainingTimeText(std::chrono::seconds left);

/// Where a transcription stands, in the pieces a line or a box is made of, each
/// present only when it is known: the step or the percent, the position in what is being
/// transcribed, and the time left.
[[nodiscard]] std::vector<std::string>
transcriptionStatusParts(const TranscriptionProgress& progress,
                         std::optional<std::chrono::seconds> left,
                         std::optional<std::chrono::seconds> elapsed = std::nullopt);

/// How long the work has been going, `elapsed: 0:05:12`, for the surfaces that show it.
[[nodiscard]] std::string elapsedTimeText(std::chrono::seconds elapsed);

/// The last text the engine obtained, for the line that shows it.
[[nodiscard]] std::string lastTextLine(std::string_view text);

/// What there is of what the transcription needs, one line per tool: the engine (and where it
/// was found), `ffmpeg`, and the model (and what kind of model it is).
///
/// What a person looks at before starting, and in the settings where they put right what is
/// missing; the pointer to the guide is not in it, so that a caller adds it only when something
/// is missing.
[[nodiscard]] std::string transcriptionToolsReport(const TranscriptionTools& tools,
                                                   const TranscriptionSettings& settings);

/// Where to read how to install what the transcription needs.
[[nodiscard]] std::string installationGuidePointer();

} // namespace subedit::core
