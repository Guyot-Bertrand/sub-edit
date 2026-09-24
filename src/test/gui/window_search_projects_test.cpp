// Searching every open project — issue #440, decision D8, `GUI-SEARCH-04`.
//
// The search itself is the core's, and one project at a time is proved in
// `window_search_test.cpp`. What is under test here is what crossing the tabs
// adds: the order they are visited in, the round that says it wrapped, and a
// `Replace All` that is one entry of history per project it touched.

#include <subedit/core/format/project_file.hpp>
#include <subedit/core/io/in_memory_file_system.hpp>
#include <subedit/core/video/video_player.hpp>
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
#include <QTabBar>
#include <catch2/catch_test_macros.hpp>

#include <cstdint>
#include <filesystem>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "fake_prompts.hpp"
#include "fake_video_player.hpp"

namespace {

using subedit::core::InMemoryFileSystem;
using subedit::core::openProject;
using subedit::core::VideoPlayer;
using subedit::gui::MainWindow;
using subedit::gui::PlayerFactory;
using subedit::gui::SearchDialog;
using subedit::test::FakePrompts;
using subedit::test::FakeVideoPlayer;

constexpr int kTextColumn = 4;

// « marie » once in the first project, twice in the second, once in the third.
constexpr const char* kFirst = "1\n00:00:01,000 --> 00:00:02,000\nRien.\n\n"
                               "2\n00:00:03,000 --> 00:00:04,000\nBonjour Marie.\n\n";

constexpr const char* kSecond = "1\n00:00:01,000 --> 00:00:02,000\nMarie.\n\n"
                                "2\n00:00:03,000 --> 00:00:04,000\nRien.\n\n"
                                "3\n00:00:05,000 --> 00:00:06,000\nEt marie.\n\n";

constexpr const char* kThird = "1\n00:00:01,000 --> 00:00:02,000\nRien.\n\n"
                               "2\n00:00:03,000 --> 00:00:04,000\nAdieu Marie.\n\n";

constexpr const char* kNone = "1\n00:00:01,000 --> 00:00:02,000\nRien.\n\n";

[[nodiscard]] InMemoryFileSystem filesystem() {
    InMemoryFileSystem files;
    files.addFile("premier.srt", kFirst);
    files.addFile("second.srt", kSecond);
    files.addFile("troisieme.srt", kThird);
    files.addFile("aucun.srt", kNone);
    return files;
}

[[nodiscard]] subedit::core::OpenedFile fileIn(const InMemoryFileSystem& files, const char* path) {
    auto opened = openProject(files, path);
    REQUIRE(opened.has_value());
    return std::move(*opened);
}

/// The window on the first project, the two others open after it, and the
/// first tab shown.
void openThree(MainWindow& window, FakePrompts& prompts) {
    prompts.nextFileToOpen = "second.srt";
    window.openAction()->trigger();
    prompts.nextFileToOpen = "troisieme.srt";
    window.openAction()->trigger();
    REQUIRE(window.tabBar()->count() == 3);
    window.tabBar()->setCurrentIndex(0);
}

[[nodiscard]] SearchDialog& searchingAll(MainWindow& window, const char* pattern) {
    window.findAndReplaceAction()->trigger();
    SearchDialog* dialog = window.searchDialog();
    REQUIRE(dialog != nullptr);
    dialog->patternField()->setText(QString::fromUtf8(pattern));
    dialog->allProjectsCheck()->setChecked(true);
    return *dialog;
}

[[nodiscard]] std::string textAt(const MainWindow& window, int row) {
    return window.table()
        ->model()
        ->data(window.table()->model()->index(row, kTextColumn), Qt::DisplayRole)
        .toString()
        .toStdString();
}

[[nodiscard]] std::vector<int> selectedRows(const MainWindow& window) {
    std::vector<int> rows;
    for (const QModelIndex& index : window.table()->selectionModel()->selectedRows())
        rows.push_back(index.row());
    return rows;
}

[[nodiscard]] std::string statusOf(const SearchDialog& dialog) {
    return dialog.statusLabel()->text().toStdString();
}

/// Where the search stands: which tab, which row.
struct Place {
    int tab;
    int row;

    friend bool operator==(const Place&, const Place&) = default;
};

[[nodiscard]] Place placeOf(const MainWindow& window) {
    const std::vector<int> rows = selectedRows(window);
    REQUIRE(rows.size() == 1);
    return Place{.tab = window.tabBar()->currentIndex(), .row = rows.front()};
}

} // namespace

TEST_CASE("the scope is offered from two projects, and only then", "[gui][GUI-SEARCH-04]") {
    InMemoryFileSystem files = filesystem();
    FakePrompts prompts;
    MainWindow window{files, fileIn(files, "premier.srt"), prompts};
    window.show();
    window.findAndReplaceAction()->trigger();
    REQUIRE(window.searchDialog() != nullptr);
    CHECK_FALSE(window.searchDialog()->allProjectsCheck()->isEnabled());

    prompts.nextFileToOpen = "second.srt";
    window.openAction()->trigger();

    CHECK(window.searchDialog()->allProjectsCheck()->isEnabled());
}

