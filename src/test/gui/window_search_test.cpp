// Finding and replacing from the window — issue #384.
//
// The search itself lives in the core and is tested there, down to the
// thirty-two cases of the corpora. What is under test here is what the window
// adds: the dialog kept open, the table moved to each match, the target
// captured and kept, the status line, the history, and the two options carried
// by the preferences.

#include <subedit/core/config/search_options.hpp>
#include <subedit/core/config/settings.hpp>
#include <subedit/core/format/project_file.hpp>
#include <subedit/core/io/in_memory_file_system.hpp>
#include <subedit/gui/main_window.hpp>
#include <subedit/gui/search_dialog.hpp>

#include <QAbstractItemModel>
#include <QAction>
#include <QCheckBox>
#include <QItemSelectionModel>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QString>
#include <catch2/catch_test_macros.hpp>

#include <string>
#include <utility>
#include <vector>

#include "fake_prompts.hpp"

namespace {

using subedit::core::InMemoryFileSystem;
using subedit::core::OpenedFile;
using subedit::core::openProject;
using subedit::core::SearchOptions;
using subedit::gui::MainWindow;
using subedit::gui::SearchDialog;
using subedit::test::FakePrompts;

/// Four subtitles, two with « Marie », one where a tag cuts « Bonjour ».
constexpr const char* kFour = "1\n00:00:01,000 --> 00:00:02,000\nBonjour Marie.\n\n"
                              "2\n00:00:03,000 --> 00:00:04,000\nRien.\n\n"
                              "3\n00:00:05,000 --> 00:00:06,000\n<i>Bon</i>jour.\n\n"
                              "4\n00:00:07,000 --> 00:00:08,000\nMarie et marie.\n\n";

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

constexpr int kTextColumn = 4;

[[nodiscard]] std::string textAt(const MainWindow& window, int row) {
    return window.table()
        ->model()
        ->data(window.table()->model()->index(row, kTextColumn), Qt::DisplayRole)
        .toString()
        .toStdString();
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
    return rows;
}

/// Opens the dialog and types `pattern` into it.
[[nodiscard]] SearchDialog& searching(MainWindow& window, const char* pattern) {
    window.findAndReplaceAction()->trigger();
    SearchDialog* dialog = window.searchDialog();
    REQUIRE(dialog != nullptr);
    dialog->patternField()->setText(QString::fromUtf8(pattern));
    return *dialog;
}

[[nodiscard]] std::string statusOf(const SearchDialog& dialog) {
    return dialog.statusLabel()->text().toStdString();
}

} // namespace

TEST_CASE("the dialog opens, stays open, and is the same one every time", "[gui][GUI-SEARCH-01]") {
    InMemoryFileSystem files = withFour();
    FakePrompts prompts;
    MainWindow window{files, fourIn(files), prompts};
    window.show();

    CHECK(window.searchDialog() == nullptr);
    CHECK(window.findAndReplaceAction()->shortcut() == QKeySequence{QKeySequence::Find});

    const SearchDialog& dialog = searching(window, "Marie");
    CHECK(dialog.isVisible());
    CHECK_FALSE(dialog.isModal());
    // What nobody asked for, Gaupol's: plain text, the case ignored.
    CHECK_FALSE(dialog.regexCheck()->isChecked());
    CHECK(dialog.ignoreCaseCheck()->isChecked());

    window.findAndReplaceAction()->trigger();
    CHECK(window.searchDialog() == &dialog);
    CHECK(dialog.patternField()->text() == QStringLiteral("Marie"));
    // Nothing was asked of prompts: the dialog does not go through the modal
    // road the other operations take.
    CHECK(prompts.runAsked == 0);
}

