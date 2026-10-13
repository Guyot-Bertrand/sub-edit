#pragma once

// The language the window speaks — spec 15, D4 and D5, issue #664.

#include <subedit/core/i18n/language.hpp>
#include <subedit/core/io/file_system.hpp>

#include <QString>

#include <filesystem>
#include <memory>
#include <optional>
#include <string>

class QTranslator;

namespace subedit::gui {

/// Installs, for as long as it lives, the catalogue of the language the environment asks for,
/// and Qt's own catalogue of the same language for the words Qt draws itself — the standard
/// buttons, the file dialog. Puts English back when it goes.
///
/// **Chosen once, at start-up, before the window is built** (D4): every label is translated as
/// the window builds it, and re-translating a live window is not worth its cost. `main` holds
/// one of these on its stack, after the `QApplication` and before the window.
///
/// **The setting `interface.language` comes first** (#669): a language chosen in `Preferences…`
/// wins over the environment when it is English or installed (`core::chosenLanguage`).
/// Otherwise **the language is read as the command line reads it** — `LANGUAGE`, `LC_ALL`,
/// `LC_MESSAGES`, `LANG`, by our own code — and **falls back to English silently**: a language that
/// is not installed, a catalogue that does not read, `LC_ALL=C`. Qt's catalogue is only asked for a
/// language of ours: an English window with buttons in another language would be neither.
/// When Qt has none for it, the buttons stay English and nothing is said.
class InterfaceLanguage {

public:
    /// From the process's environment alone — no setting —, the catalogues installed beside the
    /// executable, and the directory where Qt keeps its own.
    InterfaceLanguage();

    /// The same, `preferred` first: the value of `interface.language`, nothing for `system`.
    explicit InterfaceLanguage(const std::optional<std::string>& preferred);

    /// From `environment`, the catalogues under `localeDirectory` (gettext's layout), and Qt's
    /// `qtbase_<language>.qm` under `qtDirectory`.
    InterfaceLanguage(const core::EnvironmentLookup& environment,
                      const std::filesystem::path& localeDirectory,
                      const QString& qtDirectory);

    /// The same, `preferred` first, as `core::chosenLanguage` weighs it.
    InterfaceLanguage(const std::optional<std::string>& preferred,
                      const core::EnvironmentLookup& environment,
                      const std::filesystem::path& localeDirectory,
                      const QString& qtDirectory);

    ~InterfaceLanguage();

    InterfaceLanguage(const InterfaceLanguage&) = delete;
    InterfaceLanguage& operator=(const InterfaceLanguage&) = delete;
    InterfaceLanguage(InterfaceLanguage&&) = delete;
    InterfaceLanguage& operator=(InterfaceLanguage&&) = delete;

    /// The catalogue's name (`ru`, `fr_FR`), or nothing for English.
    [[nodiscard]] const std::optional<std::string>& language() const { return m_language; }

    /// Whether Qt's own catalogue of that language was found and installed.
    [[nodiscard]] bool hasQtCatalogue() const { return m_qt != nullptr; }

private:
    std::optional<std::string> m_language;
    std::unique_ptr<QTranslator> m_qt;
};

/// The value of `interface.language` in the settings at `path`, nothing for `system`.
///
/// **Silent**: it is read before the window — and before the language — exists, so that the
/// window can be built in the language it names; the diagnostics of the same file are written
/// once, translated, when `configureFromEnvironment` reads it again for the window. An unreadable
/// value is `system`, the default, as every unreadable value is its option's default (ADR 0022).
[[nodiscard]] std::optional<std::string> settingsLanguage(const core::FileSystem& files,
                                                          const std::filesystem::path& path);

/// The same, at the place this user's settings live — the overload `main` calls, as it calls
/// `readUserSettings`'s.
[[nodiscard]] std::optional<std::string> settingsLanguage(const core::FileSystem& files);

} // namespace subedit::gui
