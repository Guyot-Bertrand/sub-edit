// A cell being edited, and the gestures that read the model — issues #397, #416.
//
// A gesture without a dialog builds its command straight from what the model
// holds, and nothing takes the focus away from a cell being typed into first —
// unlike a dialog, which does that simply by opening. Eleven gestures call
// `commitCellEditor()` for that reason, and one of them, `Ctrl+I`, is proved in
// `window_italics_test.cpp`. This file proves the other ten: the four cases,
// the dialogue dashes, cutting, pasting, merging and splitting (#397), and
// copying, removing, undo and redo (#416).
//
// Each of them must find the typed text already committed. Two things say so:
// the editor is closed once the gesture is over, and the gesture has worked on
// the text that was typed and not on the one the model held before it. Undo and
// redo read the history and not the document, so they have a case of their own
// below the table.

#include <subedit/core/format/project_file.hpp>
#include <subedit/core/io/in_memory_file_system.hpp>
#include <subedit/core/text/letter_case.hpp>
#include <subedit/gui/main_window.hpp>

#include <QAbstractItemModel>
#include <QAction>
#include <QApplication>
#include <QClipboard>
#include <QGuiApplication>
#include <QItemSelectionModel>
#include <QPlainTextEdit>
#include <QString>
#include <QTableView>
#include <QTest>
#include <QTextCursor>
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/generators/catch_generators_range.hpp>

#include <array>
#include <cstddef>
#include <string>
#include <utility>

#include "fake_prompts.hpp"

namespace {

using subedit::core::InMemoryFileSystem;
using subedit::core::LetterCase;
using subedit::core::OpenedFile;
using subedit::core::openProject;
using subedit::gui::MainWindow;
using subedit::test::FakePrompts;

/// Three subtitles. The first is in capitals and the others are not, so that
/// each of the four cases has something to change in it once `" marie"` is
/// typed after it.
constexpr const char* kThree = "1\n00:00:01,000 --> 00:00:02,000\nBONJOUR\n\n"
                               "2\n00:00:03,000 --> 00:00:04,000\nau revoir\n\n"
                               "3\n00:00:05,000 --> 00:00:06,000\ntrois\n\n";

constexpr int kTextColumn = 4;

/// What the editor gets typed after the text the first subtitle already has.
constexpr const char* kTyped = " marie";

[[nodiscard]] InMemoryFileSystem withThree() {
    InMemoryFileSystem files;
    files.addFile("film.srt", kThree);
    return files;
}

[[nodiscard]] OpenedFile openedIn(const InMemoryFileSystem& files) {
    auto opened = openProject(files, "film.srt");
    REQUIRE(opened.has_value());
    return std::move(*opened);
}

[[nodiscard]] std::string textAt(const MainWindow& window, int row) {
    return window.table()
        ->model()
        ->data(window.table()->model()->index(row, kTextColumn), Qt::DisplayRole)
        .toString()
        .toStdString();
}

[[nodiscard]] int rowCount(const MainWindow& window) {
    return window.table()->model()->rowCount({});
}

void selectRow(const MainWindow& window, int row) {
    window.table()->selectionModel()->select(window.table()->model()->index(row, 0),
                                             QItemSelectionModel::Select |
                                                 QItemSelectionModel::Rows);
}

/// Opens the editor on the text of the first subtitle and types `kTyped` into it,
/// without committing.
///
/// A real focus-out — what the commit relies on — only fires once the window is
/// genuinely active, which `show()` alone does not make it under the offscreen
/// platform. The same staging as `Ctrl+I`'s test.
void typeIntoFirstCell(MainWindow& window) {
    window.activateWindow();
    QApplication::setActiveWindow(&window);
    REQUIRE(QTest::qWaitForWindowActive(&window));

    const QModelIndex edited = window.table()->model()->index(0, kTextColumn);
    window.table()->setCurrentIndex(edited);
    window.table()->edit(edited);
    REQUIRE(window.table()->isEditing());

    // The delegate's editor is an internal subclass of `QPlainTextEdit`, found
    // the way `main_window_test.cpp` finds it.
    auto* editor = window.table()->findChild<QPlainTextEdit*>();
    REQUIRE(editor != nullptr);
    REQUIRE(editor->hasFocus());
    editor->moveCursor(QTextCursor::End);
    QTest::keyClicks(editor, kTyped);

    // Typed, and not yet committed: the model still holds the text as it was.
    REQUIRE(textAt(window, 0) == "BONJOUR");
    REQUIRE(window.table()->selectionModel()->selectedRows().size() == 1);
}

/// One gesture, and what it must leave behind.
struct Gesture {
    const char* name;

