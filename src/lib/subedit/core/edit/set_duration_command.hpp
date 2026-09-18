#pragma once

#include <subedit/core/command/command.hpp>
#include <subedit/core/model/subtitle_index.hpp>
#include <subedit/core/time/duration.hpp>

#include <memory>

namespace subedit::core {

class Project;

/// Builds the command that gives one subtitle a duration, by moving its end.
///
/// **The end, and never the start** — decision D3 of the phase-10 spec. It is
/// the one end `adjust_durations` moves, moving the start would move the
/// subtitle, which is the work of a shift, and the start already has a command
/// of its own.
///
/// Built on `SetPositionCommand`, which already retains the **old end, and
/// nothing else** to undo itself.
///
/// **An overlap with the next subtitle is allowed.** `scanAnomalies` reports it,
/// and refusing an edit whose consequence shows would refuse what the user may
/// want — decision D4 of phase 5.
[[nodiscard]] std::unique_ptr<Command>
setDuration(const Project& project, SubtitleIndex index, Duration duration);

} // namespace subedit::core
