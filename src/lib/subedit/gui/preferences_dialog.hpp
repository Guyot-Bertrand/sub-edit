#pragma once

#include <subedit/core/config/theme.hpp>

#include <QDialog>

class QComboBox;
class QWidget;

namespace subedit::gui {

/// What is set by no gesture other than setting it.
///
/// **One preference for now, and that is a criterion and not a shortfall**: the
/// geometry, the maximised state, the column widths and the position of the
/// handle are all set by moving the window or dragging an edge, and a
/// preference that already has a gesture does not need a field. The theme has
/// none.
///
/// The default frame rate the scoping announced **will not come**: #267 looked
/// into it and set it aside. None of the three possible readers needed it — the
/// conversion prefills its top field with the deduced grid, the alignment opens
/// on what the video declares, and a project with neither is a project where
/// the user chooses. A preference nobody uses is a box that lies.
class PreferencesDialog final : public QDialog {
    Q_OBJECT

public:
    explicit PreferencesDialog(core::Theme theme, QWidget* parent = nullptr);

    /// The theme chosen, whether or not it was accepted — the caller looks at
    /// the return code to know whether to take it into account.
    [[nodiscard]] core::Theme theme() const;

    /// The list of themes, so that a test picks without clicking.
    [[nodiscard]] QComboBox* themeBox() const { return m_theme; }

private:
    QComboBox* m_theme;
};

} // namespace subedit::gui
