#pragma once

#include <cstdint>
#include <functional>
#include <memory>

namespace subedit::core {
class VideoPlayer;
} // namespace subedit::core

namespace subedit::gui {

/// How a window gets a player for the surface it has made native.
///
/// **A factory rather than a player**, and `wid` is the reason: libmpv reads
/// the window it draws into while it initialises, and ignores it afterwards —
/// measured. A player therefore cannot exist before the window that gives it
/// one, which rules out handing one in the way `Prompts` is handed in.
///
/// What it buys is the same thing `Prompts` buys: whoever builds the window
/// decides what comes out. `subedit-gui` passes `mpvPlayers`; a test passes a
/// factory of its own, and drives a window through every case a film can put
/// it in without ever decoding one.
///
/// **Answering nothing is an answer**, and so is giving no factory at all: a
/// program libmpv would not give a player to still edits subtitles, and a
/// window that is never shown a film never asks for one.
using PlayerFactory = std::function<std::unique_ptr<core::VideoPlayer>(std::uintptr_t surface)>;

/// The factory `subedit-gui` hands its window: libmpv, behind the seam.
///
/// **Here and not in `main.cpp`**, which is the rule
/// `check-architecture.sh` holds: everything that is not wiring lives in a
/// library, so that it can be looked at by something other than a human
/// running the program. It said so out loud — the entry point went eight lines
/// over its budget the moment this was written there.
[[nodiscard]] PlayerFactory mpvPlayers();

} // namespace subedit::gui
