# Phase 6 — The built-in player

**Framing issue: #165.** This document decides what the phase builds, and splits
it. It is not a report: what it writes in the future tense is what remains to be
done.

## Goal

After a shift, a transformation or a frame rate conversion, **check the result
against the film**. This is the missing gesture: the core has been able to move
four thousand subtitles since phase 2, the window has shown it since phase 5,
and nothing yet says whether the result lands right.

The video is watched **in the window**, with the current line drawn over it.

## What changed, and why it is written here

The roadmap framed this phase around a **preview through an external player** —
write a temporary file, launch mpv, MPlayer or VLC — and put the built-in player
in phase 14, "the most expensive part of the project".

That framing was revisited. And iso-functionality demanded it: **Gaupol has
both.** `gaupol/player.py` is an embedded GStreamer player, distinct from
`aeidon/agents/preview.py`. The roadmap had kept the second and forgotten the
first.

The external player **disappears** from the phase. It was only a substitute put
in place when the built-in one seemed too expensive; keeping it as well would
cost two surfaces, two manual sections and two sets of tests for a gesture the
built-in player renders better.

What this makes of the tooling already delivered, plainly:

| Issue | What it delivered | What remains of it |
| :---- | :---------------- | :----------------- |
| #162 | `ffprobe`, and `findExecutable` | **used** — `ffprobe` remains the only exact source of the frame rate |
| #163 | two video fixtures | **used** — by `ffprobe` and by the player |
| #164 | `startProcess`, `outcomeOf`, the bounded wait | **used** — this is how `ffprobe` is launched |
| #164 | the fake video player | **repurposed** — it no longer stands in for mpv, it stands in for `ffprobe`: writing to its output and returning a code is exactly what is expected of it |

Nothing is lost, and the fake player deserves to be renamed when an issue
touches it.

## Scope

**In.**

| What the phase delivers | Why here |
| :---------------------- | :------- |
| associating a video file with a project | nothing can be watched without it |
| a player in the window: open, seek, play, stop | the purpose of the phase |
| the current line drawn on the image | this is what makes the result checkable |
| reading the frame rate with `ffprobe` | the tooling is in place, and it is the only exact source |
| warning when an operation pushes subtitles beyond the end of the film | the duration comes from the player, which already has it |

**Out, and why.**

| What is not in it | Where, and why |
| :---------------- | :------------- |
| **all the playback control except play and stop** — position bar, seeking forward and backward, seeking to the neighbouring subtitle, playing the selection, volume | phase 14, which carries them all and names them one by one |
| frame-by-frame stepping, setting a mark from the current position | phase 14 |
| preview through an external player | removed, see above |
| choice of audio track, timecode overlay | phase 14 |
| waveform | never; nothing asks for it |
| deducing the frame rate from the positions, without video | phase 16 |
| refusing an operation outside the bounds of the film | never — the phase warns and prevents nothing |

**The first row was written after the fact**, and this is the defect the phase
committed: it delivered "Play / Pause from the keyboard and the menu" without
saying anywhere that Gaupol has eight more, nor where they were going. A window
without a player bar is a defensible choice for a phase; not knowing in which
phase it arrives is not. The roadmap now carries the count, taken item by item
against Gaupol's **Video** menu.

## Applicable decisions

**D1 — libmpv.** See [ADR 0020](../adr/0020-libmpv-pour-le-lecteur-integre.md).
The deciding criterion is not this phase, which Qt Multimedia could serve, but
the fine timing that remains for phase 14 — and which has no API there.

**D2 — the line is drawn from the model, never loaded from a file.** libmpv can
load an external subtitle file; for an editor that would be the wrong choice,
since the file would have to be rewritten at every keystroke. What is displayed
comes from the same `Project` as the phase 5 table, so what is seen on the image
is what was just typed.

**Not asking it is not enough**, and #176 measured it: given `film.mp4`, mpv
loads on its own the `film.srt` lying next to it — the very one being edited.
The image would have shown the state of the disk while the table shows the
typing, the two parting at the first correction. `sub-auto=no` holds D2 where it
would have been lost silently.

**D3 — previewing a change means applying it and then undoing it.** Gaupol
copies the subtitles, applies the operation outside the history, previews, and
restores. It wrote it that way for lack of a reliable undo; `subedit` has had
one since phase 2, which the window has exposed since phase 5 — "Undo: shift".
The core therefore gains no way of executing a command without recording it,
and the user sees what happens to them rather than an invisible provisional
state.

**D4 — the upper bound warns, it does not refuse.** An operation that pushes
three subtitles past the end credits may be exactly what the user wants. A
refusal that is wrong costs more than a warning that is ignored.

