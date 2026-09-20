#pragma once

#include <QDialog>
#include <QString>

class QLabel;
class QRadioButton;

namespace subedit::core {
enum class TranslationMethod;
} // namespace subedit::core

namespace subedit::gui {

/// How the lines of a translation file are matched to the subtitles — the one
/// question `File ▸ Open Translation…` asks once the file is chosen.
///
/// **A dialog of the project and not a widget slipped into the file chooser**,
/// for the reason every other dialog here is: it goes through `Prompts::run`,
/// so a test presses its buttons without entering a modal loop.
///
/// **By position is the default**, and the eight cases of `paires/` say why: one
/// line missing in the middle makes every later line slip by number, and by
/// position a single subtitle is left without a translation.
class OpenTranslationDialog final : public QDialog {
    Q_OBJECT

public:
    /// `fileName` is the file chosen, said in the question so that it reads on
    /// its own.
    explicit OpenTranslationDialog(const QString& fileName, QWidget* parent = nullptr);

    /// The method chosen.
    [[nodiscard]] core::TranslationMethod method() const;

    /// What the dialog says, for a test to read what a user would.
    [[nodiscard]] QString text() const;

    /// The two choices, so that a test picks one without clicking.
    [[nodiscard]] QRadioButton* positionButton() const { return m_position; }

    [[nodiscard]] QRadioButton* numberButton() const { return m_number; }

private:
    QLabel* m_text = nullptr;
    QRadioButton* m_position = nullptr;
    QRadioButton* m_number = nullptr;
};

} // namespace subedit::gui
