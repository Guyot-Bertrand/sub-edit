#include <subedit/core/text/correction_run.hpp>
#include <subedit/core/text/pattern_catalogue.hpp>
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

[[nodiscard]] QString reasonOf(core::PatternProblem problem) {
    switch (problem) {
    case core::PatternProblem::DirectoryUnreadable:
        return QStringLiteral("directory cannot be read");
    case core::PatternProblem::FileUnreadable:
        return QStringLiteral("file cannot be read");
    case core::PatternProblem::MalformedLine:
        return QStringLiteral("malformed line");
    case core::PatternProblem::FieldOutsideRecord:
        return QStringLiteral("field outside any pattern");
    case core::PatternProblem::UnknownField:
        return QStringLiteral("unknown field");
    case core::PatternProblem::MissingField:
        return QStringLiteral("missing field");
    case core::PatternProblem::InvalidValue:
        return QStringLiteral("invalid value");
    case core::PatternProblem::MalformedActivation:
        break; // answered below, so that no unreachable line is left after the switch
    }
    return QStringLiteral("malformed activation");
}

/// `Zyyy.common-error, line 4 (malformed line: detail)` — the file by its own
/// name, the line when there is one, the detail when the reading gave one.
[[nodiscard]] QString describe(const core::PatternDiagnostic& diagnostic) {
    QString text = QString::fromStdString(diagnostic.file.filename().string());
    if (text.isEmpty())
        text = QString::fromStdString(diagnostic.file.string());
    if (diagnostic.line > 0)
        text += QStringLiteral(", line ") + QString::number(diagnostic.line);
    text += QStringLiteral(" (") + reasonOf(diagnostic.problem);
    if (!diagnostic.detail.empty())
        text += QStringLiteral(": ") + QString::fromStdString(diagnostic.detail);
    return text + QStringLiteral(")");
}

[[nodiscard]] QString abandonedText(const std::vector<core::PatternFailure>& failures,
                                    const QStringList& unreadable) {
    QStringList paragraphs;
    if (!failures.empty()) {
        QStringList lines;
        for (const core::PatternFailure& failure : failures)
            lines << QString::fromStdString(failure.name) + " (" + reasonOf(failure.kind) + ")";
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
        m_unreadable << describe(diagnostic);
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
