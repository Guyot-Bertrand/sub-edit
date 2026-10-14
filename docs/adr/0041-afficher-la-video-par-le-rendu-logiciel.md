# 0041 — Display the video through libmpv's software rendering

**Date:** 2026-10-06
**Status:** accepted

**Complements [0020](0020-libmpv-pour-le-lecteur-integre.md)**: libmpv remains the engine, and nothing 0020 weighed is called into question. What changes is **the way the image reaches the window**, which 0020 had left "settled at implementation" — #173 had chosen the native window adopted through `wid`. Decided by the user while scoping phase 14 ([#190](https://github.com/Guyot-Bertrand/sub-edit/issues/190), [#609](https://github.com/Guyot-Bertrand/sub-edit/issues/609)).

## Context

The path of #173 cost more than it promised, and the measure of it is in #190:

- **adopting a native window is an X11 mechanism** — libmpv's header says so —: `subedit-gui` asks for `xcb` when the session allows it, and refuses to build a player on any other Qt platform;
- the context must be named, the window must be on screen before being adopted, and seven lines of coverage can only be walked with an X11 session;
- **the image is not in the Qt window**: what Qt photographs does not contain it, and without a screen there is no image at all. The manual cannot show it, a test cannot read it, and the overlay phase 14 adds (the timecode) cannot be drawn on it by Qt.

Phase 14 reopens this code anyway — frame stepping, the overlay, the playback bar —: this is the moment, and the only cheap one.

**libmpv offers two rendering APIs**: OpenGL, which its documentation recommends, and **software** (`MPV_RENDER_API_TYPE_SW`), which draws into a buffer the caller provides. The latter needs no graphics context, no window, and no X11.

**Measured at the start of this scoping**, on a development machine, with an H.264 1920×1080 video at 25 frames per second, without hardware decoding, `BLOCK_FOR_TARGET_TIME` at zero: rendering costs **2.2 to 3.4 ms per frame on average** for a buffer of 640×360 to 1920×1080, **20 to 40 ms at worst** on a loaded machine (load 9 on 12 cores), and **playback keeps up with real time** — 123 to 125 frames in five seconds, none dropped. What was not measured: 4K, HDR, a heavy codec (HEVC, AV1) and a modest machine.

## Decision

**The image is rendered by libmpv's software rendering API, into a buffer that `VideoSurface` — a Qt widget — paints.** `MpvPlayer` creates a render context (`vo=libmpv`); the update callback, called from an mpv thread, **hands over to the window's thread**, which renders without waiting for the target time and requests a paint. The buffer is the size of the widget **in physical pixels**; libmpv preserves the aspect ratio. The adopted native window, the imposed `xcb` context and `PlayerFactory`'s window identifier **go away**.

The `VideoPlayer` interface does not change for this: it still knows neither libmpv nor Qt, and the image does not go through it. **The surface is `subedit_gui`'s business.**

## Alternatives dismissed

- **Staying with the native window (`wid`).** The least work, and nothing breaks. Dismissed: it leaves the image outside Qt — hence no screenshot for the manual, no reading of the image by a test, no overlay drawn by Qt — and makes `subedit-gui` depend on XWayland forever.
- **The OpenGL rendering API.** The recommended one, which allows zero-copy hardware decoding. Dismissed **for this phase**, not forever: it requires an OpenGL context managed by hand, render callbacks wired onto the window's thread, and **a test surface that does not get any easier** — a headless GL context is one more environment to maintain on everyone's machine. The measured software cost does not justify it at the scale of a subtitle editor.
- **Qt Multimedia.** Dismissed by 0020 on fine-grained timing, and nothing has changed.

## Consequences

