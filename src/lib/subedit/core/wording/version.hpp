#pragma once

// What `--version` says, in both binaries — issue #716. Shared wording of `core/wording/`; see
// `formats.hpp` for why it lives in the core.

#include <subedit/core/libraries.hpp>

#include <string>
#include <vector>

namespace subedit::core {

/// The block `--version` writes, the command line's and the window's alike:
///
/// ```text
/// subedit 0.15.39
///   ICU      74.2 (built with 74.2)
///   Enchant  2.3.3 (built with 2.3.3)
///   Languages: en, fr
/// ```
///
/// **The first line is `subedit <version>`, alone and never translated**: scripts read it, and it
/// was the whole output until #716. A line per library follows, its name padded to the longest so
/// that the versions line up, then the installed languages. The names and versions are never
/// translated; the words around them are. Ends with an end of line.
[[nodiscard]] std::string versionReport(const std::vector<Library>& libraries,
                                        const std::vector<std::string>& languages);

/// A library's two versions as a reader is told them: `74.2 (built with 74.2)`.
[[nodiscard]] std::string libraryVersions(const Library& library);

/// The installed languages, by code: `Languages: en, fr`.
[[nodiscard]] std::string languagesLine(const std::vector<std::string>& languages);

} // namespace subedit::core
