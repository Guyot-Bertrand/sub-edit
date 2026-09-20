#pragma once

#include <subedit/core/config/insert_placement.hpp>
#include <subedit/core/config/settings.hpp>
#include <subedit/core/edit/clipboard.hpp>
#include <subedit/core/edit/duration_adjustment.hpp>
#include <subedit/core/edit/search.hpp>
#include <subedit/core/format/project_file.hpp>
#include <subedit/core/model/selection.hpp>
#include <subedit/gui/player_factory.hpp>
#include <subedit/gui/subtitle_table.hpp>

#include <QMainWindow>
#include <QStringList>

#include <array>
#include <cstdint>
#include <filesystem>
#include <functional>
#include <memory>
#include <span>
#include <string>
#include <vector>

namespace subedit::core {
class Command;
enum class CommandKind;
enum class Document;
enum class LetterCase;
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

class SearchDialog;

class DiagnosticsPanel;
class ManualWindow;
class Prompts;
struct ModifiedDocument;
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

    /// The three entries of the translation — `File ▸ Open Translation…`,
    /// `Save Translation` and `Save Translation As…` — for a test to read their
    /// state and to fire them.
    ///
    /// **The last two are out while the project has no translation**: there is
    /// nothing to write. Opening one is out while there is nothing to align it
    /// to — an empty document has no subtitle to give its lines to.
    [[nodiscard]] QAction* openTranslationAction() const { return m_openTranslation; }

    [[nodiscard]] QAction* saveTranslationAction() const { return m_saveTranslation; }

    [[nodiscard]] QAction* saveTranslationAsAction() const { return m_saveTranslationAs; }

    /// The three clipboard entries, for a test to read their state and trigger
    /// them.
    [[nodiscard]] QAction* cutAction() const { return m_cut; }

    [[nodiscard]] QAction* copyAction() const { return m_copy; }

    [[nodiscard]] QAction* pasteAction() const { return m_paste; }

    [[nodiscard]] QAction* findAndReplaceAction() const { return m_findAndReplace; }

    /// The search dialog once it has been opened, and nothing before.
    [[nodiscard]] SearchDialog* searchDialog() const { return m_search; }

    /// The two edits of structure, for a test to read their state and trigger
    /// them.
    [[nodiscard]] QAction* insertAction() const { return m_insert; }

    [[nodiscard]] QAction* removeAction() const { return m_remove; }

    [[nodiscard]] QAction* mergeAction() const { return m_mergeSubtitles; }

    [[nodiscard]] QAction* splitAction() const { return m_splitSubtitle; }

    /// The panel of what the last reading ran into.
    [[nodiscard]] DiagnosticsPanel* diagnostics() const { return m_diagnostics; }

    [[nodiscard]] QAction* shiftAction() const { return m_shift; }

    [[nodiscard]] QAction* transformAction() const { return m_transform; }

    [[nodiscard]] QAction* frameRateAction() const { return m_frameRate; }

    [[nodiscard]] QAction* adjustDurationsAction() const { return m_adjustDurations; }

    [[nodiscard]] QAction* hearingImpairedAction() const { return m_hearingImpaired; }

    /// The one button that puts a text in italics and takes them out again.
    ///
    /// **Out for a format that writes no style**, which is what says to a user
    /// of a `.lrc` that there is nothing to type — an entry that is there and
    /// grey answers « why can I not? », an entry that is gone does not.
    [[nodiscard]] QAction* italicAction() const { return m_italic; }

    /// The entry that puts the target in `wanted`, for a test to fire it.
    [[nodiscard]] QAction* caseAction(core::LetterCase wanted) const;

    /// The one entry that puts dialogue dashes on and takes them off.
    [[nodiscard]] QAction* dialogueDashesAction() const { return m_dialogueDashes; }

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

    /// The entry of the `View` menu that shows the translation column or takes
    /// it away — `GUI-TRANS-04`. Out for as long as the project has no
    /// translation: there is nothing to show.
    [[nodiscard]] QAction* translationColumnAction() const { return m_translationColumn; }

    /// What the status bar says of the text an operation aims at, and nothing
    /// while there is only one. This is what `GUI-TRANS-05` promises the user
    /// sees.
    [[nodiscard]] QLabel* targetStatus() const { return m_targetStatus; }

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
    /// Shows the translation column, or takes it away, as the project and the
    /// entry of the `View` menu together say.
    void refreshTranslationColumn();

    /// The text an operation of text aims at: the translation when the current
    /// cell is in its column and the column is shown, the main text otherwise.
    ///
    /// **The rule of Gaupol, without its grey** — `text_column_to_document`
    /// greys the operations of text outside a text column, and here nothing is
    /// greyed: without a translation column the target is always the main text,
    /// and the window behaves as it did before the translation existed.
    [[nodiscard]] core::Document targetDocument() const;

    /// Puts what depends on the target in step with it: the status bar, and
    /// the italic entry, which follows the format of the document aimed at.
    void refreshTarget();

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

