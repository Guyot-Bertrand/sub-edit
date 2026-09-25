#include <subedit/core/analysis/frame_rate_deduction.hpp>
#include <subedit/core/analysis/grid_correction.hpp>
#include <subedit/core/config/duration_adjustment_settings.hpp>
#include <subedit/core/edit/append.hpp>
#include <subedit/core/edit/clipboard.hpp>
#include <subedit/core/edit/convert_frame_rate_command.hpp>
#include <subedit/core/edit/dialogue_dashes_command.hpp>
#include <subedit/core/edit/duration_adjustment.hpp>
#include <subedit/core/edit/hearing_impaired_removal.hpp>
#include <subedit/core/edit/insert_command.hpp>
#include <subedit/core/edit/italics_command.hpp>
#include <subedit/core/edit/letter_case_command.hpp>
#include <subedit/core/edit/merge_split_command.hpp>
#include <subedit/core/edit/remove_command.hpp>
#include <subedit/core/edit/rewrite_texts.hpp>
#include <subedit/core/edit/search.hpp>
#include <subedit/core/edit/session.hpp>
#include <subedit/core/edit/shift_command.hpp>
#include <subedit/core/edit/shift_limits.hpp>
#include <subedit/core/edit/snap_command.hpp>
#include <subedit/core/edit/split_project.hpp>
#include <subedit/core/edit/transform_command.hpp>
#include <subedit/core/edit/translation.hpp>
#include <subedit/core/format/degradation.hpp>
#include <subedit/core/format/diagnostic.hpp>
#include <subedit/core/format/subtitle_writer.hpp>
#include <subedit/core/format/translation_file.hpp>
#include <subedit/core/io/file_system.hpp>
#include <subedit/core/io/find_video.hpp>
#include <subedit/core/model/associated_video.hpp>
#include <subedit/core/model/document.hpp>
#include <subedit/core/model/file_extras.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/model/selection.hpp>
#include <subedit/core/model/source_file.hpp>
#include <subedit/core/model/subtitle_index.hpp>
#include <subedit/core/text/markup_vocabulary.hpp>
#include <subedit/core/video/showing.hpp>
#include <subedit/core/video/video_player.hpp>
#include <subedit/core/wording.hpp>
#include <subedit/gui/about_dialog.hpp>
#include <subedit/gui/cell_delegates.hpp>
#include <subedit/gui/command_label.hpp>
#include <subedit/gui/diagnostics_panel.hpp>
#include <subedit/gui/duration_adjust_dialog.hpp>
#include <subedit/gui/frame_rate_dialog.hpp>
#include <subedit/gui/grid_analysis_dialog.hpp>
#include <subedit/gui/hearing_impaired_dialog.hpp>
#include <subedit/gui/insert_dialog.hpp>
#include <subedit/gui/main_window.hpp>
#include <subedit/gui/manual_window.hpp>
#include <subedit/gui/preferences_dialog.hpp>
#include <subedit/gui/project_files.hpp>
#include <subedit/gui/project_page.hpp>
#include <subedit/gui/project_search.hpp>
#include <subedit/gui/prompts.hpp>
#include <subedit/gui/search_dialog.hpp>
#include <subedit/gui/shift_dialog.hpp>
#include <subedit/gui/snap_dialog.hpp>
#include <subedit/gui/split_project_dialog.hpp>
#include <subedit/gui/subtitle_table.hpp>
#include <subedit/gui/subtitle_table_model.hpp>
#include <subedit/gui/table_columns.hpp>
#include <subedit/gui/target.hpp>
#include <subedit/gui/theme.hpp>
#include <subedit/gui/transform_dialog.hpp>

#include <QAbstractItemModel>
#include <QAbstractItemView>
#include <QAction>
#include <QClipboard>
#include <QCloseEvent>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QGuiApplication>
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
#include <QMimeData>
#include <QModelIndex>
#include <QModelIndexList>
#include <QPushButton>
#include <QShowEvent>
#include <QSplitter>
#include <QStatusBar>
#include <QString>
#include <QTabBar>
#include <QTableView>
#include <QTimer>
#include <QToolBar>
#include <QUrl>
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

