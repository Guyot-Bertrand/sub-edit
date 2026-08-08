#pragma once

#include <subedit/core/command/change.hpp>
#include <subedit/core/command/command.hpp>
#include <subedit/core/command/command_kind.hpp>
#include <subedit/core/model/boundary.hpp>
#include <subedit/core/model/subtitle_index.hpp>
#include <subedit/core/time/timestamp.hpp>

#include <vector>

namespace subedit::core {

class Project;

/// Moves one end of one subtitle.
///
/// One class for both ends rather than two nearly identical ones: what
/// separates a start from an end is a value, not a behaviour, and `Boundary`
/// is that value — the same way `Document` separates the two texts. The
/// distinction survives where it matters, in `kind()`, which the strict order
/// policy reads: only a start can break the order.
///
/// What it retains to undo itself is the **old position, and nothing else**.
///
/// **An end may be set before its start.** `end >= start` is not an invariant
/// — ADR 0008 — and refusing the edit here would refuse the user the state a
/// real file can already be in.
class SetPositionCommand final : public Command {

public:
    /// Captures the position `index` currently has at `boundary`.
    SetPositionCommand(const Project& project,
                       SubtitleIndex index,
                       Boundary boundary,
                       Timestamp position);

    void apply(Project& project) override;

    void revert(Project& project) override;

    [[nodiscard]] CommandKind kind() const override {
        return m_boundary == Boundary::Start ? CommandKind::SetStart : CommandKind::SetEnd;
    }

    /// Reports a change of positions, whichever end moved.
    ///
    /// A subtitle carries one pair of positions for both its texts, so moving
    /// either end concerns both documents.
    [[nodiscard]] std::vector<Change> describe() const override {
        return {Change{.kind = ChangeKind::Positions, .indices = {m_index}}};
    }

private:
    SubtitleIndex m_index;
    Boundary m_boundary;
    Timestamp m_newPosition;
    Timestamp m_oldPosition;
};

} // namespace subedit::core
