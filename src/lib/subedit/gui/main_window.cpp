#include <subedit/core/edit/convert_frame_rate_command.hpp>
#include <subedit/core/edit/hearing_impaired_removal.hpp>
#include <subedit/core/edit/session.hpp>
#include <subedit/core/edit/shift_command.hpp>
#include <subedit/core/edit/shift_limits.hpp>
#include <subedit/core/edit/transform_command.hpp>
#include <subedit/core/format/diagnostic.hpp>
#include <subedit/core/io/file_system.hpp>
#include <subedit/core/io/find_video.hpp>
#include <subedit/core/model/associated_video.hpp>
#include <subedit/core/model/document.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/model/source_file.hpp>
#include <subedit/core/model/subtitle.hpp>
#include <subedit/core/model/subtitle_index.hpp>
#include <subedit/core/video/showing.hpp>
#include <subedit/core/video/video_player.hpp>
#include <subedit/core/wording.hpp>
#include <subedit/gui/cell_delegates.hpp>
#include <subedit/gui/command_label.hpp>
#include <subedit/gui/diagnostics_panel.hpp>
#include <subedit/gui/frame_rate_dialog.hpp>
#include <subedit/gui/hearing_impaired_dialog.hpp>
#include <subedit/gui/main_window.hpp>
#include <subedit/gui/opening.hpp>
#include <subedit/gui/prompts.hpp>
#include <subedit/gui/saving.hpp>
#include <subedit/gui/shift_dialog.hpp>
#include <subedit/gui/subtitle_table.hpp>
#include <subedit/gui/subtitle_table_model.hpp>
#include <subedit/gui/target.hpp>
#include <subedit/gui/transform_dialog.hpp>

#include <QAbstractItemView>
#include <QAction>
#include <QCloseEvent>
#include <QHeaderView>
#include <QIcon>
#include <QItemSelectionModel>
#include <QKeySequence>
#include <QLabel>
#include <QMenu>
#include <QMenuBar>
#include <QModelIndex>
#include <QModelIndexList>
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

} // namespace

