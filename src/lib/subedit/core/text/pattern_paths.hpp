#pragma once

// Where the pattern files are, worked out from what the caller knows.
//
// **Pure functions of what they are handed** — ADR 0037, in the spirit of
// ADR 0022: the core reads no environment variable and asks no executable where
// it lives, so no test reaches a real location by accident. The one place that
// resolves them, with the real environment and the real executable, is the
// surface that first needs the answer; it calls these two and passes the
// results to `readPatternCatalogue`.

#include <filesystem>
#include <string_view>

namespace subedit::core {

/// The directory the package installs the shipped patterns in, seen from the
/// directory of the executable: `<prefix>/bin` → `<prefix>/share/subedit/patterns`.
///
/// **Worked out from the executable, and never from a path frozen at build
/// time**, for the reason `installedManualPath()` gives: the configure prefix
/// and the install prefix are not the same, and this layout is the one
/// `GNUInstallDirs` produces for any of them.
[[nodiscard]] std::filesystem::path
shippedPatternsPath(const std::filesystem::path& executableDirectory);

/// Where a user drops patterns of her own: `$XDG_DATA_HOME/subedit/patterns`,
/// or `~/.local/share/subedit/patterns` when the variable is unset, empty or
/// not absolute — the rule of the XDG specification, which says a relative
/// value is to be ignored.
///
/// Empty when neither is known: a machine with no home has no user directory,
/// and `readPatternCatalogue` reads an empty path as none.
[[nodiscard]] std::filesystem::path userPatternsPath(std::string_view xdgDataHome,
                                                     std::string_view home);

} // namespace subedit::core
