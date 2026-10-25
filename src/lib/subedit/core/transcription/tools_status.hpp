#pragma once

#include <subedit/core/config/transcription_settings.hpp>
#include <subedit/core/io/file_system.hpp>
#include <subedit/core/transcription/engine_capabilities.hpp>

#include <filesystem>
#include <optional>
#include <string_view>

namespace subedit::core {

/// What a model file turns out to be.
enum class ModelState {
    NotSet,       ///< no model is named
    NotFound,     ///< the file named is not there
    NotAModel,    ///< it is there, and does not begin as a model does
    EnglishOnly,  ///< a model that transcribes English alone, and detects nothing
    Multilingual, ///< a model that detects the language and transcribes about a hundred
};

/// What the transcription needs, and which of it is there.
///
/// The three things the program ships none of (phase 22, D11): the engine, a model, and the
/// tool that extracts the audio of a video. Looked at again each time it is asked: the person
/// installs one, comes back, and expects the window to know.
struct TranscriptionTools {
    /// The engine's program, nothing when it is not found or when the engine named is unknown.
    std::optional<std::filesystem::path> engine{};

    /// Whether `ffmpeg` is on the search path.
    bool extractor = false;

    ModelState model = ModelState::NotSet;

    /// What the engine can do with the model, when both are there and the engine says. The surfaces
    /// adapt to it: a control for what the engine cannot do is greyed.
    std::optional<EngineCapabilities> capabilities{};

    /// Whether the person can be offered the choice of not writing sounds as notes: the engine says
    /// it can, or says nothing yet (no model chosen).
    [[nodiscard]] bool suppressNotesAvailable() const {
        return !capabilities.has_value() || capabilities->suppressNotes;
    }

    /// Whether a transcription of a video can start: the three are there.
    [[nodiscard]] bool ready() const {
        return engine.has_value() && extractor &&
               (model == ModelState::EnglishOnly || model == ModelState::Multilingual);
    }

    friend bool operator==(const TranscriptionTools&, const TranscriptionTools&) = default;
};

/// Looks for the three things `settings` and `searchPath` name.
[[nodiscard]] TranscriptionTools inspectTranscriptionTools(const FileSystem& files,
                                                           std::string_view searchPath,
                                                           const TranscriptionSettings& settings);

} // namespace subedit::core
