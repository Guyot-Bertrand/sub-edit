#pragma once

#include <subedit/core/time/timestamp.hpp>

#include <cstddef>
#include <optional>
#include <string>
#include <string_view>

namespace subedit::core {

/// What one LRC line says, before its text is read.
struct LrcTimeLine {
    Timestamp start = Timestamp::origin();

    /// Where the text begins, just past the closing bracket.
    std::size_t textAt = 0;
};

/// Reads `[00:12.34]` at the head of a line, or nothing if that is not what it is.
///
/// **Shared with the detection.** One bracket holding a time is LRC, and
/// nothing else among the nine opens that way: MPL2 puts two whole numbers in
/// two brackets, MicroDVD the same grammar in braces.
///
/// **The minutes are the whole position, and they are not capped at sixty.**
/// This format writes no hours at all — `[62:03.00]` is what an hour and two
/// minutes looks like — so the field is read for as many digits as it holds.
/// Gaupol's pattern takes exactly two, which is why nothing past
/// `[99:59.99]` reads back there.
///
/// A tag line such as `[ar:Artist]` is not a time and is refused here; the
/// reader keeps those as the file's header.
[[nodiscard]] std::optional<LrcTimeLine> parseLrcTimeLine(std::string_view line);

/// Writes a position as LRC counts them: minutes, seconds, hundredths.
///
/// Rounded to the nearest hundredth, halves away from zero — the rule the rest
/// of the project follows. The minutes carry the hours, and are padded to two
/// digits without being cut at two.
[[nodiscard]] std::string lrcTimeOf(Timestamp position);

} // namespace subedit::core
