#pragma once

#include <subedit/core/command/command.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/model/subtitle_index.hpp>

#include <expected>
#include <memory>

namespace subedit::core {

/// What splitting a project made: the project that is born, and the command
/// that takes its subtitles out of the one they came from — decision D6 of the
/// phase-11 spec.
struct SplitProject {
    /// One command, one entry in the history of the origin: it removes the
    /// tail. The new project has a history of its own, and none of this.
    std::unique_ptr<Command> command{};

    /// The tail, ready to open in a project of its own.
    Project tail{};
};

/// Why a cut was refused.
struct SplitRefusal {
    /// The first subtitle, in the project that was to be cut, that the shift
    /// would carry before the start of the video.
    SubtitleIndex before;
};

/// Plans cutting `project` at `from`, the first subtitle of the tail.
///
/// **The exact inverse of `appendFile`**: the tail is shifted back by the end of
/// the last subtitle that stays, so that splitting and then appending gives the
/// project back. Both of its texts follow their subtitles, and it inherits the
/// format, the encoding and the frame rate — **and no path**, since no file
/// holds it yet. A translation the project has is inherited the same way.
///
/// **Refused when no file could write the result.** If the two halves overlap,
/// the shift lands a subtitle before the origin, which is representable and
/// which no file can hold — the rule of `firstBeforeOrigin`, and its subtitle
/// is named.
///
/// Throws `std::out_of_range` when `from` is not a cut: the first subtitle
/// cannot begin a tail, since nothing would stay, and one past the last would
/// leave an empty tail.
[[nodiscard]] std::expected<SplitProject, SplitRefusal> splitProject(const Project& project,
                                                                     SubtitleIndex from);

} // namespace subedit::core
