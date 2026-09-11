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
#include <subedit/core/format/degradation.hpp>
#include <subedit/core/format/diagnostic.hpp>
#include <subedit/core/format/subtitle_writer.hpp>
#include <subedit/core/io/file_system.hpp>
#include <subedit/core/io/find_video.hpp>
#include <subedit/core/model/associated_video.hpp>
#include <subedit/core/model/document.hpp>
#include <subedit/core/model/file_extras.hpp>
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
#include <variant>
#include <vector>

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
/// A hundred, to say "per cent". Named because the analysis asks for it, and
/// because a bare `100` in the middle of a division of pixels reads badly.
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

/// The shortcuts of `Save As…`, one of which the platform may not give.
///
/// **The platform theme gives `Ctrl+Shift+S` on every desktop** — measured
/// under xcb, under wayland, and under `offscreen` as soon as a theme is laid
/// down. With no theme, Qt gives none: its internal table defines `SaveAs` for
/// macOS and Windows alone, and that is the table a test binary meets.
///
/// The conventional binding is therefore added when the platform says nothing —
/// issue #274. This is not deciding in its stead: it is saying the same thing
/// it does where it speaks, and not leaving a destructive command out of reach
/// of the keyboard where it says nothing.
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
      m_encodingStatus(new QLabel{this}),
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

    // **A row is as tall as the subtitle it carries** — issue #322. Three
    // settings, and each one answers a question the other two do not.
    //
    // *No word wrap.* Only a real line break makes a line. A column too narrow
    // elides as it always did, and the height of a row stops depending on the
    // width of a column — which is what keeps it from having to be recomputed
    // at every drag of a column edge, and what keeps the count linear in the
    // length of the text. It is what Gaupol does.
    //
    // *Rows sized to their contents.* Without it the vertical header hands
    // every row the same default height and the delegate is never asked. The
    // header is hidden and still governs: what one sees of it is not what it
    // does.
    //
    // *Scrolling by pixel.* Rows of unequal height under a scrollbar that
    // moves by item make the film jump by a subtitle and the bar sit where no
    // one put it. By pixel, the travel matches what is shown.
    m_table->setWordWrap(false);
    m_table->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    m_table->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);

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
    // Built in the initialiser list, like the other widgets the window keeps;
    // it is added to a layout only here.
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

    // **Every binding the platform gives "redo", and not the first** — issue
    // #274.
    //
    // What `QKeySequence` answers depends on the platform theme, and a test
    // binary has none: under `offscreen`, Qt falls back on its internal table
    // and puts `Ctrl+Y` at the head; under any desktop at all, the theme gives
    // `Ctrl+Shift+Z` and nothing else. `setShortcut` keeps only the first, so
    // one line of code laid down two different shortcuts depending on where it
    // ran — and the test saw only the one the user does not have.
    // `setShortcuts` takes them all: both work everywhere.
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

    // **`Ins` and `Del`, and not Gaupol's letters.** It gives `I` and
    // `Delete`; a bare letter of window scope would be taken before the editor
    // of a cell saw it, which the `Ctrl+P` of the player already explains. The
    // two editing keys, for their part, are claimed by Qt's input fields for as
    // long as an editor is open: that is what lets `Del` erase a character
    // rather than a subtitle.
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

    // **Out for as long as nobody has said where the manual is**, which is the
    // case of a binary run from the build tree: `main` calls `setManualPath`
    // with what `installedManualPath()` resolved, and the entry lights up if
    // the manual is there. An entry that opened emptiness would be worse than
    // an entry saying it has nothing to open.
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
    // Under a separator: undoing is what one does *to* an edit; inserting and
    // removing *are* edits.
    edition->addAction(m_insert);
    edition->addAction(m_remove);
    edition->addSeparator();
    // Under another: setting the theme is no edit at all.
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
    // The encoding first, being the only one of the three that describes the
    // file rather than what is deduced from it or associated with it.
    statusBar()->addPermanentWidget(m_encodingStatus);
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
    // The only two actions whose state depends on the selection, and they
    // listen to it alone: `refreshActions` deduces the grid of the whole file,
    // and wiring it here would pay for that deduction at every row of a drag.
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

