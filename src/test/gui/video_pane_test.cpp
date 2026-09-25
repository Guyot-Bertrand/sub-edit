// The film, the player and the picture, without the window — issue #484.
//
// `VideoPane` receives the page it is about, the table it reads, and a `View`
// for the two things it asks of the window. These cases give it a page of its
// own, a table and a splitter, a double of the view, and a player the booth
// hands out. What the window does around it — the status bar, the tabs — is
// proved in `window_player_test.cpp` and `window_tabs_test.cpp`, unchanged.

#include <subedit/core/edit/session.hpp>
#include <subedit/core/format/project_file.hpp>
#include <subedit/core/io/in_memory_file_system.hpp>
#include <subedit/core/model/document.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/time/duration.hpp>
#include <subedit/core/time/timestamp.hpp>
#include <subedit/core/video/video_player.hpp>
#include <subedit/gui/player_factory.hpp>
#include <subedit/gui/project_page.hpp>
#include <subedit/gui/subtitle_table.hpp>
#include <subedit/gui/subtitle_table_model.hpp>
#include <subedit/gui/video_pane.hpp>

#include <QAbstractButton>
#include <QItemSelectionModel>
#include <QSplitter>
#include <QString>
#include <QWidget>
#include <catch2/catch_test_macros.hpp>

#include <cstdint>
#include <filesystem>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <utility>
#include <vector>

#include "fake_prompts.hpp"
#include "fake_video_player.hpp"

namespace {

using subedit::core::Document;
using subedit::core::InMemoryFileSystem;
using subedit::core::PlayerError;
using subedit::core::Timestamp;
using subedit::core::VideoPlayer;
using subedit::gui::PlayerFactory;
using subedit::gui::ProjectPage;
using subedit::gui::VideoPane;
using subedit::test::FakePrompts;
using subedit::test::FakeVideoPlayer;

constexpr const char* kThree = "1\n00:00:01,000 --> 00:00:02,000\nUn.\n\n"
                               "2\n00:00:02,500 --> 00:00:03,500\nDeux.\n\n"
                               "3\n00:00:05,000 --> 00:00:06,000\nTrois.\n\n";

/// The window, as the video sees it.
class Screen final : public VideoPane::View {

public:
    Document aimed = Document::Main;
    std::vector<bool> playables;

    [[nodiscard]] Document targetDocument() const override { return aimed; }

    void playable(bool playable) override { playables.push_back(playable); }
};

/// What the pane was given to work with, and the players it asked for.
struct Booth {
    InMemoryFileSystem files;
    FakePrompts prompts;
    Screen screen;
    QWidget owner;
    QSplitter split{&owner};
    subedit::gui::SubtitleTable table{&owner};

    std::optional<PlayerError> refusal;
    FakeVideoPlayer* player = nullptr;
    int built = 0;
    bool destroyed = false;

    std::vector<std::unique_ptr<ProjectPage>> pages;
    std::unique_ptr<VideoPane> pane;

    Booth() {
        files.addFile("/films/film.mkv", "");
        files.addFile("/films/other.mkv", "");
        files.addFile("/films/film.srt", kThree);
        files.addFile("/films/other.srt", kThree);

        PlayerFactory factory = [this](std::uintptr_t /*surface*/) -> std::unique_ptr<VideoPlayer> {
            ++built;
            auto made = std::make_unique<FakeVideoPlayer>();
            made->refusal = refusal;
            made->onDestroyed = [this] { destroyed = true; };
            player = made.get();
            return made;
        };
        pane = std::make_unique<VideoPane>(
            files, prompts, screen, table, split, std::move(factory), nullptr, &owner);
        split.addWidget(&table);
    }

    Booth(const Booth&) = delete;
    Booth& operator=(const Booth&) = delete;
    Booth(Booth&&) = delete;
    Booth& operator=(Booth&&) = delete;
    ~Booth() = default;

    /// A page on `path`, shown in the table as the window would.
    ProjectPage& open(const char* path) {
        auto opened = subedit::core::openProject(files, path);
        REQUIRE(opened.has_value());
        pages.push_back(ProjectPage::make(std::move(opened->project)));
        show(*pages.back());
        return *pages.back();
    }

    void show(ProjectPage& page) {
        table.setModel(page.model.get());
        table.setSelectionModel(page.tableSelection.get());
    }
};

} // namespace

TEST_CASE("the pane lays the picture and its band above the table", "[gui][GUI-PLAYER-01]") {
    Booth booth;

    REQUIRE(booth.split.count() == 3);
    CHECK(booth.split.widget(0) == booth.pane->picture());
    CHECK(booth.split.widget(1) == booth.pane->banner());
    CHECK(booth.pane->invite()->text() == QStringLiteral("Select Video…"));
    CHECK(booth.pane->picture()->isHidden());
}

TEST_CASE("no film is opened before the window has been on screen", "[gui][GUI-PLAYER-01]") {
    Booth booth;
    ProjectPage& page = booth.open("/films/film.srt");
    booth.pane->proposeBeside(page);

    booth.pane->watch(page);
    CHECK(booth.built == 0);
    CHECK_FALSE(page.watching);

    booth.pane->windowShown(page);
    REQUIRE(booth.player != nullptr);
    CHECK(booth.player->opened == std::vector<std::filesystem::path>{"/films/film.mkv"});
    CHECK(page.watching);
    CHECK_FALSE(booth.pane->picture()->isHidden());
    CHECK(booth.pane->banner()->isHidden());
    CHECK(booth.screen.playables.back());
}

