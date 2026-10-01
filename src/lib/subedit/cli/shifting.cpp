#include <subedit/cli/rewriting.hpp>
#include <subedit/cli/shifting.hpp>
#include <subedit/core/analysis/frame_rate_deduction.hpp>
#include <subedit/core/analysis/grid_correction.hpp>
#include <subedit/core/edit/session.hpp>
#include <subedit/core/edit/shift_command.hpp>
#include <subedit/core/edit/shift_limits.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/model/selection.hpp>
#include <subedit/core/model/subtitle_index.hpp>
#include <subedit/core/wording/analysis.hpp>
#include <subedit/core/wording/counts.hpp>

#include <cstddef>
#include <cstdint>
#include <expected>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace subedit::cli {

ExitCode shiftAll(core::FileSystem& files,
                  const std::vector<std::string>& paths,
                  const std::optional<core::Encoding>& reading,
                  core::Duration by,
                  const Destination& destination,
                  const Reporter& reporter) {
    const Operation shift = [by](core::Session& session) -> OperationOutcome {
        const core::Selection whole = core::Selection::all(session.project());

        // The rule has lived in the core since issue #132: the window asks for
        // it too, and two copies of one rule drift apart.
        if (const std::optional<core::SubtitleIndex> refused =
                core::firstBeforeOrigin(session.project(), whole, by);
            refused.has_value()) {
            return std::unexpected{
                Failure{"before-the-origin", core::shiftBeforeTheOrigin(refused->number())}};
        }

        session.apply(std::make_unique<core::ShiftCommand>(whole, by));
        const std::size_t count = session.project().count();
        return OperationResult{.sentence = core::countOf(count, "subtitle") + " shifted by " +
                                           core::secondsOf(by),
                               .counts = {{"subtitles", static_cast<std::int64_t>(count)},
                                          {"shifted_by_ms", by.milliseconds()}}};
    };

    return rewriteAll(files, paths, reading, destination, reporter, "shifted", shift);
}

ExitCode shiftOntoGridAll(core::FileSystem& files,
                          const std::vector<std::string>& paths,
                          const std::optional<core::Encoding>& reading,
                          const Destination& destination,
                          const Reporter& reporter) {
    const Operation onto = [](core::Session& session) -> OperationOutcome {
        const core::FrameRateDeduction grid = core::deduceFrameRate(session.project());
        const std::optional<core::Duration> by = core::shiftOntoGrid(grid);
        if (!by.has_value()) {
            return std::unexpected{
                Failure{"no-grid",
                        "no frame rate grid was found in these positions, so there is "
                        "nothing to bring them back onto"}};
        }

        const core::Selection whole = core::Selection::all(session.project());
        if (const std::optional<core::SubtitleIndex> refused =
                core::firstBeforeOrigin(session.project(), whole, *by);
            refused.has_value()) {
            return std::unexpected{
                Failure{"before-the-origin", core::shiftBeforeTheOrigin(refused->number())}};
        }

        session.apply(std::make_unique<core::ShiftCommand>(whole, *by));
        const std::size_t count = session.project().count();
        return OperationResult{.sentence = core::countOf(count, "subtitle") + " shifted by " +
                                           core::secondsOf(*by) + " onto their " +
                                           nameOf(grid.retained.rate) + " fps grid",
                               .counts = {{"subtitles", static_cast<std::int64_t>(count)},
                                          {"shifted_by_ms", by->milliseconds()}}};
    };

    return rewriteAll(files, paths, reading, destination, reporter, "shifted", onto);
}

} // namespace subedit::cli
