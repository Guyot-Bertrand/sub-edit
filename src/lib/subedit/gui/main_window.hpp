#pragma once

#include <subedit/gui/opening.hpp>

#include <QMainWindow>

#include <memory>
#include <span>

namespace subedit::core {
class Command;
struct Diagnostic;
class FileSystem;
class Project;
class Session;
} // namespace subedit::core

class QAction;
class QCloseEvent;
class QTableView;

namespace subedit::gui {

class DiagnosticsPanel;
class Prompts;
class SubtitleTableModel;

/// The window, and everything a project needs to be looked at.
///
/// It owns the session — the project, its history and its order policy — and
/// the table model that reads and writes through it. **The three editable
/// cells are wired**: a start, an end and a text each open the editor their
/// nature calls for. The actions — undo, redo, open, save — arrive next.
///
/// Kept in `subedit::gui` and not in `main.cpp` because that is what makes it
/// testable: `check-architecture.sh` refuses a `main` that defines a class, and
/// the reason is exactly this one.
class MainWindow final : public QMainWindow {
    Q_OBJECT

public:
    /// Opens a window on `opened`, which it takes over.
    ///
    /// `files` and `prompts` must outlive it. The second is the seam that
    /// makes this class testable at all: every question a human answers goes
    /// through it, so a test answers them instead.
    MainWindow(core::FileSystem& files,
               OpenedFile opened,
               Prompts& prompts,
               QWidget* parent = nullptr);

    ~MainWindow() override;

    MainWindow(const MainWindow&) = delete;
    MainWindow& operator=(const MainWindow&) = delete;
    MainWindow(MainWindow&&) = delete;
    MainWindow& operator=(MainWindow&&) = delete;

    /// Returns the table, for a test to look at what the window shows.
    [[nodiscard]] QTableView* table() const { return m_table; }

    /// The two actions, for a test to read their state and to fire them.
    ///
    /// **There is no `QUndoStack` behind them.** The history of the core is
    /// the authority — the command line of phase 3 depends on it too — and two
    /// sources of truth for one question would be one too many. These two only
    /// read it.
    [[nodiscard]] QAction* undoAction() const { return m_undo; }

    [[nodiscard]] QAction* redoAction() const { return m_redo; }

    [[nodiscard]] QAction* openAction() const { return m_open; }

    [[nodiscard]] QAction* saveAction() const { return m_save; }

    [[nodiscard]] QAction* saveAsAction() const { return m_saveAs; }

    /// The panel of what the last reading ran into.
    [[nodiscard]] DiagnosticsPanel* diagnostics() const { return m_diagnostics; }

    [[nodiscard]] QAction* shiftAction() const { return m_shift; }

    [[nodiscard]] QAction* transformAction() const { return m_transform; }

    [[nodiscard]] QAction* frameRateAction() const { return m_frameRate; }

    [[nodiscard]] QAction* hearingImpairedAction() const { return m_hearingImpaired; }

protected:
    /// Refuses to close while there are changes nobody chose to lose.
    void closeEvent(QCloseEvent* event) override;

private:
    /// Recomputes what the two actions may do and what they read.
    ///
    /// Called after **every** operation, the one that changed nothing
    /// included: an action left enabled over an empty history would swallow
    /// its own shortcut, and one left naming an operation that has been undone
    /// would lie.
    void refreshActions();

    /// Puts the window on `project`, dropping whatever it held.
    void openOn(core::Project project, std::span<const core::Diagnostic> diagnostics);

    /// Writes the document, asking where if it has never been anywhere.
    ///
    /// Returns whether it was written — « the user gave up » and « the disk
    /// refused » are both `false`, and both must stop whatever asked.
    [[nodiscard]] bool save();

    [[nodiscard]] bool saveAs();

    /// Returns whether whatever is about to lose the changes may go on.
    [[nodiscard]] bool mayDiscardChanges();

    void openFromPrompt();

    /// Applies `command` and refreshes what the window shows of it.
    ///
    /// The one road from a dialog to the history: every operation of this
    /// phase ends here, so the refresh cannot be forgotten in one of them.
    void applyOperation(std::unique_ptr<core::Command> command);

    void shiftTarget();

    void transformTarget();

    void convertFrameRateOfTarget();

    void removeHearingImpairedFromTarget();

    /// **Initialised here, and not only in the constructor's list.**
    ///
    /// Three actions added together at issue #132 were left out of that list,
    /// and the first symptom was a segfault inside `QObject::connect`.
    /// `-Wuninitialized` says nothing of that case — for a member, it models
    /// only the one that initialises another before its turn — and that does
    /// not depend on the optimisation level.
    ///
    /// The gate would have said it: `cppcoreguidelines-pro-type-member-init`
    /// names the fields a constructor leaves out. These `nullptr` are therefore
    /// a belt over braces — they make the omission harmless where the check
    /// makes it visible.
    core::FileSystem* m_files = nullptr;
    Prompts* m_prompts = nullptr;

    QTableView* m_table = nullptr;
    DiagnosticsPanel* m_diagnostics = nullptr;
    QAction* m_undo = nullptr;
    QAction* m_redo = nullptr;
    QAction* m_open = nullptr;
    QAction* m_save = nullptr;
    QAction* m_saveAs = nullptr;
    QAction* m_shift = nullptr;
    QAction* m_transform = nullptr;
    QAction* m_frameRate = nullptr;
    QAction* m_hearingImpaired = nullptr;

    /// Held by pointer so that this header stays parsable by `moc`, which
    /// chokes on the C++20 library headers the core drags in.
    std::unique_ptr<core::Session> m_session;
    std::unique_ptr<SubtitleTableModel> m_model;
};

} // namespace subedit::gui
