#pragma once

// `transcribe`: the audio of a video or of an audio file, written as a subtitle file.

#include <subedit/cli/exit_code.hpp>
#include <subedit/core/config/transcription_settings.hpp>
#include <subedit/core/transcription/engine_registry.hpp>

#include <optional>
#include <string>
#include <string_view>

namespace subedit::core {
class FileSystem;
}

namespace subedit::cli {

class Reporter;

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
/// `searchPath` is where `ffmpeg` and `ffprobe` are looked for, a parameter for the
/// reason `findExecutable` gives.
[[nodiscard]] ExitCode transcribeFile(subedit::core::FileSystem& files,
                                      const subedit::core::EngineRegistry& engines,
                                      std::string_view searchPath,
                                      const TranscribeOptions& options,
                                      const subedit::core::TranscriptionSettings& settings,
                                      const Reporter& reporter);

} // namespace subedit::cli
