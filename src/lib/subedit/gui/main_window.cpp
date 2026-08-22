#include <subedit/core/edit/convert_frame_rate_command.hpp>
#include <subedit/core/edit/hearing_impaired_removal.hpp>
#include <subedit/core/edit/session.hpp>
#include <subedit/core/edit/shift_command.hpp>
#include <subedit/core/edit/shift_limits.hpp>
#include <subedit/core/edit/transform_command.hpp>
#include <subedit/core/format/diagnostic.hpp>
#include <subedit/core/io/file_system.hpp>
#include <subedit/core/model/document.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/model/source_file.hpp>
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
#include <subedit/gui/subtitle_table_model.hpp>
#include <subedit/gui/target.hpp>
#include <subedit/gui/transform_dialog.hpp>

#include <QAction>
#include <QCloseEvent>
#include <QHeaderView>
#include <QIcon>
#include <QItemSelectionModel>
#include <QKeySequence>
#include <QMenu>
#include <QMenuBar>
#include <QString>
#include <QTableView>
#include <QToolBar>
#include <QVBoxLayout>
#include <QWidget>

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

} // namespace

MainWindow::MainWindow(core::FileSystem& files,
                       OpenedFile opened,
                       Prompts& prompts,
                       QWidget* parent)
    : QMainWindow(parent),
      m_files(&files),
      m_prompts(&prompts),
      m_table(new QTableView{this}),
      m_diagnostics(new DiagnosticsPanel{this}),
      m_undo(buildAction(this, QStringLiteral("Undo"), QStringLiteral("edit-undo"))),
      m_redo(buildAction(this, QStringLiteral("Redo"), QStringLiteral("edit-redo"))),
      m_open(buildAction(this, QStringLiteral("Open…"), QStringLiteral("document-open"))),
      m_save(buildAction(this, QStringLiteral("Save"), QStringLiteral("document-save"))),
      m_saveAs(buildAction(this, QStringLiteral("Save As…"), QStringLiteral("document-save-as"))),
      m_shift(buildAction(this, QStringLiteral("Shift Positions…"), {})),
      m_transform(buildAction(this, QStringLiteral("Transform Positions…"), {})),
      m_frameRate(buildAction(this, QStringLiteral("Convert Frame Rate…"), {})),
      m_hearingImpaired(
          buildAction(this, QStringLiteral("Remove Hearing-Impaired Mentions…"), {})) {
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

    // The table takes the room, the panel slips underneath and goes away when
    // it has nothing to say.
    auto* centre = new QWidget{this};
    auto* stack = new QVBoxLayout{centre};
    stack->setContentsMargins(0, 0, 0, 0);
    stack->addWidget(m_table);
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

    m_diagnostics->setDiagnostics(diagnostics);
    refreshActions();
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