TEST_CASE("a search across projects visits the tabs in their order", "[gui][GUI-SEARCH-04]") {
    InMemoryFileSystem files = filesystem();
    FakePrompts prompts;
    MainWindow window{files, fileIn(files, "premier.srt"), prompts};
    window.show();
    openThree(window, prompts);
    const SearchDialog& dialog = searchingAll(window, "marie");

    std::vector<Place> visited;
    for (int step = 0; step < 4; ++step) {
        dialog.nextButton()->click();
        visited.push_back(placeOf(window));
    }

    CHECK(visited == std::vector<Place>{{.tab = 0, .row = 1},
                                        {.tab = 1, .row = 0},
                                        {.tab = 1, .row = 2},
                                        {.tab = 2, .row = 1}});
    // Nothing yet said of a round: the first pass has not come back.
    CHECK(statusOf(dialog).empty());
}

TEST_CASE("the round ends where it began, and says it wrapped", "[gui][GUI-SEARCH-04]") {
    InMemoryFileSystem files = filesystem();
    FakePrompts prompts;
    MainWindow window{files, fileIn(files, "premier.srt"), prompts};
    window.show();
    openThree(window, prompts);
    const SearchDialog& dialog = searchingAll(window, "marie");
    for (int step = 0; step < 4; ++step)
        dialog.nextButton()->click();
    REQUIRE(placeOf(window) == Place{.tab = 2, .row = 1});

    dialog.nextButton()->click();

    CHECK(placeOf(window) == Place{.tab = 0, .row = 1});
    CHECK(statusOf(dialog).find("wrapped") != std::string::npos);

    // And the next one leaves the say alone: it is a plain step again.
    dialog.nextButton()->click();
    CHECK(placeOf(window) == Place{.tab = 1, .row = 0});
    CHECK(statusOf(dialog).empty());
}

TEST_CASE("finding backwards goes through the tabs the other way", "[gui][GUI-SEARCH-04]") {
    InMemoryFileSystem files = filesystem();
    FakePrompts prompts;
    MainWindow window{files, fileIn(files, "premier.srt"), prompts};
    window.show();
    openThree(window, prompts);
    const SearchDialog& dialog = searchingAll(window, "marie");

    std::vector<Place> visited;
    std::vector<bool> wrapped;
    for (int step = 0; step < 5; ++step) {
        dialog.previousButton()->click();
        visited.push_back(placeOf(window));
        wrapped.push_back(statusOf(dialog).find("wrapped") != std::string::npos);
    }

    // With nothing found yet, backwards starts from the last match of the tab
    // shown; the second press goes past the first tab and comes to the last
    // one — the round, which says so — and then the way back down.
    CHECK(visited == std::vector<Place>{{.tab = 0, .row = 1},
                                        {.tab = 2, .row = 1},
                                        {.tab = 1, .row = 2},
                                        {.tab = 1, .row = 0},
                                        {.tab = 0, .row = 1}});
    CHECK(wrapped == std::vector<bool>{false, true, false, false, false});
}

TEST_CASE("a pattern found in no project says so, and moves nothing", "[gui][GUI-SEARCH-04]") {
    InMemoryFileSystem files = filesystem();
    FakePrompts prompts;
    MainWindow window{files, fileIn(files, "premier.srt"), prompts};
    window.show();
    openThree(window, prompts);
    const SearchDialog& dialog = searchingAll(window, "introuvable");

    dialog.nextButton()->click();

    CHECK(statusOf(dialog).find("not found") != std::string::npos);
    CHECK(window.tabBar()->currentIndex() == 0);
}

TEST_CASE("with the scope off, the search stays in the project it is in", "[gui][GUI-SEARCH-04]") {
    InMemoryFileSystem files = filesystem();
    FakePrompts prompts;
    MainWindow window{files, fileIn(files, "premier.srt"), prompts};
    window.show();
    openThree(window, prompts);
    const SearchDialog& dialog = searchingAll(window, "marie");
    dialog.allProjectsCheck()->setChecked(false);

    dialog.nextButton()->click();
    dialog.nextButton()->click();

    CHECK(window.tabBar()->currentIndex() == 0);
    CHECK(placeOf(window) == Place{.tab = 0, .row = 1});
}

TEST_CASE("replacing one match replaces where the search stands, and goes on to the next project",
          "[gui][GUI-SEARCH-04]") {
    InMemoryFileSystem files = filesystem();
    FakePrompts prompts;
    MainWindow window{files, fileIn(files, "premier.srt"), prompts};
    window.show();
    openThree(window, prompts);
    const SearchDialog& dialog = searchingAll(window, "marie");
    dialog.replacementField()->setText(QStringLiteral("Sophie"));
    dialog.nextButton()->click();
    REQUIRE(placeOf(window) == Place{.tab = 0, .row = 1});

    dialog.replaceButton()->click();

    // The next match is in the second project, and the search moved there.
    CHECK(placeOf(window) == Place{.tab = 1, .row = 0});
    window.tabBar()->setCurrentIndex(0);
    CHECK(textAt(window, 1) == "Bonjour Sophie.");
}

