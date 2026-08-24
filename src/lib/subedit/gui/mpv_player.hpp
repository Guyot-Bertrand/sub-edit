#pragma once

#include <subedit/core/time/duration.hpp>
#include <subedit/core/time/timestamp.hpp>
#include <subedit/core/video/video_player.hpp>

#include <cstdint>
#include <expected>
#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <string_view>

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
/// **It draws into the window it is given, and the window is a number.**
/// libmpv adopts a native window handed to it as the `wid` option; the
/// rendering was decided between that and an OpenGL context Qt would provide,
/// and adopting the native window is the shorter of the two. It leaves the
/// subtitle to libmpv's own overlay, drawn from the model, which is what D2
/// asks for.
///
/// A number and not a widget, though this class lives beside the window: it
/// is all libmpv wants, and taking a `QWidget` would put Qt in a header that
/// has no other use for it.
///
/// **And that window has to be an X11 one**, which is what adopting a native
/// window means — libmpv's own header says « X11, win32, and OSX only ». So a
/// player given a window asks for an X11 context rather than letting libmpv
/// probe, and `mpvPlayers` is what keeps a window of any other kind from ever
/// reaching here.
///
/// **A player given no window draws nowhere and makes no sound**, and that is
/// the shape every test uses: `vo=null` is what lets one run where there is no
/// screen, measured rather than promised — with `vo=auto` and no display, mpv
/// does not even open the file.
///
/// **A handle is a resource**, in the sense of the project's second design
/// principle: libmpv gives one out, and it has to be given back. It is held by
/// a `unique_ptr` with a deleter of its own, so that a player which fails
/// halfway through being built, or which is moved from, gives it back exactly
/// once.
class MpvPlayer final : public core::VideoPlayer {

public:
    /// Builds a player drawing into `window`, or says why libmpv would not
    /// give one.
    ///
    /// A factory and not a constructor: building one can fail, and a
    /// constructor that fails has only exceptions to say so with.
    ///
    /// **`window` is settled here and never again.** libmpv reads `wid` while
    /// it initialises and ignores it afterwards — measured: set after
    /// `mpv_initialize`, the property answers « success » and the value does
    /// not stick. A player therefore belongs to one surface for its whole
    /// life, which is why the window builds one once its own is native and
    /// hands the number over.
    ///
    /// Zero means nowhere, which is what every test uses.
    [[nodiscard]] static std::expected<MpvPlayer, core::PlayerError>
    create(std::uintptr_t window = 0);

    [[nodiscard]] std::expected<void, core::PlayerError>
    open(const std::filesystem::path& video) override;

    [[nodiscard]] std::optional<core::Duration> duration() const override;

    [[nodiscard]] std::optional<core::Timestamp> position() const override;

    void seek(core::Timestamp position) override;

    void play() override;

    void pause() override;

    void showSubtitle(std::string_view line) override;

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

/// Turns a subtitle's text into the ASS event libmpv's overlay draws.
///
/// **Exposed for the one reason `videoFilters` is**: it is a thing this file
/// can get wrong on its own, and nothing that reaches libmpv can be read back
/// — an overlay is written to the picture, and there is no picture to look at
/// where these tests run. Out here it is an ordinary function with ordinary
/// cases.
///
/// What it does, and the whole of it: line breaks become the `\N` of ASS, and
/// the braces that would open an override block are escaped so that a subtitle
/// saying `{` says `{`. Empty in, empty out — that is how the window clears
/// the overlay.
///
/// **The text is drawn as the file holds it**, tags included (ADR 0009).
/// Gaupol strips them before drawing; understanding a tag well enough to
/// remove it is what phase 9 is for, and inventing a stripper here would be a
/// second answer to a question this project has not answered yet.
[[nodiscard]] std::string assEventOf(std::string_view line);

} // namespace subedit::gui
