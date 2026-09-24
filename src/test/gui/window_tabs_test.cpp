// Several projects in one window, in tabs — issue #437, ADR 0033,
// `GUI-TABS-01`.
//
// The player behind the window is a double, for the reason `window_player_test.cpp`
// gives: a real libmpv handed an offscreen surface draws nowhere and refuses
// the file. What is under test here is everything a switch of tab has to put
// back in step — history, selection, video — and the three gestures that
// name a tab: opening, closing, and opening what is already open.

#include <subedit/core/format/project_file.hpp>
#include <subedit/core/io/in_memory_file_system.hpp>
#include <subedit/core/video/video_player.hpp>
#include <subedit/gui/insert_dialog.hpp>
#include <subedit/gui/main_window.hpp>
#include <subedit/gui/subtitle_table.hpp>
#include <subedit/gui/unsaved_documents_dialog.hpp>

#include <QAbstractItemModel>
#include <QAction>
#include <QCheckBox>
#include <QDialog>
#include <QItemSelectionModel>
#include <QKeySequence>
#include <QPushButton>
#include <QSpinBox>
#include <QStatusBar>
#include <QString>
#include <QTabBar>
#include <catch2/catch_test_macros.hpp>

#include <cstdint>
#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "fake_prompts.hpp"
#include "fake_video_player.hpp"

