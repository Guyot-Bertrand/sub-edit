// The player of ADR 0020, driven with no screen — which is what #178 settled
// and what every one of these cases rests on.

#include <subedit/core/time/duration.hpp>
#include <subedit/core/time/timestamp.hpp>
#include <subedit/core/video/video_player.hpp>
#include <subedit/gui/mpv_player.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstdint>
#include <expected>
#include <filesystem>
#include <optional>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include "numbered_frames.hpp"
#include "waiting.hpp"

namespace {

using subedit::core::AudioTrack;
using subedit::core::Duration;
using subedit::core::PlayerError;
using subedit::core::Timestamp;
using subedit::core::VideoPlayer;
using subedit::gui::assEventOf;
using subedit::gui::MpvPlayer;
using subedit::test::waitUntil;

[[nodiscard]] std::filesystem::path fixture(const std::string& name) {
    return std::filesystem::path{SUBEDIT_TEST_DATA_DIR} / name;
}

/// A player, or a failing case saying libmpv would not give one.
[[nodiscard]] MpvPlayer player() {
    std::expected<MpvPlayer, PlayerError> built = MpvPlayer::create();
    REQUIRE(built.has_value());
    return std::move(*built);
}

/// The number the frame on screen carries.
[[nodiscard]] int shownFrame(const MpvPlayer& watching) {
    const subedit::gui::Picture picture = watching.picture().value_or(subedit::gui::Picture{});
    REQUIRE(picture.width > 0);
    return subedit::test::frameNumberOf(picture);
}

} // namespace

TEST_CASE("a player is built where there is no screen", "[video][player]") {
    const MpvPlayer built = player();

    // Nothing is open yet, and every question says so rather than answering
    // for a film that is not there.
    CHECK_FALSE(built.duration().has_value());
    CHECK_FALSE(built.position().has_value());
    CHECK_FALSE(built.isPlaying());
}

TEST_CASE("opening a video tells how long it is", "[video][player]") {
    MpvPlayer opened = player();

    // The two fixtures of #163: one rate is whole and the other is not, and
    // the second is the one that says something.
    const auto [name, milliseconds] =
        GENERATE(std::pair{"cadence-25.mp4", 2000}, std::pair{"cadence-23-976.mp4", 2002});

    REQUIRE(opened.open(fixture(std::string{"videos/"} + name)).has_value());

    CHECK(opened.duration() == Duration::fromMilliseconds(milliseconds));
    CHECK(opened.position() == Timestamp::origin());
}

// Issue #468: a second film opened on the same player. mpv first reports the
// end of the one it replaces, and that end is not the refusal of the new one.
TEST_CASE("a film opened over another one opens", "[video][player]") {
    MpvPlayer opened = player();
    REQUIRE(opened.open(fixture("videos/cadence-25.mp4")).has_value());

    const std::expected<void, PlayerError> second =
        opened.open(fixture("videos/cadence-23-976.mp4"));

    REQUIRE(second.has_value());
    CHECK(opened.duration() == Duration::fromMilliseconds(2002));
}

// What a return to a tab does: the same film, opened again.
TEST_CASE("the same film opened twice opens twice", "[video][player]") {
    MpvPlayer opened = player();
    REQUIRE(opened.open(fixture("videos/cadence-25.mp4")).has_value());

    CHECK(opened.open(fixture("videos/cadence-25.mp4")).has_value());
    CHECK(opened.duration() == Duration::fromMilliseconds(2000));
}

// GUI-PLAYER-03 rests on this: the window says a video would not open, names
// the file, and stays usable. What it says comes from here.
TEST_CASE("a file that is not a video is refused, and says why", "[video][player]") {
    MpvPlayer refused = player();

    const std::expected<void, PlayerError> opened = refused.open(fixture("valides/minimal.srt"));

    REQUIRE_FALSE(opened.has_value());
    CHECK_FALSE(opened.error().reason.empty());
    // Nothing was opened, so nothing is answered for.
    CHECK_FALSE(refused.duration().has_value());
}

// **Measured, and it is a trap**: handed a directory, mpv ends the file with
// « success » — nothing failed, and nothing played either. Reporting that word
// as the reason a film would not open is how a message stops meaning anything.
TEST_CASE("a directory is refused without being called a success", "[video][player]") {
    MpvPlayer refused = player();

    const std::expected<void, PlayerError> opened = refused.open(fixture("videos"));

    REQUIRE_FALSE(opened.has_value());
    CHECK(opened.error().reason != "success");
    CHECK_FALSE(opened.error().reason.empty());
}

