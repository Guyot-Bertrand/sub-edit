// The columns of the table, without the window — ADR 0034, issue #460.
//
// `TableColumns` knows a table and nothing else: these cases build one, give it
// a page, and read what it shows. What the window does around it — the menu,
// the target in the status bar — is proved in `window_columns_test.cpp`.

#include <subedit/core/config/settings.hpp>
#include <subedit/core/model/document.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/model/source_file.hpp>
#include <subedit/core/model/subtitle.hpp>
#include <subedit/core/time/timestamp.hpp>
#include <subedit/gui/project_page.hpp>
#include <subedit/gui/subtitle_table_model.hpp>
#include <subedit/gui/table_columns.hpp>

#include <QAction>
#include <QHeaderView>
#include <QItemSelectionModel>
#include <QObject>
#include <QTableView>
#include <catch2/catch_test_macros.hpp>

#include <filesystem>
#include <memory>
#include <utility>
#include <vector>

namespace {

using subedit::core::Document;
using subedit::core::Project;
using subedit::core::Settings;
using subedit::core::TableColumn;
using subedit::gui::ProjectPage;
using subedit::gui::SubtitleTableModel;
using subedit::gui::TableColumns;

[[nodiscard]] Project twoSubtitles(bool withTranslation) {
    Project project;
    project.setSubtitles({
        subedit::core::Subtitle{.start = subedit::core::Timestamp::fromMilliseconds(0),
                                .end = subedit::core::Timestamp::fromMilliseconds(1000),
                                .mainText = "Un."},
        subedit::core::Subtitle{.start = subedit::core::Timestamp::fromMilliseconds(2000),
                                .end = subedit::core::Timestamp::fromMilliseconds(3000),
                                .mainText = "Deux."},
    });
    if (withTranslation)
        project.setSourceFile(Document::Translation,
                              subedit::core::SourceFile{.path = std::filesystem::path{"a.en.srt"}});
    return project;
}

/// A table on a page, as the window lays them out, and the columns over it.
struct Table {
    explicit Table(bool withTranslation = false)
        : page(ProjectPage::make(twoSubtitles(withTranslation))), columns(view, &owner) {
        view.setModel(page->model.get());
        view.setSelectionModel(page->tableSelection.get());
        view.show();
        columns.refresh(*page);
    }

    QObject owner;
    QTableView view;
    std::unique_ptr<ProjectPage> page;
    TableColumns columns;
};

void makeCurrent(Table& table, int row, int column) {
    table.view.selectionModel()->setCurrentIndex(table.view.model()->index(row, column),
                                                 QItemSelectionModel::ClearAndSelect |
                                                     QItemSelectionModel::Rows);
}

} // namespace

TEST_CASE("the columns offer an entry for every column but the text", "[gui][GUI-TABLE-03]") {
    Table table;

    CHECK(table.columns.action(TableColumn::Text) == nullptr);
    CHECK(table.columns.entries().size() == 5);
    CHECK(table.columns.entries().back() == table.columns.action(TableColumn::Translation));
    CHECK(table.view.horizontalHeader()->sectionsMovable());
}

TEST_CASE("the translation column follows the project and the entry together",
          "[gui][GUI-TRANS-04]") {
    Table without;
    CHECK_FALSE(without.columns.translationShown());
    CHECK_FALSE(without.columns.action(TableColumn::Translation)->isEnabled());

    Table with{true};
    CHECK(with.columns.translationShown());

    with.columns.action(TableColumn::Translation)->setChecked(false);
    with.columns.refresh(*with.page);
    CHECK_FALSE(with.columns.translationShown());
}

TEST_CASE("the target is the translation only in its column, and only while it is shown",
          "[gui][GUI-TRANS-05]") {
    Table table{true};
    makeCurrent(table, 0, SubtitleTableModel::Translation);
    CHECK(table.columns.targetDocument() == Document::Translation);

    makeCurrent(table, 0, SubtitleTableModel::Text);
    CHECK(table.columns.targetDocument() == Document::Main);

    makeCurrent(table, 0, SubtitleTableModel::Translation);
    table.columns.action(TableColumn::Translation)->setChecked(false);
    table.columns.refresh(*table.page);
    CHECK(table.columns.targetDocument() == Document::Main);
}

TEST_CASE("a hidden column hands the current cell to the text, and keeps its width",
          "[gui][GUI-TABLE-03]") {
    Table table;
    table.view.setColumnWidth(SubtitleTableModel::Start, 140);
    makeCurrent(table, 1, SubtitleTableModel::Start);

    table.columns.action(TableColumn::Start)->setChecked(false);
    table.columns.refresh(*table.page);

    CHECK(table.view.isColumnHidden(SubtitleTableModel::Start));
    CHECK(table.view.currentIndex().row() == 1);
    CHECK(table.view.currentIndex().column() == SubtitleTableModel::Text);

    Settings written;
    table.columns.write(written);
    REQUIRE(written.columnWidths.size() == subedit::core::kColumnWidthCount);
    CHECK(written.columnWidths.at(1) == 140);
    CHECK(written.hiddenColumns == std::vector<TableColumn>{TableColumn::Start});
}

TEST_CASE("the settings written are the settings read back", "[gui][GUI-TABLE-03]") {
    Settings settings;
    settings.columnWidths = {40, 130, 120, 110};
    settings.columnOrder = {TableColumn::Text,
                            TableColumn::Number,
                            TableColumn::Start,
                            TableColumn::End,
                            TableColumn::Duration,
                            TableColumn::Translation};
    settings.hiddenColumns = {TableColumn::End};

    Table table;
    table.columns.apply(settings);
    table.columns.refresh(*table.page);

    CHECK(table.view.horizontalHeader()->logicalIndex(0) == SubtitleTableModel::Text);
    CHECK(table.view.isColumnHidden(SubtitleTableModel::End));

    Settings written;
    table.columns.write(written);
    CHECK(written.columnOrder == settings.columnOrder);
    CHECK(written.hiddenColumns == settings.hiddenColumns);
    CHECK(written.columnWidths == settings.columnWidths);
}

TEST_CASE("the default order is written as no order at all", "[gui][GUI-TABLE-03]") {
    Table table;

    Settings written;
    table.columns.write(written);

    CHECK(written.columnOrder.empty());
    CHECK(written.hiddenColumns.empty());
}
