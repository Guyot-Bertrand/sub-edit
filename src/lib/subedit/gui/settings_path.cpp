#include <subedit/gui/settings_path.hpp>

#include <QStandardPaths>
#include <QString>

#include <filesystem>

namespace subedit::gui {

std::filesystem::path userSettingsPath() {
    // `GenericConfigLocation` is `$XDG_CONFIG_HOME`, or `~/.config` when that
    // variable is unset — which is what makes the test harness able to move it:
    // a process that sets the variable moves every location derived from it,
    // itself and everything it spawns, without this function knowing.
    const QString base = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation);

    return std::filesystem::path{base.toStdString()} / "subedit" / "settings.conf";
}

} // namespace subedit::gui
