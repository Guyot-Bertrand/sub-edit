// Cutting, copying and pasting texts from the window — issue #382.
//
// The rules live in the core and are tested there. What is under test here is
// what the window adds: the system clipboard written and read back, the
// entries out without a selection, the notice after a paste that laid rows
// down or dropped tags, and the format of a copy carried from one opened file
// to the next.

#include <subedit/core/format/project_file.hpp>
#include <subedit/core/io/in_memory_file_system.hpp>
#include <subedit/gui/main_window.hpp>

#include <QAbstractItemModel>
#include <QAction>
#include <QClipboard>
#include <QGuiApplication>
#include <QItemSelectionModel>
#include <QString>
#include <catch2/catch_test_macros.hpp>

#include <string>
#include <utility>

#include "fake_prompts.hpp"

namespace {

using subedit::core::InMemoryFileSystem;
using subedit::core::OpenedFile;
using subedit::core::openProject;
using subedit::gui::MainWindow;
using subedit::test::FakePrompts;

/// Three subtitles, at one, three and five seconds, the second in italics.
constexpr const char* kThree = "1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n"
                               "2\n00:00:03,000 --> 00:00:04,000\n<i>Deux.</i>\n\n"
                               "3\n00:00:05,000 --> 00:00:06,000\nTrois.\n\n";

/// Two lines of LRC, a format that writes no style at all.
constexpr const char* kLyrics = "[00:01.00]Premier.\n[00:03.00]Second.\n";

[[nodiscard]] InMemoryFileSystem withFiles() {
    InMemoryFileSystem files;
    files.addFile("film.srt", kThree);
    files.addFile("chanson.lrc", kLyrics);
    return files;
}

[[nodiscard]] OpenedFile opened(const InMemoryFileSystem& files, const char* path) {
    auto project = openProject(files, path);
    REQUIRE(project.has_value());
    return std::move(*project);
}

constexpr int kStartColumn = 1;
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

void selectOnly(const MainWindow& window, int row) {
    window.table()->selectionModel()->clearSelection();
    selectRow(window, row);
}

[[nodiscard]] std::string systemText() {
    return QGuiApplication::clipboard()->text().toStdString();
}

} // namespace

TEST_CASE("copying puts the texts on the system clipboard, glued by a blank line",
          "[gui][GUI-CLIP-01]") {
    InMemoryFileSystem files = withFiles();
    FakePrompts prompts;
    MainWindow window{files, opened(files, "film.srt"), prompts};
    window.show();
    selectRow(window, 0);
    selectRow(window, 2);

    window.copyAction()->trigger();

    // A hole for the row that was not selected, as Gaupol writes it.
    CHECK(systemText() == "Un.\n\n\n\nTrois.");
    CHECK_FALSE(window.undoAction()->isEnabled());
}

TEST_CASE("cutting empties the texts, and undoing gives them back", "[gui][GUI-CLIP-01]") {
    InMemoryFileSystem files = withFiles();
    FakePrompts prompts;
    MainWindow window{files, opened(files, "film.srt"), prompts};
    window.show();
    selectRow(window, 0);
    selectRow(window, 1);

    window.cutAction()->trigger();

    CHECK(systemText() == "Un.\n\n<i>Deux.</i>");
    CHECK(textAt(window, 0).empty());
    CHECK(textAt(window, 1).empty());
    CHECK(window.undoAction()->text().toStdString() == "Undo: cutting texts");

    window.undoAction()->trigger();

    CHECK(textAt(window, 0) == "Un.");
    CHECK(textAt(window, 1) == "<i>Deux.</i>");
}

TEST_CASE("pasting writes from the first selected row and moves no position",
          "[gui][GUI-CLIP-01]") {
    InMemoryFileSystem files = withFiles();
    FakePrompts prompts;
    MainWindow window{files, opened(files, "film.srt"), prompts};
    window.show();
    selectRow(window, 0);
    window.copyAction()->trigger();

    selectOnly(window, 2);
    window.pasteAction()->trigger();

    CHECK(textAt(window, 2) == "Un.");
    CHECK(cellAt(window, 2, kStartColumn) == "00:00:05,000");
    CHECK(rowCount(window) == 3);
    CHECK(prompts.outcomes.empty());
    CHECK(window.undoAction()->text().toStdString() == "Undo: pasting texts");

    window.undoAction()->trigger();

    CHECK(textAt(window, 2) == "Trois.");
}

