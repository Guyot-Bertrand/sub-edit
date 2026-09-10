#pragma once

#include <subedit/core/config/insert_placement.hpp>
#include <subedit/core/config/settings.hpp>
#include <subedit/core/format/project_file.hpp>
#include <subedit/gui/player_factory.hpp>
#include <subedit/gui/subtitle_table.hpp>

#include <QMainWindow>
#include <QStringList>

#include <cstdint>
#include <filesystem>
#include <functional>
#include <memory>
#include <span>
#include <string>

namespace subedit::core {
class Command;
enum class CommandKind;
class Duration;
struct Diagnostic;
class FileSystem;
class Project;
class Selection;
class Session;
class VideoPlayer;
} // namespace subedit::core

class QAction;
class QCloseEvent;
class QLabel;
class QShowEvent;
class QSplitter;
class QTimer;

namespace subedit::gui {

class DiagnosticsPanel;
class ManualWindow;
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
    ///
    /// `readDeclaredRate` is the third, optional too: without it nothing is
    /// proposed in the frame rate dialog, which is what a machine with no
    /// `ffmpeg` gets — and no operation behaves differently for it.
    MainWindow(core::FileSystem& files,
               core::OpenedFile opened,
               Prompts& prompts,
               PlayerFactory buildPlayer = {},
               FrameRateReader readDeclaredRate = {},
               QWidget* parent = nullptr);

    ~MainWindow() override;

    MainWindow(const MainWindow&) = delete;
    MainWindow& operator=(const MainWindow&) = delete;
    MainWindow(MainWindow&&) = delete;
    MainWindow& operator=(MainWindow&&) = delete;

    /// Puts the window back where the previous session left it.
    ///
    /// **Called before `show()`**, without which the window appears at its
    /// default size and then jumps to its own, which shows.
    ///
    /// What the settings do not carry is not applied: a missing geometry leaves
    /// the window to size itself, wide enough to read the table — that is the
    /// default, and it is better than zero.
    void applySettings(const core::Settings& settings);

    /// What this session leaves behind it.
    ///
    /// Read once the window is closed, by the wiring that will write it. The
    /// window persists nothing itself: it says its state, and that is all — the
    /// same separation as between `Session` and `saveProject`.
    ///
    /// **The geometry answered is the one from before the enlargement** when
    /// the window is maximised: Qt keeps both, and keeping the size of the
    /// screen as the normal geometry would mean that unmaximising at the next
    /// session showed nothing at all.
    [[nodiscard]] core::Settings settings() const;

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

    /// The two edits of structure, for a test to read their state and trigger
    /// them.
    [[nodiscard]] QAction* insertAction() const { return m_insert; }

    [[nodiscard]] QAction* removeAction() const { return m_remove; }

    /// The panel of what the last reading ran into.
    [[nodiscard]] DiagnosticsPanel* diagnostics() const { return m_diagnostics; }

    [[nodiscard]] QAction* shiftAction() const { return m_shift; }

    [[nodiscard]] QAction* transformAction() const { return m_transform; }

    [[nodiscard]] QAction* frameRateAction() const { return m_frameRate; }

    [[nodiscard]] QAction* hearingImpairedAction() const { return m_hearingImpaired; }

    /// The entry that opens the preferences, for a test to trigger it.
    [[nodiscard]] QAction* preferencesAction() const { return m_preferences; }

    [[nodiscard]] QAction* selectVideoAction() const { return m_selectVideo; }

    [[nodiscard]] QAction* playPauseAction() const { return m_playPause; }

    /// The surface the film is drawn on, for a test to read whether it is
    /// there at all. Hidden while no film is open, which is what « the table
    /// takes the whole window » means.
    [[nodiscard]] QWidget* videoView() const { return m_videoView; }

    /// What stands where the picture would be while there is no film: a way in,
    /// rather than an absence a user has to guess is one.
    [[nodiscard]] QWidget* noVideoBanner() const { return m_noVideo; }

    /// What the status bar says of the associated film — its name, or that
    /// there is none. This is what `GUI-VIDEO-01` promises the user sees.
    [[nodiscard]] QLabel* videoStatus() const { return m_videoStatus; }

    /// What the status bar says of the grid the positions were written on.
    /// This is what `GUI-GRID-01` promises the user sees.
    [[nodiscard]] QLabel* gridStatus() const { return m_gridStatus; }

    /// What the status bar says of the encoding the document was read in.
    /// This is what `GUI-ENC-01` promises the user sees.
    [[nodiscard]] QLabel* encodingStatus() const { return m_encodingStatus; }