void MainWindow::refreshEncodingStatus() {
    // **What the document is, and not what its reading did** — issue #313. The
    // window said the encoding in a diagnostic and nowhere else, so it said it
    // only when the encoding had been guessed: a file that declares its own
    // with a mark showed nothing at all, where `inspect` writes « UTF-16LE,
    // from its byte order mark ». The command line had three answers, the
    // window one and a half.
    //
    // Where the answer came from stays with the diagnostics panel, which exists
    // to say what happened; this line says what is, permanently, as the grid's
    // and the film's do.
    m_encodingStatus->setText(
        QString::fromStdString(core::encodingStatusOf(m_session->project().sourceFile().encoding)));
}

std::optional<core::FrameRate> MainWindow::rateReadInFrames() const {
    const core::FileExtras& extras = m_session->project().sourceFile().extras;
    if (const auto* frames = std::get_if<core::MicroDvdFile>(&extras))
        return frames->rate;
    return std::nullopt;
}

void MainWindow::refreshGridStatus() {
    // **A document counted in frames gets its rate, not a grid.** Deducing one
    // from positions that were computed *from* frames at that very rate would
    // answer with the number it was given — the same choice `inspect` makes.
    if (rateReadInFrames().has_value()) {
        // **The document's rate and not the file's**, so that correcting it
        // through `Convert Frame Rate…` shows: what the line says is what the
        // positions are counted at now, and what writing MicroDVD back will use.
        m_gridStatus->setText(
            QString::fromStdString(core::framesStatusOf(m_session->project().frameRate())));
        return;
    }

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

    // **What the table showed and the two grid surfaces did not** — issue #324.
    // An operation takes the selection; the grid speaks of the document. Align
    // five rows out of a hundred and seventy-six and the timestamps move under
    // the user's eyes while the status bar and the analysis stay put, which
    // reads as a refresh that failed. It is not one: they have nothing to say.
    //
    // Said here rather than in `applyOperation`, which knows a command and a
    // target and not the rate that was asked for — and this is the only
    // operation that asks for one.
    if (const std::optional<core::PartialAlignment> partial =
            core::partialAlignment(m_session->project(), target, dialog.rate()))
        m_prompts->reportOutcome(core::noticeOf(*partial));
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

    // The home page and not the directory: a directory that is there but empty
    // is a partial installation, and that is the case the scoping names.
    m_manual->setEnabled(m_files->exists(m_manualDirectory / "index.md"));
    m_manual->setToolTip(m_manual->isEnabled()
                             ? QStringLiteral("Open the installed manual")
                             : QStringLiteral("No manual is installed beside this program"));
}

void MainWindow::openManual() {
    // **One window, brought back to the front.** A manual is consulted several
    // times in a sitting, and a new one at every call would leave a stack of
    // them behind one another.
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
    // **In this order, and the other way round was issue #323.** It is
    // `watchAssociatedVideo` that reads the rate the film declares — one call
    // to `ffprobe`, once per film — so painting the status bar before it is
    // painting it on a rate that is not there yet. Nothing painted it again,
    // and the line never carried a rate at all: it is written, worded and in
    // the manual, and it never reached the screen.
    watchAssociatedVideo();
    refreshVideoStatus();
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

    const std::expected<void, core::SaveError> written =
        core::saveProject(*m_files, m_session->project(), *source.path, source.format);
    if (!written) {
        m_prompts->reportFailure(source.path->string() + ": " +
                                 std::string{core::reasonOf(written.error())});
        return false;
    }

    rememberDirectoryOf(*source.path);
    m_session->markSaved(core::Document::Main);
    refreshActions();
    return true;
}

