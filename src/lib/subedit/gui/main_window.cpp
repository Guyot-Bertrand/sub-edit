#include <subedit/core/edit/session.hpp>
#include <subedit/core/model/document.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/model/source_file.hpp>
#include <subedit/gui/cell_delegates.hpp>
#include <subedit/gui/command_label.hpp>
#include <subedit/gui/main_window.hpp>
#include <subedit/gui/subtitle_table_model.hpp>

#include <QAction>
#include <QHeaderView>
#include <QIcon>
#include <QKeySequence>
#include <QMenu>
#include <QMenuBar>
#include <QString>
#include <QTableView>
#include <QToolBar>

#include <memory>
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

MainWindow::MainWindow(core::Project project, QWidget* parent)
    : QMainWindow(parent),
      m_table(new QTableView{this}),
      m_undo(buildAction(this, QStringLiteral("Undo"), QStringLiteral("edit-undo"))),
      m_redo(buildAction(this, QStringLiteral("Redo"), QStringLiteral("edit-redo"))) {
    setWindowTitle(titleFor(project));

    m_session = std::make_unique<core::Session>(std::move(project));
    m_model = std::make_unique<SubtitleTableModel>(*m_session);

    m_table->setModel(m_model.get());

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

    setCentralWidget(m_table);

    m_undo->setShortcut(QKeySequence::Undo);
    m_redo->setShortcut(QKeySequence::Redo);
    connect(m_undo, &QAction::triggered, this, [this] { m_model->applied(m_session->undo()); });
    connect(m_redo, &QAction::triggered, this, [this] { m_model->applied(m_session->redo()); });

    // Le modèle applique les commandes d'une cellule depuis l'issue #129, donc
    // la fenêtre ne les voit pas passer. Ce signal est ce par quoi elle
    // l'apprend — y compris d'une édition qui n'a rien changé.
    connect(m_model.get(), &SubtitleTableModel::historyChanged, this, &MainWindow::refreshActions);

    QMenu* edition = menuBar()->addMenu(QStringLiteral("&Edit"));
    edition->addAction(m_undo);
    edition->addAction(m_redo);

    QToolBar* bar = addToolBar(QStringLiteral("Edit"));
    bar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    bar->addAction(m_undo);
    bar->addAction(m_redo);

    refreshActions();
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
}

MainWindow::~MainWindow() = default;

} // namespace subedit::gui
