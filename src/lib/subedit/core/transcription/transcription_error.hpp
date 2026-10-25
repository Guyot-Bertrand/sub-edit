#pragma once

#include <subedit/core/transcription/transcript.hpp>

#include <string>
#include <vector>

namespace subedit::core {

/// Why a transcription did not happen.
///
/// **A kind, unlike `PlayerError`**, because callers act on it: a missing model
/// sends the user to the installation page, an unsupported language sends them
/// to the choice of model, a cancellation is not reported as a failure at all.
enum class TranscriptionErrorKind {
    EngineMissing,       ///< the program (or library) behind the engine is not there
    ModelMissing,        ///< the engine needs a model and none was given, or it is not at that path
    UnsupportedLanguage, ///< the engine, with that model, cannot transcribe the language asked for
    LanguageRequired, ///< no language was given and the engine, with that model, cannot detect one
    AudioUnreadable,  ///< the audio file is missing or is not audio
    Failed,           ///< anything else the engine reported
    Cancelled,        ///< the caller asked to stop
};

struct TranscriptionError {
    TranscriptionErrorKind kind;

    /// What the engine said, or what was asked. English, free, may be empty.
    /// The words the user reads come from `core/wording/`, not from here.
    std::string detail{};

    /// **What had been obtained when the work stopped**, a cancellation or a failure: the pieces
    /// the engine had written, in order, without timed words (an engine gives those at the end).
    /// Empty when it had written none, or cannot say. A long run stopped at 80 % should not throw
    /// 80 % of the work away without being asked: the surfaces show it and let the person keep it.
    std::vector<TranscribedSegment> partial{};

    friend bool operator==(const TranscriptionError&, const TranscriptionError&) = default;
};

} // namespace subedit::core
