#pragma once

// What both surfaces say of an automatic alignment — what it came to, which lines to check and
// why, and why it did not happen. Shared wording of `core/wording/`; see `formats.hpp` for why it
// lives in the core. Phase 23, #813.

#include <subedit/core/transcription/alignment_doubt.hpp>

#include <string>
#include <string_view>

namespace subedit::core {

/// There is no alignment engine of that name.
[[nodiscard]] std::string noSuchAlignmentEngine(std::string_view name);

/// `sync` was given neither a video nor an audio file to listen to, or both.
[[nodiscard]] std::string needsOneAudioSource();

/// A file whose lines were all looked for in the audio and none was found: the wrong audio, or the
/// wrong language, far more often than a file nobody says.
[[nodiscard]] std::string noLineFoundInTheAudio(std::size_t lines);

/// Finding where there is speech needs a voice-activity model, and none is named.
[[nodiscard]] std::string speechNeedsTheVoiceModel();

/// Finding where there is speech needs the program of whisper.cpp that does it, and it is not
/// there.
[[nodiscard]] std::string speechNeedsTheVoiceTool();

/// The sound and the file have too little in common for the file to be put in line with it: the
/// wrong film, or one with hardly any speech.
[[nodiscard]] std::string noAgreementWithTheSpeech();

/// What must not be believed of a file put in line with the speech alone: only the file as a whole
/// was corrected. **Shown every time, by both surfaces**, so that nobody expects a line that is on
/// its own out of place to have been found.
[[nodiscard]] std::string speechOnlyWarning();

/// The frame rate of the file was put right: its times were divided by `ratio`.
[[nodiscard]] std::string frameRateCorrectedText(double ratio);

/// Said after a failure of the matching of words: another method does not depend on the language.
[[nodiscard]] std::string speechMethodIgnoresLanguage();

/// What an alignment came to: how many subtitles were placed, how many are to be checked, and the
/// figures that tell the story of the file.
[[nodiscard]] std::string noticeOfAlignment(const AlignmentReport& report);

/// Why a line is to be checked.
[[nodiscard]] std::string doubtText(LineDoubt doubt);

/// A line cut back to the start of the next, because it would have run into it.
[[nodiscard]] std::string lineCutBackText();

/// A line that still overlaps the next, which cutting cannot cure.
[[nodiscard]] std::string lineOverlapsText();

} // namespace subedit::core
