#include <subedit/cli/rewriting.hpp>
#include <subedit/cli/sorting.hpp>
#include <subedit/core/command/change.hpp>
#include <subedit/core/edit/session.hpp>
#include <subedit/core/edit/sort_command.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/wording/counts.hpp>

#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>

namespace subedit::cli {

std::size_t sortedIn(core::Session& session) {
    // Read from what the command reports, never counted again after.
    std::size_t moved = 0;
    for (const core::Change& change : session.apply(std::make_unique<core::SortCommand>())) {
        if (change.kind == core::ChangeKind::Reordering)
            moved += change.subtitles.count();
    }
    return moved;
}

std::size_t sortedInPlace(core::Project& project) {
    core::SortCommand command;
    command.apply(project);

    std::size_t moved = 0;
    for (const core::Change& change : command.describe())
        moved += change.subtitles.count();
    return moved;
}

std::string narrationOfSort(const std::string& path, std::size_t moved) {
    return path + ": " + core::noticeOfSort(moved);
}

ExitCode sortAll(core::FileSystem& files,
                 const std::vector<std::string>& paths,
                 const std::optional<core::Encoding>& reading,
                 const Destination& destination,
                 const Reporter& reporter) {
    const Operation sort = [](core::Session& session) -> OperationOutcome {
        const std::size_t total = session.project().count();

        const std::size_t moved = sortedIn(session);

        return OperationResult{.sentence = core::noticeOfSort(moved),
                               .counts = {{"subtitles", static_cast<std::int64_t>(total)},
                                          {"moved", static_cast<std::int64_t>(moved)}}};
    };

    return rewriteAll(files, paths, reading, destination, reporter, "sorted", sort);
}

} // namespace subedit::cli
