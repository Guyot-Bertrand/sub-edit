#pragma once

#include <subedit/core/command/change.hpp>
#include <subedit/core/command/command.hpp>
#include <subedit/core/command/command_kind.hpp>
#include <subedit/core/model/selection.hpp>
#include <subedit/core/model/subtitle_index.hpp>
#include <subedit/core/time/duration.hpp>
#include <subedit/core/time/timestamp.hpp>

#include <vector>

namespace subedit::core {

class Project;

/// Gives one subtitle a duration, by moving its end.
///
/// **The end, and never the start** — decision D3 of the phase-10 spec. It is
/// the one end `adjust_durations` moves, moving the start would move the
/// subtitle, which is the work of a shift, and the start already has a command
/// of its own.
///
/// What it retains to undo itself is the **old end, and nothing else** — the
/// same state `SetPositionCommand` keeps for an end.
///
/// **An overlap with the next subtitle is allowed.** `scanAnomalies` reports it,
/// and refusing an edit whose consequence shows would refuse what the user may
/// want — decision D4 of phase 5.
class SetDurationCommand final : public Command {

public:
    /// Captures the end `index` currently has, and the end `duration` gives.
    SetDurationCommand(const Project& project, SubtitleIndex index, Duration duration);

    void apply(Project& project) override;

    void revert(Project& project) override;

    [[nodiscard]] CommandKind kind() const override { return CommandKind::SetDuration; }

    /// Reports a change of positions: the end moved, and a subtitle carries one
    /// pair of positions for both its texts.
    [[nodiscard]] std::vector<Change> describe() const override {
        return {
            Change{.kind = ChangeKind::Positions, .subtitles = Selection::range(m_index, m_index)}};
    }

private:
    SubtitleIndex m_index;
    Timestamp m_newEnd;
    Timestamp m_oldEnd;
};

} // namespace subedit::core
