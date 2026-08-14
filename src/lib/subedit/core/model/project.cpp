#include <subedit/core/model/project.hpp>
#include <subedit/core/model/selection.hpp>

#include <algorithm>
#include <cstddef>
#include <span>
#include <stdexcept>
#include <string>
#include <vector>

namespace subedit::core {

namespace {

[[noreturn]] void refuse(std::size_t value, std::size_t count) {
    throw std::out_of_range("subtitle index " + std::to_string(value) + " in a project of " +
                            std::to_string(count));
}

} // namespace

void Project::insert(SubtitleIndex index, std::span<const Subtitle> subtitles) {
    if (index.value() > m_subtitles.size())
        refuse(index.value(), m_subtitles.size());

    m_subtitles.insert(m_subtitles.begin() + static_cast<std::ptrdiff_t>(index.value()),
                       subtitles.begin(),
                       subtitles.end());
}

std::vector<Subtitle> Project::remove(const Selection& selection) {
    // Refuse before touching anything: a removal that threw halfway would
    // leave a project neither in its old state nor in the asked-for one.
    for (const SubtitleIndex index : selection.indices()) {
        if (index.value() >= m_subtitles.size())
            refuse(index.value(), m_subtitles.size());
    }

    std::vector<Subtitle> removed;
    removed.reserve(selection.count());
    for (const SubtitleIndex index : selection.indices())
        removed.push_back(m_subtitles[index.value()]);

    // Erasing backwards is what keeps a discontinuous selection right: the
    // indices still to come are all smaller, so none of them moves. The
    // selection is ascending by construction, hence the reverse walk.
    const std::span<const SubtitleIndex> indices = selection.indices();
    for (std::size_t remaining = indices.size(); remaining > 0; --remaining) {
        const std::size_t value = indices[remaining - 1].value();
        m_subtitles.erase(m_subtitles.begin() + static_cast<std::ptrdiff_t>(value));
    }

    return removed;
}

std::vector<SubtitleIndex> Project::outOfOrder(OrderReport report) const {
    std::vector<SubtitleIndex> indices;
    if (m_subtitles.empty())
        return indices;

    // The reference a start is judged against: the previous start, or the
    // largest one seen. That single difference is the whole of the two
    // readings — everything else about them is identical, and writing them as
    // two loops would invite them to drift apart.
    Timestamp highest = m_subtitles.front().start;
    for (std::size_t value = 1; value < m_subtitles.size(); ++value) {
        const Timestamp start = m_subtitles[value].start;
        const Timestamp reference =
            report == OrderReport::Breaks ? m_subtitles[value - 1].start : highest;

        if (start < reference)
            indices.push_back(SubtitleIndex::fromValue(value));

        highest = std::max(highest, start);
    }

    return indices;
}

} // namespace subedit::core