TEST_CASE("pasting more texts than rows remain lays rows down, and says so", "[gui][GUI-CLIP-01]") {
    InMemoryFileSystem files = withFiles();
    FakePrompts prompts;
    MainWindow window{files, opened(files, "film.srt"), prompts};
    window.show();
    QGuiApplication::clipboard()->setText(QStringLiteral("A.\n\nB.\n\nC."));
    selectRow(window, 2);

    window.pasteAction()->trigger();

    REQUIRE(rowCount(window) == 5);
    CHECK(textAt(window, 2) == "A.");
    CHECK(textAt(window, 4) == "C.");
    REQUIRE(prompts.outcomes.size() == 1);
    CHECK(prompts.outcomes.front() == "inserted 2 subtitles to fit the clipboard");
    // The rows written are selected, so that a second paste has a place to go.
    CHECK(window.table()->selectionModel()->selectedRows().size() == 3);

    // One entry for the whole of it.
    window.undoAction()->trigger();

    CHECK(rowCount(window) == 3);
    CHECK(textAt(window, 2) == "Trois.");
    CHECK_FALSE(window.undoAction()->isEnabled());
}

TEST_CASE("a text copied outside the program is pasted as it is", "[gui][GUI-CLIP-02]") {
    // No format to translate from: braces that came from nowhere stay braces.
    InMemoryFileSystem files = withFiles();
    FakePrompts prompts;
    MainWindow window{files, opened(files, "film.srt"), prompts};
    window.show();
    selectRow(window, 0);
    window.copyAction()->trigger();

    QGuiApplication::clipboard()->setText(QStringLiteral("{\\i1}Ailleurs."));
    window.pasteAction()->trigger();

    CHECK(textAt(window, 0) == "{\\i1}Ailleurs.");
    CHECK(prompts.outcomes.empty());
}

TEST_CASE("pasting into a document of another format translates the tags, and says what fell",
          "[gui][GUI-CLIP-02]") {
    InMemoryFileSystem files = withFiles();
    FakePrompts prompts;
    MainWindow window{files, opened(files, "film.srt"), prompts};
    window.show();
    selectRow(window, 1);
    window.copyAction()->trigger();

    // The copy survives the opening of the next file, which is its whole point.
    prompts.nextFileToOpen = "chanson.lrc";
    window.openAction()->trigger();
    REQUIRE(textAt(window, 0) == "Premier.");

    selectRow(window, 0);
    window.pasteAction()->trigger();

    // LRC writes no italic: the tag goes, and the text stays.
    CHECK(textAt(window, 0) == "Deux.");
    REQUIRE(prompts.outcomes.size() == 1);
    CHECK(prompts.outcomes.front() == "pasting SubRip texts into LRC: 1 tag dropped");
}

TEST_CASE("with no selection, the three entries are out", "[gui][GUI-CLIP-01]") {
    // The rule of `Remove Subtitles`: nothing selected is not « the whole
    // file » here, or one `Ctrl+X` would empty every text.
    InMemoryFileSystem files = withFiles();
    FakePrompts prompts;
    MainWindow window{files, opened(files, "film.srt"), prompts};
    window.show();

    CHECK_FALSE(window.cutAction()->isEnabled());
    CHECK_FALSE(window.copyAction()->isEnabled());
    CHECK_FALSE(window.pasteAction()->isEnabled());

    selectRow(window, 1);

    CHECK(window.cutAction()->isEnabled());
    CHECK(window.copyAction()->isEnabled());
    CHECK(window.pasteAction()->isEnabled());
}

TEST_CASE("forced on with no selection, the three entries change nothing", "[gui][GUI-CLIP-01]") {
    // The second guard, reached by switching the entries back on by hand.
    InMemoryFileSystem files = withFiles();
    FakePrompts prompts;
    MainWindow window{files, opened(files, "film.srt"), prompts};
    window.show();
    QGuiApplication::clipboard()->setText(QStringLiteral("Rien."));

    for (QAction* entry : {window.cutAction(), window.copyAction(), window.pasteAction()}) {
        entry->setEnabled(true);
        entry->trigger();
    }

    CHECK(systemText() == "Rien.");
    CHECK(textAt(window, 0) == "Un.");
    CHECK_FALSE(window.undoAction()->isEnabled());
}

