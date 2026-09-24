// Dropping files on the window — issue #453, decision D2, `GUI-TABS-04`.
//
// The route is `Open…`'s and `Select Video…`'s; what is under test here is the
// sorting a drop adds: a tab per subtitle file, the one film given to the tab
// shown, a file already open brought forward rather than read twice, and one
// box that says what could not be done while the rest still is.

#include <subedit/core/format/project_file.hpp>
#include <subedit/core/io/in_memory_file_system.hpp>
#include <subedit/gui/main_window.hpp>

#include <QAbstractItemModel>
#include <QCoreApplication>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QList>
#include <QMimeData>
#include <QPointF>
#include <QStatusBar>
#include <QTabBar>
#include <QUrl>
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
using subedit::test::FakePrompts;
using subedit::test::FakeVideoPlayer;

using Paths = std::vector<std::filesystem::path>;

constexpr const char* kFirst = "1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n";
constexpr const char* kSecond = "1\n00:00:01,000 --> 00:00:02,000\nDeux.\n\n";
constexpr const char* kThird = "1\n00:00:01,000 --> 00:00:02,000\nTrois.\n\n";

[[nodiscard]] InMemoryFileSystem filesystem() {
    InMemoryFileSystem files;
    files.addFile("/films/premier.srt", kFirst);
    files.addFile("/films/second.srt", kSecond);
    files.addFile("/films/troisieme.srt", kThird);
    files.addFile("/films/notes.txt", "rien de lisible");
    files.addFile("/films/film.mkv", "");
    files.addFile("/films/autre.mp4", "");
    return files;
}

[[nodiscard]] subedit::core::OpenedFile fileIn(const InMemoryFileSystem& files, const char* path) {
    auto opened = openProject(files, path);
    REQUIRE(opened.has_value());
    return std::move(*opened);
}

/// The double of the player, as `window_tabs_test.cpp` has it.
struct Projectionist {
    FakeVideoPlayer* player = nullptr;
};

[[nodiscard]] PlayerFactory projecting(Projectionist& booth) {
    return [&booth](std::uintptr_t) -> std::unique_ptr<VideoPlayer> {
        auto made = std::make_unique<FakeVideoPlayer>();
        booth.player = made.get();
        return made;
    };
}

[[nodiscard]] std::string tabText(const MainWindow& window, int index) {
    return window.tabBar()->tabText(index).toStdString();
}

[[nodiscard]] std::unique_ptr<QMimeData> urlsOf(const QList<QUrl>& urls) {
    auto data = std::make_unique<QMimeData>();
    data->setUrls(urls);
    return data;
}

} // namespace

TEST_CASE("the window takes a drag of files, and refuses a drag of anything else",
          "[gui][GUI-TABS-04]") {
    InMemoryFileSystem files = filesystem();
    FakePrompts prompts;
    MainWindow window{files, fileIn(files, "/films/premier.srt"), prompts};
    window.show();
    CHECK(window.acceptDrops());

    const std::unique_ptr<QMimeData> withFiles =
        urlsOf({QUrl::fromLocalFile(QStringLiteral("/films/second.srt"))});
    QDragEnterEvent dragged{QPoint{10, 10}, Qt::CopyAction, withFiles.get(), Qt::LeftButton, {}};
    QCoreApplication::sendEvent(&window, &dragged);
    CHECK(dragged.isAccepted());

    QMimeData text;
    text.setText(QStringLiteral("Bonjour"));
    QDragEnterEvent words{QPoint{10, 10}, Qt::CopyAction, &text, Qt::LeftButton, {}};
    words.setAccepted(false);
    QCoreApplication::sendEvent(&window, &words);
    CHECK_FALSE(words.isAccepted());
}

TEST_CASE("a dropped subtitle file opens in a tab of its own, as Open does", "[gui][GUI-TABS-04]") {
    InMemoryFileSystem files = filesystem();
    FakePrompts prompts;
    MainWindow window{files, fileIn(files, "/films/premier.srt"), prompts};
    window.show();

    const std::unique_ptr<QMimeData> data =
        urlsOf({QUrl::fromLocalFile(QStringLiteral("/films/second.srt"))});
    // The order a real drag delivers: Qt refuses a drop on a widget that no
    // drag entered first.
    QDragEnterEvent entered{QPoint{10, 10}, Qt::CopyAction, data.get(), Qt::LeftButton, {}};
    QCoreApplication::sendEvent(&window, &entered);
    QDropEvent drop{QPointF{10, 10}, Qt::CopyAction, data.get(), Qt::LeftButton, {}};
    QCoreApplication::sendEvent(&window, &drop);

    REQUIRE(window.tabBar()->count() == 2);
    CHECK(window.tabBar()->currentIndex() == 1);
    CHECK(tabText(window, 1) == "second.srt");
    CHECK(drop.isAccepted());
}

