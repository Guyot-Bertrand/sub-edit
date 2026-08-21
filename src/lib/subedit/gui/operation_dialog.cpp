#include <subedit/core/wording.hpp>
#include <subedit/gui/operation_dialog.hpp>

#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLabel>
#include <QPushButton>
#include <QString>
#include <QVBoxLayout>

#include <cstddef>

namespace subedit::gui {

OperationDialog::OperationDialog(std::size_t targetCount, QWidget* parent)
    : QDialog(parent),
      m_targetCount(targetCount),
      m_fields(new QFormLayout),
      m_buttons(new QDialogButtonBox{QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this}) {
    connect(m_buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(m_buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

QString OperationDialog::targetLabel() const {
    return QString::fromStdString(core::countOf(m_targetCount, "subtitle"));
}

void OperationDialog::revalidate() {
    m_buttons->button(QDialogButtonBox::Ok)->setEnabled(isComplete());
}

void OperationDialog::finish() {
    auto* stack = new QVBoxLayout{this};
    stack->addLayout(m_fields);
    stack->addWidget(new QLabel{QStringLiteral("Applies to: ") + targetLabel(), this});
    stack->addWidget(m_buttons);

    revalidate();
}

} // namespace subedit::gui
