#pragma once

#include <subedit/core/io/file_system.hpp>
#include <subedit/core/process/start_process.hpp>

#include <expected>
#include <filesystem>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace subedit::core {

/// One audio track of a container, as the file declares it.
///
/// **The position means nothing outside the file**: the first audio track of
/// one film is not the first of another. Language and title are whatever the
/// container wrote, empty when it wrote nothing.
struct ContainerAudioTrack {
    /// Counted among the audio tracks only, from 0 — the `N` of `-map 0:a:N`.
    int position = 0;

    /// An ISO 639 code as the file spells it (`fra`, `eng`), or empty.
    std::string language{};

    std::string title{};

    friend bool operator==(const ContainerAudioTrack&, const ContainerAudioTrack&) = default;
};

/// Reads what `ffprobe -show_entries stream=index:stream_tags=language,title`
/// writes: one `[STREAM]` block per audio track, with the tags it found.
///
/// The default output and not a CSV: a title can contain a comma.
[[nodiscard]] std::vector<ContainerAudioTrack> parseAudioTracks(std::string_view probeOutput);

/// The audio tracks of `video`, in order — empty for a file that has none.
///
/// **Nothing means the answer is not known**, and is an ordinary one: `ffprobe`
/// is not on `searchPath` (the ordinary state of a machine that never installed
/// `ffmpeg`), or the file is not a container. The window has the list from its
/// player already; this is the command line's.
[[nodiscard]] std::optional<std::vector<ContainerAudioTrack>> readAudioTracks(
    const FileSystem& files, std::string_view searchPath, const std::filesystem::path& video);

enum class AudioPreparationErrorKind {
    ToolMissing, ///< `ffmpeg` is not on the search path
    NoSuchTrack, ///< the container has fewer audio tracks than asked for
    Failed,      ///< `ffmpeg` could not extract it
    Cancelled,   ///< the caller asked to stop
};

struct AudioPreparationError {
    AudioPreparationErrorKind kind;

    /// What the tool said, or the track asked for. Free, may be empty.
    std::string detail{};

    friend bool operator==(const AudioPreparationError&, const AudioPreparationError&) = default;
};

/// What the normalised audio is: mono, 16-bit PCM in a WAV file, at the rate the engine asks for
/// (`EngineCapabilities::sampleRate`; 16 kHz for whisper.cpp, which reads it without converting) —
/// and `src/scripts/video-fixtures.sh` holds the same command against real files (#771).
///
/// Extracts the audio track `track` of `source` (the first when nothing is
/// asked) to `destination` at `sampleRate` hertz (the rate it has when 0), converting an
/// audio file the same way. **A partial
/// file is removed** when the run fails or is stopped.
///
/// `ffmpeg` is the external tool of the roadmap's rule: absent, this says so and
/// nothing else breaks — an audio file already in the right format can still be
/// handed to an engine directly.
[[nodiscard]] std::expected<void, AudioPreparationError>
extractAudio(const FileSystem& files,
             std::string_view searchPath,
             const std::filesystem::path& source,
             std::optional<int> track,
             const std::filesystem::path& destination,
             int sampleRate,
             const StreamObserver& observer = {});

/// The name of the file to keep the audio in, with the wildcards of `pattern` filled in from what
/// shaped the file: `{rate}` becomes the sampling rate in hertz, or `source` when the audio keeps
/// its own. A name without a wildcard is returned as it stands; the wildcards are listed in the
/// manual with `--keep-audio`.
[[nodiscard]] std::string expandAudioName(std::string_view pattern, int sampleRate);

} // namespace subedit::core
