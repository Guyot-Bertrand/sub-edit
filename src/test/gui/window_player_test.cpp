// The film in the window — issue #176, and the gesture phase 6 exists to serve.
//
// **The player behind the window is a double here, and that is not a shortcut.**
// The real one is proved on the fixtures of #163, in `mpv_player_test.cpp`; it
// cannot also be proved through the window, because a libmpv handed the surface
// of an offscreen Qt platform draws nowhere and refuses the file — which is the
// measurement `vo=null` came out of. What belongs here is everything the window
// decides: when a film is opened, where playback is placed, what the replica
// says, which row follows it, and who gives way to whom.

#include <subedit/core/io/in_memory_file_system.hpp>
#include <subedit/core/time/timestamp.hpp>
#include <subedit/core/video/video_player.hpp>
#include <subedit/gui/main_window.hpp>
#include <subedit/gui/opening.hpp>
#include <subedit/gui/subtitle_table.hpp>
#include <subedit/gui/subtitle_table_model.hpp>

#include <QAction>
#include <QItemSelectionModel>
#include <QLabel>
#include <QModelIndex>
#include <QTest>
#include <catch2/catch_test_macros.hpp>

#include <cstdint>
#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <utility>

#include "fake_prompts.hpp"
#include "fake_video_player.hpp"

namespace {

using subedit::core::InMemoryFileSystem;
using subedit::core::PlayerError;
using subedit::core::Timestamp;
using subedit::core::VideoPlayer;
using subedit::gui::MainWindow;
using subedit::gui::OpenedFile;
using subedit::gui::openProject;
using subedit::gui::PlayerFactory;
using subedit::gui::SubtitleTableModel;
using subedit::test::FakePrompts;
using subedit::test::FakeVideoPlayer;

constexpr const char* kThree = "1\n"
                               "00:00:01,000 --> 00:00:02,000\n"
                               "Un.\n"
                               "\n"
                               "2\n"
                               "00:00:02,500 --> 00:00:03,500\n"
                               "Deux.\n"
                               "\n"
                               "3\n"
                               "00:00:05,000 --> 00:00:06,000\n"
                               "Trois.\n"
                               "\n";

/// What a case says about the players to come, and what came out.
///
/// The window owns its player, so a case cannot hand one in and keep it. It
/// hands in a factory instead and reads this afterwards — which is also how
/// « the window never asked for a player » becomes something to assert.
struct Projectionist {
    /// Whether libmpv would give a player at all.
    bool gives = true;

    /// Why the film the next player is given will not open, if it will not.
    std::optional<PlayerError> refusal;

    /// What came out, and what it was built for.
    FakeVideoPlayer* player = nullptr;
    std::uintptr_t surface = 0;
    int built = 0;
};

/// The factory `booth` answers, which must outlive the window taking it.
[[nodiscard]] PlayerFactory projecting(Projectionist& booth) {
    return [&booth](std::uintptr_t given) -> std::unique_ptr<VideoPlayer> {
        ++booth.built;
        booth.surface = given;
        if (!booth.gives)
            return nullptr;

        auto made = std::make_unique<FakeVideoPlayer>();
        made->refusal = booth.refusal;
        booth.player = made.get();
        return made;
    };
}

/// A directory holding a subtitle file, and whatever else the case needs.
[[nodiscard]] InMemoryFileSystem directoryHolding(std::initializer_list<const char*> names) {
    InMemoryFileSystem files;
    for (const char* name : names)
        files.addFile(std::filesystem::path{"/films"} / name, "");
    files.addFile("/films/film.fr.srt", kThree);
    return files;
}

[[nodiscard]] OpenedFile fileIn(const InMemoryFileSystem& files, const char* path) {
    auto opened = openProject(files, path);
    REQUIRE(opened.has_value());
    return std::move(*opened);
}

/// Whether the picture is part of the window.
///
/// `isHidden` and not `isVisible`: these windows are never shown, so nothing in
/// them is visible in Qt's sense. What is asked is whether the window took the
/// view away, which is exactly what `hide` marks.
[[nodiscard]] bool showsPicture(const MainWindow& window) {
    return !window.videoView()->isHidden();
}

void selectRow(const MainWindow& window, int row) {
    window.table()->selectionModel()->select(window.table()->model()->index(row, 0),
                                             QItemSelectionModel::Select |
                                                 QItemSelectionModel::Rows);
}

[[nodiscard]] int currentRow(const MainWindow& window) {
    return window.table()->currentIndex().row();
}

/// Puts playback at `milliseconds` and lets the window notice.
void playbackReaches(MainWindow& window, FakeVideoPlayer& player, int milliseconds) {
    player.where = Timestamp::fromMilliseconds(milliseconds);
    window.followPlayback();
}

} // namespace

