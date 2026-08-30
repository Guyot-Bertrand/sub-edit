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
    /// **Appelée avant `show()`**, sans quoi la fenêtre s'affiche à sa taille
    /// par défaut puis saute à la sienne, ce qui se voit.
    ///
    /// Ce qui est absent des réglages n'est pas appliqué : une géométrie
    /// absente laisse la fenêtre se dimensionner elle-même, assez large pour
    /// qu'on lise la table — c'est le défaut, et il vaut mieux que zéro.
    void applySettings(const core::Settings& settings);

    /// Ce que cette session laisse derrière elle.
    ///
    /// Lue une fois la fenêtre fermée, par le câblage qui l'écrira. La fenêtre
    /// ne persiste rien elle-même : elle dit son état, et c'est tout — la même
    /// séparation qu'entre `Session` et `saveProject`.
    ///
    /// **La géométrie rendue est celle d'avant l'agrandissement** quand la
    /// fenêtre est maximisée : Qt garde les deux, et retenir la taille de
    /// l'écran comme géométrie normale ferait qu'un dé-maximisage à la session
    /// suivante ne rendrait rien à voir.
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

    /// Les deux éditions de structure, pour qu'un test lise leur état et les
    /// déclenche.
    [[nodiscard]] QAction* insertAction() const { return m_insert; }

    [[nodiscard]] QAction* removeAction() const { return m_remove; }

    /// The panel of what the last reading ran into.
    [[nodiscard]] DiagnosticsPanel* diagnostics() const { return m_diagnostics; }

    [[nodiscard]] QAction* shiftAction() const { return m_shift; }

    [[nodiscard]] QAction* transformAction() const { return m_transform; }

    [[nodiscard]] QAction* frameRateAction() const { return m_frameRate; }

    [[nodiscard]] QAction* hearingImpairedAction() const { return m_hearingImpaired; }

    /// L'entrée qui ouvre les préférences, pour qu'un test la déclenche.
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

    [[nodiscard]] QAction* analyseGridAction() const { return m_analyseGrid; }

    [[nodiscard]] QAction* snapAction() const { return m_snap; }

    [[nodiscard]] QAction* aboutAction() const { return m_about; }

    /// Ouvre le manuel installé. Éteinte tant qu'il n'y en a pas.
    [[nodiscard]] QAction* manualAction() const { return m_manual; }

    /// Dit où le manuel installé se trouve, et allume l'entrée s'il y est.
    ///
    /// **Reçu plutôt que résolu**, comme les réglages et pour la même raison —
    /// ADR 0022 : `gui::installedManualPath()` est le seul code qui sait où
    /// regarder, `main` l'appelle et passe la réponse ici. Un test donne le
    /// chemin qu'il veut, et n'atteint donc jamais le vrai manuel.
    ///
    /// **L'entrée reste éteinte quand le manuel n'est pas là**, ce qui est le
    /// cas d'un binaire lancé depuis l'arbre de construction et celui d'une
    /// installation partielle. C'est ce qui tient la promesse du cadrage : le
    /// manuel absent ne fait rien planter, il éteint une entrée.
    void setManualPath(std::filesystem::path directory);

    /// La fenêtre du manuel, si elle est ouverte. Pour qu'un test la lise.
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
    /// Recompute ce que les deux éditions de structure ont le droit de faire.
    ///
    /// **À part de `refreshActions`, et branchée sur la sélection** : ce sont
    /// les deux seules actions dont l'état dépend de ce qui est sélectionné, et
    /// `refreshActions` déduit la grille du fichier entier. La brancher sur la
    /// sélection ferait payer cette déduction à chaque ligne d'un cliquer-tirer
    /// sur quatre mille lignes.
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

    /// Opens the analysis, which reports and changes nothing.
    void analyseGrid();

    /// Says who this is and which version is running.
    void about();

    /// Ouvre le manuel, ou ramène au premier plan celui qui est déjà ouvert.
    void openManual();

    /// Ouvre les préférences, et pose ce qui en sort.
    void openPreferences();

    /// Retient le répertoire de `file` comme celui où la prochaine boîte
    /// « ouvrir » s'ouvrira.
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

    /// Demande combien de lignes vierges, et où, puis les pose.
    ///
    /// **L'index est celui du dernier sélectionné**, plus un si le côté choisi
    /// est « après ». C'est ce que Gaupol fait depuis vingt ans, et c'est le
    /// point qu'on invente mal si on ne le lit pas : le premier sélectionné
    /// paraît plus naturel et n'est pas ce que la main attend après avoir
    /// balayé du haut vers le bas.
    ///
    /// Dans un document vide, l'index est zéro et aucune sélection n'est
    /// exigée — c'est la seule façon de commencer un fichier neuf.
    void insertSubtitles();

    /// Retire la sélection, sans rien demander.
    ///
    /// **Sans confirmation, et ce n'est pas une négligence** : l'opération
    /// entre dans l'historique comme les autres, donc `Ctrl+Z` la défait. Une
    /// modale devant un geste annulable coûte un clic à chaque fois pour
    /// épargner un `Ctrl+Z` de temps en temps.
    ///
    /// **La sélection, et jamais le fichier entier.** `targetOf` lit « rien de
    /// sélectionné » comme « tout », ce qui est juste pour un décalage et
    /// serait un désastre ici ; l'action est éteinte quand rien n'est
    /// sélectionné, et cette fonction ne la rattrape pas — elle n'a pas à
    /// connaître deux règles.
    void removeSubtitles();

    /// Sélectionne la plage de lignes donnée, et l'amène sous les yeux.
    ///
    /// Ce que Gaupol fait après une insertion et après une suppression : la
    /// table a été réinitialisée, donc la sélection a disparu, et sans cela un
    /// second `Ins` ou un second `Suppr` ne trouverait plus rien à quoi se
    /// rapporter.
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

    /// Le thème demandé, qu'on rendra aux réglages. Posé, pas déduit : la
    /// palette courante ne dit pas lequel des trois l'a produite.
    core::Theme m_theme = core::Theme::System;

    /// De quel côté de la sélection la prochaine insertion posera ses lignes.
    ///
    /// Retenu d'un appel à l'autre, et rendu aux réglages : on n'insère pas une
    /// fois mais dix fois de suite, toujours du même côté.
    core::InsertPlacement m_insertPlacement = core::InsertPlacement::Below;

    /// La racine du manuel installé, ou rien.
    std::filesystem::path m_manualDirectory;

    /// Le répertoire où la boîte « ouvrir » s'ouvrira.
    ///
    /// Celui du dernier fichier **ouvert ou enregistré**, et non celui d'une
    /// boîte annulée : ce qui compte est où l'utilisateur travaille, pas où il
    /// a regardé.
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
