#pragma once

#include <cstdint>
#include <functional>
#include <memory>
#include <string_view>

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

/// Which Qt platform to ask for, given what the session offers — nothing when
/// there is no reason to ask for one.
///
/// **The decidable half of `preferEmbeddablePlatform`**, out where a test can
/// walk it: reading the environment and writing to it are two lines that
/// cannot be run twice in one process, and the choice between them is the part
/// that can be wrong.
///
/// `chosen` is what `QT_QPA_PLATFORM` already says, `wayland` what
/// `WAYLAND_DISPLAY` says, `x11` what `DISPLAY` says. Empty means unset.
[[nodiscard]] std::string_view
platformFor(std::string_view chosen, std::string_view wayland, std::string_view x11);

/// Asks for a Qt platform whose windows libmpv can adopt, when the session
/// leaves the choice open.
///
/// **Must be called before the `QApplication` is built**, which is the only
/// moment Qt reads it — and the reason this is a free function called from the
/// entry point rather than something the window does.
///
/// A Wayland session makes Qt pick `wayland`, whose windows are not the kind
/// libmpv adopts; XWayland is almost always there beside it, and going through
/// it is what makes the film appear inside the window instead of nowhere. The
/// alternative was to leave it alone and tell the user to set the variable
/// themselves — which is a feature that does not work, described in a manual.
///
/// **A choice already made is never overridden.** Somebody who asks for
/// `wayland` gets `wayland`, and no picture, which is their business.
void preferEmbeddablePlatform();

} // namespace subedit::gui