**D5 — the naming convention proposes, the explicit choice decides.** When a
subtitle file is opened, if no video is associated, the same directory is
searched for a video file whose name without extension is a prefix of ours —
`film.mkv` for `film.fr.srt`, the rule of `find_video`. What the convention
finds is **a proposal**: the explicit choice overrides it and is never
overridden by it in return.

Two points that Gaupol's rule left open, settled by #171:

- **the prefix is read segment by segment**, the boundary being the dot.
  Gaupol compares raw strings, where `fil.mkv` answers for `film.fr.srt` — a
  film proposed for a subtitle that has nothing to do with it, and the user
  only notices by watching the wrong image;
- **the longest name wins, and a tie proposes nothing.** `film.fr.mkv` beats
  `film.mkv` for `film.fr.srt`, being the closer of the two. But `film.mkv` and
  `film.mp4` cannot be told apart: neither the file system's order — which is
  not stable — nor the order of our own list of extensions — which nobody can
  read — is an answer. Silence is one, and D5 does the rest, since the user's
  choice is never overridden.

**D6 — the frame rate read from the container is proposed, never imposed.** It
is the second source of the same datum, and phase 16 will bring a third,
deduced from the positions. A source that imposes itself forbids
cross-checking; a source that proposes leaves comparison possible.

**D7 — the frame rate comes from `ffprobe`, the duration from the player.**
libmpv's `container-fps` is a floating-point number; `FrameRate` has been an
exact rational since phase 1, and approximating it would lose what that choice
protected. The duration does not have this problem and the player already knows
it: two sources for the same datum without necessity would be one too many.

## Tolerating the absence of `ffprobe`

**It is not a degraded mode, it is the normal behaviour.** When `ffprobe` is
missing, `subedit` does without what it brings — the declared frame rate. If an
operation needs it, **it is up to the user to provide it**, exactly as today.

No operation refuses on the ground that `ffprobe` is missing, and **the player
works without it**: libmpv is a build dependency, `ffprobe` an executable that
is looked for.

The order of writing follows from this: **the datum comes from the user, and
`ffprobe` proposes it when it is there.** Not the other way round. Code written
the other way treats the normal path as a fallback, and that is the one that
stops being exercised.

## The core

### The associated video

The project gains the notion of a video associated with a document. Three
things make it up, and they are not to be confused:

- **the path**, chosen or guessed;
- **where it comes from** — guessed by convention, or chosen — because D5
  depends on it;
- **what the container declares**, that is the frame rate, or nothing.

A video file is recognised by its extension, from a closed list, like
`aeidon.util.is_video_file`. It lives with the rest of the core's vocabulary:
the naming convention needs it, and the convention belongs to the core.

### Reading the frame rate

