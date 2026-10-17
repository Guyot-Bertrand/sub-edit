#pragma once

// Which libraries the program runs against, and which it was built against — issue #716.
//
// **One entry per library, gathered in one place**: a bug report has to say which ICU or which
// Enchant the binary found on the machine, since those are the libraries whose behaviour differs
// from one machine to the next. The core answers for what it links itself; the window adds Qt and
// libmpv (`gui/libraries.hpp`), the command line adds CLI11, and both write the result through
// `versionReport` (`core/wording/version.hpp`), so the two surfaces cannot disagree.
//
// Adding a library is one line in the function of the layer that links it.
// `version_report_test` in the end-to-end harness reads the `find_package` and
// `pkg_check_modules` calls of the build and fails on one that no line names.

#include <filesystem>
#include <string>
#include <vector>

namespace subedit::core {

/// A library and its two versions.
///
/// **The name is never translated**: it is the library's own. Neither are the versions, which are
/// the library's own strings — `74.2`, `2.3.3`.
struct Library {
    std::string name;

    /// The version found at run time — what the machine really gives the program.
    std::string loaded;

    /// The version of the headers the program was compiled with.
    std::string built;

    friend bool operator==(const Library&, const Library&) = default;
};

/// The libraries the core links, in this order: ICU, then Enchant.
///
/// ICU answers both versions itself (`U_ICU_VERSION`, `u_getVersion`). Enchant's headers carry no
/// version, so the one it was built against is the one `pkg-config` found when the build was
/// configured; the loaded one is `enchant_get_version()`.
[[nodiscard]] std::vector<Library> coreLibraries();

/// The interface languages installed under `localeDirectory`, by code: `en` first, which is the
/// source and needs no catalogue, then every language `availableLanguages` finds, sorted. The
/// codes are never translated.
[[nodiscard]] std::vector<std::string>
installedLanguageCodes(const std::filesystem::path& localeDirectory);

} // namespace subedit::core