TEST_CASE("a dialog made on its own starts on Gaupol's defaults", "[gui][GUI-SEARCH-01]") {
    // Found on the capture of the manual: made without being given options, the
    // dialog showed `Ignore case` unchecked, Qt's default and not the search's.
    const SearchDialog dialog;

    CHECK(dialog.options() == SearchOptions{});
    CHECK(dialog.ignoreCaseCheck()->isChecked());
    CHECK_FALSE(dialog.regexCheck()->isChecked());
}

TEST_CASE("find next moves the table to each match, and comes round again",
          "[gui][GUI-SEARCH-01]") {
    InMemoryFileSystem files = withFour();
    FakePrompts prompts;
    MainWindow window{files, fourIn(files), prompts};
    window.show();
    const SearchDialog& dialog = searching(window, "marie");

    dialog.nextButton()->click();
    CHECK(selectedRows(window) == std::vector<int>{0});
    dialog.nextButton()->click();
    CHECK(selectedRows(window) == std::vector<int>{3});
    // « marie » a second time in the same subtitle, the case ignored.
    dialog.nextButton()->click();
    CHECK(selectedRows(window) == std::vector<int>{3});
    dialog.nextButton()->click();
    CHECK(selectedRows(window) == std::vector<int>{0});
    CHECK(statusOf(dialog).empty());

    dialog.previousButton()->click();
    CHECK(selectedRows(window) == std::vector<int>{3});
}

TEST_CASE("the search looks in the visible text, and not in the tags", "[gui][GUI-SEARCH-01]") {
    InMemoryFileSystem files = withFour();
    FakePrompts prompts;
    MainWindow window{files, fourIn(files), prompts};
    window.show();
    const SearchDialog& dialog = searching(window, "<i>");

    dialog.nextButton()->click();
    CHECK(statusOf(dialog) == "\"<i>\" not found");

    dialog.patternField()->setText(QStringLiteral("Bonjour"));
    dialog.nextButton()->click();
    dialog.nextButton()->click();
    CHECK(selectedRows(window) == std::vector<int>{2});
}

TEST_CASE("a search that finds nothing says so, and touches nothing", "[gui][GUI-SEARCH-01]") {
    InMemoryFileSystem files = withFour();
    FakePrompts prompts;
    MainWindow window{files, fourIn(files), prompts};
    window.show();
    selectRow(window, 1);
    const SearchDialog& dialog = searching(window, "Sophie");

    dialog.nextButton()->click();
    CHECK(statusOf(dialog) == "\"Sophie\" not found");
    dialog.replaceAllButton()->click();
    CHECK(statusOf(dialog) == "\"Sophie\" not found");

    CHECK(selectedRows(window) == std::vector<int>{1});
    CHECK_FALSE(window.undoAction()->isEnabled());
    CHECK(prompts.outcomes.empty());
}

TEST_CASE("a replacement that changes nothing says so, and writes nothing",
          "[gui][GUI-SEARCH-01]") {
    // « Marie » is in the document, so this is not « not found »; replacing it
    // by itself leaves the text as it was, so nothing enters the history and
    // nothing is « replaced ».
    InMemoryFileSystem files = withFour();
    FakePrompts prompts;
    MainWindow window{files, fourIn(files), prompts};
    window.show();
    const SearchDialog& dialog = searching(window, "Marie");
    dialog.ignoreCaseCheck()->setChecked(false);
    dialog.replacementField()->setText(QStringLiteral("Marie"));

    dialog.replaceAllButton()->click();

    CHECK(statusOf(dialog) == "nothing to change");
    CHECK(textAt(window, 0) == "Bonjour Marie.");
    CHECK_FALSE(window.undoAction()->isEnabled());
}