A call to `ffprobe`, which returns `r_frame_rate` as an exact rational.
`findExecutable` (#162) finds it. The call waits: it takes a few milliseconds,
and no interface has any reason to hand control back in the meantime.

**It is not `startProcess` that launches it**, contrary to what this section
first announced. #172 wrote it a sibling, `runAndCapture`: it waits, and returns
what the program wrote. The one from #164 is made for a video player, which
outlives the call that launched it — it returns at once and writes the output
to a file, which would have had to be named, placed on writable storage, read
back and deleted, all for twelve bytes, and `outcomeOf` polled in a loop while
waiting. Asking the question through a pipe and waiting for the answer says the
same thing with less.

Three possible outcomes, and only one is an error:

| What happens | What the core returns |
| :----------- | :-------------------- |
| `ffprobe` is absent from the `PATH` | nothing, without error |
| it answers, but the file is not a video | nothing, without error |
| it answers with a frame rate | the exact rational |

### The player

A seam in the sense of principle 3 — "video player" has been named there as a
variation point since the foundations, and this is that case. Behind it, libmpv.

What it exposes, and nothing more until phase 14 is here:

| Operation | What it does |
| :-------- | :----------- |
| open | loads a file, or says why it does not open |
| seek | goes to a position, **exactly** |
| play, stop | what the words say |
| position | where playback is, to follow the current row |
| duration | what the container declares, once open |
| draw the line | puts a text on the image, or clears it — D2 |

**The interface lives in the core, the implementation in `subedit_gui`.** The
distinction is that of principle 3, and it was revisited during #173:
`core::VideoPlayer` names neither Qt nor libmpv, and that is what lets the rest
of the core reason about a player without knowing one. `MpvPlayer`, for its
part, is an interface element — it exists only for the window, it receives its
native window from it, and nothing else will build it. Bringing libmpv into
`subedit_core` would have given the domain an infrastructure dependency for the
mere convenience of a lighter test harness, which is not an architectural
reason.

The move also revealed a defect that its place in the core was hiding:
**libmpv refuses to start if `LC_NUMERIC` is not "C"**, and `QApplication` sets
it to the user's. In the core, the tests ran without `QApplication`, hence in
the "C" locale; the defect did not show, and any real window would have hit it.

**The rendering, settled by #173: the native window that libmpv adopts**,
through the `wid` property. It is **a number**, and that is precisely what lets
the core designate a window without knowing anything about a window. It is the
shorter of the two paths, and it leaves the line to libmpv's `osd-overlay` —
which D2 allows, since the text there comes from the model and not from a file.

**What #176 found when putting it in place**, and which nothing announced. The
three points were measured, each after a visible symptom:

| What happens | What it took |
| :----------- | :----------- |
| adopting a native window is **an X11 mechanism** — libmpv says so in its own header | `subedit-gui` asks for `xcb` when the session leaves the choice open, and refuses to build a player elsewhere |
| left to probe, mpv picks the session's context — Wayland — where `wid` means nothing, and **opens a window next to ours** | naming `gpu-context=x11egl` |
| a window not yet on screen is adopted and **never mapped**: `IsUnMapped` for the life of the process | opening the film at the first `showEvent`, the view made visible first |

The third is the one that cost: the panel stayed empty, and the review had
concluded it worked by observing that mpv's window was a **child** of the video
view — without reading its mapping state. A structural proof taken for a visual
proof.

**What the #173 decision costs, in one line**: the player only works on a Qt
`xcb` platform. [ADR 0020](../adr/0020-libmpv-pour-le-lecteur-integre.md)
records the finding and the trigger that would reopen it.

### The upper bound

The core today knows only one bound, zero: a negative shift that is too large
gives a negative position, and that is all it can say. The duration the player
announces gives it the other end.

**The shift is the clearest case, and it is not the only one.** A
transformation whose second mark falls after the end, a frame rate conversion
that stretches the whole beyond it: all three overrun, and the warning must be
wired to all three.

What the warning says: how many subtitles overrun, and by how much the furthest
one does. What it does not do: prevent anything (D4).

**What #174 settled: one computation, three phrasings.** The three operations
overrun in three ways, but they all reach the same conclusion — subtitles that
end too late — and `beyondEnd` reads it **after the operation, on the state it
produced**, which the window shows anyway. Replaying each command's arithmetic
to announce it in advance would have been three copies of what the commands
already compute.

What remains specific to each is the **sentence**, which names the operation:
`noticeOf` composes it from `CommandKind`, and a warning that did not say which
of the three just ran would leave the user guessing.

Three readings the computation assumes, and that a test pins down:

- **the scope is the selection**, that is what the operation touched. A
  subtitle nobody moved, already beyond because the associated video is the
  wrong one, is not this operation's doing;
- **ending exactly with the video means ending inside it** — the same reading
  as the zero bound, which admits position zero;
- **the overrun is that of the furthest one**, not of the last in file order: a
  document out of order is ordinary, and what the user needs to know is how far
  it goes.

## The window

| Element | Behaviour |
| :------ | :-------- |
| **Video ▸ Select Video…** | a file chooser filtered on video extensions, set to the directory of the subtitle file |
| opening a file | triggers the search by convention if no video is chosen (D5) |
| the video view | occupies the top of the window, the table below, the splitter movable |
| no associated video | the view is absent, the table takes all the space, and nothing flickers |
| **Play / Pause** | from the keyboard and the menu |
| selecting a row | sets playback to the start of that subtitle |
| playback advances | the current row follows, without stealing focus from whoever is editing |
| the line | drawn on the image, from the model (D2) |
| the video does not open | says so, names the file, and the window remains usable |
| frame rate read | proposed in the frame rate conversion dialog, with its origin |
| out-of-bounds operation | the warning already in place for the other notices, after the operation |

**Everything the user reads is in English**, and the words live in
`core/wording/`.

## Tests

| What | How |
| :--- | :-- |
| reading the frame rate | the fixtures of #163, including the one at `24000/1001` |
| `ffprobe` absent | the injected search path of #162 |
| the naming convention | empty files with the right extensions, in `src/test/data/` |
| the upper bound | unit tests, on durations given by hand — no video needed |
| open, seek, the duration | the seam, on the fixtures of #163 — two seconds of video are enough |
| the window | the Catch2 harness of #119, off screen |

**A player without a screen was the point to settle before everything else**,
and #178 settled it. It was exactly the problem #117 had solved for the window —
`QT_QPA_PLATFORM=offscreen` — and it arose again for video.

The answer is `vo=null`, and it is measured rather than promised: with
`vo=auto` and no screen, mpv does not even open the file — it answers
`end-file`, and the duration comes back "property unavailable". What a test can
then expect from a player without output: the duration, the position after an
exact seek, the geometry of the image, playback that advances in real time.
**Not a decoded image** — there is no output to take it from — and so it is in
the window that the drawn line will be proven, never here.

`src/test/gui/mpv_player_test.cpp` carries that proof. It was first a separate
harness, written by #178 before the seam existed; #173 absorbed it, because two
files that open the same fixtures for the same reason end up diverging, and the
one that counts is the one that goes through the code in use.

## Measurements

Two new measurements, and not one more:

- **opening a video and seeking to a position** — what happens at every change
  of row, hence the only path the user feels;
- **drawing the line** on an image, if it stands out from the noise.

Decoding itself is not measured: what would be timed is libmpv, which does not
belong to the project.

**Four in the record, not two**: each of the two split in two the day it had to
be measured. Opening and seeking are two calls of very different costs — 11 ms
against 0.6 — and "drawing the line" split into what belongs to us: finding the
subtitle on screen, and composing the event the overlay receives. The rest is
libmpv, and is not timed.

## Requirements

Added to the registry at the start of the issue, in the `planned` state.

**That is not what was done**, and the gap is recorded rather than corrected
after the fact: the seven rows entered the registry at the end of their issue,
directly `implemented`. Nothing suffered from it — none was forgotten, the
`check-requirements.sh` check confronts them all with a test — but the
`planned` state therefore never held anything for this phase. To be settled
when the next one is framed: either the practice joins the rule, or the rule
disappears.

| ID | Requirement |
| :- | :---------- |
| `GUI-VIDEO-01` | choosing a video associates it with the document, and the window names it |
| `GUI-VIDEO-02` | opening a subtitle file proposes the neighbouring video of the same name |
| `GUI-PLAYER-01` | the video opens in the window, and plays |
| `GUI-PLAYER-02` | selecting a subtitle sets playback to its start |
| `GUI-PLAYER-03` | a video that does not open says so, and leaves the window usable |
| `GUI-FRAMERATE-02` | the frame rate read from the video is proposed, and its origin is stated |
| `GUI-BOUNDS-01` | an operation that overruns the end of the film signals it without preventing it |

## Manual

A section of `docs/manual/subedit-gui/`, **written by hand**. This is the
decision of #116: `make manual` generates its blocks by running the binary, and
a window has no `--help`. Its correctness rests on the end-of-phase review.

What it must say, beyond the gestures: that `ffmpeg` is not required, what is
lost without it, and that no operation depends on it.

## Split into issues

| Issue | What it carries |
| :---- | :-------------- |
| #178 — libmpv in the toolchain, and a player without a screen | the package, the CI cache, and a player that a test builds without visible output |
| #171 — Core: the video associated with a document | the path, its origin, recognition by extension, the naming convention (D5) |
| #172 — Core: reading the frame rate with `ffprobe` | the call, the exact rational, and the three outcomes of which only one is an error |
| #173 — Core: the player seam, and libmpv behind it | open, seek, play, stop, the position, the duration |
| #174 — Core: warning beyond the end of the film | the three phrasings — shift, transformation, conversion (D4) |
| #175 — Interface: choosing a video, and seeing it | the file chooser, the proposal on opening, the status bar |
| #176 — Interface: the player in the window | the view, the drawn line (D2), the row that follows, the keyboard |
| #177 — Interface: the frame rate read and the bound of the film | the proposal in the conversion dialog (D6), the overrun warning |

#178 comes before all the others. #173 depends on it and on #171; #174 depends
on #173 for the duration; the three interface issues depend on everything
before them. **#176 is the one that makes the gesture possible: the five before
it serve it.**

## Deferrals

| What is deferred | Where |
| :--------------- | :---- |
| frame-by-frame stepping, setting a mark from the position | phase 14 |
| playback control beyond play and stop | phase 14, which keeps the list |
| cross-checking the frame rate read and the frame rate deduced, and presenting a disagreement | phase 16, which brings the second |
| the player for a translation document | phase 11, which brings the second document |
| rendering in a native window or in an OpenGL context | **settled by #173**: the native window, `wid` property. Put in place by #176, which has a window to give |

## Completion criteria

- [ ] The eight issues are closed
- [ ] The registry's requirements are `implemented` and cited by a test
- [ ] The manual section exists and describes what exists
- [ ] The benchmarks are rerun and the measurements recorded
- [ ] The end-of-phase review is done