TEST_CASE("a file that is not there is refused", "[video][player]") {
    MpvPlayer refused = player();

    const std::expected<void, PlayerError> opened = refused.open(fixture("videos/absent.mp4"));

    REQUIRE_FALSE(opened.has_value());
    CHECK_FALSE(opened.error().reason.empty());
}

// The claim ADR 0020 chose libmpv on, and the one thing of phase 14 written
// early. What this case holds is that playback lands where it was sent, which
// is what phase 6 needs of it.
//
// **What it does not hold is the word `exact` itself**, and saying so is worth
// a line: mpv lands exactly here without it, its `hr-seek` defaulting to
// precise seeks for absolute positions. Asking anyway is what keeps phase 14
// resting on something the player is told rather than on a default.
//
// **Nor does it hold which picture is on screen.** `position()` is what mpv says of
// itself, so a player that showed the wrong frame while announcing 1000 ms would
// pass — and the fixture is two seconds, a keyframe every ten frames. The cases below
// the numbered videos (#610) read the picture itself, from a keyframe 249 frames away.
TEST_CASE("seeking lands exactly where it was asked", "[video][player]") {
    MpvPlayer seeking = player();
    REQUIRE(seeking.open(fixture("videos/cadence-25.mp4")).has_value());

    seeking.seek(Timestamp::fromMilliseconds(1000));

    CHECK(seeking.position() == Timestamp::fromMilliseconds(1000));
}

// Selecting a line before choosing a film is an ordinary thing to do. It does
// nothing, and it says nothing about it.
TEST_CASE("driving a player with nothing open does nothing", "[video][player]") {
    MpvPlayer idle = player();

    idle.seek(Timestamp::fromMilliseconds(1000));
    idle.play();

    CHECK_FALSE(idle.isPlaying());
    CHECK_FALSE(idle.position().has_value());
}

TEST_CASE("a player says whether it is playing", "[video][player]") {
    MpvPlayer playing = player();
    REQUIRE(playing.open(fixture("videos/cadence-25.mp4")).has_value());

    // Opening is not watching: a film arrives held where it starts.
    CHECK_FALSE(playing.isPlaying());

    playing.play();
    CHECK(playing.isPlaying());

    playing.pause();
    CHECK_FALSE(playing.isPlaying());
}

// The handle is a resource, and this is the case where a hand-written one
// would give it back twice. Nothing is asserted here that a `CHECK` could
// carry: what fails, if anything does, is AddressSanitizer, under which the
// gate runs every one of these.
TEST_CASE("a player that is moved gives its handle back once", "[video][player]") {
    MpvPlayer moved = player();
    REQUIRE(moved.open(fixture("videos/cadence-25.mp4")).has_value());

    const MpvPlayer taken = std::move(moved);

    CHECK(taken.duration() == Duration::fromMilliseconds(2000));
}

// The seam is what the window holds, and what #176 doubles. Driving the real
// one through it is what says the two agree on their shape.
TEST_CASE("a player answers through the seam", "[video][player]") {
    MpvPlayer built = player();
    VideoPlayer& seam = built;

    REQUIRE(seam.open(fixture("videos/cadence-23-976.mp4")).has_value());
    seam.seek(Timestamp::fromMilliseconds(500));

    CHECK(seam.duration() == Duration::fromMilliseconds(2002));
    CHECK(seam.position() == Timestamp::fromMilliseconds(500));
}

// Nothing here can look at a picture, so what these two hold is that the order
// is composed and handed over without upsetting the player. What this file can
// get wrong on its own — the text — is tested further down, in the open.
TEST_CASE("a replica is handed to a player watching a film", "[video][player]") {
    MpvPlayer drawing = player();
    REQUIRE(drawing.open(fixture("videos/cadence-25.mp4")).has_value());

    drawing.showSubtitle("Un.\nDeux.");
    drawing.showSubtitle({});

    CHECK(drawing.duration() == Duration::fromMilliseconds(2000));
}

TEST_CASE("drawing a replica with nothing open does nothing", "[video][player]") {
    MpvPlayer idle = player();

    idle.showSubtitle("Un.");

    CHECK_FALSE(idle.position().has_value());
}

// What the replica becomes on its way to libmpv's overlay — issue #176.
//
// **Out in the open because nothing here can be read back.** An overlay is
// written to a picture, and there is no picture where these tests run; the
// order handed to libmpv is answered « success » whatever it says. What this
// file can get wrong on its own is the text it composes, so that is what is
// tested, as an ordinary function with ordinary cases.

