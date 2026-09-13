// Merging and splitting from the window — issue #380.
//
// The rules of both live in the core and are tested there. What is under test
// here is what the window adds: when the entries are out, what is selected
// after the gesture, and that one undo takes the whole of it back.

#include <subedit/core/format/project_file.hpp>
#include <subedit/core/io/in_memory_file_system.hpp>
#include <subedit/gui/main_window.hpp>

#include <QAbstractItemModel>
#include <QAction>
#include <QItemSelectionModel>
#include <QMenu>
#include <QMenuBar>
#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <string>
#include <utility>
#include <vector>

#include "fake_prompts.hpp"

namespace {

using subedit::core::InMemoryFileSystem;
using subedit::core::OpenedFile;
using subedit::core::openProject;
using subedit::gui::MainWindow;
using subedit::test::FakePrompts;

/// Four subtitles, at one, three, five and seven seconds.
constexpr const char* kFour = "1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n"
                              "2\n00:00:03,000 --> 00:00:04,000\nDeux.\n\n"
                              "3\n00:00:05,000 --> 00:00:06,000\nTrois.\n\n"
                              "4\n00:00:07,000 --> 00:00:08,000\nQuatre.\n\n";

[[nodiscard]] InMemoryFileSystem withFour() {
    InMemoryFileSystem files;
    files.addFile("film.srt", kFour);
    return files;
}

[[nodiscard]] OpenedFile fourIn(const InMemoryFileSystem& files) {
    auto opened = openProject(files, "film.srt");
    REQUIRE(opened.has_value());
    return std::move(*opened);
}

/// The columns of the table: number, start, end, duration, text.
constexpr int kStartColumn = 1;
constexpr int kEndColumn = 2;
constexpr int kTextColumn = 4;

[[nodiscard]] std::string cellAt(const MainWindow& window, int row, int column) {
    return window.table()
        ->model()
        ->data(window.table()->model()->index(row, column), Qt::DisplayRole)
        .toString()
        .toStdString();
}

[[nodiscard]] std::string textAt(const MainWindow& window, int row) {
    return cellAt(window, row, kTextColumn);
}

[[nodiscard]] int rowCount(const MainWindow& window) {
    return window.table()->model()->rowCount({});
}

void selectRow(const MainWindow& window, int row) {
    window.table()->selectionModel()->select(window.table()->model()->index(row, 0),
                                             QItemSelectionModel::Select |
                                                 QItemSelectionModel::Rows);
}

[[nodiscard]] std::vector<int> selectedRows(const MainWindow& window) {
    std::vector<int> rows;
    for (const QModelIndex& index : window.table()->selectionModel()->selectedRows())
        rows.push_back(index.row());
    std::ranges::sort(rows);
    return rows;
}

} // namespace

TEST_CASE("merging makes one line of the selection, and undoing gives it back",
          "[gui][GUI-SPLIT-01]") {
    InMemoryFileSystem files = withFour();
    FakePrompts prompts;
    MainWindow window{files, fourIn(files), prompts};
    window.show();
    selectRow(window, 1);
    selectRow(window, 2);
    selectRow(window, 3);

    window.mergeAction()->trigger();

    REQUIRE(rowCount(window) == 2);
    CHECK(textAt(window, 1) == "Deux.\nTrois.\nQuatre.");
    CHECK(cellAt(window, 1, kStartColumn) == "00:00:03,000");
    CHECK(cellAt(window, 1, kEndColumn) == "00:00:08,000");
    CHECK(window.undoAction()->text().toStdString() == "Undo: merging");

    window.undoAction()->trigger();

    REQUIRE(rowCount(window) == 4);
    CHECK(textAt(window, 1) == "Deux.");
    CHECK(textAt(window, 2) == "Trois.");
    CHECK(textAt(window, 3) == "Quatre.");
    CHECK(cellAt(window, 1, kEndColumn) == "00:00:04,000");
    CHECK_FALSE(window.undoAction()->isEnabled());
}

TEST_CASE("splitting makes two lines of one, and undoing gives it back", "[gui][GUI-SPLIT-01]") {
    InMemoryFileSystem files = withFour();
    FakePrompts prompts;
    MainWindow window{files, fourIn(files), prompts};
    window.show();
    selectRow(window, 0);

    window.splitAction()->trigger();

    REQUIRE(rowCount(window) == 5);
    CHECK(textAt(window, 0) == "Un.");
    CHECK(textAt(window, 1).empty());
    CHECK(cellAt(window, 0, kEndColumn) == "00:00:01,500");
    CHECK(cellAt(window, 1, kStartColumn) == "00:00:01,500");
    CHECK(textAt(window, 2) == "Deux.");
    CHECK(window.undoAction()->text().toStdString() == "Undo: splitting");

    window.undoAction()->trigger();

    REQUIRE(rowCount(window) == 4);
    CHECK(cellAt(window, 0, kEndColumn) == "00:00:02,000");
    CHECK(textAt(window, 1) == "Deux.");
    CHECK_FALSE(window.undoAction()->isEnabled());
}

