#include <subedit/cli/aligning.hpp>
#include <subedit/cli/rewriting.hpp>
#include <subedit/core/edit/session.hpp>
#include <subedit/core/edit/snap_command.hpp>
#include <subedit/core/i18n/messages.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/model/selection.hpp>
#include <subedit/core/model/subtitle.hpp>
#include <subedit/core/wording/analysis.hpp>
#include <subedit/core/wording/counts.hpp>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <memory>
#include <string>
#include <vector>

namespace subedit::cli {

namespace {

/// Both ends of every subtitle, in order, for comparing before with after.
[[nodiscard]] std::vector<std::int64_t> boundsOf(const core::Project& project) {
    std::vector<std::int64_t> bounds;
    bounds.reserve(project.count() * 2);
    for (const core::Subtitle& subtitle : project.subtitles()) {
        bounds.push_back(subtitle.start.milliseconds());
        bounds.push_back(subtitle.end.milliseconds());
    }
    return bounds;
}

} // namespace

ExitCode alignAll(core::FileSystem& files,
                  const std::vector<std::string>& paths,
                  const std::optional<core::Encoding>& reading,
                  core::FrameRate rate,
                  const Destination& destination,
                  const Reporter& reporter) {
    // No refusal of its own: any rate the grammar accepted defines a grid, and
    // every position has a nearest frame on it. Like the conversion, this is an
    // operation that cannot fail on the contents of a file.
    const Operation align = [rate](core::Session& session) -> OperationOutcome {
        const std::vector<std::int64_t> before = boundsOf(session.project());

        session.apply(std::make_unique<core::SnapCommand>(
            session.project(), core::Selection::all(session.project()), rate));

        const std::vector<std::int64_t> after = boundsOf(session.project());
        std::size_t moved = 0;
        std::int64_t furthest = 0;
        for (std::size_t rank = 0; rank < before.size(); ++rank) {
            const std::int64_t away = std::abs(after[rank] - before[rank]);
            if (away != 0)
                ++moved;
            furthest = std::max(furthest, away);
        }

        // **The two numbers are the point of the line.** A user who meant
        // `framerate` and typed this reads « moved by at most 20 ms » where
        // they expected seconds, and knows at once.
        const std::size_t total = session.project().count();
        const std::string movedClause =
            // TRANSLATORS: %1 positions moved, by at most %2 milliseconds.
            core::substitute(core::translatePlural("%1 position moved, by at most %2 ms",
                                                   "%1 positions moved, by at most %2 ms",
                                                   moved),
                             {std::to_string(moved), std::to_string(furthest)});
        return OperationResult{.sentence = core::substitute(
                                   // TRANSLATORS: %1 subtitles aligned on the frame rate %2; %3 is
                                   // « 4 positions moved, by at most 20 ms ».
                                   core::translatePlural("%1 subtitle aligned on %2 fps, %3",
                                                         "%1 subtitles aligned on %2 fps, %3",
                                                         total),
                                   {std::to_string(total), nameOf(rate), movedClause}),
                               .counts = {{"subtitles", static_cast<std::int64_t>(total)},
                                          {"moved", static_cast<std::int64_t>(moved)},
                                          {"furthest_ms", furthest}}};
    };

    return rewriteAll(files, paths, reading, destination, reporter, Verb::Aligned, align);
}

} // namespace subedit::cli
