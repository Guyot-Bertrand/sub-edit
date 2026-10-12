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
