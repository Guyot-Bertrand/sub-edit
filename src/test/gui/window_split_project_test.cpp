// Splitting a project in two, from the window — issue #439, decision D6,
// `GUI-PSPLIT-01`.
//
// The shift, what the new project inherits and the refusal are proved in the
// core; what is under test here is what the window adds: the entry grayed
// under two subtitles, the dialog that asks where to cut, the tab the tail
// opens in, the single entry in the history of the origin, and the refusal
// said aloud.

#include <subedit/core/format/project_file.hpp>
#include <subedit/core/io/in_memory_file_system.hpp>
#include <subedit/gui/main_window.hpp>
#include <subedit/gui/split_project_dialog.hpp>

#include <QAbstractItemModel>
#include <QAction>
#include <QDialog>
#include <QSpinBox>
#include <QTabBar>
#include <catch2/catch_test_macros.hpp>

#include <string>
#include <utility>

#include "fake_prompts.hpp"

namespace {

using subedit::core::InMemoryFileSystem;
using subedit::core::openProject;
using subedit::gui::MainWindow;
using subedit::gui::SplitProjectDialog;
using subedit::test::FakePrompts;

constexpr int kStartColumn = 1;
constexpr int kTextColumn = 4;

constexpr const char* kThree = "1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n"
                               "2\n00:00:03,000 --> 00:00:04,000\nDeux.\n\n"
                               "3\n00:00:05,000 --> 00:00:06,000\nTrois.\n\n";

/// The third subtitle starts before the second ends: cut there, the tail lands
/// at -1 s.
constexpr const char* kOverlapping = "1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n"
                                     "2\n00:00:03,000 --> 00:00:05,000\nDeux.\n\n"
                                     "3\n00:00:04,000 --> 00:00:06,000\nTrois.\n\n";

[[nodiscard]] InMemoryFileSystem filesystem(const char* content = kThree) {
    InMemoryFileSystem files;
    files.addFile("film.srt", content);
    return files;
}

[[nodiscard]] subedit::core::OpenedFile mainOf(const InMemoryFileSystem& files) {
    auto opened = openProject(files, "film.srt");
    REQUIRE(opened.has_value());
    return std::move(*opened);
}

[[nodiscard]] std::string cellAt(const MainWindow& window, int row, int column) {
    return window.table()
        ->model()
        ->data(window.table()->model()->index(row, column), Qt::DisplayRole)
        .toString()
        .toStdString();
}

/// Plays the user who types `number` into the box and accepts.
[[nodiscard]] auto cuttingAt(int number) {
    return [number](QDialog& dialog) {
        dynamic_cast<SplitProjectDialog&>(dialog).subtitleBox()->setValue(number);
    };
}

} // namespace

TEST_CASE("splitting is grayed under two subtitles, and lit from two", "[gui][GUI-PSPLIT-01]") {
    InMemoryFileSystem files = filesystem();
    FakePrompts prompts;
    MainWindow empty{files, subedit::core::OpenedFile{}, prompts};
    empty.show();
    CHECK_FALSE(empty.splitProjectAction()->isEnabled());

    InMemoryFileSystem one;
    one.addFile("film.srt", "1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n");
    MainWindow alone{one, mainOf(one), prompts};
    alone.show();
    CHECK_FALSE(alone.splitProjectAction()->isEnabled());

    MainWindow window{files, mainOf(files), prompts};
    window.show();
    CHECK(window.splitProjectAction()->isEnabled());
}

TEST_CASE("splitting opens the tail in a new tab, shifted, and takes it from the origin",
          "[gui][GUI-PSPLIT-01]") {
    InMemoryFileSystem files = filesystem();
    FakePrompts prompts;
    MainWindow window{files, mainOf(files), prompts};
    window.show();
    prompts.nextRun = true;
    prompts.fill = cuttingAt(3);

    window.splitProjectAction()->trigger();

    REQUIRE(window.tabBar()->count() == 2);
    CHECK(window.tabBar()->currentIndex() == 1);
    REQUIRE(window.table()->model()->rowCount() == 1);
    CHECK(cellAt(window, 0, kTextColumn) == "Trois.");
    // Shifted back by the end of the second subtitle, at 4 s: 5 s becomes 1 s.
    CHECK(cellAt(window, 0, kStartColumn) == "00:00:01,000");

    window.tabBar()->setCurrentIndex(0);
    REQUIRE(window.table()->model()->rowCount() == 2);
    CHECK(cellAt(window, 1, kTextColumn) == "Deux.");
}

TEST_CASE("the new project has no file, and says it is not written", "[gui][GUI-PSPLIT-01]") {
    InMemoryFileSystem files = filesystem();
    FakePrompts prompts;
    MainWindow window{files, mainOf(files), prompts};
    window.show();
    prompts.nextRun = true;
    prompts.fill = cuttingAt(2);

    window.splitProjectAction()->trigger();

    // Modified from birth: it holds subtitles no file has, and closing the
    // window must not lose them without asking.
    CHECK(window.tabBar()->tabText(1).toStdString() == "untitled*");
    // Two documents differ from their files — the origin, which lost its tail,
    // and the new project — so the one question is the list, and cancelling it
    // keeps the window.
    const int asked = prompts.runAsked;
    prompts.fill = nullptr;
    prompts.nextRun = false;
    CHECK_FALSE(window.close());
    CHECK(prompts.runAsked == asked + 1);
}

TEST_CASE("undoing the split gives the origin back, and the new project is not in its history",
          "[gui][GUI-PSPLIT-01]") {
    InMemoryFileSystem files = filesystem();
    FakePrompts prompts;
    MainWindow window{files, mainOf(files), prompts};
    window.show();
    prompts.nextRun = true;
    prompts.fill = cuttingAt(2);
    window.splitProjectAction()->trigger();

    // The new project has a history of its own, and nothing in it.
    CHECK_FALSE(window.undoAction()->isEnabled());

    window.tabBar()->setCurrentIndex(0);
    REQUIRE(window.undoAction()->isEnabled());
    CHECK(window.undoAction()->text().toStdString() == "Undo: splitting the project");
    window.undoAction()->trigger();

    REQUIRE(window.table()->model()->rowCount() == 3);
    CHECK(cellAt(window, 2, kTextColumn) == "Trois.");
    CHECK(window.tabBar()->count() == 2);
}

TEST_CASE("a cut no file could write is refused, naming the subtitle", "[gui][GUI-PSPLIT-01]") {
    InMemoryFileSystem files = filesystem(kOverlapping);
    FakePrompts prompts;
    MainWindow window{files, mainOf(files), prompts};
    window.show();
    prompts.nextRun = true;
    prompts.fill = cuttingAt(3);

    window.splitProjectAction()->trigger();

    REQUIRE(prompts.failures.size() == 1);
    CHECK(prompts.failures.front().find("subtitle 3") != std::string::npos);
    CHECK(window.tabBar()->count() == 1);
    CHECK(window.table()->model()->rowCount() == 3);
    CHECK_FALSE(window.undoAction()->isEnabled());
}

TEST_CASE("cancelling the question splits nothing", "[gui][GUI-PSPLIT-01]") {
    InMemoryFileSystem files = filesystem();
    FakePrompts prompts;
    MainWindow window{files, mainOf(files), prompts};
    window.show();
    prompts.nextRun = false;

    window.splitProjectAction()->trigger();

    CHECK(window.tabBar()->count() == 1);
    CHECK(window.table()->model()->rowCount() == 3);
}
