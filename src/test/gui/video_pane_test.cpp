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
#include <QAbstractSlider>
#include <QApplication>
#include <QIcon>
#include <QImage>
#include <QItemSelection>
#include <QItemSelectionModel>
#include <QLabel>
#include <QPoint>
#include <QScrollBar>
#include <QSize>
#include <QSlider>
#include <QSplitter>
#include <QString>
#include <QTest>
#include <QToolButton>
#include <QWheelEvent>
#include <QWidget>
#include <catch2/catch_test_macros.hpp>

#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <functional>
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

namespace {

/// The size an icon of the bar is looked at in.
constexpr QSize kIconSize{24, 24};

} // namespace

TEST_CASE("the play button plays and holds, and says which it will do", "[gui][GUI-SEEK-01]") {
    Booth booth;
    ProjectPage& page = watched(booth);
    booth.pane->follow(page);
    CHECK(booth.pane->bar()->playButton()->toolTip() == QStringLiteral("Play"));
    const QImage playIcon = booth.pane->bar()->playButton()->icon().pixmap(kIconSize).toImage();

    booth.pane->bar()->playButton()->click();
    booth.pane->follow(page);
    CHECK(booth.player->isPlaying());
    CHECK(booth.pane->bar()->playButton()->toolTip() == QStringLiteral("Pause"));
    // An icon and no text, and not the same icon: the button says what it will do.
    CHECK(booth.pane->bar()->playButton()->text().isEmpty());
    CHECK_FALSE(booth.pane->bar()->playButton()->icon().isNull());
    CHECK(booth.pane->bar()->playButton()->icon().pixmap(kIconSize).toImage() != playIcon);

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

// ## The audio tracks — issue #616

// GUI-AUDIO-01: the pane gives the tracks of the film that is open, and no other.
TEST_CASE("the pane lists the audio tracks of the open film and plays the one chosen",
          "[gui][GUI-AUDIO-01]") {
    Booth booth;
    CHECK(booth.pane->audioTracks().empty());

    watched(booth);
    booth.player->tracks = {{.id = 1, .language = "fra", .title = "Original", .selected = true},
                            {.id = 2, .language = "eng", .title = "Commentary", .selected = false}};

    const std::vector<subedit::core::AudioTrack> tracks = booth.pane->audioTracks();
    REQUIRE(tracks.size() == 2U);
    CHECK(tracks.at(0).selected);

    booth.pane->selectAudioTrack(2);

    CHECK(booth.pane->audioTracks().at(1).selected);
    CHECK_FALSE(booth.pane->audioTracks().at(0).selected);
}

// A track that is not the film's changes nothing: the player ignores it.
TEST_CASE("choosing a track the film does not have changes nothing", "[gui][GUI-AUDIO-01]") {
    Booth booth;
    watched(booth);
    booth.player->tracks = {{.id = 1, .language = "fra", .title = "", .selected = true}};

    booth.pane->selectAudioTrack(9);

    CHECK(booth.pane->audioTracks().at(0).selected);
}

// The player is shared, and what it holds is the film of one page: a page whose film is not open
// has no tracks to list, and no track to choose.
TEST_CASE("a page whose film is not open has no audio tracks", "[gui][GUI-AUDIO-01]") {
    Booth booth;
    ProjectPage& page = watched(booth);
    booth.player->tracks = {{.id = 1, .language = "fra", .title = "", .selected = true},
                            {.id = 2, .language = "eng", .title = "", .selected = false}};
    ProjectPage& other = booth.open("/films/other.srt");

    // The window is on the other page, which has no film of its own yet.
    booth.pane->leave(page);
    booth.show(other);
    booth.pane->watch(other);
    REQUIRE_FALSE(other.watching);

    CHECK(booth.pane->audioTracks().empty());
    booth.pane->selectAudioTrack(2);
    CHECK(booth.player->tracks.at(0).selected);
}

// ## The table follows the film — issue #619

namespace {

/// A document of `count` subtitles, one a second, each lasting 800 milliseconds: the subtitle of
/// row `r` shows from `r` seconds to `r` seconds and 800 milliseconds.
[[nodiscard]] std::string longFile(int count) {
    std::string text;
    for (int row = 0; row < count; ++row) {
        const auto second = [](int s) {
            return std::string{"00:"} + (s / 60 < 10 ? "0" : "") + std::to_string(s / 60) + ":" +
                   (s % 60 < 10 ? "0" : "") + std::to_string(s % 60);
        };
        text += std::to_string(row + 1) + "\n" + second(row) + ",000 --> " + second(row) +
                ",800\nLine " + std::to_string(row) + ".\n\n";
    }
    return text;
}

/// A pane on a long document, a film open, and a table tall enough to scroll.
struct Following {
    Booth booth;
    ProjectPage* page;

    Following() : page{&openLong(booth)} {
        booth.pane->proposeBeside(*page);
        booth.owner.resize(kWidth, kHeight);
        booth.split.setGeometry(0, 0, kWidth, kHeight);
        booth.owner.show();
        booth.pane->windowShown(*page);
        REQUIRE(booth.player != nullptr);
        REQUIRE(page->watching);
        QCoreApplication::processEvents();
    }

    static constexpr int kWidth = 700;
    static constexpr int kHeight = 500;

    /// The long document and its film, put in the booth and opened.
    static ProjectPage& openLong(Booth& in) {
        in.files.addFile("/films/long.mkv", "");
        in.files.addFile("/films/long.srt", longFile(300));
        return in.open("/films/long.srt");
    }

    /// Playback stands in the subtitle of `row`, and the follower runs.
    void at(int row) const {
        booth.player->where = Timestamp::fromMilliseconds((row * 1000) + 100);
        booth.pane->follow(*page);
    }

    [[nodiscard]] int currentRow() const { return booth.table.currentIndex().row(); }

    [[nodiscard]] int top() const { return booth.table.verticalScrollBar()->value(); }

    [[nodiscard]] bool following() const { return booth.pane->bar()->followButton()->isChecked(); }

    /// Whether `row` sits in the middle third of the room the table shows. A table that scrolls by
    /// rows cannot put a row exactly on the middle, and the property that matters is not being at
    /// an edge, where the row would be brought into view and no further.
    [[nodiscard]] bool centered(int row) const {
        const QRect rect = booth.table.visualRect(booth.table.model()->index(row, 0));
        const int middle = booth.table.viewport()->height() / 2;
        return rect.isValid() && std::abs(rect.center().y() - middle) <= middle / 3;
    }

    /// A turn of the wheel, as the person's hand makes it.
    void turnTheWheel() const {
        QWheelEvent wheel{QPointF{50, 50},
                          QPointF{50, 50},
                          QPoint{},
                          QPoint{0, -120},
                          Qt::NoButton,
                          Qt::NoModifier,
                          Qt::NoScrollPhase,
                          false};
        QApplication::sendEvent(booth.table.viewport(), &wheel);
    }
};

} // namespace

TEST_CASE("the table centers the row that is playing", "[gui][GUI-FOLLOW-01]") {
    const Following film;

    film.at(120);

    CHECK(film.currentRow() == 120);
    CHECK(film.centered(120));
    // Not merely brought into view: that would leave it at an edge.
    CHECK(film.top() > 0);
}

TEST_CASE("the row is centered when it changes, not at every tick", "[gui][GUI-FOLLOW-01]") {
    const Following film;
    film.at(120);
    QScrollBar* bar = film.booth.table.verticalScrollBar();

    // Moved a little by the program, as a layout would: the follower does not put it back while the
    // row is the same.
    bar->setValue(bar->value() + 3);
    const int moved = film.top();
    for (int tick = 0; tick < 10; ++tick)
        film.at(120);
    CHECK(film.top() == moved);

    film.at(121);
    CHECK(film.currentRow() == 121);
    CHECK(film.centered(121));
}

TEST_CASE("a turn of the wheel suspends the following, and the button says so",
          "[gui][GUI-FOLLOW-02]") {
    const Following film;
    film.at(120);
    REQUIRE(film.following());

    film.turnTheWheel();
    const int scrolled = film.top();
    film.at(150);

    CHECK_FALSE(film.following());
    CHECK(film.top() == scrolled);
    CHECK(film.currentRow() == 120);
}

TEST_CASE("the scroll bar suspends the following", "[gui][GUI-FOLLOW-02]") {
    const Following film;
    film.at(120);

    film.booth.table.verticalScrollBar()->triggerAction(QAbstractSlider::SliderPageStepAdd);
    const int scrolled = film.top();
    film.at(150);

    CHECK_FALSE(film.following());
    CHECK(film.top() == scrolled);
}

TEST_CASE("a click in the table suspends the following, and is not centered under the pointer",
          "[gui][GUI-FOLLOW-02]") {
    const Following film;
    film.at(120);
    const QModelIndex clicked = film.booth.table.model()->index(118, 1);
    const int before = film.top();

    QTest::mouseClick(film.booth.table.viewport(),
                      Qt::LeftButton,
                      Qt::NoModifier,
                      film.booth.table.visualRect(clicked).center());
    film.at(150);

    CHECK_FALSE(film.following());
    CHECK(film.currentRow() == 118);
    CHECK(film.top() == before);
}

TEST_CASE("every gesture of the player takes the following up again", "[gui][GUI-FOLLOW-03]") {
    using Gesture = std::function<void(const Following&)>;
    const std::vector<std::pair<const char*, Gesture>> gestures{
        {"play",
         [](const Following& f) {
             f.booth.pane->toggle(*f.page);
             // The next tick of the follower, which is what carries a film that plays.
             f.booth.pane->follow(*f.page);
         }},
        {"jump", [](const Following& f) { f.booth.pane->seekBy(*f.page, 1); }},
        {"neighbour", [](const Following& f) { f.booth.pane->seekToNeighbour(*f.page, true); }},
        {"step", [](const Following& f) { f.booth.pane->step(*f.page, 1); }},
        {"bar",
         [](const Following& f) {
             emit f.booth.pane->bar()->seekRequested(Timestamp::fromMilliseconds(170100));
         }},
        {"mark",
         [](const Following& f) {
             f.booth.table.selectionModel()->select(f.booth.table.model()->index(0, 0),
                                                    QItemSelectionModel::ClearAndSelect |
                                                        QItemSelectionModel::Rows);
             f.booth.pane->markEdge(*f.page, subedit::core::Boundary::Start);
         }},
        {"insert", [](const Following& f) { f.booth.pane->insertAtPosition(*f.page); }},
        {"selection", [](const Following& f) {
             f.booth.table.selectionModel()->select(f.booth.table.model()->index(100, 0),
                                                    QItemSelectionModel::ClearAndSelect |
                                                        QItemSelectionModel::Rows);
             f.booth.pane->playSelection(*f.page);
         }}};

    for (const auto& [name, gesture] : gestures) {
        INFO(name);
        const Following film;
        film.at(120);
        film.turnTheWheel();
        film.at(150);
        REQUIRE_FALSE(film.following());

        gesture(film);

        CHECK(film.following());
        // The row playing is the row the table points at, centered.
        // The subtitle inserted at the position starts there and shows over the one that was
        // playing, so it is the one the row points at.
        const int playing = static_cast<int>(film.booth.player->where.milliseconds() / 1000) +
                            (std::string{name} == "insert" ? 1 : 0);
        CHECK(film.currentRow() == playing);
        CHECK(film.centered(playing));
    }
}

TEST_CASE("the button says whether the table follows, and puts it right", "[gui][GUI-FOLLOW-03]") {
    const Following film;
    film.at(120);
    QToolButton* button = film.booth.pane->bar()->followButton();
    CHECK(button->isChecked());

    button->click();
    CHECK_FALSE(film.following());
    film.at(150);
    CHECK(film.currentRow() == 120);

    // Pressed again: the table follows from now, and is already where playback is.
    button->click();
    CHECK(film.following());
    CHECK(film.currentRow() == 150);
    CHECK(film.centered(150));
}
