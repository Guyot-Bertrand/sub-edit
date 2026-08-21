#pragma once

#include <subedit/core/model/subtitle_index.hpp>
#include <subedit/core/time/duration.hpp>

#include <optional>

namespace subedit::core {

class Project;
class Selection;

/// Names the first selected subtitle a shift by `by` would take before the
/// start of the video, or nothing if none would.
///
/// **The command itself does not refuse it, and that is deliberate.** A
/// position before the origin is representable — `ShiftCommand` says why, and
/// refusing it there would turn an editing operation into a special case. But
/// no subtitle *file* can hold one, so whoever is about to write a file has to
/// look before applying. That is a caller's rule, and this is where callers
/// share it.
///
/// **The first, and not any of them**: it is the one the user has to look at
/// to see by how much they overshot.
///
/// Landing exactly on the origin is allowed. Zero is a position, and refusing
/// it would turn a bound into a prohibition.
[[nodiscard]] std::optional<SubtitleIndex>
firstBeforeOrigin(const Project& project, const Selection& selection, Duration by);

} // namespace subedit::core
