// The table sets the height of its own rows, from the number of lines of their
// text. These cases give it a plain model with the six columns of the real one:
// the table knows columns by position and nothing else.

#include <subedit/gui/subtitle_table.hpp>
#include <subedit/gui/subtitle_table_model.hpp>

#include <QCoreApplication>
#include <QFont>
#include <QHeaderView>
#include <QStandardItemModel>
#include <QString>
#include <catch2/catch_test_macros.hpp>

namespace {

using subedit::gui::SubtitleTable;
using subedit::gui::SubtitleTableModel;

constexpr int kTextColumn = SubtitleTableModel::Text;
constexpr int kTranslationColumn = SubtitleTableModel::Translation;

/// A model of the table's six columns, and a text for each row.
struct Rows {
    QStandardItemModel model{0, SubtitleTableModel::kColumnCount};

    void add(const QString& text, const QString& translation = {}) {
        const int row = model.rowCount();
        model.insertRow(row);
        model.setData(model.index(row, kTextColumn), text);
        model.setData(model.index(row, kTranslationColumn), translation);
    }
};

} // namespace

TEST_CASE("a row is as tall as its lines, and the rows are not measured one by one",
          "[gui][GUI-OPEN-01]") {
    Rows rows;
    rows.add(QStringLiteral("One."));
    rows.add(QStringLiteral("One.\nTwo."));
    rows.add(QStringLiteral("One.\nTwo.\nThree."));
    rows.add(QStringLiteral("Again."));
    SubtitleTable table;
    table.setModel(&rows.model);
    table.show();

    CHECK(table.verticalHeader()->sectionResizeMode(0) == QHeaderView::Fixed);
    CHECK(table.rowHeight(1) > table.rowHeight(0));
    CHECK(table.rowHeight(2) > table.rowHeight(1));
    CHECK(table.rowHeight(3) == table.rowHeight(0));
}

TEST_CASE("a row follows the text it is edited to, and the rows that come", "[gui][GUI-EDIT-01]") {
    Rows rows;
    rows.add(QStringLiteral("One."));
    rows.add(QStringLiteral("Two."));
    SubtitleTable table;
    table.setModel(&rows.model);
    table.show();
    const int one = table.rowHeight(0);

    // A column that holds no text changes nothing.
    rows.model.setData(rows.model.index(0, 0), QStringLiteral("9"));
    CHECK(table.rowHeight(0) == one);

    rows.model.setData(rows.model.index(0, kTextColumn), QStringLiteral("One.\nAnd a half."));
    CHECK(table.rowHeight(0) > one);
    CHECK(table.rowHeight(1) == one);

    // A row inserted between them is given its height, and the others keep theirs.
    rows.model.insertRow(1);
    rows.model.setData(rows.model.index(1, kTextColumn), QStringLiteral("a\nb\nc"));
    CHECK(table.rowHeight(1) > table.rowHeight(0));
    CHECK(table.rowHeight(2) == one);
}

TEST_CASE("a row is as tall as the texts that are shown", "[gui][GUI-TRANS-05]") {
    Rows rows;
    rows.add(QStringLiteral("One."), QStringLiteral("Un.\nDeux.\nTrois."));
    SubtitleTable table;
    table.setModel(&rows.model);
    table.setColumnHidden(kTranslationColumn, true);
    table.refreshRowHeights();
    table.show();
    const int hidden = table.rowHeight(0);

    table.setColumnHidden(kTranslationColumn, false);
    table.refreshRowHeights();

    CHECK(table.rowHeight(0) > hidden);

    table.setColumnHidden(kTranslationColumn, true);
    table.refreshRowHeights();
    CHECK(table.rowHeight(0) == hidden);
}

TEST_CASE("a new font measures the rows again, and a reset gives them back", "[gui][GUI-OPEN-01]") {
    Rows rows;
    rows.add(QStringLiteral("One.\nTwo."));
    SubtitleTable table;
    table.setModel(&rows.model);
    table.show();
    const int before = table.rowHeight(0);

    QFont large = table.font();
    large.setPointSize(large.pointSize() * 3);
    table.setFont(large);
    QCoreApplication::processEvents();
    CHECK(table.rowHeight(0) > before);

    rows.model.setRowCount(0);
    rows.add(QStringLiteral("Single."));
    CHECK(table.rowHeight(0) > 0);

    // Without a model there is nothing to measure, and nothing goes wrong.
    table.setModel(nullptr);
    table.refreshRowHeights();
}
