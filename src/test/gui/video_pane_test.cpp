// The film, the player and the picture, without the window — issue #484.
//
// `VideoPane` receives the page it is about, the table it reads, and a `View`
// for the two things it asks of the window. These cases give it a page of its
// own, a table and a splitter, a double of the view, and a player the booth
// hands out. What the window does around it — the status bar, the tabs — is
// proved in `window_player_test.cpp` and `window_tabs_test.cpp`, unchanged.

#include <subedit/core/config/video_settings.hpp>
#include <subedit/core/edit/session.hpp>
#include <subedit/core/format/project_file.hpp>
#include <subedit/core/io/in_memory_file_system.hpp>
#include <subedit/core/model/document.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/model/subtitle_index.hpp>
#include <subedit/core/time/duration.hpp>
#include <subedit/core/time/timestamp.hpp>
#include <subedit/core/video/video_player.hpp>
#include <subedit/gui/play_bar.hpp>
#include <subedit/gui/player_factory.hpp>
#include <subedit/gui/project_page.hpp>
#include <subedit/gui/subtitle_table.hpp>
#include <subedit/gui/subtitle_table_model.hpp>
#include <subedit/gui/video_pane.hpp>
#include <subedit/gui/video_surface.hpp>

#include <QAbstractButton>
#include <QItemSelectionModel>
#include <QLabel>
#include <QSlider>
#include <QSplitter>
#include <QString>
#include <QTest>
#include <QToolButton>
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

        PlayerFactory factory = [this]() -> std::unique_ptr<VideoPlayer> {
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
    // The room above the table holds the picture and, under it, the bar — issue #615.
    CHECK(booth.split.widget(0) == booth.pane->picture()->parentWidget());
    CHECK(booth.pane->bar()->parentWidget() == booth.pane->picture()->parentWidget());
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

// ## Driving the film — issue #615

namespace {

/// A page on `/films/film.srt`, its film opened — the state every gesture below starts from.
ProjectPage& watched(Booth& booth) {
    ProjectPage& page = booth.open("/films/film.srt");
    booth.pane->proposeBeside(page);
    booth.pane->windowShown(page);
    REQUIRE(page.watching);
    return page;
}

void select(Booth& booth, int first, int last) {
    booth.table.selectionModel()->clearSelection();
    for (int row = first; row <= last; ++row)
        booth.table.selectionModel()->select(booth.table.model()->index(row, 0),
                                             QItemSelectionModel::Select |
                                                 QItemSelectionModel::Rows);
}

} // namespace

// GUI-SEEK-02: a jump is the length the settings say, and stays inside the film.
TEST_CASE("a jump goes the length of the setting and stays in the film", "[gui][GUI-SEEK-02]") {
    Booth booth;
    ProjectPage& page = watched(booth);
    booth.player->where = Timestamp::fromMilliseconds(100000);
    booth.player->seeks.clear();

    booth.pane->seekBy(page, 1);
    CHECK(booth.player->seeks.back() == Timestamp::fromMilliseconds(130000));

    // The player moved where the first jump put it, so the second starts from there.
    booth.pane->seekBy(page, -1);
    CHECK(booth.player->seeks.back() == Timestamp::fromMilliseconds(100000));

    // Gaupol's thirty seconds by default, and the setting is what moves it.
    booth.pane->setSettings({.seekLengthSeconds = 5});
    booth.pane->seekBy(page, 1);
    CHECK(booth.player->seeks.back() == Timestamp::fromMilliseconds(105000));
}

TEST_CASE("a jump does not pass the start or the end of the film", "[gui][GUI-SEEK-02]") {
    Booth booth;
    ProjectPage& page = watched(booth);

    booth.player->where = Timestamp::fromMilliseconds(10000);
    booth.pane->seekBy(page, -1);
    CHECK(booth.player->seeks.back() == Timestamp::origin());

    booth.player->where = Timestamp::fromMilliseconds(590000);
    booth.pane->seekBy(page, 1);
    CHECK(booth.player->seeks.back() == Timestamp::fromMilliseconds(600000));
}

// GUI-SEEK-03: the neighbours, read as Gaupol reads them.
TEST_CASE("the next subtitle is the first to start after the position", "[gui][GUI-SEEK-03]") {
    Booth booth;
    ProjectPage& page = watched(booth);

    booth.player->where = Timestamp::fromMilliseconds(2200);
    booth.pane->seekToNeighbour(page, true);
    CHECK(booth.player->seeks.back() == Timestamp::fromMilliseconds(2500));

    // At the very start of one, the next is the one after — not that very subtitle.
    booth.player->where = Timestamp::fromMilliseconds(2500);
    booth.pane->seekToNeighbour(page, true);
    CHECK(booth.player->seeks.back() == Timestamp::fromMilliseconds(5000));
}

TEST_CASE("the previous subtitle is the last to have ended before the position",
          "[gui][GUI-SEEK-03]") {
    Booth booth;
    ProjectPage& page = watched(booth);

    booth.player->where = Timestamp::fromMilliseconds(4000);
    booth.pane->seekToNeighbour(page, false);
    CHECK(booth.player->seeks.back() == Timestamp::fromMilliseconds(2500));

    // Inside a subtitle, the previous is the one before it, which has ended.
    booth.player->where = Timestamp::fromMilliseconds(3000);
    booth.pane->seekToNeighbour(page, false);
    CHECK(booth.player->seeks.back() == Timestamp::fromMilliseconds(1000));
}

TEST_CASE("with no neighbour the gesture does nothing", "[gui][GUI-SEEK-03]") {
    Booth booth;
    ProjectPage& page = watched(booth);
    booth.player->seeks.clear();

    booth.player->where = Timestamp::fromMilliseconds(500);
    booth.pane->seekToNeighbour(page, false);
    booth.player->where = Timestamp::fromMilliseconds(7000);
    booth.pane->seekToNeighbour(page, true);

    CHECK(booth.player->seeks.empty());
}

// GUI-SEEK-04: the start and the end of the selection, the lead-in before them.
TEST_CASE("the start and the end of the selection place the film, with the lead-in",
          "[gui][GUI-SEEK-04]") {
    Booth booth;
    ProjectPage& page = watched(booth);
    select(booth, 1, 2);
    booth.player->seeks.clear();

    booth.pane->seekToSelection(page, false);
    CHECK(booth.player->seeks.back() == Timestamp::fromMilliseconds(1500));

    booth.pane->seekToSelection(page, true);
    CHECK(booth.player->seeks.back() == Timestamp::fromMilliseconds(5000));

    // The lead-in is the setting's, and the film starts at zero at the earliest.
    booth.pane->setSettings({.contextLengthMilliseconds = 3000});
    select(booth, 0, 0);
    booth.pane->seekToSelection(page, false);
    CHECK(booth.player->seeks.back() == Timestamp::origin());
}

TEST_CASE("without a selection the selection gestures do nothing", "[gui][GUI-SEEK-04]") {
    Booth booth;
    ProjectPage& page = watched(booth);
    booth.table.selectionModel()->clearSelection();
    booth.player->seeks.clear();

    booth.pane->seekToSelection(page, false);
    booth.pane->seekToSelection(page, true);
    booth.pane->playSelection(page);

    CHECK(booth.player->seeks.empty());
    CHECK(booth.player->stops.empty());
}

// GUI-SEEK-05: playing the selection stops at the end of its last subtitle.
TEST_CASE("playing the selection plays up to the end of the last subtitle", "[gui][GUI-SEEK-05]") {
    Booth booth;
    ProjectPage& page = watched(booth);
    select(booth, 0, 1);
    booth.player->seeks.clear();

    booth.pane->playSelection(page);

    // From the first subtitle's start less the lead-in, and up to the end of the second.
    CHECK(booth.player->seeks.back() == Timestamp::origin());
    REQUIRE(booth.player->stops.size() == 1U);
    CHECK(booth.player->stops.back() == Timestamp::fromMilliseconds(3500));
    CHECK(booth.player->isPlaying());
}

TEST_CASE("the gestures do nothing for a page whose film is not open", "[gui][GUI-SEEK-02]") {
    Booth booth;
    ProjectPage& page = booth.open("/films/film.srt");
    select(booth, 0, 0);

    booth.pane->seekBy(page, 1);
    booth.pane->seekToNeighbour(page, true);
    booth.pane->seekToSelection(page, false);
    booth.pane->playSelection(page);

    CHECK(booth.player == nullptr);
}

// GUI-VOLUME-01.
TEST_CASE("the volume is set by gesture, held between 0 and 100, and given to the player",
          "[gui][GUI-VOLUME-01]") {
    Booth booth;
    watched(booth);
    booth.pane->setSettings({.volume = 40});

    CHECK(booth.player->volume() == 40);
    CHECK(booth.pane->bar()->volumeSlider()->value() == 40);

    booth.pane->changeVolume(-1);
    CHECK(booth.player->volume() == 35);
    CHECK(booth.pane->settings().volume == 35);

    booth.pane->changeVolume(1);
    booth.pane->changeVolume(1);
    CHECK(booth.player->volume() == 45);

    booth.pane->setSettings({.volume = 98});
    booth.pane->changeVolume(1);
    CHECK(booth.player->volume() == 100);

    booth.pane->setSettings({.volume = 2});
    booth.pane->changeVolume(-1);
    CHECK(booth.player->volume() == 0);
}

TEST_CASE("the volume of the settings is given to a player that comes after it",
          "[gui][GUI-VOLUME-01]") {
    Booth booth;
    booth.pane->setSettings({.volume = 25});
    REQUIRE(booth.player == nullptr);

    watched(booth);

    CHECK(booth.player->volume() == 25);
}

TEST_CASE("the slider of the volume moves it", "[gui][GUI-VOLUME-01]") {
    Booth booth;
    watched(booth);

    booth.pane->bar()->volumeSlider()->setValue(60);

    CHECK(booth.player->volume() == 60);
    CHECK(booth.pane->settings().volume == 60);
}

// GUI-SEEK-01: the bar says where the film stands, and how long it lasts.
TEST_CASE("the bar says the position and the length, and so does the timecode",
          "[gui][GUI-SEEK-01][GUI-TIMECODE-01]") {
    Booth booth;
    ProjectPage& page = watched(booth);

    booth.player->where = Timestamp::fromMilliseconds(12480);
    booth.pane->follow(page);

    const subedit::gui::PlayBar* bar = booth.pane->bar();
    CHECK(bar->positionLabel()->text() == QStringLiteral("00:00:12,480"));
    CHECK(bar->lengthLabel()->text() == QStringLiteral("00:10:00,000"));
    CHECK(bar->positionSlider()->value() == 12480);
    CHECK(bar->positionSlider()->maximum() == 600000);
    // The timecode over the picture is the same position, written the same way.
    const auto* surface = dynamic_cast<const subedit::gui::VideoSurface*>(booth.pane->picture());
    REQUIRE(surface != nullptr);
    CHECK(surface->timecode() == QStringLiteral("00:00:12,480"));
}

TEST_CASE("the bar and the timecode are empty with no film", "[gui][GUI-TIMECODE-01]") {
    Booth booth;
    ProjectPage& page = booth.open("/films/film.srt");
    booth.pane->windowShown(page);

    const auto* surface = dynamic_cast<const subedit::gui::VideoSurface*>(booth.pane->picture());
    REQUIRE(surface != nullptr);
    CHECK(surface->timecode().isEmpty());
    CHECK(booth.pane->bar()->positionSlider()->value() == 0);
}

TEST_CASE("the play button plays and holds, and says which it will do", "[gui][GUI-SEEK-01]") {
    Booth booth;
    ProjectPage& page = watched(booth);
    booth.pane->follow(page);
    CHECK(booth.pane->bar()->playButton()->text() == QStringLiteral("Play"));

    booth.pane->bar()->playButton()->click();
    booth.pane->follow(page);
    CHECK(booth.player->isPlaying());
    CHECK(booth.pane->bar()->playButton()->text() == QStringLiteral("Pause"));

    booth.pane->bar()->playButton()->click();
    CHECK_FALSE(booth.player->isPlaying());
}

// GUI-SEEK-01: the slider is read and moved by the person, and an update of the follower is
// not a movement of the person.
TEST_CASE("the follower moving the slider is not a request for a position", "[gui][GUI-SEEK-01]") {
    Booth booth;
    ProjectPage& page = watched(booth);
    booth.player->seeks.clear();

    booth.player->where = Timestamp::fromMilliseconds(5000);
    booth.pane->follow(page);
    booth.player->where = Timestamp::fromMilliseconds(6000);
    booth.pane->follow(page);

    CHECK(booth.pane->bar()->positionSlider()->value() == 6000);
    CHECK(booth.player->seeks.empty());
}

// The criterion that carries the whole bar: the first position is asked at once, the ones that
// follow wait for the gate, and **the last one asked is the one that is reached**.
TEST_CASE("a drag asks for the first position at once and always reaches the last",
          "[gui][GUI-SEEK-01]") {
    Booth booth;
    watched(booth);
    booth.player->seeks.clear();
    QSlider* slider = booth.pane->bar()->positionSlider();

    slider->setValue(1000);
    REQUIRE(booth.player->seeks.size() == 1U);
    CHECK(booth.player->seeks.back() == Timestamp::fromMilliseconds(1000));

    // Four more in the same breath: none reaches the player yet.
    for (const int value : {2000, 3000, 4000, 5000})
        slider->setValue(value);
    CHECK(booth.player->seeks.size() == 1U);

    // The gate opens: the last of them goes, and the ones between never did.
    QTest::qWait(200);
    REQUIRE(booth.player->seeks.size() == 2U);
    CHECK(booth.player->seeks.back() == Timestamp::fromMilliseconds(5000));
}

TEST_CASE("letting the handle go reaches the last position at once", "[gui][GUI-SEEK-01]") {
    Booth booth;
    watched(booth);
    QSlider* slider = booth.pane->bar()->positionSlider();

    slider->setValue(1000);
    slider->setValue(2000); // the gate is closed: this one waits
    REQUIRE(booth.player->seeks.back() == Timestamp::fromMilliseconds(1000));

    Q_EMIT slider->sliderReleased();

    CHECK(booth.player->seeks.back() == Timestamp::fromMilliseconds(2000));
}

// The bar can be moved while no film is open — a slider has a range of its own — and what it
// asks for then has nobody to be asked of.
TEST_CASE("a request from the bar with no film open is not handed to anyone",
          "[gui][GUI-SEEK-01]") {
    Booth booth;
    ProjectPage& page = booth.open("/films/film.srt");
    booth.pane->windowShown(page);
    REQUIRE_FALSE(page.watching);

    QSlider* slider = booth.pane->bar()->positionSlider();
    slider->setRange(0, 10000);
    slider->setValue(4000);
    QTest::qWait(100);

    CHECK(booth.player == nullptr);
}

// The film may go from under the player — the file moved, the share dropped — while the window
// still believes it is there. Asked where it stands, the player answers nothing, and the
// gestures that start from the position have nowhere to start.
TEST_CASE("a player that lost its film is not asked to jump", "[gui][GUI-SEEK-02]") {
    Booth booth;
    ProjectPage& page = watched(booth);
    booth.player->unload();
    booth.player->seeks.clear();

    booth.pane->seekBy(page, 1);
    booth.pane->seekToNeighbour(page, true);

    CHECK(booth.player->seeks.empty());
}

// The handle is in the hand: the follower must not take it back.
TEST_CASE("the follower leaves the handle alone while it is held", "[gui][GUI-SEEK-01]") {
    Booth booth;
    ProjectPage& page = watched(booth);
    QSlider* slider = booth.pane->bar()->positionSlider();
    booth.player->where = Timestamp::fromMilliseconds(3000);
    booth.pane->follow(page);
    REQUIRE(slider->value() == 3000);

    slider->setSliderDown(true);
    booth.player->where = Timestamp::fromMilliseconds(9000);
    booth.pane->follow(page);
    CHECK(slider->value() == 3000);

    slider->setSliderDown(false);
    booth.pane->follow(page);
    CHECK(slider->value() == 9000);
}