TEST_CASE("a replica is drawn at the foot of the picture", "[video][player]") {
    CHECK(assEventOf("Un.") == "{\\an2}Un.");
}

// Empty in, empty out: it is how the window says « draw nothing », and the
// player turns it into the overlay format that means the same.
TEST_CASE("an empty replica composes to nothing", "[video][player]") {
    CHECK(assEventOf("").empty());
}

TEST_CASE("a break in a replica becomes the hard break of ASS", "[video][player]") {
    CHECK(assEventOf("Un.\nDeux.") == "{\\an2}Un.\\NDeux.");
}

// A text read from a file with Windows endings would otherwise carry a stray
// character before every break.
TEST_CASE("a carriage return is not drawn", "[video][player]") {
    CHECK(assEventOf("Un.\r\nDeux.") == "{\\an2}Un.\\NDeux.");
}

// **The braces are no longer this function's business** — issue #408. The replica it is handed is
// already in the Sub Station Alpha vocabulary: `core::replicaOf` writes the styles as override
// blocks and escapes the braces of the visible text, so a block here is a style libass applies.
TEST_CASE("an override block is handed to libass as it stands", "[video][player]") {
    CHECK(assEventOf("{\\i1}Un.{\\i0}") == "{\\an2}{\\i1}Un.{\\i0}");
    CHECK(assEventOf("\\{rires\\}") == "{\\an2}\\{rires\\}");
}

// What the player makes of a tag of the file is nothing: understanding one is the pivot's, and
// it happens before the text gets here. An HTML tag that reached this far would be drawn.
TEST_CASE("a tag that was not translated is drawn as it stands", "[video][player]") {
    CHECK(assEventOf("<i>Un.</i>") == "{\\an2}<i>Un.</i>");
}

// Italic is drawn: the picture of a replica with a style is not the picture of the same
// replica without it — read from the render, which is where the overlay lands.
TEST_CASE("a style of the replica changes the picture", "[video][player][render]") {
    MpvPlayer drawing = player();
    REQUIRE(drawing.open(fixture("videos/images-25.mp4")).has_value());
    drawing.seek(Timestamp::fromMilliseconds(subedit::test::startOf(100, 25, 1)));

    drawing.showSubtitle("Un mot");
    const subedit::gui::Picture plain = subedit::test::renderedAt(drawing, 640, 320);
    drawing.showSubtitle("{\\i1}Un mot{\\i0}");
    const subedit::gui::Picture italic = subedit::test::renderedAt(drawing, 640, 320);

    REQUIRE(plain.pixels.size() == italic.pixels.size());
    CHECK(plain.pixels != italic.pixels);
}

// ## The picture is the oracle — issue #610

// The fixtures carry their frame numbers in the picture, so that these cases do not
// rest on what the player says of itself. One keyframe, at the start: the last frame
// is a decode of the whole film.
TEST_CASE("a numbered video shows the number of the frame the player stands on",
          "[video][player][numbered]") {
    MpvPlayer reading = player();
    REQUIRE(reading.open(fixture("videos/images-25.mp4")).has_value());

    const subedit::gui::Picture picture = reading.picture().value_or(subedit::gui::Picture{});

    REQUIRE(picture.width > 0);
    CHECK(picture.width == 128);
    CHECK(picture.height == 64);
    CHECK(subedit::test::frameNumberOf(picture) == 0);
}

TEST_CASE("seeking lands on the frame asked, however far the keyframe is",
          "[video][player][numbered]") {
    MpvPlayer seeking = player();
    REQUIRE(seeking.open(fixture("videos/images-25.mp4")).has_value());

    // From the first frame to the last, 249 frames from the only keyframe.
    for (const int frame : {0, 1, 37, 100, 199, 200, 249}) {
        INFO("frame " << frame);
        seeking.seek(Timestamp::fromMilliseconds(subedit::test::startOf(frame, 25, 1)));

        const subedit::gui::Picture picture = seeking.picture().value_or(subedit::gui::Picture{});
        REQUIRE(picture.width > 0);
        CHECK(subedit::test::frameNumberOf(picture) == frame);
        CHECK(seeking.position() ==
              Timestamp::fromMilliseconds(subedit::test::startOf(frame, 25, 1)));
    }
}

