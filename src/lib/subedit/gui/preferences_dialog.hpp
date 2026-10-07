#pragma once

#include <subedit/core/config/editor_settings.hpp>
#include <subedit/core/config/theme.hpp>
#include <subedit/core/config/video_settings.hpp>

#include <QDialog>

class QCheckBox;
class QComboBox;
class QDoubleSpinBox;
class QSpinBox;
class QWidget;

namespace subedit::gui {

/// What is set by no gesture other than setting it.
///
/// **Few preferences, and that is a criterion and not a shortfall**: the
/// geometry, the maximised state, the column widths and the position of the
/// handle are all set by moving the window or dragging an edge, and a
/// preference that already has a gesture does not need a field. The theme has
/// none; nor do the three settings of the editor — issue #526, Gaupol's
/// Preferences ▸ Editor: the unit a line's length is shown in, and whether the
/// cells and the cell editor show it.
///
/// The default frame rate the scoping announced **will not come**: #267 looked
/// into it and set it aside. None of the three possible readers needed it — the
/// conversion prefills its top field with the deduced grid, the alignment opens
/// on what the video declares, and a project with neither is a project where
/// the user chooses. A preference nobody uses is a box that lies.
class PreferencesDialog final : public QDialog {
    Q_OBJECT

public:
    explicit PreferencesDialog(core::Theme theme,
                               const core::EditorSettings& editor = {},
                               const core::VideoSettings& video = {},
                               QWidget* parent = nullptr);

    /// The theme chosen, whether or not it was accepted — the caller looks at
    /// the return code to know whether to take it into account.
    [[nodiscard]] core::Theme theme() const;

    /// The list of themes, so that a test picks without clicking.
    [[nodiscard]] QComboBox* themeBox() const { return m_theme; }

    /// The three settings of the editor as they stand in the dialog, whether or
    /// not it was accepted.
    [[nodiscard]] core::EditorSettings editor() const;

    /// The two settings of the video player the dialog holds, as they stand in it: the jump and the
    /// lead-in. **The volume is not here** — the bar and the gestures set it — and comes back as
    /// the default, for the caller to put its own.
    [[nodiscard]] core::VideoSettings video() const;

    /// The controls, so that a test sets them without clicking.
    [[nodiscard]] QComboBox* lengthUnitBox() const { return m_lengthUnit; }

    [[nodiscard]] QSpinBox* seekLengthBox() const { return m_seekLength; }

    [[nodiscard]] QDoubleSpinBox* contextLengthBox() const { return m_contextLength; }

    [[nodiscard]] QSpinBox* stepFramesBox() const { return m_stepFrames; }

    [[nodiscard]] QCheckBox* showLengthsInCellsBox() const { return m_showInCells; }

    [[nodiscard]] QCheckBox* showLengthsInEditorBox() const { return m_showInEditor; }

private:
    /// The unit means nothing when no length is shown, so it is greyed rather
    /// than left to be set to no effect.
    void refreshLengthUnitState();

    QComboBox* m_theme;
    QComboBox* m_lengthUnit;
    QCheckBox* m_showInCells;
    QCheckBox* m_showInEditor;
    QSpinBox* m_seekLength;
    QDoubleSpinBox* m_contextLength;
    QSpinBox* m_stepFrames;
};

} // namespace subedit::gui
