#include <subedit/gui/subtitle_table.hpp>
#include <subedit/gui/subtitle_table_model.hpp>

#include <QAbstractItemModel>
#include <QEvent>
#include <QHeaderView>
#include <QModelIndex>
#include <QString>

#include <algorithm>
#include <array>

namespace subedit::gui {

namespace {

/// The columns whose text can run over several lines.
constexpr std::array kTextColumns{static_cast<int>(SubtitleTableModel::Text),
                                  static_cast<int>(SubtitleTableModel::Translation)};

/// How many lines a cell shows: its line breaks and one.
[[nodiscard]] int linesOf(const QAbstractItemModel& model, int row, int column) {
    const QString text = model.index(row, column).data(Qt::DisplayRole).toString();
    return static_cast<int>(text.count(QLatin1Char('\n'))) + 1;
}

} // namespace

void SubtitleTable::setModel(QAbstractItemModel* model) {
    QTableView::setModel(model);
    if (model == nullptr)
        return;

    // Rows are sized by hand: see the declaration.
    verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);

    connect(model, &QAbstractItemModel::modelReset, this, [this] { refreshRowHeights(); });
    connect(model, &QAbstractItemModel::layoutChanged, this, [this] { refreshRowHeights(); });
    connect(model,
            &QAbstractItemModel::rowsInserted,
            this,
            [this](const QModelIndex&, int first, int last) { fitRows(first, last); });
    connect(model,
            &QAbstractItemModel::dataChanged,
            this,
            [this](const QModelIndex& top, const QModelIndex& bottom) {
                // Only a text can change how many lines a row has.
                const bool text = std::ranges::any_of(kTextColumns, [&](int column) {
                    return top.column() <= column && column <= bottom.column();
                });
                if (text)
                    fitRows(top.row(), bottom.row());
            });
    refreshRowHeights();
}

void SubtitleTable::refreshRowHeights() {
    if (model() == nullptr)
        return;

    m_shownColumns = 0;
    for (const int column : kTextColumns) {
        if (!isColumnHidden(column))
            m_shownColumns |= 1U << column;
    }
    m_heights.clear();
    fitRows(0, model()->rowCount() - 1);
}

void SubtitleTable::changeEvent(QEvent* event) {
    QTableView::changeEvent(event);
    if (event->type() == QEvent::FontChange || event->type() == QEvent::StyleChange)
        refreshRowHeights();
}

void SubtitleTable::fitRows(int first, int last) {
    QHeaderView* header = verticalHeader();
    const QAbstractItemModel& rows = *model();
    last = std::min(last, rows.rowCount() - 1);

    for (int row = std::max(first, 0); row <= last; ++row) {
        int lines = 1;
        for (const int column : kTextColumns) {
            if (!isColumnHidden(column))
                lines = std::max(lines, linesOf(rows, row, column));
        }
        const int height = heightOfLines(lines, row);
        if (header->sectionSize(row) != height)
            header->resizeSection(row, height);
    }
}

int SubtitleTable::heightOfLines(int lines, int row) {
    const auto known = std::ranges::find(m_heights, lines, &std::pair<int, int>::first);
    if (known != m_heights.end())
        return known->second;

    // The row is made to be the size the delegate wants, once: with the header
    // in `Fixed`, `sizeHintForRow` answers without resizing anything.
    const int measured = sizeHintForRow(row);
    m_heights.emplace_back(lines, measured);
    return measured;
}

} // namespace subedit::gui
