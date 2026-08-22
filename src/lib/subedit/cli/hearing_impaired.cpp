#include <subedit/cli/hearing_impaired.hpp>
#include <subedit/cli/rewriting.hpp>
#include <subedit/core/command/change.hpp>
#include <subedit/core/command/command.hpp>
#include <subedit/core/edit/hearing_impaired_removal.hpp>
#include <subedit/core/edit/session.hpp>
#include <subedit/core/model/document.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/model/selection.hpp>
#include <subedit/core/wording.hpp>

#include <cstddef>
#include <expected>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace subedit::cli {

namespace {} // namespace

ExitCode removeHearingImpairedIn(core::FileSystem& files,
                                 const std::vector<std::string>& paths,
                                 const Destination& destination,
                                 const Reporter& reporter) {
    const Operation clean = [](core::Session& session) -> std::expected<std::string, std::string> {
        // Le fichier entier : la sélection est arrivée au noyau avec la
        // fenêtre, et une ligne de commande n'en a pas.
        std::unique_ptr<core::Command> command = core::removeHearingImpaired(
            session.project(), core::Selection::all(session.project()), core::Document::Main);
        if (!command)
            return std::string{"no mention to remove"};

        const core::HearingImpairedTally tally = core::tallyOf(*command);
        session.apply(std::move(command));

        return core::countOf(tally.cleaned, "subtitle") + " cleaned, " +
               std::to_string(tally.removed) + " removed";
    };

    return rewriteAll(files, paths, destination, reporter, "cleaned", clean);
}

} // namespace subedit::cli