MainWindow::MainWindow(core::FileSystem& files,
                       OpenedFile opened,
                       Prompts& prompts,
                       PlayerFactory buildPlayer,
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
      m_shift(buildAction(this, QStringLiteral("Shift Positions…"), {})),
      m_transform(buildAction(this, QStringLiteral("Transform Positions…"), {})),
      m_frameRate(buildAction(this, QStringLiteral("Convert Frame Rate…"), {})),
      m_hearingImpaired(buildAction(this, QStringLiteral("Remove Hearing-Impaired Mentions…"), {})),
      m_selectVideo(buildAction(this, QStringLiteral("Select Video…"), {})),
      m_playPause(buildAction(
          this, QStringLiteral("Play / Pause"), QStringLiteral("media-playback-start"))),
      m_videoStatus(new QLabel{this}),
      m_videoView(new QWidget{this}),
      m_ticker(new QTimer{this}),
      m_buildPlayer(std::move(buildPlayer)) {
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

    // The picture on top, the table under it, and the line between them
    // draggable — which is the one thing a fixed layout could not give.
    auto* split = new QSplitter{Qt::Vertical, this};
    split->addWidget(m_videoView);
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

    m_undo->setShortcut(QKeySequence::Undo);
    m_redo->setShortcut(QKeySequence::Redo);
    connect(m_undo, &QAction::triggered, this, [this] { m_model->applied(m_session->undo()); });
    connect(m_redo, &QAction::triggered, this, [this] { m_model->applied(m_session->redo()); });

    m_open->setShortcut(QKeySequence::Open);
    m_save->setShortcut(QKeySequence::Save);
    m_saveAs->setShortcut(QKeySequence::SaveAs);
    m_open->setEnabled(true);
    m_save->setEnabled(true);
    m_saveAs->setEnabled(true);
    connect(m_open, &QAction::triggered, this, &MainWindow::openFromPrompt);
    // The returned value only serves whoever carries on afterwards; fired by
    // the action, it has nobody to inform.
    connect(m_save, &QAction::triggered, this, [this] { (void)save(); });
    connect(m_saveAs, &QAction::triggered, this, [this] { (void)saveAs(); });

    QMenu* file = menuBar()->addMenu(QStringLiteral("&File"));
    file->addAction(m_open);
    file->addSeparator();
    file->addAction(m_save);
    file->addAction(m_saveAs);

    connect(m_shift, &QAction::triggered, this, &MainWindow::shiftTarget);
    connect(m_transform, &QAction::triggered, this, &MainWindow::transformTarget);
    connect(m_frameRate, &QAction::triggered, this, &MainWindow::convertFrameRateOfTarget);
    connect(
        m_hearingImpaired, &QAction::triggered, this, &MainWindow::removeHearingImpairedFromTarget);

    QMenu* tools = menuBar()->addMenu(QStringLiteral("&Tools"));
    tools->addAction(m_shift);
    tools->addAction(m_transform);
    tools->addAction(m_frameRate);
    tools->addSeparator();
    tools->addAction(m_hearingImpaired);

    m_selectVideo->setEnabled(true);
    connect(m_selectVideo, &QAction::triggered, this, &MainWindow::selectVideo);

    // **`Ctrl+P` where Gaupol has a bare `P`**, and the difference is not
    // taste. A one-letter shortcut of window scope is taken before the widget
    // that has the focus sees it, so a `P` would be swallowed on its way into
    // a cell editor — and this table has three columns one types in. Nothing
    // prints here, so the sequence is free.
    m_playPause->setShortcut(QKeySequence{QStringLiteral("Ctrl+P")});
    connect(m_playPause, &QAction::triggered, this, &MainWindow::togglePlayback);

    QMenu* video = menuBar()->addMenu(QStringLiteral("&Video"));
    video->addAction(m_selectVideo);
    video->addSeparator();
    video->addAction(m_playPause);

    m_ticker->setInterval(kFollowIntervalMs);
    connect(m_ticker, &QTimer::timeout, this, &MainWindow::followPlayback);

    // A permanent widget and not `showMessage`: what film is associated is a
    // standing fact, not a passing remark, and a message can be pushed aside
    // by the next one.
    statusBar()->addPermanentWidget(m_videoStatus);

    QMenu* edition = menuBar()->addMenu(QStringLiteral("&Edit"));
    edition->addAction(m_undo);
    edition->addAction(m_redo);

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

void MainWindow::refreshVideoStatus() {
    const std::optional<core::AssociatedVideo>& associated = m_session->project().video();
    const std::optional<std::filesystem::path> path =
        associated.has_value() ? std::optional{associated->path} : std::nullopt;

    m_videoStatus->setText(QString::fromStdString(core::videoStatusOf(path)));
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

    // **Shown before the film is opened, and not after.** libmpv adopts the
    // window it is handed at that moment; one that is not on screen is adopted
    // and never mapped. Taken away again below if the film will not open, which
    // costs nothing anybody sees: nothing has been painted into it yet.
    m_videoView->setVisible(!wanted.empty());

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

    m_videoView->setVisible(m_watching);
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
        saveProject(*m_files, m_session->project(), *source.path, source.format);
    if (!written) {
        m_prompts->reportFailure(source.path->string() + ": " +
                                 std::string{core::reasonOf(written.error().kind)});
        return false;
    }

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
        saveProject(*m_files, m_session->project(), target->path, target->format);
    if (!written) {
        m_prompts->reportFailure(target->path.string() + ": " +
                                 std::string{core::reasonOf(written.error().kind)});
        return false;
    }

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

    const std::optional<std::filesystem::path> chosen = m_prompts->fileToOpen();
    if (!chosen.has_value())
        return;

    std::expected<OpenedFile, core::ReadError> opened = openProject(*m_files, *chosen);
    if (!opened) {
        m_prompts->reportFailure(chosen->string() + ": " +
                                 std::string{core::reasonOf(opened.error().kind)});
        return;
    }

    openOn(std::move(opened->project), opened->diagnostics);
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

    // Nothing to shift, nothing to transform: an enabled action would open a
    // dialog that could apply to nothing.
    const bool anything = m_session->project().count() != 0;
    m_shift->setEnabled(anything);
    m_transform->setEnabled(anything);
    m_frameRate->setEnabled(anything);
    m_hearingImpaired->setEnabled(anything);
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
    applyOperation(std::move(command));

    m_prompts->reportOutcome(core::countOf(tally.cleaned, "subtitle") + " cleaned, " +
                             std::to_string(tally.removed) + " removed");
}

void MainWindow::applyOperation(std::unique_ptr<core::Command> command) {
    m_model->applied(m_session->apply(std::move(command)));

    // The row playback was placed at holds something else now — a shift moved
    // it, a removal may have taken it away. Forgetting it is what lets a click
    // on that same row send playback where the subtitle has gone.
    m_placedAt = -1;
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

    applyOperation(std::make_unique<core::ShiftCommand>(target, *by));
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

    applyOperation(std::make_unique<core::TransformCommand>(std::move(*command)));
}

void MainWindow::convertFrameRateOfTarget() {
    const core::Selection target = targetOf(*m_table->selectionModel(), m_session->project());

    // Pre-filled with the project's own, never guessed: the file does not
    // carry it, and getting it wrong shifts everything without a word.
    FrameRateDialog dialog{target.count(), m_session->project().frameRate(), this};
    if (!m_prompts->run(dialog))
        return;

    applyOperation(std::make_unique<core::ConvertFrameRateCommand>(
        m_session->project(), target, dialog.input(), dialog.output()));
}

MainWindow::~MainWindow() = default;

} // namespace subedit::gui
