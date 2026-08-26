#pragma once

#include <subedit/core/command/change.hpp>
#include <subedit/core/command/command.hpp>
#include <subedit/core/command/command_kind.hpp>
#include <subedit/core/model/selection.hpp>
#include <subedit/core/time/frame_rate.hpp>
#include <subedit/core/time/timestamp.hpp>

#include <vector>

namespace subedit::core {

class Project;

/// Moves every position of a selection onto the nearest frame of a rate, and
/// re-declares the project's own rate.
///
/// ```
/// t′ = fromFrame( toFrame(t, rate), rate )
/// ```
///
/// **That composition is the one `ConvertFrameRateCommand` warns against, and
/// here it is the whole point.** A conversion must never quantise, because it
/// re-times a file whose minutage is wrong and rounding would add error to a
/// scaling. This command re-times nothing: it takes a file whose minutage is
/// already right and whose *grid* is wrong, and puts the positions back on
/// frames.
///
/// The difference is not a nuance, and it is silent when mistaken:
///
/// | | Converting | Aligning |
/// | :--- | :--------- | :------- |
/// | moves each position by | a share of the elapsed time — **seconds** over a feature film | half
/// a frame at most, everywhere | | for a file whose minutage is | **wrong** | right, its grid wrong
/// |
///
/// The use case it exists for: a film running at 25 frames per second, subtitles
/// written on a grid at 24, and cues already about where they belong to within a
/// few milliseconds. There is nothing to re-time — only to lay the timestamps
/// back on the right frames.
///
/// **Both ends, and not only the start.** A start on a frame and an end between
/// two is half aligned, and the duration it implies is not a whole number of
/// frames. Each end moves by at most half a frame, so a subtitle's duration
/// changes by at most one.
///
/// **The order survives.** Rounding to the nearest frame is monotone, so two
/// starts a frame or more apart stay in order; closer than that they may
/// coincide, which is an anomaly the model already names — and not a reversal.
/// That is why `mayBreakOrder` answers no for this kind, alone among the
/// operations that move positions.
///
/// Like the conversion and the transform, it retains the previous positions:
/// the operation rounds, so its inverse cannot be a second calculation.
class SnapCommand final : public Command {

public:
    /// Captures the positions of `selection` and the rate of `project`.
    SnapCommand(const Project& project, Selection selection, FrameRate rate);

    void apply(Project& project) override;

    void revert(Project& project) override;

    [[nodiscard]] CommandKind kind() const override { return CommandKind::Snap; }

    /// Reports a change of positions on the selected subtitles.
    ///
    /// The rate of the project changes too, and no `ChangeKind` says so — the
    /// reason `ConvertFrameRateCommand` gives holds here unchanged.
    [[nodiscard]] std::vector<Change> describe() const override {
        return {Change{.kind = ChangeKind::Positions, .subtitles = m_selection}};
    }

private:
    /// The two ends of one subtitle, as they stood before the alignment.
    struct Positions {
        Timestamp start;
        Timestamp end;
    };

    Selection m_selection;
    FrameRate m_rate;
    FrameRate m_previousFrameRate;
    std::vector<Positions> m_previous;
};

} // namespace subedit::core