    [[nodiscard]] QAction* analyseGridAction() const { return m_analyseGrid; }

    [[nodiscard]] QAction* snapAction() const { return m_snap; }

    [[nodiscard]] QAction* aboutAction() const { return m_about; }

    /// Opens the installed manual. Out for as long as there is none.
    [[nodiscard]] QAction* manualAction() const { return m_manual; }

    /// Says where the installed manual is, and lights the entry if it is there.
    ///
    /// **Received rather than resolved**, as the settings are and for the same
    /// reason — ADR 0022: `gui::installedManualPath()` is the only code that
    /// knows where to look, `main` calls it and passes the answer here. A test
    /// gives whatever path it likes, and so never reaches the real manual.
    ///
    /// **The entry stays out when the manual is not there**, which is the case
    /// of a binary run from the build tree and that of a partial installation.
    /// It is what holds the promise of the scoping: a missing manual crashes
    /// nothing, it puts an entry out.
    void setManualPath(std::filesystem::path directory);

    /// The manual window, if it is open. For a test to read it.
    [[nodiscard]] ManualWindow* manualWindow() const { return m_manualWindow; }

    /// The names of the menus, in the order the bar shows them.
    [[nodiscard]] QStringList menuTitles() const;

    /// Bringing the file back onto its own grid. Its text carries the measured
    /// amount, which is how `GUI-GRID-03` shows it before it is applied.
    [[nodiscard]] QAction* shiftOntoGridAction() const { return m_shiftOntoGrid; }

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

    /// Opens the associated film, the first time the window is on screen.
    ///
    /// **The film waits for this, and it is not a refinement.** libmpv adopts
    /// the window it is handed at the moment it loads a file; handed one that
    /// is not on screen yet, it adopts it and never maps its own — measured,
    /// mpv's window stays `IsUnMapped` for the life of the process and the
    /// panel stays empty for ever. A window built and never shown is not a
    /// window a user has, and this is where that stops being a distinction
    /// without a difference.
    void showEvent(QShowEvent* event) override;

private:
    /// Works out afresh what the two edits of structure are allowed to do.
    ///
    /// **Apart from `refreshActions`, and wired to the selection**: they are
    /// the only two actions whose state depends on what is selected, and
    /// `refreshActions` deduces the grid of the whole file. Wiring that one to
    /// the selection would pay for the deduction at every row of a drag over
    /// four thousand of them.
    void refreshStructureActions();

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

    /// Applies `command` over `target` and refreshes what the window shows.
    ///
    /// The one road from a dialog to the history: every operation of this
    /// phase ends here, so neither the refresh nor the notice below can be
    /// forgotten in one of them.
    ///
    /// `target` is what the operation was applied to, and it is carried here
    /// for one reason: what reaches past the end of the film is read over it,
    /// after the fact, on the state the operation produced.
    void applyOperation(std::unique_ptr<core::Command> command, const core::Selection& target);

    /// Says what an operation left past the end of the film, if anything.
    ///
    /// **A notice, never a refusal** — decision D4. A subtitle landing after
    /// the closing credits may be exactly what was meant; refusing wrongly
    /// costs more than a warning that is ignored.
    ///
    /// Silent without a film open: the length is what the player knows, and
    /// there is nothing to be past the end of.
    void reportWhatPassesTheEnd(core::CommandKind kind, const core::Selection& target);

    /// How long the open film lasts, or nothing.
    [[nodiscard]] std::optional<core::Duration> videoLength() const;

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

    /// Recomputes the deduction and puts the status bar in step with it.
    ///
    /// **Recomputed rather than kept**, which is ADR 0021's choice: a stored
    /// derived value is an invalidation to hold, and every edit of a position
    /// would stale it. A pure function called again has no such problem, and it
    /// costs a fraction of a millisecond on a full-length file.
    void refreshGridStatus();

    /// The rate a document counted in frames was read at, or nothing for the
    /// eight formats of nine that count in time.
    [[nodiscard]] std::optional<core::FrameRate> rateReadInFrames() const;

    /// Puts the status bar in step with the encoding the document carries.
    ///
    /// Called wherever that encoding can have changed — an opening, and a
    /// « save as » that moved the document onto another one.
    void refreshEncodingStatus();

    /// Opens the analysis, which reports and changes nothing.
    void analyseGrid();

    /// Says who this is and which version is running.
    void about();

    /// Opens the manual, or brings the one already open back to the front.
    void openManual();

    /// Opens the preferences, and lays down what comes out of them.
    void openPreferences();

