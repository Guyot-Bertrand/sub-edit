#pragma once

#include <subedit/core/time/frame_rate.hpp>
#include <subedit/core/time/timestamp.hpp>

#include <optional>
#include <span>

namespace subedit::core {

class Project;

/// A conversion from one frame rate to another, as `Convert Frame Rate…` states it.
struct RateConversion {
    FrameRate input;
    FrameRate output;

    friend bool operator==(const RateConversion&, const RateConversion&) = default;
};

/// What the search for the conversion that puts a file back on a grid found.
enum class RepairOutcome {
    /// Nothing to repair: the positions are on a grid already, or there are too few of them to
    /// judge by.
    NotNeeded,

    /// No conversion of the closed set puts the positions on a grid.
    NothingFits,

    /// Two conversions put them on a grid, and nothing tells them apart.
    Tied,

    /// One does.
    Found,
};

/// The conversion that would put a file back on a grid, and how well — decision D11 of the
/// phase-14 spec, issue #386.
struct GridRepair {
    RepairOutcome outcome = RepairOutcome::NotNeeded;

    /// The conversion to apply: the best one when `Found`, one of the two when `Tied`.
    std::optional<RateConversion> conversion{};

    /// The other, when `Tied`.
    std::optional<RateConversion> rival{};

    /// The grid the positions would be on once converted.
    std::optional<FrameRate> onto{};

    /// How well the positions fit that grid, from 0 to 100 — the deduction's own measure.
    double concentration = 0.0;

    /// How well the next best conversion fits, and how well the positions fit as they are.
    ///
    /// **The gap that separates the answer from the others and from doing nothing**, in the
    /// deduction's vocabulary: the confidence is what the deduction would say, never a number
    /// made up for the occasion.
    double runnerUp = 0.0;
    double asIs = 0.0;
};

/// Looks for the conversion of the closed set — the fifty-six ordered pairs of the eight
/// normalised rates — that puts `starts` on a grid.
///
/// **Each conversion is applied to the positions and the deduction of phase 16 judges the
/// result**; nothing is scored by anything else. A conversion fits when the deduction calls
/// the converted positions clean.
///
/// **Pairs that make the same ratio are one conversion**: 30 to 25 and 60 to 50 move every
/// position alike, and telling them apart would be telling a conversion from itself. The pair
/// kept is the one whose output is the grid the positions land on, and the first otherwise.
///
/// **A conversion is only proposed when it is the only one that fits.** Two that do — a span
/// too short to tell 23.976 from 24 — are `Tied`, and none is proposed. Nothing is looked for
/// in a file that is on a grid already, or that is too short for a verdict.
[[nodiscard]] GridRepair findGridRepair(std::span<const Timestamp> starts);

/// The same, on the starts of `project`.
[[nodiscard]] GridRepair findGridRepair(const Project& project);

} // namespace subedit::core
