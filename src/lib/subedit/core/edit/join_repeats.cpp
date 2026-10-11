#include <subedit/core/command/command_kind.hpp>
#include <subedit/core/command/composite_command.hpp>
#include <subedit/core/edit/join_repeats.hpp>
#include <subedit/core/edit/remove_command.hpp>
#include <subedit/core/edit/set_position_command.hpp>
#include <subedit/core/model/boundary.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/model/selection.hpp>
#include <subedit/core/model/subtitle.hpp>
#include <subedit/core/model/subtitle_index.hpp>
#include <subedit/core/time/timestamp.hpp>

#include <algorithm>
#include <cstddef>
#include <memory>
#include <span>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace subedit::core {

namespace {

/// A run of repeats under way: the subtitle it keeps, and how far it reaches.
struct Chain {
    std::size_t head;
    Timestamp end;
    std::size_t absorbed = 0;
};

[[nodiscard]] bool showsSameThing(const Subtitle& left, const Subtitle& right) {
    return left.mainText == right.mainText && left.translationText == right.translationText &&
           left.extras == right.extras;
}

/// Subtitles that show something, and can: the others are never joined.
[[nodiscard]] bool joinable(const Subtitle& subtitle) {
    return subtitle.end > subtitle.start &&
           (!subtitle.mainText.empty() || !subtitle.translationText.empty());
}

/// Cuts one bucket — subtitles with the same texts, in the order of their starts —
/// into runs of repeats, and says which subtitles each run takes over.
[[nodiscard]] std::vector<Chain> chainsOf(const std::vector<std::size_t>& bucket,
                                          std::span<const Subtitle> subtitles,
                                          std::vector<SubtitleIndex>& taken) {
    std::vector<Chain> chains;
    for (const std::size_t at : bucket) {
        const Subtitle& subtitle = subtitles[at];
        const auto open = std::ranges::find_if(chains, [&](const Chain& chain) {
            return subtitle.start <= chain.end && showsSameThing(subtitles[chain.head], subtitle);
        });
        if (open == chains.end()) {
            chains.push_back(Chain{.head = at, .end = subtitle.end});
            continue;
        }
        open->end = std::max(open->end, subtitle.end);
        ++open->absorbed;
        taken.push_back(SubtitleIndex::fromValue(at));
    }
    return chains;
}

} // namespace

JoinedRepeats joinRepeats(const Project& project) {
    const std::span<const Subtitle> subtitles = project.subtitles();

    // By text first, which is what tells most subtitles apart: a bucket holds
    // the subtitles that may repeat each other.
    std::unordered_map<std::string, std::vector<std::size_t>> buckets;
    for (std::size_t at = 0; at < subtitles.size(); ++at) {
        if (joinable(subtitles[at]))
            buckets[subtitles[at].mainText + '\0' + subtitles[at].translationText].push_back(at);
    }

    std::vector<SubtitleIndex> taken;
    std::vector<std::unique_ptr<Command>> commands;
    JoinReport report;

    for (auto& entry : buckets) {
        std::vector<std::size_t>& bucket = entry.second;
        std::ranges::stable_sort(bucket, {}, [&](std::size_t at) { return subtitles[at].start; });

        for (const Chain& chain : chainsOf(bucket, subtitles, taken)) {
            if (chain.absorbed == 0)
                continue;
            ++report.chains;
            report.joined += chain.absorbed;
            if (chain.end == subtitles[chain.head].end)
                continue;
            commands.push_back(std::make_unique<SetPositionCommand>(
                project, SubtitleIndex::fromValue(chain.head), Boundary::End, chain.end));
        }
    }

    JoinedRepeats result;
    result.report = report;
    if (taken.empty())
        return result;

    // The ends first, while every index is the one it was read at; then the
    // subtitles they now cover, all at once.
    commands.push_back(std::make_unique<RemoveCommand>(Selection::of(taken)));
    result.command =
        std::make_unique<CompositeCommand>(CommandKind::JoinRepeats, std::move(commands));
    return result;
}

} // namespace subedit::core
