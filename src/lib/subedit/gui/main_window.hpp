#pragma once

#include <subedit/gui/opening.hpp>
#include <subedit/gui/player_factory.hpp>
#include <subedit/gui/subtitle_table.hpp>

#include <QMainWindow>

#include <cstdint>
#include <filesystem>
#include <functional>
#include <memory>
#include <span>
#include <string>

namespace subedit::core {
class Command;
struct Diagnostic;
class FileSystem;
class Project;
class Session;
class VideoPlayer;
} // namespace subedit::core

class QAction;
class QCloseEvent;
class QLabel;
class QTimer;

namespace subedit::gui {

class DiagnosticsPanel;
class Prompts;
class SubtitleTableModel;

/// The window, and everything a project needs to be looked at.
///
/// It owns the session — the project, its history and its order policy — and
/// the table model that reads and writes through it. **The three editable
/// cells are wired**: a start, an end and a text each open the editor their
/// nature calls for. So are the actions the phase asked for — undo and redo,
/// open, save and save as, and the four operations of the `Tools` menu.
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
    ///
    /// `buildPlayer` is the other seam, and it is optional: without one, the
    /// window associates films and names them and never plays anything.
    MainWindow(core::FileSystem& files,
               OpenedFile opened,
               Prompts& prompts,
               PlayerFactory buildPlayer = {},
               QWidget* parent = nullptr);

    ~MainWindow() override;

    MainWindow(const MainWindow&) = delete;
    MainWindow& operator=(const MainWindow&) = delete;
    MainWindow(MainWindow&&) = delete;
    MainWindow& operator=(MainWindow&&) = delete;

    /// Returns the table, for a test to look at what the window shows.
    [[nodiscard]] SubtitleTable* table() const { return m_table; }

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

    [[nodiscard]] QAction* selectVideoAction() const { return m_selectVideo; }

    [[nodiscard]] QAction* playPauseAction() const { return m_playPause; }

    /// The surface the film is drawn on, for a test to read whether it is
    /// there at all. Hidden while no film is open, which is what « the table
    /// takes the whole window » means.
    [[nodiscard]] QWidget* videoView() const { return m_videoView; }

    /// What the status bar says of the associated film — its name, or that
    /// there is none. This is what `GUI-VIDEO-01` promises the user sees.
    [[nodiscard]] QLabel* videoStatus() const { return m_videoStatus; }

    /// Reads where playback stands and puts the window in step with it.
    ///
    /// Two things, and they are the same thing seen twice: the replica drawn
    /// over the picture is the subtitle showing now, and so is the row the
    /// table points at.
    ///
    /// **It gives way to whoever is typing.** Moving the current row closes an
    /// open editor, which is how a film playing in the corner of the screen
    /// would eat a correction halfway through being made. While a cell is
    /// being edited the row stays where it is; the replica still follows,
    /// since drawing on the picture disturbs nobody.
    ///
    /// **Public because the ticker is not the only thing that must run it.**
    /// A test drives it directly rather than waiting on a clock, and a seek
    /// runs it at once so that the picture and the table agree before the next
    /// tick rather than a tenth of a second later.
    void followPlayback();

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

    /// Asks which film to watch the document against, and associates it.
    void selectVideo();

    /// Offers the film the naming convention finds beside the subtitle file.
    ///
    /// Called wherever the file's name becomes known or changes — an opening,
    /// a « save as » — because that name is all the convention reads. A choice
    /// already made is never replaced: D5 lives in `Project`, so calling this
    /// too often costs nothing but a look at a directory.
    void proposeVideoBeside();

    /// Puts what the document is watched against into the status bar.
    void refreshVideoStatus();

    /// Puts the window in step with the film the document is now associated
    /// with — the status bar, the picture, and whether there is one at all.
    ///
    /// Called wherever the association can have changed, and it is cheap to
    /// call when it has not: a film already open is not opened again.
    void refreshVideo();

    /// Opens the associated film, or takes the view away.
    ///
    /// **A film that will not open is said, named, and then let go.** Nothing
    /// else about the window changes: the document is still there, the
    /// operations still work, and the association still stands — the user may
    /// well want to see which file it is that the player refused.
    void watchAssociatedVideo();

    /// Returns the player, building it the first time one is needed.
    ///
    /// Nothing, when no factory was given or when the factory declined. Asked
    /// **once**: a libmpv that would not give a player will not give one on
    /// the second film either, and asking again would report the same failure
    /// at every attempt.
    [[nodiscard]] core::VideoPlayer* player();

    /// Plays, or holds where it is — the player is the one that knows which.
    void togglePlayback();

    /// Places playback at the start of the first selected subtitle.
    ///
    /// **Only when that first row changes**, and that is not a refinement:
    /// extending a selection downwards over four thousand rows fires this at
    /// every step, and a seek waits for the player to arrive.
    void placePlaybackAtSelection();

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

    SubtitleTable* m_table = nullptr;
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
    QAction* m_selectVideo = nullptr;
    QAction* m_playPause = nullptr;
    QLabel* m_videoStatus = nullptr;
    QWidget* m_videoView = nullptr;
    QTimer* m_ticker = nullptr;

    PlayerFactory m_buildPlayer{};
    std::unique_ptr<core::VideoPlayer> m_player;
    bool m_playerAsked = false;

    /// The film the window last acted on, whether or not it opened.
    ///
    /// Distinct from `m_watching` on purpose: a film that was refused must not
    /// be offered to the player again — and refused again, and reported again
    /// — every time the naming convention speaks.
    std::filesystem::path m_associated;

    /// Whether a film is open and being drawn.
    bool m_watching = false;

    /// The line the overlay currently carries.
    ///
    /// Held so that a tick that changes nothing costs nothing: the replica is
    /// recomputed from the project ten times a second, and it is only handed
    /// over when it differs — which is also what makes a keystroke show up on
    /// the picture within a tick.
    std::string m_shown;

    /// The row playback was last placed at, or -1.
    int m_placedAt = -1;

    /// Held by pointer so that this header stays parsable by `moc`, which
    /// chokes on the C++20 library headers the core drags in.
    std::unique_ptr<core::Session> m_session;
    std::unique_ptr<SubtitleTableModel> m_model;
};

} // namespace subedit::gui
