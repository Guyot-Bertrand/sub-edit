#pragma once

#include <subedit/core/time/duration.hpp>
#include <subedit/core/time/timestamp.hpp>
#include <subedit/core/video/video_player.hpp>
#include <subedit/gui/frame_source.hpp>

#include <cstddef>
#include <cstdint>
#include <expected>
#include <filesystem>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

// The one type of libmpv that appears here, and it appears only as a name: a
// player owns a handle, and a header the window includes has no business
// dragging <mpv/client.h> along with it.
struct mpv_handle;
struct mpv_render_context;

namespace subedit::gui {

/// A picture the player shows: the pixels of the frame on screen, as libmpv
/// hands them over — four bytes a pixel, blue, green, red and one unused, rows
/// back to back.
///
/// **What a test reads to know which frame the player shows**, rather than what
/// the player says of itself: its position and its frame counter come from the
/// same place as the picture, and a player that showed the wrong frame while
/// announcing the right time would pass any check made on them. Issue #610.
struct Picture {
    int width = 0;
    int height = 0;
    std::vector<unsigned char> pixels{};

    /// The green channel at (`x`, `y`) — the brightness, for a grey picture.
    [[nodiscard]] unsigned char greenAt(int x, int y) const {
        return pixels[(((static_cast<std::size_t>(y) * static_cast<std::size_t>(width)) +
                        static_cast<std::size_t>(x)) *
                       kBytesAPixel) +
                      1U];
    }

    static constexpr std::size_t kBytesAPixel = 4;
};

/// Whether a player makes a sound. Every test builds one that does not: a player nobody
/// can see should not be heard, and a runner has no sound device anyway.
enum class Sound : unsigned char { Off, On };

/// The player of ADR 0020: libmpv, behind `core::VideoPlayer`.
///
/// **Here rather than in the core, though nothing in it knows Qt.** A player
/// is a thing of the interface — it exists for the window, and nothing else
/// will ever build one. What the core keeps is the interface alone, which names
/// neither libmpv nor Qt; letting the implementation in with it would have given
/// the domain a dependency on a media library for the convenience of a lighter
/// test harness, and convenience is not an architecture.
///
/// **It draws into a buffer it is handed, and that is all it knows of a window**
/// — ADR 0041. libmpv's software render API (`vo=libmpv`) needs no native window, no
/// graphic context and no X11: `render` fills the pixels the widget gives, and the
/// widget paints them. The same code runs on X11, Wayland and with no screen at all,
/// which is what lets a test read the picture the player shows. It leaves the subtitle
/// to libmpv's own overlay, drawn from the model, which is what D2 asks for.
///
/// **A handle is a resource**, in the sense of the project's second design
/// principle: libmpv gives one out, and it has to be given back. It is held by
/// a `unique_ptr` with a deleter of its own, so that a player which fails
/// halfway through being built, or which is moved from, gives it back exactly
/// once. **The render context is freed before the handle, always** — it is declared
/// after it, which is what makes the order not a matter of vigilance.
class MpvPlayer final : public core::VideoPlayer, public FrameSource {

public:
    /// Builds a player, or says why libmpv would not give one.
    ///
    /// A factory and not a constructor: building one can fail, and a
    /// constructor that fails has only exceptions to say so with.
    [[nodiscard]] static std::expected<MpvPlayer, core::PlayerError>
    create(Sound sound = Sound::Off);

    [[nodiscard]] std::expected<void, core::PlayerError>
    open(const std::filesystem::path& video) override;

    [[nodiscard]] std::optional<core::Duration> duration() const override;

    [[nodiscard]] std::optional<core::Timestamp> position() const override;

    void seek(core::Timestamp position) override;

    void stepFrames(int frames) override;

    void play() override;

    void playUntil(core::Timestamp end) override;

    void pause() override;

    void showSubtitle(std::string_view line) override;

    [[nodiscard]] bool isPlaying() const override;

    [[nodiscard]] int volume() const override;

    void setVolume(int volume) override;

    [[nodiscard]] std::vector<core::AudioTrack> audioTracks() const override;

    void selectAudioTrack(int id) override;

    void onFrameReady(std::function<void()> notify) override;

    [[nodiscard]] bool
    render(std::span<unsigned char> pixels, int width, int height, std::size_t stride) override;

    /// The picture on screen now, or nothing when no video is open or libmpv
    /// would not give one. **Not an order of the seam**: `VideoPlayer` stays
    /// free of pixels, and what reads this is a test, and — when ADR 0041 puts
    /// the picture in the window — whatever paints it.
    [[nodiscard]] std::optional<Picture> picture() const;

private:
    /// Gives the handle back to libmpv, once.
    struct TerminateAndDestroy {
        void operator()(mpv_handle* player) const noexcept;
    };

    /// Gives the render context back to libmpv, once.
    struct FreeRenderContext {
        void operator()(mpv_render_context* context) const noexcept;
    };

    /// What libmpv's update callback reaches: the function to call, and the lock that
    /// keeps `onFrameReady` from returning while it runs. On the heap, so that its
    /// address — which libmpv holds — survives a move of the player.
    struct Notifier {
        std::mutex lock;
        std::function<void()> notify;
    };

    using Handle = std::unique_ptr<mpv_handle, TerminateAndDestroy>;
    using RenderContext = std::unique_ptr<mpv_render_context, FreeRenderContext>;

    MpvPlayer(Handle handle, std::unique_ptr<Notifier> notifier, RenderContext render)
        : m_handle(std::move(handle)),
          m_notifier(std::move(notifier)),
          m_render(std::move(render)) {}

    /// Whether a video is loaded. Every question below answers « nothing » and
    /// every order does nothing while this is false.
    bool m_open = false;

    // **The order is the contract**: members are destroyed last to first, so the render
    // context goes before the notifier it calls and before the handle it draws from.
    Handle m_handle;
    std::unique_ptr<Notifier> m_notifier;
    RenderContext m_render;
};

/// Turns the replica handed to the player into the ASS event libmpv's overlay draws.
///
/// **Exposed for the one reason `videoFilters` is**: it is a thing this file
/// can get wrong on its own, and nothing that reaches libmpv can be read back
/// — an overlay is written to the picture, and there is no picture to look at
/// where these tests run. Out here it is an ordinary function with ordinary
/// cases.
///
/// What it does, and the whole of it: line breaks become the `\N` of ASS, and
/// the foot-of-picture alignment is put in front. Empty in, empty out — that is
/// how the window clears the overlay.
///
/// **The text is already in the Sub Station Alpha vocabulary** — issue #408. It is what
/// `core::replicaOf` writes: styles are override blocks libass applies, the braces of the
/// visible text are escaped, and tags with no equivalent on screen are gone. Escaping
/// them here as well would draw the styles as text, which is what this used to do with
/// every tag a subtitle carried.
[[nodiscard]] std::string assEventOf(std::string_view line);

} // namespace subedit::gui
