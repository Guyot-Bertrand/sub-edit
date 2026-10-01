#pragma once

// What the status bar says of the film a document is watched against. Shared
// wording of `core/wording/`; see `formats.hpp` for why it lives in the core.

#include <subedit/core/time/frame_rate.hpp>

#include <filesystem>
#include <optional>
#include <string>

namespace subedit::core {

/// What the window says of the film a document is watched against.
///
/// Its name, or that there is none — and its name alone, not its path: the
/// line sits in a status bar, where a path of two hundred characters would push
/// out everything else. Whoever wants the path has the chooser that named it.
/// What the status bar says of the associated film: its name and the rate it
/// declares, or that there is none.
///
/// **The rate goes with the film rather than beside it.** They are one fact —
/// what this document accompanies — and a third widget would put the film's own
/// cadence at the same rank as the grid deduced from the positions, which is a
/// different fact entirely.
///
/// `declared` is nothing when no film is open, or when `ffprobe` is absent: the
/// name is then said alone, as it was before the rate could be read.
[[nodiscard]] std::string videoStatusOf(const std::optional<std::filesystem::path>& video,
                                        std::optional<FrameRate> declared = {});

} // namespace subedit::core
