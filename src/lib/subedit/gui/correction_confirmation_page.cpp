#include <subedit/core/text/correction_run.hpp>
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

[[nodiscard]] QString reasonOf(core::FailureKind kind) {
    switch (kind) {
    case core::FailureKind::Untranslatable:
        return QStringLiteral("cannot be translated");
    case core::FailureKind::CompileError:
        return QStringLiteral("will not compile");
    case core::FailureKind::InvalidReplacement:
        return QStringLiteral("has an invalid replacement");
    case core::FailureKind::TimedOut:
        return QStringLiteral("timed out");
    case core::FailureKind::TooManyPasses:
        return QStringLiteral("never settled");
    case core::FailureKind::TooLong:
        return QStringLiteral("grew the text too long");
    }
    return {}; // unreachable: every enumerator is handled above
}

[[nodiscard]] QString abandonedText(const std::vector<core::PatternFailure>& failures) {
    QStringList lines;
    for (const core::PatternFailure& failure : failures)
        lines << QString::fromStdString(failure.name) + " (" + reasonOf(failure.kind) + ")";
    return QStringLiteral("Not applied: ") + lines.join(QStringLiteral(", "));
}

} // namespace

CorrectionConfirmationPage::CorrectionConfirmationPage(QWidget* parent)
    : QWizardPage(parent),
      m_table(new QTableView{this}),
      m_markAll(new QPushButton{QStringLiteral("Mark All"), this}),
      m_unmarkAll(new QPushButton{QStringLiteral("Unmark All"), this}),
      m_preview(new QPushButton{QStringLiteral("Preview"), this}),
      m_removeBlank(new QCheckBox{QStringLiteral("Remove all blank subtitles"), this}),
      m_abandoned(new QLabel{this}) {
    setTitle(QStringLiteral("Confirmation"));
    m_removeBlank->setChecked(true);
    m_abandoned->setObjectName(QStringLiteral("abandonedPatterns"));
    m_abandoned->setWordWrap(true);
    m_abandoned->hide();

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

void CorrectionConfirmationPage::initializePage() {
    delete m_model;
    m_model = new CorrectionResultModel{m_progress->result().corrections, this};
    m_table->setModel(m_model);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    auto* delegate = new CorrectionDiffDelegate{m_table};
    m_table->setItemDelegateForColumn(CorrectionResultModel::Original, delegate);
    m_table->setItemDelegateForColumn(CorrectionResultModel::Proposed, delegate);

    const bool anyFailure = !m_progress->result().failures.empty();
    m_abandoned->setVisible(anyFailure);
    if (anyFailure)
        m_abandoned->setText(abandonedText(m_progress->result().failures));
}

bool CorrectionConfirmationPage::removeBlankSubtitles() const {
    return m_removeBlank->isChecked();
}

} // namespace subedit::gui
