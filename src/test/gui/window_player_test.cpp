// The film in the window — issue #176, and the gesture phase 6 exists to serve.
//
// **The player behind the window is a double here, and that is not a shortcut.**
// The real one is proved on the fixtures of #163, in `mpv_player_test.cpp`; it
// cannot also be proved through the window without decoding a film in every case,
// and the picture it paints has its own cases (`video_surface_test.cpp`). What
// belongs here is everything the window
// decides: when a film is opened, where playback is placed, what the replica
// says, which row follows it, and who gives way to whom.

#include <subedit/core/config/settings.hpp>
#include <subedit/core/format/project_file.hpp>
#include <subedit/core/io/in_memory_file_system.hpp>
#include <subedit/core/model/document.hpp>
#include <subedit/core/model/project.hpp>
#include <subedit/core/model/source_file.hpp>
#include <subedit/core/model/subtitle.hpp>
#include <subedit/core/model/subtitle_format.hpp>
#include <subedit/core/time/frame_rate.hpp>
#include <subedit/core/time/timestamp.hpp>
#include <subedit/core/video/video_player.hpp>
#include <subedit/core/wording/video.hpp>
#include <subedit/gui/main_window.hpp>
#include <subedit/gui/play_bar.hpp>
#include <subedit/gui/subtitle_table.hpp>
#include <subedit/gui/subtitle_table_model.hpp>

#include <QAbstractItemModel>
#include <QAction>
#include <QCoreApplication>
#include <QItemSelectionModel>
#include <QLabel>
#include <QMenu>
#include <QModelIndex>
#include <QStringList>
#include <QTest>
#include <QToolButton>
#include <catch2/catch_test_macros.hpp>

#include <array>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "fake_prompts.hpp"
#include "fake_video_player.hpp"
#include "player_harness.hpp"
#include "waiting.hpp"

