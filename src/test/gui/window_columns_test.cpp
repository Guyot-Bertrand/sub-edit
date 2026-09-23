// The columns of the table: taken away, put back, moved — issue #442,
// `GUI-TABLE-03`.
//
// What the settings file makes of them is proved in the core; what is under
// test here is the window: the entries of `View ▸ Columns`, the cell that
// leaves a column as it goes, the header one drags, and a session that finds
// the table as the previous one left it.

#include <subedit/core/config/settings.hpp>
#include <subedit/core/format/project_file.hpp>
#include <subedit/core/io/in_memory_file_system.hpp>
#include <subedit/gui/invocation.hpp>
#include <subedit/gui/main_window.hpp>
#include <subedit/gui/subtitle_table.hpp>

#include <QAbstractItemModel>
#include <QAction>
#include <QHeaderView>
#include <QItemSelectionModel>
#include <QTabBar>
#include <catch2/catch_test_macros.hpp>

#include <cstddef>
#include <sstream>
#include <utility>
#include <vector>

#include "fake_prompts.hpp"

namespace {

using subedit::core::InMemoryFileSystem;
using subedit::core::openProject;
using subedit::core::Settings;
using subedit::core::TableColumn;
using subedit::gui::MainWindow;
using subedit::test::FakePrompts;

constexpr int kNumber = 0;
constexpr int kStart = 1;
constexpr int kDuration = 3;
constexpr int kText = 4;

constexpr const char* kTwo = "1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n"
                             "2\n00:00:03,000 --> 00:00:04,000\nDeux.\n\n";

[[nodiscard]] InMemoryFileSystem filesystem() {
    InMemoryFileSystem files;
    files.addFile("premier.srt", kTwo);
    files.addFile("second.srt", kTwo);
    return files;
}

[[nodiscard]] subedit::core::OpenedFile fileIn(const InMemoryFileSystem& files, const char* path) {
    auto opened = openProject(files, path);
    REQUIRE(opened.has_value());
    return std::move(*opened);
}

void makeCurrent(const MainWindow& window, int row, int column) {
    window.table()->selectionModel()->setCurrentIndex(window.table()->model()->index(row, column),
                                                      QItemSelectionModel::ClearAndSelect |
                                                          QItemSelectionModel::Rows);
}

/// The logical columns, in the order the header shows them.
[[nodiscard]] std::vector<int> shownOrder(const MainWindow& window) {
    const QHeaderView* header = window.table()->horizontalHeader();
    std::vector<int> order;
    order.reserve(static_cast<std::size_t>(header->count()));
    for (int visual = 0; visual < header->count(); ++visual)
        order.push_back(header->logicalIndex(visual));
    return order;
}

} // namespace

TEST_CASE("every column but the text has an entry, ticked", "[gui][GUI-TABLE-03]") {
    InMemoryFileSystem files = filesystem();
    FakePrompts prompts;
    MainWindow window{files, fileIn(files, "premier.srt"), prompts};
    window.show();

    for (const TableColumn column :
         {TableColumn::Number, TableColumn::Start, TableColumn::End, TableColumn::Duration}) {
        REQUIRE(window.columnAction(column) != nullptr);
        CHECK(window.columnAction(column)->isCheckable());
        CHECK(window.columnAction(column)->isChecked());
        CHECK(window.columnAction(column)->isEnabled());
    }
    CHECK(window.columnAction(TableColumn::Translation) == window.translationColumnAction());
    CHECK(window.columnAction(TableColumn::Text) == nullptr);
}

TEST_CASE("unticking an entry takes its column away, ticking puts it back", "[gui][GUI-TABLE-03]") {
    InMemoryFileSystem files = filesystem();
    FakePrompts prompts;
    MainWindow window{files, fileIn(files, "premier.srt"), prompts};
    window.show();

    window.columnAction(TableColumn::Start)->setChecked(false);
    CHECK(window.table()->isColumnHidden(kStart));
    CHECK_FALSE(window.table()->isColumnHidden(kNumber));

    window.columnAction(TableColumn::Start)->setChecked(true);
    CHECK_FALSE(window.table()->isColumnHidden(kStart));
}

TEST_CASE("the current cell leaves a column that goes, for the text of its row",
          "[gui][GUI-TABLE-03]") {
    InMemoryFileSystem files = filesystem();
    FakePrompts prompts;
    MainWindow window{files, fileIn(files, "premier.srt"), prompts};
    window.show();
    makeCurrent(window, 1, kDuration);

    window.columnAction(TableColumn::Duration)->setChecked(false);

    CHECK(window.table()->currentIndex().row() == 1);
    CHECK(window.table()->currentIndex().column() == kText);
    CHECK(window.table()->selectionModel()->isRowSelected(1));
}

