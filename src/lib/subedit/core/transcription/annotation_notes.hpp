#pragma once

#include <subedit/core/transcription/transcript.hpp>

#include <cstddef>
#include <span>
#include <string_view>

namespace subedit::core {

/// Whether `text` is nothing but notes in brackets — `[MUSIC PLAYING]`, `[SPEAKING FRENCH]`,
/// `(applause)` — one or several, separated by blanks.
///
/// What an engine writes in place of speech it did not take for speech, or in a language it was not
/// told to listen for: whisper decoding French as English writes `[SPEAKING FRENCH]` where the
/// words are.
[[nodiscard]] bool isNotesOnly(std::string_view text);

/// How many of the pieces of a transcription are notes only.
struct NotesShare {
    std::size_t notes = 0;
    std::size_t pieces = 0;
};

[[nodiscard]] NotesShare notesShare(std::span<const TranscribedSegment> segments);

/// Whether so much of the transcription is notes that the language used is probably wrong: at least
/// two, and a third of the pieces. A film with a few sung passages has a few notes and is not under
/// suspicion.
[[nodiscard]] bool looksLikeWrongLanguage(const NotesShare& share);

} // namespace subedit::core
