#include <subedit/cli/hearing_impaired.hpp>
#include <subedit/cli/rewriting.hpp>
#include <subedit/core/command/change.hpp>
#include <subedit/core/command/command.hpp>
#include <subedit/core/edit/hearing_impaired_removal.hpp>
#include <subedit/core/edit/session.hpp>
#include <subedit/core/model/document.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/model/selection.hpp>
#include <subedit/core/wording/counts.hpp>

#include <cstddef>
#include <cstdint>
#include <expected>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace subedit::cli {

namespace {} // namespace

ExitCode removeHearingImpairedIn(core::FileSystem& files,
                                 const std::vector<std::string>& paths,
                                 const std::optional<core::Encoding>& reading,
                                 const Destination& destination,
                                 const Reporter& reporter) {
    const Operation clean = [](core::Session& session) -> OperationOutcome {
        // The whole file: the selection reached the core with the window, and
        // a command line has none.
        std::unique_ptr<core::Command> command = core::removeHearingImpaired(
            session.project(), core::Selection::all(session.project()), core::Document::Main);
        if (!command)
            return OperationResult{.sentence = core::noMentionToRemove(),
                                   .counts = {{"cleaned", 0}, {"removed", 0}}};

        const core::HearingImpairedTally tally = core::tallyOf(*command);
        session.apply(std::move(command));

        return OperationResult{.sentence =
                                   core::noticeOfMentionsRemoved(tally.cleaned, tally.removed),
                               .counts = {{"cleaned", static_cast<std::int64_t>(tally.cleaned)},
                                          {"removed", static_cast<std::int64_t>(tally.removed)}}};
    };

    return rewriteAll(files, paths, reading, destination, reporter, "cleaned", clean);
}

} // namespace subedit::cli
