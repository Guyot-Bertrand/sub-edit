#include <subedit/core/wording/settings.hpp>

#include <string_view>
#include <utility>

namespace subedit::core {

std::string_view nameOf(Theme theme) {
    switch (theme) {
    case Theme::System:
        return "System";
    case Theme::Light:
        return "Light";
    case Theme::Dark:
        return "Dark";
    }
    std::unreachable();
}

std::string_view nameOf(InsertPlacement placement) {
    switch (placement) {
    case InsertPlacement::Above:
        return "Above the selection";
    case InsertPlacement::Below:
        return "Below the selection";
    }
    std::unreachable();
}

std::string_view systemThemeExplained() {
    return "leaves the colours to the desktop";
}

std::string_view settingsFileHeader() {
    return "# subedit settings.\n"
           "#\n"
           "# Options left at their default are written commented out. Uncomment a\n"
           "# line and change its value to override it; delete the line to go back\n"
           "# to the default, whatever that default becomes in a later version.\n"
           "#\n"
           "# Two of them default to whatever the window chooses rather than to a\n"
           "# number: their commented line shows the shape of a value, not a\n"
           "# default that is in force.\n"
           "#\n"
           "# A value this file cannot read is left at its default, and said so on\n"
           "# the standard error when the program starts.\n";
}

std::string_view unreadableSetting() {
    return "cannot be read, keeping the default";
}

std::string_view settingsNotWritten() {
    return "settings could not be written";
}

} // namespace subedit::core