TEST_CASE("a column taken away stays away in every tab", "[gui][GUI-TABLE-03]") {
    InMemoryFileSystem files = filesystem();
    FakePrompts prompts;
    MainWindow window{files, fileIn(files, "premier.srt"), prompts};
    window.show();
    window.columnAction(TableColumn::Start)->setChecked(false);

    prompts.nextFileToOpen = "second.srt";
    window.openAction()->trigger();
    CHECK(window.table()->isColumnHidden(kStart));

    window.tabBar()->setCurrentIndex(0);
    CHECK(window.table()->isColumnHidden(kStart));
}

TEST_CASE("the header's columns can be dragged, and the order holds across tabs",
          "[gui][GUI-TABLE-03]") {
    InMemoryFileSystem files = filesystem();
    FakePrompts prompts;
    MainWindow window{files, fileIn(files, "premier.srt"), prompts};
    window.show();
    QHeaderView* header = window.table()->horizontalHeader();
    CHECK(header->sectionsMovable());

    // What a drag of the text to the front does.
    header->moveSection(header->visualIndex(kText), 0);
    prompts.nextFileToOpen = "second.srt";
    window.openAction()->trigger();

    CHECK(shownOrder(window) == std::vector<int>{4, 0, 1, 2, 3, 5});
    CHECK(window.settings().columnOrder == std::vector<TableColumn>{TableColumn::Text,
                                                                    TableColumn::Number,
                                                                    TableColumn::Start,
                                                                    TableColumn::End,
                                                                    TableColumn::Duration,
                                                                    TableColumn::Translation});
}

TEST_CASE("the default order and nothing hidden say nothing in the settings",
          "[gui][GUI-TABLE-03]") {
    InMemoryFileSystem files = filesystem();
    FakePrompts prompts;
    MainWindow window{files, fileIn(files, "premier.srt"), prompts};
    window.show();

    const Settings said = window.settings();

    CHECK(said.columnOrder.empty());
    CHECK(said.hiddenColumns.empty());
}

TEST_CASE("the settings given are the order and the columns the table takes",
          "[gui][GUI-TABLE-03]") {
    InMemoryFileSystem files = filesystem();
    FakePrompts prompts;
    MainWindow window{files, fileIn(files, "premier.srt"), prompts};
    window.show();

    Settings settings;
    settings.columnOrder = {TableColumn::Number,
                            TableColumn::Text,
                            TableColumn::Start,
                            TableColumn::End,
                            TableColumn::Duration,
                            TableColumn::Translation};
    settings.hiddenColumns = {TableColumn::End, TableColumn::Duration};
    window.applySettings(settings);

    CHECK(shownOrder(window) == std::vector<int>{0, 4, 1, 2, 3, 5});
    CHECK(window.table()->isColumnHidden(2));
    CHECK(window.table()->isColumnHidden(kDuration));
    CHECK_FALSE(window.columnAction(TableColumn::End)->isChecked());
    CHECK(window.columnAction(TableColumn::Start)->isChecked());
}

TEST_CASE("a session finds the columns as the previous one left them", "[gui][GUI-TABLE-03]") {
    InMemoryFileSystem files = filesystem();
    FakePrompts prompts;
    std::ostringstream errors;
    constexpr const char* kPath = "/config/subedit/settings.conf";

    {
        MainWindow first{files, fileIn(files, "premier.srt"), prompts};
        first.show();
        first.applySettings(Settings{.columnWidths = {45, 125, 125, 125}});
        QHeaderView* header = first.table()->horizontalHeader();
        header->moveSection(header->visualIndex(kDuration), 1);
        first.columnAction(TableColumn::Start)->setChecked(false);

        subedit::gui::writeUserSettings(files, kPath, first.settings(), errors);
    }

    MainWindow second{files, fileIn(files, "premier.srt"), prompts};
    second.applySettings(subedit::gui::readUserSettings(files, kPath, errors));
    second.show();

    CHECK(shownOrder(second) == std::vector<int>{0, 3, 1, 2, 4, 5});
    CHECK(second.table()->isColumnHidden(kStart));
    // **The width of a hidden column is kept, not its zero**: the file refuses
    // a width of zero, and the whole line would have been lost with it.
    CHECK(errors.str().empty());
    second.columnAction(TableColumn::Start)->setChecked(true);
    CHECK(second.table()->columnWidth(kStart) == 125);
}
