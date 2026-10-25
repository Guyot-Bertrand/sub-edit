#pragma once

#include <subedit/core/transcription/assembly_limits.hpp>

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <string>
#include <string_view>

namespace subedit::core {

/// The name of the engine used when the setting names none.
inline constexpr std::string_view kDefaultTranscriptionEngine = "whisper.cpp";

/// What the audio transcription retains — phase 22, D8 of its spec.
///
/// **Paths are settings and never constants**: the program ships no engine and no
/// model, so where they are is the user's to say. Empty means the default —
/// the executable is searched for on `PATH`, the language is detected, and a
/// model must be given on the command line or here.
struct TranscriptionSettings {
    /// The engine, as the registry knows it.
    std::string engine{kDefaultTranscriptionEngine};

    /// The engine's executable; empty to search for it on `PATH`.
    std::filesystem::path executable{};

    /// The model file; empty for none.
    std::filesystem::path model{};

    /// The voice-activity-detection model (whisper.cpp: a Silero model); empty for none. With one,
    /// the engine skips the silences before it decodes, and its segments follow the speech: the
    /// first one begins where the speech does, and not at 0. Provided by the user, as the model is.
    std::filesystem::path vadModel{};

    /// An ISO 639-1 code (`en`, `fr`); empty to let the engine detect it.
    std::string language{};

    /// Whether the engine is told not to write sounds as notes — `(Sonnerie de téléphone)`,
    /// `[MUSIC PLAYING]`. On by default: such notes are noise in most subtitles. Off for subtitles
    /// for the hard of hearing, which want them. An intent, not an engine's option: the adapter
    /// turns it into whatever the engine calls it, and an engine that cannot do it ignores it.
    bool suppressNotes = true;

    /// The limits of the post-processing (`AssemblySettings`).
    std::int64_t mergeGapMilliseconds = kDefaultMergeGapMilliseconds;
    std::int64_t maxDurationMilliseconds = kDefaultMaxDurationMilliseconds;
    int maxCharacters = static_cast<int>(kDefaultMaxCharacters);

    friend bool operator==(const TranscriptionSettings&, const TranscriptionSettings&) = default;
};

} // namespace subedit::core
