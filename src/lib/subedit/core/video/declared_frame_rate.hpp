#pragma once

#include <subedit/core/io/file_system.hpp>
#include <subedit/core/time/frame_rate.hpp>

#include <filesystem>
#include <optional>
#include <string_view>

namespace subedit::core {

/// Reads what `ffprobe` writes for `r_frame_rate`, or nothing.
///
/// The form is `24000/1001` — **an exact rational, never a decimal number**,
/// and that is the whole reason `ffprobe` is kept for this one answer. libmpv
/// knows the same thing as `container-fps`, and answers 23.976025; a
/// `FrameRate` built from that would not be the rate the film was timed at.
/// See [ADR 0020](../../../../../docs/adr/0020-libmpv-pour-le-lecteur-integre.md).
///
/// Exposed beside the reading below because this is where the odd cases are,
/// and they cannot be reached through a real `ffprobe`: nothing makes it write
/// `N/A`, or a rate with no denominator, on demand. What it does write when it
/// has nothing to say — not one line — reads as nothing here.
[[nodiscard]] std::optional<FrameRate> parseDeclaredFrameRate(std::string_view text);

/// The frame rate the container of `video` declares, or nothing.
///
/// **Nothing is an ordinary answer, and never an error.** Three things can
/// happen and only one of them is a rate:
///
/// | What happens | What comes back |
/// | :----------- | :-------------- |
/// | `ffprobe` is not on `searchPath` | nothing |
/// | it answers, and the file declares no video stream | nothing |
/// | it answers a rate | the exact rational |
///
/// The absence of `ffprobe` is not a degraded mode: it is the ordinary state
/// of a machine that never installed `ffmpeg`. What it would have proposed is
/// simply not proposed, and no operation refuses itself for want of it — the
/// frame rate a conversion needs comes from the user, and this proposes one
/// when it can.
///
/// `searchPath` is a parameter rather than the `PATH` of the process, for the
/// reason `findExecutable` gives: the branch where the program is missing has
/// to be reachable from a test.
[[nodiscard]] std::optional<FrameRate> readDeclaredFrameRate(const FileSystem& files,
                                                             std::string_view searchPath,
                                                             const std::filesystem::path& video);

} // namespace subedit::core
