// A cell being edited, and the gestures that read the model — issue #397.
//
// A gesture without a dialog builds its command straight from what the model
// holds, and nothing takes the focus away from a cell being typed into first —
// unlike a dialog, which does that simply by opening. Seven gestures call
// `commitCellEditor()` for that reason, and one of them, `Ctrl+I`, is proved in
// `window_italics_test.cpp`. This file proves the other six: the four cases,
// the dialogue dashes, cutting, pasting, merging and splitting.
//
// Each of them must find the typed text already committed. Two things say so:
// the editor is closed once the gesture is over, and the gesture has worked on
// the text that was typed and not on the one the model held before it.

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

const std::array<Gesture, 9> kGestures = {{
    {"title case",
     arrangeNothing,
     [](const MainWindow& w) { return w.caseAction(LetterCase::Title); },
     "Bonjour Marie",
     nullptr},
    {"sentence case",
     arrangeNothing,
     [](const MainWindow& w) { return w.caseAction(LetterCase::Sentence); },
     "Bonjour marie",
     nullptr},
    {"upper case",
     arrangeNothing,
     [](const MainWindow& w) { return w.caseAction(LetterCase::Upper); },
     "BONJOUR MARIE",
     nullptr},
    {"lower case",
     arrangeNothing,
     [](const MainWindow& w) { return w.caseAction(LetterCase::Lower); },
     "bonjour marie",
     nullptr},
    {"dialogue dashes",
     arrangeNothing,
     [](const MainWindow& w) { return w.dialogueDashesAction(); },
     "- BONJOUR marie",
     nullptr},
    {"cut",
     arrangeNothing,
     [](const MainWindow& w) { return w.cutAction(); },
     "",
     [](const MainWindow& /*w*/) {
         // What went to the clipboard is what the row held when it was cut.
         CHECK(QGuiApplication::clipboard()->text().toStdString() == "BONJOUR marie");
     }},
    {"paste",
     [](const MainWindow& /*w*/) {
         QGuiApplication::clipboard()->setText(QStringLiteral("pasted"));
     },
     [](const MainWindow& w) { return w.pasteAction(); },
     "pasted",
     [](const MainWindow& w) {
         // What the paste replaced is the committed text, which undoing it
         // must give back: the model's earlier text is not what was there.
         w.undoAction()->trigger();
         CHECK(textAt(w, 0) == "BONJOUR marie");
     }},
    {"merge",
     [](const MainWindow& w) { selectRow(w, 1); },
     [](const MainWindow& w) { return w.mergeAction(); },
     "BONJOUR marie\nau revoir",
     [](const MainWindow& w) {
         CHECK(rowCount(w) == 2);
         CHECK(textAt(w, 1) == "trois");
     }},
    {"split",
     arrangeNothing,
     [](const MainWindow& w) { return w.splitAction(); },
     "BONJOUR marie",
     [](const MainWindow& w) {
         // The text stays with the first half, and the second starts empty.
         CHECK(rowCount(w) == 4);
         CHECK(textAt(w, 1).empty());
         CHECK(textAt(w, 2) == "au revoir");
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

    // A real focus-out — what the commit relies on — only fires once the
    // window is genuinely active, which `show()` alone does not make it under
    // the offscreen platform. The same staging as `Ctrl+I`'s test.
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

    gesture.arrange(window);
    QAction* const action = gesture.action(window);
    REQUIRE(action->isEnabled());

    action->trigger();

    CHECK_FALSE(window.table()->isEditing());
    CHECK(textAt(window, 0) == gesture.firstText);
    if (gesture.verify != nullptr)
        gesture.verify(window);
}