TEST_CASE("replace rewrites the match found, keeps its tags, and moves on",
          "[gui][GUI-SEARCH-01]") {
    InMemoryFileSystem files = withFour();
    FakePrompts prompts;
    MainWindow window{files, fourIn(files), prompts};
    window.show();
    const SearchDialog& dialog = searching(window, "Bonjour");
    dialog.replacementField()->setText(QStringLiteral("Salut"));

    // The first press finds, since nothing had been found yet.
    dialog.replaceButton()->click();
    CHECK(selectedRows(window) == std::vector<int>{0});
    CHECK(textAt(window, 0) == "Bonjour Marie.");

    dialog.replaceButton()->click();
    CHECK(textAt(window, 0) == "Salut Marie.");
    CHECK(selectedRows(window) == std::vector<int>{2});
    CHECK(window.undoAction()->text().toStdString() == "Undo: replacing");

    // A tag cut the word: it now covers the whole of the replacement.
    dialog.replaceButton()->click();
    CHECK(textAt(window, 2) == "<i>Salut</i>.");

    window.undoAction()->trigger();
    CHECK(textAt(window, 2) == "<i>Bon</i>jour.");
}

TEST_CASE("replace all is one entry in the history, and says how many", "[gui][GUI-SEARCH-01]") {
    InMemoryFileSystem files = withFour();
    FakePrompts prompts;
    MainWindow window{files, fourIn(files), prompts};
    window.show();
    const SearchDialog& dialog = searching(window, "marie");
    dialog.replacementField()->setText(QStringLiteral("Sophie"));

    dialog.replaceAllButton()->click();

    CHECK(textAt(window, 0) == "Bonjour Sophie.");
    CHECK(textAt(window, 3) == "Sophie et Sophie.");
    CHECK(statusOf(dialog) == "replaced 3 matches");
    CHECK(window.undoAction()->text().toStdString() == "Undo: replacing all");

    window.undoAction()->trigger();

    CHECK(textAt(window, 0) == "Bonjour Marie.");
    CHECK(textAt(window, 3) == "Marie et marie.");
    CHECK_FALSE(window.undoAction()->isEnabled());
}

TEST_CASE("the search stays in the selection it started from", "[gui][GUI-SEARCH-02]") {
    // Moving to a match changes the selection; the target does not follow it.
    InMemoryFileSystem files = withFour();
    FakePrompts prompts;
    MainWindow window{files, fourIn(files), prompts};
    window.show();
    selectRow(window, 2);
    selectRow(window, 3);
    const SearchDialog& dialog = searching(window, "marie");

    dialog.nextButton()->click();
    CHECK(selectedRows(window) == std::vector<int>{3});
    dialog.nextButton()->click();
    dialog.nextButton()->click();
    // Round again inside the target, never out to the first subtitle.
    CHECK(selectedRows(window) == std::vector<int>{3});

    dialog.replacementField()->setText(QStringLiteral("Sophie"));
    dialog.replaceAllButton()->click();
    CHECK(textAt(window, 0) == "Bonjour Marie.");
    CHECK(statusOf(dialog) == "replaced 2 matches");
}

TEST_CASE("with nothing selected, the search walks the whole document", "[gui][GUI-SEARCH-02]") {
    InMemoryFileSystem files = withFour();
    FakePrompts prompts;
    MainWindow window{files, fourIn(files), prompts};
    window.show();
    const SearchDialog& dialog = searching(window, "Marie");
    dialog.ignoreCaseCheck()->setChecked(false);
    dialog.replacementField()->setText(QStringLiteral("Sophie"));

    dialog.replaceAllButton()->click();

    CHECK(statusOf(dialog) == "replaced 2 matches");
    CHECK(textAt(window, 3) == "Sophie et marie.");
}

