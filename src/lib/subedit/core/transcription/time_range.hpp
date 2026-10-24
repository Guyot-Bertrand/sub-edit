#pragma once

#include <subedit/core/time/timestamp.hpp>

#include <optional>
#include <string_view>

namespace subedit::core {

/// A bound of the part of a film to transcribe, as a person types it: `2:00`, `1:02:03`,
/// `2:00.5`, or a number of seconds, `120`, `90.5`.
///
/// **Nothing before the origin, and only a point as the decimal mark**, as everywhere a time
/// is typed (spec 15, D3). Nothing for text that is none of these.
[[nodiscard]] std::optional<Timestamp> readRangeBound(std::string_view text);

/// A number of seconds as a position, to the nearest millisecond.
[[nodiscard]] Timestamp timestampFromSeconds(double seconds);

/// Whether `start` and `end` can delimit something: the end comes after the start. A range
/// with one bound or none always can.
[[nodiscard]] bool isRangePossible(const std::optional<Timestamp>& start,
                                   const std::optional<Timestamp>& end);

} // namespace subedit::core
