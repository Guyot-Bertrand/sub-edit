#pragma once

#include <subedit/core/model/document.hpp>

#include <QWizardPage>

class QCheckBox;
class QRadioButton;

/// **Declared rather than included** — the shape `subtitle_table_model.hpp`
/// adopted after `moc` once failed on `<concepts>` reached through
/// `Selection`. That failure is **not a standing rule**: `main_window.hpp`,
/// itself `Q_OBJECT`, includes `selection.hpp`, and today's `moc` parses
/// `correction_run.hpp` cleanly with this project's include paths (checked
/// for #505's final review). A declaration is kept because it is all this
/// header needs, and it keeps `moc` and every includer out of the core headers.
namespace subedit::core {
enum class CorrectionTask;
} // namespace subedit::core

namespace subedit::gui {

enum class CorrectionScope;

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
    QCheckBox* m_joinSplit;
    QCheckBox* m_commonErrors;
    QCheckBox* m_capitalization;
    QCheckBox* m_lineBreak;
};

} // namespace subedit::gui
