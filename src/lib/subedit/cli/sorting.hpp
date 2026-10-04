#pragma once

// Putting the subtitles of a file back in the order of their start.

#include <subedit/cli/exit_code.hpp>
#include <subedit/core/model/encoding.hpp>

#include <optional>
#include <string>
#include <vector>

namespace subedit::core {
class FileSystem;
}

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

} // namespace subedit::cli
