#pragma once

// The window of `Tools > Check Spelling…` — issue #509, D6: Gaupol's
// `SpellCheckDialog`, driven by a `core::SpellCheckWalk`.
//
// **It touches no project.** The walk holds the texts and what was corrected;
// the dialog shows the word the walk stopped at and forwards the gestures. A
// controller owns the walk, reveals the subtitle on `stopped`, and applies
// `walk.corrections()` when the dialog closes.

#include <QDialog>

#include <memory>
#include <string>
#include <vector>

namespace subedit::core {
class SpellCheckWalk;
struct SpellStop;
} // namespace subedit::core

class QDialogButtonBox;
class QLineEdit;
class QListWidget;
class QPlainTextEdit;
class QPushButton;
class QWidget;

namespace subedit::gui {

class SpellCheckDialog final : public QDialog {
    Q_OBJECT

public:
    /// `walk` is driven by the dialog and must outlive it. Nothing is shown
    /// until `start`.
    explicit SpellCheckDialog(core::SpellCheckWalk& walk, QWidget* parent = nullptr);
    ~SpellCheckDialog() override;
    SpellCheckDialog(const SpellCheckDialog&) = delete;
    SpellCheckDialog& operator=(const SpellCheckDialog&) = delete;
    SpellCheckDialog(SpellCheckDialog&&) = delete;
    SpellCheckDialog& operator=(SpellCheckDialog&&) = delete;

    /// Moves to the first unknown word, or finishes at once when there is none.
    void start();

    /// Whether the walk has reached its end.
    [[nodiscard]] bool walkFinished() const { return m_done; }

    /// The word the walk stands at; null once done.
    [[nodiscard]] const core::SpellStop* stop() const { return m_stop.get(); }

    [[nodiscard]] QPlainTextEdit* textView() const { return m_text; }

    [[nodiscard]] QLineEdit* replacementField() const { return m_replacement; }

    [[nodiscard]] QListWidget* suggestionList() const { return m_suggestions; }

    [[nodiscard]] QWidget* grid() const { return m_grid; }

    [[nodiscard]] QPushButton* addButton() const { return m_add; }

    [[nodiscard]] QPushButton* ignoreButton() const { return m_ignore; }

    [[nodiscard]] QPushButton* ignoreAllButton() const { return m_ignoreAll; }

    [[nodiscard]] QPushButton* replaceButton() const { return m_replace; }

    [[nodiscard]] QPushButton* replaceAllButton() const { return m_replaceAll; }

    [[nodiscard]] QPushButton* joinWithPreviousButton() const { return m_joinBack; }

    [[nodiscard]] QPushButton* joinWithNextButton() const { return m_joinForward; }

    [[nodiscard]] QPushButton* saveButton() const { return m_save; }

signals:
    /// The walk stands at a new unknown word: reveal its subtitle.
    void stopped(const subedit::core::SpellStop& stop);

    /// The walk has no more words.
    void finishedWalking();

private:
    /// Asks the walk for its next word and shows it, or finishes.
    void proceed();
    void display(const core::SpellStop& stop);
    void finish();
    void populateSuggestions(const std::vector<std::string>& suggestions, bool select);
    void setReplacementText(const QString& text);
    void onTextEdited();
    void onReplacementEdited();
    void updateReplaceButtons();

    core::SpellCheckWalk& m_walk;
    std::unique_ptr<core::SpellStop> m_stop;
    bool m_done = false;
    bool m_loading = false; // the text is being set by the dialog, not typed

    QPlainTextEdit* m_text;
    QLineEdit* m_replacement;
    QListWidget* m_suggestions;
    QWidget* m_grid;
    QPushButton* m_add;
    QPushButton* m_ignore;
    QPushButton* m_ignoreAll;
    QPushButton* m_replace;
    QPushButton* m_replaceAll;
    QPushButton* m_joinBack;
    QPushButton* m_joinForward;
    QPushButton* m_save;
    QDialogButtonBox* m_buttons;
};

} // namespace subedit::gui
