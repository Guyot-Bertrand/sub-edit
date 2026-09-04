#include <subedit/cli/aligning.hpp>
#include <subedit/cli/rewriting.hpp>
#include <subedit/core/edit/session.hpp>
#include <subedit/core/edit/snap_command.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/model/selection.hpp>
#include <subedit/core/model/subtitle.hpp>
#include <subedit/core/wording.hpp>

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
    const Operation align =
        [rate](core::Session& session) -> std::expected<std::string, std::string> {
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
        return core::countOf(session.project().count(), "subtitle") + " aligned on " +
               nameOf(rate) + " fps, " + core::countOf(moved, "position") + " moved, by at most " +
               std::to_string(furthest) + " ms";
    };

    return rewriteAll(files, paths, reading, destination, reporter, "aligned", align);
}

} // namespace subedit::cli
