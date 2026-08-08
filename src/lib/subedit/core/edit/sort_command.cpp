#include <subedit/core/edit/sort_command.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/model/subtitle.hpp>

#include <algorithm>
#include <cstddef>
#include <numeric>
#include <vector>

namespace subedit::core {

void SortCommand::apply(Project& project) {
    const std::size_t count = project.count();

    std::vector<std::size_t> order(count);
    std::ranges::iota(order, std::size_t{0});
    // Stable, and on the index rather than on the subtitle: equal starts keep
    // the order the file gave them, and the permutation is what undoing needs.
    std::ranges::stable_sort(order, {}, [&project](std::size_t position) {
        return project.subtitles()[position].start;
    });

    std::vector<Subtitle> sorted;
    sorted.reserve(count);
    m_moved.clear();
    for (std::size_t position = 0; position < count; ++position) {
        sorted.push_back(project.subtitles()[order[position]]);
        if (order[position] != position)
            m_moved.push_back(SubtitleIndex::fromValue(position));
    }

    m_previousOrder = std::move(order);
    project.setSubtitles(std::move(sorted));
}

void SortCommand::revert(Project& project) {
    std::vector<Subtitle> previous(m_previousOrder.size());
    for (std::size_t position = 0; position < m_previousOrder.size(); ++position)
        previous[m_previousOrder[position]] = project.subtitles()[position];

    project.setSubtitles(std::move(previous));
}

std::vector<Change> SortCommand::describe() const {
    if (m_moved.empty())
        return {};

    return {Change{.kind = ChangeKind::Reordering, .indices = m_moved}};
}

} // namespace subedit::core
