// Sorting the subtitles by their start from the window: the action, and the
// proposal made when a file is opened out of order — issue #697.

#include <subedit/core/format/project_file.hpp>
#include <subedit/core/io/in_memory_file_system.hpp>
#include <subedit/gui/diagnostics_button.hpp>
#include <subedit/gui/main_window.hpp>
#include <subedit/gui/sort_proposal_dialog.hpp>

#include <QAbstractItemModel>
#include <QAction>
#include <QDialog>
#include <QEvent>
#include <QItemSelectionModel>
#include <QListWidget>
#include <QObject>
#include <QProgressBar>
#include <QStatusBar>
#include <QString>
#include <QTableView>
#include <catch2/catch_test_macros.hpp>

#include <string>
#include <utility>
#include <vector>

#include "fake_prompts.hpp"

namespace {

using subedit::core::InMemoryFileSystem;
using subedit::core::OpenedFile;
using subedit::core::openProject;
using subedit::gui::MainWindow;
using subedit::gui::SortProposalDialog;
using subedit::test::FakePrompts;

/// Three subtitles, the last of which starts before the other two.
constexpr const char* kUnsorted = "1\n00:00:05,000 --> 00:00:06,000\nCinq.\n\n"
                                  "2\n00:00:07,000 --> 00:00:08,000\nSept.\n\n"
                                  "3\n00:00:01,000 --> 00:00:02,000\nUn.\n\n";

constexpr const char* kSorted = "1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n"
                                "2\n00:00:05,000 --> 00:00:06,000\nCinq.\n\n";

constexpr const char* kAlone = "1\n00:00:01,000 --> 00:00:02,000\nSeul.\n\n";

[[nodiscard]] OpenedFile read(const InMemoryFileSystem& files, const char* name) {
    auto opened = openProject(files, name);
    REQUIRE(opened.has_value());
    return std::move(*opened);
}

[[nodiscard]] std::string textAt(const MainWindow& window, int row) {
    return window.table()
        ->model()
        ->data(window.table()->model()->index(row, 4), Qt::DisplayRole)
        .toString()
        .toStdString();
}

} // namespace

TEST_CASE("sorting puts the subtitles in order of their start, and can be undone",
          "[gui][GUI-SORT-01]") {
    InMemoryFileSystem files;
    files.addFile("film.srt", kUnsorted);
    FakePrompts prompts;
    MainWindow window{files, read(files, "film.srt"), prompts};
    window.show();
    REQUIRE(window.sortAction()->isEnabled());

    window.sortAction()->trigger();

    CHECK(textAt(window, 0) == "Un.");
    CHECK(textAt(window, 1) == "Cinq.");
    CHECK(textAt(window, 2) == "Sept.");
    CHECK(window.statusBar()->currentMessage().toStdString() == "3 subtitles moved");

    window.undoAction()->trigger();
    CHECK(textAt(window, 0) == "Cinq.");
    CHECK(textAt(window, 2) == "Un.");
}

TEST_CASE("sorting a project already in order says so and records nothing", "[gui][GUI-SORT-01]") {
    InMemoryFileSystem files;
    files.addFile("film.srt", kSorted);
    FakePrompts prompts;
    MainWindow window{files, read(files, "film.srt"), prompts};
    window.show();

    window.sortAction()->trigger();

    CHECK(window.statusBar()->currentMessage().toStdString() == "already in order");
    CHECK_FALSE(window.undoAction()->isEnabled());
}

TEST_CASE("one subtitle has nothing to sort", "[gui][GUI-SORT-01]") {
    InMemoryFileSystem files;
    files.addFile("film.srt", kAlone);
    FakePrompts prompts;
    MainWindow window{files, read(files, "film.srt"), prompts};
    window.show();

    CHECK_FALSE(window.sortAction()->isEnabled());
}

TEST_CASE("a file opened out of order offers to be sorted", "[gui][GUI-SORT-02]") {
    InMemoryFileSystem files;
    files.addFile("film.srt", kSorted);
    files.addFile("desordre.srt", kUnsorted);

    SECTION("a yes sorts it, undoably") {
        FakePrompts prompts;
        prompts.nextRun = true;
        prompts.nextFileToOpen = "desordre.srt";
        MainWindow window{files, read(files, "film.srt"), prompts};
        window.show();

        window.openAction()->trigger();

        CHECK(prompts.runAsked == 1);
        CHECK(textAt(window, 0) == "Un.");
        window.undoAction()->trigger();
        CHECK(textAt(window, 0) == "Cinq.");
    }

    SECTION("a no keeps the file as it is") {
        FakePrompts prompts;
        prompts.nextRun = false;
        prompts.nextFileToOpen = "desordre.srt";
        MainWindow window{files, read(files, "film.srt"), prompts};
        window.show();

        window.openAction()->trigger();

        CHECK(prompts.runAsked == 1);
        CHECK(textAt(window, 0) == "Cinq.");
    }

    SECTION("a file in order is not asked about") {
        files.addFile("range.srt", kSorted);
        FakePrompts prompts;
        prompts.nextRun = true;
        prompts.nextFileToOpen = "range.srt";
        MainWindow window{files, read(files, "desordre.srt"), prompts};
        window.show();

        window.openAction()->trigger();

        CHECK(prompts.runAsked == 0);
    }
}