bool MainWindow::saveAs() {
    const core::SourceFile& source = m_session->project().sourceFile();

    // **The encoding of the file wins over the setting**, and the setting
    // serves the document with no file: rewriting a document one has just
    // opened in another encoding, because a setting three weeks old says so,
    // would be losing what the reading took care to keep.
    const core::Encoding proposed =
        source.path.has_value() ? source.encoding : m_writeEncoding.value_or(source.encoding);

    const std::optional<SaveTarget> target = m_prompts->saveTarget(source, proposed);
    if (!target.has_value())
        return false;

    // **What the arriving format will not carry, said before the writing.** The
    // command line prints the same words afterwards, where they are a report;
    // asked here they are a warning, and the difference is that the answer can
    // still be « no ». ADR 0031: the tags are translated on the way, so the
    // count of what fell is the count of a conversion that really happened.
    const core::SourceFile before = m_session->project().sourceFile();
    const std::span<const core::Subtitle> held = m_session->project().subtitles();
    // **The document's own rate, and it is a real answer here.** A file counted
    // in frames was read at it, `Convert Frame Rate…` moves it, and nothing
    // else in this window can leave it unset — so the command line's third
    // case, « no rate and no grid, refuse », cannot arise.
    core::ConvertedProject converted = core::convertProjectFor(
        m_session->project(), target->format, m_session->project().frameRate());

    if (const std::string notice = core::noticeOf(converted.loss, before.format, target->format);
        !notice.empty() && !m_prompts->aboutLoss(notice)) {
        return false;
    }

    // What the document becomes, laid down before the writing: `saveProject`
    // writes what the project carries, and what it carries is now what has just
    // been chosen. One act and not two — a format and the texts that speak it —
    // and not a command, for the reasons `Session::becomeFile` writes out.
    const std::vector<core::Subtitle> heldBefore{held.begin(), held.end()};
    core::SourceFile moved = before;
    moved.path = target->path;
    moved.format = target->format;
    moved.encoding = target->encoding;
    moved.newline = target->newline;
    // What the file declares of itself follows the conversion, which is the one
    // place that decides what crosses a format boundary — ADR 0030.
    moved.extras = converted.extras;
    moved.header = converted.header;
    m_session->becomeFile(moved, std::move(converted.subtitles));

    const std::expected<void, core::SaveError> written =
        core::saveProject(*m_files, m_session->project(), target->path, target->format);
    if (!written) {
        // **And undone when the writing fails.** A document that was not
        // written has not moved: without this step back it aims at a file that
        // does not exist, the title still shows the old name — it is only taken
        // up further down — and `Save` writes somewhere other than where anyone
        // thinks. The case has been reachable since phase 8: a `ł` and a
        // Latin-1 encoding are enough, and it does not even ask the disk to
        // refuse.
        m_session->becomeFile(before, heldBefore);
        m_prompts->reportFailure(target->path.string() + ": " +
                                 std::string{core::reasonOf(written.error())});
        return false;
    }

    rememberDirectoryOf(target->path);

    // Kept even if the document already had one: it is a choice that has just
    // been made, and the next document with no file will open on it.
    m_writeEncoding = target->encoding;

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

    // **Kept here and not at the asking**: what counts is where the user
    // works, not where they looked. A box dismissed, or a file that does not
    // open, therefore moves nothing.
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

    // **And the status bar after it**, for `refreshVideo`'s reason: this is
    // where the film the naming convention offered is actually taken, the
    // window not having been shown when `proposeVideoBeside` went by. Without
    // this line, `subedit-gui film.srt` — the ordinary road — never showed the
    // rate.
    watchAssociatedVideo();
    refreshVideoStatus();
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

    // **The encoding rides here although no edit can change it**, and that is
    // deliberate: the two places it does change — an opening, a « save as » that
    // moved the document — both pass through here already, and a third call
    // site is a third one to forget. Issue #323 is what that costs, on the line
    // beside this one. Building a short string is not a deduction.
    refreshEncodingStatus();

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

    // **An empty document takes an insertion with no selection**, and it is
    // the only way to start a new file. As soon as it carries rows, one has to
    // say after which to insert: Gaupol sets the same condition, and it is the
    // one that keeps the index from being guessed.
    m_insert->setEnabled(!anything || selected);

    // Nothing selected, nothing to remove. The action being out is what holds
    // the rule: without it, `Del` on a table with no selection would become
    // "the whole file", which is what `targetOf` answers and would be a
    // disaster here.
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

    // The guard of the action, said again here: an action that is out does not
    // fire under the mouse, but nothing keeps a shortcut from finding it out a
    // fraction of a second too late.
    const int against = lastSelectedRow(*m_table->selectionModel());
    if (project.count() != 0 && against < 0)
        return;

    InsertDialog dialog{project.count() != 0, m_insertPlacement, this};
    if (!m_prompts->run(dialog))
        return;

    // Kept even if the insertion that follows changes nothing: it is a
    // setting, and it has been made. Handed back to the preferences when the
    // window closes.
    m_insertPlacement = dialog.placement();

    // The last selected, plus one when inserting below. On an empty document
    // there is nothing to place it against: the index is zero.
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

    // The table has been reset, so the selection went with it. Leaving the new
    // rows selected is what Gaupol does, and what makes it possible to press
    // `Ins` a second time.
    selectRows(static_cast<int>(at), static_cast<int>(at + count - 1));
}