TEST_CASE("the film beside the document opens in the window", "[gui][GUI-PLAYER-01]") {
    InMemoryFileSystem files = directoryHolding({"film.mkv"});
    FakePrompts prompts;
    Projectionist booth;
    const MainWindow window{files, fileIn(files, "/films/film.fr.srt"), prompts, projecting(booth)};

    REQUIRE(booth.player != nullptr);
    CHECK(booth.player->opened == std::vector<std::filesystem::path>{"/films/film.mkv"});
    CHECK(showsPicture(window));
    CHECK(window.playPauseAction()->isEnabled());
}

// The number is what libmpv is given, and the whole reason the player is built
// by the window rather than handed to it.
TEST_CASE("the player is built for the surface of the window", "[gui][GUI-PLAYER-01]") {
    InMemoryFileSystem files = directoryHolding({"film.mkv"});
    FakePrompts prompts;
    Projectionist booth;
    const MainWindow window{files, fileIn(files, "/films/film.fr.srt"), prompts, projecting(booth)};

    CHECK(booth.built == 1);
    CHECK(booth.surface == static_cast<std::uintptr_t>(window.videoView()->winId()));
}

TEST_CASE("a document with no film has no picture and nothing to play", "[gui][GUI-PLAYER-01]") {
    InMemoryFileSystem files = directoryHolding({});
    FakePrompts prompts;
    Projectionist booth;
    const MainWindow window{files, fileIn(files, "/films/film.fr.srt"), prompts, projecting(booth)};

    CHECK_FALSE(showsPicture(window));
    CHECK_FALSE(window.playPauseAction()->isEnabled());
    // Nothing was asked of libmpv either: a window shown no film builds no
    // player at all.
    CHECK(booth.built == 0);
}

TEST_CASE("play and pause reach the player", "[gui][GUI-PLAYER-01]") {
    InMemoryFileSystem files = directoryHolding({"film.mkv"});
    FakePrompts prompts;
    Projectionist booth;
    const MainWindow window{files, fileIn(files, "/films/film.fr.srt"), prompts, projecting(booth)};
    REQUIRE(booth.player != nullptr);

    // Opening a film is not watching it.
    CHECK_FALSE(booth.player->isPlaying());

    window.playPauseAction()->trigger();
    CHECK(booth.player->isPlaying());

    window.playPauseAction()->trigger();
    CHECK_FALSE(booth.player->isPlaying());
}

TEST_CASE("selecting a subtitle places playback at its start", "[gui][GUI-PLAYER-02]") {
    InMemoryFileSystem files = directoryHolding({"film.mkv"});
    FakePrompts prompts;
    Projectionist booth;
    const MainWindow window{files, fileIn(files, "/films/film.fr.srt"), prompts, projecting(booth)};
    REQUIRE(booth.player != nullptr);

    selectRow(window, 1);

    CHECK(booth.player->seeks.back() == Timestamp::fromMilliseconds(2500));
}

