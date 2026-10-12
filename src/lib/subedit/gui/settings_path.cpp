#include <subedit/gui/settings_path.hpp>
#include <subedit/platform/locations.hpp>

namespace subedit::gui {

std::filesystem::path userSettingsPath() {
    return platform::resolvedUserSettingsPath();
}

} // namespace subedit::gui
