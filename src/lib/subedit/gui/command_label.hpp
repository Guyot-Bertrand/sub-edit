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

/// Names an operation, in the language of the interface.
///
/// **A free function of `subedit::gui`, and that is the point.** The core
/// speaks no French — `CommandKind` exists precisely so that the wording lives
/// where the wording belongs, and so that a test can hold on to the
/// enumerator rather than to prose that will be reworded.
[[nodiscard]] QString labelOf(core::CommandKind kind);

/// What the undo action reads: « Annuler : décalage », or « Annuler » alone
/// when there is nothing to defeat.
[[nodiscard]] QString undoLabel(std::optional<core::CommandKind> kind);

/// What the redo action reads.
[[nodiscard]] QString redoLabel(std::optional<core::CommandKind> kind);

} // namespace subedit::gui