// Extending a selection downwards fires the selection signal at every step, and
// a seek waits for the player to arrive. What playback follows is the first row
// of the selection, so extending it changes nothing to follow.
TEST_CASE("extending a selection does not place playback again", "[gui][GUI-PLAYER-02]") {
    InMemoryFileSystem files = directoryHolding({"film.mkv"});
    FakePrompts prompts;
    Projectionist booth;
    const MainWindow window{files, fileIn(files, "/films/film.fr.srt"), prompts, projecting(booth)};
    REQUIRE(booth.player != nullptr);

    selectRow(window, 0);
    selectRow(window, 1);
    selectRow(window, 2);

    CHECK(booth.player->seeks.size() == 1U);
    CHECK(booth.player->seeks.back() == Timestamp::fromMilliseconds(1000));
}

// Deselecting everything is what a click in the empty part of the table does.
// There is no row to follow then, and playback stays where it was.
TEST_CASE("clearing the selection places playback nowhere", "[gui][GUI-PLAYER-02]") {
    InMemoryFileSystem files = directoryHolding({"film.mkv"});
    FakePrompts prompts;
    Projectionist booth;
    const MainWindow window{files, fileIn(files, "/films/film.fr.srt"), prompts, projecting(booth)};
    REQUIRE(booth.player != nullptr);
    selectRow(window, 1);
    const std::size_t placed = booth.player->seeks.size();

    window.table()->selectionModel()->clearSelection();

    CHECK(booth.player->seeks.size() == placed);
}

TEST_CASE("selecting a subtitle with no film open does nothing at all", "[gui][GUI-PLAYER-02]") {
    InMemoryFileSystem files = directoryHolding({});
    FakePrompts prompts;
    Projectionist booth;
    const MainWindow window{files, fileIn(files, "/films/film.fr.srt"), prompts, projecting(booth)};

    selectRow(window, 1);

    CHECK(booth.built == 0);
}

TEST_CASE("a film that will not open is named, and the window stays usable",
          "[gui][GUI-PLAYER-03]") {
    InMemoryFileSystem files = directoryHolding({"film.mkv"});
    FakePrompts prompts;
    Projectionist booth;
    booth.refusal = PlayerError{.reason = "unrecognized file format"};
    const MainWindow window{files, fileIn(files, "/films/film.fr.srt"), prompts, projecting(booth)};

    REQUIRE(prompts.failures.size() == 1U);
    CHECK(prompts.failures.front() == "/films/film.mkv: unrecognized file format");

    // Nothing else changed: no picture, nothing to play — and a document still
    // open, still named, still there to be edited.
    CHECK_FALSE(showsPicture(window));
    CHECK_FALSE(window.playPauseAction()->isEnabled());
    CHECK(window.table()->model()->rowCount(QModelIndex{}) == 3);
    CHECK(window.saveAction()->isEnabled());

    // The association stands: the user has to see which file it is that was
    // refused in order to choose another.
    CHECK(window.videoStatus()->text().toStdString() == "Video: film.mkv");
}

TEST_CASE("a film that was refused is not offered again", "[gui][GUI-PLAYER-03]") {
    InMemoryFileSystem files = directoryHolding({"film.mkv"});
    FakePrompts prompts;
    Projectionist booth;
    booth.refusal = PlayerError{.reason = "unrecognized file format"};
    const MainWindow window{files, fileIn(files, "/films/film.fr.srt"), prompts, projecting(booth)};
    REQUIRE(prompts.failures.size() == 1U);

    // The naming convention speaks again at every « save as ». It has the same
    // film to offer, and saying so a second time would say nothing new.
    prompts.nextSaveTarget = subedit::gui::SaveTarget{.path = "/films/film.en.srt"};
    window.saveAsAction()->trigger();

    CHECK(prompts.failures.size() == 1U);
}

// A libmpv that gives no player is not a reason to refuse to open a document.
TEST_CASE("a window with no player edits all the same", "[gui][GUI-PLAYER-03]") {
    InMemoryFileSystem files = directoryHolding({"film.mkv"});
    FakePrompts prompts;
    Projectionist booth;
    booth.gives = false;
    const MainWindow window{files, fileIn(files, "/films/film.fr.srt"), prompts, projecting(booth)};

    CHECK(booth.built == 1);
    CHECK_FALSE(showsPicture(window));
    CHECK(window.videoStatus()->text().toStdString() == "Video: film.mkv");

    // Said once, where it matters: a film was named and nothing will show it.
    REQUIRE(prompts.failures.size() == 1U);
    CHECK(prompts.failures.front() == "/films/film.mkv: no video player is available");
}

