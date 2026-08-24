#include <subedit/core/video/video_player.hpp>
#include <subedit/gui/mpv_player.hpp>
#include <subedit/gui/player_factory.hpp>

#include <QByteArray>
#include <QGuiApplication>
#include <QLatin1StringView>
#include <QString>
#include <qglobal.h>

#include <cstdint>
#include <expected>
#include <memory>
#include <string_view>
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
/// So this program plays a film on `xcb` and nowhere else — which is why
/// `preferEmbeddablePlatform` asks for it when the session leaves the choice
/// open, rather than leaving the user to set a variable and start again.
constexpr std::string_view kEmbeddablePlatform{"xcb"};

/// The same name, for the half of this file that speaks to Qt. Built from the
/// one above so that the word is written once.
constexpr QLatin1StringView kEmbeddable{kEmbeddablePlatform.data(),
                                        static_cast<qsizetype>(kEmbeddablePlatform.size())};

/// What Qt reads to know which platform to run on.
constexpr const char* kPlatformVariable = "QT_QPA_PLATFORM";

} // namespace

std::string_view
platformFor(std::string_view chosen, std::string_view wayland, std::string_view x11) {
    // Somebody who named a platform gets it, whatever it costs them.
    if (!chosen.empty())
        return {};

    // Not a Wayland session: Qt is about to pick something this program has no
    // opinion about, and `xcb` is very likely what it picks anyway.
    if (wayland.empty())
        return {};

    // A Wayland session with no X server beside it. There is nothing better to
    // ask for, and the window will say it cannot show a film — which is true,
    // and better said than worked around.
    if (x11.empty())
        return {};

    return kEmbeddablePlatform;
}

void preferEmbeddablePlatform() {
    const std::string_view wanted = platformFor(qgetenv(kPlatformVariable).constData(),
                                                qgetenv("WAYLAND_DISPLAY").constData(),
                                                qgetenv("DISPLAY").constData());
    if (!wanted.empty())
        qputenv(kPlatformVariable,
                QByteArray::fromRawData(wanted.data(), static_cast<qsizetype>(wanted.size())));
}

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