    /// The same for the translation, which is a file of its own.
    [[nodiscard]] bool saveTranslation();

    [[nodiscard]] bool saveTranslationAs();

    /// Writes one of the two documents, or asks where — what the four above
    /// are, once the document is a parameter. **One body and not two**: the
    /// dialog, the warning about a loss, the failure that leaves the document
    /// where it was are the same for both, and they had been written once
    /// already.
    [[nodiscard]] bool saveDocument(core::Document document);

    [[nodiscard]] bool saveDocumentAs(core::Document document);

    /// Whether `document` differs from its file. **The translation only counts
    /// while the project has one**: a translation that was undone away has no
    /// file to differ from.
    [[nodiscard]] bool isModified(core::Document document) const;

    /// The documents a closing would lose, in the order the window shows them.
    ///
    /// **A file gone from the disk counts as modified**, and is marked as such:
    /// what the window holds is then the only copy of it.
    [[nodiscard]] std::vector<ModifiedDocument> modifiedDocuments() const;

    /// Returns whether whatever is about to lose the changes may go on.
    ///
    /// **One question however many documents are modified.** Nothing modified
    /// goes on; one asks what it always asked; two ask through the list, with a
    /// box each.
    [[nodiscard]] bool mayDiscardChanges();

    /// Returns whether a translation that is open may be replaced — asked
    /// before the file is, as for the main document.
    [[nodiscard]] bool mayReplaceTranslation();

    /// Asks which translation to open and how to align it, then opens it.
    void openTranslationFromPrompt();

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
    ///
    /// Says what it left past the end of the film in a box of its own. An
    /// operation that has something to say as well uses `applyOperationQuietly`
    /// and puts the two in one box — issue #418.
    void applyOperation(std::unique_ptr<core::Command> command, const core::Selection& target);

    /// The same, and it says nothing: what the operation left past the end of
    /// the film comes back as the sentence to say, empty when there is none.
    ///
    /// **Why the box is not opened here**: `reportOutcome` is modal, and an
    /// operation with an account of its own to give — what an adjustment could
    /// not satisfy, what an alignment left behind — used to open a second one
    /// straight after the first. One operation, one box.
    [[nodiscard]] std::string applyOperationQuietly(std::unique_ptr<core::Command> command,
                                                    const core::Selection& target);

    /// What an operation left past the end of the film, said as a sentence, or
    /// nothing.
    ///
    /// **A notice, never a refusal** — decision D4. A subtitle landing after
    /// the closing credits may be exactly what was meant; refusing wrongly
    /// costs more than a warning that is ignored.
    ///
    /// Empty without a film open: the length is what the player knows, and
    /// there is nothing to be past the end of.
    [[nodiscard]] std::string whatPassesTheEnd(core::CommandKind kind,
                                               const core::Selection& target) const;

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

    /// Copies the texts of the selection, to this window and to the system.
    ///
    /// **Both, and for two different readers.** The system clipboard receives
    /// plain text, which is what makes a copy pasteable anywhere; the window
    /// keeps the texts with their format, which is what lets a paste into a
    /// document of another format translate the tags.
    void copyTexts();

    /// Copies the texts of the selection, then empties them.
    void cutTexts();

    /// Writes the clipboard into the texts from the first selected row down.
    ///
    /// **What the system holds decides.** When it is still what this window
    /// copied, the format comes with it and the tags are translated; when a
    /// copy was made elsewhere since, that one is meant, and it has no format.
    /// Rows laid down past the end, and tags a translation dropped, are said.
    void pasteTexts();

    /// Opens the search dialog, or brings it back to the front.
    ///
    /// **The same dialog every time**, kept from one opening to the next with
    /// what was typed in it: finding again is the common case.
    void openSearch();

    /// Finds the next or the previous match in the search target, and moves
    /// the table to it.
    ///
    /// **This one and the two after it answer the dialog's signals and nothing
    /// else**, so the dialog exists whenever they run: `openSearch` makes it
    /// before connecting them, and a test reaches them only by pressing its
    /// buttons.
    void findInTarget(bool forward);

    /// Replaces the match last found, then finds the next one.
    void replaceCurrentMatch();

    /// Replaces every match of the search target, as one entry in the history.
    void replaceAllInTarget();

    /// Compiles what the dialog asks for, or shows why it cannot be.
    [[nodiscard]] std::optional<core::SearchPattern> searchPattern();

    /// What the search walks: the selection, or the whole document — captured
    /// when a search starts, and kept while the search itself moves the
    /// selection from match to match.
    [[nodiscard]] core::Selection searchTarget();

    /// Merges the selected rows into one, and selects it.
    ///
    /// **A contiguous run of two or more**, which is what the action being out
    /// holds: merging rows one and three would swallow row two under an
    /// overlap. Gaupol refuses the same selection.
    void mergeSubtitles();

    /// Splits the selected row in two at the middle of its duration, and
    /// selects both halves.
    ///
    /// Both, and not the first: it is what Gaupol does after any insertion, and
    /// it leaves `Merge Subtitles` ready to take the split back.
    void splitSubtitle();