namespace {

using subedit::core::Document;
using subedit::core::FrameRate;
using subedit::core::InMemoryFileSystem;
using subedit::core::noFrameToCountBy;
using subedit::core::OpenedFile;
using subedit::core::PlayerError;
using subedit::core::Settings;
using subedit::core::SourceFile;
using subedit::core::StandardFrameRate;
using subedit::core::Subtitle;
using subedit::core::SubtitleFormat;
using subedit::core::Timestamp;
using subedit::gui::MainWindow;
using subedit::gui::SubtitleTableModel;
using subedit::test::FakePrompts;
using subedit::test::FakeVideoPlayer;
using subedit::test::fileIn;
using subedit::test::projecting;
using subedit::test::Projectionist;
using subedit::test::waitUntil;

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
/// A directory holding a subtitle file, and whatever else the case needs.
[[nodiscard]] InMemoryFileSystem directoryHolding(std::initializer_list<const char*> names) {
    InMemoryFileSystem files;
    for (const char* name : names)
        files.addFile(std::filesystem::path{"/films"} / name, "");
    files.addFile("/films/film.fr.srt", kThree);
    return files;
}

/// The same file, with a translation of each of its three subtitles.
[[nodiscard]] OpenedFile translatedIn(const InMemoryFileSystem& files, const char* path) {
    OpenedFile opened = fileIn(files, path);
    std::vector<Subtitle> subtitles{opened.project.subtitles().begin(),
                                    opened.project.subtitles().end()};
    constexpr std::array<const char*, 3> kTranslations = {"One.", "Two.", "Three."};
    for (std::size_t row = 0; row < subtitles.size() && row < kTranslations.size(); ++row)
        subtitles[row].translationText = kTranslations[row];
    opened.project.setSubtitles(std::move(subtitles));
    opened.project.setSourceFile(Document::Translation,
                                 SourceFile{.format = SubtitleFormat::SubRip});
    return opened;
}

/// Puts the current cell there, without selecting anything.
void currentAt(const MainWindow& window, int row, int column) {
    window.table()->selectionModel()->setCurrentIndex(window.table()->model()->index(row, column),
                                                      QItemSelectionModel::NoUpdate);
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
    MainWindow window{files, fileIn(files, "/films/film.fr.srt"), prompts, projecting(booth)};
    window.show();

    REQUIRE(booth.player != nullptr);
    CHECK(booth.player->opened == std::vector<std::filesystem::path>{"/films/film.mkv"});
    CHECK(showsPicture(window));
    CHECK(window.playPauseAction()->isEnabled());
}

// Built by the window, when a film first needs one — and once.
TEST_CASE("the player is built once, when the first film is shown", "[gui][GUI-PLAYER-01]") {
    InMemoryFileSystem files = directoryHolding({"film.mkv"});
    FakePrompts prompts;
    Projectionist booth;
    MainWindow window{files, fileIn(files, "/films/film.fr.srt"), prompts, projecting(booth)};
    window.show();

    CHECK(booth.built == 1);
}

TEST_CASE("a document with no film has no picture and nothing to play", "[gui][GUI-PLAYER-01]") {
    InMemoryFileSystem files = directoryHolding({});
    FakePrompts prompts;
    Projectionist booth;
    MainWindow window{files, fileIn(files, "/films/film.fr.srt"), prompts, projecting(booth)};
    window.show();

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
    MainWindow window{files, fileIn(files, "/films/film.fr.srt"), prompts, projecting(booth)};
    window.show();
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
    MainWindow window{files, fileIn(files, "/films/film.fr.srt"), prompts, projecting(booth)};
    window.show();
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
    MainWindow window{files, fileIn(files, "/films/film.fr.srt"), prompts, projecting(booth)};
    window.show();
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
    MainWindow window{files, fileIn(files, "/films/film.fr.srt"), prompts, projecting(booth)};
    window.show();
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
    MainWindow window{files, fileIn(files, "/films/film.fr.srt"), prompts, projecting(booth)};
    window.show();

    selectRow(window, 1);

    CHECK(booth.built == 0);
}

TEST_CASE("a film that will not open is named, and the window stays usable",
          "[gui][GUI-PLAYER-03]") {
    InMemoryFileSystem files = directoryHolding({"film.mkv"});
    FakePrompts prompts;
    Projectionist booth;
    booth.refusal = PlayerError{.reason = "unrecognized file format"};
    MainWindow window{files, fileIn(files, "/films/film.fr.srt"), prompts, projecting(booth)};
    window.show();

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
    MainWindow window{files, fileIn(files, "/films/film.fr.srt"), prompts, projecting(booth)};
    window.show();
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
    MainWindow window{files, fileIn(files, "/films/film.fr.srt"), prompts, projecting(booth)};
    window.show();

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
    window.show();
    REQUIRE(booth.player != nullptr);

    playbackReaches(window, *booth.player, 1500);
    CHECK(booth.player->onScreen() == "Un.");

    playbackReaches(window, *booth.player, 3000);
    CHECK(booth.player->onScreen() == "Deux.");

    // Between two subtitles the picture carries nothing.
    playbackReaches(window, *booth.player, 4000);
    CHECK(booth.player->onScreen().empty());
}

// ## Driving the film from the menu — issue #615

namespace {

/// The gestures, in the order the menu lists them — the nine of #615, the five of #617, the six of
/// #618.
[[nodiscard]] std::array<QAction*, 20> gestures(const MainWindow& window) {
    return {window.playSelectionAction(),
            window.seekPreviousAction(),
            window.seekNextAction(),
            window.seekBackwardAction(),
            window.seekForwardAction(),
            window.seekSelectionStartAction(),
            window.seekSelectionEndAction(),
            window.volumeDownAction(),
            window.volumeUpAction(),
            window.setStartFromVideoAction(),
            window.setEndFromVideoAction(),
            window.insertAtVideoAction(),
            window.selectPreviousFromVideoAction(),
            window.selectNextFromVideoAction(),
            window.stepBackwardAction(),
            window.stepForwardAction(),
            window.nudgeStartEarlierAction(),
            window.nudgeStartLaterAction(),
            window.nudgeEndEarlierAction(),
            window.nudgeEndLaterAction()};
}

void selectRows(const MainWindow& window, int first, int last) {
    window.table()->selectionModel()->clearSelection();
    for (int row = first; row <= last; ++row)
        window.table()->selectionModel()->select(window.table()->model()->index(row, 0),
                                                 QItemSelectionModel::Select |
                                                     QItemSelectionModel::Rows);
}

} // namespace

// Out without a film, as playing is: a gesture on nothing does nothing.
TEST_CASE("the gestures that drive the film are out without one", "[gui][GUI-SEEK-02]") {
    InMemoryFileSystem files;
    files.addFile("/films/seul.srt", kThree);
    FakePrompts prompts;
    Projectionist booth;
    MainWindow window{files, fileIn(files, "/films/seul.srt"), prompts, projecting(booth)};
    window.show();

    for (const QAction* gesture : gestures(window))
        CHECK_FALSE(gesture->isEnabled());
}

TEST_CASE("with a film, the gestures are in, and three of them wait for a selection",
          "[gui][GUI-SEEK-02]") {
    InMemoryFileSystem files = directoryHolding({"film.mkv"});
    FakePrompts prompts;
    Projectionist booth;
    MainWindow window{files, fileIn(files, "/films/film.fr.srt"), prompts, projecting(booth)};
    window.show();
    REQUIRE(booth.player != nullptr);
    window.table()->selectionModel()->clearSelection();

    CHECK(window.seekBackwardAction()->isEnabled());
    CHECK(window.seekForwardAction()->isEnabled());
    CHECK(window.seekPreviousAction()->isEnabled());
    CHECK(window.seekNextAction()->isEnabled());
    CHECK(window.volumeDownAction()->isEnabled());
    CHECK(window.volumeUpAction()->isEnabled());
    // The three that act on the selection have nothing to act on.
    CHECK_FALSE(window.playSelectionAction()->isEnabled());
    CHECK_FALSE(window.seekSelectionStartAction()->isEnabled());
    CHECK_FALSE(window.seekSelectionEndAction()->isEnabled());

    selectRows(window, 0, 1);
    CHECK(window.playSelectionAction()->isEnabled());
    CHECK(window.seekSelectionStartAction()->isEnabled());
    CHECK(window.seekSelectionEndAction()->isEnabled());
}

TEST_CASE("the gestures of the menu drive the film", "[gui][GUI-SEEK-02][GUI-SEEK-05]") {
    InMemoryFileSystem files = directoryHolding({"film.mkv"});
    FakePrompts prompts;
    Projectionist booth;
    MainWindow window{files, fileIn(files, "/films/film.fr.srt"), prompts, projecting(booth)};
    window.show();
    REQUIRE(booth.player != nullptr);
    booth.player->where = Timestamp::fromMilliseconds(100000);

    window.seekForwardAction()->trigger();
    CHECK(booth.player->seeks.back() == Timestamp::fromMilliseconds(130000));

    window.seekBackwardAction()->trigger();
    CHECK(booth.player->seeks.back() == Timestamp::fromMilliseconds(100000));

    // The three subtitles of the file: 1000, 2500 and 5000 ms.
    booth.player->where = Timestamp::fromMilliseconds(1200);
    window.seekNextAction()->trigger();
    CHECK(booth.player->seeks.back() == Timestamp::fromMilliseconds(2500));

    // The previous one is the last to have ended before the position: 2500 is the start of the
    // second, so the first, which ended at 2000, is the one.
    window.seekPreviousAction()->trigger();
    CHECK(booth.player->seeks.back() == Timestamp::fromMilliseconds(1000));

    selectRows(window, 1, 2);
    booth.player->seeks.clear();
    window.seekSelectionStartAction()->trigger();
    CHECK(booth.player->seeks.back() == Timestamp::fromMilliseconds(1500));
    window.seekSelectionEndAction()->trigger();
    CHECK(booth.player->seeks.back() == Timestamp::fromMilliseconds(5000));

    window.playSelectionAction()->trigger();
    CHECK(booth.player->stops.back() == Timestamp::fromMilliseconds(6000));
}

TEST_CASE("the gestures of the volume move it from five to five", "[gui][GUI-VOLUME-01]") {
    InMemoryFileSystem files = directoryHolding({"film.mkv"});
    FakePrompts prompts;
    Projectionist booth;
    MainWindow window{files, fileIn(files, "/films/film.fr.srt"), prompts, projecting(booth)};
    window.applySettings(subedit::core::Settings{.video = {.volume = 50}});
    window.show();
    REQUIRE(booth.player != nullptr);
    CHECK(booth.player->volume() == 50);

    window.volumeDownAction()->trigger();
    CHECK(booth.player->volume() == 45);

    window.volumeUpAction()->trigger();
    window.volumeUpAction()->trigger();
    CHECK(booth.player->volume() == 55);
    CHECK(window.settings().video.volume == 55);
}

// Issue #647: the permission reaches the player when it is built, and again when it changes.
TEST_CASE("the hardware decoding setting reaches the player", "[gui][video]") {
    InMemoryFileSystem files = directoryHolding({"film.mkv"});
    FakePrompts prompts;
    Projectionist booth;
    MainWindow window{files, fileIn(files, "/films/film.fr.srt"), prompts, projecting(booth)};
    window.applySettings(subedit::core::Settings{.video = {.hardwareDecoding = false}});
    window.show();
    REQUIRE(booth.player != nullptr);
    CHECK_FALSE(booth.player->hardwareDecoding);

    window.applySettings(subedit::core::Settings{.video = {.hardwareDecoding = true}});
    CHECK(booth.player->hardwareDecoding);
}

// The bar is under the picture, in the room above the table, and shown with it.
TEST_CASE("the bar is shown with the picture and hidden with it", "[gui][GUI-SEEK-01]") {
    InMemoryFileSystem files = directoryHolding({"film.mkv"});
    FakePrompts prompts;
    Projectionist booth;
    MainWindow window{files, fileIn(files, "/films/film.fr.srt"), prompts, projecting(booth)};
    window.show();

    REQUIRE(window.playBar() != nullptr);
    CHECK_FALSE(window.playBar()->isHidden());
    CHECK(window.playBar()->parentWidget() == window.videoView()->parentWidget());
}

// ## The audio tracks, in the menu — issue #616, GUI-AUDIO-01

namespace {

/// The labels of the entries of the menu of languages.
[[nodiscard]] QStringList entriesOf(const MainWindow& window) {
    QStringList labels;
    for (const QAction* entry : window.audioLanguageMenu()->actions())
        labels << entry->text();
    return labels;
}

} // namespace

TEST_CASE("the menu of languages lists the tracks of the film and marks the one that plays",
          "[gui][GUI-AUDIO-01]") {
    InMemoryFileSystem files = directoryHolding({"film.mkv"});
    FakePrompts prompts;
    Projectionist booth;
    booth.tracks = {{.id = 1, .language = "fra", .title = "Original", .selected = true},
                    {.id = 2, .language = "eng", .title = "Commentary", .selected = false}};
    MainWindow window{files, fileIn(files, "/films/film.fr.srt"), prompts, projecting(booth)};
    window.show();
    REQUIRE(booth.player != nullptr);

    CHECK(window.audioLanguageMenu()->menuAction()->isEnabled());
    CHECK(entriesOf(window) ==
          QStringList{QStringLiteral("1: fra — Original"), QStringLiteral("2: eng — Commentary")});
    CHECK(window.audioLanguageMenu()->actions().at(0)->isChecked());
    CHECK_FALSE(window.audioLanguageMenu()->actions().at(1)->isChecked());
}

TEST_CASE("choosing a language plays that track, and the mark follows", "[gui][GUI-AUDIO-01]") {
    InMemoryFileSystem files = directoryHolding({"film.mkv"});
    FakePrompts prompts;
    Projectionist booth;
    booth.tracks = {{.id = 1, .language = "fra", .title = "", .selected = true},
                    {.id = 2, .language = "eng", .title = "", .selected = false}};
    MainWindow window{files, fileIn(files, "/films/film.fr.srt"), prompts, projecting(booth)};
    window.show();
    REQUIRE(booth.player != nullptr);

    window.audioLanguageMenu()->actions().at(1)->trigger();

    // The player is the one that says which plays, and the menu shows its answer.
    CHECK(booth.player->tracks.at(1).selected);
    CHECK_FALSE(booth.player->tracks.at(0).selected);
    CHECK(window.audioLanguageMenu()->actions().at(1)->isChecked());
    CHECK_FALSE(window.audioLanguageMenu()->actions().at(0)->isChecked());
}

TEST_CASE("the menu of languages is out with no film and in with a single track",
          "[gui][GUI-AUDIO-01]") {
    {
        InMemoryFileSystem files;
        files.addFile("/films/seul.srt", kThree);
        FakePrompts prompts;
        Projectionist booth;
        MainWindow window{files, fileIn(files, "/films/seul.srt"), prompts, projecting(booth)};
        window.show();

        CHECK_FALSE(window.audioLanguageMenu()->menuAction()->isEnabled());
        CHECK(window.audioLanguageMenu()->actions().isEmpty());
    }
    {
        InMemoryFileSystem files = directoryHolding({"film.mkv"});
        FakePrompts prompts;
        Projectionist booth;
        booth.tracks = {{.id = 1, .language = "fra", .title = "", .selected = true}};
        MainWindow window{files, fileIn(files, "/films/film.fr.srt"), prompts, projecting(booth)};
        window.show();
        REQUIRE(booth.player != nullptr);

        CHECK(window.audioLanguageMenu()->menuAction()->isEnabled());
        CHECK(window.audioLanguageMenu()->actions().size() == 1);
    }
}

// The volume is in the same menu, and it must stay within reach when there is no choice of
// language to make: the gestures work whatever the track menu says.
TEST_CASE("the volume stays within reach when the languages are out", "[gui][GUI-AUDIO-01]") {
    InMemoryFileSystem files = directoryHolding({"film.mkv"});
    FakePrompts prompts;
    Projectionist booth;
    MainWindow window{files, fileIn(files, "/films/film.fr.srt"), prompts, projecting(booth)};
    window.show();
    REQUIRE(booth.player != nullptr);

    CHECK_FALSE(window.audioLanguageMenu()->menuAction()->isEnabled());
    CHECK(window.volumeDownAction()->isEnabled());
    CHECK(window.volumeUpAction()->isEnabled());
}

// Issue #408, GUI-REPLICA-01: a subtitle is held as its file wrote it, and the picture used to
// draw its tags as letters. What the player is handed is the text with them understood.
TEST_CASE("the replica handed to the player has its tags understood", "[gui][GUI-REPLICA-01]") {
    InMemoryFileSystem files;
    files.addFile("/films/film.mkv", "");
    files.addFile("/films/film.fr.srt",
                  "1\n00:00:01,000 --> 00:00:02,000\n<i>Un</i> {mot}.\n\n"
                  "2\n00:00:03,000 --> 00:00:04,000\n<font size=\"3\">Deux.</font>\n\n");
    FakePrompts prompts;
    Projectionist booth;
    MainWindow window{files, fileIn(files, "/films/film.fr.srt"), prompts, projecting(booth)};
    window.show();
    REQUIRE(booth.player != nullptr);

    playbackReaches(window, *booth.player, 1500);
    // The italic is a block of the overlay, and the braces of the visible text are escaped.
    CHECK(booth.player->onScreen() == "{\\i1}Un{\\i0} \\{mot\\}.");

    // A size is not one the overlay can honour: the tag is gone, the text is not.
    playbackReaches(window, *booth.player, 3500);
    CHECK(booth.player->onScreen() == "Deux.");
}

// The translation is held in the format of its own file, not of the main one: the vocabulary
// that reads it is the one that file was written in.
TEST_CASE("a translation is read in the format of its own file", "[gui][GUI-REPLICA-01]") {
    InMemoryFileSystem files = directoryHolding({"film.mkv"});
    FakePrompts prompts;
    Projectionist booth;
    OpenedFile opened = translatedIn(files, "/films/film.fr.srt");
    std::vector<Subtitle> subtitles{opened.project.subtitles().begin(),
                                    opened.project.subtitles().end()};
    subtitles[0].translationText = "{\\i1}One{\\i0}.";
    opened.project.setSubtitles(std::move(subtitles));
    opened.project.setSourceFile(Document::Translation,
                                 SourceFile{.format = SubtitleFormat::AdvancedSubStationAlpha});
    MainWindow window{files, std::move(opened), prompts, projecting(booth)};
    window.show();
    REQUIRE(booth.player != nullptr);

    currentAt(window, 0, SubtitleTableModel::Translation);
    playbackReaches(window, *booth.player, 1500);

    CHECK(booth.player->onScreen() == "{\\i1}One{\\i0}.");
}

// Decision D2, and the whole reason the replica is not a file: what is on the
// picture is what was just typed, with nothing written to a disk in between.
TEST_CASE("an edited text reaches the picture", "[gui][GUI-PLAYER-01]") {
    InMemoryFileSystem files = directoryHolding({"film.mkv"});
    FakePrompts prompts;
    Projectionist booth;
    MainWindow window{files, fileIn(files, "/films/film.fr.srt"), prompts, projecting(booth)};
    window.show();
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
    window.show();
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
    window.show();
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
    window.show();
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
    window.show();
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
    MainWindow window{files, fileIn(files, "/films/film.fr.srt"), prompts, projecting(booth)};
    window.show();
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
    MainWindow window{files, fileIn(files, "/films/film.fr.srt"), prompts, projecting(booth)};
    window.show();
    REQUIRE(booth.player != nullptr);

    window.selectVideoAction()->trigger();

    CHECK(booth.player->opened.size() == 2U);
    CHECK(booth.player->opened.back() == std::filesystem::path{"/ailleurs/le-bon-montage.mkv"});
    // One player for the life of the window: the surface it is painted on does
    // not change, and the player has no reason to.
    CHECK(booth.built == 1);
}

// A window hidden and shown again does not open the film a second time. It is
// already open, and opening it afresh would send playback back to the start
// under somebody who had just placed it where they wanted it.
TEST_CASE("showing the window again does not open the film again", "[gui][GUI-PLAYER-01]") {
    InMemoryFileSystem files = directoryHolding({"film.mkv"});
    FakePrompts prompts;
    Projectionist booth;
    MainWindow window{files, fileIn(files, "/films/film.fr.srt"), prompts, projecting(booth)};
    window.show();
    REQUIRE(booth.player != nullptr);
    REQUIRE(booth.player->opened.size() == 1U);

    window.hide();
    window.show();

    CHECK(booth.player->opened.size() == 1U);
}

// Everything above drives the follower by hand, which is what keeps these cases
// off a clock. This one case pays for the clock, and it is the only thing that
// says the ticker is wired at all.
TEST_CASE("the window follows playback on its own", "[gui][GUI-PLAYER-01]") {
    InMemoryFileSystem files = directoryHolding({"film.mkv"});
    FakePrompts prompts;
    Projectionist booth;
    MainWindow window{files, fileIn(files, "/films/film.fr.srt"), prompts, projecting(booth)};
    window.show();
    REQUIRE(booth.player != nullptr);

    booth.player->where = Timestamp::fromMilliseconds(5500);
    // The ticker is what shows the line, so the case waits for the line and not for its period.
    CHECK(waitUntil([&booth] { return booth.player->onScreen() == "Trois."; }));
    CHECK(currentRow(window) == 2);
}

// Decision D8, and the return of the phase-6 question « which of the two texts
// is drawn » — answered by the rule the whole window follows, and not by a
// setting: the document aimed at is the one of the current column.
TEST_CASE("the replica drawn follows the column of the current cell", "[gui][GUI-PLAYER-04]") {
    InMemoryFileSystem files = directoryHolding({"film.mkv"});
    FakePrompts prompts;
    Projectionist booth;
    MainWindow window{files, translatedIn(files, "/films/film.fr.srt"), prompts, projecting(booth)};
    window.show();
    REQUIRE(booth.player != nullptr);

    playbackReaches(window, *booth.player, 1500);
    CHECK(booth.player->onScreen() == "Un.");

    currentAt(window, 0, SubtitleTableModel::Translation);
    window.followPlayback();
    CHECK(booth.player->onScreen() == "One.");

    // The film goes on, and the current cell keeps its column while it follows.
    playbackReaches(window, *booth.player, 3000);
    CHECK(booth.player->onScreen() == "Two.");

    currentAt(window, 1, SubtitleTableModel::Text);
    window.followPlayback();
    CHECK(booth.player->onScreen() == "Deux.");
}

// A column nobody can see is not one a cell is current in — the rule of
// `targetDocument`, and the picture must not show a text the table hides.
TEST_CASE("a hidden translation column is not the one the replica is drawn from",
          "[gui][GUI-PLAYER-04]") {
    InMemoryFileSystem files = directoryHolding({"film.mkv"});
    FakePrompts prompts;
    Projectionist booth;
    MainWindow window{files, translatedIn(files, "/films/film.fr.srt"), prompts, projecting(booth)};
    window.show();
    REQUIRE(booth.player != nullptr);
    playbackReaches(window, *booth.player, 1500);
    currentAt(window, 0, SubtitleTableModel::Translation);
    window.followPlayback();
    REQUIRE(booth.player->onScreen() == "One.");

    window.translationColumnAction()->trigger();
    window.followPlayback();

    CHECK(booth.player->onScreen() == "Un.");
}

// The empty answer is the translation's own: a subtitle with nothing translated
// yet draws nothing while its translation is the aimed text, and not the main
// text in its place.
TEST_CASE("a subtitle with no translation draws nothing from the translation column",
          "[gui][GUI-PLAYER-04]") {
    InMemoryFileSystem files = directoryHolding({"film.mkv"});
    FakePrompts prompts;
    Projectionist booth;
    OpenedFile opened = translatedIn(files, "/films/film.fr.srt");
    std::vector<Subtitle> subtitles{opened.project.subtitles().begin(),
                                    opened.project.subtitles().end()};
    subtitles[0].translationText.clear();
    opened.project.setSubtitles(std::move(subtitles));
    MainWindow window{files, std::move(opened), prompts, projecting(booth)};
    window.show();
    REQUIRE(booth.player != nullptr);
    currentAt(window, 0, SubtitleTableModel::Translation);

    playbackReaches(window, *booth.player, 1500);

    CHECK(booth.player->onScreen().empty());
}

// Issue #469: the share of the table a session left behind is laid down with
// no film shown. The picture that comes afterwards must get room of its own,
// and not a slot of no height with the table drawn over it.
TEST_CASE("a film opened under a saved table share gets room of its own", "[gui][GUI-PLAYER-01]") {
    InMemoryFileSystem files = directoryHolding({"autre.mkv"});
    FakePrompts prompts;
    Projectionist booth;
    MainWindow window{files, fileIn(files, "/films/film.fr.srt"), prompts, projecting(booth)};
    window.applySettings(subedit::core::Settings{.tableShare = 35});
    window.show();
    QCoreApplication::processEvents();

    prompts.nextVideoToOpen = "/films/autre.mkv";
    window.selectVideoAction()->trigger();
    QCoreApplication::processEvents();

    REQUIRE(showsPicture(window));
    CHECK(window.videoView()->height() >= 180);
    // Above the table, and not under it.
    CHECK(window.videoView()->geometry().bottom() < window.table()->geometry().top());
}

TEST_CASE("the room of the picture goes back to the band when the film goes",
          "[gui][GUI-PLAYER-01]") {
    InMemoryFileSystem files = directoryHolding({"film.mkv"});
    FakePrompts prompts;
    Projectionist booth;
    booth.refusal = PlayerError{.reason = "unrecognized file format"};
    MainWindow window{files, fileIn(files, "/films/film.fr.srt"), prompts, projecting(booth)};
    window.applySettings(subedit::core::Settings{.tableShare = 35});
    window.show();
    QCoreApplication::processEvents();

    // The film would not open: the band is back, above the table.
    REQUIRE_FALSE(showsPicture(window));
    CHECK(window.noVideoBanner()->geometry().bottom() < window.table()->geometry().top());
}

// Issue #470, and its lesson kept under ADR 0041: the surface and the player call each
// other — one paints what the other draws, the other announces from a thread of its own
// — and neither may outlive the other. The player is let go while the window is still
// there, with the surface let go of it first.
TEST_CASE("closing the window lets the player go while the window still exists",
          "[gui][GUI-PLAYER-01]") {
    InMemoryFileSystem files = directoryHolding({"film.mkv"});
    FakePrompts prompts;
    Projectionist booth;
    MainWindow window{files, fileIn(files, "/films/film.fr.srt"), prompts, projecting(booth)};
    window.show();
    REQUIRE(booth.player != nullptr);
    window.playPauseAction()->trigger();
    REQUIRE(booth.player->isPlaying());

    bool gone = false;
    bool windowWhenGone = false;
    booth.player->onDestroyed = [&] {
        gone = true;
        windowWhenGone = window.videoView() != nullptr;
    };

    REQUIRE(window.close());

    CHECK(gone);
    CHECK(windowWhenGone);
    CHECK_FALSE(window.playPauseAction()->isEnabled());
}

// ## Marks taken from the position of the film — issue #617

namespace {

/// What a cell holds, seen from the window.
[[nodiscard]] std::string cell(const MainWindow& window, int row, int column) {
    return window.table()
        ->model()
        ->data(window.table()->model()->index(row, column), Qt::DisplayRole)
        .toString()
        .toStdString();
}

/// A window on the three subtitles, with a film whose position is `at` milliseconds.
struct MarkedWindow {
    InMemoryFileSystem files = directoryHolding({"film.mkv"});
    FakePrompts prompts;
    Projectionist booth;
    MainWindow window;

