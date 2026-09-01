#include <subedit/core/analysis/frame_rate_deduction.hpp>
#include <subedit/core/analysis/grid_correction.hpp>
#include <subedit/core/edit/convert_frame_rate_command.hpp>
#include <subedit/core/edit/hearing_impaired_removal.hpp>
#include <subedit/core/edit/insert_command.hpp>
#include <subedit/core/edit/remove_command.hpp>
#include <subedit/core/edit/session.hpp>
#include <subedit/core/edit/shift_command.hpp>
#include <subedit/core/edit/shift_limits.hpp>
#include <subedit/core/edit/snap_command.hpp>
#include <subedit/core/edit/transform_command.hpp>
#include <subedit/core/format/diagnostic.hpp>
#include <subedit/core/io/file_system.hpp>
#include <subedit/core/io/find_video.hpp>
#include <subedit/core/model/associated_video.hpp>
#include <subedit/core/model/document.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/model/selection.hpp>
#include <subedit/core/model/source_file.hpp>
#include <subedit/core/model/subtitle_index.hpp>
#include <subedit/core/video/showing.hpp>
#include <subedit/core/video/video_player.hpp>
#include <subedit/core/wording.hpp>
#include <subedit/gui/about_dialog.hpp>
#include <subedit/gui/cell_delegates.hpp>
#include <subedit/gui/command_label.hpp>
#include <subedit/gui/diagnostics_panel.hpp>
#include <subedit/gui/frame_rate_dialog.hpp>
#include <subedit/gui/grid_analysis_dialog.hpp>
#include <subedit/gui/hearing_impaired_dialog.hpp>
#include <subedit/gui/insert_dialog.hpp>
#include <subedit/gui/main_window.hpp>
#include <subedit/gui/manual_window.hpp>
#include <subedit/gui/preferences_dialog.hpp>
#include <subedit/gui/prompts.hpp>
#include <subedit/gui/shift_dialog.hpp>
#include <subedit/gui/snap_dialog.hpp>
#include <subedit/gui/subtitle_table.hpp>
#include <subedit/gui/subtitle_table_model.hpp>
#include <subedit/gui/target.hpp>
#include <subedit/gui/theme.hpp>
#include <subedit/gui/transform_dialog.hpp>

#include <QAbstractItemView>
#include <QAction>
#include <QCloseEvent>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QIcon>
#include <QItemSelection>
#include <QItemSelectionModel>
#include <QKeySequence>
#include <QLabel>
#include <QList>
#include <QMenu>
#include <QMenuBar>
#include <QModelIndex>
#include <QModelIndexList>
#include <QPushButton>
#include <QShowEvent>
#include <QSplitter>
#include <QStatusBar>
#include <QString>
#include <QTableView>
#include <QTimer>
#include <QToolBar>
#include <QVBoxLayout>
#include <QWidget>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <filesystem>
#include <memory>
#include <numeric>
#include <optional>
#include <span>
#include <string>
#include <utility>

