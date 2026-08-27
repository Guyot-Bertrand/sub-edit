#include <subedit/core/command/command_kind.hpp>
#include <subedit/core/wording.hpp>
#include <subedit/gui/command_label.hpp>

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

    return verb + QStringLiteral(": ") + QString::fromUtf8(core::nameOf(*kind));
}

} // namespace

QString undoLabel(std::optional<CommandKind> kind) {
    return actionLabel(QStringLiteral("Undo"), kind);
}

QString redoLabel(std::optional<CommandKind> kind) {
    return actionLabel(QStringLiteral("Redo"), kind);
}

QString shiftOntoGridLabel(std::optional<core::Duration> by) {
    if (!by.has_value())
        return QStringLiteral("Shift onto Grid");

    return QStringLiteral("Shift onto Grid (%1)").arg(QString::fromStdString(core::secondsOf(*by)));
}

} // namespace subedit::gui
