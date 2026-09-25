#pragma once

#include <subedit/core/config/insert_placement.hpp>
#include <subedit/core/config/settings.hpp>
#include <subedit/core/edit/clipboard.hpp>
#include <subedit/core/edit/duration_adjustment.hpp>
#include <subedit/core/edit/search.hpp>
#include <subedit/core/format/project_file.hpp>
#include <subedit/core/model/selection.hpp>
#include <subedit/gui/player_factory.hpp>
#include <subedit/gui/status_line.hpp>
#include <subedit/gui/subtitle_table.hpp>
#include <subedit/gui/video_pane.hpp>
#include <subedit/gui/window_actions.hpp>

#include <QMainWindow>
#include <QStringList>

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
class VideoPlayer;
} // namespace subedit::core

class QAction;
class QCloseEvent;
class QDragEnterEvent;
class QDropEvent;
class QLabel;
class QShowEvent;
class QSplitter;
class QTabBar;
class QToolButton;
class QTimer;

namespace subedit::gui {

class SearchDialog;

class DiagnosticsPanel;
class ManualWindow;
class Prompts;
struct ProjectPage;
struct ModifiedDocument;
class SubtitleTableModel;
class TableColumns;
class ProjectSearch;
class ProjectFiles;

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
    [[nodiscard]] QAction* undoAction() const { return m_actions->undo; }

    [[nodiscard]] QAction* redoAction() const { return m_actions->redo; }

    [[nodiscard]] QAction* openAction() const { return m_actions->open; }

    /// `File ▸ New` and `File ▸ Close` — a project of its own in a new tab,
    /// and the current tab taken away. `GUI-TABS-01`.
    ///
    /// **`Close` is out with one tab left**: the window always holds at least
    /// one project, and closing the last would mean closing the window, which
    /// is what the title bar's own button already does.
    [[nodiscard]] QAction* newProjectAction() const { return m_actions->newProject; }

    [[nodiscard]] QAction* closeProjectAction() const { return m_actions->closeProject; }

    /// `Projects ▸ Save All` and `Projects ▸ Close All`. `GUI-SAVE-04`,
    /// `GUI-TABS-02`.
    ///
    /// **`Close All` is the window's own close**: the window always holds one
    /// project, so closing all of them and closing the window are one act, and
    /// they ask one question.
    [[nodiscard]] QAction* saveAllDocumentsAction() const { return m_actions->saveAllDocuments; }

    [[nodiscard]] QAction* closeAllProjectsAction() const { return m_actions->closeAllProjects; }

    /// `Ctrl+PageDown` and `Ctrl+PageUp`, wrapping around at either end. No
    /// menu entry: the bar already offers a click, and these are for whoever
    /// would rather not reach for the mouse.
    [[nodiscard]] QAction* nextTabAction() const { return m_actions->nextTab; }

    [[nodiscard]] QAction* previousTabAction() const { return m_actions->previousTab; }

    /// One tab per open project, in the order they were opened. A test reads
    /// its count and its labels, and drives it the way a click would —
    /// `setCurrentIndex` fires the same signal either way.
    [[nodiscard]] QTabBar* tabBar() const { return m_tabBar; }

    /// The « + » after the last tab, which opens a new project — issue #473.
    [[nodiscard]] QToolButton* newTabButton() const { return m_newTab; }

    [[nodiscard]] QAction* saveAction() const { return m_actions->save; }

    [[nodiscard]] QAction* saveAsAction() const { return m_actions->saveAs; }

    /// The three entries of the translation — `File ▸ Open Translation…`,
    /// `Save Translation` and `Save Translation As…` — for a test to read their
    /// state and to fire them.
    ///
    /// **The last two are out while the project has no translation**: there is
    /// nothing to write. Opening one is out while there is nothing to align it
    /// to — an empty document has no subtitle to give its lines to.
    [[nodiscard]] QAction* openTranslationAction() const { return m_actions->openTranslation; }

    [[nodiscard]] QAction* saveTranslationAction() const { return m_actions->saveTranslation; }

    [[nodiscard]] QAction* saveTranslationAsAction() const { return m_actions->saveTranslationAs; }

    /// The three clipboard entries, for a test to read their state and trigger
    /// them.
    [[nodiscard]] QAction* cutAction() const { return m_actions->cut; }

    [[nodiscard]] QAction* copyAction() const { return m_actions->copy; }

    [[nodiscard]] QAction* pasteAction() const { return m_actions->paste; }

    [[nodiscard]] QAction* findAndReplaceAction() const { return m_actions->findAndReplace; }

    /// The search dialog once it has been opened, and nothing before.
    [[nodiscard]] SearchDialog* searchDialog() const;

    /// The two edits of structure, for a test to read their state and trigger
    /// them.
    [[nodiscard]] QAction* insertAction() const { return m_actions->insert; }

