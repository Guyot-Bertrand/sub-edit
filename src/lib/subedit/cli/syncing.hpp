#pragma once

// `sync`: re-time the subtitles of a file on the audio of a video or an audio file.

#include <subedit/cli/exit_code.hpp>
#include <subedit/core/config/alignment_settings.hpp>
#include <subedit/core/config/transcription_settings.hpp>
#include <subedit/core/model/encoding.hpp>
#include <subedit/core/transcription/engine_registry.hpp>

#include <cstdint>
#include <functional>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace subedit::core {
class FileSystem;
}

namespace subedit::cli {

class Destination;
class Reporter;

/// What `sync` was asked for, as the command line wrote it. Empty means « the setting decides ».
struct SyncOptions {
    /// The film, or an audio file: exactly one of the two is given.
    std::string video{};
    std::string audio{};

    /// The audio track of a video, counted from 0; the first when nothing is asked.
    std::optional<int> audioTrack{};

    std::string language{};
    std::string model{};
    std::string vadModel{};
    std::string engine{};

    /// A file to keep the extracted audio in.
    std::string keepAudio{};

    /// The part of the film to listen to, as typed (`2:00`, `1:02:03`, seconds); empty for no
    /// bound. Only the subtitles that begin in it are touched.
    std::string start{};
    std::string end{};

    /// Over `alignment.min-match` (percent) and `alignment.doubt-gap` (milliseconds).
    std::optional<int> minimumMatchPercent{};
    std::optional<std::int64_t> doubtGapMilliseconds{};

    /// A file to write the lines to check to, one per line; none when empty.
    std::string report{};
};

/// Re-times every file of `paths` on the audio, and writes each back as it was found.
///
/// **Only starts and ends change**, and a subtitle the audio does not give keeps its times: the
/// run says which, and succeeds. A file none of whose subtitles could be found is refused and not
/// written — the audio or the language is almost certainly the wrong one.
///
/// `cancelled`, when given, is asked while the work goes on: a true stops it.
[[nodiscard]] ExitCode syncAll(subedit::core::FileSystem& files,
                               const subedit::core::EngineRegistry& engines,
                               std::string_view searchPath,
                               const std::vector<std::string>& paths,
                               const std::optional<subedit::core::Encoding>& reading,
                               const SyncOptions& options,
                               const subedit::core::TranscriptionSettings& transcription,
                               const subedit::core::AlignmentSettings& alignment,
                               const Destination& destination,
                               const Reporter& reporter,
                               const std::function<bool()>& cancelled = {});

} // namespace subedit::cli
