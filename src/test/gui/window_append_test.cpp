// Appending a file to the end of a project, from the window — issue #434,
// decision D6.
//
// The shift and the crossing of formats are proved in the core; what is under
// test here is what the window adds: the entry grayed on an empty project, the
// file it reads, the rows it selects, and the status bar or the box that says
// what happened.

#include <subedit/core/format/project_file.hpp>
#include <subedit/core/io/in_memory_file_system.hpp>
#include <subedit/core/wording.hpp>
#include <subedit/gui/diagnostics_panel.hpp>
#include <subedit/gui/main_window.hpp>

#include <QAbstractItemModel>
#include <QAction>
#include <QItemSelectionModel>
#include <QStatusBar>
#include <QTableView>
#include <catch2/catch_test_macros.hpp>

#include <string>
#include <utility>
#include <vector>

#include "fake_prompts.hpp"

namespace {

using subedit::core::InMemoryFileSystem;
using subedit::core::openProject;
using subedit::gui::MainWindow;
using subedit::test::FakePrompts;

constexpr int kTextColumn = 4;

constexpr const char* kThree = "1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n"
                               "2\n00:00:03,000 --> 00:00:04,000\nDeux.\n\n"
                               "3\n00:00:05,000 --> 00:00:06,000\nTrois.\n\n";

constexpr const char* kSuite = "1\n00:00:00,000 --> 00:00:01,000\nQuatre.\n\n"
                               "2\n00:00:02,000 --> 00:00:03,000\nCinq.\n\n";

[[nodiscard]] InMemoryFileSystem withThreeAndASuite() {
    InMemoryFileSystem files;
    files.addFile("film.srt", kThree);
    files.addFile("suite.srt", kSuite);
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

[[nodiscard]] std::vector<int> selectedRows(const MainWindow& window) {
    std::vector<int> rows;
    for (const QModelIndex& index : window.table()->selectionModel()->selectedRows())
        rows.push_back(index.row());
    return rows;
}

} // namespace

TEST_CASE("appending is grayed on an empty project, and lit otherwise", "[gui][GUI-APPEND-01]") {
    InMemoryFileSystem files = withThreeAndASuite();
    FakePrompts prompts;
    MainWindow empty{files, subedit::core::OpenedFile{}, prompts};
    empty.show();
    CHECK_FALSE(empty.appendFileAction()->isEnabled());

    MainWindow window{files, mainOf(files), prompts};
    window.show();
    CHECK(window.appendFileAction()->isEnabled());
}

TEST_CASE("appending shifts the file from the end of the last subtitle, and selects it",
          "[gui][GUI-APPEND-01]") {
    InMemoryFileSystem files = withThreeAndASuite();
    FakePrompts prompts;
    MainWindow window{files, mainOf(files), prompts};
    window.show();
    prompts.nextFileToOpen = "suite.srt";

    window.appendFileAction()->trigger();

    REQUIRE(window.table()->model()->rowCount() == 5);
    CHECK(cellAt(window, 3, kTextColumn) == "Quatre.");
    CHECK(cellAt(window, 4, kTextColumn) == "Cinq.");
    CHECK(selectedRows(window) == std::vector<int>{3, 4});

    // Nothing before the append moved.
    CHECK(cellAt(window, 0, kTextColumn) == "Un.");
    CHECK(cellAt(window, 2, kTextColumn) == "Trois.");
}

TEST_CASE("cancelling the file chooser appends nothing", "[gui][GUI-APPEND-01]") {
    InMemoryFileSystem files = withThreeAndASuite();
    FakePrompts prompts;
    MainWindow window{files, mainOf(files), prompts};
    window.show();
    prompts.nextFileToOpen = std::nullopt;

    window.appendFileAction()->trigger();

    CHECK(window.table()->model()->rowCount() == 3);
}

TEST_CASE("a file that will not open is named, and nothing is appended", "[gui][GUI-APPEND-01]") {
    InMemoryFileSystem files = withThreeAndASuite();
    FakePrompts prompts;
    MainWindow window{files, mainOf(files), prompts};
    window.show();
    prompts.nextFileToOpen = "absent.srt";

    window.appendFileAction()->trigger();

    REQUIRE(prompts.failures.size() == 1);
    CHECK(prompts.failures.front() == "absent.srt: does not exist");
    CHECK(window.table()->model()->rowCount() == 3);
}

TEST_CASE("appending a plain SubRip file is said in the status bar", "[gui][GUI-APPEND-01]") {
    InMemoryFileSystem files = withThreeAndASuite();
    FakePrompts prompts;
    MainWindow window{files, mainOf(files), prompts};
    window.show();
    prompts.nextFileToOpen = "suite.srt";

    window.appendFileAction()->trigger();

    CHECK(prompts.outcomes.empty());
    CHECK(window.statusBar()->currentMessage().toStdString() == "appended 2 subtitles");
}

TEST_CASE("appending a file of another format translates its tags, and says so",
          "[gui][GUI-APPEND-01]") {
    InMemoryFileSystem files;
    files.addFile("film.srt", kThree);
    files.addFile(
        "suite.ass",
        "[Script Info]\n"
        "ScriptType: v4.00+\n"
        "\n"
        "[Events]\n"
        "Format: Layer, Start, End, Style, Name, MarginL, MarginR, MarginV, Effect, Text\n"
        R"(Dialogue: 0,0:00:00.00,0:00:01.00,Default,,0000,0000,0000,,{\i1}Quatre{\i0})"
        "\n");
    FakePrompts prompts;
    MainWindow window{files, mainOf(files), prompts};
    window.show();
    prompts.nextFileToOpen = "suite.ass";

    window.appendFileAction()->trigger();

    CHECK(cellAt(window, 3, kTextColumn) == "<i>Quatre</i>");
    // The tag crossed intact; what is lost is the `[Script Info]` header, which
    // a SubRip has nowhere to keep — which is why this is not silent.
    REQUIRE(prompts.outcomes.size() == 1);
    CHECK(prompts.outcomes.front() ==
          "appended 1 subtitle; Advanced SSA into SubRip: the header was dropped");
}

TEST_CASE("undoing an append removes everything it added, in one entry", "[gui][GUI-APPEND-01]") {
    InMemoryFileSystem files = withThreeAndASuite();
    FakePrompts prompts;
    MainWindow window{files, mainOf(files), prompts};
    window.show();
    prompts.nextFileToOpen = "suite.srt";
    window.appendFileAction()->trigger();
    REQUIRE(window.table()->model()->rowCount() == 5);

    window.undoAction()->trigger();

    CHECK(window.table()->model()->rowCount() == 3);
    CHECK_FALSE(window.undoAction()->isEnabled());
}

TEST_CASE("what the appended file's own reading ran into is shown", "[gui][GUI-APPEND-01]") {
    InMemoryFileSystem files = withThreeAndASuite();
    // A block with no number is what the reading reports and recovers from —
    // the file still opens, and the panel says what it met.
    files.addFile("sans-numero.srt", "00:00:00,000 --> 00:00:01,000\nQuatre.\n\n");
    FakePrompts prompts;
    MainWindow window{files, mainOf(files), prompts};
    window.show();
    REQUIRE(window.diagnostics()->count() == 0);
    prompts.nextFileToOpen = "sans-numero.srt";

    window.appendFileAction()->trigger();

    CHECK(window.diagnostics()->count() > 0);
}
