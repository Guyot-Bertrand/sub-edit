#pragma once

#include <subedit/core/io/file_system.hpp>
#include <subedit/core/transcription/audio_source.hpp>
#include <subedit/core/transcription/transcription_engine.hpp>

#include <expected>
#include <filesystem>
#include <functional>
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

    /// See `TranscriptionRequest::suppressNotes`.
    bool suppressNotes = true;

    /// See `TranscriptionRequest::vadModel`.
    std::optional<std::filesystem::path> vadModel{};

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

    /// What the engine had obtained when it stopped (`TranscriptionError::partial`): empty when the
    /// audio could not be prepared, or the engine had written nothing.
    std::vector<TranscribedSegment> partial{};
};

/// Prepares the audio of `request.input` and hands it to `use`: **the road both transcription and
/// alignment take**, so that the extraction, its refusals and its fallbacks are one.
///
/// `capabilities` are those of whatever will listen: the audio is extracted at the rate it asks
/// for, and a request it cannot honour is refused before the extraction. The audio is removed once
/// `use` returns, unless `request.keepAudio` names a file.
[[nodiscard]] std::expected<void, MediaTranscriptionError>
withPreparedAudio(FileSystem& files,
                  const EngineCapabilities& capabilities,
                  std::string_view searchPath,
                  const MediaTranscriptionRequest& request,
                  const TranscriptionObserver& observer,
                  const std::function<std::expected<void, TranscriptionError>(
                      const std::filesystem::path& audio)>& use);

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
