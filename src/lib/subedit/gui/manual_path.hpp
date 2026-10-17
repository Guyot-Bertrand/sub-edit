#pragma once

#include <filesystem>
#include <optional>
#include <string>
#include <vector>

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

/// The trees `Help ▸ Manual` reads a page from, the first that has it winning — spec 15, D9,
/// issue #682.
///
/// **English is the root, and every other language a directory under it**, named as its catalogue
/// is: `<root>/fr`. That keeps the English manual where it always was — the man pages name it,
/// the packages carry it — and puts each translation beside it, under the same
/// `share/subedit/manual`.
///
/// `language` is the one the window speaks, as `InterfaceLanguage::language()` answers it, nothing
/// for English. A regional name is tried first, then its language: `fr_CA` gives `<root>/fr_CA`,
/// `<root>/fr`, then `<root>`. **English always comes last**, so that a page the translation does
/// not have, or a language with no manual at all, opens in English rather than not at all. A name
/// that is not a language's (`core::isLanguageName`) is ignored: it would name a directory, and
/// `../` is not one this function lets through.
///
/// Pure: nothing is looked up. A tree that is not installed simply has no page.
[[nodiscard]] std::vector<std::filesystem::path>
manualTrees(const std::filesystem::path& root, const std::optional<std::string>& language);

} // namespace subedit::gui