    /// What the gesture needs beyond the first row being selected: a second row
    /// for a merge, a system clipboard for a paste.
    void (*arrange)(const MainWindow&);

    /// The entry the menu triggers.
    QAction* (*action)(const MainWindow&);

    /// The text of the first subtitle once the gesture is over. Every one of
    /// them is built from `BONJOUR marie`, the text as committed, and none from
    /// `BONJOUR`, the text the model held before the commit.
    const char* firstText;

    /// What the gesture does to the rest of the document, and nothing else; may
    /// be null.
    void (*verify)(const MainWindow&);
};

void arrangeNothing(const MainWindow& /*window*/) {}

const std::array<Gesture, 11> kGestures = {{
    {.name = "title case",
     .arrange = arrangeNothing,
     .action = [](const MainWindow& w) { return w.caseAction(LetterCase::Title); },
     .firstText = "Bonjour Marie",
     .verify = nullptr},
    {.name = "sentence case",
     .arrange = arrangeNothing,
     .action = [](const MainWindow& w) { return w.caseAction(LetterCase::Sentence); },
     .firstText = "Bonjour marie",
     .verify = nullptr},
    {.name = "upper case",
     .arrange = arrangeNothing,
     .action = [](const MainWindow& w) { return w.caseAction(LetterCase::Upper); },
     .firstText = "BONJOUR MARIE",
     .verify = nullptr},
    {.name = "lower case",
     .arrange = arrangeNothing,
     .action = [](const MainWindow& w) { return w.caseAction(LetterCase::Lower); },
     .firstText = "bonjour marie",
     .verify = nullptr},
    {.name = "dialogue dashes",
     .arrange = arrangeNothing,
     .action = [](const MainWindow& w) { return w.dialogueDashesAction(); },
     .firstText = "- BONJOUR marie",
     .verify = nullptr},
    {.name = "cut",
     .arrange = arrangeNothing,
     .action = [](const MainWindow& w) { return w.cutAction(); },
     .firstText = "",
     .verify =
         [](const MainWindow& /*w*/) {
             // What went to the clipboard is what the row held when it was cut.
             CHECK(QGuiApplication::clipboard()->text().toStdString() == "BONJOUR marie");
         }},
    {.name = "paste",
     .arrange =
         [](const MainWindow& /*w*/) {
             QGuiApplication::clipboard()->setText(QStringLiteral("pasted"));
         },
     .action = [](const MainWindow& w) { return w.pasteAction(); },
     .firstText = "pasted",
     .verify =
         [](const MainWindow& w) {
             // What the paste replaced is the committed text, which undoing it
             // must give back: the model's earlier text is not what was there.
             w.undoAction()->trigger();
             CHECK(textAt(w, 0) == "BONJOUR marie");
         }},
    {.name = "merge",
     .arrange = [](const MainWindow& w) { selectRow(w, 1); },
     .action = [](const MainWindow& w) { return w.mergeAction(); },
     .firstText = "BONJOUR marie\nau revoir",
     .verify =
         [](const MainWindow& w) {
             CHECK(rowCount(w) == 2);
             CHECK(textAt(w, 1) == "trois");
         }},
    {.name = "split",
     .arrange = arrangeNothing,
     .action = [](const MainWindow& w) { return w.splitAction(); },
     .firstText = "BONJOUR marie",
     .verify =
         [](const MainWindow& w) {
             // The text stays with the first half, and the second starts empty.
             CHECK(rowCount(w) == 4);
             CHECK(textAt(w, 1).empty());
             CHECK(textAt(w, 2) == "au revoir");
         }},
    {.name = "copy",
     .arrange = arrangeNothing,
     .action = [](const MainWindow& w) { return w.copyAction(); },
     .firstText = "BONJOUR marie",
     .verify =
         [](const MainWindow& /*w*/) {
             // What went to the clipboard is what was typed, and not the text
             // the model held before it.
             CHECK(QGuiApplication::clipboard()->text().toStdString() == "BONJOUR marie");
         }},
    {.name = "remove",
     .arrange = arrangeNothing,
     .action = [](const MainWindow& w) { return w.removeAction(); },
     .firstText = "au revoir",
     .verify =
         [](const MainWindow& w) {
             CHECK(rowCount(w) == 2);
             // What the removal brings back is the subtitle the user was
             // looking at, typed text included.
             w.undoAction()->trigger();
             CHECK(textAt(w, 0) == "BONJOUR marie");
         }},
}};

} // namespace

