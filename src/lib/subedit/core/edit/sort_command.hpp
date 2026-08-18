#pragma once

#include <subedit/core/command/change.hpp>
#include <subedit/core/command/command.hpp>
#include <subedit/core/command/command_kind.hpp>
#include <subedit/core/model/selection.hpp>
#include <subedit/core/model/subtitle_index.hpp>

#include <cstddef>
#include <vector>

namespace subedit::core {

class Project;

/// Puts the subtitles back in order of their start, and undoes it.
///
/// **Stable**: two subtitles starting at the same moment keep the order the
/// file gave them. Neither precedes the other, so moving them would be a
/// decision nobody asked for.
///
/// A command like any other, which is the whole point of ADR 0012: Gaupol
/// sorts at opening, before its history exists, and that sort cannot be
/// undone. Here it can.
///
/// What it retains to undo itself is the **previous order**, one index per
/// subtitle — not a copy of the subtitles. On a file of several thousand, that
/// is the difference between a few tens of kilobytes and a few megabytes per
/// history entry.
class SortCommand final : public Command {

public:
    void apply(Project& project) override;

    void revert(Project& project) override;

    [[nodiscard]] CommandKind kind() const override { return CommandKind::Sort; }

    /// Reports the subtitles that moved, or nothing when none did.
    ///
    /// Nothing rather than an empty reordering: a sort that changed no order
    /// must not mark the document as differing from its file. A strict policy
    /// appends this command after every operation that could have broken the
    /// order, and most of them will not have.
    [[nodiscard]] std::vector<Change> describe() const override;

private:
    /// Where each subtitle came from: `m_previousOrder[p]` is the index the
    /// subtitle now at `p` had before the sort. Empty until `apply`.
    std::vector<std::size_t> m_previousOrder;

    /// The indices that ended up holding a different subtitle.
    Selection m_moved = Selection::of({});
};

} // namespace subedit::core
