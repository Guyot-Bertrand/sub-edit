#pragma once

// Re-timing a file mastered at one frame rate for playback at another.

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

/// Re-times every path from `input` to `output`, and says how it went.
///
/// ```
/// t′ = round( t × (input / output) )
/// ```
///
/// A film mastered at 23.976 and played at 25 frames per second runs faster, so
/// its subtitles come earlier; the other way round they come later. Naming the
/// direction is what keeps the two apart — applied the wrong way the file
/// drifts some seven seconds over a feature film, with nothing to say so until
/// someone plays it.
///
/// **Never through the frames.** The factor is an exact rational and the result
/// is rounded once, as ADR 0013 requires. Converting by way of the frame
/// numbers would quantise every position on the input grid and land up to half
/// a frame away — 21 ms, measured.
///
/// Total, unlike the transform: two frame rates always define a ratio, both
/// their terms being strictly positive and bounded. Whether the text the user
/// typed named a rate at all is the grammar's business, and settled before
/// anything is read.
[[nodiscard]] ExitCode convertFrameRateAll(subedit::core::FileSystem& files,
                                           const std::vector<std::string>& paths,
                                           subedit::core::FrameRate input,
                                           subedit::core::FrameRate output,
                                           const Destination& destination,
                                           const Reporter& reporter);

} // namespace subedit::cli
