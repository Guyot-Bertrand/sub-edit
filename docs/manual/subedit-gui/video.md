# The associated video

The **Video** menu carries first two commands, and the gestures that drive the film
— see [The player](player.md#moving-through-the-film).

| Command | Shortcut | What it does |
| :------ | :------- | :----------- |
| Select Video… | none | chooses the video file the document goes with |
| Play / Pause | `Ctrl+P` | plays or stops the video — see [The player](player.md#playing-and-stopping) |

The associated video is **a property of the open document**, not a setting of
the application: opening another subtitle file starts from scratch. It
is written to no file and disappears with the window.

## The audio track

A film can carry several sound tracks — the original version, a dub, a commentary. The
**`Video ▸ Audio ▸ Language`** submenu lists them, **one entry per track**, and **marks the one playing**:

| What the file gives | What the entry says |
| :------------------ | :------------------ |
| a language and a title | `1: fra — Original` |
| a language alone | `2: eng` |
| a title alone | `3: Commentary` |
| neither | `1` |

The number is **the place of the track in the film**, one by one, in the file's order: two tracks
can say `eng`, and a file silent about its labels still has a first and a second.

**Choosing an entry plays that track.** The mark is the player's answer, not the click: it tells the
track that is playing.

- **The submenu is disabled** with no video or no audio track. With a single track, it stays enabled and
  shows it, checked: it tells the language of the film. The two volume gestures, which are next to it, stay
  enabled in any case: see [The volume](player.md#the-volume).
- **The track is not remembered** from one video to the next, nor from one session to the next: a track number is
  that of its file, and carrying it over to the next film would choose whatever bears that number there.
  Going back to a tab reopens its film on the track the player chooses by itself.
- No shortcut: you change language rarely, and the menu is enough.

## Choosing a video

The dialog opens on the directory of the subtitle file and filters on
the recognised video extensions:

    .avi .flv .m2ts .mkv .mov .mp4 .ogg .ogv .vob .webm

It also offers to show all files. Cancelling the dialog changes
nothing.

The chosen file is **opened at once**: the picture appears in the window,
paused on its first instant. See [The player](player.md).

**Dropping a video file on the window does the same thing** — see
[Dropping files on the window](files.md#dropping-files-on-the-window).

**The video is that of the current tab's project.** Each tab keeps
its own; changing tabs changes the film. **Going back to a tab resumes its
film where you left it**, paused, like any opening. Another film
chosen for that project in the meantime starts, for its part, from the beginning.

## The automatic proposal

When a subtitle file is opened, **if no video has been chosen**,
the window looks for one in the same directory: a video file whose name
without extension is a prefix of ours, up to a dot.

| Subtitle file | Video found | Why |
| :------------ | :---------- | :-- |
| `film.fr.srt` | `film.mkv` | `film` is a prefix of `film.fr` |
| `film.en.forced.srt` | `film.mkv` | likewise, whatever the number of segments |
| `film.fr.srt` | `film.fr.mkv` rather than `film.mkv` | the longest name wins |
| `film.fr.srt` | none, if `film.mkv` **and** `film.mp4` are there | two candidates cannot be told apart |
| `film.fr.srt` | none, if only `fil.mkv` is there | `fil` does not stop on a dot |

**A proposal is not a choice.** It lets itself be replaced by another
proposal — opening another file, saving under another name. An explicit
choice, on the other hand, is never overwritten: once `Select Video…` has been used, the
automatic proposal no longer comes back behind it.

That is what makes the guess acceptable: it can be corrected, and its
correction holds.

## What the status bar shows

On the right of the status bar, a line says what the document goes with. It
is the last of three: the [encoding](files.md#the-encoding-in-the-status-bar)
of the file read comes first, the [frame grid](frame-grid.md) deduced from the
positions next.

| Situation | What is written |
| :-------- | :-------------- |
| a video is associated | `Video: film.mkv` |
| … and `ffprobe` gives its frame rate | `Video: film.mkv, 24000/1001 fps` |
| none | `No video` |

**The frame rate goes with the film rather than beside it.** They are a single fact — what
this document goes with — and a third mention would put it on the same rank as
the [grid deduced from the positions](frame-grid.md), which is another one.

## When there is no film

The window shows a **`Select Video…`** button where the picture would go, rather than
nothing at all. It is the same command as the one of the `Video` menu, within
reach of the mouse: an absence you cannot act on is hard to tell from a
fault.

The button disappears as soon as a film is open, and the picture takes **all** its
place: the height the table leaves above it — the one
[`window.table-share`](preferences.md) remembered from one session to the next —, and
never less than the minimum height of the picture, which the table yields if needed.
When the film goes away, the button takes back the same place.

**The file's name, never its path.** A path of two hundred characters
would push everything else out of the bar; the full path is the one the
dialog showed.

## `ffmpeg` is not required

`subedit-gui` queries `ffprobe` — shipped with `ffmpeg` — for one thing and one
thing only: **the frame rate the container declares**. It is read once
per film, when it is associated.

| With `ffprobe` | Without |
| :------------- | :------ |
| the status bar writes the frame rate beside the film's name | it writes only the name |
| the conversion dialog proposes the film's frame rate in the bottom field | it proposes nothing, and opens as before |
| the alignment dialog opens on it | it opens on the project's frame rate |

**Nothing else depends on it, and no operation refuses.** The player, for its part,
does not need `ffprobe` at all: it relies on `libmpv`, which is a
build dependency. The film's duration, the one used to report what
goes past the end, comes from the player and not from `ffprobe`.

It is therefore not a degraded mode but **the normal operation** of a machine
that has never installed `ffmpeg`: the data comes from the user, and `ffprobe`
proposes it when it is there.

A frame rate read is an **exact rational** — `24000/1001`, not `23.976`.
That is the reason `ffprobe` is kept for that answer: the
player knows the same thing as a floating-point number, and an approximate frame rate
would re-time the whole file slightly off.
