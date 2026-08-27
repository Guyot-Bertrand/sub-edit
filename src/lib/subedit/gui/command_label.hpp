#pragma once

#include <subedit/core/time/duration.hpp>

#include <QString>

#include <optional>

/// Declared rather than included: this header names the enumeration and
/// nothing else, and the core header that defines it costs a dependency the
/// window's other translation units do not need.
namespace subedit::core {
enum class CommandKind;
} // namespace subedit::core

namespace subedit::gui {

/// What the undo action reads: « Undo: shifting », or « Undo » alone when
/// there is nothing to defeat.
///
/// **Only the sentence lives here.** Naming the operation is `core::nameOf`,
/// which the command line uses too; putting a verb in front of it is what a
/// menu does, and no report has any use for that.
[[nodiscard]] QString undoLabel(std::optional<core::CommandKind> kind);

/// What the redo action reads.
[[nodiscard]] QString redoLabel(std::optional<core::CommandKind> kind);

/// What the « bring back onto the grid » action reads — « Shift onto Grid
/// (+0.001 s) », or the bare name when there is no grid to rejoin.
///
/// **The amount is in the label because there is no dialog.** The operation
/// takes no option, so asking for one would be a window to dismiss; but a user
/// is entitled to know what a menu entry will do before choosing it. The window
/// already words a menu this way for undo, and for the same reason.
[[nodiscard]] QString shiftOntoGridLabel(std::optional<core::Duration> by);

} // namespace subedit::gui
