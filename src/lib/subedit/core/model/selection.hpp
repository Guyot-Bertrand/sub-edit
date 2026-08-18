#pragma once

#include <subedit/core/model/subtitle_index.hpp>

#include <cstddef>
#include <iterator>
#include <span>
#include <vector>

namespace subedit::core {

class Project;

/// A run of consecutive subtitles, **both bounds included**.
///
/// Included rather than half-open because that is what a selection means to
/// the user — « from this line to that one » — and because `Selection::range`
/// already promised it. A half-open form would have made the empty range and
/// the one-subtitle range look alike at every call site.
struct IndexRange {
    SubtitleIndex first;
    SubtitleIndex last;

    /// Returns how many subtitles the run covers, never zero.
    [[nodiscard]] constexpr std::size_t count() const { return last.value() - first.value() + 1; }

    [[nodiscard]] friend constexpr bool operator==(const IndexRange&, const IndexRange&) = default;
};

/// The subtitles an operation applies to: **runs of consecutive indices**,
/// ascending, disjoint, and never adjacent.
///
/// Gaupol repeats in every signature an `indices` parameter whose absent value
/// means «the whole project». A missing value that means «all of them» is
/// exactly the convention a type should carry, and `Selection::all` says it.
///
/// Sorted and unique is not a courtesy to the caller: removal counts on the
/// indices being ascending, and insertion counts on them not repeating. Both
/// would be wrong on a list handed over in any order.
///
/// **Runs and not indices, since issue #45.** A selection covering four
/// thousand subtitles used to cost four thousand indices, copied into every
/// command that carried it — thirty-two kilobytes per shift in the history.
/// The same selection is now two bounds. Adjacent runs are always merged, so
/// the form is canonical and two equal selections compare equal.
///
/// The trade is written rather than hidden: a maximally fragmented selection —
/// one subtitle in two — costs one run per subtitle, that is **twice** what the
/// flat list cost. It is bounded, it needs a deliberate gesture to produce, and
/// the history already carries more for a transform. Should a measurement ever
/// show it weighs, a mixed form is the way out.
class Selection {

public:
    /// A lazy walk over the selected indices, ascending.
    ///
    /// Lazy is the whole point: materialising them would give back the cost the
    /// runs just removed. Callers that write `for (const SubtitleIndex index :
    /// selection.indices())` are unaffected by the change.
    class Indices {

    public:
        class Iterator {

        public:
            using iterator_category = std::forward_iterator_tag;
            using value_type = SubtitleIndex;
            using difference_type = std::ptrdiff_t;
            using reference = SubtitleIndex;
            using pointer = void;

            Iterator() = default;

            [[nodiscard]] SubtitleIndex operator*() const {
                return SubtitleIndex::fromValue(m_value);
            }

            Iterator& operator++() {
                if (m_value < m_ranges[m_range].last.value()) {
                    ++m_value;
                    return *this;
                }

                ++m_range;
                m_value = m_range < m_ranges.size() ? m_ranges[m_range].first.value() : 0;
                return *this;
            }

            Iterator operator++(int) {
                Iterator previous = *this;
                ++*this;
                return previous;
            }

            /// Compares the position, and never the runs it walks: two
            /// iterators are only ever compared within one selection.
            [[nodiscard]] friend bool operator==(const Iterator& left, const Iterator& right) {
                return left.m_range == right.m_range && left.m_value == right.m_value;
            }

        private:
            friend class Indices;

            Iterator(std::span<const IndexRange> ranges, std::size_t range, std::size_t value)
                : m_ranges(ranges), m_range(range), m_value(value) {}

            std::span<const IndexRange> m_ranges{};
            std::size_t m_range = 0;
            std::size_t m_value = 0;
        };

        [[nodiscard]] Iterator begin() const {
            return Iterator{m_ranges, 0, m_ranges.empty() ? 0 : m_ranges.front().first.value()};
        }

        [[nodiscard]] Iterator end() const { return Iterator{m_ranges, m_ranges.size(), 0}; }

    private:
        friend class Selection;

        explicit Indices(std::span<const IndexRange> ranges) : m_ranges(ranges) {}

        std::span<const IndexRange> m_ranges;
    };

    /// Builds the selection of every subtitle of `project`, in order.
    [[nodiscard]] static Selection all(const Project& project);

    /// Builds the selection of `indices`, sorted, deduplicated and merged into
    /// runs.
    [[nodiscard]] static Selection of(std::span<const SubtitleIndex> indices);

    /// Builds the selection of every index from `first` to `last`, both
    /// included.
    ///
    /// Empty when `last` precedes `first`: the caller asked for what lies
    /// between two bounds, and nothing does. Reversing them would answer a
    /// question that was not asked, and refusing would turn an empty range
    /// into a case to handle.
    [[nodiscard]] static Selection range(SubtitleIndex first, SubtitleIndex last);

    /// Returns the runs, ascending, disjoint and non-adjacent.
    ///
    /// What a table model wants: one `dataChanged(topLeft, bottomRight)` per
    /// run rather than one per row.
    [[nodiscard]] std::span<const IndexRange> ranges() const { return m_ranges; }

    /// Returns a lazy walk over the indices, ascending.
    [[nodiscard]] Indices indices() const { return Indices{m_ranges}; }

    /// Returns how many subtitles the selection covers.
    ///
    /// A sum over the runs rather than a stored count: one addition for the
    /// selection that matters, and no second piece of state to keep true.
    [[nodiscard]] std::size_t count() const;

    [[nodiscard]] bool isEmpty() const { return m_ranges.empty(); }

    /// Tells whether `index` is one of the selected.
    [[nodiscard]] bool contains(SubtitleIndex index) const;

    [[nodiscard]] friend bool operator==(const Selection&, const Selection&) = default;

private:
    explicit Selection(std::vector<IndexRange> ranges) : m_ranges(std::move(ranges)) {}

    std::vector<IndexRange> m_ranges;
};

} // namespace subedit::core