TEST_CASE("at 23.976 images a second, every frame lands on itself at the millisecond it starts",
          "[video][player][numbered]") {
    // The frame rate is 24000/1001 and a frame starts at a fraction of a millisecond:
    // frame 10 at 417.08 ms. Whole milliseconds are not the defect — asked at the
    // millisecond below, the nearest, or the one above, every frame shows itself.
    MpvPlayer seeking = player();
    REQUIRE(seeking.open(fixture("videos/images-23-976.mp4")).has_value());

    for (const int frame : {1,  2,  3,  5,  7,  11, 13, 17, 19,  23,  24,  29,
                            31, 37, 41, 47, 53, 59, 61, 71, 100, 143, 200, 233}) {
        const std::int64_t near = subedit::test::startOf(frame, 24000, 1001);
        for (const std::int64_t asked : {near - 1, near, near + 1}) {
            INFO("frame " << frame << " asked at " << asked << " ms");
            seeking.seek(Timestamp::fromMilliseconds(asked));

            const subedit::gui::Picture picture =
                seeking.picture().value_or(subedit::gui::Picture{});
            REQUIRE(picture.width > 0);
            CHECK(subedit::test::frameNumberOf(picture) == frame);
        }
    }
}

// What « the frame at a position » means, written as it was measured and not as the
// comment of `seek` once said: **the nearest frame**, which is not always the one on
// screen at that instant, and `position()` then says where that frame starts.
TEST_CASE("a position inside a frame lands on the nearest frame, and says where it starts",
          "[video][player][numbered]") {
    MpvPlayer seeking = player();
    REQUIRE(seeking.open(fixture("videos/images-23-976.mp4")).has_value());

    // Frame 10 spans 417.08 to 458.79 ms. The first half of that is frame 10, the second
    // half is frame 11 — which starts at 458.79 ms, said as 459.
    struct Ask {
        std::int64_t milliseconds;
        int frame;
        std::int64_t startsAt;
    };

    for (const Ask& ask : {Ask{.milliseconds = 417, .frame = 10, .startsAt = 417},
                           Ask{.milliseconds = 421, .frame = 10, .startsAt = 417},
                           Ask{.milliseconds = 437, .frame = 11, .startsAt = 459},
                           Ask{.milliseconds = 454, .frame = 11, .startsAt = 459},
                           Ask{.milliseconds = 458, .frame = 11, .startsAt = 459},
                           // Frame 50 spans 2085.42 to 2127.12 ms.
                           Ask{.milliseconds = 2089, .frame = 50, .startsAt = 2085},
                           Ask{.milliseconds = 2106, .frame = 51, .startsAt = 2127},
                           Ask{.milliseconds = 2126, .frame = 51, .startsAt = 2127}}) {
        INFO("asked at " << ask.milliseconds << " ms");
        seeking.seek(Timestamp::fromMilliseconds(ask.milliseconds));

        const subedit::gui::Picture picture = seeking.picture().value_or(subedit::gui::Picture{});
        REQUIRE(picture.width > 0);
        CHECK(subedit::test::frameNumberOf(picture) == ask.frame);
        CHECK(seeking.position() == Timestamp::fromMilliseconds(ask.startsAt));
    }
}

TEST_CASE("a player with nothing open has no picture", "[video][player][numbered]") {
    const MpvPlayer idle = player();

    CHECK_FALSE(idle.picture().has_value());
}

// ## Stepping, the volume, the tracks and playing up to a position — issue #614

TEST_CASE("a step forward shows the next frame", "[video][player][numbered][step]") {
    MpvPlayer stepping = player();
    REQUIRE(stepping.open(fixture("videos/images-25.mp4")).has_value());

    stepping.stepFrames(1);

    CHECK(shownFrame(stepping) == 1);
    CHECK(stepping.position() == Timestamp::fromMilliseconds(40));
}

TEST_CASE("a step back shows the previous frame", "[video][player][numbered][step]") {
    MpvPlayer stepping = player();
    REQUIRE(stepping.open(fixture("videos/images-25.mp4")).has_value());
    stepping.seek(Timestamp::fromMilliseconds(subedit::test::startOf(100, 25, 1)));

    stepping.stepFrames(-1);

    CHECK(shownFrame(stepping) == 99);
    CHECK(stepping.position() == Timestamp::fromMilliseconds(subedit::test::startOf(99, 25, 1)));
}

