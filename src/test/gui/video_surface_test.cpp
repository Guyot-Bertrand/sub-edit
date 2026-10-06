// The widget the picture is painted on — ADR 0041, issue #613.
//
// **Driven by a real player, on Qt's `offscreen` platform**, which is what the decision
// made possible: the picture is in the widget, so a test reads what the widget shows
// and compares it to the numbered frames of #610. Before, the picture lived in a native
// window Qt could not see, and no test could say which frame was on screen.

#include <subedit/core/time/timestamp.hpp>
#include <subedit/gui/mpv_player.hpp>
#include <subedit/gui/video_surface.hpp>

#include <QCoreApplication>
#include <QImage>
#include <QPixmap>
#include <QSize>
#include <catch2/catch_test_macros.hpp>

#include <chrono>
#include <cstddef>
#include <expected>
#include <filesystem>
#include <functional>
#include <string>
#include <utility>

#include "numbered_frames.hpp"

namespace {

using subedit::core::PlayerError;
using subedit::core::Timestamp;
using subedit::gui::MpvPlayer;
using subedit::gui::Picture;
using subedit::gui::VideoSurface;

[[nodiscard]] std::filesystem::path fixture(const std::string& name) {
    return std::filesystem::path{SUBEDIT_TEST_DATA_DIR} / name;
}

[[nodiscard]] MpvPlayer player() {
    std::expected<MpvPlayer, PlayerError> built = MpvPlayer::create();
    REQUIRE(built.has_value());
    return std::move(*built);
}

/// The pixels of `image` as the reader of numbered frames wants them.
[[nodiscard]] Picture pictureOf(const QImage& image) {
    const QImage converted = image.convertToFormat(QImage::Format_RGB32);
    Picture picture{.width = converted.width(), .height = converted.height()};
    const std::size_t row = static_cast<std::size_t>(converted.width()) * Picture::kBytesAPixel;
    for (int y = 0; y < converted.height(); ++y) {
        const unsigned char* line = converted.constScanLine(y);
        picture.pixels.insert(picture.pixels.end(), line, line + row);
    }
    return picture;
}

/// Lets the event loop run until `done`, for a few seconds at most. The announcement of a
/// new picture is queued from another thread, so what a case waits for is events.
[[nodiscard]] bool pumpUntil(const std::function<bool()>& done) {
    const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds{5};
    while (!done() && std::chrono::steady_clock::now() < deadline)
        QCoreApplication::processEvents(QEventLoop::AllEvents, 20);
    return done();
}

[[nodiscard]] int numberShown(const VideoSurface& surface) {
    return subedit::test::frameNumberOf(pictureOf(surface.image()));
}

} // namespace

// GUI-SURFACE-01: the picture is in the widget — what Qt grabs from it is the film.
TEST_CASE("the picture is painted in the widget, and Qt can grab it",
          "[gui][video][numbered][GUI-SURFACE-01]") {
    MpvPlayer playing = player();
    REQUIRE(playing.open(fixture("videos/images-25.mp4")).has_value());
    playing.seek(Timestamp::fromMilliseconds(subedit::test::startOf(100, 25, 1)));

    VideoSurface surface;
    surface.resize(256, 128);
    surface.show();
    surface.attach(&playing);

    // What the widget paints, taken from the widget and not from its buffer.
    const Picture grabbed = pictureOf(surface.grab().toImage());

    REQUIRE(grabbed.width > 0);
    CHECK(subedit::test::frameNumberOf(grabbed) == 100);
}

// GUI-SURFACE-02: after a jump the widget shows the frame of the position, **without the
// window doing anything** — the player announces it and the widget draws.
TEST_CASE("after a jump the widget shows the frame asked, on its own",
          "[gui][video][numbered][GUI-SURFACE-02]") {
    MpvPlayer playing = player();
    REQUIRE(playing.open(fixture("videos/images-25.mp4")).has_value());

    VideoSurface surface;
    surface.resize(256, 128);
    surface.show();
    surface.attach(&playing);

    for (const int frame : {37, 200, 5, 249}) {
        INFO("frame " << frame);
        playing.seek(Timestamp::fromMilliseconds(subedit::test::startOf(frame, 25, 1)));

        CHECK(pumpUntil([&] { return numberShown(surface) == frame; }));
    }
}

// A step shows at once too: the same road.
TEST_CASE("a step is shown by the widget", "[gui][video][numbered][GUI-SURFACE-02]") {
    MpvPlayer playing = player();
    REQUIRE(playing.open(fixture("videos/images-25.mp4")).has_value());
    playing.seek(Timestamp::fromMilliseconds(subedit::test::startOf(50, 25, 1)));

    VideoSurface surface;
    surface.resize(128, 64);
    surface.show();
    surface.attach(&playing);

    playing.stepFrames(1);
    CHECK(pumpUntil([&] { return numberShown(surface) == 51; }));

    playing.stepFrames(-2);
    CHECK(pumpUntil([&] { return numberShown(surface) == 49; }));
}

// GUI-SURFACE-01: resizable, here at the 2:1 of the film so that the number can be read
// across the whole width (the aspect ratio has its own case in `mpv_player_test.cpp`). A paused
// film has no frame coming, so the widget has to draw again by itself, at the new size.
TEST_CASE("resizing the widget draws the picture again at the new size",
          "[gui][video][numbered][GUI-SURFACE-01]") {
    MpvPlayer playing = player();
    REQUIRE(playing.open(fixture("videos/images-25.mp4")).has_value());
    playing.seek(Timestamp::fromMilliseconds(subedit::test::startOf(100, 25, 1)));

    VideoSurface surface;
    surface.resize(256, 128);
    surface.show();
    surface.attach(&playing);
    REQUIRE(numberShown(surface) == 100);

    surface.resize(512, 256);
    QCoreApplication::processEvents();

    CHECK(surface.image().size() == QSize{512, 256});
    CHECK(numberShown(surface) == 100);

    surface.resize(192, 96);
    QCoreApplication::processEvents();
    CHECK(surface.image().size() == QSize{192, 96});
    CHECK(numberShown(surface) == 100);
}

// A surface with nothing attached paints black, and attaching nothing lets go of a
// player that goes on without it.
TEST_CASE("a detached widget paints black and leaves the player alone", "[gui][video][render]") {
    MpvPlayer playing = player();
    REQUIRE(playing.open(fixture("videos/images-25.mp4")).has_value());

    VideoSurface surface;
    surface.resize(128, 64);
    surface.show();
    surface.attach(&playing);
    surface.attach(nullptr);

    playing.seek(Timestamp::fromMilliseconds(1000));
    QCoreApplication::processEvents();

    CHECK(playing.position() == Timestamp::fromMilliseconds(1000));
    CHECK(numberShown(surface) == 0);
}

// Either may go first. The widget lets go of its source in its destructor, and the source
// has to be told nothing afterwards.
TEST_CASE("a widget destroyed before its player, and a player after its widget, are safe",
          "[gui][video][render]") {
    MpvPlayer playing = player();
    REQUIRE(playing.open(fixture("videos/images-25.mp4")).has_value());

    {
        VideoSurface surface;
        surface.resize(128, 64);
        surface.attach(&playing);
        playing.seek(Timestamp::fromMilliseconds(1000));
    }
    // Announcements still queued for a widget that is gone must not reach it.
    playing.seek(Timestamp::fromMilliseconds(2000));
    QCoreApplication::processEvents();

    CHECK(playing.position() == Timestamp::fromMilliseconds(2000));
}
