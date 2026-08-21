#include <subedit/core/time/duration.hpp>
#include <subedit/core/time/timestamp.hpp>
#include <subedit/gui/shift_dialog.hpp>

#include <QFormLayout>
#include <QLineEdit>
#include <QString>

#include <cstddef>
#include <optional>

namespace subedit::gui {

ShiftDialog::ShiftDialog(std::size_t targetCount, QWidget* parent)
    : OperationDialog(targetCount, parent), m_by(new QLineEdit{this}) {
    setWindowTitle(QStringLiteral("Shift positions"));

    m_by->setPlaceholderText(QStringLiteral("00:00:02,500"));
    connect(m_by, &QLineEdit::textChanged, this, [this] { revalidate(); });

    fields()->addRow(QStringLiteral("Shift by"), m_by);
    finish();
}

std::optional<core::Duration> ShiftDialog::shift() const {
    // Lu comme un horodatage, signe compris : une durée signée s'écrit comme
    // une position, et le lecteur du noyau est déjà permissif comme les
    // fichiers réels l'exigent.
    const std::optional<core::Timestamp> read = core::Timestamp::parse(m_by->text().toStdString());
    if (!read.has_value())
        return std::nullopt;

    return core::Duration::fromMilliseconds(read->milliseconds());
}

bool ShiftDialog::isComplete() const {
    const std::optional<core::Duration> by = shift();

    // Décaler de rien n'est pas une opération : le laisser valider mettrait
    // une entrée dans l'historique pour un fichier inchangé.
    return by.has_value() && by->milliseconds() != 0;
}

void ShiftDialog::setTyped(const QString& text) {
    m_by->setText(text);
}

} // namespace subedit::gui