    [[nodiscard]] QAction* removeAction() const { return m_actions->remove; }

    [[nodiscard]] QAction* mergeAction() const { return m_actions->mergeSubtitles; }

    [[nodiscard]] QAction* splitAction() const { return m_actions->splitSubtitle; }

    /// The panel of what the last reading ran into.
    [[nodiscard]] DiagnosticsPanel* diagnostics() const { return m_diagnostics; }

    [[nodiscard]] QAction* shiftAction() const { return m_actions->shift; }

    [[nodiscard]] QAction* transformAction() const { return m_actions->transform; }

    [[nodiscard]] QAction* frameRateAction() const { return m_actions->frameRate; }

    [[nodiscard]] QAction* adjustDurationsAction() const { return m_actions->adjustDurations; }

    [[nodiscard]] QAction* appendFileAction() const { return m_actions->appendFile; }

    /// `Tools ▸ Split Project…` — the inverse of appending, `GUI-PSPLIT-01`.
    /// Out under two subtitles: a cut needs one on each side.
    [[nodiscard]] QAction* splitProjectAction() const { return m_actions->splitProject; }

    [[nodiscard]] QAction* hearingImpairedAction() const { return m_actions->hearingImpaired; }

    /// The one button that puts a text in italics and takes them out again.
    ///
    /// **Out for a format that writes no style**, which is what says to a user
    /// of a `.lrc` that there is nothing to type — an entry that is there and
    /// grey answers « why can I not? », an entry that is gone does not.
    [[nodiscard]] QAction* italicAction() const { return m_actions->italic; }

    /// The entry that puts the target in `wanted`, for a test to fire it.
    [[nodiscard]] QAction* caseAction(core::LetterCase wanted) const;

    /// The one entry that puts dialogue dashes on and takes them off.
    [[nodiscard]] QAction* dialogueDashesAction() const { return m_actions->dialogueDashes; }

    /// The entry that opens the preferences, for a test to trigger it.
    [[nodiscard]] QAction* preferencesAction() const { return m_actions->preferences; }

    [[nodiscard]] QAction* selectVideoAction() const { return m_actions->selectVideo; }

    [[nodiscard]] QAction* playPauseAction() const { return m_actions->playPause; }

    /// The surface the film is drawn on, for a test to read whether it is
    /// there at all. Hidden while no film is open, which is what « the table
    /// takes the whole window » means.
    [[nodiscard]] QWidget* videoView() const { return m_video->picture(); }

    /// What stands where the picture would be while there is no film: a way in,
    /// rather than an absence a user has to guess is one.
    [[nodiscard]] QWidget* noVideoBanner() const { return m_video->banner(); }

    /// What the status bar says of the associated film — its name, or that
    /// there is none. This is what `GUI-VIDEO-01` promises the user sees.
    [[nodiscard]] QLabel* videoStatus() const { return m_status->video(); }

    /// What the status bar says of the grid the positions were written on.
    /// This is what `GUI-GRID-01` promises the user sees.
    [[nodiscard]] QLabel* gridStatus() const { return m_status->grid(); }

    /// What the status bar says of the encoding the document was read in.
    /// This is what `GUI-ENC-01` promises the user sees.
    [[nodiscard]] QLabel* encodingStatus() const { return m_status->encoding(); }

    /// The entry of the `View` menu that shows the translation column or takes
    /// it away — `GUI-TRANS-04`. Out for as long as the project has no
    /// translation: there is nothing to show.
    [[nodiscard]] QAction* translationColumnAction() const;

    /// The entry of `View ▸ Columns` that shows `column` or takes it away —
    /// `GUI-TABLE-03`. The translation's is `translationColumnAction`; the text
    /// has none, and this answers nothing for it.
    [[nodiscard]] QAction* columnAction(core::TableColumn column) const;

    /// What the status bar says of the text an operation aims at, and nothing
    /// while there is only one. This is what `GUI-TRANS-05` promises the user
    /// sees.
    [[nodiscard]] QLabel* targetStatus() const { return m_status->target(); }

    [[nodiscard]] QAction* analyseGridAction() const { return m_actions->analyseGrid; }

    [[nodiscard]] QAction* snapAction() const { return m_actions->snap; }

    [[nodiscard]] QAction* aboutAction() const { return m_actions->about; }

    /// Opens the installed manual. Out for as long as there is none.
    [[nodiscard]] QAction* manualAction() const { return m_actions->manual; }

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

