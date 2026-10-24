#pragma once

#include <subedit/core/io/file_system.hpp>
#include <subedit/core/transcription/audio_source.hpp>
#include <subedit/core/transcription/transcription_engine.hpp>

#include <expected>
#include <filesystem>
#include <optional>
#include <string_view>
#include <variant>
#include <vector>

namespace subedit::core {

/// What a transcription of a video or an audio file is asked to do, once the caller
/// has chosen the engine.
struct MediaTranscriptionRequest {
    /// A video, or an audio file.
    std::filesystem::path input;

    /// The audio track of a video, counted from 0; the first when nothing is asked.
    std::optional<int> audioTrack{};

    /// A file to keep the extracted audio in; a temporary one, removed, when empty.
    std::filesystem::path keepAudio{};

    std::optional<std::string> language{};
    std::optional<std::filesystem::path> model{};

    /// The part of the film to transcribe, in its own time; all of it when not given.
    std::optional<Timestamp> start{};
    std::optional<Timestamp> end{};
};

/// Why it did not come to anything: the audio could not be prepared, or the engine
/// failed. Words for either are in `core/wording/transcription`.
struct MediaTranscriptionError {
    std::variant<AudioPreparationError, TranscriptionError> cause;

    /// The audio tracks the video has, when a track that is not there was asked for.
    std::vector<ContainerAudioTrack> tracks{};
};

/// Prepares the audio of `request.input`, and has `engine` transcribe it.
///
/// **The one road both surfaces take**, so that they cannot differ: the command line
/// writes what it returns, the window replaces the project's subtitles with it.
///
/// The tools are the user's to install and their absence breaks nothing: without
/// `ffmpeg`, an audio file the engine reads as it is (`wav`, `mp3`, `flac`, `ogg`) is
/// still transcribed, when no track is asked for; a video is not, and the error says so.
///
/// `observer` is called from the thread this runs on, as for the engine itself; its
/// cancellation also stops the extraction.
[[nodiscard]] std::expected<Transcription, MediaTranscriptionError>
transcribeMedia(FileSystem& files,
                const TranscriptionEngine& engine,
                std::string_view searchPath,
                const MediaTranscriptionRequest& request,
                const TranscriptionObserver& observer = {});

} // namespace subedit::core
