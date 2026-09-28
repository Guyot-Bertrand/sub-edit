#pragma once

#include <filesystem>

namespace subedit::gui {

/// Where the shipped patterns are installed, worked out from the executable —
/// never from a path frozen at build time, the same rule `installedManualPath()`
/// follows and for the same reason: the configure prefix and the install
/// prefix are not the same.
[[nodiscard]] std::filesystem::path installedPatternsPath();

/// Where a user drops patterns of her own, read from the real environment —
/// the one place allowed to, since `core::userPatternsPath` itself takes the
/// two strings rather than reading them (ADR 0037, ADR 0022's own rule for
/// `userSettingsPath()`).
[[nodiscard]] std::filesystem::path resolvedUserPatternsPath();

} // namespace subedit::gui