TEST_CASE("a step by several frames lands that many frames away",
          "[video][player][numbered][step]") {
    MpvPlayer stepping = player();
    REQUIRE(stepping.open(fixture("videos/images-25.mp4")).has_value());
    stepping.seek(Timestamp::fromMilliseconds(subedit::test::startOf(50, 25, 1)));

    stepping.stepFrames(7);
    CHECK(shownFrame(stepping) == 57);

    stepping.stepFrames(-30);
    CHECK(shownFrame(stepping) == 27);

    stepping.stepFrames(0);
    CHECK(shownFrame(stepping) == 27);
}

TEST_CASE("stepping back from the first frame stays on it", "[video][player][numbered][step]") {
    MpvPlayer stepping = player();
    REQUIRE(stepping.open(fixture("videos/images-25.mp4")).has_value());

    stepping.stepFrames(-1);
    CHECK(shownFrame(stepping) == 0);
    CHECK(stepping.position() == Timestamp::origin());

    stepping.stepFrames(-100);
    CHECK(shownFrame(stepping) == 0);
}

TEST_CASE("stepping forward from the last frame stays on it", "[video][player][numbered][step]") {
    MpvPlayer stepping = player();
    REQUIRE(stepping.open(fixture("videos/images-25.mp4")).has_value());
    stepping.seek(Timestamp::fromMilliseconds(subedit::test::startOf(249, 25, 1)));

    stepping.stepFrames(1);
    CHECK(shownFrame(stepping) == 249);

    // And a step that would overshoot by far ends on the last frame, not past it.
    stepping.seek(Timestamp::origin());
    stepping.stepFrames(1000);
    CHECK(shownFrame(stepping) == 249);
    CHECK(stepping.duration() == Duration::fromMilliseconds(10000));

    // The film is still there: it can be stepped back from the end.
    stepping.stepFrames(-1);
    CHECK(shownFrame(stepping) == 248);
}

// What a mark rests on (D4): a step lands on whole frames at 23.976 as well, where a
// frame starts at a fraction of a millisecond and a step that added a rounded 42 ms
// would drift.
TEST_CASE("steps at 23.976 images a second keep to whole frames, forward and back",
          "[video][player][numbered][step]") {
    MpvPlayer stepping = player();
    REQUIRE(stepping.open(fixture("videos/images-23-976.mp4")).has_value());
    stepping.seek(Timestamp::fromMilliseconds(subedit::test::startOf(10, 24000, 1001)));

    stepping.stepFrames(1);
    CHECK(shownFrame(stepping) == 11);
    CHECK(stepping.position() == Timestamp::fromMilliseconds(459));

    stepping.stepFrames(-1);
    CHECK(shownFrame(stepping) == 10);
    CHECK(stepping.position() == Timestamp::fromMilliseconds(417));

    // Forty frames on, and forty back: no drift.
    for (int round = 0; round < 3; ++round) {
        stepping.stepFrames(40);
        stepping.stepFrames(-40);
    }
    CHECK(shownFrame(stepping) == 10);

    // The last frame of the second fixture is 239.
    stepping.stepFrames(10000);
    CHECK(shownFrame(stepping) == 239);
}

TEST_CASE("a step holds playback", "[video][player][step]") {
    MpvPlayer stepping = player();
    REQUIRE(stepping.open(fixture("videos/images-25.mp4")).has_value());
    stepping.play();
    REQUIRE(stepping.isPlaying());

    stepping.stepFrames(1);

    CHECK_FALSE(stepping.isPlaying());
}

// No picture, so no frame rate and nothing to step by: the order is a no-op, not an error.
TEST_CASE("a film with no picture has no frame to step to", "[video][player][step]") {
    MpvPlayer listening = player();
    REQUIRE(listening.open(fixture("videos/sound-only.mkv")).has_value());
    listening.seek(Timestamp::fromMilliseconds(1000));

    listening.stepFrames(1);
    listening.stepFrames(-1);

    CHECK(listening.position() == Timestamp::fromMilliseconds(1000));
}

TEST_CASE("a step with nothing open does nothing", "[video][player][step]") {
    MpvPlayer idle = player();

    idle.stepFrames(1);
    idle.stepFrames(-1);

    CHECK_FALSE(idle.position().has_value());
}

TEST_CASE("the volume is set, read back, and held between 0 and 100", "[video][player][volume]") {
    MpvPlayer listening = player();

    listening.setVolume(40);
    CHECK(listening.volume() == 40);

    listening.setVolume(0);
    CHECK(listening.volume() == 0);

    listening.setVolume(150);
    CHECK(listening.volume() == 100);

    listening.setVolume(-20);
    CHECK(listening.volume() == 0);
}

