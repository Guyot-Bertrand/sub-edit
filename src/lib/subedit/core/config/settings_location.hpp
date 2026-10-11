#pragma once

#include <filesystem>
#include <string_view>

namespace subedit::core {

/// Where the user's settings file is, from the two variables that decide it.
///
/// **Pure, like `userPatternsPath` (ADR 0037)**: it reads no variable. The one
/// code that does is `platform::resolvedUserSettingsPath()`. A relative
/// `xdgConfigHome` is ignored, as the XDG specification says. Empty when neither
/// variable gives a base.
[[nodiscard]] std::filesystem::path userSettingsLocation(std::string_view xdgConfigHome,
                                                         std::string_view home);

} // namespace subedit::core
