#include <subedit/core/command/composite_command.hpp>
#include <subedit/core/edit/set_position_command.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/transcription/synchronization.hpp>

#include <utility>

namespace subedit::core {

LinesToSynchronize subtitlesBeginningIn(const Project& project,
                                        const std::optional<Timestamp>& from,
                                        const std::optional<Timestamp>& to) {
    LinesToSynchronize looked;
    const auto subtitles = project.subtitles();
    for (std::size_t at = 0; at < subtitles.size(); ++at) {
        const Timestamp start = subtitles[at].start;
        if ((from && start < *from) || (to && start > *to))
            continue;
        looked.subtitles.push_back(at);
        looked.lines.push_back(subtitles[at].mainText);
    }
    return looked;
}

LinesToSynchronize subtitlesOf(const Project& project, const Selection& selection) {
    LinesToSynchronize looked;
    for (const SubtitleIndex index : selection.indices()) {
        looked.subtitles.push_back(index.value());
        looked.lines.push_back(project.subtitleAt(index).mainText);
    }
    return looked;
}

SynchronizationPlan planSynchronization(const Project& project,
                                        std::vector<std::size_t> subtitles,
                                        std::span<const LineMatch> matches,
                                        const DoubtLimits& limits) {
    SynchronizationPlan plan{
        .subtitles = std::move(subtitles), .report = {}, .retimed = {}, .originals = {}};
    std::vector<Timestamp> starts;
    plan.originals.reserve(plan.subtitles.size());
    starts.reserve(plan.subtitles.size());
    for (const std::size_t at : plan.subtitles) {
        const Subtitle& subtitle = project.subtitleAt(SubtitleIndex::fromValue(at));
        starts.push_back(subtitle.start);
        plan.originals.push_back({.start = subtitle.start, .end = subtitle.end});
    }
    plan.report = judgeAlignment(matches, starts, limits);

    std::vector<AlignedLine> places;
    places.reserve(matches.size());
    for (const LineMatch& match : matches)
        places.push_back(match.place);
    plan.retimed = retime(plan.originals, places);
    return plan;
}

std::size_t SynchronizationPlan::changed() const {
    std::size_t count = 0;
    for (std::size_t line = 0; line < retimed.size(); ++line)
        if (changes(line))
            ++count;
    return count;
}

std::unique_ptr<Command> synchronizationCommand(const Project& project,
                                                const SynchronizationPlan& plan) {
    std::vector<std::unique_ptr<Command>> commands;
    for (std::size_t line = 0; line < plan.retimed.size(); ++line) {
        if (!plan.changes(line))
            continue;
        const auto index = SubtitleIndex::fromValue(plan.subtitles[line]);
        commands.push_back(std::make_unique<SetPositionCommand>(
            project, index, Boundary::Start, plan.retimed[line].times.start));
        commands.push_back(std::make_unique<SetPositionCommand>(
            project, index, Boundary::End, plan.retimed[line].times.end));
    }
    if (commands.empty())
        return nullptr;
    return std::make_unique<CompositeCommand>(CommandKind::Synchronize, std::move(commands));
}

} // namespace subedit::core