TEST_CASE("the volume survives opening a film", "[video][player][volume]") {
    MpvPlayer listening = player();
    listening.setVolume(35);

    REQUIRE(listening.open(fixture("videos/audio-1.mkv")).has_value());

    CHECK(listening.volume() == 35);
}

TEST_CASE("a video with two audio tracks lists both, the first playing", "[video][player][audio]") {
    MpvPlayer listening = player();
    REQUIRE(listening.open(fixture("videos/audio-2.mkv")).has_value());

    const std::vector<AudioTrack> tracks = listening.audioTracks();

    REQUIRE(tracks.size() == 2);
    CHECK(tracks[0].language == "fra");
    CHECK(tracks[0].title == "Original");
    CHECK(tracks[0].selected);
    CHECK(tracks[1].language == "eng");
    CHECK(tracks[1].title == "Commentary");
    CHECK_FALSE(tracks[1].selected);
    CHECK(tracks[0].id != tracks[1].id);
}

TEST_CASE("selecting an audio track moves the mark to it", "[video][player][audio]") {
    MpvPlayer listening = player();
    REQUIRE(listening.open(fixture("videos/audio-2.mkv")).has_value());
    const int second = listening.audioTracks().at(1).id;

    listening.selectAudioTrack(second);

    const std::vector<AudioTrack> tracks = listening.audioTracks();
    REQUIRE(tracks.size() == 2);
    CHECK_FALSE(tracks[0].selected);
    CHECK(tracks[1].selected);
}

TEST_CASE("selecting a track the video does not have changes nothing", "[video][player][audio]") {
    MpvPlayer listening = player();
    REQUIRE(listening.open(fixture("videos/audio-2.mkv")).has_value());
    const std::vector<AudioTrack> before = listening.audioTracks();

    listening.selectAudioTrack(99);

    CHECK(listening.audioTracks() == before);
}

TEST_CASE("a video with one audio track lists it", "[video][player][audio]") {
    MpvPlayer listening = player();
    REQUIRE(listening.open(fixture("videos/audio-1.mkv")).has_value());

    const std::vector<AudioTrack> tracks = listening.audioTracks();

    REQUIRE(tracks.size() == 1);
    CHECK(tracks[0].language == "fra");
    CHECK(tracks[0].selected);
}

TEST_CASE("a video without sound has no audio track, and nothing breaks",
          "[video][player][audio]") {
    MpvPlayer silent = player();
    REQUIRE(silent.open(fixture("videos/cadence-25.mp4")).has_value());

    CHECK(silent.audioTracks().empty());

    silent.selectAudioTrack(1);
    silent.setVolume(50);
    silent.seek(Timestamp::fromMilliseconds(480));

    CHECK(silent.audioTracks().empty());
    CHECK(silent.position() == Timestamp::fromMilliseconds(480));
}

TEST_CASE("with nothing open there are no audio tracks", "[video][player][audio]") {
    MpvPlayer idle = player();

    CHECK(idle.audioTracks().empty());
    idle.selectAudioTrack(1);
    CHECK(idle.audioTracks().empty());
}

TEST_CASE("playing up to a position holds playback on the last frame before it",
          "[video][player][numbered][until]") {
    MpvPlayer playing = player();
    REQUIRE(playing.open(fixture("videos/images-25.mp4")).has_value());
    playing.seek(Timestamp::fromMilliseconds(1000));

    playing.playUntil(Timestamp::fromMilliseconds(1500));
    CHECK(playing.isPlaying());
    REQUIRE(waitUntil([&playing] { return !playing.isPlaying(); }));

    // Frame 37 starts at 1480 ms and is the last that starts before 1500.
    CHECK(playing.position() == Timestamp::fromMilliseconds(1480));
    CHECK(shownFrame(playing) == 37);
}

TEST_CASE("the stop of playing up to a position is not kept for the next play",
          "[video][player][until]") {
    MpvPlayer playing = player();
    REQUIRE(playing.open(fixture("videos/images-25.mp4")).has_value());
    playing.seek(Timestamp::fromMilliseconds(1000));
    playing.playUntil(Timestamp::fromMilliseconds(1200));
    REQUIRE(waitUntil([&playing] { return !playing.isPlaying(); }));
    REQUIRE(playing.position() == Timestamp::fromMilliseconds(1160));

    // Played again, it goes past the stop it had — which is the thing to wait for, not a pause.
    playing.play();
    CHECK(waitUntil([&playing] {
        return playing.position().value_or(Timestamp::origin()) > Timestamp::fromMilliseconds(1200);
    }));
    CHECK(playing.isPlaying());
}

