#include <subedit/core/model/project.hpp>
#include <subedit/core/model/selection.hpp>

#include <algorithm>
#include <cstddef>
#include <span>
#include <utility>
#include <vector>

namespace subedit::core {

Selection Selection::all(const Project& project) {
    std::vector<SubtitleIndex> indices;
    indices.reserve(project.count());
    for (std::size_t value = 0; value < project.count(); ++value)
        indices.push_back(SubtitleIndex::fromValue(value));

    // Already ascending and unique, so the sort the other factories need would
    // only walk the vector again.
    return Selection{std::move(indices)};
}

Selection Selection::of(std::span<const SubtitleIndex> indices) {
    std::vector<SubtitleIndex> sorted(indices.begin(), indices.end());
    std::ranges::sort(sorted);
    const auto duplicates = std::ranges::unique(sorted);
    sorted.erase(duplicates.begin(), duplicates.end());

    return Selection{std::move(sorted)};
}

Selection Selection::range(SubtitleIndex first, SubtitleIndex last) {
    std::vector<SubtitleIndex> indices;
    if (last < first)
        return Selection{std::move(indices)};

    indices.reserve(last.value() - first.value() + 1);
    for (std::size_t value = first.value(); value <= last.value(); ++value)
        indices.push_back(SubtitleIndex::fromValue(value));

    return Selection{std::move(indices)};
}

bool Selection::contains(SubtitleIndex index) const {
    return std::ranges::binary_search(m_indices, index);
}

} // namespace subedit::core
