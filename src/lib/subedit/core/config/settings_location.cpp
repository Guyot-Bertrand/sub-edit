#include <subedit/core/config/settings_location.hpp>

namespace subedit::core {

std::filesystem::path userSettingsLocation(std::string_view xdgConfigHome, std::string_view home) {
    std::filesystem::path base;
    if (const std::filesystem::path xdg{xdgConfigHome}; !xdgConfigHome.empty() && xdg.is_absolute())
        base = xdg;
    else if (!home.empty())
        base = std::filesystem::path{home} / ".config";
    else
        return {};
    return base / "subedit" / "settings.conf";
}

} // namespace subedit::core