TEST_CASE("playing up to a position already reached plays nothing", "[video][player][until]") {
    MpvPlayer playing = player();
    REQUIRE(playing.open(fixture("videos/images-25.mp4")).has_value());
    playing.seek(Timestamp::fromMilliseconds(1000));

    playing.playUntil(Timestamp::fromMilliseconds(1000));
    CHECK_FALSE(playing.isPlaying());

    playing.playUntil(Timestamp::fromMilliseconds(500));
    CHECK_FALSE(playing.isPlaying());
    CHECK(playing.position() == Timestamp::fromMilliseconds(1000));
}

TEST_CASE("playing up to a position past the end stops on the last frame",
          "[video][player][numbered][until]") {
    MpvPlayer playing = player();
    REQUIRE(playing.open(fixture("videos/images-25.mp4")).has_value());
    playing.seek(Timestamp::fromMilliseconds(subedit::test::startOf(245, 25, 1)));

    playing.playUntil(Timestamp::fromMilliseconds(60000));

    REQUIRE(waitUntil([&playing] { return !playing.isPlaying(); }));
    CHECK(shownFrame(playing) == 249);
    // The film is still open once it has ended.
    CHECK(playing.duration() == Duration::fromMilliseconds(10000));
}

TEST_CASE("playing up to a position with nothing open does nothing", "[video][player][until]") {
    MpvPlayer idle = player();

    idle.playUntil(Timestamp::fromMilliseconds(1000));

    CHECK_FALSE(idle.isPlaying());
}

// ## The picture drawn through the render API — ADR 0041, issue #613

// What a widget paints is what `render` draws, and it is read here as the widget would
// get it: pixels in a buffer, no window and no screen anywhere.
TEST_CASE("the picture rendered into a buffer is the frame the player stands on",
          "[video][player][numbered][render][GUI-SURFACE-02]") {
    MpvPlayer drawing = player();
    REQUIRE(drawing.open(fixture("videos/images-25.mp4")).has_value());

    for (const int frame : {0, 1, 37, 100, 249}) {
        INFO("frame " << frame);
        drawing.seek(Timestamp::fromMilliseconds(subedit::test::startOf(frame, 25, 1)));

        const subedit::gui::Picture picture = subedit::test::renderedAt(drawing, 128, 64);

        REQUIRE(picture.width == 128);
        CHECK(subedit::test::frameNumberOf(picture) == frame);
    }
}

// The size is the widget's, not the film's: the picture is scaled to the buffer, and the
// number is still there to read at any of them.
TEST_CASE("the picture is drawn at the size of the buffer", "[video][player][numbered][render]") {
    MpvPlayer drawing = player();
    REQUIRE(drawing.open(fixture("videos/images-25.mp4")).has_value());
    drawing.seek(Timestamp::fromMilliseconds(subedit::test::startOf(100, 25, 1)));

    for (const auto& [width, height] :
         {std::pair{256, 128}, std::pair{640, 360}, std::pair{64, 32}}) {
        INFO(width << "x" << height);
        const subedit::gui::Picture picture = subedit::test::renderedAt(drawing, width, height);

        REQUIRE(picture.width == width);
        CHECK(subedit::test::frameNumberOf(picture) == 100);
    }
}

// GUI-SURFACE-01: the aspect ratio of the film is kept, the rest is black. A 2:1 film in
// a square buffer is drawn across the middle, with a band above and below.
TEST_CASE("the aspect ratio of the film is kept, and the rest is black",
          "[video][player][numbered][render][GUI-SURFACE-01]") {
    MpvPlayer drawing = player();
    REQUIRE(drawing.open(fixture("videos/images-25.mp4")).has_value());
    // Frame 255 would be all bars light; frame 1 has a single light bar at the left.
    drawing.seek(Timestamp::fromMilliseconds(subedit::test::startOf(1, 25, 1)));

    const subedit::gui::Picture picture = subedit::test::renderedAt(drawing, 128, 128);

    REQUIRE(picture.width == 128);
    // The bars are drawn across the middle half of the height, and read at the middle.
    CHECK(subedit::test::frameNumberOf(picture) == 1);
    // Above and below the film: black, whatever the bar.
    constexpr int kBlack = 40;
    for (const int x : {4, 20, 60, 100, 124}) {
        CHECK(picture.greenAt(x, 4) < kBlack);
        CHECK(picture.greenAt(x, 123) < kBlack);
    }
}

