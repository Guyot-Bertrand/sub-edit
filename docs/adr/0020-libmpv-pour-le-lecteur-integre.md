# 0020 — libmpv for the built-in player

**Date:** 2026-08-22
**Status:** accepted — **complemented by [0041](0041-afficher-la-video-par-le-rendu-logiciel.md)** for the way the picture reaches the window: the native window adopted through `wid`, which this decision retained, has given way to libmpv's software rendering, painted by a Qt widget.

## Context

Phase 6 was scoped around an **external player preview**: write a temporary
file, launch mpv, MPlayer or VLC positioned at the current subtitle, as
`aeidon/agents/preview.py` does. The built-in player was in phase 14, where the
roadmap had filed it as "the most expensive part of the project".

That scoping was revisited along the way: **a player is needed in the
window**. Feature parity in fact required it — Gaupol has both, and
`gaupol/player.py` is an embedded GStreamer player, distinct from the external
preview. The roadmap had kept the second and forgotten the first.

The choice of backend had been identified from the start as an upcoming ADR —
the index carried it in so many words: "Video backend — libmpv or
QtMultimedia. Phase 14." It is due now.

## Decision

**libmpv**, the `libmpv-dev` package, version 0.37 in Ubuntu 24.04.

The deciding criterion is not phase 6, which both can serve: it is **what
remains after it**. Phase 14 now keeps only fine timing — frame-by-frame
stepping, setting a mark from the current position. Yet that is precisely what
a backend can or cannot do, and it cannot be made up for by code on top.

| What phase 14 will require | libmpv | Qt Multimedia |
| :------------------------- | :----- | :------------ |
| seeking to an exact position | `seek <t> absolute+exact` | approximate, backend-dependent |
| stepping one frame forward and back | `frame-step`, `frame-back-step` | no API |
| the container's frame rate | `container-fps` property | not exposed |

**Choosing Qt Multimedia would mean choosing to redo this work** the day phase
14 arrives, or to add a second backend to it — hence having two video engines
in a project that wanted only one.

Two lighter arguments point the same way: mpv is the player Gaupol recommends
first, hence the behaviour the user already knows; and mpv exists on Windows
and macOS, so ADR 0003 — "at comparable quality, the portable option is
systematically chosen" — loses nothing.

## Alternatives dismissed

- **Qt Multimedia.** Already installed since #117, and `QVideoWidget` fits into
  the window without glue: that is a real advantage, and it does not weigh
  enough. Dismissed on fine timing, as said above. Ubuntu 24.04 provides Qt
  6.4.2 with both plugins, `libffmpegmediaplugin` and
  `libgstreamermediaplugin` — so it is not the decoding engine that is
  missing, it is the API.

- **GStreamer directly**, like Gaupol. Dismissed: it means going one level
  below what Qt Multimedia already offers, to inherit the same API limits on
  timing, and writing the pipeline oneself.

- **Keeping the external player and waiting**, delivering phase 6 as it was
  scoped. Dismissed by the request, and rightly so: the external player was
  only a substitute put in place when the built-in one seemed too expensive.
  Keeping it as well would cost two surfaces, two manual sections and two test
  suites for a gesture the built-in player renders better.

## Consequences

**One more system dependency**, `libmpv-dev`, to bring into
`setup-toolchain.sh` and hence into the CI package cache. The trap is known and
already paid twice — Qt in #117, `ffmpeg` in #162.

**A seam to write by hand.** libmpv exposes a C API: a handle that one creates,
queries through properties and drives through commands, and that must be
destroyed. It is a resource owner in the sense of the project's principle 2,
and it will deserve its class, as `SpawnActions` deserved its own.

**A display question settled at implementation, and its price.** The choice
was between rendering in a native window that libmpv adopts (`wid` property)
and rendering in an OpenGL context that Qt provides. #173 retained the first,
as the shortest. This paragraph announced that the decision would "not be
expensive to undo"; it turned out to be expensive to **keep**, and the finding
is recorded here because it changes what is known, not what was decided.

What #176 measured while putting it in place:

- **adopting a native window is an X11 mechanism.** libmpv's header says so —
  "This works on X11, win32, and OSX only" — and recommends the render API.
  `subedit-gui` therefore asks for `xcb` when the session leaves the choice
  open, and **refuses to build a player** on any other Qt platform;
- **the context must be named.** Left to probe on a machine where
  `WAYLAND_DISPLAY` is set, mpv chooses Wayland, where `wid` means nothing, and
  opens a window next to ours;
- **the window must be on screen before being adopted**, otherwise mpv never
  maps its own.

Three lines of code, and three defects that only use revealed.

**What this leaves open, and for when.** The render API works under Wayland,
and it is the one libmpv recommends. Phase 14 reopens this code anyway —
frame-by-frame stepping, timecode overlay — and that is where the change would
cost least. The decision is not taken here: it requires weighing a
hand-maintained OpenGL context against a dependency on XWayland, and that is
done when scoping phase 14.

**The trigger that would force it**, on the other hand, is clear: a session
with no X server at all. `subedit-gui` would play no film there, and says so.

**The displayed subtitle does not come from a file.** libmpv can load an
external subtitle file, and that would be the wrong choice for an editor: it
would have to be rewritten at every keystroke. The current line is drawn from
the model, which is already what the phase 5 table displays. The temporary file
of the old scoping disappears with the external player.

**`ffprobe` stays**, and for a single reason: it returns the frame rate as
**an exact rational** — `24000/1001` — whereas libmpv's `container-fps` is a
float. `FrameRate` has been a rational since phase 1, and converting to it by
approximation would lose what that choice protected. The duration, on the
other hand, comes from the player, which already has it: two sources for the
same data without necessity would be one too many.

The trigger that would reopen this decision: libmpv missing from a targeted
distribution, or a Qt Multimedia API that would gain frame-level stepping.
Neither is in sight.

Undoing this decision costs what a backend costs: the seam is behind an
interface in the sense of principle 3 — "video player" has been named there as
a point of variation since the foundations, precisely for this case.
