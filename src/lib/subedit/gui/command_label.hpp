#pragma once

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

} // namespace subedit::gui
