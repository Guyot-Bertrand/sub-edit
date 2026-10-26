#pragma once

#include <string>
#include <string_view>
#include <vector>

namespace subedit::core {

/// What an engine can do **with a given model**.
///
/// Declared for the pair and not for the engine alone: an English-only model
/// has no language detection and one language, a multilingual one has both,
/// and they are the same engine. The surfaces adapt to the declaration instead
/// of assuming — they offer the languages it lists, and ask for one when it
/// cannot detect.
struct EngineCapabilities {
    /// Whether the segments carry timed words.
    bool timedWords = false;

    /// Whether the engine can find the language of the speech by itself.
    bool languageDetection = false;

    /// The ISO 639-1 codes it can transcribe. **Empty means any**: a
    /// multilingual model does not list its hundred languages.
    std::vector<std::string> languages{};

    /// Whether it can be told not to write sounds as notes (`TranscriptionRequest::suppressNotes`).
    /// Greyed in the window, and refused when asked for on the command line, for an engine that
    /// cannot.
    bool suppressNotes = false;

    /// Whether it can skip the silences with a voice-activity model
    /// (`TranscriptionRequest::vadModel`). Refused on the command line for an engine that cannot,
    /// when asked for by name; a setting is dropped without a word.
    bool voiceActivity = false;

    /// The sampling rate, in hertz, of the audio the engine wants: whisper.cpp reads 16 kHz. **The
    /// rate is the engine's need, not the program's**: the extraction produces what the engine asks
    /// for. 0 means no preference, and the audio keeps the rate it has.
    int sampleRate = 0;

    /// Whether it tells who speaks.
    bool speakers = false;

    /// Whether it cannot run without a model.
    bool needsModel = true;

    /// Whether `language` can be transcribed.
    [[nodiscard]] bool supportsLanguage(std::string_view language) const;

    friend bool operator==(const EngineCapabilities&, const EngineCapabilities&) = default;
};

} // namespace subedit::core
