// The one button that writes a tag — issue #365.
//
// **What it buys is the vocabulary a user no longer has to know.** The text of
// a subtitle is the text of its file, tags included: an `.ass` says `{\i1}` and
// a `.sub` says `{Y:i}`, and before this entry existed there was nowhere to
// learn it. The tests below check the two things that follow from that — the
// right tag for the document open, and an entry that goes out where a format
// carries no style at all.

#include <subedit/core/format/project_file.hpp>
#include <subedit/core/io/in_memory_file_system.hpp>
#include <subedit/gui/main_window.hpp>

#include <QAbstractItemModel>
#include <QAction>
#include <QApplication>
#include <QItemSelectionModel>
#include <QMenu>
#include <QMenuBar>
#include <QPlainTextEdit>
#include <QStatusBar>
#include <QTableView>
#include <QTest>
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

constexpr const char* kSubRip = "1\n00:00:01,000 --> 00:00:02,000\nBonjour.\n\n"
                                "2\n00:00:03,000 --> 00:00:04,000\nAu revoir.\n\n";

/// Advanced SSA, whose events carry the fields its `Format:` line names.
constexpr const char* kAdvancedSsa =
    "[Script Info]\nScriptType: v4.00+\n\n"
    "[V4+ Styles]\n"
    "Format: Name, Fontname\nStyle: Default,Arial\n\n"
    "[Events]\n"
    "Format: Layer, Start, End, Style, Name, MarginL, MarginR, MarginV, Effect, Text\n"
    "Dialogue: 0,0:00:01.00,0:00:02.00,Default,,0,0,0,,Bonjour.\n";

/// A blank row among two written ones, as an insertion leaves one.
constexpr const char* kWithBlank = "1\n00:00:01,000 --> 00:00:02,000\nBonjour.\n\n"
                                   "2\n00:00:03,000 --> 00:00:04,000\n\n"
                                   "3\n00:00:05,000 --> 00:00:06,000\nAu revoir.\n\n";

/// LRC, which has no way of saying anything about a style.
constexpr const char* kLrc = "[00:01.00]Bonjour.\n[00:03.00]Au revoir.\n";

[[nodiscard]] InMemoryFileSystem withFile(const char* name, const char* content) {
    InMemoryFileSystem files;
    files.addFile(name, content);
    return files;
}

[[nodiscard]] OpenedFile fileIn(const InMemoryFileSystem& files, const char* name) {
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

void selectRow(const MainWindow& window, int row) {
    window.table()->selectionModel()->select(window.table()->model()->index(row, 0),
                                             QItemSelectionModel::Select |
                                                 QItemSelectionModel::Rows);
}

} // namespace

TEST_CASE("the entry writes the italic tag of the document open", "[gui][GUI-ITALIC-01]") {
    InMemoryFileSystem files = withFile("film.srt", kSubRip);
    FakePrompts prompts;
    MainWindow window{files, fileIn(files, "film.srt"), prompts};
    window.show();

    window.italicAction()->trigger();

    CHECK(textAt(window, 0) == "<i>Bonjour.</i>");
    CHECK(textAt(window, 1) == "<i>Au revoir.</i>");
}

TEST_CASE("the same entry writes braces on an Advanced SSA", "[gui][GUI-ITALIC-01]") {
    InMemoryFileSystem files = withFile("film.ass", kAdvancedSsa);
    FakePrompts prompts;
    MainWindow window{files, fileIn(files, "film.ass"), prompts};
    window.show();

    window.italicAction()->trigger();

    CHECK(textAt(window, 0) == R"({\i1}Bonjour.{\i0})");
}

TEST_CASE("pressing it twice leaves the text as it was found", "[gui][GUI-ITALIC-01]") {
    InMemoryFileSystem files = withFile("film.srt", kSubRip);
    FakePrompts prompts;
    MainWindow window{files, fileIn(files, "film.srt"), prompts};
    window.show();

    window.italicAction()->trigger();
    window.italicAction()->trigger();

    CHECK(textAt(window, 0) == "Bonjour.");
    CHECK(textAt(window, 1) == "Au revoir.");
}

TEST_CASE("only what is selected is touched", "[gui][GUI-ITALIC-01]") {
    InMemoryFileSystem files = withFile("film.srt", kSubRip);
    FakePrompts prompts;
    MainWindow window{files, fileIn(files, "film.srt"), prompts};
    window.show();

    selectRow(window, 1);
    window.italicAction()->trigger();

    CHECK(textAt(window, 0) == "Bonjour.");
    CHECK(textAt(window, 1) == "<i>Au revoir.</i>");
}

TEST_CASE("the operation enters the history and comes back out", "[gui][GUI-ITALIC-01]") {
    InMemoryFileSystem files = withFile("film.srt", kSubRip);
    FakePrompts prompts;
    MainWindow window{files, fileIn(files, "film.srt"), prompts};
    window.show();

    window.italicAction()->trigger();
    REQUIRE(window.undoAction()->isEnabled());
    CHECK(window.undoAction()->text().toStdString() == "Undo: putting in italics");
    // What undo is about to take back: without it, the two checks below would
    // pass on rows that were never put in italics.
    REQUIRE(textAt(window, 0) == "<i>Bonjour.</i>");
    REQUIRE(textAt(window, 1) == "<i>Au revoir.</i>");

    window.undoAction()->trigger();
    // Both rows, since both were changed: one entry undoes the whole operation.
    CHECK(textAt(window, 0) == "Bonjour.");
    CHECK(textAt(window, 1) == "Au revoir.");
}

