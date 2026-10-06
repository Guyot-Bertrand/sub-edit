#include <subedit/core/io/file_system.hpp>
#include <subedit/core/time/frame_rate.hpp>
#include <subedit/core/video/declared_frame_rate.hpp>
#include <subedit/core/video/video_player.hpp>
#include <subedit/gui/mpv_player.hpp>
#include <subedit/gui/player_factory.hpp>

#include <QByteArray>
#include <qglobal.h>

#include <expected>
#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <utility>

namespace subedit::gui {

FrameRateReader declaredFrameRates(const core::FileSystem& files) {
    return [&files](const std::filesystem::path& video) -> std::optional<core::FrameRate> {
        // Read here rather than captured once: a `PATH` is a thing a session
        // can change, and asking again costs a string.
        const QByteArray path = qgetenv("PATH");
        return core::readDeclaredFrameRate(
            files,
            std::string_view{path.constData(), static_cast<std::size_t>(path.size())},
            video);
    };
}

PlayerFactory mpvPlayers() {
    return []() -> std::unique_ptr<core::VideoPlayer> {
        std::expected<MpvPlayer, core::PlayerError> built = MpvPlayer::create(Sound::On);
        if (!built)
            // Nothing, and no reason given: a libmpv that would not start ends where
            // every other absence of a player does, in a window with no picture. The
            // window says so once, when a film is actually chosen, rather than here
            // where nobody has asked for anything yet.
            return nullptr;

        return std::make_unique<MpvPlayer>(std::move(*built));
    };
}

} // namespace subedit::gui
