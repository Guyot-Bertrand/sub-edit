#pragma once

#include <cstddef>
#include <functional>
#include <span>

namespace subedit::gui {

/// Where the picture of a video comes from, for the widget that paints it — ADR 0041.
///
/// **Beside `core::VideoPlayer` and not inside it**: the core's interface knows neither
/// Qt nor libmpv, and a picture is neither. A player that can be drawn implements both;
/// the double of the tests implements the first only, and a window given one shows a
/// surface that stays black.
///
/// **Two orders, and a thread to keep in mind.** The notification comes from a thread
/// of the player and must touch nothing but a queue to the window's own thread; the
/// drawing is asked from the window's thread, and from no other.
class FrameSource {

public:
    virtual ~FrameSource() = default;

    /// Sets what is called when a new picture is ready — **from a thread of the player**,
    /// so `notify` may only hand over to the window's thread. An empty function stops the
    /// notifications, and returns once none is running: a widget calls this before it is
    /// destroyed.
    virtual void onFrameReady(std::function<void()> notify) = 0;

    /// Draws the picture the player is on into `pixels`, four bytes a pixel — blue,
    /// green, red and one unused — `stride` bytes a row, at `width` by `height`. libmpv
    /// keeps the aspect ratio of the film and leaves the rest black.
    ///
    /// Says whether a picture was drawn: nothing is, with no film open.
    [[nodiscard]] virtual bool
    render(std::span<unsigned char> pixels, int width, int height, std::size_t stride) = 0;

protected:
    FrameSource() = default;
    FrameSource(const FrameSource&) = default;
    FrameSource(FrameSource&&) = default;
    FrameSource& operator=(const FrameSource&) = default;
    FrameSource& operator=(FrameSource&&) = default;
};

} // namespace subedit::gui
