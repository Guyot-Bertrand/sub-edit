#include <subedit/core/model/project.hpp>
#include <subedit/core/model/selection.hpp>

#include <algorithm>
#include <cstddef>
#include <span>
#include <utility>
#include <vector>

namespace subedit::core {

namespace {

/// Turns ascending, duplicate-free indices into runs, merging the consecutive
/// ones.
[[nodiscard]] std::vector<IndexRange> runsOf(std::span<const SubtitleIndex> ascending) {
    std::vector<IndexRange> ranges;
    for (const SubtitleIndex index : ascending) {
        // Adjacent to the run being built means extending it, which is what
        // keeps the form canonical: two selections holding the same subtitles
        // hold the same runs, so comparing them compares what they cover.
        if (!ranges.empty() && ranges.back().last.value() + 1 == index.value()) {
            ranges.back().last = index;
            continue;
        }

        ranges.push_back(IndexRange{.first = index, .last = index});
    }

    return ranges;
}

} // namespace

Selection Selection::all(const Project& project) {
    if (project.count() == 0)
        return Selection{{}};

    return Selection{{IndexRange{.first = SubtitleIndex::fromValue(0),
                                 .last = SubtitleIndex::fromValue(project.count() - 1)}}};
}

Selection Selection::of(std::span<const SubtitleIndex> indices) {
    std::vector<SubtitleIndex> sorted(indices.begin(), indices.end());
    std::ranges::sort(sorted);
    const auto duplicates = std::ranges::unique(sorted);
    sorted.erase(duplicates.begin(), duplicates.end());

    return Selection{runsOf(sorted)};
}

Selection Selection::range(SubtitleIndex first, SubtitleIndex last) {
    if (last < first)
        return Selection{{}};

    return Selection{{IndexRange{.first = first, .last = last}}};
}

std::size_t Selection::count() const {
    std::size_t total = 0;
    for (const IndexRange& range : m_ranges)
        total += range.count();

    return total;
}

bool Selection::contains(SubtitleIndex index) const {
    // The first run that could hold it is the first whose end is not before
    // it; the runs being ascending and disjoint, no other one can.
    const auto candidate = std::ranges::lower_bound(
        m_ranges, index, {}, [](const IndexRange& range) { return range.last; });

    return candidate != m_ranges.end() && candidate->first <= index;
}

} // namespace subedit::core
