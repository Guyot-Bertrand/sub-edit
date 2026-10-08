#pragma once

// Where the player is sent to, and what is selected or inserted from where it is — the rules
// `VideoPane` and `MpvPlayer` used to carry in their own `.cpp` (issue #644). Positions in,
// positions out: none of it needs a window, a table or a film to be tried.

#include <subedit/core/model/subtitle.hpp>
#include <subedit/core/time/duration.hpp>
#include <subedit/core/time/frame_rate.hpp>
#include <subedit/core/time/timestamp.hpp>

#include <cstddef>
#include <optional>
#include <span>

namespace subedit::core {

/// How long a subtitle inserted at the position of the film lasts, when nothing stops it sooner.
inline constexpr std::int64_t kInsertedLengthMilliseconds = 3000;

/// How close to a position a subtitle must start, or have ended, to count as ahead of it or behind
/// it: Gaupol's millisecond, so that the neighbour of a position that is exactly a start is not
/// that very subtitle.
inline constexpr std::int64_t kNeighbourMarginMilliseconds = 1;

/// Where `Seek Previous` and `Seek Next` go: the start of the subtitle that comes after `where`
/// (`next`), or the start of the last one that ended before it.
///
/// **Walked and not searched**: a project may be out of order (ADR 0008), and « the first that
/// starts after » means the smallest start, not the first one met. Nothing when there is none.
[[nodiscard]] std::optional<Timestamp>
neighbourStart(std::span<const Subtitle> subtitles, Timestamp where, bool next);

/// The row `Select Previous` and `Select Next` pick from the position: the first that starts after
/// (`next`), or the last that started before — **by the order of the file**, as Gaupol reads it —
/// and the end of the file on that side when there is none. Nothing for no subtitles at all.
[[nodiscard]] std::optional<std::size_t>
rowFrom(std::span<const Subtitle> subtitles, Timestamp where, bool next);

/// Where a subtitle inserted at the position goes, and where it ends.
struct Insertion {
    /// The place in the list: after every subtitle that starts at or before the position.
    std::size_t rank;

    /// Three seconds after the position, or the start of the subtitle it lands before.
    Timestamp end;
};

/// Counted and not searched, as the neighbours are: a project may be out of order.
[[nodiscard]] Insertion insertionAt(std::span<const Subtitle> subtitles, Timestamp where);

/// `where` moved by `seconds`, earlier for a negative `direction`, and kept inside the film.
[[nodiscard]] Timestamp jumpedBy(Timestamp where, Duration length, int seconds, int direction);

/// An edge, taken back by the context length so that a subtitle is seen coming — never before the
/// start of the film.
[[nodiscard]] Timestamp withLeadIn(Timestamp edge, std::int64_t contextMilliseconds);

/// `here` moved by `frames` frames of `rate` — earlier when negative — and held inside the film:
/// never before the origin, never past **the last frame, which starts one frame before the end**.
///
/// Exact, like `movedByFrames`, which it extends: one rounding, from the rational.
[[nodiscard]] Timestamp steppedTo(Timestamp here, Duration length, FrameRate rate, int frames);

/// The frame rate a player reports as a number of frames per second, or nothing when it is no
/// rate at all.
///
/// **A standard rate when it is within a ten-thousandth of one** — mpv says 23.976023976…, and
/// the rate is 24000/1001; anything else is read to the thousandth. Floating point stops here: a
/// step counted from this rate is a rational computation.
[[nodiscard]] std::optional<FrameRate> frameRateNear(double framesPerSecond);

} // namespace subedit::core
