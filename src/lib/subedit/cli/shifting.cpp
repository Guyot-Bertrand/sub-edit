#include <subedit/cli/rewriting.hpp>
#include <subedit/cli/shifting.hpp>
#include <subedit/core/edit/session.hpp>
#include <subedit/core/edit/shift_command.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/model/selection.hpp>

#include <cstddef>
#include <expected>
#include <memory>
#include <span>
#include <string>

namespace subedit::cli {

namespace {

constexpr std::int64_t kMillisecondsPerSecond = 1000;

/// The shift as the narration writes it: seconds, signed, to the millisecond.
std::string secondsOf(core::Duration by) {
    const std::int64_t total = by.milliseconds();
    const std::int64_t whole = total / kMillisecondsPerSecond;
    std::int64_t rest = total % kMillisecondsPerSecond;
    if (rest < 0) {
        rest = -rest;
    }

    std::string text = std::to_string(whole);
    if (total < 0 && whole == 0) {
        text = "-" + text;
    }
    std::string decimals = std::to_string(rest);
    decimals.insert(0, 3 - decimals.size(), '0');
    return text + "." + decimals + " s";
}

} // namespace

ExitCode shiftAll(core::FileSystem& files,
                  const std::vector<std::string>& paths,
                  core::Duration by,
                  const Destination& destination,
                  const Reporter& reporter) {
    const Operation shift = [by](core::Session& session) -> std::expected<void, std::string> {
        const std::span<const core::Subtitle> subtitles = session.project().subtitles();
        for (std::size_t i = 0; i < subtitles.size(); ++i) {
            if ((subtitles[i].start + by).milliseconds() < 0) {
                return std::unexpected{"subtitle " + std::to_string(i + 1) +
                                       " would start before the origin, which no subtitle "
                                       "file can hold"};
            }
        }

        session.apply(
            std::make_unique<core::ShiftCommand>(core::Selection::all(session.project()), by));
        return {};
    };

    return rewriteAll(files, paths, destination, reporter, "shifted", "by " + secondsOf(by), shift);
}

} // namespace subedit::cli
