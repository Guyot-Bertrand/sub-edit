#include <subedit/core/video/video_player.hpp>
#include <subedit/gui/mpv_player.hpp>
#include <subedit/gui/player_factory.hpp>

#include <QGuiApplication>
#include <QLatin1StringView>
#include <QString>

#include <cstdint>
#include <expected>
#include <memory>
#include <utility>

namespace subedit::gui {

namespace {

/// The one Qt platform whose windows libmpv can adopt.
///
/// **Embedding a native window is an X11 mechanism**, and libmpv says so in its
/// own header: « This works on X11, win32, and OSX only ». On Wayland the
/// number `winId` gives is a surface of another kind entirely, and handing it
/// over would put libmpv into an X error rather than into a picture.
///
/// So this program plays a film on `xcb` and nowhere else, and the manual says
/// what to do about it — a Wayland session runs `subedit-gui` under XWayland by
/// setting `QT_QPA_PLATFORM=xcb`.
constexpr QLatin1StringView kEmbeddable{"xcb"};

} // namespace

PlayerFactory mpvPlayers() {
    return [](std::uintptr_t surface) -> std::unique_ptr<core::VideoPlayer> {
        if (QGuiApplication::platformName() != kEmbeddable)
            return nullptr;

        std::expected<MpvPlayer, core::PlayerError> built = MpvPlayer::create(surface);
        if (!built)
            // Nothing, and no reason given: the two ways of coming back
            // empty-handed — a platform that cannot host a picture, a libmpv
            // that would not start — end in the same place for whoever asked,
            // which is a window with no picture in it. The window says so
            // once, when a film is actually chosen, rather than here where
            // nobody has asked for anything yet.
            return nullptr;

        return std::make_unique<MpvPlayer>(std::move(*built));
    };
}

} // namespace subedit::gui
