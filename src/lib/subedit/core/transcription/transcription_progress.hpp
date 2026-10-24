#pragma once

#include <subedit/core/time/timestamp.hpp>

#include <optional>
#include <string>

namespace subedit::core {

/// The two steps of a transcription, in the order they run.
enum class TranscriptionStage {
    /// The audio of a video is being extracted. The tool reports nothing of how far.
    Extracting,

    /// The engine is working.
    Transcribing,
};

/// Where a transcription stands, as far as anyone can tell.
///
/// **Every field but the stage is optional**, because what is known depends on the step
/// and on the engine: the extraction says nothing of how far it is, and an engine that
/// works by windows says nothing until the first one is done. A surface shows what is
/// there and is silent about the rest, rather than inventing a figure.
struct TranscriptionProgress {
    TranscriptionStage stage = TranscriptionStage::Transcribing;

    /// How much of the stage is done, from 0 to 1, never going back.
    std::optional<double> fraction{};

    /// The end of the last piece of speech obtained, in the time of the film.
    std::optional<Timestamp> position{};

    /// The end of what is being transcribed: that of the range, or of the audio.
    std::optional<Timestamp> total{};

    /// The last text obtained, as the engine wrote it.
    std::string lastText{};
};

} // namespace subedit::core
