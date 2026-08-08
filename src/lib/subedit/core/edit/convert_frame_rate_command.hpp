#pragma once

#include <subedit/core/command/change.hpp>
#include <subedit/core/command/command.hpp>
#include <subedit/core/command/command_kind.hpp>
#include <subedit/core/model/selection.hpp>
#include <subedit/core/time/frame_rate.hpp>
#include <subedit/core/time/ratio.hpp>
#include <subedit/core/time/timestamp.hpp>

#include <vector>

namespace subedit::core {

class Project;

/// Re-times a selection from one frame rate to another, and re-declares the
/// project's own rate.
///
/// ```
/// t′ = round( t × (input / output) )
/// ```
///
/// A film mastered at 23.976 and played at 25 frames per second runs faster,
/// so its subtitles come earlier; the other way round they come later. That is
/// Gaupol's semantics, and the one the ratio above expresses.
///
/// **Never through the frames.** `fromFrame(toFrame(t, input), output)` looks
/// like the same thing and is not: it quantises every position on the input
/// grid, and lands up to half a frame away — 21 ms, measured. ADR 0013 exists
/// for this, and `Ratio` is what replaces it.
///
/// The rate the command puts back when undone is **the one the project had**,
/// which is not always the one declared as input: a user may state that the
/// file was really timed at another rate than the project assumed. Undoing
/// restores the state that was, not the one that was asserted.
///
/// Like the transform, it retains the previous positions: the operation
/// rounds, so its inverse cannot be a second calculation.
class ConvertFrameRateCommand final : public Command {

public:
    /// Captures the positions of `selection` and the rate of `project`.
    ///
    /// Total, unlike the transform: two frame rates always define a ratio,
    /// since both of their terms are strictly positive and bounded.
    ConvertFrameRateCommand(const Project& project,
                            Selection selection,
                            FrameRate input,
                            FrameRate output);

    void apply(Project& project) override;

    void revert(Project& project) override;

    [[nodiscard]] CommandKind kind() const override { return CommandKind::ConvertFrameRate; }

    /// Reports a change of positions on the selected subtitles.
    ///
    /// The rate of the project changes too, and no `ChangeKind` says so: it is
    /// not a property of any subtitle, and inventing an enumerator nothing
    /// reads would be a promise with no keeper — the rule `DiagnosticKind`
    /// already follows.
    [[nodiscard]] std::vector<Change> describe() const override {
        return {Change{.kind = ChangeKind::Positions,
                       .indices = {m_selection.indices().begin(), m_selection.indices().end()}}};
    }

private:
    /// The two ends of one subtitle, as they stood before the conversion.
    struct Positions {
        Timestamp start;
        Timestamp end;
    };

    Selection m_selection;
    Ratio m_factor;
    FrameRate m_output;
    FrameRate m_previousFrameRate;
    std::vector<Positions> m_previous;
};

} // namespace subedit::core
