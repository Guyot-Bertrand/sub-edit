#pragma once

#include <subedit/core/model/subtitle_index.hpp>

#include <cstddef>
#include <span>
#include <vector>

namespace subedit::core {

class Project;

/// The subtitles an operation applies to: indices, sorted and without
/// duplicates by construction.
///
/// Gaupol repeats in every signature an `indices` parameter whose absent value
/// means «the whole project». A missing value that means «all of them» is
/// exactly the convention a type should carry, and `Selection::all` says it.
///
/// Sorted and unique is not a courtesy to the caller: removal walks the indices
/// backwards to keep the earlier ones valid, and insertion counts on them not
/// repeating. Both would be wrong on a list handed over in any order.
class Selection {

public:
    /// Builds the selection of every subtitle of `project`, in order.
    [[nodiscard]] static Selection all(const Project& project);

    /// Builds the selection of `indices`, sorted and deduplicated.
    [[nodiscard]] static Selection of(std::span<const SubtitleIndex> indices);

    /// Builds the selection of every index from `first` to `last`, both
    /// included.
    ///
    /// Empty when `last` precedes `first`: the caller asked for what lies
    /// between two bounds, and nothing does. Reversing them would answer a
    /// question that was not asked, and refusing would turn an empty range
    /// into a case to handle.
    [[nodiscard]] static Selection range(SubtitleIndex first, SubtitleIndex last);

    /// Returns the indices, ascending.
    [[nodiscard]] std::span<const SubtitleIndex> indices() const { return m_indices; }

    /// Returns how many subtitles the selection covers.
    [[nodiscard]] std::size_t count() const { return m_indices.size(); }

    [[nodiscard]] bool isEmpty() const { return m_indices.empty(); }

    /// Tells whether `index` is one of the selected.
    [[nodiscard]] bool contains(SubtitleIndex index) const;

    friend bool operator==(const Selection&, const Selection&) = default;

private:
    explicit Selection(std::vector<SubtitleIndex> indices) : m_indices(std::move(indices)) {}

    std::vector<SubtitleIndex> m_indices;
};

} // namespace subedit::core