    /// Where the film stands: said after a selection, which places playback at its first row.
    void at(int milliseconds) const {
        booth.player->where = Timestamp::fromMilliseconds(milliseconds);
    }

    explicit MarkedWindow(int at)
        : window{files, fileIn(files, "/films/film.fr.srt"), prompts, projecting(booth)} {
        window.show();
        REQUIRE(booth.player != nullptr);
        this->at(at);
    }
};

[[nodiscard]] std::string start(const MainWindow& window, int row) {
    return cell(window, row, 1);
}

[[nodiscard]] std::string end(const MainWindow& window, int row) {
    return cell(window, row, 2);
}

[[nodiscard]] int selectedRow(const MainWindow& window) {
    const QModelIndexList rows = window.table()->selectionModel()->selectedRows();
    return rows.size() == 1 ? rows.front().row() : -1;
}

} // namespace

TEST_CASE("the marks that need a selection wait for one, the others need only a film",
          "[gui][GUI-MARK-01]") {
    const MarkedWindow marked{1200};
    marked.window.table()->selectionModel()->clearSelection();

    CHECK(marked.window.insertAtVideoAction()->isEnabled());
    CHECK(marked.window.selectPreviousFromVideoAction()->isEnabled());
    CHECK(marked.window.selectNextFromVideoAction()->isEnabled());
    CHECK_FALSE(marked.window.setStartFromVideoAction()->isEnabled());
    CHECK_FALSE(marked.window.setEndFromVideoAction()->isEnabled());

    selectRows(marked.window, 0, 0);
    marked.at(1200);
    CHECK(marked.window.setStartFromVideoAction()->isEnabled());
    CHECK(marked.window.setEndFromVideoAction()->isEnabled());
}

TEST_CASE("setting the start from the position is one entry of the history", "[gui][GUI-MARK-01]") {
    const MarkedWindow marked{1200};
    selectRows(marked.window, 0, 0);
    marked.at(1200);

    marked.window.setStartFromVideoAction()->trigger();

    CHECK(start(marked.window, 0) == "00:00:01,200");
    CHECK(end(marked.window, 0) == "00:00:02,000");

    marked.window.undoAction()->trigger();
    CHECK(start(marked.window, 0) == "00:00:01,000");
    CHECK_FALSE(marked.window.undoAction()->isEnabled());
}

TEST_CASE("setting the end from the position is one entry of the history", "[gui][GUI-MARK-02]") {
    const MarkedWindow marked{1800};
    selectRows(marked.window, 0, 0);
    marked.at(1800);

    marked.window.setEndFromVideoAction()->trigger();

    CHECK(start(marked.window, 0) == "00:00:01,000");
    CHECK(end(marked.window, 0) == "00:00:01,800");

    marked.window.undoAction()->trigger();
    CHECK(end(marked.window, 0) == "00:00:02,000");
    CHECK_FALSE(marked.window.undoAction()->isEnabled());
}

// What the cell does with a start typed after the end: it lets it stand, and the table flags
// the subtitle. The gesture is the same command, so it says the same.
TEST_CASE("a start set after the end stands and is flagged as the cell's would be",
          "[gui][GUI-MARK-01]") {
    const MarkedWindow marked{4000};
    QAbstractItemModel* model = marked.window.table()->model();
    REQUIRE(model->setData(model->index(1, 1), QStringLiteral("00:00:04,000"), Qt::EditRole));
    const auto flagged = [&](int row) {
        return marked.window.table()->model()->data(marked.window.table()->model()->index(row, 1),
                                                    Qt::BackgroundRole);
    };
    const QVariant byTyping = flagged(1);

    selectRows(marked.window, 0, 0);
    marked.at(4000);
    marked.window.setStartFromVideoAction()->trigger();

    CHECK(start(marked.window, 0) == "00:00:04,000");
    CHECK(end(marked.window, 0) == "00:00:02,000");
    // Flagged, in the way the cell flagged a start past its own end.
    CHECK(flagged(0).isValid());
    CHECK(flagged(0) == byTyping);
}

TEST_CASE("setting a mark does nothing when the position already is that mark",
          "[gui][GUI-MARK-01]") {
    const MarkedWindow marked{1000};
    selectRows(marked.window, 0, 0);
    marked.at(1000);

    marked.window.setStartFromVideoAction()->trigger();

    CHECK_FALSE(marked.window.undoAction()->isEnabled());
}

TEST_CASE("a subtitle is inserted at the position, before the next one, and selected",
          "[gui][GUI-MARK-03]") {
    const MarkedWindow marked{3600};

    marked.window.insertAtVideoAction()->trigger();

    // After the two that start before 3600 ms, and before the one at 5000 ms.
    REQUIRE(marked.window.table()->model()->rowCount({}) == 4);
    CHECK(start(marked.window, 2) == "00:00:03,600");
    CHECK(end(marked.window, 2) == "00:00:05,000");
    CHECK(start(marked.window, 3) == "00:00:05,000");
    CHECK(selectedRow(marked.window) == 2);

    marked.window.undoAction()->trigger();
    CHECK(marked.window.table()->model()->rowCount({}) == 3);
    CHECK_FALSE(marked.window.undoAction()->isEnabled());
}

TEST_CASE("an inserted subtitle lasts three seconds when nothing comes sooner",
          "[gui][GUI-MARK-03]") {
    const MarkedWindow marked{7000};

    marked.window.insertAtVideoAction()->trigger();

    REQUIRE(marked.window.table()->model()->rowCount({}) == 4);
    CHECK(start(marked.window, 3) == "00:00:07,000");
    CHECK(end(marked.window, 3) == "00:00:10,000");
    CHECK(selectedRow(marked.window) == 3);
}

TEST_CASE("the neighbours are the first to start after the position and the last before it",
          "[gui][GUI-MARK-04]") {
    // The subtitles start at 1000, 2500 and 5000 ms.
    const MarkedWindow marked{3000};

    marked.window.selectNextFromVideoAction()->trigger();
    CHECK(selectedRow(marked.window) == 2);

    marked.window.selectPreviousFromVideoAction()->trigger();
    CHECK(selectedRow(marked.window) == 1);
}

TEST_CASE("with no neighbour on that side, the end of the file that way is selected",
          "[gui][GUI-MARK-04]") {
    const MarkedWindow marked{9000};
    marked.window.selectNextFromVideoAction()->trigger();
    CHECK(selectedRow(marked.window) == 2);

    marked.booth.player->where = Timestamp::fromMilliseconds(100);
    marked.window.selectPreviousFromVideoAction()->trigger();
    CHECK(selectedRow(marked.window) == 0);
}

TEST_CASE("the marks do nothing without a position", "[gui][GUI-MARK-01]") {
    const MarkedWindow marked{1200};
    marked.booth.player->unload();
    selectRows(marked.window, 0, 0);
    marked.at(1200);

    marked.window.setStartFromVideoAction()->trigger();
    marked.window.insertAtVideoAction()->trigger();
    marked.window.selectNextFromVideoAction()->trigger();

    CHECK_FALSE(marked.window.undoAction()->isEnabled());
    CHECK(marked.window.table()->model()->rowCount({}) == 3);
    CHECK(selectedRow(marked.window) == 0);
}

// ## The step, the nudge and the buttons of the bar — issue #618

namespace {

/// A window on the three subtitles with a film that declares `declared`, and a frame step of
/// `stepFrames` frames.
struct SteppedWindow {
    InMemoryFileSystem files = directoryHolding({"film.mkv"});
    FakePrompts prompts;
    Projectionist booth;
    MainWindow window;

