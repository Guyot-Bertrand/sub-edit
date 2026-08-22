#pragma once

#include <subedit/core/io/file_system.hpp>

#include <filesystem>
#include <optional>
#include <string_view>

namespace subedit::core {

/// Where `name` was found in `searchPath`, or nothing if it is not there.
///
/// The search path is a parameter and not the `PATH` of the process, which is
/// the whole point: **the branch where an external program is missing has to be
/// reachable from a test**, and it cannot be if answering the question means
/// asking the machine the test runs on. A branch only a user without `ffmpeg`
/// ever walks is a branch written once, never read again, and wrong the day
/// somebody meets it.
///
/// Entries are separated by `:` and read left to right, first match wins — the
/// rule of the shells, so that what the user sees agrees with what `command -v`
/// tells them.
///
/// Two deliberate departures from that rule:
///
/// - **an empty entry is skipped.** POSIX reads it as the current directory,
///   which turns a trailing colon into a program taken from wherever the user
///   happens to stand. Nothing here needs it, and it is a well-known way to run
///   the wrong binary.
/// - **a name carrying a separator is a path**, taken as given and looked up
///   nowhere. This is what makes a configured player command able to name an
///   executable that is not installed system-wide.
[[nodiscard]] std::optional<std::filesystem::path>
findExecutable(const FileSystem& files, std::string_view name, std::string_view searchPath);

} // namespace subedit::core
