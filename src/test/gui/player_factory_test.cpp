// What `subedit-gui` hands its window, and the platform question behind it.
//
// **Adopting a native window is an X11 mechanism** — libmpv's own header says
// « X11, win32, and OSX only ». Two things follow, and both live here: the
// program asks for a Qt platform whose windows can be adopted when the session
// leaves that open, and it declines to build a player when they cannot be.

#include <subedit/core/io/in_memory_file_system.hpp>
#include <subedit/core/video/video_player.hpp>
#include <subedit/gui/player_factory.hpp>

#include <QByteArray>
#include <QGuiApplication>
#include <catch2/catch_test_macros.hpp>
#include <qglobal.h>

#include <string_view>

namespace {

using subedit::core::InMemoryFileSystem;
using subedit::gui::declaredFrameRates;
using subedit::gui::mpvPlayers;
using subedit::gui::platformFor;
using subedit::gui::PlayerFactory;
using subedit::gui::preferEmbeddablePlatform;

/// What the three variables say, in the order the decision reads them.
[[nodiscard]] std::string_view
asked(std::string_view chosen, std::string_view wayland, std::string_view x11) {
    return platformFor(chosen, wayland, x11);
}

/// Puts a variable back as it was, unset included.
///
/// `qputenv` with an empty value sets it empty, which is not the same thing as
/// not being there at all — and the difference is exactly what the decision
/// reads.
void restore(const char* name, const QByteArray& value) {
    if (value.isEmpty())
        qunsetenv(name);
    else
        qputenv(name, value);
}

} // namespace

// The case this exists for: a Wayland session, an X server beside it, and
// nobody having said otherwise. Left alone, Qt would give a window libmpv
// cannot adopt and the film would play nowhere.
TEST_CASE("a Wayland session with X beside it is asked to go through X", "[video][player]") {
    CHECK(asked({}, "wayland-0", ":0") == "xcb");
}

// Their business, and no picture. A program that overrode this would be
// deciding something the user has just decided.
TEST_CASE("a platform already chosen is left alone", "[video][player]") {
    CHECK(asked("wayland", "wayland-0", ":0").empty());
    CHECK(asked("xcb", "wayland-0", ":0").empty());
    CHECK(asked("offscreen", {}, {}).empty());
}

// Nothing to ask for: Qt is about to pick something this program has no opinion
// about, and on an X11 session that is `xcb` already.
TEST_CASE("a session that is not Wayland is left alone", "[video][player]") {
    CHECK(asked({}, {}, ":0").empty());
    CHECK(asked({}, {}, {}).empty());
}

// A Wayland session with no X server beside it. There is nothing better to ask
// for, and the window will say it cannot show a film — which is true, and
// better said than worked around.
TEST_CASE("a Wayland session with no X server is left alone", "[video][player]") {
    CHECK(asked({}, "wayland-0", {}).empty());
}

// The one case where anything is written, walked through the environment the
// program actually reads rather than through the decision alone.
//
// **The variables are process-wide, so they are put back before the case can
// fail.** Moving them is safe here for one reason: Qt reads the platform when
// the `QApplication` is built, which happened in `main` long before this.
TEST_CASE("a session that leaves the choice open is given one", "[video][player]") {
    const QByteArray chosen = qgetenv("QT_QPA_PLATFORM");
    const QByteArray wayland = qgetenv("WAYLAND_DISPLAY");
    const QByteArray x11 = qgetenv("DISPLAY");

    qunsetenv("QT_QPA_PLATFORM");
    qputenv("WAYLAND_DISPLAY", "wayland-0");
    qputenv("DISPLAY", ":0");

    preferEmbeddablePlatform();
    const QByteArray given = qgetenv("QT_QPA_PLATFORM");

    restore("QT_QPA_PLATFORM", chosen);
    restore("WAYLAND_DISPLAY", wayland);
    restore("DISPLAY", x11);

    CHECK(given == "xcb");
}

// These tests run under a platform the harness chose, so this is the « already
// chosen » case: it must do nothing at all, which is what makes it safe to call
// here rather than only from an entry point.
TEST_CASE("asking for a platform respects the one already set", "[video][player]") {
    const QString before = QGuiApplication::platformName();

    preferEmbeddablePlatform();

    CHECK(QGuiApplication::platformName() == before);
}

// Embedding a native window is an X11 mechanism, and these run on Qt's
// `offscreen` platform. Nothing comes back, and that is the answer: a window
// with no player shows no picture, and goes on editing subtitles.
TEST_CASE("no player is made where a window cannot be adopted", "[video][player]") {
    const PlayerFactory players = mpvPlayers();

    CHECK(players(1) == nullptr);
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
