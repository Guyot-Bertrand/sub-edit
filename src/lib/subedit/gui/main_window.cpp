#include <subedit/core/edit/session.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/model/source_file.hpp>
#include <subedit/gui/cell_delegates.hpp>
#include <subedit/gui/main_window.hpp>
#include <subedit/gui/subtitle_table_model.hpp>

#include <QHeaderView>
#include <QString>
#include <QTableView>

#include <memory>
#include <utility>

namespace subedit::gui {

namespace {

/// What the title bar says: the file name, or that nothing is open.
[[nodiscard]] QString titleFor(const core::Project& project) {
    const core::SourceFile& source = project.sourceFile();
    if (!source.path.has_value())
        return QStringLiteral("subedit");

    return QString::fromStdString(source.path.value().filename().string()) +
           QStringLiteral(" — subedit");
}

} // namespace

MainWindow::MainWindow(core::Project project, QWidget* parent)
    : QMainWindow(parent), m_table(new QTableView{this}) {
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
}

MainWindow::~MainWindow() = default;

} // namespace subedit::gui
