#include <subedit/core/config/settings.hpp>
#include <subedit/core/edit/session.hpp>
#include <subedit/core/model/document.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/gui/project_page.hpp>
#include <subedit/gui/subtitle_table_model.hpp>
#include <subedit/gui/table_columns.hpp>

#include <QAction>
#include <QHeaderView>
#include <QItemSelectionModel>
#include <QString>
#include <QTableView>

#include <algorithm>
#include <array>
#include <cstddef>
#include <utility>
#include <vector>

namespace subedit::gui {

// The file names the columns in the model's order, and this converts by that
// order alone.
static_assert(static_cast<int>(core::TableColumn::Number) == SubtitleTableModel::Number);
static_assert(static_cast<int>(core::TableColumn::Duration) == SubtitleTableModel::Duration);
static_assert(static_cast<int>(core::TableColumn::Text) == SubtitleTableModel::Text);
static_assert(static_cast<int>(core::TableColumn::Translation) == SubtitleTableModel::Translation);
static_assert(core::kTableColumnCount == SubtitleTableModel::kColumnCount);

TableColumns::TableColumns(QTableView& table, QObject* owner) : m_table(&table) {
    // The four that may go, beside the translation. `No.` as Gaupol names it:
    // the header's « # » reads badly as a menu entry.
    const std::array<QString, 4> names = {QStringLiteral("&No."),
                                          QStringLiteral("&Start"),
                                          QStringLiteral("&End"),
                                          QStringLiteral("&Duration")};
    for (std::size_t column = 0; column < m_positions.size(); ++column) {
        m_positions.at(column) = new QAction{names.at(column), owner};
        m_positions.at(column)->setCheckable(true);
        m_positions.at(column)->setChecked(true);
    }

    m_translation = new QAction{QStringLiteral("&Translation"), owner};
    m_translation->setCheckable(true);
    m_translation->setChecked(true);
    m_translation->setEnabled(false);
    m_translation->setToolTip(QStringLiteral("Show the translation next to the text"));

    // **The header's columns are dragged into another order** — issue #442.
    // The order belongs to the one table, and so to every tab: a model does
    // not change the number of columns the header counts, so `setModel` keeps
    // it as it keeps the widths.
    m_table->horizontalHeader()->setSectionsMovable(true);
}

QAction* TableColumns::action(core::TableColumn column) const {
    switch (column) {
    case core::TableColumn::Number:
    case core::TableColumn::Start:
    case core::TableColumn::End:
    case core::TableColumn::Duration:
        return m_positions.at(static_cast<std::size_t>(column));
    case core::TableColumn::Translation:
        return m_translation;
    case core::TableColumn::Text:
        return nullptr;
    }
    std::unreachable();
}

std::array<QAction*, 5> TableColumns::entries() const {
    return {
        m_positions.at(0), m_positions.at(1), m_positions.at(2), m_positions.at(3), m_translation};
}

void TableColumns::setPositionColumnShown(int column, bool shown) {
    const auto at = static_cast<std::size_t>(column);
    if (!shown && !m_table->isColumnHidden(column))
        m_hiddenWidths.at(at) = m_table->columnWidth(column);
    m_table->setColumnHidden(column, !shown);
}

void TableColumns::refresh(const ProjectPage& page) {
    for (std::size_t column = 0; column < m_positions.size(); ++column)
        setPositionColumnShown(static_cast<int>(column), m_positions.at(column)->isChecked());

    // **A translation is a fact of the project, and showing it is the user's
    // choice**: the column is there when both hold. The entry says which of the
    // two is missing — out when the project has nothing to show, unchecked when
    // the user took it away.
    const bool hasTranslation = page.session->project().translationFile().has_value();
    m_translation->setEnabled(hasTranslation);
    m_table->setColumnHidden(SubtitleTableModel::Translation,
                             !(hasTranslation && m_translation->isChecked()));

    // **The cell goes to the text of its row**, and the selection stays: a
    // current cell in a column nobody can see is one a keystroke would edit
    // blind — the rule `targetDocument` applies to a hidden translation, made
    // true of the cell itself.
    const QModelIndex current = m_table->currentIndex();
    if (current.isValid() && m_table->isColumnHidden(current.column()))
        m_table->selectionModel()->setCurrentIndex(
            m_table->model()->index(current.row(), SubtitleTableModel::Text),
            QItemSelectionModel::NoUpdate);
}

core::Document TableColumns::targetDocument() const {
    // **A column one cannot see is not one a cell is current in**: the view
    // keeps the current index where it was when the column goes, and an
    // operation must not reach a text nobody is looking at.
    const bool inTranslation =
        m_table->currentIndex().column() == SubtitleTableModel::Translation && translationShown();
    return inTranslation ? core::Document::Translation : core::Document::Main;
}

bool TableColumns::translationShown() const {
    return !m_table->isColumnHidden(SubtitleTableModel::Translation);
}

void TableColumns::apply(const core::Settings& settings) {
    // The first four columns only: the last one shown takes what the others
    // leave, and giving it a width would do nothing. The reader has already
    // refused a different count, so arriving here with anything else would mean
    // the two no longer speak of the same columns.
    if (settings.columnWidths.size() == core::kColumnWidthCount) {
        for (std::size_t column = 0; column < core::kColumnWidthCount; ++column)
            m_table->setColumnWidth(static_cast<int>(column), settings.columnWidths[column]);
    }

    // **The order after the widths, the hidden columns after the order** — the
    // widths are those of columns still shown, and a column hidden keeps the
    // width it had when it went.
    QHeaderView* header = m_table->horizontalHeader();
    if (settings.columnOrder.size() == core::kTableColumnCount) {
        for (std::size_t visual = 0; visual < core::kTableColumnCount; ++visual) {
            const int logical = static_cast<int>(settings.columnOrder.at(visual));
            header->moveSection(header->visualIndex(logical), static_cast<int>(visual));
        }
    }
    for (std::size_t column = 0; column < m_positions.size(); ++column) {
        const bool hidden =
            std::ranges::find(settings.hiddenColumns, static_cast<core::TableColumn>(column)) !=
            settings.hiddenColumns.end();
        m_positions.at(column)->setChecked(!hidden);
    }
}

void TableColumns::write(core::Settings& settings) const {
    settings.columnWidths.clear();
    settings.columnWidths.reserve(core::kColumnWidthCount);
    for (std::size_t column = 0; column < core::kColumnWidthCount; ++column) {
        const int logical = static_cast<int>(column);
        settings.columnWidths.push_back(m_table->isColumnHidden(logical)
                                            ? m_hiddenWidths.at(column)
                                            : m_table->columnWidth(logical));
    }

    // Written only when it differs from the default order: a file that says
    // the default in so many words would stop following it if it changed.
    const QHeaderView* header = m_table->horizontalHeader();
    std::vector<core::TableColumn> order;
    order.reserve(core::kTableColumnCount);
    bool moved = false;
    for (int visual = 0; visual < SubtitleTableModel::kColumnCount; ++visual) {
        const int logical = header->logicalIndex(visual);
        moved = moved || logical != visual;
        order.push_back(static_cast<core::TableColumn>(logical));
    }
    settings.columnOrder = moved ? std::move(order) : std::vector<core::TableColumn>{};

    settings.hiddenColumns.clear();
    for (std::size_t column = 0; column < m_positions.size(); ++column) {
        if (!m_positions.at(column)->isChecked())
            settings.hiddenColumns.push_back(static_cast<core::TableColumn>(column));
    }
}

} // namespace subedit::gui
