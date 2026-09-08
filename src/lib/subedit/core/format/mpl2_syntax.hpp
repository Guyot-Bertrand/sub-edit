#pragma once

#include <subedit/core/time/timestamp.hpp>

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>

namespace subedit::core {

/// What one MPL2 line says, before its text is read.
struct Mpl2TimeLine {
    Timestamp start = Timestamp::origin();
    Timestamp end = Timestamp::origin();

    /// Where the text begins, just past the second bracket.
    std::size_t textAt = 0;
};

/// Reads `[10][30]` at the head of a line, or nothing if that is not what it is.
///
/// **Shared with the detection**, which asks exactly this question and no
/// other: a line opening on two bracketed whole numbers is MPL2, and nothing
/// else in the nine is written that way. LRC opens on one bracket holding a
/// time, MicroDVD on two braces.
[[nodiscard]] std::optional<Mpl2TimeLine> parseMpl2TimeLine(std::string_view line);

/// Writes a position as MPL2 counts them: whole tenths of a second.
///
/// Rounded to the nearest tenth, halves away from zero — the rule the rest of
/// the project follows. A hundred milliseconds is the grain, and what falls
/// between two of them has to land on one.
[[nodiscard]] std::string tenthsOf(Timestamp position);

} // namespace subedit::core
