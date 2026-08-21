#include <subedit/core/edit/convert_frame_rate_command.hpp>
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
/// « Annuler : décalage ». `iconText` is what the toolbar button reads and it
/// never changes: a button whose width followed the last operation would move
/// under the pointer.
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
      m_frameRate(buildAction(this, QStringLiteral("Convert Frame Rate…"), {})) {
    // Un délégué par nature de cellule, et aucun sur le numéro ni sur la durée,
    // qui ne s'éditent pas : la table de Qt n'en pose que là où on lui en pose.
    m_table->setItemDelegateForColumn(SubtitleTableModel::Start, new PositionDelegate{this});
    m_table->setItemDelegateForColumn(SubtitleTableModel::End, new PositionDelegate{this});
    m_table->setItemDelegateForColumn(SubtitleTableModel::Text, new TextDelegate{this});
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->verticalHeader()->setVisible(false);

    // The text takes what the positions leave: it is the column that varies,
    // and the four others are known widths.
    m_table->horizontalHeader()->setStretchLastSection(true);

    // La table prend la place, le panneau se glisse dessous et disparaît quand
    // il n'a rien à dire.
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
    // La valeur rendue ne sert qu'à qui enchaîne derrière ; déclenchée par
    // l'action, elle n'a personne à renseigner.
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

    QMenu* tools = menuBar()->addMenu(QStringLiteral("&Tools"));
    tools->addAction(m_shift);
    tools->addAction(m_transform);
    tools->addAction(m_frameRate);

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

    // Le document arrive par le même chemin que ceux qui suivront : une seule
    // façon de poser un fichier dans la fenêtre, donc un seul endroit où elle
    // peut être fausse.
    openOn(std::move(opened.project), opened.diagnostics);
}

void MainWindow::openOn(core::Project project, std::span<const core::Diagnostic> diagnostics) {
    setWindowTitle(titleFor(project));

    // Reconstruits plutôt que remis à zéro : une session porte un historique,
    // et l'historique d'un fichier n'a rien à dire du suivant.
    auto session = std::make_unique<core::Session>(std::move(project));
    auto model = std::make_unique<SubtitleTableModel>(*session);

    m_table->setModel(model.get());
    // Le modèle applique les commandes d'une cellule depuis l'issue #129, donc
    // la fenêtre ne les voit pas passer. Ce signal est ce par quoi elle
    // l'apprend — y compris d'une édition qui n'a rien changé. Rebranché à
    // chaque ouverture, le modèle d'avant partant avec le fichier d'avant.
    connect(model.get(), &SubtitleTableModel::historyChanged, this, &MainWindow::refreshActions);

    // Dans cet ordre : la vue lâche l'ancien modèle avant qu'il ne parte, et
    // le modèle avant la session qu'il lit.
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

    // Le document vit désormais là, et dans ce format : ce n'est pas une
    // commande, personne ne voudrait l'annuler.
    core::SourceFile moved = m_session->project().sourceFile();
    moved.path = target->path;
    moved.format = target->format;
    m_session->setSourceFile(std::move(moved));

    m_session->markSaved(core::Document::Main);
    setWindowTitle(titleFor(m_session->project()));

    // Le format gouverne le séparateur décimal que la table montre : il vient
    // de changer, donc tout ce qu'elle affiche est à relire.
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
    // Demandé avant de demander quoi ouvrir : renoncer à perdre son travail ne
    // doit pas obliger à choisir un fichier d'abord.
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
    // Posée explicitement : sans elle, Qt fait de l'infobulle l'`iconText`,
    // et le bouton de la barre d'outils dirait « Annuler » deux fois au lieu
    // de nommer ce qu'il défera.
    m_undo->setToolTip(undo);

    m_redo->setEnabled(m_session->canRedo());
    m_redo->setText(redo);
    m_redo->setToolTip(redo);

    setWindowModified(m_session->hasUnsavedChanges(core::Document::Main));

    // Rien à décaler, rien à transformer : une action active ouvrirait un
    // dialogue qui ne pourrait porter sur rien.
    const bool anything = m_session->project().count() != 0;
    m_shift->setEnabled(anything);
    m_transform->setEnabled(anything);
    m_frameRate->setEnabled(anything);
}

void MainWindow::applyOperation(std::unique_ptr<core::Command> command) {
    m_model->applied(m_session->apply(std::move(command)));
}

void MainWindow::shiftTarget() {
    const core::Selection target = targetOf(*m_table->selectionModel(), m_session->project());

    ShiftDialog dialog{m_session->project().count(), this};
    if (!m_prompts->run(dialog))
        return;

    const std::optional<core::Duration> by = dialog.shift();
    if (!by.has_value())
        return;

    // Une position avant l'origine est représentable, mais aucun fichier de
    // sous-titres ne peut la porter. La règle vit dans le noyau depuis #132,
    // partagée avec la ligne de commande.
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

    TransformDialog dialog{m_session->project().count(), this};
    if (!m_prompts->run(dialog))
        return;

    const std::optional<TypedReference> first = dialog.first();
    const std::optional<TypedReference> second = dialog.second();
    if (!first.has_value() || !second.has_value())
        return;

    // Ce que le dialogue a lu devient ici la valeur du noyau : il tient des
    // widgets, pas le vocabulaire d'une commande.
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

    // Pré-remplie par celle du projet, jamais devinée : le fichier ne la
    // porte pas, et se tromper décale tout sans rien signaler.
    FrameRateDialog dialog{m_session->project().count(), m_session->project().frameRate(), this};
    if (!m_prompts->run(dialog))
        return;

    applyOperation(std::make_unique<core::ConvertFrameRateCommand>(
        m_session->project(), target, dialog.input(), dialog.output()));
}

MainWindow::~MainWindow() = default;

} // namespace subedit::gui
