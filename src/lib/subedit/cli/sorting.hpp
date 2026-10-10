#pragma once

// Putting the subtitles of a file back in the order of their start.

#include <subedit/cli/exit_code.hpp>
#include <subedit/core/model/encoding.hpp>

#include <cstddef>
#include <optional>
#include <string>
#include <vector>

namespace subedit::core {
class FileSystem;
class Project;
class Session;
} // namespace subedit::core

namespace subedit::cli {

class Destination;
class Reporter;

/// Sorts the subtitles of every path by their start, and says how many moved.
///
/// **The core never sorts of itself** (ADR 0012): reading a file leaves its
/// order as it was, and `inspect` reports a break in it as an anomaly. This is the
/// gesture made on purpose, for a file or a batch whose order is broken.
///
/// **Stable**: two subtitles that start together keep the order the file gave
/// them. What is counted is the places that end up holding a different subtitle,
/// which is what the history of the window would call moved.
///
/// A file already in order is written all the same, and says so: a destination
/// given is a destination written.
[[nodiscard]] ExitCode sortAll(subedit::core::FileSystem& files,
                               const std::vector<std::string>& paths,
                               const std::optional<subedit::core::Encoding>& reading,
                               const Destination& destination,
                               const Reporter& reporter);

/// Puts the project of `session` in order of start, **through the history**, and
/// says how many subtitles changed place — nothing when it was already in order.
///
/// What `--sort` does on the commands that carry the option: the gesture of
/// `sort`, made on the way to something else.
[[nodiscard]] std::size_t sortedIn(subedit::core::Session& session);

/// The same, on a bare project that has no history: for a command that reads,
/// converts and writes without ever opening a session.
[[nodiscard]] std::size_t sortedInPlace(subedit::core::Project& project);

/// What `--sort` says about one file: how many subtitles moved, or that the
/// order was right. One line, at the first level, for every command that has
/// the option.
[[nodiscard]] std::string narrationOfSort(const std::string& path, std::size_t moved);

} // namespace subedit::cli
