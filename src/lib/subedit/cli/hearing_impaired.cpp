#include <subedit/cli/hearing_impaired.hpp>
#include <subedit/cli/rewriting.hpp>
#include <subedit/cli/wording.hpp>
#include <subedit/core/command/change.hpp>
#include <subedit/core/command/command.hpp>
#include <subedit/core/edit/hearing_impaired_removal.hpp>
#include <subedit/core/edit/session.hpp>
#include <subedit/core/model/document.hpp>
#include <subedit/core/model/project.hpp>

#include <cstddef>
#include <expected>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace subedit::cli {

namespace {

/// What the removal did, read from the command rather than counted again.
///
/// `describe()` already says it: one change of main text per subtitle
/// rewritten, one removal naming every subtitle taken away. Counting a second
/// time — by comparing the texts before and after, say — would give two answers
/// to one question, and the one shown would be the one no test compares.
struct Tally {
    std::size_t cleaned = 0;
    std::size_t removed = 0;
};

[[nodiscard]] Tally tallyOf(const core::Command& command) {
    Tally tally;
    for (const core::Change& change : command.describe()) {
        if (change.kind == core::ChangeKind::Removal)
            tally.removed += change.subtitles.count();
        else
            ++tally.cleaned;
    }
    return tally;
}

} // namespace

ExitCode removeHearingImpairedIn(core::FileSystem& files,
                                 const std::vector<std::string>& paths,
                                 const Destination& destination,
                                 const Reporter& reporter) {
    const Operation clean = [](core::Session& session) -> std::expected<std::string, std::string> {
        std::unique_ptr<core::Command> command =
            core::removeHearingImpaired(session.project(), core::Document::Main);
        if (!command)
            return std::string{"no mention to remove"};

        const Tally tally = tallyOf(*command);
        session.apply(std::move(command));

        return countOf(tally.cleaned, "subtitle") + " cleaned, " + std::to_string(tally.removed) +
               " removed";
    };

    return rewriteAll(files, paths, destination, reporter, "cleaned", clean);
}

} // namespace subedit::cli