TEST_CASE("the three entries carry the platform's shortcuts", "[gui][GUI-CLIP-01]") {
    InMemoryFileSystem files = withFiles();
    FakePrompts prompts;
    const MainWindow window{files, opened(files, "film.srt"), prompts};

    CHECK(window.cutAction()->shortcut() == QKeySequence{QKeySequence::Cut});
    CHECK(window.copyAction()->shortcut() == QKeySequence{QKeySequence::Copy});
    CHECK(window.pasteAction()->shortcut() == QKeySequence{QKeySequence::Paste});
}

TEST_CASE("pasting with nothing on the clipboard changes nothing", "[gui][GUI-CLIP-01]") {
    InMemoryFileSystem files = withFiles();
    FakePrompts prompts;
    MainWindow window{files, opened(files, "film.srt"), prompts};
    window.show();
    QGuiApplication::clipboard()->clear();
    selectRow(window, 0);

    window.pasteAction()->trigger();

    CHECK(textAt(window, 0) == "Un.");
    CHECK_FALSE(window.undoAction()->isEnabled());
}

TEST_CASE("pasting the texts already there does not enter the history", "[gui][GUI-CLIP-01]") {
    InMemoryFileSystem files = withFiles();
    FakePrompts prompts;
    MainWindow window{files, opened(files, "film.srt"), prompts};
    window.show();
    selectRow(window, 0);
    window.copyAction()->trigger();

    window.pasteAction()->trigger();

    CHECK_FALSE(window.undoAction()->isEnabled());
    CHECK(prompts.outcomes.empty());
}

TEST_CASE("cutting texts that are already empty copies them and changes nothing",
          "[gui][GUI-CLIP-01]") {
    InMemoryFileSystem files = withFiles();
    FakePrompts prompts;
    MainWindow window{files, opened(files, "film.srt"), prompts};
    window.show();
    selectRow(window, 0);
    window.cutAction()->trigger();
    REQUIRE(textAt(window, 0).empty());

    QGuiApplication::clipboard()->setText(QStringLiteral("autre"));
    window.cutAction()->trigger();

    // Copied all the same — the system clipboard now holds the empty text —
    // and no second entry: one undo empties the history.
    CHECK(systemText().empty());
    window.undoAction()->trigger();
    CHECK(textAt(window, 0) == "Un.");
    CHECK_FALSE(window.undoAction()->isEnabled());
}

TEST_CASE("an empty text pastes as a hole, whichever road the copy takes", "[gui][GUI-CLIP-01]") {
    InMemoryFileSystem files = withFiles();
    FakePrompts prompts;
    MainWindow window{files, opened(files, "film.srt"), prompts};
    window.show();

    // A copy of three rows whose middle one has no text: cut it, copy the three,
    // and put the middle text back — the copy is all that keeps the emptiness.
    selectOnly(window, 1);
    window.cutAction()->trigger();
    REQUIRE(textAt(window, 1).empty());
    window.table()->selectAll();
    window.copyAction()->trigger();
    window.undoAction()->trigger();
    REQUIRE(textAt(window, 1) == "<i>Deux.</i>");

    // First road: the same window, whose own copy the system clipboard still
    // agrees with.
    selectOnly(window, 0);
    window.pasteAction()->trigger();
    CHECK(textAt(window, 0) == "Un.");
    CHECK(textAt(window, 1) == "<i>Deux.</i>");
    CHECK(textAt(window, 2) == "Trois.");

    // Second road: a window that has copied nothing, and reads the plain text
    // the system holds, as it would from any other program.
    MainWindow other{files, opened(files, "film.srt"), prompts};
    other.show();
    selectOnly(other, 0);
    other.pasteAction()->trigger();
    CHECK(textAt(other, 0) == "Un.");
    CHECK(textAt(other, 1) == "<i>Deux.</i>");
    CHECK(textAt(other, 2) == "Trois.");
}
