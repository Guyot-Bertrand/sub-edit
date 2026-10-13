#pragma once

// Which language the user asked for — ADR 0042, D4.
//
// The libc functions answer only for locales generated on the machine, so a user who sets
// `LANGUAGE=fr` on a fresh install gets English without being told. This reads the same
// variables in the same order, by our own code, and answers with names to look for: whether
// a catalogue of that name exists is for the caller to find out.

#include <subedit/core/i18n/catalogue.hpp>

#include <filesystem>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace subedit::core {

/// The value of an environment variable, or nothing when it is not set.
using EnvironmentLookup = std::function<std::optional<std::string>(std::string_view name)>;

/// The names of the catalogues to look for, the preferred first: `fr_FR` then `fr` for a
/// locale `fr_FR.UTF-8@euro`; every entry of `LANGUAGE` in turn, each expanded the same way.
///
/// **Empty means English**, which is the source language and has no catalogue: `C`, `POSIX`,
/// an unset environment, and the point where an entry names English (`en`, `en_GB`) — what
/// comes after it never applies, as with gettext. `LANGUAGE` is ignored under the `C` locale,
/// as gettext ignores it: that is what makes `LC_ALL=C` the way to ask for English.
[[nodiscard]] std::vector<std::string> languageCandidates(const EnvironmentLookup& environment);

/// The process's own environment.
[[nodiscard]] EnvironmentLookup processEnvironment();

/// Where the catalogue of `language` is under `localeDirectory`: gettext's own layout,
/// `<directory>/<language>/LC_MESSAGES/subedit.mo`, which `cmake/Translations.cmake` installs.
[[nodiscard]] std::filesystem::path cataloguePath(const std::filesystem::path& localeDirectory,
                                                  std::string_view language);

/// A language found installed: the candidate it was found under, and its catalogue.
struct InstalledLanguage {
    std::string name;
    std::shared_ptr<const Catalogue> catalogue;
};

/// The first of `candidates` that is installed under `localeDirectory` and reads, or nothing —
/// English. The window needs the name as well as the catalogue: it asks Qt for its own catalogue
/// of the same language (#664).
///
/// Silent, as `installedCatalogue` below, for the same reason.
[[nodiscard]] std::optional<InstalledLanguage>
installedLanguage(const std::vector<std::string>& candidates,
                  const std::filesystem::path& localeDirectory);

/// Whether `text` has the shape of a catalogue's name: two or three lower-case letters, then
/// optionally `_` and two upper-case letters — `fr`, `ast`, `pt_BR`. What the settings file
/// accepts for `interface.language` (#669); whether such a catalogue exists is another question.
[[nodiscard]] bool isLanguageName(std::string_view text);

/// The language the window starts in — D4, issue #669: `preferred`, the user's setting, when it
/// names English or a language installed under `localeDirectory`; the environment's otherwise.
///
/// **The setting wins**, over `LANGUAGE` and over `LC_ALL=C` alike: it was chosen in the window,
/// on purpose. A setting whose catalogue is not installed — a file carried to another machine, a
/// language removed — is passed over silently, as an uninstalled language of the environment is,
/// and the environment decides. Nothing for English.
[[nodiscard]] std::optional<InstalledLanguage>
chosenLanguage(const std::optional<std::string>& preferred,
               const EnvironmentLookup& environment,
               const std::filesystem::path& localeDirectory);

/// The languages installed under `localeDirectory`, by name and sorted: every directory whose
/// catalogue reads, tried by the code that loads one, so that a language offered is a language
/// that would load. English is not among them: it is the source and has no catalogue. Empty when
/// the directory is missing.
[[nodiscard]] std::vector<std::string>
availableLanguages(const std::filesystem::path& localeDirectory);

/// A language by its own name, in its own language — `Français` for `fr`, `English` for `en` —
/// or nothing for a language this table does not know. A regional name with no entry of its own
/// takes its language's: `fr_CA` is `Français`. Never translated: a reader looks for the name of
/// the language they read (#669).
[[nodiscard]] std::optional<std::string_view> endonymOf(std::string_view language);

/// The catalogue of the first of `candidates` that is installed under `localeDirectory` and
/// reads, or null — English.
///
/// **Silent, on purpose** (D4): a language that is not installed, and a catalogue that is
/// refused, are passed over like a missing one. A user who asks for a language the program
/// does not ship gets English, which is the source, rather than a complaint on every run.
[[nodiscard]] std::shared_ptr<const Catalogue>
installedCatalogue(const std::vector<std::string>& candidates,
                   const std::filesystem::path& localeDirectory);

} // namespace subedit::core