TEST_CASE("the entry says how many subtitles it moved", "[gui][GUI-ITALIC-01]") {
    InMemoryFileSystem files = withFile("film.srt", kSubRip);
    FakePrompts prompts;
    MainWindow window{files, fileIn(files, "film.srt"), prompts};
    window.show();

    window.italicAction()->trigger();
    CHECK(window.statusBar()->currentMessage().toStdString() == "2 subtitles put in italics");

    window.italicAction()->trigger();
    CHECK(window.statusBar()->currentMessage().toStdString() == "2 subtitles taken out of italics");
}

TEST_CASE("a blank row gains no tags, and its selection changes nothing", "[gui][GUI-ITALIC-01]") {
    InMemoryFileSystem files = withFile("film.srt", kWithBlank);
    FakePrompts prompts;
    MainWindow window{files, fileIn(files, "film.srt"), prompts};
    window.show();

    // The whole file: the blank row is carried along and stays blank, where an
    // `<i></i>` around nothing would be visible in the table.
    window.italicAction()->trigger();
    CHECK(textAt(window, 0) == "<i>Bonjour.</i>");
    CHECK(textAt(window, 1).empty());
    CHECK(textAt(window, 2) == "<i>Au revoir.</i>");

    window.table()->selectionModel()->clearSelection();
    selectRow(window, 1);

    // Nothing but blank rows: no operation, and nothing added to the history.
    const bool undoable = window.undoAction()->isEnabled();
    window.italicAction()->trigger();
    CHECK(window.statusBar()->currentMessage().toStdString() == "nothing to change");
    CHECK(window.undoAction()->isEnabled() == undoable);
    CHECK(window.undoAction()->text().toStdString() == "Undo: putting in italics");
}

TEST_CASE("a format that carries no style leaves the entry out", "[gui][GUI-ITALIC-02]") {
    InMemoryFileSystem files = withFile("film.lrc", kLrc);
    FakePrompts prompts;
    MainWindow window{files, fileIn(files, "film.lrc"), prompts};
    window.show();

    // Out and not gone: what a user of an LRC has to learn is that there is
    // nothing to type, and an entry that disappeared would teach nothing. So the
    // entry is looked for where the user looks — in the `Tools` menu itself.
    QMenu* tools = nullptr;
    for (QAction* entry : window.menuBar()->actions()) {
        if (entry->text() == QStringLiteral("&Tools"))
            tools = entry->menu();
    }
    REQUIRE(tools != nullptr);

    CHECK(tools->actions().contains(window.italicAction()));
    CHECK(window.italicAction()->isVisible());
    // Laid out by the menu, which gives no room to an entry that is hidden.
    CHECK_FALSE(tools->actionGeometry(window.italicAction()).isEmpty());
    CHECK_FALSE(window.italicAction()->isEnabled());
}

// Issue #397: a gesture without a dialog reads the target and builds a command
// straight from what the model already holds — nothing takes the focus away
// from a cell being edited first, unlike a dialog, which does that simply by
// opening. `Ctrl+I` is the gesture; the row being typed into is what it must
// not race.
TEST_CASE("Ctrl+I while a cell is being edited commits the edit first", "[gui][GUI-EDIT-01]") {
    InMemoryFileSystem files = withFile("film.srt", kSubRip);
    FakePrompts prompts;
    MainWindow window{files, fileIn(files, "film.srt"), prompts};
    window.show();

    // A real focus-out — what the fix relies on — only fires once the window
    // is genuinely active, which `show()` alone does not make it under the
    // offscreen platform.
    window.activateWindow();
    QApplication::setActiveWindow(&window);
    REQUIRE(QTest::qWaitForWindowActive(&window));

    const QModelIndex edited = window.table()->model()->index(0, 4);
    window.table()->setCurrentIndex(edited);
    window.table()->edit(edited);
    REQUIRE(window.table()->isEditing());

    // The delegate's editor is a `QPlainTextEdit` (an internal subclass of it,
    // `SubtitleEditor` — see `cell_delegates.cpp`), found the same way
    // `main_window_test.cpp` already does for the ordinary typing case, since
    // that internal type is not reachable from a test.
    auto* editor = window.table()->findChild<QPlainTextEdit*>();
    REQUIRE(editor != nullptr);
    REQUIRE(editor->hasFocus());
    QTest::keyClicks(editor, " tout");

    window.italicAction()->trigger();

    CHECK_FALSE(window.table()->isEditing());
    CHECK(textAt(window, 0).find("tout") != std::string::npos);
    // Both, not either: the italic went on the text the editor had just
    // committed, and not on the stale text that the commit would then overwrite.
    CHECK(textAt(window, 0).find("<i>") != std::string::npos);
}
