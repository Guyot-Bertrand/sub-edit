#pragma once

#include <QDialog>
#include <QString>

#include <string>
#include <vector>

class QLabel;
class QTableWidget;

namespace subedit::gui {

/// Who this is, which version of it is running, and against which libraries.
///
/// **A dialog of the project and not `QMessageBox::about`.** The latter opens
/// its own modal loop, which is the one thing `Prompts` exists to keep out of a
/// test — `qt_prompts.cpp` carries thirty-eight lines no test can reach for
/// exactly that reason, and there is no need to add more for three lines of
/// text. This one goes through the same seam as every other dialog.
///
/// **The libraries are `windowLibraries()`**, the list `--version` writes (#716):
/// one row each, its name, the version loaded and the version built against —
/// then the interface languages installed, `languages`, by code.
class AboutDialog final : public QDialog {
    Q_OBJECT

public:
    explicit AboutDialog(const std::vector<std::string>& languages, QWidget* parent = nullptr);

    /// What the dialog says above the table, for a test to read what a user would.
    [[nodiscard]] QString text() const;

    /// The table of the libraries: name, loaded, built — a row each.
    [[nodiscard]] const QTableWidget& libraries() const { return *m_libraries; }

    /// The line of the installed languages, under the table.
    [[nodiscard]] QString languages() const;

private:
    QLabel* m_text = nullptr;
    QTableWidget* m_libraries = nullptr;
    QLabel* m_languages = nullptr;
};

} // namespace subedit::gui
