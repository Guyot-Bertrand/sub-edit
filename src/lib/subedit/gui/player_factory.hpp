#pragma once

#include <subedit/core/time/frame_rate.hpp>

#include <filesystem>
#include <functional>
#include <memory>
#include <optional>
#include <string_view>

namespace subedit::core {
class FileSystem;
class VideoPlayer;
} // namespace subedit::core

namespace subedit::gui {

/// How a window gets a player.
///
/// **A factory rather than a player**, for the reason `Prompts` is a seam: whoever builds
/// the window decides what comes out. `subedit-gui` passes `mpvPlayers`; a test passes a
/// factory of its own, and drives a window through every case a film can put it in
/// without ever decoding one. It is asked the first time a film needs one, so that a
/// window nobody shows a film to never starts libmpv.
///
/// **Answering nothing is an answer**, and so is giving no factory at all: a program
/// libmpv would not give a player to still edits subtitles, and a window that is never
/// shown a film never asks for one.
using PlayerFactory = std::function<std::unique_ptr<core::VideoPlayer>()>;

/// The factory `subedit-gui` hands its window: libmpv, behind the seam.
///
/// **Here and not in `main.cpp`**, which is the rule
/// `check-architecture.sh` holds: everything that is not wiring lives in a
/// library, so that it can be looked at by something other than a human
/// running the program. It said so out loud — the entry point went eight lines
/// over its budget the moment this was written there.
[[nodiscard]] PlayerFactory mpvPlayers();

/// How the window asks a film what frame rate it declares.
///
/// **The second seam of the video**, beside `PlayerFactory`, and separate from
/// it on purpose: the rate comes from `ffprobe` and the duration from the
/// player — decision D7. One is an external program that may not be installed,
/// the other a library the project links.
///
/// Answering nothing is the ordinary answer, and it is what a window with no
/// reader at all behaves like: nothing is proposed, and every operation goes
/// on working. A test passes a reader that answers what the scenario needs,
/// without an `ffprobe` anywhere near it.
using FrameRateReader =
    std::function<std::optional<core::FrameRate>(const std::filesystem::path& video)>;

/// The reader `subedit-gui` hands its window: `ffprobe`, found on the `PATH`.
///
/// `files` must outlive the reader — it is how the executable is looked for,
/// and the reason the search is not asked of the process directly: a branch
/// only a machine without `ffmpeg` walks has to be reachable from a test.
[[nodiscard]] FrameRateReader declaredFrameRates(const core::FileSystem& files);

} // namespace subedit::gui
