# Video fixtures

Two tiny containers, of which not one frame is ever read: what is read in them
is **a frame rate and a duration**, the two metadata that phase 6 needs and that a
subtitle file does not declare.

| File | Rate | Duration | Weight |
| :------ | :-------- | ----: | ----: |
| `cadence-25.mp4` | `25/1` — PAL, integer | 2.000 s | 1,504 B |
| `cadence-23-976.mp4` | `24000/1001` — NTSC, fractional | 2.002 s | 1,519 B |

The second is the one that says something. A fixture with an integer rate
alone would suggest a correct reading where the core has handled exact
rationals since phase 1.

**Do not edit them by hand, and do not take them on trust.**
[`src/scripts/video-fixtures.sh`](../../../scripts/video-fixtures.sh) carries the
command that builds them and the table of what is expected of them:

```console
$ ./src/scripts/video-fixtures.sh --check      # what ffprobe says of them, against the table
$ ./src/scripts/video-fixtures.sh --generate   # rebuild them
```

`--check` runs in `make check-local`. That is where the guarantee lies: a
container is unreadable in a diff, and no one will reread these 3 KB.

## The numbered frames — #610

Two other fixtures, with another use: **each frame carries its own number**, and that is what is read.

| File | Rate | Frames | Key frame | Weight |
| :------ | :-------- | -----: | :-------- | ----: |
| `images-25.mp4` | `25/1` | 250 | **a single one, at the start** | 13,275 B |
| `images-23-976.mp4` | `24000/1001` | 240 | **a single one, at the start** | 13,062 B |

128×64, ten seconds. **The number is written as eight bars of 16 pixels**, each light for a bit at 1
and dark for a bit at 0, the least significant bit on the left: from 0 to 255. A test that has placed playback at a
frame **reads the picture the player displays** (`MpvPlayer::picture()`) and compares the number with the one it asked for —
an oracle that does not rest on what the player says of itself, since its position and its frame counter
come from the same place as the picture, and a player that showed the wrong frame while announcing the right time would
satisfy them.

**A single key frame**: this is the hard case, that of a real film — position 249 is decoded from 0.
The `mpeg4` encoder does not give a single one by default: despite `-g 250` it places one every 32 frames when
the content changes on every frame (eight out of 250, measured), hence `-keyint_min 250 -sc_threshold 1000000000 -bf 0`.

**The second is at 24000/1001**: frame 10 occupies 417.08 to 458.79 ms there, which a whole millisecond does not write.
This is what shows that the `Timestamp`'s fineness is not the defect — all frames start exactly — and
that **`seek` places playback on the nearest frame**, not on the one displayed at that instant.

`--check` verifies, for each, the rate, the number of frames, **the number of key frames**, the size, and that
**the first frame, the second, the middle one and the last carry their number — read by ffmpeg**, without libmpv: what
a test will read in the picture is worth something only if the fixture it reads is honest.

## The audio tracks — #614

Two more fixtures, which show nothing (a black 16×16 screen, two seconds) and **carry sound**: what is read in them
is the list of tracks that the player returns to the `Audio` menu.

| File | Tracks (language: title) | Weight |
| :------ | :---------------------- | ----: |
| `audio-1.mkv` | `fra`: "Original" | 6,124 B |
| `audio-2.mkv` | `fra`: "Original", `eng`: "Commentary" | 10,900 B |

**A video without a track** is `cadence-25.mp4`, which never had one: it is the player's "breaks nothing" case.

`sound-only.mkv` (5,072 B) is the reverse: **sound and no picture**. It has no frame rate, hence nothing to advance
by a step — the case where `stepFrames` does nothing rather than get it wrong.

**In Matroska, and not in MP4 like the others**: MP4 has no track title — ffmpeg writes a `handler_name` there —,
and the title is precisely what these fixtures must carry. The timestamps that Matroska rounds to the millisecond
do not matter here: no frame is read.

`--check` verifies, for each, the language and the title of each track, in order, and the size.

## The bench video — #611

**One last video, which is not in this directory**: `make bench` builds it in the build tree
(`build/release/bench-film.mp4`, `video-fixtures.sh --film`), because a film of this size has no place in a
repository. It follows the logic of the numbered frames: **1280×720, 250 frames at 25 per second, a single key frame**,
each frame carrying its number (eight bars of 160 pixels). `--film` verifies that it is honest as `--check` does
for the others, and does not rebuild it if it already is. Without ffmpeg, the seek and step bench says so and abstains.
