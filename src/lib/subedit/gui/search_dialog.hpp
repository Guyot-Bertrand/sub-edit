#pragma once

#include <QDialog>
#include <QString>

class QCheckBox;
class QLabel;
class QLineEdit;
class QPushButton;

namespace subedit::core {
struct SearchOptions;
} // namespace subedit::core

namespace subedit::gui {

/// `Find and Replace…`: a pattern, a replacement, two options, four gestures.
///
/// **Not modal**, as Gaupol's is not: one finds, looks at the table, edits a
/// cell, finds again — a dialog that held the window still would forbid the
/// middle two.
///
/// **It searches nothing itself.** The window holds the document, the history
/// and the target; the dialog says what was asked, through its four signals,
/// and shows what the window answers in its status line.
class SearchDialog final : public QDialog {
    Q_OBJECT

public:
    explicit SearchDialog(QWidget* parent = nullptr);

    [[nodiscard]] QString pattern() const;

    [[nodiscard]] QString replacement() const;

    [[nodiscard]] core::SearchOptions options() const;

    /// Whether the search goes through every open project — decision D8 of the
    /// phase-11 spec — rather than the one shown.
    [[nodiscard]] bool allProjects() const;

    /// Offers the scope, or takes it away: with one project open there is
    /// nothing to widen to, and a box left ticked would say so falsely.
    ///
    /// **Greyed and unticked, not hidden**: a dialog that grows a box when a
    /// second project opens is a dialog whose layout moves under the pointer.
    void setAllProjectsAvailable(bool available);

    /// Checks the two boxes as `options` says.
    ///
    /// **By value, and it is a correction.** Each box announces its change, the
    /// window answers by reading the dialog's options back into its own member —
    /// and a reference to that member, handed here, was read again for the
    /// second box after the first had already rewritten it.
    void setOptions(core::SearchOptions options);

    /// Says above the fields which text the search looks in, as `field` puts
    /// it; an empty `field` takes the line away.
    ///
    /// **Told and not deduced**: the dialog searches nothing, and the text it
    /// looks in is the window's to know.
    void setField(const QString& field);

    /// Shows `message` under the fields; an empty message clears the line.
    void setStatus(const QString& message);

    /// The fields and buttons, for a test to fill and press them.
    [[nodiscard]] QLineEdit* patternField() const { return m_pattern; }

    [[nodiscard]] QLineEdit* replacementField() const { return m_replacement; }

    [[nodiscard]] QCheckBox* regexCheck() const { return m_regex; }

    [[nodiscard]] QCheckBox* ignoreCaseCheck() const { return m_ignoreCase; }

    [[nodiscard]] QCheckBox* allProjectsCheck() const { return m_allProjects; }

    [[nodiscard]] QLabel* fieldLabel() const { return m_field; }

    [[nodiscard]] QLabel* statusLabel() const { return m_status; }

    [[nodiscard]] QPushButton* previousButton() const { return m_previous; }

    [[nodiscard]] QPushButton* nextButton() const { return m_next; }

    [[nodiscard]] QPushButton* replaceButton() const { return m_replace; }

    [[nodiscard]] QPushButton* replaceAllButton() const { return m_replaceAll; }

signals:
    void findPreviousRequested();
    void findNextRequested();
    void replaceRequested();
    void replaceAllRequested();

    /// The pattern or an option changed: a match found before means nothing
    /// any more.
    void searchChanged();

private:
    /// The four gestures need something to look for.
    void refreshButtons();

    QLineEdit* m_pattern;
    QLineEdit* m_replacement;
    QCheckBox* m_regex;
    QCheckBox* m_ignoreCase;
    QCheckBox* m_allProjects;
    QLabel* m_field;
    QLabel* m_status;
    QPushButton* m_previous;
    QPushButton* m_next;
    QPushButton* m_replace;
    QPushButton* m_replaceAll;
};

} // namespace subedit::gui
