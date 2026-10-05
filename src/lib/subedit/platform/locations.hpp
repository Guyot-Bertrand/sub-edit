#pragma once

// Where the shipped and the user's patterns are, resolved against the real
// executable and the real environment — for the two surfaces, from one place.
//
// **The one place that resolves them.** `core::shippedPatternsPath` and
// `core::userPatternsPath` stay pure functions of what they are handed (ADR 0037):
// the core reads no variable and asks no executable where it lives. This library
// is what hands them the answers, and it knows no Qt — the command line links it
// as the window does, so that both read the same directories by the same code.
//
// Linux first, the door left open (ADR 0003): on a system this does not know the
// executable directory is empty, and the shipped patterns are then not found,
// which the catalogue says rather than hides.

#include <filesystem>

namespace subedit::platform {

/// The directory the running executable is in, or empty when the system does not
/// say. Linux reads it from `/proc/self/exe`.
[[nodiscard]] std::filesystem::path executableDirectory();

/// Where the shipped patterns are installed, worked out from the executable —
/// never from a path frozen at build time, the rule `installedManualPath()`
/// follows and for the same reason: the configure prefix and the install prefix
/// are not the same. The build tree reproduces the layout (`CMakeLists.txt`), so
/// a binary run from it finds them the same way.
[[nodiscard]] std::filesystem::path installedPatternsPath();

/// Where a user drops patterns of her own, read from the real environment — the
/// one place allowed to, since `core::userPatternsPath` takes the two strings
/// rather than reading them. **A test never calls this to find a location**: it
/// is given one, and the harness moves `XDG_DATA_HOME` for every binary it runs.
[[nodiscard]] std::filesystem::path resolvedUserPatternsPath();

} // namespace subedit::platform
