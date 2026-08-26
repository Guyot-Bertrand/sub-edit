#pragma once

// Laying every position of a file back onto the frames of a rate.

#include <subedit/cli/exit_code.hpp>
#include <subedit/core/time/frame_rate.hpp>

#include <string>
#include <vector>

namespace subedit::core {
class FileSystem;
}

namespace subedit::cli {

class Destination;
class Reporter;

/// Aligns every path on `rate`, and says how it went.
///
/// **This is not `framerate`, and confusing the two is silent.** A conversion
/// re-times a file whose minutage is wrong, scaling every position and dragging
/// the file by seconds over a feature film. An alignment re-times nothing: it
/// takes a file whose minutage is already right and whose *grid* is wrong, and
/// moves each position by half a frame at most.
///
/// The report says how many positions moved and by how much at most, which is
/// what tells a user in one line which of the two they just ran.
[[nodiscard]] ExitCode alignAll(subedit::core::FileSystem& files,
                                const std::vector<std::string>& paths,
                                subedit::core::FrameRate rate,
                                const Destination& destination,
                                const Reporter& reporter);

} // namespace subedit::cli
