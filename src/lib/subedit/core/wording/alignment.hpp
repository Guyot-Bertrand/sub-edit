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
