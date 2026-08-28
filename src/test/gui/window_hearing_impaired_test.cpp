// Removing the mentions from the window — issue #133.
//
// **The one road of the phase that makes lines disappear**, therefore the only
// one that exercises the removal and the putting back an undo calls for.

#include <subedit/core/format/project_file.hpp>
#include <subedit/core/io/in_memory_file_system.hpp>
#include <subedit/gui/main_window.hpp>
#include <subedit/gui/operation_dialog.hpp>

#include <QAbstractItemModel>
#include <QAction>
#include <QItemSelectionModel>
#include <QTableView>
#include <catch2/catch_test_macros.hpp>

#include <string>
#include <utility>

#include "fake_prompts.hpp"

namespace {

using subedit::core::InMemoryFileSystem;
using subedit::core::OpenedFile;
using subedit::core::openProject;
using subedit::gui::MainWindow;
using subedit::gui::OperationDialog;
using subedit::test::FakePrompts;

/// A real file in miniature: one clean subtitle, one carrying a mention in the
/// middle, one that is nothing but a mention, one clean.
constexpr const char* kFour = "1\n00:00:01,000 --> 00:00:02,000\nBonjour.\n\n"
                              "2\n00:00:03,000 --> 00:00:04,000\nAttends [il tousse] Marie\n\n"
                              "3\n00:00:05,000 --> 00:00:06,000\n[Bruit de pas]\n\n"
                              "4\n00:00:07,000 --> 00:00:08,000\nAu revoir.\n\n";

/// No mention anywhere.
constexpr const char* kClean = "1\n00:00:01,000 --> 00:00:02,000\nBonjour.\n\n"
                               "2\n00:00:03,000 --> 00:00:04,000\nAu revoir.\n\n";

[[nodiscard]] InMemoryFileSystem withFile(const char* content) {
    InMemoryFileSystem files;
    files.addFile("film.srt", content);
    return files;
}

[[nodiscard]] OpenedFile fileIn(const InMemoryFileSystem& files) {
    auto opened = openProject(files, "film.srt");
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

TEST_CASE("removing mentions cleans what it can and takes away what it empties",
          "[gui][GUI-HEARING-01]") {
    InMemoryFileSystem files = withFile(kFour);
    FakePrompts prompts;
    prompts.nextRun = true;
    MainWindow window{files, fileIn(files), prompts};
    window.show();

    window.hearingImpairedAction()->trigger();

    REQUIRE(window.table()->model()->rowCount({}) == 3);
    CHECK(textAt(window, 0) == "Bonjour.");
    CHECK(textAt(window, 1) == "Attends Marie");
    CHECK(textAt(window, 2) == "Au revoir.");
}

TEST_CASE("the removal reports what it did", "[gui][GUI-HEARING-02]") {
    InMemoryFileSystem files = withFile(kFour);
    FakePrompts prompts;
    prompts.nextRun = true;
    MainWindow window{files, fileIn(files), prompts};
    window.show();

    window.hearingImpairedAction()->trigger();

    REQUIRE(prompts.outcomes.size() == 1);
    CHECK(prompts.outcomes.at(0) == "1 subtitle cleaned, 1 removed");
}

TEST_CASE("removing mentions from a selection leaves the rest alone", "[gui][GUI-HEARING-01]") {
    InMemoryFileSystem files = withFile(kFour);
    FakePrompts prompts;
    prompts.nextRun = true;
    MainWindow window{files, fileIn(files), prompts};
    window.show();
    selectRow(window, 1);

    window.hearingImpairedAction()->trigger();

    // The third would have been emptied, but it was not aimed at.
    REQUIRE(window.table()->model()->rowCount({}) == 4);
    CHECK(textAt(window, 1) == "Attends Marie");
    CHECK(textAt(window, 2) == "[Bruit de pas]");
}

TEST_CASE("a removal that changes nothing says so and stays out of the history",
          "[gui][GUI-HEARING-02]") {
    // An operation that changes nothing is not an operation to undo.
    InMemoryFileSystem files = withFile(kClean);
    FakePrompts prompts;
    prompts.nextRun = true;
    MainWindow window{files, fileIn(files), prompts};
    window.show();

    window.hearingImpairedAction()->trigger();

    REQUIRE(prompts.outcomes.size() == 1);
    CHECK(prompts.outcomes.at(0) == "no mention to remove");
    CHECK_FALSE(window.undoAction()->isEnabled());
    CHECK_FALSE(window.isWindowModified());
}

TEST_CASE("giving up on the confirmation removes nothing", "[gui][GUI-HEARING-01]") {
    InMemoryFileSystem files = withFile(kFour);
    FakePrompts prompts;
    prompts.nextRun = false;
    MainWindow window{files, fileIn(files), prompts};
    window.show();

    window.hearingImpairedAction()->trigger();

    CHECK(prompts.runAsked == 1);
    CHECK(window.table()->model()->rowCount({}) == 4);
    CHECK(prompts.outcomes.empty());
}

TEST_CASE("undoing a removal puts the subtitles back, with their own text",
          "[gui][GUI-HEARING-01]") {
    // What ADR 0019 accepts: a change of structure resets the table. What it
    // does not cost: the content, which comes back untouched.
    InMemoryFileSystem files = withFile(kFour);
    FakePrompts prompts;
    prompts.nextRun = true;
    MainWindow window{files, fileIn(files), prompts};
    window.show();
    window.hearingImpairedAction()->trigger();
    REQUIRE(window.table()->model()->rowCount({}) == 3);

    window.undoAction()->trigger();

    REQUIRE(window.table()->model()->rowCount({}) == 4);
    CHECK(textAt(window, 1) == "Attends [il tousse] Marie");
    CHECK(textAt(window, 2) == "[Bruit de pas]");
    CHECK_FALSE(window.isWindowModified());
}

TEST_CASE("the confirmation names the selection, and not the file", "[gui][GUI-HEARING-01]") {
    // The target **is** the question of this dialog: it has nothing else to
    // ask. A count announcing the whole file while the operation applies to two
    // rows makes the confirmation say the opposite of what it confirms.
    InMemoryFileSystem files = withFile(kFour);
    FakePrompts prompts;
    prompts.nextRun = false;
    std::string said;
    prompts.fill = [&said](QDialog& dialog) {
        said = dynamic_cast<OperationDialog&>(dialog).targetLabel().toStdString();
    };
    MainWindow window{files, fileIn(files), prompts};
    window.show();
    selectRow(window, 1);
    selectRow(window, 2);

    window.hearingImpairedAction()->trigger();

    CHECK(said == "2 subtitles");
}

TEST_CASE("the removal is not offered on an empty file", "[gui][GUI-HEARING-01]") {
    InMemoryFileSystem files;
    FakePrompts prompts;
    MainWindow window{files, OpenedFile{}, prompts};
    window.show();

    CHECK_FALSE(window.hearingImpairedAction()->isEnabled());
}