TEST_CASE("replace all touches every project, one entry of history each", "[gui][GUI-SEARCH-04]") {
    InMemoryFileSystem files = filesystem();
    files.addFile("aucun.srt", kNone);
    FakePrompts prompts;
    MainWindow window{files, fileIn(files, "premier.srt"), prompts};
    window.show();
    openThree(window, prompts);
    prompts.nextFileToOpen = "aucun.srt";
    window.openAction()->trigger();
    window.tabBar()->setCurrentIndex(1);
    const SearchDialog& dialog = searchingAll(window, "marie");
    dialog.replacementField()->setText(QStringLiteral("Sophie"));

    dialog.replaceAllButton()->click();

    // Four matches in three projects: the fourth, that has none, is not counted.
    CHECK(statusOf(dialog) == "replaced 4 matches in 3 projects");
    // The tab the gesture was made from is the one shown afterwards.
    CHECK(window.tabBar()->currentIndex() == 1);
    CHECK(textAt(window, 0) == "Sophie.");

    window.tabBar()->setCurrentIndex(0);
    CHECK(textAt(window, 1) == "Bonjour Sophie.");
    CHECK(window.undoAction()->text().toStdString() == "Undo: replacing all");
    window.tabBar()->setCurrentIndex(3);
    CHECK_FALSE(window.undoAction()->isEnabled());
}

TEST_CASE("replace all leaves the tab and the film shown where they were", "[gui][GUI-SEARCH-04]") {
    InMemoryFileSystem files = filesystem();
    files.addFile("premier.mkv", "");
    files.addFile("second.mkv", "");
    files.addFile("troisieme.mkv", "");
    FakePrompts prompts;
    FakeVideoPlayer* player = nullptr;
    const PlayerFactory projecting = [&player](std::uintptr_t) -> std::unique_ptr<VideoPlayer> {
        auto made = std::make_unique<FakeVideoPlayer>();
        player = made.get();
        return made;
    };
    MainWindow window{files, fileIn(files, "premier.srt"), prompts, projecting};
    window.show();
    openThree(window, prompts);
    REQUIRE(player != nullptr);
    const std::vector<std::filesystem::path> before = player->opened;
    REQUIRE(before.back() == "premier.mkv");
    const SearchDialog& dialog = searchingAll(window, "marie");
    dialog.replacementField()->setText(QStringLiteral("Sophie"));

    dialog.replaceAllButton()->click();

    REQUIRE(statusOf(dialog) == "replaced 4 matches in 3 projects");
    // Issue #461: each project received its replacement behind its tab — the
    // player was never sent to the other films, and each tab says it changed.
    CHECK(player->opened == before);
    CHECK(window.tabBar()->currentIndex() == 0);
    CHECK(window.tabBar()->tabText(1).toStdString() == "second.srt*");
    CHECK(window.tabBar()->tabText(2).toStdString() == "troisieme.srt*");
}

TEST_CASE("undoing in one project undoes only what that project received", "[gui][GUI-SEARCH-04]") {
    InMemoryFileSystem files = filesystem();
    FakePrompts prompts;
    MainWindow window{files, fileIn(files, "premier.srt"), prompts};
    window.show();
    openThree(window, prompts);
    const SearchDialog& dialog = searchingAll(window, "marie");
    dialog.replacementField()->setText(QStringLiteral("Sophie"));
    dialog.replaceAllButton()->click();

    window.tabBar()->setCurrentIndex(1);
    window.undoAction()->trigger();

    CHECK(textAt(window, 0) == "Marie.");
    CHECK(textAt(window, 2) == "Et marie.");
    window.tabBar()->setCurrentIndex(0);
    CHECK(textAt(window, 1) == "Bonjour Sophie.");
    window.tabBar()->setCurrentIndex(2);
    CHECK(textAt(window, 1) == "Adieu Sophie.");
}

TEST_CASE("one project touched is counted as one", "[gui][GUI-SEARCH-04]") {
    InMemoryFileSystem files = filesystem();
    FakePrompts prompts;
    MainWindow window{files, fileIn(files, "premier.srt"), prompts};
    window.show();
    prompts.nextFileToOpen = "aucun.srt";
    window.openAction()->trigger();
    window.tabBar()->setCurrentIndex(0);
    const SearchDialog& dialog = searchingAll(window, "marie");
    dialog.replacementField()->setText(QStringLiteral("Sophie"));

    dialog.replaceAllButton()->click();

    CHECK(statusOf(dialog) == "replaced 1 match in 1 project");
}

TEST_CASE("nothing found in any project is not found, and not a change", "[gui][GUI-SEARCH-04]") {
    InMemoryFileSystem files = filesystem();
    FakePrompts prompts;
    MainWindow window{files, fileIn(files, "premier.srt"), prompts};
    window.show();
    openThree(window, prompts);
    const SearchDialog& dialog = searchingAll(window, "introuvable");
    dialog.replacementField()->setText(QStringLiteral("x"));

    dialog.replaceAllButton()->click();

    CHECK(statusOf(dialog).find("not found") != std::string::npos);
    CHECK_FALSE(window.undoAction()->isEnabled());
}
