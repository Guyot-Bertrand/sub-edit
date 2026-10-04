#include <subedit/cli/hearing_impaired.hpp>
#include <subedit/cli/rewriting.hpp>
#include <subedit/core/command/change.hpp>
#include <subedit/core/command/command.hpp>
#include <subedit/core/edit/hearing_impaired_removal.hpp>
#include <subedit/core/edit/session.hpp>
#include <subedit/core/model/document.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/model/selection.hpp>
#include <subedit/core/model/subtitle.hpp>
#include <subedit/core/model/subtitle_index.hpp>
#include <subedit/core/wording/counts.hpp>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <iterator>
#include <memory>
#include <optional>
#include <set>
#include <string>
#include <utility>
#include <vector>

namespace subedit::cli {

ExitCode removeHearingImpairedIn(core::FileSystem& files,
                                 const std::vector<std::string>& paths,
                                 const std::optional<core::Encoding>& reading,
                                 const Destination& destination,
                                 const Reporter& reporter) {
    const ChangingOperation clean = [](core::Session& session,
                                       const Request& request) -> OperationOutcome {
        // The texts as they are, set aside only when someone reads the list.
        std::vector<std::string> before;
        if (request.changes)
            before = mainTextsOf(session.project());

        // What the loop hands over: the whole file, as this subcommand takes no
        // `--range` yet.
        std::unique_ptr<core::Command> command =
            core::removeHearingImpaired(session.project(), request.selection, core::Document::Main);
        if (!command)
            return OperationResult{.sentence = core::noMentionToRemove(),
                                   .counts = {{"cleaned", 0}, {"removed", 0}},
                                   .changes = request.changes
                                                  ? std::optional{std::vector<TextChange>{}}
                                                  : std::nullopt};

        const core::HearingImpairedTally tally = core::tallyOf(*command);
        const std::vector<core::Change> described = command->describe();
        session.apply(std::move(command));

        return OperationResult{
            .sentence = core::noticeOfMentionsRemoved(tally.cleaned, tally.removed),
            .counts = {{"cleaned", static_cast<std::int64_t>(tally.cleaned)},
                       {"removed", static_cast<std::int64_t>(tally.removed)}},
            .changes = request.changes
                           ? std::optional{changesOfCommand(session.project(), before, described)}
                           : std::nullopt};
    };

    return rewriteAll(files, paths, reading, destination, reporter, "cleaned", clean);
}

} // namespace subedit::cli