    /// Opens what was dropped on the window — issue #453, `GUI-TABS-04`.
    ///
    /// **`Open…`'s road for each subtitle file**, in the order they came: a tab
    /// each, and a file already open brings its tab forward instead of being
    /// read again. **`Select Video…`'s for a film**, once the subtitles are open
    /// — so given to the tab shown then, the last one this drop opened if it
    /// opened any. A film is told apart by its extension alone, as the naming
    /// convention tells it. Two films or more are left alone: a project watches
    /// one.
    ///
    /// **A file that will not open does not stop the others**, and what could
    /// not be done is said once, a line each.
    ///
    /// Public because `dropEvent` only unpacks the paths: the sorting is what a
    /// test drives.
    void openDropped(std::span<const std::filesystem::path> paths);

    /// The manual window, if it is open. For a test to read it.
    [[nodiscard]] ManualWindow* manualWindow() const { return m_manualWindow; }

    /// The names of the menus, in the order the bar shows them.
    [[nodiscard]] QStringList menuTitles() const;

    /// Bringing the file back onto its own grid. Its text carries the measured
    /// amount, which is how `GUI-GRID-03` shows it before it is applied.
    [[nodiscard]] QAction* shiftOntoGridAction() const { return m_actions->shiftOntoGrid; }

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

    /// Takes a drag that carries files, and nothing else — issue #453.
    void dragEnterEvent(QDragEnterEvent* event) override;

    /// Hands the local files a drop carries to `openDropped`.
    void dropEvent(QDropEvent* event) override;

private:
    /// Shows or hides the columns as `View ▸ Columns` and the current project
    /// say — `TableColumns::refresh` on the page shown.
    void refreshColumns();

    /// The text an operation of text aims at: the translation when the current
    /// cell is in its column and the column is shown, the main text otherwise.
    ///
    /// **The rule of Gaupol, without its grey** — `text_column_to_document`
    /// greys the operations of text outside a text column, and here nothing is
    /// greyed: without a translation column the target is always the main text,
    /// and the window behaves as it did before the translation existed.
    [[nodiscard]] core::Document targetDocument() const;

    /// Puts what depends on the target in step with it: the status bar, the
    /// italic entry, which follows the format of the document aimed at, and the
    /// search, which forgets a match found in the other text and names the field
    /// its box looks in.
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

    /// Says in the tab of `page` its name, and whether it is modified.
    ///
    /// **Any page and not only the one shown**: `Replace All` over every
    /// project and `Save All` reach pages that stay behind their tabs — issue
    /// #461 — and each tab has to say what its own page now is.
    void refreshTabOf(const ProjectPage& page);

    /// Opens `project` in a new tab, and switches to it — ADR 0033, `GUI-TABS-01`.
    void openOn(core::Project project, std::span<const core::Diagnostic> diagnostics);

    /// Switches to the page at `index`, doing nothing when it is already the
    /// one showing.
    ///
    /// **The one road to a change of tab**, whether a click on the bar fires
    /// it, a new page is born on it, or `closeEvent` walks every page in turn:
    /// one function that recalculates the window rather than several that
    /// might one day recalculate it differently.
    void switchToPage(int index);

    /// Puts what depends on the current page in step with it: the title, the
    /// picture, the panel of what its reading met, and everything
    /// `refreshActions` already cascades to — the grid, the encoding, the
    /// translation column, the target, the search box.
    void refreshForPage();

    /// `File ▸ New`: an empty project in a new tab.
    void newProject();

    /// `File ▸ Close`: `closeProject` on the current tab.
    void closeCurrentProject();

    /// Asks about the modified documents of the project at `index`, then takes
    /// its tab away — the current one or not: the cross of a tab behind closes
    /// that tab, and the one shown stays shown (#472). A current tab closed
    /// gives way to its neighbour.
    void closeProject(int index);

    /// Takes the page at `index` and its tab away, asking nothing: whoever
    /// calls has asked already, or knows there is nothing to lose. A current
    /// page gives way to its neighbour; one behind leaves the shown one shown.
    void removePage(int index);

    /// Whether `Close` and the crosses of the tabs may do anything — neither
    /// with one tab left.
    void refreshTabActions();

    /// Whether `document` of the project on screen differs from its file —
    /// `ProjectFiles::isModified`.
    [[nodiscard]] bool isModified(core::Document document) const;

    /// Asks which translation to open and how to align it, then opens it.
    void openTranslationFromPrompt();

    void openFromPrompt();

    /// Opens `path` in a tab of its own, or brings forward the one that already
    /// holds it — what `Open…` and a drop share. Says why the file will not
    /// open, or nothing when it opened or was already there.
    [[nodiscard]] std::optional<std::string> openFile(const std::filesystem::path& path);

    /// Applies `command` to `page`, over `target`, and refreshes what the
    /// window shows.
    ///
    /// **`page` need not be the one on screen** — issue #461: `Replace All`
    /// over every project reaches each page without bringing its tab forward.
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
    void applyOperation(ProjectPage& page,
                        std::unique_ptr<core::Command> command,
                        const core::Selection& target);

