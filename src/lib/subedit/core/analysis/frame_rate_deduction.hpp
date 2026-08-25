#pragma once

#include <subedit/core/model/subtitle_index.hpp>
#include <subedit/core/time/duration.hpp>
#include <subedit/core/time/frame_rate.hpp>
#include <subedit/core/time/timestamp.hpp>

#include <array>
#include <cstddef>
#include <optional>
#include <span>
#include <vector>

namespace subedit::core {

/// How well the positions of a document fit the grid of one candidate rate.
struct GridFit {
    FrameRate rate;

    /// The concentration of the phases, from 0 to 100.
    ///
    /// One hundred is a perfect grid, whatever its phase; near zero is no
    /// structure at all. It is the length of the mean unit vector, and it has
    /// **no tolerance parameter**: nothing here is tuned.
    double concentration;

    /// The constant offset between the positions and the grid.
    ///
    /// Not waste. A file whose positions sit on the grid to within a constant
    /// has been shifted, and the constant is what shifts it back.
    Duration phase;
};

/// What the deduction concludes about a document as a whole.
///
/// **The thresholds that separate these three live in the implementation, and
/// deliberately so.** A caller that could read them could re-implement the
/// verdict, and four surfaces re-implementing the same two numbers end up
/// disagreeing. Ask for the verdict; do not recompute it.
enum class GridVerdict {
    Clean,   ///< the positions are on a grid
    Partial, ///< a part of them is, and the rest is not
    Silent,  ///< no candidate explains anything, which is an answer
};

/// Everything the deduction found, hiding nothing.
struct FrameRateDeduction {
    /// The eight candidates, ordered by decreasing concentration.
    ///
    /// The whole ranking rather than a single answer: it costs eight numbers
    /// and it lets a reader see what was close.
    std::array<GridFit, kStandardFrameRates.size()> ranked;

    /// The answer, which is **not always `ranked.front()`**.
    ///
    /// A grid at 25 frames per second is included in a grid at 50, so a file
    /// written on 25 scores 100 on both, and rounding decides which sorts
    /// first. The implication holds one way only — a file truly on 50 collapses
    /// on 25 — so the lower rate is the parsimonious reading, and it is the one
    /// retained whenever both members of the pair land in the same band.
    GridFit retained;

    GridVerdict verdict = GridVerdict::Silent;

    /// The multiple set aside by that rule, when it applied.
    ///
    /// Named rather than dropped: an ambiguity the user is not told about is
    /// an ambiguity they cannot check.
    std::optional<FrameRate> harmonic{};

    /// The candidates this span is too short to tell apart from the retained
    /// one.
    ///
    /// 23.976 and 24 drift by one millisecond per second of film, and a frame
    /// lasts 41.7 of them: the phase on the wrong candidate makes a full turn
    /// every forty-two seconds. Over a feature film they separate without
    /// appeal; over ten seconds they do not, and answering "24" without saying
    /// so would be lying by omission.
    std::vector<FrameRate> notSeparated{};

    /// Between the first start and the last.
    Duration span = Duration::zero();

    /// How many starts the deduction had to work with.
    std::size_t starts = 0;

    /// The starts that leave the retained grid, in order.
    ///
    /// Empty on a clean file, and on a silent one — there, everything would be
    /// a stray, which says nothing. Counting the runs of consecutive entries
    /// separates the two causes of a partial file: many runs of one are
    /// positions corrected by hand, a few long runs are a section retimed.
    std::vector<SubtitleIndex> strays{};
};

/// Deduces which frame rate grid the starts of a document were written on.
///
/// **The starts, and not the ends.** Wherever a grid exists the starts sit on
/// it without exception, while the ends run from 55 to 100: a *cue-out* is
/// often computed by a reading-speed rule rather than placed on a frame.
/// Mixing them would lower a clean signal with known noise.
///
/// Pure, and it stays that way: no project, no session, no history. It takes
/// positions and returns a judgement, which is what makes it testable against
/// thirteen files and a table of constants.
[[nodiscard]] FrameRateDeduction deduceFrameRate(std::span<const Timestamp> starts);

} // namespace subedit::core
