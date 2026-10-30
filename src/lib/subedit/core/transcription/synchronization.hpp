#pragma once

#include <subedit/core/command/command.hpp>
#include <subedit/core/model/selection.hpp>
#include <subedit/core/time/timestamp.hpp>
#include <subedit/core/transcription/alignment_doubt.hpp>
#include <subedit/core/transcription/alignment_retiming.hpp>
#include <subedit/core/transcription/word_matching.hpp>

#include <cstddef>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <vector>

namespace subedit::core {

class Project;

/// The subtitles to look for in the audio, and their texts: the numbers of the subtitles (from 0)
/// and, side by side, the lines the matching is given.
struct LinesToSynchronize {
    std::vector<std::size_t> subtitles;
    std::vector<std::string> lines;
};

/// Those subtitles of `project` that begin between `from` and `to`, either bound open.
[[nodiscard]] LinesToSynchronize subtitlesBeginningIn(const Project& project,
                                                      const std::optional<Timestamp>& from,
                                                      const std::optional<Timestamp>& to);

/// The subtitles of `selection`.
[[nodiscard]] LinesToSynchronize subtitlesOf(const Project& project, const Selection& selection);

/// What a matching comes to for a file: how each subtitle came out, and the times each ends up
/// with — the single account both surfaces show and apply.
struct SynchronizationPlan {
    /// The subtitles that were looked for, in file order, as numbers from 0.
    std::vector<std::size_t> subtitles;

    AlignmentReport report;

    /// One entry per subtitle looked for, in the same order.
    std::vector<RetimedLine> retimed;

    /// The times the subtitles had before.
    std::vector<LineTimes> originals;

    /// Whether the subtitle looked for as number `line` of the plan ends up with other times than
    /// it had.
    [[nodiscard]] bool changes(std::size_t line) const {
        return retimed[line].times != originals[line];
    }

    /// How many subtitles end up with other times.
    [[nodiscard]] std::size_t changed() const;
};

/// Judges `matches` (one per subtitle looked for), and works out the times each subtitle takes.
[[nodiscard]] SynchronizationPlan planSynchronization(const Project& project,
                                                      std::vector<std::size_t> subtitles,
                                                      std::span<const LineMatch> matches,
                                                      const DoubtLimits& limits);

/// The command that puts the plan's times on the project, as one entry of the history; nothing
/// when no subtitle moves.
[[nodiscard]] std::unique_ptr<Command> synchronizationCommand(const Project& project,
                                                              const SynchronizationPlan& plan);

} // namespace subedit::core
