#include <subedit/core/edit/transform_command.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/model/subtitle.hpp>
#include <subedit/core/time/duration.hpp>

#include <cstddef>
#include <optional>
#include <span>
#include <utility>
#include <vector>

namespace subedit::core {

std::optional<TransformCommand> TransformCommand::create(const Project& project,
                                                         Selection selection,
                                                         TransformReference first,
                                                         TransformReference second) {
    const Timestamp firstStart = project.subtitleAt(first.index).start;
    const Timestamp secondStart = project.subtitleAt(second.index).start;

    // The two conditions have to bear on two distinct positions. Testing the
    // starts and not the indices catches both refusals at once: one subtitle
    // named twice, and two subtitles that begin together.
    const std::optional<Ratio> ratio = Ratio::create((second.target - first.target).milliseconds(),
                                                     (secondStart - firstStart).milliseconds());
    if (!ratio.has_value())
        return std::nullopt;

    std::vector<Positions> previous;
    previous.reserve(selection.count());
    for (const SubtitleIndex index : selection.indices()) {
        const Subtitle& subtitle = project.subtitleAt(index);
        previous.push_back(Positions{.start = subtitle.start, .end = subtitle.end});
    }

    return TransformCommand{
        std::move(selection), *ratio, firstStart, first.target, std::move(previous)};
}

void TransformCommand::apply(Project& project) {
    for (const SubtitleIndex index : m_selection.indices()) {
        Subtitle& subtitle = project.subtitleAt(index);
        subtitle.start = transformed(subtitle.start);
        subtitle.end = transformed(subtitle.end);
    }
}

void TransformCommand::revert(Project& project) {
    std::size_t rank = 0;
    for (const SubtitleIndex index : m_selection.indices()) {
        Subtitle& subtitle = project.subtitleAt(index);
        subtitle.start = m_previous[rank].start;
        subtitle.end = m_previous[rank].end;
        ++rank;
    }
}

} // namespace subedit::core