TEST_CASE("the replica drawn is the subtitle showing now", "[gui][GUI-PLAYER-01]") {
    InMemoryFileSystem files = directoryHolding({"film.mkv"});
    FakePrompts prompts;
    Projectionist booth;
    MainWindow window{files, fileIn(files, "/films/film.fr.srt"), prompts, projecting(booth)};
    REQUIRE(booth.player != nullptr);

    playbackReaches(window, *booth.player, 1500);
    CHECK(booth.player->onScreen() == "Un.");

    playbackReaches(window, *booth.player, 3000);
    CHECK(booth.player->onScreen() == "Deux.");

    // Between two subtitles the picture carries nothing.
    playbackReaches(window, *booth.player, 4000);
    CHECK(booth.player->onScreen().empty());
}

// Decision D2, and the whole reason the replica is not a file: what is on the
// picture is what was just typed, with nothing written to a disk in between.
TEST_CASE("an edited text reaches the picture", "[gui][GUI-PLAYER-01]") {
    InMemoryFileSystem files = directoryHolding({"film.mkv"});
    FakePrompts prompts;
    Projectionist booth;
    MainWindow window{files, fileIn(files, "/films/film.fr.srt"), prompts, projecting(booth)};
    REQUIRE(booth.player != nullptr);

    playbackReaches(window, *booth.player, 1500);
    REQUIRE(booth.player->onScreen() == "Un.");

    window.table()->model()->setData(window.table()->model()->index(0, SubtitleTableModel::Text),
                                     QStringLiteral("Corrigé."),
                                     Qt::EditRole);
    window.followPlayback();

    CHECK(booth.player->onScreen() == "Corrigé.");
}

// Handed the same line twice, the window says it once: the replica is
// recomputed ten times a second, and an overlay redrawn each time would be a
// hundred pointless orders a second.
TEST_CASE("a replica that has not changed is not drawn again", "[gui][GUI-PLAYER-01]") {
    InMemoryFileSystem files = directoryHolding({"film.mkv"});
    FakePrompts prompts;
    Projectionist booth;
    MainWindow window{files, fileIn(files, "/films/film.fr.srt"), prompts, projecting(booth)};
    REQUIRE(booth.player != nullptr);

    playbackReaches(window, *booth.player, 1200);
    const std::size_t drawn = booth.player->shown.size();

    playbackReaches(window, *booth.player, 1300);
    playbackReaches(window, *booth.player, 1400);

    CHECK(booth.player->shown.size() == drawn);
}

TEST_CASE("the current row follows playback", "[gui][GUI-PLAYER-01]") {
    InMemoryFileSystem files = directoryHolding({"film.mkv"});
    FakePrompts prompts;
    Projectionist booth;
    MainWindow window{files, fileIn(files, "/films/film.fr.srt"), prompts, projecting(booth)};
    REQUIRE(booth.player != nullptr);

    playbackReaches(window, *booth.player, 1500);
    CHECK(currentRow(window) == 0);

    playbackReaches(window, *booth.player, 5500);
    CHECK(currentRow(window) == 2);
}

// The selection is what an operation applies to. A film playing in the corner
// of the screen has no business rewriting the user's target row by row.
TEST_CASE("following playback leaves the selection alone", "[gui][GUI-PLAYER-01]") {
    InMemoryFileSystem files = directoryHolding({"film.mkv"});
    FakePrompts prompts;
    Projectionist booth;
    MainWindow window{files, fileIn(files, "/films/film.fr.srt"), prompts, projecting(booth)};
    REQUIRE(booth.player != nullptr);

    selectRow(window, 0);
    const std::size_t placed = booth.player->seeks.size();

    playbackReaches(window, *booth.player, 5500);

    CHECK(currentRow(window) == 2);
    CHECK(window.table()->selectionModel()->selectedRows().size() == 1);
    CHECK(window.table()->selectionModel()->selectedRows().front().row() == 0);
    // And the row playback moved to did not send playback anywhere.
    CHECK(booth.player->seeks.size() == placed);
}

