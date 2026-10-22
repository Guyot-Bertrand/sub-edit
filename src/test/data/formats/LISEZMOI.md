# One scene, in nine formats

Nine files that say **the same thing**: four lines of dialogue, at the same
positions, two of them on two lines and one in italics. What changes from one
file to the next is the format, and only the format — this is the discipline
of [`encodages/`](../encodages), which varies only the encoding.

This is what allows comparing **one reading with another** rather than two
different texts, and it is from here that everything that can be said about a
conversion comes.

## The scene

| # | Start | End | Text |
| -: | :--- | :--- | :---- |
| 1 | 00:00:01,000 | 00:00:03,000 | `The harbour wakes before` ⏎ `the town does.` |
| 2 | 00:00:04,000 | 00:00:06,000 | `Nothing moves on the water.` |
| 3 | 00:00:08,000 | 00:00:11,000 | *`A gull turns once above the mast`* ⏎ *`and settles.`* |
| 4 | 00:00:12,000 | 00:00:15,000 | `Then the light comes.` |

**The positions all fall on the second**, and this is a constraint of the poorest of
the nine: TMPlayer counts only in whole seconds. A scene it could not
write would no longer be the same scene for it.

**MicroDVD's grid is 25 frames per second**, exactly forty
milliseconds per frame — so the four positions also fall on a frame.
**The file does not say so**, no MicroDVD file does, and that is the whole subject of
the conditional promise below.

## What each rendering loses, and this is the table that matters

| File | Format | Line breaks | Italics | Ends | Precision |
| :------ | :----- | :------------- | :------- | :--- | :-------- |
| `scene.srt` | SubRip | ⏎ | `<i>…</i>` | carried | ms |
| `scene.vtt` | WebVTT | ⏎ | `<i>…</i>` | carried | ms |
| `scene.subviewer2.sub` | SubViewer 2 | `[br]` | `<i>…</i>` | carried | hundredth |
| `scene.ssa` | Sub Station Alpha | `\N` | `{\i1}…{\i0}` | carried | hundredth |
| `scene.ass` | Advanced SSA | `\N` | `{\i1}…{\i0}` | carried | hundredth |
| `scene.microdvd.sub` | MicroDVD | `\|` | `{Y:i}` | carried, **in frames** | frame |
| `scene.mpl2.txt` | MPL2 | `\|` | `/` at line start | carried | tenth |
| `scene.tmplayer.txt` | TMPlayer | `\|` | **lost** | **absent** | second |
| `scene.lrc` | LRC | **lost** | **lost** | **absent** | hundredth |

Three losses are visible to the eye in the files themselves, and that is on
purpose: LRC renders the two two-line lines on a single line, LRC and
TMPlayer carry no end, and neither has a vocabulary for
italics. **The full declaration of these losses belongs to the conversion
report** — issue #339; this table describes nine files, not
seventy-two pairs.

## They are written by hand, and there will be no factory

This was settled in #336, and the reason is not borrowed from the two existing
factories — it distinguishes them.

**Neither `encoding-fixtures.py` nor `subtitle-fixtures.py` reimplements what it
tests.** The first calls Python's codec, a deterministic third party; the
second lays down exact arithmetic, and the SubRip syntax around it takes three
lines.

**An ASS generator would have no third party to call.** The sections, the
`Format:` line, the order of the fields, the `\N`, the hundredths are exactly what
phase 9 writes in C++. A file generated from our understanding of the format,
read back by our reader of the same format, would prove only their agreement — and it would
look like something else, which is worse than nothing.

**And an `.ass` can be reread in a diff**, which a CP1252 file cannot
do. The argument that founded the two factories does not carry over.

## Where the form comes from

**The syntaxes and the headers are transcribed from real files** — the
examples Gaupol publishes with its code for the seven formats, and two files
from the private corpus for MicroDVD and Advanced SSA. Read by a human, never linked:
no test goes to fetch them where they are, and the private corpus is not cited.

**The words, on the other hand, are our own.** The text of a real file is
content, and the repository does not publish any; what is transcribed from a format is its
form.

## What the scene does not carry, and to whom that is owed

**A real-world MicroDVD file often opens with two lines of credits placed at
`{1}{1}`** — the name of the video encoding, the file size — which a
naive reader counts as lines of dialogue. The scene does not carry them: nine renderings of a
same scene cannot have a tenth. The fixture that carries them is owed
to the issue that will write the MicroDVD reader, and its place is
[`malformes/`](../malformes).

## What the harness asks of them

[`format_corpus_test.cpp`](../../unit/core/format/format_corpus_test.cpp) carries
a **table of promises**, one line per format, and **refuses a file in this
directory that does not appear in it** — as `valides/` has refused since #289 a
fixture that no list enumerates.

Three promises, and the second is the reason for the table:

- **the byte, fully** — the file comes back identical, and that means
  something;
- **the byte, but empty** — LRC and TMPlayer. The reading invents ends,
  the writing returns only starts: **the bytes would come back identical whatever
  the reading invented.** The test would pass while proving nothing. These two
  formats therefore carry, in the table, the expected ends **written by hand**;
- **the byte, under a declared condition** — MicroDVD. It counts in frames,
  the model counts in milliseconds ([ADR 0006](../../../../docs/adr/0006-positions-en-millisecondes.md)),
  and the file states no grid.

## Why not in `valides/`

Two reasons, and the second is a verified trap.

**Seven of the nine did not yet open.** `valides/` is the corpus of what
opens and comes back byte for byte; an `.ass` dropped there before an ASS
reader existed would have turned the gate red for the whole phase. All nine
open today, but two still cannot go there: `valides/`
asks only for the bytes, and on TMPlayer and LRC the bytes prove nothing.
It is the table of promises that has what it takes.

**And `valides/` tolerates no file that is not a subtitle.** Since
#289 the directory *is* the list: `validFiles()` keeps every regular file
and the first case requires that it open. This `LISEZMOI.md`, dropped there, would make
the test fail. Here it is exempted by name, which can be read in the harness.

**A shipped format does not move out for all that.** Its rendering stays here, where the
scene is; what changes is the line of the table that concerns it.
