#include <subedit/core/text/correction_run.hpp>
#include <subedit/core/text/pattern_catalogue.hpp>
#include <subedit/core/wording/correction.hpp>
#include <subedit/gui/correction_confirmation_page.hpp>
#include <subedit/gui/correction_progress_page.hpp>
#include <subedit/gui/correction_result_model.hpp>

#include <QCheckBox>
#include <QHBoxLayout>
#include <QItemSelectionModel>
#include <QLabel>
#include <QPushButton>
#include <QStringList>
#include <QTableView>
#include <QVBoxLayout>

namespace subedit::gui {

namespace {

[[nodiscard]] QString abandonedText(const std::vector<core::PatternFailure>& failures,
                                    const QStringList& unreadable) {
    QStringList paragraphs;
    if (!failures.empty()) {
        QStringList lines;
        for (const core::PatternFailure& failure : failures)
            lines << QString::fromStdString(failure.name) + " (" +
                         QString::fromUtf8(core::reasonOf(failure.kind).data()) + ")";
        paragraphs << QStringLiteral("Not applied: ") + lines.join(QStringLiteral(", "));
    }
    if (!unreadable.isEmpty())
        paragraphs << QStringLiteral("Could not be read: ") + unreadable.join(QStringLiteral(", "));
    return paragraphs.join(QStringLiteral("\n"));
}

} // namespace

CorrectionConfirmationPage::CorrectionConfirmationPage(QWidget* parent)
    : QWizardPage(parent),
      m_table(new QTableView{this}),
      m_markAll(new QPushButton{QStringLiteral("Mark All"), this}),
      m_unmarkAll(new QPushButton{QStringLiteral("Unmark All"), this}),
      m_preview(new QPushButton{QStringLiteral("Preview"), this}),
      m_removeBlank(new QCheckBox{QStringLiteral("Remove all blank subtitles"), this}),
      m_abandoned(new QLabel{this}),
      m_delegate(new CorrectionDiffDelegate{m_table}) {
    setTitle(QStringLiteral("Confirmation"));
    m_removeBlank->setChecked(true);
    m_abandoned->setObjectName(QStringLiteral("abandonedPatterns"));
    m_abandoned->setWordWrap(true);
    m_abandoned->hide();
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setItemDelegateForColumn(CorrectionResultModel::Original, m_delegate);
    m_table->setItemDelegateForColumn(CorrectionResultModel::Proposed, m_delegate);

    auto* buttons = new QHBoxLayout{};
    buttons->addWidget(m_markAll);
    buttons->addWidget(m_unmarkAll);
    buttons->addStretch();
    buttons->addWidget(m_preview);

    auto* layout = new QVBoxLayout{this};
    layout->addLayout(buttons);
    layout->addWidget(m_table);
    layout->addWidget(m_removeBlank);
    layout->addWidget(m_abandoned);

    connect(m_markAll, &QPushButton::clicked, this, [this] { m_model->markAll(true); });
    connect(m_unmarkAll, &QPushButton::clicked, this, [this] { m_model->markAll(false); });
    connect(m_preview, &QPushButton::clicked, this, [this] {
        const QModelIndexList selected = m_table->selectionModel()->selectedRows();
        if (!selected.isEmpty())
            emit previewRequested(selected.first().row());
    });
}

void CorrectionConfirmationPage::setRemoveBlankSubtitlesDefault(bool removeBlank) {
    m_removeBlank->setChecked(removeBlank);
}

void CorrectionConfirmationPage::setReadDiagnostics(
    const std::vector<core::PatternDiagnostic>& diagnostics) {
    m_unreadable.clear();
    for (const core::PatternDiagnostic& diagnostic : diagnostics)
        m_unreadable << QString::fromStdString(core::describe(diagnostic));
}

void CorrectionConfirmationPage::initializePage() {
    delete m_model;
    m_model = new CorrectionResultModel{m_progress->result().corrections, this};
    m_table->setModel(m_model);

    const bool anything = !m_progress->result().failures.empty() || !m_unreadable.isEmpty();
    m_abandoned->setVisible(anything);
    if (anything)
        m_abandoned->setText(abandonedText(m_progress->result().failures, m_unreadable));
}

bool CorrectionConfirmationPage::removeBlankSubtitles() const {
    return m_removeBlank->isChecked();
}

} // namespace subedit::gui
