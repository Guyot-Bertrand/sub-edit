#pragma once

#include <subedit/core/command/change.hpp>
#include <subedit/core/command/command.hpp>
#include <subedit/core/command/command_kind.hpp>
#include <subedit/core/model/selection.hpp>
#include <subedit/core/time/duration.hpp>

#include <utility>
#include <vector>

namespace subedit::core {

class Project;

/// Moves the selected subtitles by a fixed amount of time.
///
/// Both ends by the same amount, which is what makes a shift a shift: no
/// subtitle stays on screen any longer or shorter than it did.
///
/// **What it retains to undo itself is the `Duration`, not the positions.**
/// Its inverse is the same shift the other way, and it is exact — no rounding
/// happens here, so nothing is lost that would have to be remembered. That is
/// the economy ADR 0010 asks for, and the reason this command costs a handful
/// of bytes where the transform and the frame-rate conversion cost one
/// position per subtitle.
///
/// A shift backwards may take a subtitle before the origin. Positions there
/// are valid, and refusing them would turn an editing operation into a special
/// case.
class ShiftCommand final : public Command {

public:
    ShiftCommand(Selection selection, Duration shift)
        : m_selection(std::move(selection)), m_shift(shift) {}

    void apply(Project& project) override;

    void revert(Project& project) override;

    [[nodiscard]] CommandKind kind() const override { return CommandKind::Shift; }

    /// Reports a change of positions on the selected subtitles.
    [[nodiscard]] std::vector<Change> describe() const override {
        return {Change{.kind = ChangeKind::Positions, .subtitles = m_selection}};
    }

private:
    /// Moves every selected subtitle by `shift`.
    void moveBy(Project& project, Duration shift) const;

    Selection m_selection;
    Duration m_shift;
};

} // namespace subedit::core