    /// The same, and it says nothing: what the operation left past the end of
    /// the film comes back as the sentence to say, empty when there is none.
    ///
    /// **Why the box is not opened here**: `reportOutcome` is modal, and an
    /// operation with an account of its own to give — what an adjustment could
    /// not satisfy, what an alignment left behind — used to open a second one
    /// straight after the first. One operation, one box.
    [[nodiscard]] std::string applyOperationQuietly(ProjectPage& page,
                                                    std::unique_ptr<core::Command> command,
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
    [[nodiscard]] std::string whatPassesTheEnd(const ProjectPage& page,
                                               core::CommandKind kind,
                                               const core::Selection& target) const;

    /// Asks which film to watch the document against, and associates it.
    void selectVideo();

    /// Offers the film the naming convention finds beside the subtitle file.
    ///
    /// Called wherever the file's name becomes known or changes — an opening,
    /// a « save as » — because that name is all the convention reads. A choice
    /// already made is never replaced: D5 lives in `Project`, so calling this
    /// too often costs nothing but a look at a directory.
    void proposeVideoBeside();

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

    /// Opens the search dialog, or brings it back to the front —
    /// `ProjectSearch::open`.
    void openSearch();

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

    /// Asks for a file, and appends it to the end of the project — D6.
    void appendFileFromPrompt();

    /// Asks where to cut, and moves the tail into a project of its own, in a
    /// new tab — D6.
    void splitProjectFromPrompt();

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

    /// The four standing facts of the status bar — issue #485.
    std::unique_ptr<StatusLine> m_status;
    DiagnosticsPanel* m_diagnostics = nullptr;

    /// Every action, the menus and the toolbar — issue #483.
    std::unique_ptr<WindowActions> m_actions;

    /// The columns of the table and the entries of `View ▸ Columns` — ADR 0034.
    std::unique_ptr<TableColumns> m_columns;
    ManualWindow* m_manualWindow = nullptr;
    QSplitter* m_split = nullptr;
    QTabBar* m_tabBar = nullptr;
    QToolButton* m_newTab = nullptr;

    /// What the video asks of the window, and the video itself — issue #484.
    /// Declared after the table and the splitter it is built on, the side
    /// before the pane that holds a reference to it — and the pane, which owns
    /// the player, goes before the widgets the window owns: #470.
    class VideoSide;
    std::unique_ptr<VideoSide> m_videoSide;
    std::unique_ptr<VideoPane> m_video;

    /// The theme asked for, to be handed back to the settings. Laid down, not
    /// deduced: the current palette does not say which of the three made it.
    core::Theme m_theme = core::Theme::System;

    /// Which side of the selection the next insertion will lay its rows on.
    ///
    /// Kept from one call to the next, and handed back to the settings: one
    /// does not insert once but ten times in a row, always on the same side.
    core::InsertPlacement m_insertPlacement = core::InsertPlacement::Below;

    /// The texts last copied or cut in this window, with their format.
    ///
    /// **Kept across openings**, and that is its reason to exist: copying from
    /// one file and pasting into the next is the one case where the format of
    /// the copy and that of the document differ.
    core::ClipboardTexts m_clipboard;

    /// The form of the last adjustment of durations, offered again by the next
    /// one. Gaupol's defaults until then.
    core::DurationAdjustmentSettings m_durationSettings;

    /// What the search asks of the window, and the search itself — ADR 0034.
    /// The first is declared before the second, which holds a reference to it.
    class SearchSide;
    std::unique_ptr<SearchSide> m_searchSide;
    std::unique_ptr<ProjectSearch> m_search;

    /// What the files ask of the window, and the files themselves — ADR 0034.
    class FilesSide;
    std::unique_ptr<FilesSide> m_filesSide;
    std::unique_ptr<ProjectFiles> m_projectFiles;

    /// The root of the installed manual, or nothing.
    std::filesystem::path m_manualDirectory;

    /// Every open project — ADR 0033, `GUI-TABS-01`. One tab, one entry, in
    /// the order they were opened; never empty, since a window with nothing
    /// left to show a blank one rather than none.
    std::vector<std::unique_ptr<ProjectPage>> m_pages;

    /// The one `m_pages` holds that the window shows — the session and its
    /// history, the table model, where playback was placed, the video
    /// associated with this project and whether it is drawn, the replica the
    /// overlay carries, the search's own state, and the encoding last chosen
    /// for a document with no file of its own.
    ///
    /// **A raw, non-owning pointer into `m_pages`**, repointed by
    /// `switchToPage` and never itself allocated or freed: what owns a page
    /// is the vector, and this only says which one is current.
    ProjectPage* m_page = nullptr;

    /// The index `m_page` sits at in `m_pages` — what `switchToPage` compares
    /// a request against to tell « already showing » from « switch ».
    int m_currentPage = -1;
};

} // namespace subedit::gui