**The image is in the window**, hence: photographable by `make manual`; readable by a headless test, which compares the displayed image to the numbered frame of [#610](https://github.com/Guyot-Bertrand/sub-edit/issues/610); overlayable by Qt (the timecode); and **the same under Wayland, X11 and elsewhere**. The platform guard and its coverage lines go away.

**Everything goes through the processor.** Zero-copy hardware decoding is not available with this rendering — `hwdec-copy`, if suitable, copies each frame — and scaling as well as colour conversions are those of software libmpv. A frame costs one copy of the buffer into a `QImage`: 3.7 MB at 720p, 25 times per second, with no measurable consequence.

**One more thread to cross**: the callback comes from an mpv thread and touches nothing; everything else — the render request, the painting, the orders to the player — stays on the window's thread, which `VideoPlayer` already promises ("a single thread").

**A modest cost to undo**: the surface is a widget behind `MpvPlayer`. Switching to OpenGL later would replace the buffer with a context, without touching the interface or the callers.

**Trigger for reconsidering:** a user whose playback drops frames at an ordinary size and codec — the end-of-phase review measures on what is actually watched —, or the need for a format software rendering handles badly (HDR, 10-bit). The answer is then the OpenGL API **behind the same surface**.

## Measured at the end-of-phase-14 review ([#623](https://github.com/Guyot-Bertrand/sub-edit/issues/623))

**The decision holds for what it measured, and a limit is now quantified.** Three ten-second films, 25 frames per second, with noise added so that they weigh like real films, **decoded under the gate's two-core limit** (`limit-cores.sh`):

| Film | Bit rate | Decoding 10 s of film | Speed |
| :--- | -------: | --------------------: | ----: |
| H.264 1080p | 69 Mb/s | 4.0 s | 2.5 × real time |
| HEVC 4K, 8-bit | 117 Mb/s | 16.1 s | **0.62 ×** |
| HEVC 4K, 10-bit | 70 Mb/s | 10.9 s | **0.92 ×** |
| HEVC 4K, 8-bit, VAAPI (ffmpeg) | 117 Mb/s | 4.7 s | 2.1 × |

- **H.264 1080p keeps up with real time with margin to spare**, as the ADR had measured. The rendering itself — scaling and conversion into the buffer — does not depend on the film.
- **Heavy HEVC 4K does not keep up on two cores**, in 8-bit as in 10-bit. A frame step, which is an exact seek and decodes from the keyframe, costs from 0.4 s (25 frames from the keyframe) to 5 s (225 frames) on these films, which have only one keyframe; a real film has one every two to ten seconds.
- **This machine's hardware decoding** brings 4K to more than twice real time — measured by ffmpeg, not through libmpv. The trigger above is reached for anyone who watches 4K on a modest machine, and **the answer is `hwdec=auto-copy` under the same surface, not the OpenGL API**: [#647](https://github.com/Guyot-Bertrand/sub-edit/issues/647).
- **What is not measured**: hardware decoding through libmpv, continuous playback with a count of dropped frames, HDR, AV1, a machine without a graphics card. The films are not versioned; the `seeking and stepping on a real film` bench takes them through `SUBEDIT_BENCH_REAL_FILM` and abstains without it.

## Addendum — hardware decoding with copy ([#647](https://github.com/Guyot-Bertrand/sub-edit/issues/647))

**`MpvPlayer` asks for `hwdec=auto-copy`.** The card decodes, the frame is copied back into memory, and software rendering draws it as before: the surface, the headless tests and the manual's screenshots do not change. On a machine without a card, mpv decodes on the processor without saying anything. A `Preferences…` setting, `video.hardware-decoding` (on by default), turns it off, immediately and for the following films.

**Measured through libmpv**, under the two-core limit (`limit-cores.sh`), on the ten-second films of the end-of-phase-14 review, 100 samples per case, machine not idle (load 2 to 3). Seeking to a frame then rendering it, average, with the card then without:

| Film | 25 frames from the keyframe | 100 frames | 225 frames |
| :--- | --------------------------: | ---------: | ---------: |
| H.264 1080p | 0.10 s vs 0.36 s | 0.47 s vs 1.55 s | 1.11 s vs 3.49 s |
| HEVC 4K, 8-bit | 0.83 s vs 1.59 s | 2.66 s vs 4.66 s | 4.79 s vs 5.32 s |
| HEVC 4K, 10-bit | 0.72 s vs 0.98 s | 1.74 s vs 2.94 s | 3.83 s vs 5.15 s |

A frame step gives the same orders of magnitude (same exact seek).

- **The gain is real and it is uneven.** It ranges from 1.1 × (4K 8-bit, far from the keyframe) to 3.2 × (1080p). It is **smaller than the 2.1 × of decoding alone measured by ffmpeg**: copying the frame into memory, colour conversion and scaling remain on the processor, and at 4K they weigh.
- **Heavy 4K does not become smooth.** At 225 frames from the keyframe, a step still costs nearly four seconds. What the copy does not solve — and what only the zero-copy OpenGL API would solve, the ADR's trigger remaining what it is — is the price of exact seeking, which decodes from the keyframe.
- **Exactness does not move**: the tests on numbered videos pass with and without (`seeking and stepping are exact with and without hardware decoding`). On a machine without a card, both series are software and prove only that; with a card, they prove that the copied-back frame is the one requested.
- **What is still not measured**: continuous playback with a count of dropped frames, HDR, AV1, a machine without a card.
