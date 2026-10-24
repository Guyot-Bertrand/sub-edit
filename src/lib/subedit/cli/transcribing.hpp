#pragma once

// `transcribe`: the audio of a video or of an audio file, written as a subtitle file.

#include <subedit/cli/exit_code.hpp>
#include <subedit/core/config/transcription_settings.hpp>
#include <subedit/core/transcription/engine_registry.hpp>
#include <subedit/core/transcription/remaining_time.hpp>
#include <subedit/core/transcription/transcription_progress.hpp>

#include <chrono>
#include <functional>
#include <optional>
#include <string>
#include <string_view>

namespace subedit::core {
class FileSystem;
}

namespace subedit::cli {

class Reporter;

/// Says how the work goes, as it goes: **one line per percent and per step** at the second
/// level of detail, and each new text at the third. The time left is worked out from the
/// fractions it sees.
///
/// **And one line every ten seconds when nothing new has come**, with the time spent: the engine
/// reports by windows of thirty seconds, which on a large model can be longer than a person
/// will wait for a sign of life. The clock is given, so that a test fixes the time.
class ProgressNarration {
public:
    using Clock = core::RemainingTime::Clock;

    static constexpr std::chrono::seconds kHeartbeat{10};

    explicit ProgressNarration(
        const Reporter& reporter,
        std::function<Clock::time_point()> clock = [] { return Clock::now(); },
        std::chrono::seconds heartbeat = kHeartbeat);

    /// Something was learnt.
    void operator()(const core::TranscriptionProgress& progress);

    /// Nothing was: says the state again if the heartbeat is due.
    void idle();

private:
    void say(const core::TranscriptionProgress& progress, Clock::time_point now);

    const Reporter& m_reporter;
    std::function<Clock::time_point()> m_clock;
    std::chrono::seconds m_heartbeat;
    Clock::time_point m_started;
    Clock::time_point m_lastSaid;
    core::RemainingTime m_remaining;
    std::optional<core::TranscriptionProgress> m_last{};
    std::optional<std::pair<core::TranscriptionStage, int>> m_shown{};
    std::string m_lastText;
};

/// What `transcribe` was asked for, as the command line wrote it. Empty means
/// « the setting decides ».
struct TranscribeOptions {
    std::string input{};

    /// The subtitle file to write.
    std::string output{};

    /// The short name of a format, or empty for the one the extension of `output` says.
    std::string format{};

    /// The audio track of a video, counted from 0; the first when nothing is asked.
    std::optional<int> audioTrack{};

    std::string language{};
    std::string model{};
    std::string engine{};

    /// A file to keep the extracted audio in.
    std::string keepAudio{};

    /// The part of the film to transcribe, as typed (`2:00`, `1:02:03`, seconds); empty for
    /// no bound.
    std::string start{};
    std::string end{};

    /// Write the segments as the engine cut them, with no post-processing.
    bool raw = false;
};

/// Transcribes `options.input` and writes the subtitles to `options.output`.
///
/// **The tools are the user's to install and their absence breaks nothing**: a
/// missing engine, model or `ffmpeg` is said with the pointer to the guide, and
/// nothing is written. An audio file is still read without `ffmpeg`. A language
/// the model cannot transcribe is refused before any work starts.
///
/// `cancelled`, when given, is asked while the work goes on: a true stops it and fails the file.
///
/// `searchPath` is where `ffmpeg` and `ffprobe` are looked for, a parameter for the
/// reason `findExecutable` gives.
[[nodiscard]] ExitCode transcribeFile(subedit::core::FileSystem& files,
                                      const subedit::core::EngineRegistry& engines,
                                      std::string_view searchPath,
                                      const TranscribeOptions& options,
                                      const subedit::core::TranscriptionSettings& settings,
                                      const Reporter& reporter,
                                      const std::function<bool()>& cancelled = {});

} // namespace subedit::cli
