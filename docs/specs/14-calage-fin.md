# Phase 14 — Fine timing

Framing of issue [#609](https://github.com/Guyot-Bertrand/sub-edit/issues/609),
after the initialisation [#608](https://github.com/Guyot-Bertrand/sub-edit/issues/608).

## What the roadmap promises, and what is already there

Three families. **Driving playback** — position, jumps, previous and next subtitle,
start and end of the selection, playing the selection, volume. **Timing itself**
— setting the start or the end from the video position, inserting at that position, stepping
forward and back frame by frame, nudging by small increments. **What is displayed** — the timecode,
the audio track, the line without its raw markup. Plus five deferrals: the frame editing mode,
the shifted translation, the pair of a conversion at the wrong frame rate, the centred following
of the table, and the display path of #190.

**The hard point is positioning accuracy**: a `seek` exact to the frame.
**It is no longer an unknown**: the initialisation measured it (see "What the initialisation
measured"), and what remains is to **say** it — what "exact" means — and to **prove** it
with an oracle that does not rely on the player.

**Gaupol has part of this outline, and not the other.** Its `Video` and `Edit` actions carry
the driving and the five marking gestures; **it has neither frame-by-frame stepping, nor the one-frame nudge,
nor the detection of a shift**. The phase is therefore half iso-functional, half gain.

| What is already there | Where |
| :-------------------- | :---- |
| **the built-in player**: libmpv, `VideoPlayer` (`open`, `duration`, `position`, exact `seek`, `play`, `pause`, `showSubtitle`, `isPlaying`), the `FakeVideoPlayer` double, `VideoPane` (the shared player, the picture, the 100 ms follower) | phase 6, [ADR 0020](../adr/0020-libmpv-pour-le-lecteur-integre.md) |
| associating a film, opening it, playing and stopping (`Ctrl+P`), the line drawn from the model, the current row that follows, **the warning when an operation goes past the end of the film** | phase 6 |
| the frame rate the video declares, and the **deduction of a grid** from the positions, with its verdict `Clean`, `Partial`, `Silent` | phases 6 and 16 |
| the commands the timing gestures replay: `SetPositionCommand` (one edge of a subtitle), `InsertCommand`, `ShiftCommand`, `ConvertFrameRateCommand` | phases 2 and 10 |
| opening a translation, **attaching it by position or by number** and its count (attached, born, without a subtitle, out of order) | phase 11 |
| the markup pivot, which translates a text from one format to another | [ADR 0031](../adr/0031-pivot-de-balises-a-la-conversion.md) |
| the strong type `Frame`, and positions in whole milliseconds | ADR [0011](../adr/0011-numero-d-image-en-type-fort.md), [0006](../adr/0006-positions-en-millisecondes.md) |
| **the window's settings**, tolerant option by option | [ADR 0022](../adr/0022-configuration-au-noyau-et-tolerance-par-option.md) |

**What is not there.** No driving beyond play and pause; no marking gesture;
no frame-by-frame stepping; **no picture in the Qt window** (see D2); no notion of what
"the position" means inside a frame.

## What the initialisation measured

Measurements of [#608](https://github.com/Guyot-Bertrand/sub-edit/issues/608), libmpv without a display, on a
development machine. **Observations, not a benchmark**: #610 and #611 make them replayable.

- **Exact seeking is exact.** On a video in which every frame carries its number and whose keyframe
  is unique, seven positions from 0 to 249 render the requested frame, 720p included. The oracle is
  `screenshot-raw`, which renders the real frame without a display.
- **mpv places the `seek` on the frame nearest to the position**, not on the frame displayed at that instant.
  Inside the interval of a frame, the first half renders that frame, the second **the next one**
  (at 23.976: frame 10 occupies 417.08 to 458.79 ms, and 437 ms renders frame 11).
- **After a `seek`, `time-pos` is the start of the displayed frame**, not the requested position: 437 ms requested,
  458.8 ms returned. **`position()` is therefore always the start of a frame**, to within rounding to the millisecond.
- **In whole milliseconds, every frame starts right.** The 24 frames tried at 23.976 render the
  right frame, whether the millisecond is rounded up or down: the resolution of `Timestamp` is
  not the defect.
- **The cost of a jump grows with the distance to the keyframe** (≈ 7 ms at 0, ≈ 100 ms at 249 frames, 720p), **and the step
  back costs no more than the step forward** (≈ 40 ms each): the hypothesis "going back is much more
  expensive" does not hold for this codec. #611 replays it on a film-sized video — see below.
- **`frame-step` emits no event**: there is no knowing when the frame is displayed. #614 concludes that **the step is
  a `seek` of a whole number of frames**, which waits like every other command; that is what the benchmark of #611
  measures, and it checks the frame obtained with the oracle of #610.
- **libmpv's software rendering keeps up with real time**: 2.2 to 3.4 ms per frame for 1080p H.264, into a
  buffer from 640×360 to 1920×1080 ([ADR 0041](../adr/0041-afficher-la-video-par-le-rendu-logiciel.md)).
- **The `mpeg4` encoder ignores `-g`** when the content changes on every frame: a keyframe every 32 frames
  without `-keyint_min 250 -sc_threshold 1000000000 -bf 0`.
- **Following the table polls the player every 100 ms** (`kFollowIntervalMs`); the tests drive the follower
  by hand, and only one case pays for the clock.
- **The private corpus contains no video**: no measurement on a real film is made here, and that one is
  for the user to make with their own.

### What #611 measured

`make bench` builds, in the build tree, **a 1280×720 video, 250 frames, a single keyframe**
(`video-fixtures.sh --film`) and hands it to the benchmark; without ffmpeg, the benchmark says so and abstains. Each gesture is checked
against the number the frame carries before it is timed. Observations from a development machine, `mpeg4`,
relatively quiet — **the record in the log is the one that counts**:

| Gesture | Target frame | Mean cost |
| :------ | -----------: | --------: |
| `seek` | 0 (the keyframe) | ≈ 8 ms |
| `seek` | 100 | ≈ 45 ms |
| `seek` | 249 | ≈ 107 ms |
| step forward | 100 | ≈ 51 ms |
| step forward | 249 | ≈ 106 ms |
| step back | 99 | ≈ 46 ms |
| step back | 248 | ≈ 95 ms |

- **The step back costs no more than the step forward**, at the same distance from the keyframe: the hypothesis "going back is
  much more expensive" is ruled out for this codec, and **the cost depends only on the distance to the keyframe**, not on
  the gesture. A real film (H.264, HEVC, 1080p) may say otherwise; the measurement on their own is the user's.
- **What this changes for the gesture (D6)**: at 25 frames per second, a frame lasts 40 ms, and a held key repeats
  faster than 100 ms. **Far from the keyframe, the step therefore does not keep up with the repetition**: the repetition must not
  pile up — that is already the rule of the gesture, "one step at a time" — and #618 holds it, by keeping only the last step
  requested while a step is running. The cost of the last frame of a video with a single keyframe is the worst case.
- **The benchmark found a defect of #613**: with `vo=libmpv`, the output waits for each frame to be rendered, up to
  200 ms, and a `seek` that waited on the window's thread stalled for 400 ms whatever the distance. The player
  now takes the frames itself while it waits; a case of `mpv_player_test.cpp` holds it.

## D1 — The scope: what Gaupol does, what it does not, and what makes sense here

Recounted **item by item** against `gaupol/actions/video.py`, `audio.py` and `edit.py`.

| Item | Gaupol | Here | Decision |
| :--- | :----- | :--- | :------- |
| position (slider) | `Gtk.Scale` of the player bar | **kept** | D5 |
| step back, step forward | `Seek Backward`, `Seek Forward` — `video_player.seek_length`, **30 s** | **kept**, adjustable step | D5, D12 |
| previous, next subtitle | `Seek Previous`, `Seek Next` | **kept** | D5 |
| start, end of the selection | `Seek Selection Start`, `Seek Selection End` — `context_length`, **1 s** of lead | **kept** | D5, D12 |
| play the selection | `Play Selection` | **kept**, up to its end | D5 |
| volume | `Volume Up`, `Volume Down` | **kept**, remembered from one session to the next | D5, D12 |
| audio track | `Set Audio Language` (a radio choice) | **kept** | D5 |
| set start, end from the video position | `Set Start / End from Video Position` | **kept** | D6 |
| insert at the video position | `Insert Subtitle at Video Position` | **kept** | D6 |
| select the neighbour from the position | `Select Previous / Next from Video Position` | **kept** | D6 |
| **frame by frame** | **none** | **kept** — the reason libmpv was chosen | D3, D6 |
| **nudge an edge by one frame** | **none** | **kept** | D6 |
| timecode overlay | `time_alpha`, `time_background` | **kept**, drawn by Qt | D9 |
| the line without raw markup | rendered by GStreamer | **kept** ([#408](https://github.com/Guyot-Bertrand/sub-edit/issues/408)) | D9 |
| positions in frames | `Frames` mode | **kept** | D8 |
| **recognise a shifted translation** | **none** | **kept** | D10 |
| **find the pair of a wrong conversion** | **none** | **kept** ([#386](https://github.com/Guyot-Bertrand/sub-edit/issues/386)) | D11 |
| automatic playback on opening (`autoplay`) | setting | **set aside** | nothing asks for it; trigger: a request |
| style of the line (font, colour, position, background, transparency) | seven `subtitle_*` settings | **set aside** | the line keeps the style phase 6 laid down; trigger: a request, and software rendering would make it possible to honour them |
| no subcommand | — | **set aside** | the player belongs to the window; an invocation has neither picture nor position |

**Four gains over Gaupol**, in the right-hand column: frame-by-frame stepping, the one-frame nudge, and the
two detections.

## D2 — The display path: libmpv's software rendering

**Decided by the user on 2026-10-06; [ADR 0041](../adr/0041-afficher-la-video-par-le-rendu-logiciel.md)
carries it.** The picture is drawn by libmpv's **software** rendering API into a buffer that `VideoSurface` — a
Qt widget — paints. The native window adopted through `wid` disappears, along with the platform guard and its seven lines
of coverage. **#190 is settled by this decision**, and the phase implements it.

**What this changes for the rest of the phase**: the picture can be screenshotted and read without a display, so **the manual
of timing shows the picture**, and a test compares the displayed picture with the numbered frame of #610; the timecode is drawn
**by Qt**, on the surface, without asking the player to do it. What this does not change: `VideoPlayer` still knows
neither libmpv nor Qt.

## D3 — The player interface gains five commands, and no notification

`VideoPlayer` gains:

- **`stepFrames(int)`**, positive forward, negative backward;
- **`volume()` and `setVolume(int)`**, from 0 to 100;
- **`audioTracks()` and `selectAudioTrack(id)`**: the identifier, the language, the title;
- **`playUntil(Timestamp)`**: play and stop at a position, to the frame. `Play Selection` needs it, and
  a 100 ms follower that stopped playback would let it overrun by up to three frames.

**No position notification.** The interface says "a single thread" and "waits", because the core has no
event loop; a notification would require one. **Following remains a poll**, that of `VideoPane`, which the
tests drive by hand: that is what keeps them away from a clock, and only one case pays for the timer. **A mark or
a step shows at once**, without waiting for the next tick — as `VideoPane` already does for a selection.

The double follows, without a clock: it keeps its position by hand, and gains the same commands.

## D4 — "The position", "the frame", "exact"

**Stated, because it was wrong in a comment** (`seek`: "the frame at its own start").

- **`seek(t)` places playback on the frame nearest to `t`.** Inside a frame, the first half renders
  that frame, the second the next one. That is what mpv does, and what is promised; writing it in the header is
  an issue ([#614](https://github.com/Guyot-Bertrand/sub-edit/issues/614)).
- **`position()` is the start of the displayed frame**, rounded to the millisecond. After `seek(437)`, it is 459: the
  start of frame 11. **A mark set from the position is therefore always the start of a frame**, and going back to it
  renders the same frame — a round trip that can be proved.
- **A subtitle that starts in the middle of a frame shows, at its start, the nearest frame**, sometimes the next one.
  This is not an error of the player: a frame has no middle a subtitle could aim at, and timing to the
  mark, for its part, is not fooled by it.
- **The resolution of `Timestamp` is not at fault**: every frame at 23.976 starts right in whole
  milliseconds. No fractional representation is introduced.
- **The proof is #610**, with an oracle that reads the displayed frame and not the player's counters.

## D5 — Driving

In the `Video` menu, **Gaupol's gestures** — the same names, and their shortcuts as long as they do not clash with a
shortcut the window already has (the uniqueness check decides):

- **step back and forward** (30 s by default), **previous and next subtitle** (playback goes to their start),
  **start and end of the selection** (with the lead, 1 s by default, before the start), **play the selection** — which
  stops at the end of the last selected subtitle, through `playUntil`;
- **the volume**, through two gestures and a slider;
- **an `Audio` submenu**, which lists the tracks and marks the one playing.

**A playback bar under the picture**: the position slider, the position and the duration, play/pause **flanked by two step buttons** (step back, step forward by one frame — #618), the volume.
**The slider does not hand back control at every pixel**: seeking is rate-limited, and **the last position
requested is always the one reached**.

**Audio tracks are not remembered** from one video to the next: a track number belongs to its file.
**The gestures are disabled without a video**, as playback is today.

## D6 — Timing

**Gaupol's five marking gestures, replayed on commands that exist.** Setting the start or the end of the
selected subtitle from the video position: `SetPositionCommand`. Inserting at the position: `InsertCommand`.
Selecting the neighbour: a selection move. **The same rules and the same messages as typing into a
cell** — strict order, an end before its start — because a gesture with its own rules would say something different
from the cell for the same value. **One history entry per gesture.**

**The step buttons of the bar trigger the menu actions**: same step, same disabling without a film, same tooltip (with the shortcut), and a held button chains the steps, one at a time.

**The shortcuts** (#617, #618): `Alt+Left` and `Alt+Right` for the step; `Alt+Q`, `Alt+W` for the start earlier, later, `Alt+Shift+Q`, `Alt+Shift+W` for the end — Gaupol's `E` is the letter of `Edit`; for the marks, `Alt+U` the start, `Alt+K` the end, `Alt+J` the insertion — Gaupol's `U`, `K` and `J`, which the window would take before a cell being edited — and `Alt+Up`, `Alt+Down` for the neighbour. Gaupol's `Ctrl+Y` is a *redo* on some systems.

**The gain over Gaupol: frame-by-frame stepping.**

- **Stepping forward and back**, two gestures that chain under a held key. **The step is a number
  of frames, and its minimum is one frame**: that is frame-by-frame stepping, which makes it possible to place a subtitle to the
  frame. The step is adjusted **upwards**, in a whole number of frames, for larger jumps (5, 10, 24…); it never
  goes below one, and it is never in milliseconds. One step at a time: repetition does not pile up.
- **Nudging the start or the end of the selected subtitle by one step**, for fine adjustment from the keyboard: an
  undoable command, which reports overlap and order as typing does. **The same step** as the player's, hence one
  frame by default.
- **The duration of a frame** comes, in this order, from **the frame rate the video declares**, from **that of the document
  when it is counted in frames**, from **the deduced grid** (phase 16) — **and without any of them, the gesture refuses and says so**,
  like `shift --to-grid`: picking a frame rate at random would move the whole file.

**The step is counted in frames, never in milliseconds**: a frame is what the player can show, and a thousandth
of a second cannot be seen. **What it is worth in time is the duration of one frame of this video** — 40 ms at 25 frames per
second, 41.7 ms at 23.976 —, so that a step of N frames is always N frames, whatever the frame rate. That is what
keeps the gesture right for fine timing, where an edge is placed on a frame and not on an instant. A setting in
milliseconds is a trigger, not a promise.

## D7 — Following in the table

Clarified by the user on 2026-09-30: **the table centres on the current subtitle**, which it does not do
— it brings it back into the visible area.

- **It centres when the row changes**, not at every tick: a row lasts seconds on average, a
  recentring at every 100 ms tick would be pointless scrolling.
- **Scrolling by hand suspends following** — the wheel, the scroll bar, a click in the table.
- **Following resumes on a player gesture**: playback started, a jump, a mark, a step — **or on demand**.
  **No timer**: what resumes following is a gesture, so a test drives it without a clock.
- **A button of the video strip**, checked when the table follows, shows the state and restores it.
- **Delivered (#619)**: the `Follow` button is at the right of the playback bar. Recentring costs 1.3 µs on a table of 4,000 rows (`make bench`, `centrer une ligne sur 4000 sous-titres`, painting excluded): it is paid at each row change, never at each tick.

## D8 — Positions in frames

**The frame editing mode** — deferred from phase 5, passed on again by phase 9. **A `View` setting**, a single one for
the window, which shows `Start` and `End` as frame numbers and accepts numbers on input. **The frame rate that
counts** is that of the document when it is counted in frames (MicroDVD carries numbers and no frame rate), otherwise
that of the video, otherwise the deduced grid; **without any of them, the setting is disabled and says why**. Toggling it redraws
columns: **it does not touch the model**, and what the file writes does not change. An entered number is converted
by exact scaling ([ADR 0013](../adr/0013-mise-a-l-echelle-exacte-des-positions.md)), rounded once.

**Delivered (#620)**: `View ▸ Positions in Frames`, a checkable entry whose tooltip states the frame rate retained and its source, or why it is disabled. Only `Start` and `End` change, `Duration` stays a time; the setting is not remembered (D12). The table model receives the frame rate and the project is not touched. **The playback bar receives it too**: its position and its duration follow the same setting; the clock drawn on the picture stays a timestamp.

## D9 — The line and the timecode

- **The line without raw markup** ([#408](https://github.com/Guyot-Bertrand/sub-edit/issues/408)): `showSubtitle`
  receives the text with the markup of its format. The **markup pivot** (ADR 0031) translates it into ASS tags — italic,
  bold, underline, colour —, **and a tag with no on-screen equivalent is removed, never drawn as
  is**. No second parser: the conversion's one serves.
- **The timecode** is drawn by Qt on the surface, **from `position()`**; it asks nothing of the player. Its
  style is the window's.

## D10 — Recognising a shifted translation

**Decided by the user on 2026-10-06: it belongs to this phase.** A translation timed two seconds too late
aligns with the wrong subtitles without anything distinguishing it from a correct attachment: phase 11 makes it
**visible** (the count), not **correctable**.

- **The core looks for a constant shift.** The gaps between the start of each line and that of the nearest
  subtitle give candidates, grouped to the ten milliseconds.
- **Each candidate is judged by what the attachment would say after it** — lines attached, born, without a subtitle,
  out of order —, **by the same algorithm as the opening**. The confidence is therefore **what the opening would already
  report**, never an invented number: "shifted by +2.000 s, four lines out of four would attach, against one".
- **Nothing is proposed** when the attachment does not clearly improve, **when two candidates are equal**, or
  when the gap is not constant (a drift, a partial shift) — **and the sentence then does not say there is one**.
- **The window**: when the opening is not clean and a shift would make it clean, **the sentence says so** and
  **offers to reopen shifted** (undo the opening, then reopen with the shift): a single history entry, and
  undo is enough to go back.
- **No command line**: neither subcommand nor option. If `inspect -t` must say it one day, the key is additive.
- **The eight pairs of `src/test/data/paires/`** serve as a guard: none is offered a shift it does not
  have. Putting it to the test requires a **generator of pairs with their ground truth**, written with the detection.

**Delivered (#621).** The candidates are the gaps to the subtitle that starts just before the line and to the one that starts
just after: the nearest alone is wrong when two subtitles are equally near. A gap that at least two lines
do not show is noise, and at most eight candidates are judged. The shift returned is the **median** of the gaps
of its group, hence exact to the thousandth when it is constant. The window looks for it **before** applying the opening —
afterwards, the project carries the subtitles born from the lines. The generator is `src/scripts/shifted-pairs.py`
(`src/test/data/decalages/`, six cases: no shift, three constant, a drift, a partial shift), held by the
gate like the other generated fixtures.

## D11 — Finding the pair of a conversion at the wrong frame rate

**Decided by the user on 2026-10-06: it belongs to this phase** ([#386](https://github.com/Guyot-Bertrand/sub-edit/issues/386)),
the spec of phase 16 having called it "the most useful and the least certain" and deferred it here, because phase 14 has
a player able to **show the result to the frame**. The three questions phase 10 had left:

- **How far to search: a closed set** — the **fifty-six ordered pairs** of the eight standard frame rates.
  An arbitrary rational would always yield a grid to whoever searches long enough; eight frame rates yield
  fifty-six, which are enumerated and told apart.
- **The confidence: that of the deduction.** Each ratio is applied to the positions, **the deduction of phase 16
  judges the result** (`Clean`, `Partial`, `Silent`), and what is returned is **the best pair with the gap that
  separates it from the second and from "do nothing"**. **When two pairs are equal, neither is proposed**, and the sentence
  says so. The trigger: the deduction returns `Silent` or `Partial`.
- **What the interface shows before applying**: the analysis dialog (`GUI-GRID-02`) shows the proposal;
  **`Convert Frame Rate…` opens prefilled**; **nothing is applied without a gesture**, undo is enough to go back, and
  the video makes it possible to judge to the frame.

**Delivered (#622, which takes over #386).** The fifty-six pairs make **forty distinct ratios**: 30 to 25 and
60 to 50 move every position alike and are only one conversion, of which the pair whose output
is the grid the positions fall on is kept. The judge is the deduction: a conversion fits when it returns `Clean`, and
**two that fit are a tie** — with no further threshold, since the deduction has only one. What the enumeration
of the three hundred and twenty combinations (grid × wrong ratio) showed, and what the spec had not foreseen: **a single
repair is the exception**. A grid at 28.8 frames per second goes back onto 24 and onto 30, and a file with
doubled times stays on the grid of 25; only the conversion of a file already at the target frame rate, the most
common gesture, has a unique repair. The tie is therefore stated with its two pairs, and the button is only offered for a
single conversion. The generator is `src/scripts/wrong-rate-pairs.py` (`src/test/data/conversions-fausses/`, ten cases);
the cost of the worst path, forty deductions on four thousand subtitles, is on the order of two milliseconds.

## D12 — The settings

Four settings, in `Preferences…`, under [ADR 0022](../adr/0022-configuration-au-noyau-et-tolerance-par-option.md)
— tolerant option by option, an absurd value does not break opening: **the jump step** (30 s, like
Gaupol), **the frame step** (1 frame, from 1 to a reasonable bound; D6), **the lead before a selection** (1 s, like
Gaupol) and **the volume**. **Nothing else is remembered**: neither the
audio track, nor following, nor the frame mode (a window setting, not a document's — it follows `View`).


## D13 — The manual and the screenshots

A page on timing and the player, **the shortcut table held by [#612](https://github.com/Guyot-Bertrand/sub-edit/issues/612)**,
and screenshots **of the playback bar and of the picture**: ADR 0041 makes them possible, the picture being in the
window. `make manual` photographs them **under the real player, with a versioned video fixture** — that of #610 or a
picture chosen for the manual — rather than under the double, which paints nothing; the end-of-phase review will reread the
manual in one go.

## How the phase is proved

- **Positioning accuracy has an oracle independent of the player** ([#610](https://github.com/Guyot-Bertrand/sub-edit/issues/610)):
  13 kB videos in which every frame carries its number, and a reading of the displayed frame. **A single keyframe**,
  because that is the hard case. After the surface (D2), the test reads the picture **in the window**.
- **The cost of the jump and of the step is measured** ([#611](https://github.com/Guyot-Bertrand/sub-edit/issues/611)) on a
  film-sized video, entered in the log like the other records; **a rendering benchmark** is added with ADR 0041.
- **The shortcuts are confronted with the manual** ([#612](https://github.com/Guyot-Bertrand/sub-edit/issues/612)): some fifteen
  keyboard gestures cannot be kept by eye.
- **The marking gestures are proved by the round trip**: set a mark, go back to it, and read the same frame.
- **The detections are proved by a planted truth**: a shift or a wrong conversion that was introduced, and
  that is found again; and **by silence** — a drift, two equal candidates, a clean file propose nothing.
- **Following is proved without a clock**: the follower is driven by hand, only one case pays for the timer.
- **A case on a real film** is that of the end-of-phase review, on what the user watches: the private
  corpus has no video, and no figure is attributed to it. The benchmark `seeking and stepping on a real film` takes the
  film through `SUBEDIT_BENCH_REAL_FILM` and abstains without it; it runs under `limit-cores.sh` like the whole benchmark.

## What the phase does not deliver

Each with a recipient.

- **Zero-copy hardware decoding** — **set aside by ADR 0041**, with its trigger: a user whose playback
  drops frames at an ordinary size and codec. The answer would be the OpenGL API **behind the same surface**.
- **4K, HDR, a heavy codec**: **partly measured at the review** — heavy 4K HEVC does not keep up with real time on two cores, hardware decoding is the answer ([#647](https://github.com/Guyot-Bertrand/sub-edit/issues/647)); HDR and AV1 remain unmeasured.
- **The style of the line and of the timecode** (font, colour, position, background) — **set aside**, trigger: a request.
- **Automatic playback on opening** — **set aside**, same trigger.
- **A command line for the player or the detections** — **set aside**: D10 and D1.
- **Increments in milliseconds** — **set aside**, D6; trigger: a caller who adjusts to the thousandth.
- **A shift proposal for a drift or a partial shift** — **set aside**: D10 says nothing rather than
  something wrong. A linear drift is a frame rate conversion, and D11 handles it.
- **Position notifications** from the player — **set aside**, D3; trigger: a need the 100 ms poll
  does not serve.
- **Sound**: audio transcription, automatic alignment — phases 22 and 23.

## Deviations from Gaupol

| What Gaupol does | What `subedit` does | Why |
| :--------------- | :------------------ | :-- |
| the picture is rendered by GStreamer into a GTK widget | **by libmpv, in software, into a Qt widget** | ADR 0041 |
| `Seek Backward / Forward` by 30 s, with no setting in the interface | **the step is a `Preferences…` setting**, 30 s by default | D12 |
| no one-frame step | **step forward and back by one frame**, and nudge an edge by one frame | the reason libmpv was chosen |
| the drawn subtitle has seven style settings | **the style of phase 6, without settings** | D1 |
| `autoplay` | **no automatic playback** | D1 |
| no shift detection | **two detections, proposed and never applied** | D10, D11 |
| following playback brings the row back on screen | **the table centres, and is suspended by scrolling by hand** | D7, user request |
| `Set Audio Language`: a choice of language | **a submenu of the tracks**, by language, title or number | a track does not always have a language |
| no way to suspend or resume following by hand | **a `Follow` button**, which shows the state of following and resumes it | user request, #619 |
| the table's positions can be shown in frames; the playback bar's cannot | **the bar also shows its two positions in frames**, through the same setting | user request, #620 |

## Requirements

**Twenty-eight**, all `implemented` and cited by a test at the end-of-phase review — none `abandoned` —, **entered in the registry with this spec**: `check-requirements.sh` confronts the
table of a phase spec with the registry in both directions. Each issue **rereads** its own before its code — the state
moves to `implemented` when a test cites them, never before. **Twelve new subjects**, none of whose identifiers was taken:
`SURFACE`, `STEP`, `NUDGE`, `SEEK`, `VOLUME`, `TIMECODE`, `AUDIO`, `MARK`, `FOLLOW`, `REPLICA`, `DRIFT`, `REPAIR`; `FRAMES`
continues from `GUI-FRAMES-01`.

| ID | Requirement |
| :- | :---------- |
| `GUI-SURFACE-01` | the video picture is drawn in the window, resizable, its aspect ratio preserved |
| `GUI-SURFACE-02` | after a jump, the displayed frame is that of the requested position, the nearest |
| `GUI-SURFACE-03` | the window adopts no native window and does not require X11 |
| `GUI-STEP-01` | stepping forward one frame displays the next frame |
| `GUI-STEP-02` | stepping back one frame displays the previous frame |
| `GUI-STEP-03` | the duration of a frame comes from the video, failing that from the document, failing that from the grid; without any of them, the gesture refuses and says so |
| `GUI-STEP-04` | the step is set in a number of frames, at least one; an absurd value does not break opening |
| `GUI-NUDGE-01` | nudging the start or the end of a subtitle by one frame is undoable in one entry |
| `GUI-SEEK-01` | the position slider can be read and moved |
| `GUI-SEEK-02` | step back and forward by an adjustable step |
| `GUI-SEEK-03` | the previous and the next subtitle place playback at their start |
| `GUI-SEEK-04` | the start and the end of the selection place playback, with the configured lead |
| `GUI-SEEK-05` | playing the selection stops at its end |
| `GUI-VOLUME-01` | the volume is adjusted and remembered from one session to the next |
| `GUI-TIMECODE-01` | the timecode is overlaid on the picture |
| `GUI-AUDIO-01` | the audio track is chosen among those of the video |
| `GUI-MARK-01` | set the start of a subtitle from the video position, in one history entry |
| `GUI-MARK-02` | set its end from the video position |
| `GUI-MARK-03` | insert a subtitle at the video position |
| `GUI-MARK-04` | select the previous or next subtitle from the video position |
| `GUI-FOLLOW-01` | the table centres the current subtitle during playback |
| `GUI-FOLLOW-02` | scrolling by hand suspends following |
| `GUI-FOLLOW-03` | following resumes on a player gesture or on demand |
| `GUI-REPLICA-01` | the line drawn on the picture no longer has raw markup |
| `GUI-FRAMES-02` | positions are displayed and entered as frame numbers, the frame rate stated |
| `GUI-DRIFT-01` | opening a translation says that a constant shift would attach it better, without saying so of a drift |
| `GUI-DRIFT-02` | reopening shifted attaches according to the proposed shift, in one history entry |
| `GUI-REPAIR-01` | the analysis dialog proposes the frame rate conversion that puts the file back on a grid, or nothing when two are equal |
| `GUI-REPAIR-02` | `Convert Frame Rate…` opens prefilled with that proposal |

## Split into issues

**Five slices**, the foundations first — the surface and the player interface, on which everything depends —, then what
the user touches, then the two detections, which depend on the player only to judge to the frame. **The tooling
issues of the initialisation (#610 to #612) come first; #610 before any implementation issue.**

**The issues are open**, in milestone 14: [#610](https://github.com/Guyot-Bertrand/sub-edit/issues/610)
to [#623](https://github.com/Guyot-Bertrand/sub-edit/issues/623), with [#386](https://github.com/Guyot-Bertrand/sub-edit/issues/386)
and [#408](https://github.com/Guyot-Bertrand/sub-edit/issues/408), older ones, which find their place there.

| Slice | Issue | What it delivers | Depends on | Size |
| :---- | :---- | :--------------- | :--------- | :--- |
| 0 — the tooling | [#610](https://github.com/Guyot-Bertrand/sub-edit/issues/610) | **videos that carry the number of their frames**, and an oracle | — | M |
| | [#611](https://github.com/Guyot-Bertrand/sub-edit/issues/611) | **the benchmark of the jump and of the step**, according to the distance to the keyframe | #610 | S |
| | [#612](https://github.com/Guyot-Bertrand/sub-edit/issues/612) | **the manual states the shortcuts** of the window | — | S |
| 1 — the foundations | [#613](https://github.com/Guyot-Bertrand/sub-edit/issues/613) | **the surface**: libmpv's software rendering, in the Qt window (ADR 0041, #190) | #610 | L |
| | [#614](https://github.com/Guyot-Bertrand/sub-edit/issues/614) | **the player interface**: frame step, volume, tracks, `playUntil`, and what "the position" means | #610 | M |
| 2 — driving | [#615](https://github.com/Guyot-Bertrand/sub-edit/issues/615) | **the playback bar**: slider, jumps, neighbours, selection, volume, timecode, three settings | #613, #614 | L |
| | [#616](https://github.com/Guyot-Bertrand/sub-edit/issues/616) | **the audio track** | #614 | S |
| 3 — timing | [#617](https://github.com/Guyot-Bertrand/sub-edit/issues/617) | **the marks**: start, end, insertion, neighbour, from the position | #614 | M |
| | [#618](https://github.com/Guyot-Bertrand/sub-edit/issues/618) | **frame-by-frame stepping**, and nudging an edge by one frame | #614, #617 | M |
| | [#619](https://github.com/Guyot-Bertrand/sub-edit/issues/619) | **following in the table**: centred, suspended by hand, resumed on a gesture | — | M |
| 4 — the display | [#408](https://github.com/Guyot-Bertrand/sub-edit/issues/408) | **the line without raw markup**, through the markup pivot | — | S |
| | [#620](https://github.com/Guyot-Bertrand/sub-edit/issues/620) | **positions in frames**: a `View` setting | — | M |
| 5 — the detections | [#621](https://github.com/Guyot-Bertrand/sub-edit/issues/621) | **recognise a shifted translation**, and reopen it shifted | — | L |
| | [#622](https://github.com/Guyot-Bertrand/sub-edit/issues/622) | **propose the conversion that puts a file back on a grid** (#386) | #613, #615 | L |
| | [#623](https://github.com/Guyot-Bertrand/sub-edit/issues/623) | **end-of-phase review** | everything | M |

**The end-of-phase review ([#623](https://github.com/Guyot-Bertrand/sub-edit/issues/623)) opened four issues**,
nothing being fixed in the review: [#644](https://github.com/Guyot-Bertrand/sub-edit/issues/644) (the player's
position rules, outside the core), [#645](https://github.com/Guyot-Bertrand/sub-edit/issues/645) (a single path for
the player's gestures, and the test that holds it), [#646](https://github.com/Guyot-Bertrand/sub-edit/issues/646) (the player's
tests without a clock) and [#647](https://github.com/Guyot-Bertrand/sub-edit/issues/647) (hardware decoding with copy).

**#613 is the cornerstone of the phase, and the riskiest**: it changes the way the picture arrives, hence
what each player issue touches, and it is the one that commits to ADR 0041. **#614 is the narrowest and the most
blocking**: everything that is set or driven goes through `VideoPlayer`.

**What can move forward in parallel**: #619, #620, #621 and #408 depend on no player issue; #611 and #612
as soon as #610, and at any time.

## Open points

**Two were settled by the user on 2026-10-06** (1 and 2). The others are what the framing cannot settle
alone, **each with what is assumed in the meantime**.

| No. | Point | Assumed | Who, when |
| :-- | :---- | :------ | :-------- |
| 1 | **the display path**: native window, OpenGL rendering, software rendering | software rendering (ADR 0041) | **settled on 2026-10-06**: the assumption is kept |
| 2 | **the two detections**: in the phase, or deferred | both in the phase (D10, D11) | **settled on 2026-10-06**: in the phase, against the assumption of the framing, which deferred #386 |
| 3 | **what a frame "is"**: the nearest to a position, or the one displayed at that instant | the nearest, which is what mpv does (D4) | **settled in #614: the assumption is kept**, and proved on videos that carry the number of their frames (23.976 and 25 frames per second); a contrary decision would require correcting the requested position before the `seek` |
| 4 | **the jump step**: 30 s, like Gaupol | 30 s | **settled in #615: 30 s, and a `Preferences…` setting** (`video.seek-length`) |
| 5 | **resuming following**: a player gesture, or also a delay | a gesture only, no timer (D7) | **settled at the review: the assumption is kept** — following resumes on a player gesture or on the `Follow` button, never on a delay, and no use has asked for the opposite. Trigger: a user who loses following without knowing why |
| 6 | **a translation shift proposed for a drift** | no (D10) | **settled at the review: no** — a linear drift is a frame rate conversion, and D11 handles it; the `derive` and `partiel` fixtures prove that nothing is proposed. Trigger: a real request |
| 7 | **the form of "reopen shifted"**: a button of the opening's box, or a menu action | a button of the box, which already exists when the opening is not clean | **settled in #621: the count box becomes a `Yes` / `No` question** when a shift repairs the opening — the count, then the shift sentence, `No` by default; without a shift, it stays the box to close |
| 8 | **4K, HDR, HEVC** under software rendering | unmeasured; the ADR states its trigger | **measured at the end-of-phase review** (ADR 0041, addendum): 1080p H.264 keeps up with real time at 2.5 ×, a heavy 4K HEVC does not on two cores (0.62 × in 8 bits, 0.92 × in 10 bits). HDR, AV1 and sustained playback with dropped-frame counting remain unmeasured |
| 9 | **hardware decoding**: `hwdec-copy` under software rendering | no | **delivered: yes, `hwdec=auto-copy` and a setting to turn it off** — the gain through libmpv is 1.1 × to 3.2 × depending on the film, less than the 2.1 × of ffmpeg alone (ADR 0041, addendum). [#647](https://github.com/Guyot-Bertrand/sub-edit/issues/647) |
