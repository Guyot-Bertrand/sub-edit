// What `subedit-gui` hands its window — ADR 0041.
//
// **The picture is drawn by the window**, through libmpv's software render API, so the
// program asks nothing of the Qt platform and a player is made on any of them. What was
// here before — choosing `xcb` on a Wayland session, declining a player anywhere else —
// belonged to adopting a native window, and went with it.

#include <subedit/core/io/in_memory_file_system.hpp>
#include <subedit/core/video/video_player.hpp>
#include <subedit/gui/player_factory.hpp>

#include <catch2/catch_test_macros.hpp>

#include <memory>

namespace {

using subedit::core::InMemoryFileSystem;
using subedit::core::VideoPlayer;
using subedit::gui::declaredFrameRates;
using subedit::gui::mpvPlayers;
using subedit::gui::PlayerFactory;

} // namespace

// GUI-SURFACE-03: these tests run on Qt's `offscreen` platform, which has no native
// window to adopt — and a player is made all the same.
TEST_CASE("a player is made on a platform that has no window to adopt",
          "[video][player][GUI-SURFACE-03]") {
    const PlayerFactory players = mpvPlayers();

    const std::unique_ptr<VideoPlayer> made = players();

    CHECK(made != nullptr);
}

// The other seam of the video: what a film says of itself, read by `ffprobe`.
//
// **Asked of a file system that holds no `ffprobe`**, which is how a machine
// with no `ffmpeg` answers — and it is an ordinary machine, not a degraded one.
// Nothing comes back, nothing is proposed, and no operation behaves differently
// for it. What `ffprobe` says when it is there is proved in the core, on the
// fixtures of #163.
TEST_CASE("a film declares nothing when there is no ffprobe to ask", "[video][framerate]") {
    const InMemoryFileSystem files;
    const auto declares = declaredFrameRates(files);

    CHECK_FALSE(declares("/films/film.mkv").has_value());
}
