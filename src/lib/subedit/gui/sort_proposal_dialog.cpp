#include <subedit/core/wording/counts.hpp>
#include <subedit/gui/sort_proposal_dialog.hpp>

#include <QDialogButtonBox>
#include <QLabel>
#include <QPushButton>
#include <QString>
#include <QVBoxLayout>

#include <cstddef>

namespace subedit::gui {

SortProposalDialog::SortProposalDialog(std::size_t outOfOrder, QWidget* parent) : QDialog(parent) {
    setWindowTitle(QStringLiteral("Subtitles out of order"));

    auto* label = new QLabel{
        QStringLiteral("This file is not in order of start (") +
            QString::fromStdString(core::countOf(outOfOrder, "subtitle")) +
            QStringLiteral(" out of place).\nSort the subtitles now? The change can be undone."),
        this};
    label->setObjectName(QStringLiteral("message"));
    label->setWordWrap(true);

    auto* buttons = new QDialogButtonBox{this};
    auto* sort = buttons->addButton(QStringLiteral("Sort"), QDialogButtonBox::AcceptRole);
    buttons->addButton(QStringLiteral("Keep as is"), QDialogButtonBox::RejectRole);
    sort->setDefault(true);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    auto* stack = new QVBoxLayout{this};
    stack->addWidget(label);
    stack->addWidget(buttons);
}

QString SortProposalDialog::message() const {
    return findChild<QLabel*>(QStringLiteral("message"))->text();
}

} // namespace subedit::gui
