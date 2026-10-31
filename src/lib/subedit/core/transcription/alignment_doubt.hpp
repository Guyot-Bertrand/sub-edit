#pragma once

#include <subedit/core/time/timestamp.hpp>
#include <subedit/core/transcription/word_matching.hpp>

#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <vector>

namespace subedit::core {

/// Below this fraction of its words heard, a line that was found is only partly heard (ADR 0047).
inline constexpr double kDefaultPartialBelow = 0.8;

/// How far a line's shift may depart from its neighbours' before it is out of line, in
/// milliseconds. A setting of phase 23; this is its default.
inline constexpr std::int64_t kDefaultDoubtGapMilliseconds = 2000;

/// How many lines on each side make the neighbours of a line.
inline constexpr std::size_t kDoubtNeighbours = 2;

/// Why the alignment of a line is not to be trusted, if it is not. The first that holds.
enum class LineDoubt : std::uint8_t {
    None,

    /// The matching gave no position: the line keeps its time.
    NotFound,

    /// Found, but fewer than `partialBelow` of its words were heard.
    Partial,

    /// Found, but its shift departs from its neighbours' by more than the gap.
    OutOfLine,
};

struct DoubtLimits {
    double partialBelow = kDefaultPartialBelow;
    std::int64_t gapMilliseconds = kDefaultDoubtGapMilliseconds;
};

struct LineVerdict {
    LineDoubt doubt = LineDoubt::None;

    /// How far the new start is from the original one, in milliseconds (negative: the line moves
    /// earlier). Nothing for a line not found.
    std::optional<std::int64_t> shiftMilliseconds{};

    friend bool operator==(const LineVerdict&, const LineVerdict&) = default;
};

/// What the alignment of a whole file looks like, line by line and as a figure or two.
struct AlignmentReport {
    std::vector<LineVerdict> lines;

    std::size_t sure = 0;
    std::size_t partial = 0;
    std::size_t outOfLine = 0;
    std::size_t notFound = 0;

    /// The median shift of the lines that have a position, in milliseconds: a constant offset
    /// reads here.
    std::int64_t medianShiftMilliseconds = 0;

    /// The median shift of the last tenth of the positioned lines minus that of the first tenth: a
    /// progressive drift reads here. Nothing to compare with fewer than two positioned lines.
    std::int64_t driftMilliseconds = 0;

    /// Lines that have a position: all but the ones not found.
    [[nodiscard]] std::size_t positioned() const { return sure + partial + outOfLine; }
};

/// Judges a matching. `originalStarts` are the times the file gives, one per line, in the same
/// order as `matches`; an extra or missing one is read as no position.
[[nodiscard]] AlignmentReport judgeAlignment(std::span<const LineMatch> matches,
                                             std::span<const Timestamp> originalStarts,
                                             const DoubtLimits& limits = {});

} // namespace subedit::core