/// What a tab says: the file name, or that there is none yet — the title
/// without the `[*]` Qt reads on a window, and without repeating "subedit" on
/// every one of them.
[[nodiscard]] QString tabLabelFor(const core::Project& project, bool modified) {
    const std::optional<std::filesystem::path>& path = project.sourceFile().path;
    const QString name = path.has_value() ? QString::fromStdString(path->filename().string())
                                          : QStringLiteral("untitled");
    // An asterisk by hand: the `[*]` Qt reads is for a window's title alone.
    return modified ? name + QStringLiteral("*") : name;
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

/// How wide the text column is while the translation column shares the table
/// with it, in pixels.
///
/// Half of what a window opened at its default size leaves after the four
/// columns that are known widths. Not persisted with the others: a text that
/// takes the room left has no width worth keeping, and the settings file has
/// never held one.
constexpr int kDefaultTextWidth = 400;

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

/// Long enough to be read without a click, short enough not to survive past
/// the next gesture — Qt's own convention for a transient status.
constexpr int kOperationStatusTimeoutMs = 5000;

/// Two sentences for one box, one to a line, and whichever is empty left out.
///
/// What was done comes first and what it left past the end of the film after
/// it: the second is a warning about the first, and is read as one.
[[nodiscard]] std::string joinedNotices(const std::string& done, const std::string& warning) {
    if (done.empty())
        return warning;
    return warning.empty() ? done : done + "\n" + warning;
}

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

/// What the search asks of the window — ADR 0034. A class of its own rather
/// than the window itself: `View::show(int)` would hide `QWidget::show()`.
class MainWindow::SearchSide final : public ProjectSearch::View {

public:
    explicit SearchSide(MainWindow& window) : m_window(&window) {}

    [[nodiscard]] int projectCount() const override {
        return static_cast<int>(m_window->m_pages.size());
    }

    [[nodiscard]] ProjectPage& project(int index) override {
        return *m_window->m_pages.at(static_cast<std::size_t>(index));
    }

    [[nodiscard]] int shownProject() const override { return m_window->m_currentPage; }

    void show(int index) override { m_window->switchToPage(index); }

    [[nodiscard]] core::Document targetDocument() const override {
        return m_window->targetDocument();
    }

    [[nodiscard]] bool twoTexts() const override { return m_window->m_columns->translationShown(); }

    [[nodiscard]] core::Selection selectionTarget() const override {
        return targetOf(*m_window->m_table->selectionModel(), m_window->m_page->session->project());
    }

    void moveTo(int row) override {
        m_window->m_page->movingToMatch = true;
        m_window->selectRows(row, row);
        m_window->m_page->movingToMatch = false;
    }

    void apply(ProjectPage& page,
               std::unique_ptr<core::Command> command,
               const core::Selection& target) override {
        m_window->applyOperation(page, std::move(command), target);
    }

private:
    MainWindow* m_window;
};

/// What the files ask of the window — ADR 0034.
class MainWindow::FilesSide final : public ProjectFiles::View {

public:
    explicit FilesSide(MainWindow& window) : m_window(&window) {}

    [[nodiscard]] int projectCount() const override {
        return static_cast<int>(m_window->m_pages.size());
    }

    [[nodiscard]] ProjectPage& project(int index) override {
        return *m_window->m_pages.at(static_cast<std::size_t>(index));
    }

    [[nodiscard]] int shownProject() const override { return m_window->m_currentPage; }

    void show(int index) override { m_window->switchToPage(index); }

    void saved(ProjectPage& page, core::Document document, bool moved) override {
        // The file answers to another name now: the title says it, and the
        // convention has something new to say — D5 makes it safe to ask, a film
        // the user chose is not replaced by one the convention finds. The film
        // goes with the main document; the translation's name says nothing of it.
        if (moved && document == core::Document::Main && &page == m_window->m_page) {
            m_window->setWindowTitle(titleFor(page.session->project()));
            m_window->proposeVideoBeside();
        }
        // A page saved behind its tab — issue #461 — has only its tab to say
        // it: the actions are the shown page's, and are refreshed with it.
        if (&page == m_window->m_page)
            m_window->refreshActions();
        else
            m_window->refreshTabOf(page);
    }

    void announce(const std::string& message) override {
        m_window->statusBar()->showMessage(QString::fromStdString(message),
                                           kOperationStatusTimeoutMs);
    }

    [[nodiscard]] QWidget* dialogParent() override { return m_window; }

private:
    MainWindow* m_window;
};

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
      m_newProject(buildAction(this, QStringLiteral("&New"), QStringLiteral("document-new"))),
      m_closeProject(buildAction(this, QStringLiteral("&Close"), QStringLiteral("window-close"))),
      m_saveAllDocuments(buildAction(this, QStringLiteral("&Save All"), {})),
      m_closeAllProjects(buildAction(this, QStringLiteral("&Close All"), {})),
      m_nextTab(new QAction{this}),
      m_previousTab(new QAction{this}),
      m_save(buildAction(this, QStringLiteral("Save"), QStringLiteral("document-save"))),
      m_saveAs(buildAction(this, QStringLiteral("Save As…"), QStringLiteral("document-save-as"))),
      m_openTranslation(buildAction(this, QStringLiteral("Open &Translation…"), {})),
      m_saveTranslation(buildAction(this, QStringLiteral("Save Tr&anslation"), {})),
      m_saveTranslationAs(buildAction(this, QStringLiteral("Save Translation As…"), {})),
      m_cut(buildAction(this, QStringLiteral("Cu&t Texts"), QStringLiteral("edit-cut"))),
      m_copy(buildAction(this, QStringLiteral("&Copy Texts"), QStringLiteral("edit-copy"))),
      m_paste(buildAction(this, QStringLiteral("&Paste Texts"), QStringLiteral("edit-paste"))),
      m_findAndReplace(buildAction(
          this, QStringLiteral("&Find and Replace…"), QStringLiteral("edit-find-replace"))),
      m_insert(buildAction(this, QStringLiteral("Insert Subtitles…"), QStringLiteral("list-add"))),
      m_remove(
          buildAction(this, QStringLiteral("Remove Subtitles"), QStringLiteral("list-remove"))),
      m_mergeSubtitles(buildAction(this, QStringLiteral("&Merge Subtitles"), {})),
      m_splitSubtitle(buildAction(this, QStringLiteral("S&plit Subtitle"), {})),
      m_shift(buildAction(this, QStringLiteral("Shift Positions…"), {})),
      m_transform(buildAction(this, QStringLiteral("Transform Positions…"), {})),
      m_frameRate(buildAction(this, QStringLiteral("Convert Frame Rate…"), {})),
      m_adjustDurations(buildAction(this, QStringLiteral("Adjust Durations…"), {})),
      m_appendFile(buildAction(this, QStringLiteral("Append &File…"), {})),
      m_splitProject(buildAction(this, QStringLiteral("Spli&t Project…"), {})),
      m_hearingImpaired(buildAction(this, QStringLiteral("Remove Hearing-Impaired Mentions…"), {})),
      m_italic(buildAction(this, QStringLiteral("&Italic"), QStringLiteral("format-text-italic"))),
      m_dialogueDashes(buildAction(this, QStringLiteral("&Dialogue"), {})),
      m_snap(buildAction(this, QStringLiteral("Snap to Frame Rate…"), {})),
      m_shiftOntoGrid(buildAction(this, shiftOntoGridLabel(std::nullopt), {})),
      m_selectVideo(buildAction(this, QStringLiteral("Select Video…"), {})),
      m_playPause(buildAction(
          this, QStringLiteral("Play / Pause"), QStringLiteral("media-playback-start"))),
      m_videoStatus(new QLabel{this}),
      m_gridStatus(new QLabel{this}),
      m_encodingStatus(new QLabel{this}),
      m_targetStatus(new QLabel{this}),
      m_videoView(new QWidget{this}),
      m_noVideo(new QWidget{this}),
      m_split(new QSplitter{Qt::Vertical, this}),
      m_tabBar(new QTabBar{this}),
      m_ticker(new QTimer{this}),
      m_buildPlayer(std::move(buildPlayer)),
      m_readDeclaredRate(std::move(readDeclaredRate)) {
    // One delegate per nature of cell, and none on the number, which is not
    // editable: Qt's table puts one only where it is given one.
    m_table->setItemDelegateForColumn(SubtitleTableModel::Start, new PositionDelegate{this});
    m_table->setItemDelegateForColumn(SubtitleTableModel::End, new PositionDelegate{this});
    m_table->setItemDelegateForColumn(SubtitleTableModel::Duration, new DurationDelegate{this});
    m_table->setItemDelegateForColumn(SubtitleTableModel::Text, new TextDelegate{this});
    m_table->setItemDelegateForColumn(SubtitleTableModel::Translation, new TextDelegate{this});
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

    // The last column that is shown takes what the positions leave: it is the
    // one that varies, and the four first are known widths. That is the text
    // while there is no translation, and the translation once there is one —
    // which is why the text is given a width of its own, that it keeps as long
    // as the translation is there and that stretching ignores as long as it is
    // not.
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

    // **One tab per open project** — ADR 0033, `GUI-TABS-01`. Above the
    // picture and the table, which is what it names: it says which project
    // the room below belongs to.
    m_tabBar->setExpanding(false);
    m_tabBar->setDocumentMode(true);
    connect(m_tabBar, &QTabBar::currentChanged, this, &MainWindow::switchToPage);

    // The table takes the room, the panel slips underneath and goes away when
    // it has nothing to say.
    auto* centre = new QWidget{this};
    auto* stack = new QVBoxLayout{centre};
    stack->setContentsMargins(0, 0, 0, 0);
    stack->addWidget(m_tabBar);
    stack->addWidget(split);
    stack->addWidget(m_diagnostics);
    setCentralWidget(centre);

    // **The whole window takes a dropped file** — issue #453. The table, the
    // picture, the tabs and the bars all pass a drop they do not take on to
    // it, so there is one place that sorts what arrives.
    setAcceptDrops(true);

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
    connect(m_undo, &QAction::triggered, this, [this] {
        commitCellEditor();
        m_page->model->applied(m_page->session->undo());
    });
    connect(m_redo, &QAction::triggered, this, [this] {
        commitCellEditor();
        m_page->model->applied(m_page->session->redo());
    });

    m_open->setShortcut(QKeySequence::Open);
    m_newProject->setShortcut(QKeySequence::New);
    m_closeProject->setShortcut(QKeySequence::Close);
    m_save->setShortcut(QKeySequence::Save);
    m_saveAs->setShortcuts(saveAsShortcuts());
    m_open->setEnabled(true);
    m_newProject->setEnabled(true);
    m_save->setEnabled(true);
    m_saveAs->setEnabled(true);
    connect(m_open, &QAction::triggered, this, &MainWindow::openFromPrompt);
    connect(m_newProject, &QAction::triggered, this, &MainWindow::newProject);
    connect(m_closeProject, &QAction::triggered, this, &MainWindow::closeCurrentProject);
    m_saveAllDocuments->setEnabled(true);
    m_closeAllProjects->setEnabled(true);
    m_saveAllDocuments->setShortcut(QKeySequence{Qt::CTRL | Qt::SHIFT | Qt::Key_L});
    m_closeAllProjects->setShortcut(QKeySequence{Qt::CTRL | Qt::SHIFT | Qt::Key_W});
    connect(m_saveAllDocuments, &QAction::triggered, this, [this] { m_projectFiles->saveAll(); });
    // **Closing every project is closing the window**: the window always holds
    // one, so there is no state in between. `closeEvent` asks the one question.
    connect(m_closeAllProjects, &QAction::triggered, this, &QWidget::close);
    // **`Ctrl+PageDown` and `Ctrl+PageUp`**, the platform's own for moving
    // between tabs — no `QKeySequence::StandardKey` names them, so they are
    // written out, as Gaupol's own binding is. `addAction` and not a menu:
    // the bar already offers a click, and this is for whoever would rather
    // not reach for the mouse.
    m_nextTab->setShortcut(QKeySequence{QStringLiteral("Ctrl+PgDown")});
    connect(m_nextTab, &QAction::triggered, this, [this] {
        switchToPage((m_currentPage + 1) % static_cast<int>(m_pages.size()));
    });
    addAction(m_nextTab);
    m_previousTab->setShortcut(QKeySequence{QStringLiteral("Ctrl+PgUp")});
    connect(m_previousTab, &QAction::triggered, this, [this] {
        const int count = static_cast<int>(m_pages.size());
        switchToPage((m_currentPage - 1 + count) % count);
    });
    addAction(m_previousTab);
    // The returned value only serves whoever carries on afterwards; fired by
    // the action, it has nobody to inform.
    connect(m_save, &QAction::triggered, this, [this] {
        (void)m_projectFiles->save(*m_page, core::Document::Main);
    });
    connect(m_saveAs, &QAction::triggered, this, [this] {
        (void)m_projectFiles->saveAs(*m_page, core::Document::Main);
    });
    connect(m_openTranslation, &QAction::triggered, this, &MainWindow::openTranslationFromPrompt);
    connect(m_saveTranslation, &QAction::triggered, this, [this] {
        (void)m_projectFiles->save(*m_page, core::Document::Translation);
    });
    connect(m_saveTranslationAs, &QAction::triggered, this, [this] {
        (void)m_projectFiles->saveAs(*m_page, core::Document::Translation);
    });

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

    // **No shortcut, where Gaupol has `M` and `S`.** A bare letter of window
    // scope would be taken before a cell editor saw it, and the `Ctrl` forms
    // are spoken for: `Ctrl+S` saves. Two entries one reaches by the menu are
    // better than a key that types a letter into the wrong place.
    connect(m_mergeSubtitles, &QAction::triggered, this, &MainWindow::mergeSubtitles);

    // **The platform's three, as Gaupol has them.** No conflict with a cell
    // editor: a text field claims these sequences for as long as it has the
    // focus, the way it claims `Del`, so inside an open cell they copy and
    // paste characters rather than subtitles.
    m_cut->setShortcut(QKeySequence::Cut);
    m_copy->setShortcut(QKeySequence::Copy);
    m_paste->setShortcut(QKeySequence::Paste);
    connect(m_cut, &QAction::triggered, this, &MainWindow::cutTexts);
    connect(m_copy, &QAction::triggered, this, &MainWindow::copyTexts);
    connect(m_paste, &QAction::triggered, this, &MainWindow::pasteTexts);

    // `Ctrl+F`, Gaupol's. A cell editor does not claim it, so it opens the
    // dialog from inside an open cell too.
    m_findAndReplace->setShortcut(QKeySequence::Find);
    connect(m_findAndReplace, &QAction::triggered, this, &MainWindow::openSearch);
    connect(m_splitSubtitle, &QAction::triggered, this, &MainWindow::splitSubtitle);

    connect(m_shift, &QAction::triggered, this, &MainWindow::shiftTarget);
    connect(m_transform, &QAction::triggered, this, &MainWindow::transformTarget);
    connect(m_frameRate, &QAction::triggered, this, &MainWindow::convertFrameRateOfTarget);
    connect(m_adjustDurations, &QAction::triggered, this, &MainWindow::adjustDurationsOfTarget);
    connect(m_appendFile, &QAction::triggered, this, &MainWindow::appendFileFromPrompt);
    connect(m_splitProject, &QAction::triggered, this, &MainWindow::splitProjectFromPrompt);
    connect(
        m_hearingImpaired, &QAction::triggered, this, &MainWindow::removeHearingImpairedFromTarget);

    // **`Ctrl+I` and not a bare `I`**, for the reason the player's `Ctrl+P`
    // already carries: a one-letter shortcut of window scope is taken before
    // the cell editor sees it, and this table has three columns one types in.
    m_italic->setShortcut(QKeySequence{QStringLiteral("Ctrl+I")});
    connect(m_italic, &QAction::triggered, this, &MainWindow::toggleItalicsOnTarget);

    // **Gaupol's four, in Gaupol's order** — `Text ▸ Case` offers Title,
    // Sentence, Upper, Lower, and a user who knows one knows the other.
    static constexpr std::array<const char*, 4> kCaseLabels = {
        "&Title Case", "&Sentence case", "&UPPER CASE", "&lower case"};
    for (std::size_t which = 0; which < m_case.size(); ++which) {
        const core::LetterCase wanted = core::kLetterCases[which];
        m_case[which] = buildAction(this, QString::fromUtf8(kCaseLabels[which]), {});
        connect(m_case[which], &QAction::triggered, this, [this, wanted] {
            changeCaseOfTarget(wanted);
        });
    }

    connect(m_dialogueDashes, &QAction::triggered, this, &MainWindow::toggleDialogueDashesOnTarget);

    connect(m_snap, &QAction::triggered, this, &MainWindow::snapToFrameRate);
    connect(m_shiftOntoGrid, &QAction::triggered, this, &MainWindow::shiftOntoGrid);

    m_analyseGrid = new QAction{QStringLiteral("Frame Rate &Analysis…"), this};
    m_analyseGrid->setEnabled(false);
    connect(m_analyseGrid, &QAction::triggered, this, &MainWindow::analyseGrid);

    // The columns and their entries are the collaborator's — ADR 0034. The
    // column first, then the target: what a cell can be aiming at depends on
    // whether the column is there.
    m_columns = std::make_unique<TableColumns>(*m_table, this);
    m_searchSide = std::make_unique<SearchSide>(*this);
    m_search = std::make_unique<ProjectSearch>(*m_searchSide, this);
    m_filesSide = std::make_unique<FilesSide>(*this);
    m_projectFiles = std::make_unique<ProjectFiles>(*m_files, *m_prompts, *m_filesSide);
    for (QAction* entry : m_columns->entries()) {
        connect(entry, &QAction::toggled, this, [this] {
            refreshColumns();
            refreshTarget();
        });
    }

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
    file->addAction(m_newProject);
    file->addAction(m_open);
    file->addAction(m_openTranslation);
    file->addSeparator();
    file->addAction(m_save);
    file->addAction(m_saveAs);
    // The translation is a file of its own, and so are the entries that write
    // it: under the two of the main document, where a user looking for how to
    // save will look first.
    file->addSeparator();
    file->addAction(m_saveTranslation);
    file->addAction(m_saveTranslationAs);
    // Below everything the document itself offers: closing is what one does
    // to the tab, not to what it holds.
    file->addSeparator();
    file->addAction(m_closeProject);

    QMenu* edition = menuBar()->addMenu(QStringLiteral("&Edit"));
    edition->addAction(m_undo);
    edition->addAction(m_redo);
    edition->addSeparator();
    // Where every program puts them, and above the edits of structure: they
    // move texts, and never add or take away a row — save a paste that runs
    // past the end.
    edition->addAction(m_cut);
    edition->addAction(m_copy);
    edition->addAction(m_paste);
    edition->addSeparator();
    edition->addAction(m_findAndReplace);
    edition->addSeparator();
    // Under a separator: undoing is what one does *to* an edit; inserting and
    // removing *are* edits.
    edition->addAction(m_insert);
    edition->addAction(m_remove);
    // Beside them: merging and splitting change how many rows there are, as
    // inserting and removing do, and Gaupol keeps the four together.
    edition->addAction(m_mergeSubtitles);
    edition->addAction(m_splitSubtitle);
    edition->addSeparator();
    // Under another: setting the theme is no edit at all.
    edition->addAction(m_preferences);

    // Born with the translation column, and where the other columns would sit
    // one day — issue #442. After `Edit` and before `Video`: what one does to
    // the document, then how one looks at it, then what accompanies it.
    QMenu* view = menuBar()->addMenu(QStringLiteral("&View"));
    // A submenu, as Gaupol's `View ▸ Columns`: five entries loose in the menu
    // would be five entries for one question.
    QMenu* columns = view->addMenu(QStringLiteral("&Columns"));
    for (QAction* entry : m_columns->entries())
        columns->addAction(entry);

    QMenu* video = menuBar()->addMenu(QStringLiteral("&Video"));
    video->addAction(m_selectVideo);
    video->addSeparator();
    video->addAction(m_playPause);

    QMenu* tools = menuBar()->addMenu(QStringLiteral("&Tools"));
    tools->addAction(m_shift);
    tools->addAction(m_transform);
    tools->addAction(m_frameRate);
    // With the operations on positions: it moves ends, and nothing else.
    tools->addAction(m_adjustDurations);
    tools->addSeparator();
    // On its own: it adds subtitles rather than editing the ones already
    // there — Gaupol's own placement, next to the position operations and
    // apart from the two that follow.
    tools->addAction(m_appendFile);
    // Its inverse, beside it.
    tools->addAction(m_splitProject);
    tools->addSeparator();
    // Those that rewrite a text rather than move a position, together.
    tools->addAction(m_italic);
    tools->addAction(m_dialogueDashes);
    // A submenu for the four, which is what Gaupol does: four entries side by
    // side in a menu of nine would drown the rest.
    QMenu* letterCase = tools->addMenu(QStringLiteral("Ca&se"));
    for (QAction* one : m_case)
        letterCase->addAction(one);
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

    // What acts on every project at once. Gaupol's menu of the same name also
    // lists the tabs and has `Save All As…`; the first is left to the tab bar
    // and the second is a series of `Save As…` that `Save All` already asks.
    QMenu* projects = menuBar()->addMenu(QStringLiteral("&Projects"));
    projects->addAction(m_saveAllDocuments);
    projects->addAction(m_closeAllProjects);

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
    // The text an operation aims at first: it is the one that changes with a
    // key press, and it is absent for most users. Then the encoding, being the
    // only one of the other three that describes the file rather than what is
    // deduced from it or associated with it.
    statusBar()->addPermanentWidget(m_targetStatus);
    m_targetStatus->hide();
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
    bar->addSeparator();
    // **The one operation of the window that belongs on a bar.** It takes no
    // option and no dialog, so a button applies it whole; the others open a box
    // and would be a button that asks a question.
    bar->addAction(m_italic);

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
    std::unique_ptr<ProjectPage> page = ProjectPage::make(std::move(project), diagnostics);

    // The model has carried a cell edit out as a command since issue #129, so
    // the window does not see them go by. This signal is how it learns of
    // one — including an edit that changed nothing. Made once, for the life
    // of this model: a page never gets another.
    //
    // **Any page's history, and not only the shown one's** — issue #461:
    // `Replace All` over every project writes into pages that stay behind
    // their tabs. Such a page has only its tab to say it changed; the actions
    // and the title are the shown page's, and wait for it to be shown.
    ProjectPage* const born = page.get();
    connect(page->model.get(), &SubtitleTableModel::historyChanged, this, [this, born] {
        if (born == m_page)
            refreshActions();
        else
            refreshTabOf(*born);
    });
    // A structural undo or redo resets the model rather than reporting which
    // rows changed — Qt then clears the selection without a
    // `selectionChanged`, which is otherwise what forgets a stale target. This
    // catches that one case directly on Qt's own reset signal — on the page
    // whose model it is, which since #461 need not be the one on screen.
    connect(page->model.get(), &QAbstractItemModel::modelReset, this, [born] {
        born->searchTarget.reset();
        born->match.reset();
    });

    // **This page's own selection model, connected once, for its whole
    // life.** `setModel` throws away whichever one the table had and makes a
    // fresh, empty one of its own — `switchToPage` swaps it back out for this
    // one on every visit, which is what carries the selection over rather
    // than losing it. Reconnecting on every visit would connect the same
    // signal to the same slot again each time, since this one object outlives
    // them all.
    connect(page->tableSelection.get(),
            &QItemSelectionModel::selectionChanged,
            this,
            &MainWindow::placePlaybackAtSelection);
    // A selection the user makes is a new target for the next search; the one
    // the search makes, moving to a match, is not.
    connect(page->tableSelection.get(), &QItemSelectionModel::selectionChanged, this, [this] {
        if (!m_page->movingToMatch) {
            m_page->searchTarget.reset();
            m_page->match.reset();
        }
    });
    // The only two actions whose state depends on the selection, and they
    // listen to it alone: `refreshActions` deduces the grid of the whole file,
    // and wiring it here would pay for that deduction at every row of a drag.
    connect(page->tableSelection.get(),
            &QItemSelectionModel::selectionChanged,
            this,
            &MainWindow::refreshStructureActions);
    // Which text an operation aims at follows the column of the current cell,
    // and so does the status bar that says it. A change of *row* changes
    // neither, and is not listened to.
    connect(page->tableSelection.get(),
            &QItemSelectionModel::currentColumnChanged,
            this,
            &MainWindow::refreshTarget);

    m_pages.push_back(std::move(page));
    const int index = static_cast<int>(m_pages.size()) - 1;
    {
        // Blocked: adding the very first tab to an empty bar makes Qt pick it
        // as current on its own and fire `currentChanged` right there — before
        // this function has had its own say, and reachable from inside the
        // constructor, where nothing else has connected to it yet.
        const QSignalBlocker blocker{m_tabBar};
        m_tabBar->addTab(tabLabelFor(m_pages.back()->session->project(), false));
    }

    switchToPage(index);

    // Here and not folded into `switchToPage`, which runs at every visit to a
    // tab and not only at its birth: the header has no section before it has
    // a model, so this could not run any earlier than the `setModel` above —
    // and a column a user has since resized must not be put back at every
    // return to this one.
    m_table->horizontalHeader()->resizeSection(SubtitleTableModel::Text, kDefaultTextWidth);
}

void MainWindow::switchToPage(int index) {
    if (index == m_currentPage)
        return;

    m_currentPage = index;
    m_page = m_pages[static_cast<std::size_t>(index)].get();

    // Mirrors the choice without firing `currentChanged` a second time: a
    // click on the bar reaches this function through that very signal, and a
    // keyboard shortcut or a closed tab reach it directly.
    const QSignalBlocker blocker{m_tabBar};
    m_tabBar->setCurrentIndex(index);

    m_table->setModel(m_page->model.get());
    // **Put back rather than left to what `setModel` just built.** It throws
    // away whichever selection model the table had and makes a fresh, empty
    // one of its own every time — including a return to a model it has shown
    // before. This page's own, made once in `openOn` and connected there, is
    // what carries its selection over instead of losing it.
    m_table->setSelectionModel(m_page->tableSelection.get());

    refreshForPage();
}

void MainWindow::refreshForPage() {
    setWindowTitle(titleFor(m_page->session->project()));
    m_diagnostics->setDiagnostics(m_page->diagnostics);
    // Reads `m_page->associated` against `m_playingPage`, not only against
    // what this project itself last wanted: a switch of tab means the shared
    // player is showing someone else's film even when this one's own
    // association has not changed since it was last the page on screen.
    proposeVideoBeside();
    refreshActions();
    refreshTabActions();
}

void MainWindow::selectVideo() {
    const std::optional<std::filesystem::path>& source =
        m_page->session->project().sourceFile().path;
    const std::filesystem::path directory =
        source.has_value() ? source->parent_path() : std::filesystem::path{};

    const std::optional<std::filesystem::path> chosen = m_prompts->videoToOpen(directory);
    if (!chosen.has_value())
        return;

    m_page->session->chooseVideo(*chosen);
    refreshVideo();
}

void MainWindow::proposeVideoBeside() {
    const std::optional<std::filesystem::path>& source =
        m_page->session->project().sourceFile().path;
    if (source.has_value()) {
        if (const std::optional<std::filesystem::path> found =
                core::findVideoBeside(*m_files, *source);
            found.has_value()) {
            // The answer is dropped on purpose: whether the proposal was taken
            // is D5's business, and a caller acting on it would be a second
            // place where that rule lives.
            (void)m_page->session->proposeVideo(*found);
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
    m_encodingStatus->setText(QString::fromStdString(
        core::encodingStatusOf(m_page->session->project().sourceFile().encoding)));
}

std::optional<core::FrameRate> MainWindow::rateReadInFrames() const {
    const core::FileExtras& extras = m_page->session->project().sourceFile().extras;
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
            QString::fromStdString(core::framesStatusOf(m_page->session->project().frameRate())));
        return;
    }

    const core::FrameRateDeduction grid = core::deduceFrameRate(m_page->session->project());
    const std::optional<core::FrameRate> retained = grid.verdict == core::GridVerdict::Silent
                                                        ? std::nullopt
                                                        : std::optional{grid.retained.rate};

    m_gridStatus->setText(QString::fromStdString(core::gridStatusOf(grid.verdict, retained)));
}

void MainWindow::snapToFrameRate() {
    const core::Selection target = targetOf(*m_table->selectionModel(), m_page->session->project());
    const std::optional<core::AssociatedVideo>& associated = m_page->session->project().video();

    SnapDialog dialog{target.count(),
                      m_page->session->project().frameRate(),
                      associated.has_value() ? associated->declared : std::nullopt,
                      this};
    if (!m_prompts->run(dialog))
        return;

    const std::string pastTheEnd = applyOperationQuietly(
        *m_page,
        std::make_unique<core::SnapCommand>(m_page->session->project(), target, dialog.rate()),
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
    //
    // The same box as what the alignment left past the end of the film, when it
    // left anything — issue #418.
    const std::optional<core::PartialAlignment> partial =
        core::partialAlignment(m_page->session->project(), target, dialog.rate());
    const std::string behind = partial.has_value() ? core::noticeOf(*partial) : std::string{};
    if (const std::string notice = joinedNotices(behind, pastTheEnd); !notice.empty())
        m_prompts->reportOutcome(notice);
}

void MainWindow::shiftOntoGrid() {
    const std::optional<core::Duration> by =
        core::shiftOntoGrid(core::deduceFrameRate(m_page->session->project()));
    if (!by.has_value())
        return;

    const core::Selection whole = core::Selection::all(m_page->session->project());

    // The rule the core has held since #132, shared with the command line: a
    // position before the origin is representable, and no subtitle file can
    // hold one.
    if (const std::optional<core::SubtitleIndex> refused =
            core::firstBeforeOrigin(m_page->session->project(), whole, *by);
        refused.has_value()) {
        m_prompts->reportFailure("subtitle " + std::to_string(refused->number()) +
                                 " would start before the origin, which no subtitle file can hold");
        return;
    }

    applyOperation(*m_page, std::make_unique<core::ShiftCommand>(whole, *by), whole);
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
    GridAnalysisDialog dialog{core::deduceFrameRate(m_page->session->project()), this};
    (void)m_prompts->run(dialog);
}

void MainWindow::refreshVideoStatus() {
    const std::optional<core::AssociatedVideo>& associated = m_page->session->project().video();
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
    // `m_page->associated` alone so that it finds the film still waiting.
    if (!m_wasShown)
        return;

    const std::optional<core::AssociatedVideo>& associated = m_page->session->project().video();
    const std::filesystem::path wanted =
        associated.has_value() ? associated->path : std::filesystem::path{};

    // **Two things have to agree, not one.** `wanted == m_page->associated`
    // alone answers « has this project's own association changed since it was
    // last synced », which is right for one project and wrong for several: a
    // switch of tab back to a page whose association never changed still
    // means the shared player is showing whatever the page just left behind
    // was watching, not this one.
    if (wanted == m_page->associated && m_playingPage == m_page)
        return;

    m_playingPage = m_page;
    m_page->associated = wanted;
    m_page->watching = false;
    m_page->shown.clear();
    m_page->placedAt = -1;

    // Asked once per film, here, where the association has just changed for
    // certain: `ffprobe` is a process, and running it at every opening of the
    // frame rate dialog would pay for it again for an answer that cannot have
    // moved. Nothing, without a reader or without a film — which is what a
    // machine with no `ffmpeg` gets, and it is an ordinary state.
    m_page->session->setDeclaredFrameRate(
        !wanted.empty() && m_readDeclaredRate ? m_readDeclaredRate(wanted) : std::nullopt);

    // **Shown before the film is opened, and not after.** libmpv adopts the
    // window it is handed at that moment; one that is not on screen is adopted
    // and never mapped. Taken away again below if the film will not open, which
    // costs nothing anybody sees: nothing has been painted into it yet.
    showPicture(!wanted.empty());

    core::VideoPlayer* watching = wanted.empty() ? nullptr : player();
    if (watching != nullptr) {
        if (const std::expected<void, core::PlayerError> opened = watching->open(wanted); opened)
            m_page->watching = true;
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
    if (!m_page->watching && m_player) {
        m_player->pause();
        m_player->showSubtitle({});
    }

    // The mark goes with the film: a row left green under a document that no
    // longer shows anything would name a moment nobody is at.
    if (!m_page->watching && m_page->model)
        m_page->model->setShowing(std::nullopt);

    // Exactly one of the two, always: a band that stayed under a playing film
    // would offer to choose the one already chosen.
    showPicture(m_page->watching);
    m_playPause->setEnabled(m_page->watching);

    if (m_page->watching) {
        m_ticker->start();
        followPlayback();
    } else {
        m_ticker->stop();
    }
}

void MainWindow::showPicture(bool picture) {
    QList<int> sizes = m_split->sizes();
    const int total = std::accumulate(sizes.begin(), sizes.end(), 0);
    // The room above the table is one, whichever child holds it; the picture
    // is never given less than its minimum, the table pays for it.
    const int above =
        std::min(std::max(sizes.at(0) + sizes.at(1), picture ? kMinimumVideoHeight : 0), total);

    m_videoView->setVisible(picture);
    m_noVideo->setVisible(!picture);

    sizes[0] = picture ? above : 0;
    sizes[1] = picture ? 0 : above;
    sizes[2] = total - above;
    m_split->setSizes(sizes);
}

void MainWindow::togglePlayback() {
    if (!m_page->watching)
        return;

    if (m_player->isPlaying())
        m_player->pause();
    else
        m_player->play();
}

void MainWindow::placePlaybackAtSelection() {
    if (!m_page->watching)
        return;

    const int row = firstSelectedRow(*m_table->selectionModel());
    if (row < 0 || row == m_page->placedAt)
        return;

    m_page->placedAt = row;
    const auto index = core::SubtitleIndex::fromValue(static_cast<std::size_t>(row));
    m_player->seek(m_page->session->project().subtitleAt(index).start);

    // At once rather than at the next tick: what the picture shows and what the
    // table points at have to agree by the time the click is over.
    followPlayback();
}

void MainWindow::followPlayback() {
    if (!m_page->watching)
        return;

    const core::Project& project = m_page->session->project();

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
    //
    // **The text of the document aimed at**, the rule of the whole window: the
    // column of the current cell says which, and there is no setting.
    const std::string line =
        showing.has_value() ? project.subtitleAt(*showing).text(targetDocument()) : std::string{};
    if (line != m_page->shown) {
        m_player->showSubtitle(line);
        m_page->shown = line;
    }

    m_page->model->setShowing(showing);

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
    const QModelIndex followed =
        m_page->model->index(row, current.isValid() ? current.column() : 0);
    m_table->selectionModel()->setCurrentIndex(followed, QItemSelectionModel::NoUpdate);
    m_table->scrollTo(followed);
}

bool MainWindow::isModified(core::Document document) const {
    return ProjectFiles::isModified(*m_page, document);
}

void MainWindow::openTranslationFromPrompt() {
    std::optional<ProjectFiles::ChosenTranslation> chosen =
        m_projectFiles->chooseTranslation(*m_page);
    if (!chosen.has_value())
        return;
    const core::TranslationFile& read = chosen->read;

    core::AttachedTranslation attached = core::attachTranslation(
        m_page->session->project(), read.lines, read.source, chosen->method);
    const core::TranslationOutcome outcome = attached.outcome;
    const core::Selection whole = core::Selection::all(m_page->session->project());
    const std::string pastTheEnd =
        applyOperationQuietly(*m_page, std::move(attached.command), whole);

    // **What has just been read is what its file says**: nothing was typed, and
    // closing must not offer to save a translation back to the file it came from.
    m_page->session->markSaved(core::Document::Translation);

    // An act of the user's, as showing the column is: the choice of having taken
    // it away once does not outlast the opening of a translation.
    m_columns->action(core::TableColumn::Translation)->setChecked(true);
    refreshActions();

    // What the reading ran into that is not about alignment — the panel of what
    // the last reading met.
    if (!read.diagnostics.empty())
        m_diagnostics->setDiagnostics(read.diagnostics);

    // **In the status bar when everything found its place, in a box to close
    // otherwise** — the rule #398 set for a gesture that has something to say.
    const std::string notice = core::noticeOf(outcome);
    if (outcome.isClean() && pastTheEnd.empty()) {
        statusBar()->showMessage(QString::fromStdString(notice), kOperationStatusTimeoutMs);
        return;
    }

    m_prompts->reportOutcome(joinedNotices(notice, pastTheEnd));
}

void MainWindow::openFromPrompt() {
    // **Nothing to discard, and nothing asked.** Opening lands on a tab of
    // its own since #437 — ADR 0033 — and no longer replaces the one the
    // window was showing.
    const std::optional<std::filesystem::path> chosen = m_projectFiles->askFileToOpen();
    if (!chosen.has_value())
        return;

    if (const std::optional<std::string> failure = openFile(*chosen); failure.has_value())
        m_prompts->reportFailure(*failure);
}

std::optional<std::string> MainWindow::openFile(const std::filesystem::path& path) {
    // Already open, in another tab or this one: the file a second choice of
    // it means is the one already there, and the window says so rather than
    // reading it a second time — Gaupol's own rule.
    if (const std::optional<int> already = m_projectFiles->indexOf(path); already.has_value()) {
        switchToPage(*already);
        statusBar()->showMessage(
            QString::fromStdString(path.filename().string() + ": already open"),
            kOperationStatusTimeoutMs);
        return std::nullopt;
    }

    std::expected<core::OpenedFile, std::string> opened = m_projectFiles->read(path);
    if (!opened)
        return std::move(opened.error());

    openOn(std::move(opened->project), opened->diagnostics);
    return std::nullopt;
}

void MainWindow::openDropped(std::span<const std::filesystem::path> paths) {
    const ProjectFiles::Dropped dropped = ProjectFiles::sort(paths);
    std::vector<std::string> failures;

    for (const std::filesystem::path& path : dropped.subtitles) {
        if (std::optional<std::string> failure = openFile(path); failure.has_value())
            failures.push_back(std::move(*failure));
    }

    // After the subtitles, whatever order the drop listed them in: the film
    // goes to the tab shown once they are open, which is the last one opened.
    if (dropped.films.size() == 1) {
        m_page->session->chooseVideo(dropped.films.front());
        refreshVideo();
    } else if (dropped.films.size() > 1) {
        failures.push_back(std::to_string(dropped.films.size()) +
                           " videos dropped at once: a project watches one film");
    }

    // One box and not one per file: a drop of ten unreadable files is one
    // gesture, and ten boxes would be ten to dismiss.
    if (failures.empty())
        return;
    std::string message;
    for (const std::string& failure : failures) {
        if (!message.empty())
            message += '\n';
        message += failure;
    }
    m_prompts->reportFailure(message);
}

void MainWindow::dragEnterEvent(QDragEnterEvent* event) {
    if (event->mimeData()->hasUrls())
        event->acceptProposedAction();
}

void MainWindow::dropEvent(QDropEvent* event) {
    std::vector<std::filesystem::path> paths;
    for (const QUrl& url : event->mimeData()->urls()) {
        // A local file, and nothing else: a link dragged from a browser names
        // nothing this window can read.
        if (url.isLocalFile())
            paths.emplace_back(url.toLocalFile().toStdString());
    }
    event->acceptProposedAction();
    openDropped(paths);
}

void MainWindow::newProject() {
    openOn(core::Project{}, {});
}

void MainWindow::refreshTabActions() {
    // **Out with one tab left.** The window always holds at least one
    // project; closing the last would be closing the window, which is what
    // the title bar's own button already does.
    m_closeProject->setEnabled(m_pages.size() > 1);
    // Opened and closed tabs change what « all » means.
    m_search->refresh();
}

void MainWindow::closeCurrentProject() {
    if (m_pages.size() <= 1)
        return;
    if (!m_projectFiles->mayDiscard(m_currentPage))
        return;

    const int closed = m_currentPage;
    {
        // Blocked: removing the current tab would otherwise make the bar pick
        // its own replacement and fire `currentChanged` before `m_pages` has
        // been told the same tab is gone — `switchToPage` would then read an
        // index the vector does not have yet.
        const QSignalBlocker blocker{m_tabBar};
        m_tabBar->removeTab(closed);
    }

    // Cleared before the page it might name is freed: `watchAssociatedVideo`
    // only ever compares this pointer, never dereferences it, but comparing
    // one that no longer points at anything is not a comparison this class
    // makes anywhere else, and it does not start here.
    if (m_playingPage == m_page)
        m_playingPage = nullptr;
    m_pages.erase(m_pages.begin() + closed);

    // The tab that takes its place is the one now at the same rank, or the
    // last one if the closed tab was itself the last — never out of range,
    // since a tab was just refused to close alone.
    const int next = std::min(closed, static_cast<int>(m_pages.size()) - 1);
    // `m_currentPage` no longer names a page that exists: read as "already
    // there" it would skip the very switch this needs.
    m_currentPage = -1;
    switchToPage(next);
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
    // **One question for every project**, `Close All` and the window's own
    // button alike — `GUI-TABS-02`. The first refusal stops the window closing
    // at all.
    if (!m_projectFiles->mayDiscardAll()) {
        event->ignore();
        return;
    }
    event->accept();
}

void MainWindow::refreshTabOf(const ProjectPage& page) {
    // Every caller hands a page the window holds: one of its own models
    // signalled, or one of its own projects was saved.
    const auto found =
        std::ranges::find_if(m_pages, [&page](const auto& held) { return held.get() == &page; });

    const bool modified = ProjectFiles::isModified(page, core::Document::Main) ||
                          ProjectFiles::isModified(page, core::Document::Translation);
    m_tabBar->setTabText(static_cast<int>(found - m_pages.begin()),
                         tabLabelFor(page.session->project(), modified));
}

void MainWindow::refreshActions() {
    const QString undo = undoLabel(m_page->session->nextUndoKind());
    const QString redo = redoLabel(m_page->session->nextRedoKind());

    m_undo->setEnabled(m_page->session->canUndo());
    m_undo->setText(undo);
    // Set explicitly: without it, Qt makes the tooltip out of the `iconText`,
    // and the toolbar button would say « Undo » twice instead of naming what it
    // would defeat.
    m_undo->setToolTip(undo);

    m_redo->setEnabled(m_page->session->canRedo());
    m_redo->setText(redo);
    m_redo->setToolTip(redo);

    // Modified if either document is: the title has one asterisk for the two,
    // and so does the tab. Here rather than at each edit because every edit,
    // save and change of tab already passes through this function.
    setWindowModified(isModified(core::Document::Main) || isModified(core::Document::Translation));
    refreshTabOf(*m_page);

    // Before `refreshTarget`, further down: a column that has come or gone
    // changes which text the current cell can be aiming at.
    refreshColumns();

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
    const bool anything = m_page->session->project().count() != 0;
    m_shift->setEnabled(anything);
    m_transform->setEnabled(anything);
    m_frameRate->setEnabled(anything);
    m_adjustDurations->setEnabled(anything);
    // Nothing to shift from: an empty document has no last subtitle to offset
    // the appended file by.
    m_appendFile->setEnabled(anything);
    // A cut needs a subtitle on each side of it.
    m_splitProject->setEnabled(m_page->session->project().count() >= 2);
    m_findAndReplace->setEnabled(anything);
    // Nothing to analyse either: an empty document has no positions to read a
    // grid off, and the dialog would open on « too few subtitles ».
    m_analyseGrid->setEnabled(anything);
    m_snap->setEnabled(anything);

    // The amount is measured here rather than when the entry is chosen, so that
    // the menu can say what it will do — and the entry goes out when there is
    // no grid to rejoin, which is not the same thing as an amount of zero.
    const std::optional<core::Duration> onto =
        anything ? core::shiftOntoGrid(core::deduceFrameRate(m_page->session->project()))
                 : std::nullopt;
    m_shiftOntoGrid->setEnabled(onto.has_value());
    m_shiftOntoGrid->setText(shiftOntoGridLabel(onto));
    m_hearingImpaired->setEnabled(anything);

    // Nothing to give a translation's lines to in an empty document, and nothing
    // to write without a translation.
    m_openTranslation->setEnabled(anything);
    const bool hasTranslation = m_page->session->project().translationFile().has_value();
    m_saveTranslation->setEnabled(hasTranslation);
    m_saveTranslationAs->setEnabled(hasTranslation);

    // The italic entry is the one whose state depends on the target: see
    // `refreshTarget`.
    refreshTarget();

    // **Nothing about a format decides these five**, unlike the italic: a case
    // and a dash are text, not style, and every format carries text.
    m_dialogueDashes->setEnabled(anything);
    for (QAction* one : m_case)
        one->setEnabled(anything);

    refreshStructureActions();
}

QAction* MainWindow::columnAction(core::TableColumn column) const {
    return m_columns->action(column);
}

QAction* MainWindow::translationColumnAction() const {
    return m_columns->action(core::TableColumn::Translation);
}

void MainWindow::refreshColumns() {
    m_columns->refresh(*m_page);
}

core::Document MainWindow::targetDocument() const {
    return m_columns->targetDocument();
}

void MainWindow::refreshTarget() {
    // Two texts, and only then: the label of a window that never opens a
    // translation says nothing, which is what keeps it from being noise.
    const bool twoTexts = m_columns->translationShown();
    if (twoTexts) {
        m_targetStatus->setText(QStringLiteral("Text: %1").arg(documentName(targetDocument())));
    } else {
        m_targetStatus->clear();
    }
    m_targetStatus->setVisible(twoTexts);

    // The search forgets a match found in the other text, and names the field
    // its box looks in.
    m_search->refresh();

    // **Grey rather than gone for TMPlayer and LRC**: the two formats write no
    // style at all, and an entry that is there and out is what tells a user
    // there is nothing to type. What the format can carry is the question, not
    // how it spells it — SubRip and WebVTT spell italics alike and disagree
    // about colour, which is why `abilitiesOf` exists beside `vocabularyOf`.
    //
    // **Of the document aimed at**: a translation may be in a format that
    // writes no style while the main text is in one that does.
    const bool anything = m_page->session->project().count() != 0;
    m_italic->setEnabled(
        anything &&
        core::abilitiesOf(m_page->session->project().sourceFile(targetDocument()).format).italic);
}

void MainWindow::refreshStructureActions() {
    const bool anything = m_page->session->project().count() != 0;
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

    // **The selection, and never the whole file** — the rule `Remove Subtitles`
    // follows, and for the same reason: a `Ctrl+X` on a table with nothing
    // selected would empty every text of the document. A paste needs a row to
    // start from, and without a selection the row would be guessed.
    m_cut->setEnabled(selected);
    m_copy->setEnabled(selected);
    m_paste->setEnabled(selected);

    // Read on the runs rather than the rows: one run is what contiguous means,
    // and its length says whether there is anything to merge or to split.
    const core::Selection rows = selectionOf(*m_table->selectionModel());
    const bool oneRun = rows.ranges().size() == 1;
    m_mergeSubtitles->setEnabled(oneRun && rows.count() >= 2);
    m_splitSubtitle->setEnabled(oneRun && rows.count() == 1);
}

void MainWindow::adjustDurationsOfTarget() {
    const core::Selection target = targetOf(*m_table->selectionModel(), m_page->session->project());

    DurationAdjustDialog dialog{target.count(), m_durationSettings, this};
    if (!m_prompts->run(dialog))
        return;

    // Kept even if nothing moves: it is what was asked, and the next dialog
    // offers it again.
    m_durationSettings = dialog.settings();

    core::DurationAdjustment adjustment = core::adjustDurations(
        m_page->session->project(), target, core::constraintsOf(m_durationSettings));
    const std::string account =
        core::noticeOfAdjustment(adjustment.adjusted, adjustment.sacrificed);

    // One box for both: lengthening an end is exactly what can carry it past
    // the film, and two modal boxes in a row was one too many — issue #418.
    std::string pastTheEnd;
    if (adjustment.command != nullptr)
        pastTheEnd = applyOperationQuietly(*m_page, std::move(adjustment.command), target);

    m_prompts->reportOutcome(joinedNotices(account, pastTheEnd));
}

void MainWindow::appendFileFromPrompt() {
    const std::optional<std::filesystem::path> chosen = m_projectFiles->askFileToOpen();
    if (!chosen.has_value())
        return;

    std::expected<core::OpenedFile, std::string> opened = m_projectFiles->read(*chosen);
    if (!opened) {
        m_prompts->reportFailure(opened.error());
        return;
    }

    // What the reading ran into, whether or not there was anything to append —
    // the panel of what the last reading met, as an ordinary opening shows it.
    if (!opened->diagnostics.empty())
        m_diagnostics->setDiagnostics(opened->diagnostics);

    core::AppendedFile appended = core::appendFile(m_page->session->project(), opened->project);
    if (appended.command == nullptr)
        return;

    // Read before the command goes: the project it names is about to grow.
    const core::SubtitleFormat from = opened->project.sourceFile().format;
    const core::SubtitleFormat to = m_page->session->project().sourceFile().format;
    const std::size_t first = m_page->session->project().count();
    const core::Selection target =
        core::Selection::range(core::SubtitleIndex::fromValue(first),
                               core::SubtitleIndex::fromValue(first + appended.inserted - 1));

    const std::string pastTheEnd =
        applyOperationQuietly(*m_page, std::move(appended.command), target);

    // The rows the append just wrote: what a second append starts past, and
    // what selecting them shows was added.
    selectRows(static_cast<int>(first), static_cast<int>(first + appended.inserted - 1));

    // **In the status bar when there was nothing else to say, in a box to
    // close otherwise** — the rule #398 set for a gesture that has something
    // to say.
    const std::string account = core::noticeOfAppend(appended.inserted, appended.loss, from, to);
    if (!appended.loss.isAny() && pastTheEnd.empty()) {
        statusBar()->showMessage(QString::fromStdString(account), kOperationStatusTimeoutMs);
        return;
    }
    m_prompts->reportOutcome(joinedNotices(account, pastTheEnd));
}

void MainWindow::splitProjectFromPrompt() {
    const core::Project& project = m_page->session->project();

    // Opens on the current row, the natural place to cut: « from here ».
    const int current = m_table->currentIndex().row();
    SplitProjectDialog dialog{
        project.count(), static_cast<std::size_t>(std::max(current, 0)) + 1, this};

    // **Where the cut falls, shown before it is made**, as Gaupol does: each
    // number the box takes selects its row. Cancelling gives the selection
    // back — issue #462.
    QItemSelectionModel& selection = *m_table->selectionModel();
    const QItemSelection before = selection.selection();
    const QModelIndex wasCurrent = selection.currentIndex();
    connect(
        &dialog, &SplitProjectDialog::rowChosen, this, [this](int row) { selectRows(row, row); });
    if (!m_prompts->run(dialog)) {
        selection.setCurrentIndex(wasCurrent, QItemSelectionModel::NoUpdate);
        selection.select(before, QItemSelectionModel::ClearAndSelect);
        return;
    }

    const core::SubtitleIndex from = core::SubtitleIndex::fromValue(dialog.firstOfTail());
    std::expected<core::SplitProject, core::SplitRefusal> split = core::splitProject(project, from);
    if (!split) {
        m_prompts->reportFailure("Cannot split at subtitle " + std::to_string(from.value() + 1) +
                                 ": subtitle " + std::to_string(split.error().before.value() + 1) +
                                 " would fall before the start of the video. Cut somewhere else.");
        return;
    }

    // The origin loses the tail in one entry of its own history; the new
    // project begins another, and the two know nothing of each other.
    const core::Selection tail =
        core::Selection::range(from, core::SubtitleIndex::fromValue(project.count() - 1));
    (void)applyOperationQuietly(*m_page, std::move(split->command), tail);

    const std::size_t moved = split->tail.count();
    openOn(std::move(split->tail), {});

    // Born holding subtitles no file has: closing it must ask, as it does for
    // any document that differs from what is on disk.
    m_page->session->markUnsaved(core::Document::Main);
    m_page->session->markUnsaved(core::Document::Translation);
    refreshActions();

    statusBar()->showMessage(QString::fromStdString(core::noticeOfSplit(moved)),
                             kOperationStatusTimeoutMs);
}

void MainWindow::removeHearingImpairedFromTarget() {
    const core::Selection target = targetOf(*m_table->selectionModel(), m_page->session->project());

    HearingImpairedDialog dialog{target.count(), this};
    if (!m_prompts->run(dialog))
        return;

    // Built before being applied, and asked what it will do: the count is read
    // from the command, never by counting again afterwards.
    std::unique_ptr<core::Command> command =
        core::removeHearingImpaired(m_page->session->project(), target, targetDocument());
    if (!command) {
        // Nothing bit. Say so, and put nothing in the history: an operation
        // that changes nothing is not an operation to undo.
        m_prompts->reportOutcome("no mention to remove");
        return;
    }

    const core::HearingImpairedTally tally = core::tallyOf(*command);
    applyOperation(*m_page, std::move(command), target);

    m_prompts->reportOutcome(core::countOf(tally.cleaned, "subtitle") + " cleaned, " +
                             std::to_string(tally.removed) + " removed");
}

void MainWindow::commitCellEditor() {
    // Why, and for which gestures: see the declaration — issue #397.
    if (m_table->isEditing())
        m_table->setFocus();
}

void MainWindow::toggleItalicsOnTarget() {
    commitCellEditor();

    const core::Selection target = targetOf(*m_table->selectionModel(), m_page->session->project());

    // Asked before anything is built, and of the target rather than of the
    // document: the button says what it will do to what is selected.
    const core::Document document = targetDocument();
    const bool italic = core::wouldItalicise(m_page->session->project(), target, document);

    std::unique_ptr<core::Command> command =
        core::setItalics(m_page->session->project(), target, document, italic);
    if (!command) {
        // Every text was already the way it was asked for. Say so, and put
        // nothing in the history: an operation that changes nothing is not an
        // operation to undo.
        statusBar()->showMessage(QString::fromStdString(core::nothingToChange()),
                                 kOperationStatusTimeoutMs);
        return;
    }

    // Read from the command before it goes, never by counting again after.
    const std::size_t rewritten = core::rewrittenCount(*command);
    applyOperation(*m_page, std::move(command), target);
    statusBar()->showMessage(QString::fromStdString(core::noticeOfItalics(rewritten, italic)),
                             kOperationStatusTimeoutMs);
}

QAction* MainWindow::caseAction(core::LetterCase wanted) const {
    const auto* const found = std::ranges::find(core::kLetterCases, wanted);
    return m_case.at(
        static_cast<std::size_t>(std::distance(std::ranges::begin(core::kLetterCases), found)));
}

void MainWindow::changeCaseOfTarget(core::LetterCase wanted) {
    commitCellEditor();

    const core::Selection target = targetOf(*m_table->selectionModel(), m_page->session->project());

    std::unique_ptr<core::Command> command =
        core::setLetterCase(m_page->session->project(), target, targetDocument(), wanted);
    if (!command) {
        statusBar()->showMessage(QString::fromStdString(core::nothingToChange()),
                                 kOperationStatusTimeoutMs);
        return;
    }

    const std::size_t rewritten = core::rewrittenCount(*command);
    applyOperation(*m_page, std::move(command), target);
    statusBar()->showMessage(QString::fromStdString(core::noticeOfRecase(rewritten)),
                             kOperationStatusTimeoutMs);
}

void MainWindow::toggleDialogueDashesOnTarget() {
    commitCellEditor();

    const core::Selection target = targetOf(*m_table->selectionModel(), m_page->session->project());

    // Asked of the target before anything is built: the entry says what it will
    // do to what is selected.
    const core::Document document = targetDocument();
    const bool dashed = core::wouldAddDialogueDashes(m_page->session->project(), target, document);

    std::unique_ptr<core::Command> command =
        core::setDialogueDashes(m_page->session->project(), target, document, dashed);
    if (!command) {
        statusBar()->showMessage(QString::fromStdString(core::nothingToChange()),
                                 kOperationStatusTimeoutMs);
        return;
    }

    const std::size_t rewritten = core::rewrittenCount(*command);
    applyOperation(*m_page, std::move(command), target);
    statusBar()->showMessage(
        QString::fromStdString(core::noticeOfDialogueDashes(rewritten, dashed)),
        kOperationStatusTimeoutMs);
}

void MainWindow::applyOperation(ProjectPage& page,
                                std::unique_ptr<core::Command> command,
                                const core::Selection& target) {
    // A notice and not a failure: nothing was prevented, and the sentence is
    // written to be read after the fact.
    if (const std::string notice = applyOperationQuietly(page, std::move(command), target);
        !notice.empty())
        m_prompts->reportOutcome(notice);
}

std::string MainWindow::applyOperationQuietly(ProjectPage& page,
                                              std::unique_ptr<core::Command> command,
                                              const core::Selection& target) {
    // Read before the command goes: what it is, is what the notice names.
    const core::CommandKind kind = command->kind();

    page.model->applied(page.session->apply(std::move(command)));

    // The row playback was placed at holds something else now — a shift moved
    // it, a removal may have taken it away. Forgetting it is what lets a click
    // on that same row send playback where the subtitle has gone.
    page.placedAt = -1;

    return whatPassesTheEnd(page, kind, target);
}

std::optional<core::Duration> MainWindow::videoLength(const ProjectPage& page) const {
    // The player is shared: what it knows is the length of the film of the
    // page it plays for, and of no other.
    return page.watching && &page == m_playingPage ? m_player->duration() : std::nullopt;
}

std::string MainWindow::whatPassesTheEnd(const ProjectPage& page,
                                         core::CommandKind kind,
                                         const core::Selection& target) const {
    // **Only the operations that move a position.** `beyondEnd` reads the state
    // an operation produced; on its own it cannot tell whether that operation
    // put anything there. A subtitle already past the end because the film is
    // the wrong one is nobody's doing, least of all that of a removal of
    // hearing-impaired mentions.
    if (!core::movesPositions(kind))
        return {};

    const std::optional<core::BeyondEnd> beyond =
        core::beyondEnd(page.session->project(), target, videoLength(page));
    return beyond.has_value() ? core::noticeOf(kind, *beyond) : std::string{};
}

void MainWindow::insertSubtitles() {
    const core::Project& project = m_page->session->project();

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
        *m_page,
        std::make_unique<core::InsertCommand>(core::InsertCommand::blank(project, index, count)),
        inserted);

    // The table has been reset, so the selection went with it. Leaving the new
    // rows selected is what Gaupol does, and what makes it possible to press
    // `Ins` a second time.
    selectRows(static_cast<int>(at), static_cast<int>(at + count - 1));
}

void MainWindow::removeSubtitles() {
    commitCellEditor();

    const core::Selection target = selectionOf(*m_table->selectionModel());
    if (target.isEmpty())
        return;

    // Read before the operation goes: it is the place the first removed row
    // leaves, and it has no name any more once the removal is done.
    const int emptied = static_cast<int>(target.ranges().front().first.value());

    applyOperation(*m_page, std::make_unique<core::RemoveCommand>(target), target);

    // The row that took that place, or the last one when the removal carried
    // off the end of the file. Without it, a second `Del` would find no
    // selection left and the action would be out.
    const int left = static_cast<int>(m_page->session->project().count());
    if (left > 0)
        selectRows(std::min(emptied, left - 1), std::min(emptied, left - 1));
}

void MainWindow::copyTexts() {
    commitCellEditor();

    const core::Selection target = selectionOf(*m_table->selectionModel());
    if (target.isEmpty())
        return;

    m_clipboard = core::copyTexts(m_page->session->project(), target, targetDocument());
    QGuiApplication::clipboard()->setText(QString::fromStdString(core::plainTextOf(m_clipboard)));
}

void MainWindow::cutTexts() {
    commitCellEditor();

    const core::Selection target = selectionOf(*m_table->selectionModel());
    if (target.isEmpty())
        return;

    copyTexts();

    std::unique_ptr<core::Command> command =
        core::cutTexts(m_page->session->project(), target, targetDocument());
    if (command == nullptr)
        return;

    // A change of text and not of structure: the table keeps its selection.
    applyOperation(*m_page, std::move(command), target);
}

void MainWindow::pasteTexts() {
    commitCellEditor();

    const core::Selection target = selectionOf(*m_table->selectionModel());
    if (target.isEmpty())
        return;

    // Compared on the plain form, which is all the system keeps: the same
    // string means the same copy, and this window knows its format.
    const std::string plain = QGuiApplication::clipboard()->text().toStdString();
    const core::ClipboardTexts clipboard =
        !m_clipboard.isEmpty() && plain == core::plainTextOf(m_clipboard)
            ? m_clipboard
            : core::textsFromPlain(plain);
    if (clipboard.isEmpty())
        return;

    const core::SubtitleIndex at = target.ranges().front().first;
    const core::Document document = targetDocument();
    const core::SubtitleFormat format = m_page->session->project().sourceFile(document).format;
    core::PastedTexts pasted =
        core::pasteTexts(m_page->session->project(), clipboard, at, document);
    if (pasted.command == nullptr)
        return;

    applyOperation(*m_page, std::move(pasted.command), target);

    // The rows written, which a paste past the end has just rebuilt the table
    // around: without them the selection would be gone, and a second paste
    // would have nowhere to start.
    const int first = static_cast<int>(at.value());
    selectRows(first, first + static_cast<int>(clipboard.texts.size()) - 1);

    const core::ConversionLoss loss{.tags = pasted.droppedTags};
    const std::string notice = core::noticeOfPaste(pasted.inserted, loss, clipboard.format, format);
    if (!notice.empty())
        m_prompts->reportOutcome(notice);
}

void MainWindow::openSearch() {
    m_search->open();
}

SearchDialog* MainWindow::searchDialog() const {
    return m_search->dialog();
}

void MainWindow::mergeSubtitles() {
    commitCellEditor();

    // The guard of the action, said again: nothing keeps a trigger from finding
    // it a fraction of a second too late. A run of one gets no command from the
    // core, which is the second half of the same guard.
    const core::Selection target = selectionOf(*m_table->selectionModel());
    if (target.ranges().size() != 1)
        return;

    const core::IndexRange run = target.ranges().front();
    std::unique_ptr<core::Command> command = core::mergeSubtitles(m_page->session->project(), run);
    if (command == nullptr)
        return;

    applyOperation(*m_page, std::move(command), target);

    const int merged = static_cast<int>(run.first.value());
    selectRows(merged, merged);
}

void MainWindow::splitSubtitle() {
    commitCellEditor();

    const core::Selection target = selectionOf(*m_table->selectionModel());
    if (target.count() != 1)
        return;

    const core::SubtitleIndex index = target.ranges().front().first;
    applyOperation(*m_page, core::splitSubtitle(m_page->session->project(), index), target);

    const int first = static_cast<int>(index.value());
    selectRows(first, first + 1);
}

void MainWindow::selectRows(int first, int last) {
    // **The column of the current cell stays where it is.** It says which text
    // an operation aims at, and moving to a match, or to the rows a paste has
    // just written, must not change the answer: a search begun in the
    // translation column would otherwise go on in the main text from its first
    // match on.
    const QModelIndex current = m_table->currentIndex();
    const int column = current.isValid() ? current.column() : 0;
    const QModelIndex from = m_page->model->index(first, column);
    const QModelIndex to = m_page->model->index(last, SubtitleTableModel::kColumnCount - 1);

    // The current row first, and without touching the selection: going through
    // the view's `setCurrentIndex` would shrink it to that one row, which would
    // undo the selection laid down right after.
    m_table->selectionModel()->setCurrentIndex(from, QItemSelectionModel::NoUpdate);
    m_table->selectionModel()->select(
        QItemSelection{from, to}, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
    m_table->scrollTo(from);
}

void MainWindow::shiftTarget() {
    const core::Selection target = targetOf(*m_table->selectionModel(), m_page->session->project());

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
            core::firstBeforeOrigin(m_page->session->project(), target, *by);
        refused.has_value()) {
        m_prompts->reportFailure("subtitle " + std::to_string(refused->number()) +
                                 " would start before the origin, which no subtitle file can "
                                 "hold");
        return;
    }

    applyOperation(*m_page, std::make_unique<core::ShiftCommand>(target, *by), target);
}

void MainWindow::transformTarget() {
    const core::Selection target = targetOf(*m_table->selectionModel(), m_page->session->project());

    TransformDialog dialog{target.count(), m_page->session->project().count(), this};
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
        m_page->session->project(), target, referenceOf(*first), referenceOf(*second));
    if (!command.has_value()) {
        m_prompts->reportFailure("the two references define no correction");
        return;
    }

    applyOperation(*m_page, std::make_unique<core::TransformCommand>(std::move(*command)), target);
}

void MainWindow::convertFrameRateOfTarget() {
    const core::Selection target = targetOf(*m_table->selectionModel(), m_page->session->project());

    // Pre-filled with the project's own, never guessed: the file does not
    // carry it, and getting it wrong shifts everything without a word. What the
    // film declares is handed over beside it, and the dialog decides what to do
    // with it — proposed, never imposed (D6).
    const std::optional<core::AssociatedVideo>& associated = m_page->session->project().video();
    // **Only a clean grid pre-fills the field.** A partial one is evidence the
    // deduction itself calls partial, and this field decides an operation on
    // the whole file; the status bar and the analysis carry that case instead.
    //
    // **And a document counted in frames leaves it out entirely.** Its
    // positions come from its frames at the rate it was read at, so the
    // deduction can only find that rate again; what is offered instead is the
    // rate itself, said for what it is.
    const std::optional<core::FrameRate> read = rateReadInFrames();
    const core::FrameRateDeduction grid = core::deduceFrameRate(m_page->session->project());
    const std::optional<core::FrameRate> measured =
        !read.has_value() && grid.verdict == core::GridVerdict::Clean
            ? std::optional{grid.retained.rate}
            : std::nullopt;

    FrameRateDialog dialog{target.count(),
                           m_page->session->project().frameRate(),
                           associated.has_value() ? associated->declared : std::nullopt,
                           measured,
                           read,
                           this};
    if (!m_prompts->run(dialog))
        return;

    applyOperation(*m_page,
                   std::make_unique<core::ConvertFrameRateCommand>(
                       m_page->session->project(), target, dialog.input(), dialog.output()),
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

    m_columns->apply(settings);

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

    m_projectFiles->setLastDirectory(settings.lastDirectory.value_or(std::filesystem::path{}));

    m_theme = settings.theme;
    applyTheme(m_theme);

    m_insertPlacement = settings.insertPlacement;
    m_durationSettings = settings.durationAdjustment;
    m_search->setOptions(settings.search);
    m_page->writeEncoding = settings.writeEncoding;
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

    m_columns->write(settings);

    if (m_split != nullptr) {
        const QList<int> sizes = m_split->sizes();
        const int total = std::accumulate(sizes.begin(), sizes.end(), 0);
        if (total > 0 && !sizes.isEmpty())
            settings.tableShare = std::clamp(sizes.back() * kPerCent / total,
                                             core::kSmallestTableShare,
                                             core::kLargestTableShare);
    }

    if (!m_projectFiles->lastDirectory().empty())
        settings.lastDirectory = m_projectFiles->lastDirectory();

    settings.theme = m_theme;
    settings.insertPlacement = m_insertPlacement;
    settings.search = m_search->options();
    settings.durationAdjustment = m_durationSettings;
    settings.writeEncoding = m_page->writeEncoding;

    return settings;
}

} // namespace subedit::gui
