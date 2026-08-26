#pragma once

#include <subedit/core/analysis/frame_rate_deduction.hpp>
#include <subedit/core/time/duration.hpp>

#include <optional>

namespace subedit::core {

/// The shortest shift that puts the positions back onto their deduced grid, or
/// nothing when no grid was found.
///
/// A file whose positions sit on a grid to within a constant has been shifted,
/// and the constant is what shifts it back. The deduction measures it as a
/// phase; this turns that phase into an instruction.
///
/// **The shortest, and not simply the opposite of the phase.** Past halfway
/// through a frame, the nearer grid line is the next one, so the correction
/// goes forwards rather than back by almost a whole frame. Nothing ever moves
/// by more than half a frame — 21 milliseconds at 24 frames per second.
///
/// **Nothing when the verdict is silent.** There is no grid to rejoin, and a
/// phase measured on noise would move the file by an arbitrary amount. A caller
/// that offers this correction has to offer it as unavailable, not as zero: the
/// two say different things.
///
/// It returns the amount rather than applying it, and that is the point. The
/// operation itself is a `ShiftCommand` like any other — writing a second one
/// would duplicate a behaviour already proved, down to the warning about
/// passing the end of the film. What was missing was never the shift; it was
/// knowing by how much.
[[nodiscard]] std::optional<Duration> shiftOntoGrid(const FrameRateDeduction& deduction);

} // namespace subedit::core
