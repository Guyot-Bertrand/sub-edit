#include <subedit/core/command/command_kind.hpp>
#include <subedit/core/wording/analysis.hpp>
#include <subedit/gui/command_label.hpp>
#include <subedit/gui/messages.hpp>

#include <QString>

#include <optional>

namespace subedit::gui {

namespace {

using core::CommandKind;

/// Builds « Undo: shifting », or « Undo » when there is nothing named.
///
/// The two actions read the same way, so they are written once: what differs
/// between them is a verb.
[[nodiscard]] QString actionLabel(const QString& verb, std::optional<CommandKind> kind) {
    if (!kind.has_value())
        return verb;

    // TRANSLATORS: %1 is Undo or Redo, %2 the operation it undoes or redoes, such as shifting.
    return substitute(translate("%1: %2"), verb, QString::fromUtf8(core::nameOf(*kind)));
}

} // namespace

QString undoLabel(std::optional<CommandKind> kind) {
    return actionLabel(translate("Undo"), kind);
}

QString redoLabel(std::optional<CommandKind> kind) {
    return actionLabel(translate("Redo"), kind);
}

QString shiftOntoGridLabel(std::optional<core::Duration> by) {
    if (!by.has_value())
        return translate("Shift Whole File onto Grid");

    // TRANSLATORS: %1 is how far the whole file moves, in seconds.
    return substitute(translate("Shift Whole File onto Grid (%1)"),
                      QString::fromStdString(core::secondsOf(*by)));
}

} // namespace subedit::gui