namespace {

using subedit::core::InMemoryFileSystem;
using subedit::core::OpenedFile;
using subedit::core::openProject;
using subedit::core::VideoPlayer;
using subedit::gui::InsertDialog;
using subedit::gui::MainWindow;
using subedit::gui::PlayerFactory;
using subedit::gui::SaveTarget;
using subedit::gui::UnsavedChoice;
using subedit::gui::UnsavedDocumentsDialog;
using subedit::test::FakePrompts;
using subedit::test::FakeVideoPlayer;

constexpr int kTextColumn = 4;

constexpr const char* kFirst = "1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n"
                               "2\n00:00:03,000 --> 00:00:04,000\nDeux.\n\n";

constexpr const char* kSecond = "1\n00:00:05,000 --> 00:00:06,000\nTrois.\n\n"
                                "2\n00:00:07,000 --> 00:00:08,000\nQuatre.\n\n";

/// What a case says about the players to come, and what came out — the same
/// double `window_player_test.cpp` uses.
struct Projectionist {
    FakeVideoPlayer* player = nullptr;
    int built = 0;
};

[[nodiscard]] PlayerFactory projecting(Projectionist& booth) {
    return [&booth](std::uintptr_t) -> std::unique_ptr<VideoPlayer> {
        ++booth.built;
        auto made = std::make_unique<FakeVideoPlayer>();
        booth.player = made.get();
        return made;
    };
}

[[nodiscard]] InMemoryFileSystem withTwoFilms() {
    InMemoryFileSystem files;
    files.addFile("premier.srt", kFirst);
    files.addFile("second.srt", kSecond);
    files.addFile("premier.mkv", "");
    files.addFile("second.mkv", "");
    return files;
}

[[nodiscard]] OpenedFile fileIn(const InMemoryFileSystem& files, const char* path) {
    auto opened = openProject(files, path);
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

[[nodiscard]] bool edit(const MainWindow& window, int row, const char* typed) {
    return window.table()->model()->setData(
        window.table()->model()->index(row, kTextColumn), QString::fromUtf8(typed), Qt::EditRole);
}

void selectRow(const MainWindow& window, int row) {
    window.table()->selectionModel()->setCurrentIndex(window.table()->model()->index(row, 0),
                                                      QItemSelectionModel::ClearAndSelect |
                                                          QItemSelectionModel::Rows);
}

[[nodiscard]] int currentRow(const MainWindow& window) {
    return window.table()->currentIndex().row();
}

} // namespace

TEST_CASE("opening a second file opens a second tab, and the first stays put",
          "[gui][GUI-TABS-01]") {
    InMemoryFileSystem files = withTwoFilms();
    FakePrompts prompts;
    MainWindow window{files, fileIn(files, "premier.srt"), prompts};
    window.show();
    prompts.nextFileToOpen = "second.srt";

    window.openAction()->trigger();

    REQUIRE(window.tabBar()->count() == 2);
    CHECK(window.tabBar()->currentIndex() == 1);
    CHECK(textAt(window, 0) == "Trois.");

    window.tabBar()->setCurrentIndex(0);
    CHECK(textAt(window, 0) == "Un.");
}

TEST_CASE("each tab keeps its own history: undoing in one does not touch the other",
          "[gui][GUI-TABS-01]") {
    InMemoryFileSystem files = withTwoFilms();
    FakePrompts prompts;
    MainWindow window{files, fileIn(files, "premier.srt"), prompts};
    window.show();
    prompts.nextFileToOpen = "second.srt";
    window.openAction()->trigger();
    REQUIRE(edit(window, 0, "Trois bis."));
    REQUIRE(window.undoAction()->isEnabled());

    window.tabBar()->setCurrentIndex(0);

    // The first tab never had anything to undo: the action follows the tab.
    CHECK_FALSE(window.undoAction()->isEnabled());
    CHECK(textAt(window, 0) == "Un.");

    window.tabBar()->setCurrentIndex(1);
    CHECK(window.undoAction()->isEnabled());
    CHECK(textAt(window, 0) == "Trois bis.");
}

TEST_CASE("each tab keeps its own selection", "[gui][GUI-TABS-01]") {
    InMemoryFileSystem files = withTwoFilms();
    FakePrompts prompts;
    MainWindow window{files, fileIn(files, "premier.srt"), prompts};
    window.show();
    selectRow(window, 1);
    prompts.nextFileToOpen = "second.srt";
    window.openAction()->trigger();
    selectRow(window, 0);
    REQUIRE(currentRow(window) == 0);

    window.tabBar()->setCurrentIndex(0);

    CHECK(currentRow(window) == 1);
}

TEST_CASE("each tab keeps its own video, and switching reopens the shared player",
          "[gui][GUI-TABS-01]") {
    InMemoryFileSystem files = withTwoFilms();
    FakePrompts prompts;
    Projectionist booth;
    MainWindow window{files, fileIn(files, "premier.srt"), prompts, projecting(booth)};
    window.show();
    window.selectVideoAction()->trigger(); // no video chosen: nothing to select yet
    prompts.nextVideoToOpen = "premier.mkv";
    window.selectVideoAction()->trigger();
    REQUIRE(booth.player != nullptr);
    REQUIRE(booth.player->opened == std::vector<std::filesystem::path>{"premier.mkv"});

    prompts.nextFileToOpen = "second.srt";
    window.openAction()->trigger();
    prompts.nextVideoToOpen = "second.mkv";
    window.selectVideoAction()->trigger();
    CHECK(booth.player->opened.back() == "second.mkv");

    window.tabBar()->setCurrentIndex(0);

    // The shared player is showing the second tab's film: coming back to the
    // first must reopen its own, even though this tab's own association has
    // not changed since it was last shown.
    CHECK(booth.player->opened.back() == "premier.mkv");
}

TEST_CASE("opening a file already open focuses its tab, and says so", "[gui][GUI-TABS-01]") {
    InMemoryFileSystem files = withTwoFilms();
    FakePrompts prompts;
    MainWindow window{files, fileIn(files, "premier.srt"), prompts};
    window.show();
    prompts.nextFileToOpen = "second.srt";
    window.openAction()->trigger();
    REQUIRE(window.tabBar()->count() == 2);

    prompts.nextFileToOpen = "premier.srt";
    window.openAction()->trigger();

    // Nothing new opened: still two tabs, the first one now current.
    CHECK(window.tabBar()->count() == 2);
    CHECK(window.tabBar()->currentIndex() == 0);
    CHECK(window.statusBar()->currentMessage().toStdString() == "premier.srt: already open");
}

TEST_CASE("a path spelled differently still names the same open file", "[gui][GUI-TABS-01]") {
    InMemoryFileSystem files = withTwoFilms();
    FakePrompts prompts;
    MainWindow window{files, fileIn(files, "premier.srt"), prompts};
    window.show();
    prompts.nextFileToOpen = "./premier.srt";

    window.openAction()->trigger();

    CHECK(window.tabBar()->count() == 1);
}

TEST_CASE("File New opens an empty project in a new tab", "[gui][GUI-TABS-01]") {
    InMemoryFileSystem files = withTwoFilms();
    FakePrompts prompts;
    MainWindow window{files, fileIn(files, "premier.srt"), prompts};
    window.show();

    window.newProjectAction()->trigger();

    REQUIRE(window.tabBar()->count() == 2);
    CHECK(window.tabBar()->currentIndex() == 1);
    CHECK(window.tabBar()->tabText(1).toStdString() == "untitled");
    CHECK(window.table()->model()->rowCount({}) == 0);
}

TEST_CASE("Close is out with one tab, and lit with two", "[gui][GUI-TABS-01]") {
    InMemoryFileSystem files = withTwoFilms();
    FakePrompts prompts;
    MainWindow window{files, fileIn(files, "premier.srt"), prompts};
    window.show();
    CHECK_FALSE(window.closeProjectAction()->isEnabled());

    prompts.nextFileToOpen = "second.srt";
    window.openAction()->trigger();

    CHECK(window.closeProjectAction()->isEnabled());
}

TEST_CASE("File Close takes the current tab away, and switches to its neighbour",
          "[gui][GUI-TABS-01]") {
    InMemoryFileSystem files = withTwoFilms();
    FakePrompts prompts;
    MainWindow window{files, fileIn(files, "premier.srt"), prompts};
    window.show();
    prompts.nextFileToOpen = "second.srt";
    window.openAction()->trigger();
    REQUIRE(window.tabBar()->count() == 2);

    window.closeProjectAction()->trigger();

    CHECK(window.tabBar()->count() == 1);
    CHECK(textAt(window, 0) == "Un.");
}

TEST_CASE("closing a tab with unsaved changes asks, and cancelling keeps it",
          "[gui][GUI-TABS-01]") {
    InMemoryFileSystem files = withTwoFilms();
    FakePrompts prompts;
    MainWindow window{files, fileIn(files, "premier.srt"), prompts};
    window.show();
    prompts.nextFileToOpen = "second.srt";
    window.openAction()->trigger();
    REQUIRE(edit(window, 0, "Trois bis."));
    prompts.nextUnsavedChoice = UnsavedChoice::Cancel;

    window.closeProjectAction()->trigger();

    CHECK(prompts.unsavedAsked == 1);
    CHECK(window.tabBar()->count() == 2);
    CHECK(textAt(window, 0) == "Trois bis.");
}

TEST_CASE("Ctrl+PageDown and Ctrl+PageUp move between tabs, and wrap around",
          "[gui][GUI-TABS-01]") {
    InMemoryFileSystem files = withTwoFilms();
    FakePrompts prompts;
    MainWindow window{files, fileIn(files, "premier.srt"), prompts};
    window.show();
    prompts.nextFileToOpen = "second.srt";
    window.openAction()->trigger();
    REQUIRE(window.tabBar()->currentIndex() == 1);
    CHECK(window.nextTabAction()->shortcut() == QKeySequence{QStringLiteral("Ctrl+PgDown")});
    CHECK(window.previousTabAction()->shortcut() == QKeySequence{QStringLiteral("Ctrl+PgUp")});

    window.nextTabAction()->trigger();
    CHECK(window.tabBar()->currentIndex() == 0);

    window.previousTabAction()->trigger();
    CHECK(window.tabBar()->currentIndex() == 1);
}

TEST_CASE("copying in one tab and pasting in another carries the text across",
          "[gui][GUI-TABS-01]") {
    InMemoryFileSystem files = withTwoFilms();
    FakePrompts prompts;
    MainWindow window{files, fileIn(files, "premier.srt"), prompts};
    window.show();
    selectRow(window, 0);
    window.copyAction()->trigger();

    prompts.nextFileToOpen = "second.srt";
    window.openAction()->trigger();
    selectRow(window, 1);

    window.pasteAction()->trigger();

    CHECK(textAt(window, 1) == "Un.");
}

TEST_CASE("a tab says it is modified, and stops saying so once saved", "[gui][GUI-TABS-03]") {
    InMemoryFileSystem files = withTwoFilms();
    FakePrompts prompts;
    MainWindow window{files, fileIn(files, "premier.srt"), prompts};
    window.show();
    CHECK(window.tabBar()->tabText(0).toStdString() == "premier.srt");

    REQUIRE(edit(window, 0, "Un bis."));
    CHECK(window.tabBar()->tabText(0).toStdString() == "premier.srt*");

    prompts.nextFileToOpen = "second.srt";
    window.openAction()->trigger();
    // The other tab keeps saying what it said when it was left.
    CHECK(window.tabBar()->tabText(0).toStdString() == "premier.srt*");
    CHECK(window.tabBar()->tabText(1).toStdString() == "second.srt");

    window.tabBar()->setCurrentIndex(0);
    window.saveAction()->trigger();
    CHECK(window.tabBar()->tabText(0).toStdString() == "premier.srt");
}

namespace {

/// Three modified projects: `premier.srt`, a new one with no file, `second.srt`.
void openThreeModified(MainWindow& window, FakePrompts& prompts) {
    REQUIRE(edit(window, 0, "Un bis."));
    window.newProjectAction()->trigger();
    // A project with no file, made modified by giving it a subtitle.
    prompts.nextRun = true;
    prompts.fill = [](QDialog& dialog) {
        dynamic_cast<InsertDialog&>(dialog).countBox()->setValue(1);
    };
    window.insertAction()->trigger();
    REQUIRE(window.tabBar()->tabText(1).toStdString() == "untitled*");
    prompts.nextRun = false;
    prompts.fill = nullptr;
    prompts.nextFileToOpen = "second.srt";
    window.openAction()->trigger();
    REQUIRE(edit(window, 0, "Trois bis."));
    REQUIRE(window.tabBar()->count() == 3);
}

} // namespace

TEST_CASE("Save All writes every modified project and asks a name for the one with none",
          "[gui][GUI-SAVE-04]") {
    InMemoryFileSystem files = withTwoFilms();
    FakePrompts prompts;
    MainWindow window{files, fileIn(files, "premier.srt"), prompts};
    window.show();
    openThreeModified(window, prompts);
    prompts.nextSaveTarget = SaveTarget{.path = "nouveau.srt"};

    window.saveAllDocumentsAction()->trigger();

    CHECK(files.contentOf("premier.srt").value_or("").find("Un bis.") != std::string::npos);
    CHECK(files.contentOf("second.srt").value_or("").find("Trois bis.") != std::string::npos);
    CHECK(files.contentOf("nouveau.srt").has_value());
    CHECK(prompts.saveTargetAsked == 1);
    // Back on the tab it was fired from.
    CHECK(window.tabBar()->currentIndex() == 2);
    CHECK(window.tabBar()->tabText(0).toStdString() == "premier.srt");
    CHECK(window.tabBar()->tabText(1).toStdString() == "nouveau.srt");
}

TEST_CASE("Save All writes every project behind its tab, and the player opens no film",
          "[gui][GUI-SAVE-04]") {
    InMemoryFileSystem files = withTwoFilms();
    FakePrompts prompts;
    Projectionist booth;
    MainWindow window{files, fileIn(files, "premier.srt"), prompts, projecting(booth)};
    window.show();
    REQUIRE(edit(window, 0, "Un bis."));
    prompts.nextFileToOpen = "second.srt";
    window.openAction()->trigger();
    REQUIRE(edit(window, 0, "Trois bis."));
    window.tabBar()->setCurrentIndex(0);
    REQUIRE(booth.player != nullptr);
    const std::vector<std::filesystem::path> before = booth.player->opened;
    REQUIRE(before.back() == "premier.mkv");

    window.saveAllDocumentsAction()->trigger();

    CHECK(files.contentOf("second.srt").value_or("").find("Trois bis.") != std::string::npos);
    // Issue #461: the second project was written without being shown, so the
    // shared player was never sent to its film and back.
    CHECK(booth.player->opened == before);
    CHECK(window.tabBar()->currentIndex() == 0);
    CHECK(window.tabBar()->tabText(0).toStdString() == "premier.srt");
    CHECK(window.tabBar()->tabText(1).toStdString() == "second.srt");
}

TEST_CASE("Save All stops at a Save As that is given up, and says what was written",
          "[gui][GUI-SAVE-04]") {
    InMemoryFileSystem files = withTwoFilms();
    FakePrompts prompts;
    MainWindow window{files, fileIn(files, "premier.srt"), prompts};
    window.show();
    openThreeModified(window, prompts);
    prompts.nextSaveTarget.reset();

    window.saveAllDocumentsAction()->trigger();

    CHECK(files.contentOf("premier.srt").value_or("").find("Un bis.") != std::string::npos);
    // The third project is after the one that was given up: not reached.
    CHECK(files.contentOf("second.srt").value_or("") == kSecond);
    REQUIRE(prompts.outcomes.size() == 1);
    CHECK(prompts.outcomes.front().find("1 of 3") != std::string::npos);
    CHECK(window.tabBar()->currentIndex() == 2);
}

TEST_CASE("closing the window asks once for every modified project", "[gui][GUI-TABS-02]") {
    InMemoryFileSystem files = withTwoFilms();
    FakePrompts prompts;
    MainWindow window{files, fileIn(files, "premier.srt"), prompts};
    window.show();
    REQUIRE(edit(window, 0, "Un bis."));
    prompts.nextFileToOpen = "second.srt";
    window.openAction()->trigger();
    REQUIRE(edit(window, 0, "Trois bis."));
    std::size_t boxes = 0;
    prompts.nextRun = true;
    prompts.fill = [&](QDialog& dialog) {
        if (auto* list = dynamic_cast<UnsavedDocumentsDialog*>(&dialog)) {
            boxes = static_cast<std::size_t>(list->boxes().size());
            list->discardButton()->click();
        }
    };

    CHECK(window.close());

    CHECK(prompts.runAsked == 1);
    CHECK(prompts.unsavedAsked == 0);
    CHECK(boxes == 2);
}

TEST_CASE("closing all: saving the ticked documents writes those and only those",
          "[gui][GUI-TABS-02]") {
    InMemoryFileSystem files = withTwoFilms();
    FakePrompts prompts;
    MainWindow window{files, fileIn(files, "premier.srt"), prompts};
    window.show();
    REQUIRE(edit(window, 0, "Un bis."));
    prompts.nextFileToOpen = "second.srt";
    window.openAction()->trigger();
    REQUIRE(edit(window, 0, "Trois bis."));
    prompts.nextRun = true;
    prompts.fill = [&](QDialog& dialog) {
        if (auto* list = dynamic_cast<UnsavedDocumentsDialog*>(&dialog)) {
            list->boxes().at(0)->setChecked(false);
            list->saveButton()->click();
        }
    };

    window.closeAllProjectsAction()->trigger();

    CHECK(files.contentOf("premier.srt").value_or("") == kFirst);
    CHECK(files.contentOf("second.srt").value_or("").find("Trois bis.") != std::string::npos);
    CHECK_FALSE(window.isVisible());
}

TEST_CASE("closing all without saving loses every project's changes and closes",
          "[gui][GUI-TABS-02]") {
    InMemoryFileSystem files = withTwoFilms();
    FakePrompts prompts;
    MainWindow window{files, fileIn(files, "premier.srt"), prompts};
    window.show();
    REQUIRE(edit(window, 0, "Un bis."));
    prompts.nextFileToOpen = "second.srt";
    window.openAction()->trigger();
    REQUIRE(edit(window, 0, "Trois bis."));
    prompts.nextRun = true;
    prompts.fill = [&](QDialog& dialog) {
        if (auto* list = dynamic_cast<UnsavedDocumentsDialog*>(&dialog))
            list->discardButton()->click();
    };

    window.closeAllProjectsAction()->trigger();

    CHECK(files.contentOf("premier.srt").value_or("") == kFirst);
    CHECK(files.contentOf("second.srt").value_or("") == kSecond);
    CHECK_FALSE(window.isVisible());
}

TEST_CASE("cancelling the question of closing all leaves every project open",
          "[gui][GUI-TABS-02]") {
    InMemoryFileSystem files = withTwoFilms();
    FakePrompts prompts;
    MainWindow window{files, fileIn(files, "premier.srt"), prompts};
    window.show();
    REQUIRE(edit(window, 0, "Un bis."));
    prompts.nextFileToOpen = "second.srt";
    window.openAction()->trigger();
    REQUIRE(edit(window, 0, "Trois bis."));
    prompts.nextRun = false;

    window.closeAllProjectsAction()->trigger();

    CHECK(window.isVisible());
    CHECK(window.tabBar()->count() == 2);
    CHECK(files.contentOf("premier.srt").value_or("") == kFirst);
}

TEST_CASE("one modified project among several asks the plain question, about its tab",
          "[gui][GUI-TABS-02]") {
    InMemoryFileSystem files = withTwoFilms();
    FakePrompts prompts;
    MainWindow window{files, fileIn(files, "premier.srt"), prompts};
    window.show();
    prompts.nextFileToOpen = "second.srt";
    window.openAction()->trigger();
    window.tabBar()->setCurrentIndex(0);
    REQUIRE(edit(window, 0, "Un bis."));
    window.tabBar()->setCurrentIndex(1);
    prompts.nextUnsavedChoice = UnsavedChoice::Cancel;

    CHECK_FALSE(window.close());

    CHECK(prompts.unsavedAsked == 1);
    CHECK(prompts.runAsked == 0);
}

TEST_CASE("discarding the one modified document closes the window without writing it",
          "[gui][GUI-TABS-02]") {
    InMemoryFileSystem files = withTwoFilms();
    FakePrompts prompts;
    MainWindow window{files, fileIn(files, "premier.srt"), prompts};
    window.show();
    REQUIRE(edit(window, 0, "Un bis."));
    prompts.nextUnsavedChoice = UnsavedChoice::Discard;

    CHECK(window.close());

    CHECK(prompts.unsavedAsked == 1);
    CHECK(files.contentOf("premier.srt").value_or("") == kFirst);
}