    explicit SteppedWindow(std::optional<FrameRate> declared, int stepFrames = 1)
        : window{files,
                 fileIn(files, "/films/film.fr.srt"),
                 prompts,
                 projecting(booth),
                 [declared](const std::filesystem::path&) { return declared; }} {
        window.applySettings(Settings{.video = {.stepFrames = stepFrames}});
        window.show();
        REQUIRE(booth.player != nullptr);
    }
};

/// Waits until the player has been stepped `count` times: a step asked while the gate of the one
/// before is shut is made when it opens, which on a loaded machine is later than a fixed pause.
[[nodiscard]] bool stepsReach(const FakeVideoPlayer& player, std::size_t count) {
    return waitUntil([&player, count] { return player.steps.size() >= count; }) &&
           player.steps.size() == count;
}
} // namespace

TEST_CASE("stepping moves playback by one frame, either way", "[gui][GUI-STEP-01][GUI-STEP-02]") {
    const SteppedWindow stepped{FrameRate{StandardFrameRate::Fps25}};

    stepped.window.stepForwardAction()->trigger();
    stepped.window.stepBackwardAction()->trigger();

    CHECK(stepsReach(*stepped.booth.player, 2));
    CHECK(stepped.booth.player->steps == std::vector<int>{1, -1});
}

TEST_CASE("the step is the frame step of the settings, in frames", "[gui][GUI-STEP-04]") {
    const SteppedWindow stepped{FrameRate{StandardFrameRate::Fps25}, 5};

    stepped.window.stepForwardAction()->trigger();
    stepped.window.stepBackwardAction()->trigger();

    CHECK(stepsReach(*stepped.booth.player, 2));
    CHECK(stepped.booth.player->steps == std::vector<int>{5, -5});
}

// A held key repeats faster than a step is made; what comes in while the gate is shut takes one
// place, and does not queue.
TEST_CASE("a key held down steps one frame at a time and does not pile up", "[gui][GUI-STEP-01]") {
    const SteppedWindow stepped{FrameRate{StandardFrameRate::Fps25}};

    for (int repeat = 0; repeat < 40; ++repeat)
        stepped.window.stepForwardAction()->trigger();

    // The first went at once; the thirty-nine others wait for one place.
    CHECK(stepped.booth.player->steps.size() == 1U);

    CHECK(stepsReach(*stepped.booth.player, 2));

    // And nothing more comes after: the thirty-eight others were not queued. Seen by what comes
    // next and not by waiting for nothing — a step the other way takes the one place there is, and
    // if the others had queued up they would have gone before it.
    stepped.window.stepBackwardAction()->trigger();
    CHECK(stepsReach(*stepped.booth.player, 3));
    CHECK(stepped.booth.player->steps == std::vector<int>{1, 1, -1});
}

TEST_CASE("the step of a window with no selection still needs only the film",
          "[gui][GUI-STEP-01]") {
    const SteppedWindow stepped{FrameRate{StandardFrameRate::Fps25}};
    stepped.window.table()->selectionModel()->clearSelection();

    CHECK(stepped.window.stepForwardAction()->isEnabled());
    CHECK(stepped.window.stepBackwardAction()->isEnabled());
}

TEST_CASE("the buttons of the bar run the actions of the menu, and say what they say",
          "[gui][GUI-STEP-01]") {
    const SteppedWindow stepped{FrameRate{StandardFrameRate::Fps25}};
    const subedit::gui::PlayBar* bar = stepped.window.playBar();

    CHECK(bar->stepBackButton()->isEnabled() == stepped.window.stepBackwardAction()->isEnabled());
    CHECK(bar->stepBackButton()->toolTip() == stepped.window.stepBackwardAction()->toolTip());
    CHECK(bar->stepForwardButton()->toolTip().contains(QStringLiteral("Alt+Right")));
    CHECK(bar->stepBackButton()->toolTip().contains(QStringLiteral("Alt+Left")));

    // Either side of play and pause.
    CHECK(bar->stepBackButton()->x() < bar->playButton()->x());
    CHECK(bar->playButton()->x() < bar->stepForwardButton()->x());

    // Icons and no text, as play and pause are.
    CHECK_FALSE(bar->stepBackButton()->icon().isNull());
    CHECK_FALSE(bar->stepForwardButton()->icon().isNull());
    CHECK(bar->stepBackButton()->toolButtonStyle() == Qt::ToolButtonIconOnly);

    // Held, they repeat.
    CHECK(bar->stepBackButton()->autoRepeat());
    CHECK(bar->stepForwardButton()->autoRepeat());

    bar->stepForwardButton()->click();
    CHECK(stepped.booth.player->steps == std::vector<int>{1});
}

TEST_CASE("the buttons of the bar are out without a film, as the menu is", "[gui][GUI-STEP-01]") {
    InMemoryFileSystem files;
    files.addFile("/films/seul.srt", kThree);
    FakePrompts prompts;
    Projectionist booth;
    MainWindow window{files, fileIn(files, "/films/seul.srt"), prompts, projecting(booth)};
    window.show();

    CHECK_FALSE(window.playBar()->stepBackButton()->isEnabled());
    CHECK_FALSE(window.playBar()->stepForwardButton()->isEnabled());
}

TEST_CASE("a nudge moves an edge by the frame step, counted by the rate of the film",
          "[gui][GUI-NUDGE-01]") {
    const SteppedWindow stepped{FrameRate{StandardFrameRate::Fps25}};
    selectRows(stepped.window, 0, 0);

    stepped.window.nudgeStartLaterAction()->trigger();
    CHECK(start(stepped.window, 0) == "00:00:01,040");
    CHECK(end(stepped.window, 0) == "00:00:02,000");

    stepped.window.nudgeEndEarlierAction()->trigger();
    CHECK(end(stepped.window, 0) == "00:00:01,960");

    stepped.window.nudgeStartEarlierAction()->trigger();
    stepped.window.nudgeEndLaterAction()->trigger();
    CHECK(start(stepped.window, 0) == "00:00:01,000");
    CHECK(end(stepped.window, 0) == "00:00:02,000");
}

TEST_CASE("a nudge is one entry of the history, and the film shows the edge",
          "[gui][GUI-NUDGE-01]") {
    const SteppedWindow stepped{FrameRate{StandardFrameRate::Fps25}, 5};
    selectRows(stepped.window, 0, 0);

    stepped.window.nudgeStartLaterAction()->trigger();

    // The same step as the film's: five frames of 25 images a second.
    CHECK(start(stepped.window, 0) == "00:00:01,200");
    CHECK(stepped.booth.player->seeks.back() == Timestamp::fromMilliseconds(1200));

    stepped.window.undoAction()->trigger();
    CHECK(start(stepped.window, 0) == "00:00:01,000");
    CHECK_FALSE(stepped.window.undoAction()->isEnabled());
}

// What a cell says of a position that crosses its neighbour: nothing, and the table flags it.
TEST_CASE("a nudge that crosses the neighbour is flagged as the cell's would be",
          "[gui][GUI-NUDGE-01]") {
    const SteppedWindow stepped{FrameRate{StandardFrameRate::Fps25}, 13};
    selectRows(stepped.window, 0, 0);
    const QAbstractItemModel* model = stepped.window.table()->model();
    REQUIRE_FALSE(model->data(model->index(1, 1), Qt::BackgroundRole).isValid());

    // 13 frames of 40 ms: the end of the first goes from 2000 to 2520, past the start of the
    // second at 2500.
    stepped.window.nudgeEndLaterAction()->trigger();

    CHECK(end(stepped.window, 0) == "00:00:02,520");
    CHECK(model->data(model->index(1, 1), Qt::BackgroundRole).isValid());
}

TEST_CASE("a nudge with no rate to count a frame by refuses and says so", "[gui][GUI-STEP-03]") {
    const SteppedWindow stepped{std::nullopt};
    selectRows(stepped.window, 0, 0);

    stepped.window.nudgeStartLaterAction()->trigger();

    REQUIRE(stepped.prompts.failures.size() == 1U);
    CHECK(stepped.prompts.failures.front() == noFrameToCountBy());
    CHECK(start(stepped.window, 0) == "00:00:01,000");
    CHECK_FALSE(stepped.window.undoAction()->isEnabled());
}

TEST_CASE("the nudge needs a selection and no film", "[gui][GUI-NUDGE-01]") {
    InMemoryFileSystem files;
    files.addFile("/films/seul.srt", kThree);
    FakePrompts prompts;
    Projectionist booth;
    MainWindow window{files, fileIn(files, "/films/seul.srt"), prompts, projecting(booth)};
    window.show();
    window.table()->selectionModel()->clearSelection();

    CHECK_FALSE(window.nudgeStartLaterAction()->isEnabled());
    CHECK_FALSE(window.nudgeEndEarlierAction()->isEnabled());

    selectRows(window, 0, 0);
    CHECK(window.nudgeStartEarlierAction()->isEnabled());
    CHECK(window.nudgeEndLaterAction()->isEnabled());
}

// Brought to the origin and no further: the edge is where it can go, and one more nudge finds
// nothing to do.
TEST_CASE("a nudge back stops at the origin and adds nothing to the history once there",
          "[gui][GUI-NUDGE-01]") {
    const SteppedWindow stepped{FrameRate{StandardFrameRate::Fps25}, 1000};
    selectRows(stepped.window, 0, 0);

    stepped.window.nudgeStartEarlierAction()->trigger();
    CHECK(start(stepped.window, 0) == "00:00:00,000");

    stepped.window.nudgeStartEarlierAction()->trigger();
    CHECK(start(stepped.window, 0) == "00:00:00,000");

    // One entry, and not two.
    stepped.window.undoAction()->trigger();
    CHECK(start(stepped.window, 0) == "00:00:01,000");
    CHECK_FALSE(stepped.window.undoAction()->isEnabled());
}
