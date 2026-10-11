#include <subedit/core/wording/counts.hpp>
#include <subedit/gui/fixes_proposal_dialog.hpp>

#include <QCheckBox>
#include <QDialogButtonBox>
#include <QLabel>
#include <QPushButton>
#include <QString>
#include <QStringList>
#include <QVBoxLayout>

#include <cstddef>

namespace subedit::gui {

FixesProposalDialog::FixesProposalDialog(std::size_t repeated,
                                         std::size_t outOfOrder,
                                         std::size_t overlapping,
                                         QWidget* parent)
    : QDialog(parent) {
    setWindowTitle(QStringLiteral("Problems found in this file"));

    auto* intro =
        new QLabel{QStringLiteral("Opening this file found something that can be put right. "
                                  "Each correction can be undone."),
                   this};
    intro->setWordWrap(true);

    auto* buttons = new QDialogButtonBox{this};
    auto* fix = buttons->addButton(QStringLiteral("Fix"), QDialogButtonBox::AcceptRole);
    buttons->addButton(QStringLiteral("Keep as is"), QDialogButtonBox::RejectRole);
    fix->setDefault(true);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    auto* stack = new QVBoxLayout{this};
    stack->addWidget(intro);

    if (repeated > 0) {
        m_join = new QCheckBox{QStringLiteral("Join the repeated subtitles: ") +
                                   QString::fromStdString(core::countOf(repeated, "subtitle")) +
                                   QStringLiteral(" repeat one that they touch"),
                               this};
        m_join->setChecked(true);
        stack->addWidget(m_join);
    }
    if (outOfOrder > 0) {
        m_sort = new QCheckBox{QStringLiteral("Sort the subtitles: not in order of start (") +
                                   QString::fromStdString(core::countOf(outOfOrder, "subtitle")) +
                                   QStringLiteral(" out of place)"),
                               this};
        m_sort->setChecked(true);
        stack->addWidget(m_sort);
    }
    if (overlapping > 0) {
        const QString text = QStringLiteral("Make the ranges disjoint: ") +
                             QString::fromStdString(core::countOf(overlapping, "subtitle")) +
                             QStringLiteral(" start before the previous one ends");
        m_disjoint = new QCheckBox{text, this};
        m_disjoint->setChecked(true);
        stack->addWidget(m_disjoint);

        // Only in order: with the sort offered and not wanted, there is nothing
        // to make disjoint.
        if (m_sort != nullptr) {
            connect(m_sort, &QCheckBox::toggled, this, [this](bool sorted) {
                m_disjoint->setEnabled(sorted);
                if (!sorted)
                    m_disjoint->setChecked(false);
            });
        }
    }
    stack->addWidget(buttons);
}

QString FixesProposalDialog::message() const {
    QStringList lines;
    for (const QCheckBox* check : {m_join, m_sort, m_disjoint}) {
        if (check != nullptr)
            lines.push_back(check->text());
    }
    return lines.join(QLatin1Char('\n'));
}

bool FixesProposalDialog::joinChosen() const {
    return m_join != nullptr && m_join->isChecked();
}

bool FixesProposalDialog::sortChosen() const {
    return m_sort != nullptr && m_sort->isChecked();
}

bool FixesProposalDialog::disjointChosen() const {
    return m_disjoint != nullptr && m_disjoint->isChecked();
}

} // namespace subedit::gui