void MainWindow::removeSubtitles() {
    const core::Selection target = selectionOf(*m_table->selectionModel());
    if (target.isEmpty())
        return;

    // Read before the operation goes: it is the place the first removed row
    // leaves, and it has no name any more once the removal is done.
    const int emptied = static_cast<int>(target.ranges().front().first.value());

    applyOperation(std::make_unique<core::RemoveCommand>(target), target);

    // The row that took that place, or the last one when the removal carried
    // off the end of the file. Without it, a second `Del` would find no
    // selection left and the action would be out.
    const int left = static_cast<int>(m_session->project().count());
    if (left > 0)
        selectRows(std::min(emptied, left - 1), std::min(emptied, left - 1));
}

void MainWindow::selectRows(int first, int last) {
    const QModelIndex from = m_model->index(first, 0);
    const QModelIndex to = m_model->index(last, SubtitleTableModel::kColumnCount - 1);

    // The current row first, and without touching the selection: going through
    // the view's `setCurrentIndex` would shrink it to that one row, which would
    // undo the selection laid down right after.
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
    //
    // **And a document counted in frames leaves it out entirely.** Its
    // positions come from its frames at the rate it was read at, so the
    // deduction can only find that rate again; what is offered instead is the
    // rate itself, said for what it is.
    const std::optional<core::FrameRate> read = rateReadInFrames();
    const core::FrameRateDeduction grid = core::deduceFrameRate(m_session->project());
    const std::optional<core::FrameRate> measured =
        !read.has_value() && grid.verdict == core::GridVerdict::Clean
            ? std::optional{grid.retained.rate}
            : std::nullopt;

    FrameRateDialog dialog{target.count(),
                           m_session->project().frameRate(),
                           associated.has_value() ? associated->declared : std::nullopt,
                           measured,
                           read,
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

    // Laid down at once: a preference whose effect waits for a restart looks
    // like a preference that was not taken.
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

    // The first four columns only: the fifth takes what the others leave, and
    // giving it a width would do nothing. The reader has already refused a
    // different count, so arriving here with anything else would mean the two
    // no longer speak of the same columns.
    if (settings.columnWidths.size() == core::kColumnWidthCount) {
        for (std::size_t column = 0; column < core::kColumnWidthCount; ++column)
            m_table->setColumnWidth(static_cast<int>(column), settings.columnWidths[column]);
    }

    // The handle: a share and not heights, so it replays at any size of
    // window. The hidden children of the splitter count zero, so the band at
    // the top takes all the rest whichever of the two is shown.
    if (settings.tableShare.has_value() && m_split != nullptr) {
        // **The sum of the sizes, and not the height of the splitter**: the
        // handle itself takes pixels, so the two are not equal. Setting against
        // one and reading back against the other would drift the share from one
        // launch to the next, by a few per cent every time.
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
    m_writeEncoding = settings.writeEncoding;
}

core::Settings MainWindow::settings() const {
    core::Settings settings;

    // `normalGeometry()` and not `geometry()`: maximised, the second answers
    // the size of the screen, and the next session would no longer know what to
    // give back to whoever unmaximises.
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
    settings.writeEncoding = m_writeEncoding;

    return settings;
}

} // namespace subedit::gui
