#include <subedit/cli/rewriting.hpp>
#include <subedit/cli/shifting.hpp>
#include <subedit/cli/time_grammar.hpp>
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

namespace {

/// Shifts the whole file by `by`, or refuses when a subtitle would land before the
/// origin. `suffix` is what the sentence adds after the amount.
///
/// The rule has lived in the core since issue #132: the window asks for it too,
/// and two copies of one rule drift apart.
[[nodiscard]] OperationOutcome
shiftWhole(core::Session& session, core::Duration by, const std::string& suffix) {
    const core::Selection whole = core::Selection::all(session.project());
    if (const std::optional<core::SubtitleIndex> refused =
            core::firstBeforeOrigin(session.project(), whole, by);
        refused.has_value()) {
        return std::unexpected{
            Failure{"before-the-origin", core::shiftBeforeTheOrigin(refused->number())}};
    }

    session.apply(std::make_unique<core::ShiftCommand>(whole, by));
    const std::size_t count = session.project().count();
    return OperationResult{.sentence = core::countOf(count, "subtitle") + " shifted by " +
                                       core::secondsOf(by) + suffix,
                           .counts = {{"subtitles", static_cast<std::int64_t>(count)},
                                      {"shifted_by_ms", by.milliseconds()}}};
}

} // namespace

std::expected<std::optional<core::Duration>, std::string> shiftAmountOf(bool toGrid,
                                                                        const std::string& by) {
    if (toGrid && !by.empty()) {
        return std::unexpected{
            std::string{"--by and --to-grid both say by how much to move; give one or the other"}};
    }
    // Measured rather than given, and file by file: two files shifted off the same
    // grid by different amounts come back by different amounts.
    if (toGrid) {
        return std::optional<core::Duration>{};
    }
    if (by.empty()) {
        return std::unexpected{std::string{
            "shift needs --by, or --to-grid to work the amount out from the positions"}};
    }

    const std::expected<core::Duration, std::string> parsed = parseTime(by);
    if (!parsed) {
        return std::unexpected{parsed.error()};
    }
    return std::optional{*parsed};
}

ExitCode shiftAll(core::FileSystem& files,
                  const std::vector<std::string>& paths,
                  const std::optional<core::Encoding>& reading,
                  core::Duration by,
                  const Destination& destination,
                  const Reporter& reporter) {
    const Operation shift = [by](core::Session& session) -> OperationOutcome {
        return shiftWhole(session, by, "");
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

        return shiftWhole(
            session, *by, " onto their " + std::string{nameOf(grid.retained.rate)} + " fps grid");
    };

    return rewriteAll(files, paths, reading, destination, reporter, "shifted", onto);
}

} // namespace subedit::cli
