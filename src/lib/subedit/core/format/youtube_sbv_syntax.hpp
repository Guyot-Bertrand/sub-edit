#pragma once

#include <subedit/core/time/timestamp.hpp>

#include <optional>
#include <string_view>

namespace subedit::core {

/// What one YouTube SBV time line says.
struct YouTubeSbvTimeLine {
    Timestamp start;
    Timestamp end;
};

/// Reads `0:00:01.000,0:00:03.000`, or nothing if that is not what the line is.
///
/// **Shared by the reader and the detection**, as SubViewer 2's is, and for the
/// same reason: they ask the same question of a grammar small enough that two
/// copies would be two places to forget the same thing.
///
/// **The shape is what tells it from SubViewer 2**, whose line is also two
/// stamps and a comma. SubViewer 2 writes `HH:MM:SS.cc`, two decimals; SBV
/// writes three, and its hour as YouTube writes it, without padding. One or two
/// hour digits are read; only the shape with three decimals is this format.
[[nodiscard]] std::optional<YouTubeSbvTimeLine> parseYouTubeSbvTimeLine(std::string_view line);

} // namespace subedit::core
