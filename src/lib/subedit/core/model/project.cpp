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
    // leave a project neither in its old state nor in the asked-for one. The
    // runs being ascending, only the last index can be out of range.
    if (!selection.isEmpty() && selection.ranges().back().last.value() >= m_subtitles.size())
        refuse(selection.ranges().back().last.value(), m_subtitles.size());

    std::vector<Subtitle> removed;
    removed.reserve(selection.count());
    for (const SubtitleIndex index : selection.indices())
        removed.push_back(m_subtitles[index.value()]);

    // One walk, copying forward what stays. Erasing one index at a time shifts
    // the tail once per removed subtitle — quadratic on a wide selection, which
    // is what issue #45 came to fix.
    Selection::Indices::Iterator selected = selection.indices().begin();
    const Selection::Indices::Iterator stop = selection.indices().end();
    std::size_t kept = 0;

    for (std::size_t position = 0; position < m_subtitles.size(); ++position) {
        if (selected != stop && (*selected).value() == position) {
            ++selected;
            continue;
        }

        if (kept != position)
            m_subtitles[kept] = std::move(m_subtitles[position]);
        ++kept;
    }

    m_subtitles.resize(kept);
    return removed;
}

void Project::restore(const Selection& destinations, std::span<const Subtitle> subtitles) {
    if (destinations.count() != subtitles.size())
        throw std::invalid_argument("restoring " + std::to_string(subtitles.size()) +
                                    " subtitles at " + std::to_string(destinations.count()) +
                                    " destinations");

    if (subtitles.empty())
        return;

    const std::size_t total = m_subtitles.size() + subtitles.size();
    if (destinations.ranges().back().last.value() >= total)
        refuse(destinations.ranges().back().last.value(), total);

    // One walk over the restored project, taking from the kept subtitles or
    // from the returned ones according to the destination that comes next.
    // Both sequences are ascending, so neither has to be searched.
    std::vector<Subtitle> merged;
    merged.reserve(total);

    Selection::Indices::Iterator destination = destinations.indices().begin();
    const Selection::Indices::Iterator stop = destinations.indices().end();
    std::size_t taken = 0;
    std::size_t kept = 0;

    for (std::size_t position = 0; position < total; ++position) {
        if (destination != stop && (*destination).value() == position) {
            merged.push_back(subtitles[taken]);
            ++taken;
            ++destination;
            continue;
        }

        merged.push_back(std::move(m_subtitles[kept]));
        ++kept;
    }

    m_subtitles = std::move(merged);
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