TEST_CASE("a broken expression is said, and nothing is searched", "[gui][GUI-SEARCH-01]") {
    InMemoryFileSystem files = withFour();
    FakePrompts prompts;
    MainWindow window{files, fourIn(files), prompts};
    window.show();
    const SearchDialog& dialog = searching(window, "(Marie");
    dialog.regexCheck()->setChecked(true);

    dialog.nextButton()->click();

    CHECK(statusOf(dialog).starts_with("not a regular expression ("));
    CHECK(selectedRows(window).empty());

    // The same answer from the two gestures that write, and nothing written.
    dialog.statusLabel()->clear();
    dialog.replaceButton()->click();
    CHECK(statusOf(dialog).starts_with("not a regular expression ("));

    dialog.statusLabel()->clear();
    dialog.replaceAllButton()->click();
    CHECK(statusOf(dialog).starts_with("not a regular expression ("));
    CHECK(textAt(window, 0) == "Bonjour Marie.");
    CHECK_FALSE(window.undoAction()->isEnabled());
}

TEST_CASE("with nothing to look for, the four gestures are out", "[gui][GUI-SEARCH-01]") {
    InMemoryFileSystem files = withFour();
    FakePrompts prompts;
    MainWindow window{files, fourIn(files), prompts};
    window.show();
    const SearchDialog& dialog = searching(window, "");

    CHECK_FALSE(dialog.nextButton()->isEnabled());
    CHECK_FALSE(dialog.previousButton()->isEnabled());
    CHECK_FALSE(dialog.replaceButton()->isEnabled());
    CHECK_FALSE(dialog.replaceAllButton()->isEnabled());
}

TEST_CASE("the two options are kept by the preferences", "[gui][GUI-SEARCH-01]") {
    InMemoryFileSystem files = withFour();
    FakePrompts prompts;
    MainWindow window{files, fourIn(files), prompts};
    window.applySettings(
        subedit::core::Settings{.search = SearchOptions{.regex = true, .ignoreCase = false}});
    window.show();

    const SearchDialog& dialog = searching(window, "Marie");
    CHECK(dialog.regexCheck()->isChecked());
    CHECK_FALSE(dialog.ignoreCaseCheck()->isChecked());

    dialog.ignoreCaseCheck()->setChecked(true);
    CHECK(window.settings().search == SearchOptions{.regex = true, .ignoreCase = true});
}

TEST_CASE("an empty document has nothing to search", "[gui][GUI-SEARCH-01]") {
    InMemoryFileSystem files;
    FakePrompts prompts;
    const MainWindow window{files, OpenedFile{}, prompts};

    CHECK_FALSE(window.findAndReplaceAction()->isEnabled());
}

TEST_CASE("preferences read while the dialog is open reach it", "[gui][GUI-SEARCH-01]") {
    InMemoryFileSystem files = withFour();
    FakePrompts prompts;
    MainWindow window{files, fourIn(files), prompts};
    window.show();
    const SearchDialog& dialog = searching(window, "Marie");

    window.applySettings(
        subedit::core::Settings{.search = SearchOptions{.regex = true, .ignoreCase = false}});

    CHECK(dialog.regexCheck()->isChecked());
    CHECK_FALSE(dialog.ignoreCaseCheck()->isChecked());
}

TEST_CASE("an undo that resets the model forgets a stale search target", "[gui][GUI-SEARCH-03]") {
    // Four subtitles; select the fourth, split it into a fifth. The search
    // then captures a target of {3, 4} before the split is undone.
    InMemoryFileSystem files = withFour();
    FakePrompts prompts;
    MainWindow window{files, fourIn(files), prompts};
    window.show();

    selectRow(window, 3);
    window.splitAction()->trigger();
    CHECK(selectedRows(window) == std::vector<int>{3, 4});

    const SearchDialog& dialog = searching(window, "marie");
    dialog.nextButton()->click();
    CHECK(selectedRows(window) == std::vector<int>{3});

    // The split is undone: back to four subtitles, and the model was reset
    // rather than told which rows changed — Qt clears the selection without a
    // `selectionChanged`.
    window.undoAction()->trigger();

    // Before the fix, this throws `std::out_of_range` out of `spansAt`.
    // With the fix, the search target and match are reset, so the search
    // starts from the beginning of the document.
    dialog.nextButton()->click();
    CHECK(selectedRows(window) == std::vector<int>{0});
}
