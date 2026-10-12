#include <subedit/cli/disjoining.hpp>
#include <subedit/cli/rewriting.hpp>
#include <subedit/cli/sorting.hpp>
#include <subedit/cli/time_grammar.hpp>
#include <subedit/core/edit/session.hpp>
#include <subedit/core/i18n/messages.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/wording/editing.hpp>

#include <charconv>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string_view>
#include <system_error>
#include <utility>

namespace subedit::cli {

namespace {

[[nodiscard]] Failure notInOrder(core::DisjointRefusal refusal) {
    return Failure{"not-in-order", [refusal] {
                       // TRANSLATORS: %1 is why the subtitles cannot be made disjoint as they are.
                       return core::substitute(core::translate("%1 Use --sort to do that here."),
                                               {core::refusalOfDisjoint(refusal)});
                   }};
}

} // namespace

std::expected<core::DisjointSettings, std::string> settingsOf(const DisjointOptions& options) {
    core::DisjointSettings settings;

    if (!options.minimum.empty()) {
        const std::expected<core::Duration, std::string> time = parseTime(options.minimum);
        if (!time) {
            // TRANSLATORS: %1 is why the value does not read as a time.
            return std::unexpected{
                core::substitute(core::translate("--minimum: %1"), {time.error()})};
        }
        if (time->milliseconds() < 0) {
            return std::unexpected{core::substitute(
                // TRANSLATORS: %1 is what the user typed.
                core::translate("--minimum: \"%1\" is not a duration of zero or more"),
                {options.minimum})};
        }
        settings.shortest = *time;
    }

    if (!options.maxLines.empty()) {
        std::size_t lines = 0;
        const std::string_view text = options.maxLines;
        const auto [end, error] = std::from_chars(text.data(), text.data() + text.size(), lines);
        if (error != std::errc{} || end != text.data() + text.size()) {
            return std::unexpected{core::substitute(
                // TRANSLATORS: %1 is what the user typed.
                core::translate("--max-lines: \"%1\" is not a number of lines: expected a whole "
                                "number, 0 for no limit"),
                {options.maxLines})};
        }
        settings.mostLines = lines;
    }

    settings.dashes = options.dashes;
    return settings;
}

std::expected<core::DisjointReport, Failure> disjointedIn(core::Session& session) {
    auto made = core::makeDisjoint(session.project(), core::DisjointSettings{});
    if (!made) {
        return std::unexpected{notInOrder(made.error())};
    }
    if (made->command != nullptr) {
        (void)session.apply(std::move(made->command));
    }
    return made->report;
}

std::expected<core::DisjointReport, Failure> disjointedInPlace(core::Project& project) {
    auto made = core::makeDisjoint(project, core::DisjointSettings{});
    if (!made) {
        return std::unexpected{notInOrder(made.error())};
    }
    if (made->command != nullptr) {
        made->command->apply(project);
    }
    return made->report;
}

std::string narrationOfDisjoint(const std::string& path, const core::DisjointReport& report) {
    return core::substitute(core::translate("%1: %2"), {path, core::noticeOfDisjoint(report)});
}

std::vector<Count> fullCountsOfDisjoint(const core::DisjointReport& report) {
    const auto count = [](std::size_t number) { return static_cast<std::int64_t>(number); };
    return {{"split", count(report.split)},
            {"merged", count(report.merged)},
            {"windowed", count(report.windowed)},
            {"absorbed", count(report.absorbed)},
            {"too_short", count(report.tooShort)},
            {"ending_before_start", count(report.endingBeforeStart)},
            {"extras_dropped", count(report.extrasDropped)}};
}

std::vector<Count> countsOfDisjoint(const core::DisjointReport& report) {
    return {{"split", static_cast<std::int64_t>(report.split)},
            {"merged", static_cast<std::int64_t>(report.merged)}};
}

void addCountsOfDisjoint(std::vector<Count>& counts,
                         const std::optional<core::DisjointReport>& cut) {
    if (!cut.has_value()) {
        return;
    }
    for (Count& count : countsOfDisjoint(*cut)) {
        counts.push_back(std::move(count));
    }
}

ExitCode disjointAll(core::FileSystem& files,
                     const std::vector<std::string>& paths,
                     const std::optional<core::Encoding>& reading,
                     const core::DisjointSettings& settings,
                     bool sort,
                     const Destination& destination,
                     const Reporter& reporter) {
    const Operation disjoint = [settings, sort](core::Session& session) -> OperationOutcome {
        const std::size_t moved = sort ? sortedIn(session) : 0;

        auto made = core::makeDisjoint(session.project(), settings);
        if (!made) {
            return std::unexpected{notInOrder(made.error())};
        }
        const core::DisjointReport report = made->report;
        if (made->command != nullptr) {
            (void)session.apply(std::move(made->command));
        }

        std::vector<Count> counts = fullCountsOfDisjoint(report);
        counts.insert(counts.begin(),
                      Count{"subtitles", static_cast<std::int64_t>(session.project().count())});
        if (sort) {
            counts.emplace_back("moved", static_cast<std::int64_t>(moved));
        }
        return OperationResult{.sentence = core::noticeOfDisjoint(report),
                               .counts = std::move(counts)};
    };

    return rewriteAll(files, paths, reading, destination, reporter, Verb::MadeDisjoint, disjoint);
}

} // namespace subedit::cli
