# Timing from the video

*The film is assumed to be open and the player known: see [The player](player.md).*

You watch the film, stop on the frame where a line must begin, and **that position becomes
the start of the subtitle**. Five gestures, in the **Video** menu, under the names Gaupol gives them. They
rest on what already exists: setting a position is **the same command as typing the value into the
cell**, and inserting is **the same as `Edit ▸ Insert Subtitles…`**.

| Command | Shortcut | What it does |
| :------ | :------- | :----------- |
| `Video ▸ Set Start from Video Position` | `Alt+U` | sets the **start** of the selected subtitle to the film's position |
| `Video ▸ Set End from Video Position` | `Alt+K` | sets its **end** to the film's position |
| `Video ▸ Insert Subtitle at Video Position` | `Alt+J` | inserts a subtitle that **starts** at the position, and selects it |
| `Video ▸ Select Previous from Video Position` | `Alt+Up` | selects the last subtitle that **starts before** the position |
| `Video ▸ Select Next from Video Position` | `Alt+Down` | selects the first one that **starts after** the position |

**All are disabled with no video open**, like the player's gestures. `Set Start…` and `Set End…`
are also disabled as long as nothing is selected: they act on the **first line** of the
selection.

**The shortcuts are Gaupol's, with `Alt`**: there they are `U`, `K` and `J` — bare letters, which
the window would take before the cell being edited, so that typing a `k` in a text
would set an end. `Select Previous…` and `Select Next…` are `Ctrl+Y` and `Ctrl+U` in Gaupol; `Ctrl+Y`
is a *redo* on some systems, and the arrows with `Alt` say *before* and *after* better than
two letters. A cell being edited is **committed** before `Set Start…`, `Set End…` or
`Insert Subtitle…`, as before any modification.

## Setting an edge

- **One history entry per gesture**: `Edit ▸ Undo` gives back the previous position, and nothing else.
- **The same rules as the cell.** A start set after the end of the subtitle, or an end before its
  start, **is not refused**: it is set, and the table marks the subtitle as it marks the one
  whose value was typed by hand — see [The anomalies](table.md#the-anomalies). Nothing is corrected silently.
- **A position that is already the edge's does nothing**, and adds nothing to the history.
- **The position is always the start of a frame**: the player never reports a position
  inside a frame. Setting an edge, then going back to it — with `Seek Selection Start`, for example —
  **shows the same frame**.

## Inserting at the position

The new subtitle **starts at the position**, and lasts **three seconds**, or until the start of the next one
when that one comes earlier. Its text is empty. It takes its place in the file's order: after
all those that start at that position or before it. The insertion is **one history entry**,
and the inserted subtitle is selected: you can type it right away.

## Selecting from the position

Both follow the file's order, not the current selection. **With no neighbour on that side**, the
selection goes to the last subtitle of the file for `Select Next…`, to the first for
`Select Previous…` — Gaupol's behaviour. These two gestures do not move the film.

## Frame by frame

Moving forward and back **by one frame** — or by N, see below —, to set an edge **to the exact frame**.
Two gestures, and two buttons in the playback bar, on either side of the play button:

| Command | Shortcut | What it does |
| :------ | :------- | :----------- |
| `Video ▸ Step Backward` | `Alt+Left` | moves the film back by the frame step |
| `Video ▸ Step Forward` | `Alt+Right` | moves it forward by the frame step |

- **The step is a number of frames, one at least**: it is the `Frame step` setting of the
  [preferences](preferences.md#the-player). A step of N frames shows the frame **N further on**, at 25 frames per
  second as at 23.976; it is **never expressed in milliseconds**.
- **The film stops at its bounds**: stepping back from the first frame leaves it there, stepping forward from the last
  too. Playback is stopped after a step.
- **The bar's buttons** — two icons, on either side of play/pause — **trigger the same actions as the menu**: same step, disabled
  with no video like the menu's entries, and the same tooltip, which gives the shortcut. **A finger that stays on a
  button, or on the key, chains the steps.**
- **One step at a time.** A held key repeats faster than a step is made: the first repetition goes out right
  away, those that arrive while the step is being made **do not pile up** — they share one place, the
  last, which goes out afterwards. Releasing the key therefore stops the picture where you are, and does not let the film
  carry on under its momentum.

## Nudging an edge by one frame

To adjust an edge with the keyboard: **the start or the end of the selected subtitle**, by one step, earlier or later.

| Command | Shortcut | What it does |
| :------ | :------- | :----------- |
| `Video ▸ Nudge Start Earlier` | `Alt+Q` | the subtitle **starts one step earlier** |
| `Video ▸ Nudge Start Later` | `Alt+W` | it **starts one step later** |
| `Video ▸ Nudge End Earlier` | `Alt+Shift+Q` | it **ends one step earlier** |
| `Video ▸ Nudge End Later` | `Alt+Shift+W` | it **ends one step later** |

`Q` and `W` are Gaupol's for *earlier* and *later*, with `Alt` for the reason given above; the end
takes `Alt+Shift`, because Gaupol's `E` is here the letter of `Edit`.

- **One history entry per nudge**, undoable. The neighbouring subtitle and the overlap are reported
  **as when typing**: the edge is set, and the table marks what no longer holds — see
  [The anomalies](table.md#the-anomalies).
- **The same step as the player**, so one frame by default. **What a frame is worth** is read, in this order,
  from **the frame rate the video declares**, that of the **document when it counts in frames** (MicroDVD), then
  **the grid the positions draw**. **With none of the three, the gesture refuses and says so**: choosing a
  frame rate at random would shift any edge by something that is not a frame.
- **It does not require a video**, only a selection: it is also used on a file counted in frames, with no
  film. When a film is open, **playback moves to the new position**, to show the frame the edge
  has just taken.
- An edge does not go before the origin: stepping back by one step from less than a frame puts it at zero.
