#pragma once

// moc (Qt 6.4.2) fails to parse libstdc++ 13's <concepts>, reached
// transitively through <compare>, itself pulled in by subtitle_index.hpp —
// which every one of the three headers below drags in. None of them are
// needed for moc's own pass: every member below is a plain method, never a
// signal or slot, so moc never has to resolve `CorrectionScope`,
// `core::Document` or `core::CorrectionTask` — it only has to skip past the
// declarations syntactically, which it already does without seeing them
// defined (confirmed directly against moc's own output). `Q_MOC_RUN` is the
// macro moc predefines for exactly this: real compilation never defines it,
// so nothing here is hidden from the actual build.
#ifndef Q_MOC_RUN
#    include <subedit/core/model/document.hpp>
#    include <subedit/core/text/correction_run.hpp>
#    include <subedit/gui/correction_target.hpp>
#endif

#include <QWizardPage>

class QCheckBox;
class QRadioButton;

namespace subedit::gui {

/// The first page of the assistant — which tasks, on which target, on which
/// document (D8).
class CorrectionTargetPage final : public QWizardPage {
    Q_OBJECT

public:
    /// `selectionAvailable`: the page on screen has rows selected. `translationAvailable`:
    /// at least one candidate project carries a translation.
    CorrectionTargetPage(bool selectionAvailable,
                         bool translationAvailable,
                         QWidget* parent = nullptr);

    [[nodiscard]] CorrectionScope scope() const;
    [[nodiscard]] core::Document document() const;
    [[nodiscard]] bool taskChecked(core::CorrectionTask task) const;
    void setTaskChecked(core::CorrectionTask task, bool checked);

private:
    QRadioButton* m_selection;
    QRadioButton* m_currentProject;
    QRadioButton* m_allProjects;
    QRadioButton* m_text;
    QRadioButton* m_translation;
    QCheckBox* m_mentions;
    QCheckBox* m_commonErrors;
    QCheckBox* m_capitalization;
    QCheckBox* m_lineBreak;
};

} // namespace subedit::gui
