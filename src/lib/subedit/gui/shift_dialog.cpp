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
    // Read as a timestamp, sign included: a signed duration is written like a
    // position, and the core's reader is already as permissive as real files
    // require.
    const std::optional<core::Timestamp> read = core::Timestamp::parse(m_by->text().toStdString());
    if (!read.has_value())
        return std::nullopt;

    return core::Duration::fromMilliseconds(read->milliseconds());
}

bool ShiftDialog::isComplete() const {
    const std::optional<core::Duration> by = shift();

    // Shifting by nothing is no operation: letting it be validated would put
    // an entry in the history for an unchanged file.
    return by.has_value() && by->milliseconds() != 0;
}

void ShiftDialog::setTyped(const QString& text) {
    m_by->setText(text);
}

} // namespace subedit::gui