    /// Keeps the directory of `file` as the one the next "open" box will open
    /// in.
    void rememberDirectoryOf(const std::filesystem::path& file);

    /// Asks which grid to lay the positions on, and lays them on it.
    void snapToFrameRate();

    /// Moves the whole file back onto the grid it was written on.
    ///
    /// No dialog: the operation takes no option, and the amount it will use is
    /// already in the menu entry that opened it.
    void shiftOntoGrid();

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

    /// Asks how many blank rows, and where, then lays them down.
    ///
    /// **The index is that of the last selected**, plus one if the side chosen
    /// is "below". It is what Gaupol has done for twenty years, and it is the
    /// point one invents badly without reading it: the first selected looks
    /// more natural and is not what the hand expects after sweeping from top to
    /// bottom.
    ///
    /// In an empty document the index is zero and no selection is required —
    /// it is the only way to start a new file.
    void insertSubtitles();

    /// Removes the selection, asking nothing.
    ///
    /// **Without confirmation, and it is no oversight**: the operation enters
    /// the history like the others, so `Ctrl+Z` undoes it. A dialog in front of
    /// an undoable gesture costs a click every time to spare a `Ctrl+Z` now and
    /// then.
    ///
    /// **The selection, and never the whole file.** `targetOf` reads "nothing
    /// selected" as "everything", which is right for a shift and would be a
    /// disaster here; the action is out when nothing is selected, and this
    /// function does not catch up for it — it has no business knowing two
    /// rules.
    void removeSubtitles();

    /// Selects the range of rows given, and brings it into view.
    ///
    /// What Gaupol does after an insertion and after a removal: the table has
    /// been reset, so the selection is gone, and without this a second `Ins` or
    /// a second `Del` would find nothing left to work from.
    void selectRows(int first, int last);

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
    QAction* m_insert = nullptr;
    QAction* m_remove = nullptr;
    QAction* m_shift = nullptr;
    QAction* m_transform = nullptr;
    QAction* m_frameRate = nullptr;
    QAction* m_hearingImpaired = nullptr;
    QAction* m_analyseGrid = nullptr;
    QAction* m_snap = nullptr;
    QAction* m_shiftOntoGrid = nullptr;
    QAction* m_preferences = nullptr;
    QAction* m_about = nullptr;
    QAction* m_manual = nullptr;
    ManualWindow* m_manualWindow = nullptr;
    QAction* m_selectVideo = nullptr;
    QAction* m_playPause = nullptr;
    QLabel* m_videoStatus = nullptr;
    QLabel* m_gridStatus = nullptr;
    QLabel* m_encodingStatus = nullptr;
    QWidget* m_videoView = nullptr;
    QWidget* m_noVideo = nullptr;
    QSplitter* m_split = nullptr;
    QTimer* m_ticker = nullptr;

    PlayerFactory m_buildPlayer{};
    FrameRateReader m_readDeclaredRate{};
    std::unique_ptr<core::VideoPlayer> m_player;
    bool m_playerAsked = false;

    /// The film the window last acted on, whether or not it opened.
    ///
    /// Distinct from `m_watching` on purpose: a film that was refused must not
    /// be offered to the player again — and refused again, and reported again
    /// — every time the naming convention speaks.
    std::filesystem::path m_associated;

    /// The theme asked for, to be handed back to the settings. Laid down, not
    /// deduced: the current palette does not say which of the three made it.
    core::Theme m_theme = core::Theme::System;

    /// Which side of the selection the next insertion will lay its rows on.
    ///
    /// Kept from one call to the next, and handed back to the settings: one
    /// does not insert once but ten times in a row, always on the same side.
    core::InsertPlacement m_insertPlacement = core::InsertPlacement::Below;

    /// The encoding last chosen in `Save As…`, absent until one has been.
    ///
    /// **It serves the document with no file, and nothing else.** An opened
    /// document carries its own, and that is the one the box proposes: the byte
    /// round trip of phase 8 is that promise, and a setting does not undo it
    /// behind the back of whoever saves.
    std::optional<core::Encoding> m_writeEncoding;

    /// The root of the installed manual, or nothing.
    std::filesystem::path m_manualDirectory;

    /// The directory the "open" box will open in.
    ///
    /// That of the last file **opened or saved**, and not that of a box
    /// dismissed: what counts is where the user works, not where they looked.
    std::filesystem::path m_lastDirectory;

    /// Whether a film is open and being drawn.
    bool m_watching = false;

    /// Whether the window has been on screen once.
    ///
    /// Nothing is handed to a player before it has: see `showEvent`.
    bool m_wasShown = false;

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
