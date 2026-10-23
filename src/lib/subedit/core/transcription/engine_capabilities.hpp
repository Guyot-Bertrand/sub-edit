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

    /// Whether it tells who speaks.
    bool speakers = false;

    /// Whether it cannot run without a model.
    bool needsModel = true;

    /// Whether `language` can be transcribed.
    [[nodiscard]] bool supportsLanguage(std::string_view language) const;

    friend bool operator==(const EngineCapabilities&, const EngineCapabilities&) = default;
};

} // namespace subedit::core
