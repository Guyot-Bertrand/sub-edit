// The player of ADR 0020, driven with no screen — which is what #178 settled
// and what every one of these cases rests on.

#include <subedit/core/time/duration.hpp>
#include <subedit/core/time/timestamp.hpp>
#include <subedit/core/video/video_player.hpp>
#include <subedit/gui/mpv_player.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <expected>
#include <filesystem>
#include <optional>
#include <string>
#include <utility>

namespace {

using subedit::core::Duration;
using subedit::core::PlayerError;
using subedit::core::Timestamp;
using subedit::core::VideoPlayer;
using subedit::gui::assEventOf;
using subedit::gui::MpvPlayer;

[[nodiscard]] std::filesystem::path fixture(const std::string& name) {
    return std::filesystem::path{SUBEDIT_TEST_DATA_DIR} / name;
}

/// A player, or a failing case saying libmpv would not give one.
[[nodiscard]] MpvPlayer player() {
    std::expected<MpvPlayer, PlayerError> built = MpvPlayer::create();
    REQUIRE(built.has_value());
    return std::move(*built);
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

// A brace opens an override block in ASS. Unescaped, a subtitle saying
// « {rires} » would disappear into a tag libass does not recognise.
TEST_CASE("braces in a replica are escaped", "[video][player]") {
    CHECK(assEventOf("{rires}") == "{\\an2}\\{rires\\}");
}

// ADR 0009: the model holds the text as the file wrote it, tags included, and
// this draws what the model holds. Gaupol strips them; understanding a tag
// well enough to remove it is what phase 9 is for.
TEST_CASE("a tag of the format is drawn as it stands", "[video][player]") {
    CHECK(assEventOf("<i>Un.</i>") == "{\\an2}<i>Un.</i>");
}
