#pragma once

// Moving every position of a file by a fixed amount.

#include <subedit/cli/exit_code.hpp>
#include <subedit/core/time/duration.hpp>

#include <string>
#include <vector>

namespace subedit::core {
class FileSystem;
}

namespace subedit::cli {

class Destination;
class Reporter;

/// Shifts every path by `by`, and says how it went.
///
/// **A shift that would take a position before the origin is refused**, naming
/// the subtitle that could not take it. The core allows such positions on
/// purpose — refusing them there would turn an editing operation into a special
/// case — but a file is another matter: a timestamp before the origin is
/// written `-00:00:01,000`, which neither SubRip nor WebVTT defines and no
/// player reads. Writing it would be worse than refusing.
[[nodiscard]] ExitCode shiftAll(subedit::core::FileSystem& files,
                                const std::vector<std::string>& paths,
                                subedit::core::Duration by,
                                const Destination& destination,
                                const Reporter& reporter);

/// Shifts every path back onto the frame grid its positions were written on.
///
/// The amount is **measured, not given**: a file whose positions sit on a grid
/// to within a constant has been shifted, and the deduction reads that constant
/// off the positions themselves. Each file therefore moves by its own amount,
/// which is why this cannot be `shiftAll` with a number worked out beforehand.
///
/// **A file on no grid is refused, and told why.** There is nothing to shift
/// onto, and a phase measured on noise would move the file by an arbitrary
/// amount — which is exactly the silent failure this whole phase exists to
/// avoid.
[[nodiscard]] ExitCode shiftOntoGridAll(subedit::core::FileSystem& files,
                                        const std::vector<std::string>& paths,
                                        const Destination& destination,
                                        const Reporter& reporter);

} // namespace subedit::cli