TEST_CASE("the proposal counts what is out of place", "[gui][GUI-SORT-02]") {
    const SortProposalDialog one{1};
    const SortProposalDialog several{3};

    CHECK(one.message().toStdString().find("1 subtitle out of place") != std::string::npos);
    CHECK(several.message().toStdString().find("3 subtitles out of place") != std::string::npos);
}

TEST_CASE("the diagnostics list the subtitles that are out of order", "[gui][GUI-SORT-03]") {
    InMemoryFileSystem files;
    files.addFile("film.srt", kUnsorted);
    FakePrompts prompts;
    MainWindow window{files, read(files, "film.srt"), prompts};
    window.show();

    // Subtitle 3 starts before the one above it ends and starts: two kinds, each
    // counted, in the same words as the report of the command line.
    REQUIRE(window.diagnostics()->isVisible());
    CHECK(window.diagnostics()->count() == 2);
    CHECK(window.diagnostics()->lineAt(0).toStdString() ==
          "subtitle starts before the previous one ends: 1");
    CHECK(window.diagnostics()->lineAt(1).toStdString() ==
          "subtitle starts before the previous one starts: 1");
}

TEST_CASE("choosing an anomaly in the list goes to its subtitle", "[gui][GUI-SORT-03]") {
    InMemoryFileSystem files;
    files.addFile("film.srt", kUnsorted);
    FakePrompts prompts;
    MainWindow window{files, read(files, "film.srt"), prompts};
    window.show();

    window.diagnostics()->chooseLine(1);

    REQUIRE(window.table()->selectionModel()->selectedRows().size() == 1);
    CHECK(window.table()->selectionModel()->selectedRows().front().row() == 2);

    // And by a click on the line, which is what a user does.
    window.table()->selectionModel()->clearSelection();
    auto* list = window.diagnostics()->popup()->findChild<QListWidget*>();
    REQUIRE(list != nullptr);
    emit list->itemClicked(list->item(0));
    REQUIRE(window.table()->selectionModel()->selectedRows().size() == 1);
    CHECK(window.table()->selectionModel()->selectedRows().front().row() == 2);

    // A line that is not an anomaly, or no line at all, goes nowhere.
    window.table()->selectionModel()->clearSelection();
    window.diagnostics()->chooseLine(99);
    CHECK(window.table()->selectionModel()->selectedRows().isEmpty());
}

TEST_CASE("the list follows the corrections, and goes when nothing is left", "[gui][GUI-SORT-03]") {
    InMemoryFileSystem files;
    files.addFile("film.srt", kUnsorted);
    FakePrompts prompts;
    MainWindow window{files, read(files, "film.srt"), prompts};
    window.show();
    REQUIRE(window.diagnostics()->isVisible());

    window.sortAction()->trigger();

    CHECK_FALSE(window.diagnostics()->isVisible());
    CHECK(window.diagnostics()->count() == 0);

    // And back with the undo.
    window.undoAction()->trigger();
    CHECK(window.diagnostics()->isVisible());
    CHECK(window.diagnostics()->count() == 2);
}

TEST_CASE("the list counts what repeats instead of listing it", "[gui][GUI-SORT-03]") {
    // Two subtitles out of place, the first at row 1 and the second at row 3.
    InMemoryFileSystem files;
    files.addFile("film.srt",
                  "1\n00:00:09,000 --> 00:00:10,000\nA.\n\n"
                  "2\n00:00:01,000 --> 00:00:02,000\nB.\n\n"
                  "3\n00:00:11,000 --> 00:00:12,000\nC.\n\n"
                  "4\n00:00:03,000 --> 00:00:04,000\nD.\n\n");
    FakePrompts prompts;
    MainWindow window{files, read(files, "film.srt"), prompts};
    window.show();

    REQUIRE(window.diagnostics()->count() == 2);
    CHECK(window.diagnostics()->lineAt(0).toStdString() ==
          "subtitle starts before the previous one ends: 2");

    // A click goes to the first subtitle with that kind.
    window.diagnostics()->chooseLine(0);
    REQUIRE(window.table()->selectionModel()->selectedRows().size() == 1);
    CHECK(window.table()->selectionModel()->selectedRows().front().row() == 1);
}

namespace {

/// Notes when a widget is shown and hidden, in order.
class VisibilityLog final : public QObject {

public:
    std::vector<bool> changes;

protected:
    bool eventFilter(QObject* watched, QEvent* event) override {
        if (event->type() == QEvent::Show)
            changes.push_back(true);
        else if (event->type() == QEvent::Hide)
            changes.push_back(false);
        return QObject::eventFilter(watched, event);
    }
};

} // namespace

TEST_CASE("a bar says the sort is under way, and goes with it", "[gui][GUI-SORT-04]") {
    InMemoryFileSystem files;
    files.addFile("film.srt", kUnsorted);
    FakePrompts prompts;
    MainWindow window{files, read(files, "film.srt"), prompts};
    window.show();
    REQUIRE(window.busyBar() != nullptr);
    CHECK_FALSE(window.busyBar()->isVisible());
    VisibilityLog log;
    window.busyBar()->installEventFilter(&log);

    window.sortAction()->trigger();

    CHECK(log.changes == std::vector<bool>{true, false});
    CHECK_FALSE(window.busyBar()->isVisible());
    // An indeterminate bar: the sort reports no fraction.
    CHECK(window.busyBar()->maximum() == 0);
}
