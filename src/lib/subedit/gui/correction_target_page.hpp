#pragma once

#include <subedit/core/model/document.hpp>

#include <QWizardPage>

class QCheckBox;
class QRadioButton;

/// **Declared rather than included, deliberately** — the same reason and the
/// same shape as `subtitle_table_model.hpp`: `moc` parses this header, and it
/// chokes on the C++20 library headers `correction_run.hpp` drags in through
/// `Selection` (its iterator pulls `<iterator>`, and `<concepts>` follows).
/// Declaring what the signatures need keeps `moc` out of all that; the
/// definitions come in the implementation file.
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
    QCheckBox* m_commonErrors;
    QCheckBox* m_capitalization;
    QCheckBox* m_lineBreak;
};

} // namespace subedit::gui
