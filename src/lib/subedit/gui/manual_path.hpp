#pragma once

#include <filesystem>

namespace subedit::gui {

/// Where the installed manual is, whether or not it is there.
///
/// **Worked out from the executable, and never from a path frozen at build
/// time.** The configure prefix and the install prefix are not the same: the
/// manual describes `cmake --install build/release --prefix ~/.local`, where
/// the second is `~/.local` while the first stayed `/usr/local`. A path carved
/// in at build time would therefore name the wrong place in the very use the
/// manual recommends.
///
/// `<directory of the executable>/../share/subedit/manual` is right for every
/// prefix — the package's `/usr`, `/usr/local`, `~/.local`, the temporary
/// directory of `check-installation.sh` — because it is the layout
/// `GNUInstallDirs` produces, whatever the prefix.
///
/// **The same role as `userSettingsPath()`, and the same rule** — ADR 0022:
/// this is the only code that resolves this location, `main.cpp` calls it and
/// passes the answer to the window. No test therefore reaches the real manual
/// by accident: it gives whatever path it likes.
///
/// Absolute, and it may not exist — a binary run from the build tree has no
/// manual beside it, which is not an error.
[[nodiscard]] std::filesystem::path installedManualPath();

} // namespace subedit::gui