TEST_CASE("several subtitle files open one tab each, in the order they came",
          "[gui][GUI-TABS-04]") {
    InMemoryFileSystem files = filesystem();
    FakePrompts prompts;
    MainWindow window{files, fileIn(files, "/films/premier.srt"), prompts};
    window.show();

    window.openDropped(Paths{"/films/second.srt", "/films/troisieme.srt"});

    REQUIRE(window.tabBar()->count() == 3);
    CHECK(tabText(window, 1) == "second.srt");
    CHECK(tabText(window, 2) == "troisieme.srt");
    CHECK(window.tabBar()->currentIndex() == 2);
    CHECK(prompts.failures.empty());
}

TEST_CASE("a dropped file already open brings its tab forward, and is not read again",
          "[gui][GUI-TABS-04]") {
    InMemoryFileSystem files = filesystem();
    FakePrompts prompts;
    MainWindow window{files, fileIn(files, "/films/premier.srt"), prompts};
    window.show();
    window.openDropped(Paths{"/films/second.srt"});
    REQUIRE(window.tabBar()->currentIndex() == 1);

    window.openDropped(Paths{"/films/premier.srt"});

    CHECK(window.tabBar()->count() == 2);
    CHECK(window.tabBar()->currentIndex() == 0);
    CHECK(window.statusBar()->currentMessage().toStdString() == "premier.srt: already open");
}

TEST_CASE("a dropped film is given to the tab shown, as Select Video does", "[gui][GUI-TABS-04]") {
    InMemoryFileSystem files = filesystem();
    FakePrompts prompts;
    Projectionist booth;
    MainWindow window{files, fileIn(files, "/films/premier.srt"), prompts, projecting(booth)};
    window.show();

    window.openDropped(Paths{"/films/film.mkv"});

    REQUIRE(booth.player != nullptr);
    CHECK(booth.player->opened.back() == "/films/film.mkv");
    CHECK(window.tabBar()->count() == 1);
}

TEST_CASE("a film dropped with subtitle files goes to the last tab they opened",
          "[gui][GUI-TABS-04]") {
    InMemoryFileSystem files = filesystem();
    FakePrompts prompts;
    Projectionist booth;
    MainWindow window{files, fileIn(files, "/films/premier.srt"), prompts, projecting(booth)};
    window.show();

    // The film first in the list: it waits for the subtitles all the same.
    window.openDropped(Paths{"/films/film.mkv", "/films/second.srt"});

    REQUIRE(window.tabBar()->count() == 2);
    CHECK(window.tabBar()->currentIndex() == 1);
    REQUIRE(booth.player != nullptr);
    CHECK(booth.player->opened.back() == "/films/film.mkv");
}

TEST_CASE("two films dropped at once are left alone, and the box says why", "[gui][GUI-TABS-04]") {
    InMemoryFileSystem files = filesystem();
    FakePrompts prompts;
    Projectionist booth;
    MainWindow window{files, fileIn(files, "/films/premier.srt"), prompts, projecting(booth)};
    window.show();

    window.openDropped(Paths{"/films/film.mkv", "/films/autre.mp4", "/films/second.srt"});

    // The subtitle file still opens.
    CHECK(window.tabBar()->count() == 2);
    CHECK(booth.player == nullptr);
    REQUIRE(prompts.failures.size() == 1);
    CHECK(prompts.failures.front() == "2 videos dropped at once: a project watches one film");
}

TEST_CASE("a file that will not open is named, and the others open all the same",
          "[gui][GUI-TABS-04]") {
    InMemoryFileSystem files = filesystem();
    FakePrompts prompts;
    MainWindow window{files, fileIn(files, "/films/premier.srt"), prompts};
    window.show();

    window.openDropped(Paths{"/films/notes.txt", "/films/absent.srt", "/films/second.srt"});

    CHECK(window.tabBar()->count() == 2);
    CHECK(tabText(window, 1) == "second.srt");
    // One box for all of them, a line each.
    REQUIRE(prompts.failures.size() == 1);
    CHECK(prompts.failures.front() == "/films/notes.txt: is in no format this tool knows\n"
                                      "/films/absent.srt: does not exist");
}