    /// Selects the range of rows given, and brings it into view.
    ///
    /// What Gaupol does after an insertion and after a removal: the table has
    /// been reset, so the selection is gone, and without this a second `Ins` or
    /// a second `Del` would find nothing left to work from.
    void selectRows(int first, int last);

    void shiftTarget();

    void transformTarget();

    void convertFrameRateOfTarget();

    /// Asks for the four constraints, applies them to the target, and says what
    /// no end could satisfy.
    ///
    /// **Said even when nothing moved**: a target already at its gaps may still
    /// hold subtitles too short for their minimum, and « nothing to adjust »
    /// alone would let that pass for « everything is fine ».
    void adjustDurationsOfTarget();

    void removeHearingImpairedFromTarget();

    /// Puts the target in italics, or takes its italics out.
    ///
    /// **One entry and not two**, as in Gaupol: which of the two it does is
    /// read from the target before anything is built, and a mixed selection
    /// goes into italics whole.
    ///
    /// No dialog — the operation takes no option, and it enters the history
    /// like the others, so `Ctrl+Z` undoes it.
    void toggleItalicsOnTarget();

    /// Puts the target in `wanted`.
    ///
    /// No dialog: the case takes no option beyond which of the four, and the
    /// menu already said it.
    void changeCaseOfTarget(core::LetterCase wanted);

    /// Puts dialogue dashes on the target, or takes them off.
    ///
    /// **One entry and not two**, as for the italic: which of the two it does
    /// is read from the target before anything is built.
    void toggleDialogueDashesOnTarget();

    /// Commits and closes the cell editor open on the table, if any — the same
    /// validation a modal dialog already gets by taking the focus away.
    ///
    /// **Ahead of every gesture without a dialog** — the seven issue #397 names
    /// (the italic, the case, the dialogue dashes, merging, splitting, cutting
    /// and pasting texts), and the four issue #416 added (copying texts,
    /// removing subtitles, undo and redo) — so that the target each reads and
    /// the command it builds see the edit already applied rather than racing
    /// it. **Undo and redo are the two that read the history and not the
    /// document**: the typing goes into it first, so an undo takes the typing
    /// off and not what came before it, and a redo finds nothing left to redo,
    /// as typing after an undo does anywhere else. A gesture with a dialog does
    /// not call it: opening the dialog takes the focus, which does the same
    /// thing. Giving the table itself the focus is what a click elsewhere
    /// already does, and the delegate's own focus-out handling — Qt's,
    /// unmodified — takes it from there: it validates rather than discards, the
    /// same answer a dialog already gave.
    void commitCellEditor();

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
    QAction* m_openTranslation = nullptr;
    QAction* m_saveTranslation = nullptr;
    QAction* m_saveTranslationAs = nullptr;
    QAction* m_cut = nullptr;
    QAction* m_copy = nullptr;
    QAction* m_paste = nullptr;
    QAction* m_findAndReplace = nullptr;
    QAction* m_insert = nullptr;
    QAction* m_remove = nullptr;
    QAction* m_mergeSubtitles = nullptr;
    QAction* m_splitSubtitle = nullptr;
    QAction* m_shift = nullptr;
    QAction* m_transform = nullptr;
    QAction* m_frameRate = nullptr;
    QAction* m_adjustDurations = nullptr;
    QAction* m_hearingImpaired = nullptr;
    QAction* m_italic = nullptr;
    std::array<QAction*, 4> m_case{};
    QAction* m_dialogueDashes = nullptr;
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
    QLabel* m_targetStatus = nullptr;
    QAction* m_translationColumn = nullptr;
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

    /// The texts last copied or cut in this window, with their format.
    ///
    /// **Kept across openings**, and that is its reason to exist: copying from
    /// one file and pasting into the next is the one case where the format of
    /// the copy and that of the document differ.
    core::ClipboardTexts m_clipboard;

    /// The form of the last adjustment of durations, offered again by the next
    /// one. Gaupol's defaults until then.
    core::DurationAdjustmentSettings m_durationSettings;

    /// The search dialog, made at its first opening and kept.
    SearchDialog* m_search = nullptr;

    /// The two options of a search, which the preferences carry.
    core::SearchOptions m_searchOptions;

    /// The match last found, which `Find Next` starts after and `Replace`
    /// rewrites. Forgotten when the pattern, an option or the document
    /// changes — including a structural undo or redo, which resets the model
    /// rather than reporting the change.
    std::optional<core::TextMatch> m_match;

    /// The target of the search under way, captured at its first gesture.
    ///
    /// **Captured and not read again**, because the search itself moves the
    /// selection: read at every `Find Next`, the target would shrink to the row
    /// of the last match. A selection the user makes resets it.
    std::optional<core::Selection> m_searchTarget;

    /// Set while the search moves the selection, so that the move is not
    /// mistaken for the user choosing another target.
    bool m_movingToMatch = false;

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
