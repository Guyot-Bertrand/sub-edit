#pragma once

#include <filesystem>

namespace subedit::gui {

/// Where this user's settings live, whether or not anything is there yet.
///
/// **The one place the standard locations are read.** ADR 0022 puts the
/// settings in the core, reading and writing through `core::FileSystem`, and
/// has them *receive* a path rather than look one up. That leaves exactly one
/// piece of code that has to know where the file is, and this is it: `main.cpp`
/// calls it and hands the answer down.
///
/// The consequence is the point. Nothing else resolves a location, so no test
/// can reach the real one by accident — a test either gives a path of its own
/// or works through `InMemoryFileSystem`. What used to be a rule to remember
/// is a shape the code has.
///
/// **Delegates to `platform::resolvedUserSettingsPath()`**, which the command
/// line shares, so that both surfaces read the same file by the same code. It
/// honours `$XDG_CONFIG_HOME` (when absolute) and falls back to `~/.config`, and
/// never depends on the running program's name: a path that changes with the
/// process that asks for it is a path no test can state.
///
/// Absolute, and it need not exist: a missing file is every default, which is
/// not an error.
[[nodiscard]] std::filesystem::path userSettingsPath();

} // namespace subedit::gui