namespace subedit::gui {

namespace {

/// What the title bar says: the file name, or that nothing is open.
///
/// **`[*]` is a token and not decoration.** Qt puts the platform's own mark of
/// unsaved changes there — an asterisk here, nothing at all where the platform
/// says so — and warns if `setWindowModified` is called on a title without it.
[[nodiscard]] QString titleFor(const core::Project& project) {
    const core::SourceFile& source = project.sourceFile();
    if (!source.path.has_value())
        return QStringLiteral("subedit[*]");

    return QString::fromStdString(source.path.value().filename().string()) +
           QStringLiteral("[*] — subedit");
}

/// Builds one of the two actions, named for the toolbar and for the menu.
///
/// `text` is what the menu reads and it changes at every operation —
/// « Undo: shifting ». `iconText` is what the toolbar button reads and it never
/// changes: a button whose width followed the last operation would move under
/// the pointer.
[[nodiscard]] QAction*
buildAction(QObject* parent, const QString& shortName, const QString& themeIcon) {
    auto* action = new QAction{QIcon::fromTheme(themeIcon), shortName, parent};
    action->setIconText(shortName);
    action->setEnabled(false);
    return action;
}

/// How often the window asks the player where it is, in milliseconds.
///
/// Ten times a second, which is under what an eye notices on a replica and far
/// under what the reading costs — the answer is a property of an object in
/// this same process. Gaupol polls its own overlay every ten milliseconds; a
/// hundred is the same promise at a tenth of the price.
constexpr int kFollowIntervalMs = 100;

/// How tall the picture may not go under, in pixels.
///
/// A splitter with nothing to stop it lets a child be dragged to nothing, and
/// a video view of zero pixels is indistinguishable from the one that is not
/// there — which is the state the window uses to mean « no film ».
constexpr int kMinimumVideoHeight = 180;

/// What the window opens on, in pixels, the first time.
///
/// Five columns of a table and a picture above them do not fit in what Qt gives
/// a window that never asks: it sizes to the layout's hints, and the table's
/// hint knows nothing of how many rows there are. Twelve hundred by eight
/// hundred shows a dozen subtitles and their whole text without a horizontal
/// scrollbar, on the smallest screen this is likely to meet.
///
/// **A default, not a memory.** Remembering the size a user last chose is
/// phase 7's business, with the rest of the persisted configuration; this is
/// what there is to remember from before anything was.
/// Cent, pour dire « en pour cent ». Nommé parce que l'analyse le demande, et
/// parce qu'un `100` nu au milieu d'une division de pixels se lit mal.
constexpr int kPerCent = 100;

constexpr int kInitialWidth = 1200;
constexpr int kInitialHeight = 800;

/// Which row of a selection playback follows: the first, in table order.
///
/// -1 when nothing is selected. `selectedRows` hands them back in the order
/// they were selected, which is not the order they are read in.
[[nodiscard]] int firstSelectedRow(const QItemSelectionModel& selection) {
    const QModelIndexList rows = selection.selectedRows();
    if (rows.isEmpty())
        return -1;

    return std::ranges::min(rows, {}, [](const QModelIndex& index) { return index.row(); }).row();
}

/// Which row of a selection an insertion is placed against: the last, in table
/// order.
///
/// -1 when nothing is selected. **The last and not the first**, which is the
/// point one invents wrongly without reading it: Gaupol takes
/// `get_selected_rows()[-1]`, and has for twenty years. It is what the hand
/// expects after sweeping downwards.
[[nodiscard]] int lastSelectedRow(const QItemSelectionModel& selection) {
    const QModelIndexList rows = selection.selectedRows();
    if (rows.isEmpty())
        return -1;

    return std::ranges::max(rows, {}, [](const QModelIndex& index) { return index.row(); }).row();
}

/// Les raccourcis de `Save As…`, dont un que la plateforme peut ne pas donner.
///
/// **Le thème de plateforme donne `Ctrl+Maj+S` sur tout bureau** — mesuré sous
/// xcb, sous wayland, et sous `offscreen` dès qu'un thème est posé. Sans thème,
/// Qt n'en donne aucun : sa table interne ne définit `SaveAs` que pour macOS et
/// Windows, et c'est cette table-là qu'un binaire de test rencontre.
///
/// La liaison conventionnelle est donc ajoutée quand la plateforme se tait —
/// issue #274. Ce n'est pas décider à sa place : c'est dire la même chose
/// qu'elle là où elle parle, et ne pas laisser une commande destructive
/// inatteignable au clavier là où elle ne dit rien.
[[nodiscard]] QList<QKeySequence> saveAsShortcuts() {
    static const QKeySequence conventional{QStringLiteral("Ctrl+Shift+S")};

    QList<QKeySequence> given = QKeySequence::keyBindings(QKeySequence::SaveAs);
    if (!given.contains(conventional))
        given.append(conventional);

    return given;
}

} // namespace

MainWindow::MainWindow(core::FileSystem& files,
                       core::OpenedFile opened,
                       Prompts& prompts,
                       PlayerFactory buildPlayer,
                       FrameRateReader readDeclaredRate,
                       QWidget* parent)
    : QMainWindow(parent),
      m_files(&files),
      m_prompts(&prompts),
      m_table(new SubtitleTable{this}),
      m_diagnostics(new DiagnosticsPanel{this}),
      m_undo(buildAction(this, QStringLiteral("Undo"), QStringLiteral("edit-undo"))),
      m_redo(buildAction(this, QStringLiteral("Redo"), QStringLiteral("edit-redo"))),
      m_open(buildAction(this, QStringLiteral("Open…"), QStringLiteral("document-open"))),
      m_save(buildAction(this, QStringLiteral("Save"), QStringLiteral("document-save"))),
      m_saveAs(buildAction(this, QStringLiteral("Save As…"), QStringLiteral("document-save-as"))),
      m_insert(buildAction(this, QStringLiteral("Insert Subtitles…"), QStringLiteral("list-add"))),
      m_remove(
          buildAction(this, QStringLiteral("Remove Subtitles"), QStringLiteral("list-remove"))),
      m_shift(buildAction(this, QStringLiteral("Shift Positions…"), {})),
      m_transform(buildAction(this, QStringLiteral("Transform Positions…"), {})),
      m_frameRate(buildAction(this, QStringLiteral("Convert Frame Rate…"), {})),
      m_hearingImpaired(buildAction(this, QStringLiteral("Remove Hearing-Impaired Mentions…"), {})),
      m_snap(buildAction(this, QStringLiteral("Snap to Frame Rate…"), {})),
      m_shiftOntoGrid(buildAction(this, shiftOntoGridLabel(std::nullopt), {})),
      m_selectVideo(buildAction(this, QStringLiteral("Select Video…"), {})),
      m_playPause(buildAction(
          this, QStringLiteral("Play / Pause"), QStringLiteral("media-playback-start"))),
      m_videoStatus(new QLabel{this}),
      m_gridStatus(new QLabel{this}),
      m_videoView(new QWidget{this}),
      m_noVideo(new QWidget{this}),
      m_split(new QSplitter{Qt::Vertical, this}),
      m_ticker(new QTimer{this}),
      m_buildPlayer(std::move(buildPlayer)),
      m_readDeclaredRate(std::move(readDeclaredRate)) {
    // One delegate per nature of cell, and none on the number or the duration,
    // which are not editable: Qt's table puts one only where it is given one.
    m_table->setItemDelegateForColumn(SubtitleTableModel::Start, new PositionDelegate{this});
    m_table->setItemDelegateForColumn(SubtitleTableModel::End, new PositionDelegate{this});
    m_table->setItemDelegateForColumn(SubtitleTableModel::Text, new TextDelegate{this});
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->verticalHeader()->setVisible(false);

    // The text takes what the positions leave: it is the column that varies,
    // and the four others are known widths.
    m_table->horizontalHeader()->setStretchLastSection(true);

    // **A window of the system, and that is the whole point of these two
    // attributes.** libmpv draws into a window the platform numbers; a plain Qt
    // widget shares its parent's, and there would be nothing of its own to hand
    // over. `WA_DontCreateNativeAncestors` keeps the demand from spreading
    // upwards and turning the table into a native window as well.
    m_videoView->setAttribute(Qt::WA_NativeWindow);
    m_videoView->setAttribute(Qt::WA_DontCreateNativeAncestors);
    m_videoView->setMinimumHeight(kMinimumVideoHeight);
    m_videoView->hide();

    // **An absence a user cannot act on is worse than an empty pane.** Hiding
    // the picture when there is no film left nothing at all where one would go,
    // and the only way in was a menu one had to know about. A single button
    // says both things at once: there is no film, and here is how to choose
    // one.
    //
    // A band rather than a pane: it costs the table a line of height, where the
    // picture costs it a third of the window.
    auto* invite = new QPushButton{QStringLiteral("Select Video…"), m_noVideo};
    connect(invite, &QPushButton::clicked, this, &MainWindow::selectVideo);

    auto* banner = new QHBoxLayout{m_noVideo};
    banner->addStretch();
    banner->addWidget(invite);
    banner->addStretch();

    // The picture on top, the table under it, and the line between them
    // draggable — which is the one thing a fixed layout could not give.
    // Construit dans la liste d'initialisation, comme les autres widgets que la
    // fenêtre garde ; il n'est ajouté à une disposition qu'ici.
    QSplitter* split = m_split;
    split->addWidget(m_videoView);
    split->addWidget(m_noVideo);
    split->addWidget(m_table);

    // A window made taller gives the room to the table, not to the picture.
    // Gaupol reads it the same way, and for the same reason: what one runs out
    // of while editing is rows.
    split->setStretchFactor(0, 0);
    split->setStretchFactor(1, 1);

    // The table takes the room, the panel slips underneath and goes away when
    // it has nothing to say.
    auto* centre = new QWidget{this};
    auto* stack = new QVBoxLayout{centre};
    stack->setContentsMargins(0, 0, 0, 0);
    stack->addWidget(split);
    stack->addWidget(m_diagnostics);
    setCentralWidget(centre);

    // **Toutes les liaisons que la plateforme donne à « rétablir », et non la
    // première** — issue #274.
    //
    // Ce que `QKeySequence` rend dépend du thème de plateforme, et un binaire
    // de test n'en a aucun : sous `offscreen`, Qt retombe sur sa table interne
    // et met `Ctrl+Y` en tête ; sous n'importe quel bureau, le thème donne
    // `Ctrl+Maj+Z` et lui seul. `setShortcut` ne retient que la première, donc
    // la même ligne de code posait deux raccourcis différents selon l'endroit
    // où elle tournait — et le test n'y voyait que celui que l'utilisateur n'a
    // pas. `setShortcuts` les prend toutes : les deux marchent partout.
    m_undo->setShortcut(QKeySequence::Undo);
    m_redo->setShortcuts(QKeySequence::keyBindings(QKeySequence::Redo));
    connect(m_undo, &QAction::triggered, this, [this] { m_model->applied(m_session->undo()); });
    connect(m_redo, &QAction::triggered, this, [this] { m_model->applied(m_session->redo()); });

    m_open->setShortcut(QKeySequence::Open);
    m_save->setShortcut(QKeySequence::Save);
    m_saveAs->setShortcuts(saveAsShortcuts());
    m_open->setEnabled(true);
    m_save->setEnabled(true);
    m_saveAs->setEnabled(true);
    connect(m_open, &QAction::triggered, this, &MainWindow::openFromPrompt);
    // The returned value only serves whoever carries on afterwards; fired by
    // the action, it has nobody to inform.
    connect(m_save, &QAction::triggered, this, [this] { (void)save(); });
    connect(m_saveAs, &QAction::triggered, this, [this] { (void)saveAs(); });

    // **`Ins` et `Suppr`, et non les lettres de Gaupol.** Il donne `I` et
    // `Delete` ; une lettre nue de portée fenêtre serait prise avant que
    // l'éditeur d'une cellule la voie, ce que le `Ctrl+P` du lecteur explique
    // déjà. Les deux touches d'édition, elles, sont réclamées par les champs de
    // saisie de Qt tant qu'un éditeur est ouvert : c'est ce qui laisse `Suppr`
    // effacer un caractère plutôt qu'un sous-titre.
    m_insert->setShortcut(QKeySequence{Qt::Key_Insert});
    m_remove->setShortcut(QKeySequence::Delete);
    connect(m_insert, &QAction::triggered, this, &MainWindow::insertSubtitles);
    connect(m_remove, &QAction::triggered, this, &MainWindow::removeSubtitles);

    connect(m_shift, &QAction::triggered, this, &MainWindow::shiftTarget);
    connect(m_transform, &QAction::triggered, this, &MainWindow::transformTarget);
    connect(m_frameRate, &QAction::triggered, this, &MainWindow::convertFrameRateOfTarget);
    connect(
        m_hearingImpaired, &QAction::triggered, this, &MainWindow::removeHearingImpairedFromTarget);

    connect(m_snap, &QAction::triggered, this, &MainWindow::snapToFrameRate);
    connect(m_shiftOntoGrid, &QAction::triggered, this, &MainWindow::shiftOntoGrid);

    m_analyseGrid = new QAction{QStringLiteral("Frame Rate &Analysis…"), this};
    m_analyseGrid->setEnabled(false);
    connect(m_analyseGrid, &QAction::triggered, this, &MainWindow::analyseGrid);

    m_selectVideo->setEnabled(true);
    connect(m_selectVideo, &QAction::triggered, this, &MainWindow::selectVideo);

    // **`Ctrl+P` where Gaupol has a bare `P`**, and the difference is not
    // taste. A one-letter shortcut of window scope is taken before the widget
    // that has the focus sees it, so a `P` would be swallowed on its way into
    // a cell editor — and this table has three columns one types in. Nothing
    // prints here, so the sequence is free.
    m_playPause->setShortcut(QKeySequence{QStringLiteral("Ctrl+P")});
    connect(m_playPause, &QAction::triggered, this, &MainWindow::togglePlayback);

    m_preferences = new QAction{QStringLiteral("&Preferences…"), this};
    connect(m_preferences, &QAction::triggered, this, &MainWindow::openPreferences);

    m_about = new QAction{QStringLiteral("&About subedit"), this};
    connect(m_about, &QAction::triggered, this, &MainWindow::about);

    // **Éteinte tant que personne ne lui a dit où est le manuel**, ce qui est
    // le cas d'un binaire lancé depuis l'arbre de construction : `main` appelle
    // `setManualPath` avec ce que `installedManualPath()` a résolu, et l'entrée
    // s'allume si le manuel y est. Une entrée qui ouvrirait le vide serait pire
    // qu'une entrée qui dit qu'elle n'a rien à ouvrir.
    m_manual = new QAction{QStringLiteral("&Manual"), this};
    m_manual->setEnabled(false);
    m_manual->setShortcut(QKeySequence::HelpContents);
    connect(m_manual, &QAction::triggered, this, &MainWindow::openManual);

    // **The menu bar, in the order a user reads it**: the document, what one
    // does to it, what accompanies it, what inspects it, what explains it.
    // Reading order and not construction order — the two had drifted apart, and
    // it is the first that a user meets.
    QMenu* file = menuBar()->addMenu(QStringLiteral("&File"));
    file->addAction(m_open);
    file->addSeparator();
    file->addAction(m_save);
    file->addAction(m_saveAs);

    QMenu* edition = menuBar()->addMenu(QStringLiteral("&Edit"));
    edition->addAction(m_undo);
    edition->addAction(m_redo);
    edition->addSeparator();
    // Sous un séparateur : défaire est ce qu'on fait *à* une édition, insérer et
    // supprimer *sont* des éditions.
    edition->addAction(m_insert);
    edition->addAction(m_remove);
    edition->addSeparator();
    // Sous un autre : régler le thème n'est pas une édition du tout.
    edition->addAction(m_preferences);

    QMenu* video = menuBar()->addMenu(QStringLiteral("&Video"));
    video->addAction(m_selectVideo);
    video->addSeparator();
    video->addAction(m_playPause);

    QMenu* tools = menuBar()->addMenu(QStringLiteral("&Tools"));
    tools->addAction(m_shift);
    tools->addAction(m_transform);
    tools->addAction(m_frameRate);
    tools->addSeparator();
    tools->addAction(m_hearingImpaired);
    tools->addSeparator();
    // The two of phase 16, together: one lays each position on the nearest
    // frame, the other moves the whole file back onto its own grid. They read
    // alike and are not alike, which is why they sit side by side rather than
    // among the four above.
    tools->addAction(m_snap);
    tools->addAction(m_shiftOntoGrid);
    tools->addSeparator();
    // Below the separator because it changes nothing: the four above it act on
    // the document, this one only reports on it.
    tools->addAction(m_analyseGrid);

    QMenu* help = menuBar()->addMenu(QStringLiteral("&Help"));
    help->addAction(m_manual);
    help->addSeparator();
    help->addAction(m_about);

    resize(kInitialWidth, kInitialHeight);

    m_ticker->setInterval(kFollowIntervalMs);
    connect(m_ticker, &QTimer::timeout, this, &MainWindow::followPlayback);

    // **Permanent widgets and not `showMessage`.** What film a document
    // accompanies and what grid its positions were written on are standing
    // facts, not passing remarks, and a message can be pushed aside by the next
    // one.
    statusBar()->addPermanentWidget(m_gridStatus);
    statusBar()->addPermanentWidget(m_videoStatus);

    QToolBar* bar = addToolBar(QStringLiteral("Edit"));
    bar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    bar->addAction(m_open);
    bar->addAction(m_save);
    bar->addSeparator();
    bar->addAction(m_undo);
    bar->addAction(m_redo);

    // The boxes sit over this window, and it is the window that says so: built
    // before it, prompts cannot know it, and leaving that to `main` is what let
    // it be forgotten once already.
    m_prompts->ownedBy(this);

    // The document arrives by the same road as those that will follow: one way
    // of putting a file in the window, therefore one place where it can be
    // wrong.
    openOn(std::move(opened.project), opened.diagnostics);
}

void MainWindow::openOn(core::Project project, std::span<const core::Diagnostic> diagnostics) {
    setWindowTitle(titleFor(project));

    // Rebuilt rather than reset: a session carries a history, and the history
    // of one file has nothing to say about the next.
    auto session = std::make_unique<core::Session>(std::move(project));
    auto model = std::make_unique<SubtitleTableModel>(*session);

    m_table->setModel(model.get());
    // The model has carried a cell edit out as a command since issue #129, so
    // the window does not see them go by. This signal is how it learns of one —
    // including an edit that changed nothing. Reconnected at every opening, the
    // previous model leaving with the previous file.
    connect(model.get(), &SubtitleTableModel::historyChanged, this, &MainWindow::refreshActions);

    // In this order: the view lets go of the old model before it goes, and the
    // model before the session it reads.
    m_model = std::move(model);
    m_session = std::move(session);

    // Made again at every opening, with the selection model the table has just
    // been given: `setModel` throws the previous one away, and every connection
    // that named it with it.
    connect(m_table->selectionModel(),
            &QItemSelectionModel::selectionChanged,
            this,
            &MainWindow::placePlaybackAtSelection);
    // Les deux seules actions dont l'état dépend de la sélection, et elles
    // l'écoutent seules : `refreshActions` déduit la grille du fichier entier,
    // et la brancher ici ferait payer cette déduction à chaque ligne d'un
    // cliquer-tirer.
    connect(m_table->selectionModel(),
            &QItemSelectionModel::selectionChanged,
            this,
            &MainWindow::refreshStructureActions);
    m_placedAt = -1;

    m_diagnostics->setDiagnostics(diagnostics);
    proposeVideoBeside();
    refreshActions();
}

void MainWindow::selectVideo() {
    const std::optional<std::filesystem::path>& source = m_session->project().sourceFile().path;
    const std::filesystem::path directory =
        source.has_value() ? source->parent_path() : std::filesystem::path{};

    const std::optional<std::filesystem::path> chosen = m_prompts->videoToOpen(directory);
    if (!chosen.has_value())
        return;

    m_session->chooseVideo(*chosen);
    refreshVideo();
}

void MainWindow::proposeVideoBeside() {
    const std::optional<std::filesystem::path>& source = m_session->project().sourceFile().path;
    if (source.has_value()) {
        if (const std::optional<std::filesystem::path> found =
                core::findVideoBeside(*m_files, *source);
            found.has_value()) {
            // The answer is dropped on purpose: whether the proposal was taken
            // is D5's business, and a caller acting on it would be a second
            // place where that rule lives.
            (void)m_session->proposeVideo(*found);
        }
    }

    refreshVideo();
}

void MainWindow::refreshGridStatus() {
    const core::FrameRateDeduction grid = core::deduceFrameRate(m_session->project());
    const std::optional<core::FrameRate> retained = grid.verdict == core::GridVerdict::Silent
                                                        ? std::nullopt
                                                        : std::optional{grid.retained.rate};

    m_gridStatus->setText(QString::fromStdString(core::gridStatusOf(grid.verdict, retained)));
}

void MainWindow::snapToFrameRate() {
    const core::Selection target = targetOf(*m_table->selectionModel(), m_session->project());
    const std::optional<core::AssociatedVideo>& associated = m_session->project().video();

    SnapDialog dialog{target.count(),
                      m_session->project().frameRate(),
                      associated.has_value() ? associated->declared : std::nullopt,
                      this};
    if (!m_prompts->run(dialog))
        return;

    applyOperation(std::make_unique<core::SnapCommand>(m_session->project(), target, dialog.rate()),
                   target);
}

void MainWindow::shiftOntoGrid() {
    const std::optional<core::Duration> by =
        core::shiftOntoGrid(core::deduceFrameRate(m_session->project()));
    if (!by.has_value())
        return;

    const core::Selection whole = core::Selection::all(m_session->project());

    // The rule the core has held since #132, shared with the command line: a
    // position before the origin is representable, and no subtitle file can
    // hold one.
    if (const std::optional<core::SubtitleIndex> refused =
            core::firstBeforeOrigin(m_session->project(), whole, *by);
        refused.has_value()) {
        m_prompts->reportFailure("subtitle " + std::to_string(refused->number()) +
                                 " would start before the origin, which no subtitle file can hold");
        return;
    }

    applyOperation(std::make_unique<core::ShiftCommand>(whole, *by), whole);
}

void MainWindow::about() {
    AboutDialog dialog{this};
    (void)m_prompts->run(dialog);
}

void MainWindow::setManualPath(std::filesystem::path directory) {
    m_manualDirectory = std::move(directory);

    // La page d'accueil et non le répertoire : un répertoire présent mais vide
    // est une installation partielle, et c'est le cas que le cadrage nomme.
    m_manual->setEnabled(m_files->exists(m_manualDirectory / "index.md"));
    m_manual->setToolTip(m_manual->isEnabled()
                             ? QStringLiteral("Open the installed manual")
                             : QStringLiteral("No manual is installed beside this program"));
}

void MainWindow::openManual() {
    // **Une seule fenêtre, ramenée au premier plan.** Un manuel se consulte
    // plusieurs fois pendant une séance, et chaque appel en ouvrant une
    // nouvelle en laisserait une pile derrière l'autre.
    if (m_manualWindow == nullptr)
        m_manualWindow = new ManualWindow{*m_files, m_manualDirectory, this};

    m_manualWindow->show();
    m_manualWindow->raise();
    m_manualWindow->activateWindow();
}

QStringList MainWindow::menuTitles() const {
    QStringList titles;
    for (const QAction* action : menuBar()->actions())
        titles << action->text();
    return titles;
}

void MainWindow::analyseGrid() {
    GridAnalysisDialog dialog{core::deduceFrameRate(m_session->project()), this};
    (void)m_prompts->run(dialog);
}

void MainWindow::refreshVideoStatus() {
    const std::optional<core::AssociatedVideo>& associated = m_session->project().video();
    const std::optional<std::filesystem::path> path =
        associated.has_value() ? std::optional{associated->path} : std::nullopt;
    const std::optional<core::FrameRate> declared =
        associated.has_value() ? associated->declared : std::nullopt;

    m_videoStatus->setText(QString::fromStdString(core::videoStatusOf(path, declared)));
}

void MainWindow::refreshVideo() {
    refreshVideoStatus();
    watchAssociatedVideo();
}

core::VideoPlayer* MainWindow::player() {
    if (!m_playerAsked && m_buildPlayer) {
        m_playerAsked = true;
        // Asked here and not in the constructor, so that the surface is native
        // before its number is read — and so that a window nobody shows a film
        // to never builds a player at all.
        m_player = m_buildPlayer(static_cast<std::uintptr_t>(m_videoView->winId()));
    }

    return m_player.get();
}

void MainWindow::watchAssociatedVideo() {
    // Nothing is handed to a player before the window has been on screen once.
    // `showEvent` comes back here the moment it has, and until then this leaves
    // `m_associated` alone so that it finds the film still waiting.
    if (!m_wasShown)
        return;

    const std::optional<core::AssociatedVideo>& associated = m_session->project().video();
    const std::filesystem::path wanted =
        associated.has_value() ? associated->path : std::filesystem::path{};

    if (wanted == m_associated)
        return;

    m_associated = wanted;
    m_watching = false;
    m_shown.clear();
    m_placedAt = -1;

    // Asked once per film, here, where the association has just changed for
    // certain: `ffprobe` is a process, and running it at every opening of the
    // frame rate dialog would pay for it again for an answer that cannot have
    // moved. Nothing, without a reader or without a film — which is what a
    // machine with no `ffmpeg` gets, and it is an ordinary state.
    m_session->setDeclaredFrameRate(
        !wanted.empty() && m_readDeclaredRate ? m_readDeclaredRate(wanted) : std::nullopt);

    // **Shown before the film is opened, and not after.** libmpv adopts the
    // window it is handed at that moment; one that is not on screen is adopted
    // and never mapped. Taken away again below if the film will not open, which
    // costs nothing anybody sees: nothing has been painted into it yet.
    m_videoView->setVisible(!wanted.empty());
    m_noVideo->setVisible(wanted.empty());

    core::VideoPlayer* watching = wanted.empty() ? nullptr : player();
    if (watching != nullptr) {
        if (const std::expected<void, core::PlayerError> opened = watching->open(wanted); opened)
            m_watching = true;
        else
            m_prompts->reportFailure(wanted.string() + ": " + opened.error().reason);
    } else if (!wanted.empty() && m_buildPlayer) {
        // A film was named and there is no player to show it with. Said here
        // and not when the program started, because that is where it matters
        // and where it is not a remark about something nobody asked for yet.
        // Why there is none — a session whose windows libmpv cannot adopt, a
        // libmpv that would not start — is one sentence in the manual rather
        // than a taxonomy in a dialog.
        m_prompts->reportFailure(wanted.string() + ": no video player is available");
    }

    // A film that has been left behind must not go on playing under a document
    // that no longer shows it — least of all one nobody can see any more.
    if (!m_watching && m_player) {
        m_player->pause();
        m_player->showSubtitle({});
    }

    // The mark goes with the film: a row left green under a document that no
    // longer shows anything would name a moment nobody is at.
    if (!m_watching && m_model)
        m_model->setShowing(std::nullopt);

    m_videoView->setVisible(m_watching);
    // Exactly one of the two, always: a band that stayed under a playing film
    // would offer to choose the one already chosen.
    m_noVideo->setVisible(!m_watching);
    m_playPause->setEnabled(m_watching);

    if (m_watching) {
        m_ticker->start();
        followPlayback();
    } else {
        m_ticker->stop();
    }
}

void MainWindow::togglePlayback() {
    if (!m_watching)
        return;

    if (m_player->isPlaying())
        m_player->pause();
    else
        m_player->play();
}

void MainWindow::placePlaybackAtSelection() {
    if (!m_watching)
        return;

    const int row = firstSelectedRow(*m_table->selectionModel());
    if (row < 0 || row == m_placedAt)
        return;

    m_placedAt = row;
    const auto index = core::SubtitleIndex::fromValue(static_cast<std::size_t>(row));
    m_player->seek(m_session->project().subtitleAt(index).start);

    // At once rather than at the next tick: what the picture shows and what the
    // table points at have to agree by the time the click is over.
    followPlayback();
}

void MainWindow::followPlayback() {
    if (!m_watching)
        return;

    const core::Project& project = m_session->project();

    // Written as one running answer rather than as a guard and a return, the
    // way `seconds` is in the player: « the player does not know where it is »
    // is an answer of the same rank as a position, and it leads to the same
    // place as a moment between two subtitles — nothing drawn, and the row
    // left where it was.
    const std::optional<core::Timestamp> where = m_player->position();
    const std::optional<core::SubtitleIndex> showing =
        where.has_value() ? core::showingAt(project, *where) : std::nullopt;

    // Read from the project at every tick, which is what makes D2 true rather
    // than merely stated: a text edited a moment ago is on the picture within a
    // tenth of a second, and nothing was written to a disk to put it there.
    const std::string line = showing.has_value()
                                 ? project.subtitleAt(*showing).text(core::Document::Main)
                                 : std::string{};
    if (line != m_shown) {
        m_player->showSubtitle(line);
        m_shown = line;
    }

    m_model->setShowing(showing);

    if (!showing.has_value())
        return;

    // **Whoever is typing wins.** Moving the current cell closes the editor
    // open on it, and a correction half made would go with it.
    if (m_table->isEditing())
        return;

    const QModelIndex current = m_table->currentIndex();
    const int row = static_cast<int>(showing->value());
    if (current.isValid() && current.row() == row)
        return;

    // `NoUpdate` is what keeps the selection out of this. The selection is what
    // an operation applies to, and a film playing in the background has no
    // business rewriting the user's target row by row — it also happens to be
    // what keeps this from firing the seek that watches the selection.
    const QModelIndex followed = m_model->index(row, current.isValid() ? current.column() : 0);
    m_table->selectionModel()->setCurrentIndex(followed, QItemSelectionModel::NoUpdate);
    m_table->scrollTo(followed);
}

bool MainWindow::save() {
    const core::SourceFile& source = m_session->project().sourceFile();
    if (!source.path.has_value())
        return saveAs();

    const std::expected<void, core::FileError> written =
        core::saveProject(*m_files, m_session->project(), *source.path, source.format);
    if (!written) {
        m_prompts->reportFailure(source.path->string() + ": " +
                                 std::string{core::reasonOf(written.error().kind)});
        return false;
    }

    rememberDirectoryOf(*source.path);
    m_session->markSaved(core::Document::Main);
    refreshActions();
    return true;
}

bool MainWindow::saveAs() {
    const std::optional<SaveTarget> target =
        m_prompts->saveTarget(m_session->project().sourceFile());
    if (!target.has_value())
        return false;

    const std::expected<void, core::FileError> written =
        core::saveProject(*m_files, m_session->project(), target->path, target->format);
    if (!written) {
        m_prompts->reportFailure(target->path.string() + ": " +
                                 std::string{core::reasonOf(written.error().kind)});
        return false;
    }

    rememberDirectoryOf(target->path);

    // The document lives there now, and in that format: this is not a command,
    // nobody would want to undo it.
    core::SourceFile moved = m_session->project().sourceFile();
    moved.path = target->path;
    moved.format = target->format;
    m_session->setSourceFile(std::move(moved));

    m_session->markSaved(core::Document::Main);
    setWindowTitle(titleFor(m_session->project()));

    // The file answers to another name now, so the convention has something new
    // to say — and D5 makes it safe to ask: a film the user chose is not
    // replaced by one the convention finds.
    proposeVideoBeside();

    // The format governs the decimal mark the table shows: it has just
    // changed, so everything on screen is to be read again.
    m_model->refreshAll();
    refreshActions();
    return true;
}

bool MainWindow::mayDiscardChanges() {
    if (!m_session->hasUnsavedChanges(core::Document::Main))
        return true;

    switch (m_prompts->aboutUnsavedChanges()) {
    case UnsavedChoice::Save:
        return save();
    case UnsavedChoice::Discard:
        return true;
    case UnsavedChoice::Cancel:
        return false;
    }

    std::unreachable();
}

void MainWindow::openFromPrompt() {
    // Asked before asking what to open: giving up rather than losing one's
    // work should not require choosing a file first.
    if (!mayDiscardChanges())
        return;

    const std::optional<std::filesystem::path> chosen = m_prompts->fileToOpen(m_lastDirectory);
    if (!chosen.has_value())
        return;

    std::expected<core::OpenedFile, core::OpenError> opened = core::openProject(*m_files, *chosen);
    if (!opened) {
        m_prompts->reportFailure(chosen->string() + ": " +
                                 std::string{core::reasonOf(opened.error())});
        return;
    }

    // **Retenu ici et non à la question** : ce qui compte est où l'utilisateur
    // travaille, pas où il a regardé. Une boîte annulée, ou un fichier qui ne
    // s'ouvre pas, ne déplace donc rien.
    rememberDirectoryOf(*chosen);

    openOn(std::move(opened->project), opened->diagnostics);
}

void MainWindow::rememberDirectoryOf(const std::filesystem::path& file) {
    if (file.has_parent_path())
        m_lastDirectory = file.parent_path();
}

void MainWindow::showEvent(QShowEvent* event) {
    QMainWindow::showEvent(event);

    if (m_wasShown)
        return;

    m_wasShown = true;
    watchAssociatedVideo();
}

void MainWindow::closeEvent(QCloseEvent* event) {
    if (mayDiscardChanges())
        event->accept();
    else
        event->ignore();
}

void MainWindow::refreshActions() {
    const QString undo = undoLabel(m_session->nextUndoKind());
    const QString redo = redoLabel(m_session->nextRedoKind());

    m_undo->setEnabled(m_session->canUndo());
    m_undo->setText(undo);
    // Set explicitly: without it, Qt makes the tooltip out of the `iconText`,
    // and the toolbar button would say « Undo » twice instead of naming what it
    // would defeat.
    m_undo->setToolTip(undo);

    m_redo->setEnabled(m_session->canRedo());
    m_redo->setText(redo);
    m_redo->setToolTip(redo);

    setWindowModified(m_session->hasUnsavedChanges(core::Document::Main));

    // Every change of the document may have moved a position, so the verdict is
    // taken again here rather than at the opening alone: an alignment that put
    // the file on another grid must not leave the status bar saying the old one.
    refreshGridStatus();

    // Nothing to shift, nothing to transform: an enabled action would open a
    // dialog that could apply to nothing.
    const bool anything = m_session->project().count() != 0;
    m_shift->setEnabled(anything);
    m_transform->setEnabled(anything);
    m_frameRate->setEnabled(anything);
    // Nothing to analyse either: an empty document has no positions to read a
    // grid off, and the dialog would open on « too few subtitles ».
    m_analyseGrid->setEnabled(anything);
    m_snap->setEnabled(anything);

    // The amount is measured here rather than when the entry is chosen, so that
    // the menu can say what it will do — and the entry goes out when there is
    // no grid to rejoin, which is not the same thing as an amount of zero.
    const std::optional<core::Duration> onto =
        anything ? core::shiftOntoGrid(core::deduceFrameRate(m_session->project())) : std::nullopt;
    m_shiftOntoGrid->setEnabled(onto.has_value());
    m_shiftOntoGrid->setText(shiftOntoGridLabel(onto));
    m_hearingImpaired->setEnabled(anything);

    refreshStructureActions();
}

void MainWindow::refreshStructureActions() {
    const bool anything = m_session->project().count() != 0;
    const bool selected = !m_table->selectionModel()->selectedRows().isEmpty();

    // **Un document vide s'insère sans sélection**, et c'est la seule façon de
    // commencer un fichier neuf. Dès qu'il porte des lignes, il faut dire après
    // laquelle insérer : Gaupol pose la même condition, et c'est celle qui
    // empêche l'index d'être deviné.
    m_insert->setEnabled(!anything || selected);

    // Rien de sélectionné, rien à retirer. L'action éteinte est ce qui tient la
    // règle : sans elle, `Suppr` sur une table sans sélection deviendrait « tout
    // le fichier », qui est ce que `targetOf` répond et qui serait ici un
    // désastre.
    m_remove->setEnabled(selected);
}

void MainWindow::removeHearingImpairedFromTarget() {
    const core::Selection target = targetOf(*m_table->selectionModel(), m_session->project());

    HearingImpairedDialog dialog{target.count(), this};
    if (!m_prompts->run(dialog))
        return;

    // Built before being applied, and asked what it will do: the count is read
    // from the command, never by counting again afterwards.
    std::unique_ptr<core::Command> command =
        core::removeHearingImpaired(m_session->project(), target, core::Document::Main);
    if (!command) {
        // Nothing bit. Say so, and put nothing in the history: an operation
        // that changes nothing is not an operation to undo.
        m_prompts->reportOutcome("no mention to remove");
        return;
    }

    const core::HearingImpairedTally tally = core::tallyOf(*command);
    applyOperation(std::move(command), target);

    m_prompts->reportOutcome(core::countOf(tally.cleaned, "subtitle") + " cleaned, " +
                             std::to_string(tally.removed) + " removed");
}

void MainWindow::applyOperation(std::unique_ptr<core::Command> command,
                                const core::Selection& target) {
    // Read before the command goes: what it is, is what the notice names.
    const core::CommandKind kind = command->kind();

    m_model->applied(m_session->apply(std::move(command)));

    // The row playback was placed at holds something else now — a shift moved
    // it, a removal may have taken it away. Forgetting it is what lets a click
    // on that same row send playback where the subtitle has gone.
    m_placedAt = -1;

    reportWhatPassesTheEnd(kind, target);
}

std::optional<core::Duration> MainWindow::videoLength() const {
    return m_watching ? m_player->duration() : std::nullopt;
}

void MainWindow::reportWhatPassesTheEnd(core::CommandKind kind, const core::Selection& target) {
    // **Only the operations that move a position.** `beyondEnd` reads the state
    // an operation produced; on its own it cannot tell whether that operation
    // put anything there. A subtitle already past the end because the film is
    // the wrong one is nobody's doing, least of all that of a removal of
    // hearing-impaired mentions.
    if (!core::movesPositions(kind))
        return;

    const std::optional<core::BeyondEnd> beyond =
        core::beyondEnd(m_session->project(), target, videoLength());
    if (!beyond.has_value())
        return;

    // A notice and not a failure: nothing was prevented, and the sentence is
    // written to be read after the fact.
    m_prompts->reportOutcome(core::noticeOf(kind, *beyond));
}

void MainWindow::insertSubtitles() {
    const core::Project& project = m_session->project();

    // Le garde de l'action, redit ici : une action éteinte ne se déclenche pas
    // à la souris, mais rien n'empêche un raccourci de la trouver éteinte une
    // fraction de seconde trop tard.
    const int against = lastSelectedRow(*m_table->selectionModel());
    if (project.count() != 0 && against < 0)
        return;

    InsertDialog dialog{project.count() != 0, m_insertPlacement, this};
    if (!m_prompts->run(dialog))
        return;

    // Retenu même si l'insertion qui suit ne change rien : c'est un réglage, et
    // il a été posé. Rendu aux préférences à la fermeture de la fenêtre.
    m_insertPlacement = dialog.placement();

    // Le dernier sélectionné, plus un si l'on insère en dessous. Sur un
    // document vide il n'y a rien à situer : c'est l'index zéro.
    std::size_t at = 0;
    if (against >= 0) {
        at = static_cast<std::size_t>(against);
        if (m_insertPlacement == core::InsertPlacement::Below)
            ++at;
    }

    const std::size_t count = dialog.count();
    const core::SubtitleIndex index = core::SubtitleIndex::fromValue(at);
    const core::Selection inserted =
        core::Selection::range(index, core::SubtitleIndex::fromValue(at + count - 1));

    applyOperation(
        std::make_unique<core::InsertCommand>(core::InsertCommand::blank(project, index, count)),
        inserted);

    // La table a été réinitialisée, donc la sélection a disparu avec elle.
    // Rendre les lignes neuves sélectionnées est ce que Gaupol fait, et ce qui
    // permet d'appuyer sur `Ins` une seconde fois.
    selectRows(static_cast<int>(at), static_cast<int>(at + count - 1));
}

void MainWindow::removeSubtitles() {
    const core::Selection target = selectionOf(*m_table->selectionModel());
    if (target.isEmpty())
        return;

    // Lu avant que l'opération parte : c'est la place que la première ligne
    // retirée laisse, et elle n'a plus de nom une fois le retrait fait.
    const int emptied = static_cast<int>(target.ranges().front().first.value());

    applyOperation(std::make_unique<core::RemoveCommand>(target), target);

    // La ligne qui a pris cette place, ou la dernière quand le retrait a emporté
    // la fin du fichier. Sans elle, un second `Suppr` ne trouverait plus de
    // sélection et l'action serait éteinte.
    const int left = static_cast<int>(m_session->project().count());
    if (left > 0)
        selectRows(std::min(emptied, left - 1), std::min(emptied, left - 1));
}

void MainWindow::selectRows(int first, int last) {
    const QModelIndex from = m_model->index(first, 0);
    const QModelIndex to = m_model->index(last, SubtitleTableModel::kColumnCount - 1);

    // La ligne courante d'abord, et sans toucher à la sélection : passer par
    // `setCurrentIndex` de la vue la réduirait à cette seule ligne, ce qui
    // défairait la sélection posée juste après.
    m_table->selectionModel()->setCurrentIndex(from, QItemSelectionModel::NoUpdate);
    m_table->selectionModel()->select(
        QItemSelection{from, to}, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
    m_table->scrollTo(from);
}

void MainWindow::shiftTarget() {
    const core::Selection target = targetOf(*m_table->selectionModel(), m_session->project());

    ShiftDialog dialog{target.count(), this};
    if (!m_prompts->run(dialog))
        return;

    const std::optional<core::Duration> by = dialog.shift();
    if (!by.has_value())
        return;

    // A position before the origin is representable, but no subtitle file can
    // hold one. The rule has lived in the core since #132, shared with the
    // command line.
    if (const std::optional<core::SubtitleIndex> refused =
            core::firstBeforeOrigin(m_session->project(), target, *by);
        refused.has_value()) {
        m_prompts->reportFailure("subtitle " + std::to_string(refused->number()) +
                                 " would start before the origin, which no subtitle file can "
                                 "hold");
        return;
    }

    applyOperation(std::make_unique<core::ShiftCommand>(target, *by), target);
}

void MainWindow::transformTarget() {
    const core::Selection target = targetOf(*m_table->selectionModel(), m_session->project());

    TransformDialog dialog{target.count(), m_session->project().count(), this};
    if (!m_prompts->run(dialog))
        return;

    const std::optional<TypedReference> first = dialog.first();
    const std::optional<TypedReference> second = dialog.second();
    if (!first.has_value() || !second.has_value())
        return;

    // What the dialog read becomes the core's own value here: it holds
    // widgets, not the vocabulary of a command.
    const auto referenceOf = [](const TypedReference& typed) {
        return core::TransformReference{
            .index = core::SubtitleIndex::fromNumber(static_cast<std::size_t>(typed.number)),
            .target = typed.target,
        };
    };

    std::optional<core::TransformCommand> command = core::TransformCommand::create(
        m_session->project(), target, referenceOf(*first), referenceOf(*second));
    if (!command.has_value()) {
        m_prompts->reportFailure("the two references define no correction");
        return;
    }

    applyOperation(std::make_unique<core::TransformCommand>(std::move(*command)), target);
}

void MainWindow::convertFrameRateOfTarget() {
    const core::Selection target = targetOf(*m_table->selectionModel(), m_session->project());

    // Pre-filled with the project's own, never guessed: the file does not
    // carry it, and getting it wrong shifts everything without a word. What the
    // film declares is handed over beside it, and the dialog decides what to do
    // with it — proposed, never imposed (D6).
    const std::optional<core::AssociatedVideo>& associated = m_session->project().video();
    // **Only a clean grid pre-fills the field.** A partial one is evidence the
    // deduction itself calls partial, and this field decides an operation on
    // the whole file; the status bar and the analysis carry that case instead.
    const core::FrameRateDeduction grid = core::deduceFrameRate(m_session->project());
    const std::optional<core::FrameRate> measured =
        grid.verdict == core::GridVerdict::Clean ? std::optional{grid.retained.rate} : std::nullopt;

    FrameRateDialog dialog{target.count(),
                           m_session->project().frameRate(),
                           associated.has_value() ? associated->declared : std::nullopt,
                           measured,
                           this};
    if (!m_prompts->run(dialog))
        return;

    applyOperation(std::make_unique<core::ConvertFrameRateCommand>(
                       m_session->project(), target, dialog.input(), dialog.output()),
                   target);
}

MainWindow::~MainWindow() = default;

void MainWindow::openPreferences() {
    PreferencesDialog dialog{m_theme, this};
    if (!m_prompts->run(dialog))
        return;

    // Posé tout de suite : une préférence dont l'effet attend le redémarrage
    // laisse croire qu'elle n'a pas été prise.
    m_theme = dialog.theme();
    applyTheme(m_theme);
}

void MainWindow::applySettings(const core::Settings& settings) {
    if (settings.geometry.has_value()) {
        const core::WindowGeometry& where = *settings.geometry;
        setGeometry(where.x, where.y, where.width, where.height);
    }

    if (settings.maximised)
        setWindowState(windowState() | Qt::WindowMaximized);

    // Les quatre premières colonnes seulement : la cinquième prend ce que les
    // autres laissent, et lui poser une largeur ne ferait rien. Le lecteur a
    // déjà refusé un compte différent, donc arriver ici avec autre chose
    // voudrait dire que les deux ne parlent plus des mêmes colonnes.
    if (settings.columnWidths.size() == core::kColumnWidthCount) {
        for (std::size_t column = 0; column < core::kColumnWidthCount; ++column)
            m_table->setColumnWidth(static_cast<int>(column), settings.columnWidths[column]);
    }

    // La poignée : une part et non des hauteurs, donc elle se rejoue à
    // n'importe quelle taille de fenêtre. Les enfants cachés du séparateur
    // valent zéro, si bien que la bande du haut prend tout le reste quel que
    // soit celui des deux qui est montré.
    if (settings.tableShare.has_value() && m_split != nullptr) {
        // **La somme des tailles, et non la hauteur du séparateur** : la
        // poignée elle-même prend des pixels, si bien que les deux ne sont pas
        // égales. Poser sur l'une et relire sur l'autre ferait dériver la part
        // d'un lancement au suivant, de quelques pour cent à chaque fois.
        const QList<int> sizes = m_split->sizes();
        const int total = std::accumulate(sizes.begin(), sizes.end(), 0);
        const int height = total > 0 ? total : kInitialHeight;
        const int table = height * *settings.tableShare / kPerCent;
        m_split->setSizes({0, height - table, table});
    }

    m_lastDirectory = settings.lastDirectory.value_or(std::filesystem::path{});

    m_theme = settings.theme;
    applyTheme(m_theme);

    m_insertPlacement = settings.insertPlacement;
}

core::Settings MainWindow::settings() const {
    core::Settings settings;

    // `normalGeometry()` et non `geometry()` : maximisée, la seconde rend la
    // taille de l'écran, et la session suivante ne saurait plus quoi rendre à
    // qui dé-maximise.
    const QRect where = isMaximized() ? normalGeometry() : geometry();
    settings.geometry = core::WindowGeometry{
        .x = where.x(), .y = where.y(), .width = where.width(), .height = where.height()};

    settings.maximised = isMaximized();

    settings.columnWidths.reserve(core::kColumnWidthCount);
    for (std::size_t column = 0; column < core::kColumnWidthCount; ++column)
        settings.columnWidths.push_back(m_table->columnWidth(static_cast<int>(column)));

    if (m_split != nullptr) {
        const QList<int> sizes = m_split->sizes();
        const int total = std::accumulate(sizes.begin(), sizes.end(), 0);
        if (total > 0 && !sizes.isEmpty())
            settings.tableShare = std::clamp(sizes.back() * kPerCent / total,
                                             core::kSmallestTableShare,
                                             core::kLargestTableShare);
    }

    if (!m_lastDirectory.empty())
        settings.lastDirectory = m_lastDirectory;

    settings.theme = m_theme;
    settings.insertPlacement = m_insertPlacement;

    return settings;
}

} // namespace subedit::gui