TEST_CASE("a gesture without a dialog commits the cell being edited first", "[gui][GUI-EDIT-01]") {
    const Gesture& gesture =
        kGestures.at(GENERATE(Catch::Generators::range(std::size_t{0}, kGestures.size())));
    INFO("gesture: " << gesture.name);

    InMemoryFileSystem files = withThree();
    FakePrompts prompts;
    MainWindow window{files, openedIn(files), prompts};
    window.show();

    typeIntoFirstCell(window);

    gesture.arrange(window);
    QAction* const action = gesture.action(window);
    REQUIRE(action->isEnabled());

    action->trigger();

    CHECK_FALSE(window.table()->isEditing());
    CHECK(textAt(window, 0) == gesture.firstText);
    if (gesture.verify != nullptr)
        gesture.verify(window);
}

TEST_CASE("undo, with a cell being edited, undoes the typing and not what came before",
          "[gui][GUI-EDIT-01]") {
    InMemoryFileSystem files = withThree();
    FakePrompts prompts;
    MainWindow window{files, openedIn(files), prompts};
    window.show();

    // Something to undo that is not the typing: the third subtitle in capitals.
    window.table()->selectRow(2);
    window.caseAction(LetterCase::Upper)->trigger();
    REQUIRE(textAt(window, 2) == "TROIS");

    typeIntoFirstCell(window);
    REQUIRE(window.undoAction()->isEnabled());

    window.undoAction()->trigger();

    // The typing was the last thing the user did, so it is what goes: it has to
    // be committed first, or the undo reaches past it and takes the capitals
    // off the third subtitle instead.
    CHECK_FALSE(window.table()->isEditing());
    CHECK(textAt(window, 0) == "BONJOUR");
    CHECK(textAt(window, 2) == "TROIS");
}

TEST_CASE("redo, with a cell being edited, keeps the typing and drops what it would have redone",
          "[gui][GUI-EDIT-01]") {
    InMemoryFileSystem files = withThree();
    FakePrompts prompts;
    MainWindow window{files, openedIn(files), prompts};
    window.show();

    // Something to redo: the capitals on the third subtitle, done and undone.
    window.table()->selectRow(2);
    window.caseAction(LetterCase::Upper)->trigger();
    window.undoAction()->trigger();
    REQUIRE(textAt(window, 2) == "trois");
    REQUIRE(window.redoAction()->isEnabled());

    typeIntoFirstCell(window);

    window.redoAction()->trigger();

    // Typing after an undo ends what could be redone, here as in any editor:
    // the typing is committed first, and there is nothing left to redo.
    CHECK_FALSE(window.table()->isEditing());
    CHECK(textAt(window, 0) == "BONJOUR marie");
    CHECK(textAt(window, 2) == "trois");
    CHECK_FALSE(window.redoAction()->isEnabled());
}

TEST_CASE("undo, with a cell open and nothing typed, still undoes what came before",
          "[gui][GUI-EDIT-01]") {
    InMemoryFileSystem files = withThree();
    FakePrompts prompts;
    MainWindow window{files, openedIn(files), prompts};
    window.show();

    window.table()->selectRow(2);
    window.caseAction(LetterCase::Upper)->trigger();
    REQUIRE(textAt(window, 2) == "TROIS");

    // The editor opens on the first subtitle and is left as it is: committing
    // an unchanged text puts nothing in the history, so the undo below has to
    // reach the capitals and not a non-event the commit would have left.
    window.activateWindow();
    QApplication::setActiveWindow(&window);
    REQUIRE(QTest::qWaitForWindowActive(&window));
    const QModelIndex edited = window.table()->model()->index(0, kTextColumn);
    window.table()->setCurrentIndex(edited);
    window.table()->edit(edited);
    REQUIRE(window.table()->isEditing());

    window.undoAction()->trigger();

    CHECK_FALSE(window.table()->isEditing());
    CHECK(textAt(window, 0) == "BONJOUR");
    CHECK(textAt(window, 2) == "trois");
}
