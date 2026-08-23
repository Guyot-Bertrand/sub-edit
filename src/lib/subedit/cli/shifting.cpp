#include <subedit/cli/rewriting.hpp>
#include <subedit/cli/shifting.hpp>
#include <subedit/core/edit/session.hpp>
#include <subedit/core/edit/shift_command.hpp>
#include <subedit/core/edit/shift_limits.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/model/selection.hpp>
#include <subedit/core/wording.hpp>

#include <cstddef>
#include <expected>
#include <memory>
#include <optional>
#include <span>
#include <string>

namespace subedit::cli {

ExitCode shiftAll(core::FileSystem& files,
                  const std::vector<std::string>& paths,
                  core::Duration by,
                  const Destination& destination,
                  const Reporter& reporter) {
    const Operation shift =
        [by](core::Session& session) -> std::expected<std::string, std::string> {
        const core::Selection whole = core::Selection::all(session.project());

        // The rule has lived in the core since issue #132: the window asks for
        // it too, and two copies of one rule drift apart.
        if (const std::optional<core::SubtitleIndex> refused =
                core::firstBeforeOrigin(session.project(), whole, by);
            refused.has_value()) {
            return std::unexpected{"subtitle " + std::to_string(refused->number()) +
                                   " would start before the origin, which no subtitle "
                                   "file can hold"};
        }

        session.apply(std::make_unique<core::ShiftCommand>(whole, by));
        return core::countOf(session.project().count(), "subtitle") + " shifted by " +
               core::secondsOf(by);
    };

    return rewriteAll(files, paths, destination, reporter, "shifted", shift);
}

} // namespace subedit::cli