TEST_CASE("nothing is drawn with no film open, or into a buffer that is too small",
          "[video][player][render]") {
    MpvPlayer idle = player();
    CHECK(subedit::test::renderedAt(idle, 128, 64).pixels.empty());

    MpvPlayer drawing = player();
    REQUIRE(drawing.open(fixture("videos/images-25.mp4")).has_value());
    std::vector<unsigned char> small(10);
    CHECK_FALSE(drawing.render(small, 128, 64, 512U));
    CHECK_FALSE(drawing.render(small, 0, 64, 0));
    // A stride shorter than a row is no buffer either (128 x 64 pixels, four bytes each).
    std::vector<unsigned char> whole(32768U);
    CHECK_FALSE(drawing.render(whole, 128, 64, 100));
}

// The replica is drawn by libmpv's overlay, and it is part of what `render` draws — which
// is what puts the subtitle on the picture the window shows.
TEST_CASE("the replica is drawn on the picture", "[video][player][numbered][render]") {
    MpvPlayer drawing = player();
    REQUIRE(drawing.open(fixture("videos/images-25.mp4")).has_value());
    drawing.seek(Timestamp::fromMilliseconds(subedit::test::startOf(100, 25, 1)));
    const subedit::gui::Picture bare = subedit::test::renderedAt(drawing, 640, 320);

    drawing.showSubtitle("A line of dialogue.");
    const subedit::gui::Picture drawn = subedit::test::renderedAt(drawing, 640, 320);

    REQUIRE(bare.pixels.size() == drawn.pixels.size());
    CHECK(bare.pixels != drawn.pixels);
    // And cleared, the picture is the bare one again.
    drawing.showSubtitle({});
    CHECK(subedit::test::renderedAt(drawing, 640, 320).pixels == bare.pixels);
}

// The announcement comes from a thread of the player, and is what makes a widget draw.
TEST_CASE("a new picture is announced, and the announcement can be withdrawn",
          "[video][player][render]") {
    MpvPlayer announcing = player();
    REQUIRE(announcing.open(fixture("videos/images-25.mp4")).has_value());

    std::atomic<int> announced = 0;
    announcing.onFrameReady([&announced] { ++announced; });
    announcing.seek(Timestamp::fromMilliseconds(1000));

    CHECK(waitUntil([&announced] { return announced > 0; }));

    announcing.onFrameReady({});
    const int before = announced;
    announcing.seek(Timestamp::fromMilliseconds(2000));

    // Withdrawn, the first callback is not called again. That is an absence, so it is read after
    // something that is not: a second callback, put in its place, hears the next seek — and by
    // then the thread that announces has gone through everything the first seek queued.
    std::atomic<int> heard = 0;
    announcing.onFrameReady([&heard] { ++heard; });
    announcing.seek(Timestamp::fromMilliseconds(3000));
    CHECK(waitUntil([&heard] { return heard > 0; }));
    CHECK(announced == before);
}

// Found by the benchmark of #611, which a seek at 400 ms gave away: with `vo=libmpv` the
// output waits for each picture to be rendered — up to 200 ms — and a caller that waits
// holds the thread the window paints on. The player takes the frames itself while it waits.
// Ten seeks on a small film take milliseconds; stalled, they took four seconds.
TEST_CASE("a seek does not wait for a window to paint", "[video][player][render]") {
    MpvPlayer seeking = player();
    REQUIRE(seeking.open(fixture("videos/images-25.mp4")).has_value());

    // Each seek is timed, and the **median** is what is held to account. A player that stalls waits
    // for each picture — 200 ms a seek — so all ten are slow and the median with them; a machine
    // that is merely loaded, or a sanitizer, slows a few and leaves the median where it was.
    constexpr int kSeeks = 10;
    constexpr std::chrono::milliseconds kStall{200};
    std::vector<std::chrono::steady_clock::duration> took;
    for (int frame = 0; frame < kSeeks; ++frame) {
        const auto begin = std::chrono::steady_clock::now();
        seeking.seek(Timestamp::fromMilliseconds(subedit::test::startOf(frame * 20, 25, 1)));
        took.push_back(std::chrono::steady_clock::now() - begin);
    }
    std::ranges::sort(took);
    const auto median = took[took.size() / 2];

    CHECK(median < kStall / 2);
    CHECK(seeking.position() == Timestamp::fromMilliseconds(subedit::test::startOf(180, 25, 1)));
}
