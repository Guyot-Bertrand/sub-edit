#pragma once

#include <subedit/core/time/duration.hpp>
#include <subedit/core/time/timestamp.hpp>
#include <subedit/core/video/video_player.hpp>

#include <expected>
#include <filesystem>
#include <memory>
#include <optional>

// The one type of libmpv that appears here, and it appears only as a name: a
// player owns a handle, and a header the window includes has no business
// dragging <mpv/client.h> along with it.
struct mpv_handle;

namespace subedit::gui {

/// The player of ADR 0020: libmpv, behind `core::VideoPlayer`.
///
/// **Here rather than in the core, though nothing in it knows Qt.** A player
/// is a thing of the interface — it exists for the window, it takes its window
/// from it, and nothing else will ever build one. What the core keeps is the
/// interface alone, which names neither libmpv nor Qt; letting the
/// implementation in with it would have given the domain a dependency on a
/// media library for the convenience of a lighter test harness, and
/// convenience is not an architecture.
///
/// **It shows nothing yet, and that is where #176 comes in.** libmpv draws
/// into a window the system numbers, handed to it as the `wid` option — a
/// number, which is how the core's interface can carry a window without
/// knowing what a window is. The rendering was decided between that and an
/// OpenGL context Qt would provide: adopting the native window is the shorter
/// of the two, and it leaves the subtitle to be drawn by libmpv's own overlay
/// from the model, which is what D2 asks for. Until a window exists to hand
/// over, this player decodes and displays nowhere.
///
/// **A handle is a resource**, in the sense of the project's second design
/// principle: libmpv gives one out, and it has to be given back. It is held by
/// a `unique_ptr` with a deleter of its own, so that a player which fails
/// halfway through being built, or which is moved from, gives it back exactly
/// once.
class MpvPlayer final : public core::VideoPlayer {

public:
    /// Builds a player, or says why libmpv would not give one.
    ///
    /// A factory and not a constructor: building one can fail, and a
    /// constructor that fails has only exceptions to say so with.
    [[nodiscard]] static std::expected<MpvPlayer, core::PlayerError> create();

    [[nodiscard]] std::expected<void, core::PlayerError>
    open(const std::filesystem::path& video) override;

    [[nodiscard]] std::optional<core::Duration> duration() const override;

    [[nodiscard]] std::optional<core::Timestamp> position() const override;

    void seek(core::Timestamp position) override;

    void play() override;

    void pause() override;

    [[nodiscard]] bool isPlaying() const override;

private:
    /// Gives the handle back to libmpv, once.
    struct TerminateAndDestroy {
        void operator()(mpv_handle* player) const noexcept;
    };

    using Handle = std::unique_ptr<mpv_handle, TerminateAndDestroy>;

    explicit MpvPlayer(Handle handle) : m_handle(std::move(handle)) {}

    /// Whether a video is loaded. Every question below answers « nothing » and
    /// every order does nothing while this is false.
    bool m_open = false;

    Handle m_handle;
};

} // namespace subedit::gui
