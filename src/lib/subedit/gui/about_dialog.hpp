#pragma once

#include <QDialog>
#include <QString>

class QLabel;

namespace subedit::gui {

/// Who this is and which version of it is running.
///
/// **A dialog of the project and not `QMessageBox::about`.** The latter opens
/// its own modal loop, which is the one thing `Prompts` exists to keep out of a
/// test — `qt_prompts.cpp` carries thirty-eight lines no test can reach for
/// exactly that reason, and there is no need to add more for three lines of
/// text. This one goes through the same seam as every other dialog.
class AboutDialog final : public QDialog {
    Q_OBJECT

public:
    explicit AboutDialog(QWidget* parent = nullptr);

    /// What the dialog says, for a test to read what a user would.
    [[nodiscard]] QString text() const;

private:
    QLabel* m_text = nullptr;
};

} // namespace subedit::gui
