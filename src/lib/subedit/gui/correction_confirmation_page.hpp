#pragma once

#include <QWizardPage>

class QCheckBox;
class QLabel;
class QPushButton;
class QTableView;

namespace subedit::gui {

class CorrectionProgressPage;
class CorrectionResultModel;

/// The last page — the table of proposed changes, `Mark All`/`Unmark All`,
/// `Preview`, the remove-blank-subtitles case, and the patterns the run could
/// not apply, named (D8, GUI-CORRECT-02/03/06).
class CorrectionConfirmationPage final : public QWizardPage {
    Q_OBJECT

public:
    explicit CorrectionConfirmationPage(QWidget* parent = nullptr);

    /// `progress` must outlive this page — the wizard owns both.
    void setProgressPage(const CorrectionProgressPage* progress) { m_progress = progress; }

    void setRemoveBlankSubtitlesDefault(bool removeBlank);

    void initializePage() override;

    [[nodiscard]] CorrectionResultModel* resultModel() const { return m_model; }

    [[nodiscard]] bool removeBlankSubtitles() const;

signals:
    /// The row at `row` (into `resultModel()`) was asked to be previewed.
    void previewRequested(int row);

private:
    const CorrectionProgressPage* m_progress = nullptr;
    QTableView* m_table;
    CorrectionResultModel* m_model = nullptr;
    QPushButton* m_markAll;
    QPushButton* m_unmarkAll;
    QPushButton* m_preview;
    QCheckBox* m_removeBlank;
    QLabel* m_abandoned;
};

} // namespace subedit::gui