// The defect this phase was told to expect: a film advancing while somebody is
// typing. Moving the current cell closes the editor open on it.
TEST_CASE("an edit in progress survives playback advancing", "[gui][GUI-PLAYER-01]") {
    InMemoryFileSystem files = directoryHolding({"film.mkv"});
    FakePrompts prompts;
    Projectionist booth;
    MainWindow window{files, fileIn(files, "/films/film.fr.srt"), prompts, projecting(booth)};
    REQUIRE(booth.player != nullptr);

    const QModelIndex edited = window.table()->model()->index(0, SubtitleTableModel::Text);
    window.table()->selectionModel()->setCurrentIndex(edited, QItemSelectionModel::NoUpdate);
    window.table()->edit(edited);
    REQUIRE(window.table()->isEditing());

    playbackReaches(window, *booth.player, 5500);

    CHECK(window.table()->isEditing());
    CHECK(currentRow(window) == 0);
    // The replica follows all the same: drawing on the picture disturbs nobody.
    CHECK(booth.player->onScreen() == "Trois.");
}

TEST_CASE("opening a document with no film takes the picture away", "[gui][GUI-PLAYER-01]") {
    InMemoryFileSystem files = directoryHolding({"film.mkv"});
    files.addFile("/autre/seul.srt", kThree);
    FakePrompts prompts;
    Projectionist booth;
    const MainWindow window{files, fileIn(files, "/films/film.fr.srt"), prompts, projecting(booth)};
    REQUIRE(booth.player != nullptr);
    window.playPauseAction()->trigger();
    REQUIRE(booth.player->isPlaying());

    prompts.nextFileToOpen = "/autre/seul.srt";
    window.openAction()->trigger();

    CHECK_FALSE(showsPicture(window));
    CHECK_FALSE(window.playPauseAction()->isEnabled());
    // And the film left behind is not still playing under a document that no
    // longer shows it.
    CHECK_FALSE(booth.player->isPlaying());
    CHECK(booth.player->onScreen().empty());
}

TEST_CASE("choosing another film opens that one", "[gui][GUI-PLAYER-01]") {
    InMemoryFileSystem files = directoryHolding({"film.mkv"});
    FakePrompts prompts;
    Projectionist booth;
    prompts.nextVideoToOpen = "/ailleurs/le-bon-montage.mkv";
    const MainWindow window{files, fileIn(files, "/films/film.fr.srt"), prompts, projecting(booth)};
    REQUIRE(booth.player != nullptr);

    window.selectVideoAction()->trigger();

    CHECK(booth.player->opened.size() == 2U);
    CHECK(booth.player->opened.back() == std::filesystem::path{"/ailleurs/le-bon-montage.mkv"});
    // One player for the life of the window: the surface it draws into cannot
    // change, and neither can the player that adopted it.
    CHECK(booth.built == 1);
}

// Everything above drives the follower by hand, which is what keeps these cases
// off a clock. This one case pays for the clock, and it is the only thing that
// says the ticker is wired at all.
TEST_CASE("the window follows playback on its own", "[gui][GUI-PLAYER-01]") {
    InMemoryFileSystem files = directoryHolding({"film.mkv"});
    FakePrompts prompts;
    Projectionist booth;
    const MainWindow window{files, fileIn(files, "/films/film.fr.srt"), prompts, projecting(booth)};
    REQUIRE(booth.player != nullptr);

    booth.player->where = Timestamp::fromMilliseconds(5500);
    QTest::qWait(300);

    CHECK(booth.player->onScreen() == "Trois.");
    CHECK(currentRow(window) == 2);
}
