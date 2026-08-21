#include <subedit/core/command/command_kind.hpp>
#include <subedit/gui/command_label.hpp>

#include <QString>

#include <optional>
#include <utility>

namespace subedit::gui {

namespace {

using core::CommandKind;

/// Builds « Annuler : décalage », or « Annuler » when there is nothing named.
///
/// The two actions read the same way, so they are written once: what differs
/// between them is a verb.
[[nodiscard]] QString actionLabel(const QString& verb, std::optional<CommandKind> kind) {
    if (!kind.has_value())
        return verb;

    return verb + QStringLiteral(" : ") + labelOf(*kind);
}

} // namespace

QString labelOf(CommandKind kind) {
    switch (kind) {
    case CommandKind::SetText:
        return QStringLiteral("modification du texte");
    case CommandKind::SetStart:
        return QStringLiteral("modification du début");
    case CommandKind::SetEnd:
        return QStringLiteral("modification de la fin");
    case CommandKind::Insert:
        return QStringLiteral("insertion");
    case CommandKind::Remove:
        return QStringLiteral("suppression");
    case CommandKind::Shift:
        return QStringLiteral("décalage");
    case CommandKind::Transform:
        return QStringLiteral("transformation");
    case CommandKind::ConvertFrameRate:
        return QStringLiteral("conversion de fréquence");
    case CommandKind::Sort:
        return QStringLiteral("tri");
    case CommandKind::RemoveHearingImpaired:
        return QStringLiteral("retrait des mentions");
    }

    // Les dix valeurs sont traitées et le compilateur le vérifie. Un `default`
    // à leur place accepterait en silence un énumérateur ajouté sans nom, et
    // l'action l'annoncerait par une chaîne vide.
    std::unreachable();
}

QString undoLabel(std::optional<CommandKind> kind) {
    return actionLabel(QStringLiteral("Annuler"), kind);
}

QString redoLabel(std::optional<CommandKind> kind) {
    return actionLabel(QStringLiteral("Rétablir"), kind);
}

} // namespace subedit::gui