TEST_CASE("merging needs two contiguous lines or more", "[gui][GUI-SPLIT-01]") {
    InMemoryFileSystem files = withFour();
    FakePrompts prompts;
    MainWindow window{files, fourIn(files), prompts};
    window.show();

    CHECK_FALSE(window.mergeAction()->isEnabled());

    selectRow(window, 1);
    CHECK_FALSE(window.mergeAction()->isEnabled());

    // One and three: merging them would swallow two under an overlap.
    selectRow(window, 3);
    CHECK_FALSE(window.mergeAction()->isEnabled());

    selectRow(window, 2);
    CHECK(window.mergeAction()->isEnabled());
}

TEST_CASE("splitting needs exactly one line", "[gui][GUI-SPLIT-01]") {
    InMemoryFileSystem files = withFour();
    FakePrompts prompts;
    MainWindow window{files, fourIn(files), prompts};
    window.show();

    // Nothing selected is not « the whole file » here, as it is for a shift.
    CHECK_FALSE(window.splitAction()->isEnabled());

    selectRow(window, 1);
    CHECK(window.splitAction()->isEnabled());

    selectRow(window, 2);
    CHECK_FALSE(window.splitAction()->isEnabled());
}

TEST_CASE("after a merge, the merged line is selected", "[gui][GUI-SPLIT-01]") {
    // And so splitting it again is one click away.
    InMemoryFileSystem files = withFour();
    FakePrompts prompts;
    MainWindow window{files, fourIn(files), prompts};
    window.show();
    selectRow(window, 2);
    selectRow(window, 3);

    window.mergeAction()->trigger();

    CHECK(selectedRows(window) == std::vector<int>{2});
    CHECK(window.splitAction()->isEnabled());
}

TEST_CASE("after a split, both halves are selected", "[gui][GUI-SPLIT-01]") {
    // What Gaupol does after any insertion — and it is what leaves `Merge
    // Subtitles` ready to take the split back.
    InMemoryFileSystem files = withFour();
    FakePrompts prompts;
    MainWindow window{files, fourIn(files), prompts};
    window.show();
    selectRow(window, 3);

    window.splitAction()->trigger();

    CHECK(selectedRows(window) == std::vector<int>{3, 4});
    REQUIRE(window.mergeAction()->isEnabled());

    window.mergeAction()->trigger();

    REQUIRE(rowCount(window) == 4);
    CHECK(textAt(window, 3) == "Quatre.");
    CHECK(cellAt(window, 3, kEndColumn) == "00:00:08,000");
}

TEST_CASE("a merge the selection does not allow does not enter the history",
          "[gui][GUI-SPLIT-01]") {
    // The second guard, reached by switching the entry back on by hand: a
    // shortcut can find it a fraction of a second too late.
    InMemoryFileSystem files = withFour();
    FakePrompts prompts;
    MainWindow window{files, fourIn(files), prompts};
    window.show();
    selectRow(window, 0);
    selectRow(window, 2);

    window.mergeAction()->setEnabled(true);
    window.mergeAction()->trigger();
    window.splitAction()->setEnabled(true);
    window.splitAction()->trigger();

    CHECK(rowCount(window) == 4);
    CHECK_FALSE(window.undoAction()->isEnabled());
}

TEST_CASE("merging a single line forced on does not enter the history", "[gui][GUI-SPLIT-01]") {
    // One run, but of one line: the window lets it through to the core, which
    // answers with no command at all.
    InMemoryFileSystem files = withFour();
    FakePrompts prompts;
    MainWindow window{files, fourIn(files), prompts};
    window.show();
    selectRow(window, 1);

    window.mergeAction()->setEnabled(true);
    window.mergeAction()->trigger();

    CHECK(rowCount(window) == 4);
    CHECK_FALSE(window.undoAction()->isEnabled());
}

TEST_CASE("merging and splitting live in the Edit menu", "[gui][GUI-SPLIT-01]") {
    InMemoryFileSystem files = withFour();
    FakePrompts prompts;
    const MainWindow window{files, fourIn(files), prompts};

    QMenu* edition = nullptr;
    for (QAction* entry : window.menuBar()->actions()) {
        if (entry->text() == QStringLiteral("&Edit"))
            edition = entry->menu();
    }
    REQUIRE(edition != nullptr);
    CHECK(edition->actions().contains(window.mergeAction()));
    CHECK(edition->actions().contains(window.splitAction()));
}
