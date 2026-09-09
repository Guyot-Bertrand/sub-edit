#pragma once

#include <subedit/core/time/timestamp.hpp>

#include <cstddef>
#include <optional>
#include <string>
#include <string_view>

namespace subedit::core {

/// What one TMPlayer line says, before its text is read.
struct TMPlayerTimeLine {
    Timestamp start = Timestamp::origin();

    /// Whether the hour was written on two digits, `00:00:12:`, or on one.
    ///
    /// **A shape, not a value**, and the file is the only place it is written.
    /// Both forms are TMPlayer, a file uses one of them throughout, and a
    /// reading that forgot which would write the other one back and change
    /// every line. ADR 0030 is where that kind of thing is kept.
    bool twoDigitHour = true;

    /// Where the text begins, just past the fourth colon.
    std::size_t textAt = 0;
};

/// Reads `00:00:12:` at the head of a line, or nothing if that is not what it is.
///
/// **Shared with the detection**, which asks exactly this question: three
/// fields and a closing colon opening a line is TMPlayer, and nothing else
/// among the nine is written that way — SubViewer 2 puts a period after its
/// seconds, and the two Sub Station Alpha formats put their times inside a
/// `Dialogue:` line rather than at the head of one.
///
/// Out-of-range minutes and seconds are refused rather than folded, as
/// `Timestamp::parse` refuses them: the caller can then report the line
/// instead of inventing a position for it.
[[nodiscard]] std::optional<TMPlayerTimeLine> parseTMPlayerTimeLine(std::string_view line);

/// Writes a position as TMPlayer counts them: whole seconds, and no decimals.
///
/// Rounded to the nearest second, halves away from zero — the rule the rest of
/// the project follows.
[[nodiscard]] std::string tmPlayerTimeOf(Timestamp position, bool twoDigitHour);

} // namespace subedit::core
