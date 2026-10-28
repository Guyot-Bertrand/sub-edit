#pragma once

#include <subedit/core/time/timestamp.hpp>
#include <subedit/core/transcription/alignment_engine.hpp>

#include <span>
#include <vector>

namespace subedit::core {

/// The times of one subtitle.
struct LineTimes {
    Timestamp start;
    Timestamp end;

    friend bool operator==(const LineTimes&, const LineTimes&) = default;
};

/// The times a line ends up with, and what had to be done to keep the file in order.
struct RetimedLine {
    LineTimes times;

    /// The line took the place the alignment found; otherwise it keeps its own times.
    bool moved = false;

    /// Its end was cut back to the start of the next line, which it would have overlapped.
    bool trimmed = false;

    /// It still overlaps the next line, which cannot be cured by cutting: the next one begins
    /// no later than this one does. Left as it is, and said.
    bool overlapping = false;

    friend bool operator==(const RetimedLine&, const RetimedLine&) = default;
};

/// Puts the found places on a file (ADR 0047): a line takes the place the alignment found, a line
/// not found keeps its times — **no position is invented, none is interpolated** — and a line that
/// would run into the next is cut back to where the next begins. Only starts and ends change.
///
/// `places` has one entry per line, as the port answers; a line with a start and no end, or the
/// reverse, is read as not found.
[[nodiscard]] std::vector<RetimedLine> retime(std::span<const LineTimes> originals,
                                              std::span<const AlignedLine> places);

} // namespace subedit::core
