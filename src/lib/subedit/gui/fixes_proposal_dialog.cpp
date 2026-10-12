#include <subedit/gui/fixes_proposal_dialog.hpp>
#include <subedit/gui/messages.hpp>

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
    setWindowTitle(translate("Problems found in this file"));

    auto* intro = new QLabel{translate("Opening this file found something that can be put right. "
                                       "Each correction can be undone."),
                             this};
    intro->setWordWrap(true);

    auto* buttons = new QDialogButtonBox{this};
    auto* fix = buttons->addButton(translate("Fix"), QDialogButtonBox::AcceptRole);
    buttons->addButton(translate("Keep as is"), QDialogButtonBox::RejectRole);
    fix->setDefault(true);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    auto* stack = new QVBoxLayout{this};
    stack->addWidget(intro);

    if (repeated > 0) {
        const QString text = substitute(
            translatePlural("Join the repeated subtitles: %1 subtitle repeat one that they touch",
                            "Join the repeated subtitles: %1 subtitles repeat one that they touch",
                            repeated),
            QString::number(repeated));
        m_join = new QCheckBox{text, this};
        m_join->setChecked(true);
        stack->addWidget(m_join);
    }
    if (outOfOrder > 0) {
        const QString text = substitute(
            translatePlural("Sort the subtitles: not in order of start (%1 subtitle out of place)",
                            "Sort the subtitles: not in order of start (%1 subtitles out of place)",
                            outOfOrder),
            QString::number(outOfOrder));
        m_sort = new QCheckBox{text, this};
        m_sort->setChecked(true);
        stack->addWidget(m_sort);
    }
    if (overlapping > 0) {
        const QString text = substitute(
            translatePlural(
                "Make the ranges disjoint: %1 subtitle start before the previous one ends",
                "Make the ranges disjoint: %1 subtitles start before the previous one ends",
                overlapping),
            QString::number(overlapping));
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