TEST_CASE("a film is chosen through the prompts, or not at all", "[gui][GUI-VIDEO-01]") {
    Booth booth;
    ProjectPage& page = booth.open("/films/film.srt");

    CHECK_FALSE(booth.pane->choose(page));
    CHECK_FALSE(page.session->project().video().has_value());

    booth.prompts.nextVideoToOpen = std::filesystem::path{"/films/other.mkv"};
    CHECK(booth.pane->choose(page));
    CHECK(page.session->project()
              .video()
              .transform([](const auto& video) { return video.path; })
              .value_or(std::filesystem::path{}) == "/films/other.mkv");
}

TEST_CASE("a film the player refuses is said, and the band comes back", "[gui][GUI-PLAYER-03]") {
    Booth booth;
    booth.refusal = PlayerError{.reason = "unreadable"};
    ProjectPage& page = booth.open("/films/film.srt");
    booth.pane->proposeBeside(page);

    booth.pane->windowShown(page);

    REQUIRE(booth.prompts.failures.size() == 1);
    CHECK(booth.prompts.failures.front() == "/films/film.mkv: unreadable");
    CHECK_FALSE(page.watching);
    CHECK(booth.pane->picture()->isHidden());
    CHECK_FALSE(booth.pane->banner()->isHidden());
    CHECK_FALSE(booth.screen.playables.back());

    // Refused once, and not offered again while the association stands.
    booth.pane->watch(page);
    CHECK(booth.prompts.failures.size() == 1);
}

TEST_CASE("playback follows the selection and the overlay follows playback",
          "[gui][GUI-PLAYER-02]") {
    Booth booth;
    ProjectPage& page = booth.open("/films/film.srt");
    booth.pane->proposeBeside(page);
    booth.pane->windowShown(page);
    REQUIRE(booth.player != nullptr);

    page.tableSelection->select(page.model->index(1, 0),
                                QItemSelectionModel::Select | QItemSelectionModel::Rows);
    booth.pane->placeAtSelection(page);

    CHECK(booth.player->seeks.back() == Timestamp::fromMilliseconds(2500));
    CHECK(booth.player->onScreen() == "Deux.");
    CHECK(booth.table.currentIndex().row() == 1);

    booth.player->where = Timestamp::fromMilliseconds(5500);
    booth.pane->follow(page);
    CHECK(booth.player->onScreen() == "Trois.");
    CHECK(booth.table.currentIndex().row() == 2);
}

TEST_CASE("the overlay carries the text aimed at", "[gui][GUI-PLAYER-04]") {
    Booth booth;
    ProjectPage& page = booth.open("/films/film.srt");
    booth.pane->proposeBeside(page);
    booth.pane->windowShown(page);
    REQUIRE(booth.player != nullptr);

    booth.player->where = Timestamp::fromMilliseconds(1500);
    booth.pane->follow(page);
    CHECK(booth.player->onScreen() == "Un.");

    // No translation here: aiming at it draws its empty text.
    booth.screen.aimed = Document::Translation;
    booth.pane->follow(page);
    CHECK(booth.player->onScreen().empty());
}

TEST_CASE("the pane plays and holds the film of the page", "[gui][GUI-PLAYER-01]") {
    Booth booth;
    ProjectPage& page = booth.open("/films/film.srt");
    booth.pane->toggle(page);
    CHECK(booth.built == 0);

    booth.pane->proposeBeside(page);
    booth.pane->windowShown(page);
    REQUIRE(booth.player != nullptr);
    booth.pane->toggle(page);
    CHECK(booth.player->isPlaying());
    booth.pane->toggle(page);
    CHECK_FALSE(booth.player->isPlaying());
}

TEST_CASE("a page left and returned to finds its film where it was", "[gui][GUI-TABS-01]") {
    Booth booth;
    ProjectPage& first = booth.open("/films/film.srt");
    booth.pane->proposeBeside(first);
    booth.pane->windowShown(first);
    booth.player->where = Timestamp::fromMilliseconds(5500);

    booth.pane->leave(first);
    ProjectPage& second = booth.open("/films/other.srt");
    booth.pane->proposeBeside(second);
    booth.pane->watch(second);
    CHECK(booth.player->opened.back() == "/films/other.mkv");
    // The length is that of the film the player holds, and of no other page.
    CHECK_FALSE(booth.pane->length(first).has_value());
    CHECK(booth.pane->length(second) == booth.player->length);

    booth.pane->leave(second);
    booth.show(first);
    booth.pane->watch(first);
    CHECK(booth.player->opened.back() == "/films/film.mkv");
    CHECK(booth.player->seeks.back() == Timestamp::fromMilliseconds(5500));
    CHECK(booth.built == 1);
}

TEST_CASE("releasing lets the player go and every page forget its film", "[gui][GUI-PLAYER-01]") {
    Booth booth;
    ProjectPage& page = booth.open("/films/film.srt");
    booth.pane->proposeBeside(page);
    booth.pane->windowShown(page);
    REQUIRE_FALSE(booth.destroyed);

    booth.pane->release(booth.pages);

    CHECK(booth.destroyed);
    CHECK_FALSE(page.watching);
    CHECK(page.associated.empty());
    CHECK_FALSE(booth.screen.playables.back());
    CHECK_FALSE(booth.pane->length(page).has_value());
}
