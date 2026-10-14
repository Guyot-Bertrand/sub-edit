# Conversion between formats

What a conversion loses, **measured** pair by pair, and what each format forces
to be lost, **declared** alongside.

**A loss, not a result.** [ADR 0009](../adr/0009-texte-en-chaine-brute.md) has
announced since phase 1 that a structured model serves as a pivot "only during
a conversion between formats, where the loss is inevitable and **accepted**".
*Accepted* assumes it is known; that is what issue #339 wrote into
`measure-conversion-loss.py`, on the model of #290 for encoding detection.

## How to replay it

```console
$ make conversion
```

It is a step of `check-local`: the measurement is replayed on every pull
request, and **a departure from the record below shows**. A loss that grows
fails, a loss that shrinks invites `make conversion-record`, an unchanged loss
stays silent. It is not a threshold: the number compared is the one of the last
measurement, never a bar someone has set.

To see what does not come back, file by file:

```console
$ ./src/scripts/measure-conversion-loss.py --diff
```

**The binary is an argument**, as the detector is for the encoding score: the
measurement bears on the `convert` subcommand the user types, and two binaries
compare on the same corpus without touching the tool.

```console
$ ./src/scripts/measure-conversion-loss.py --binary ./build/release/bin/subedit-cli
```

## What is measured

**A round trip, and the starting format is the yardstick.** A conversion cannot
be looked at from outside: the resulting file is in another format, and there
is nothing to compare it to. A file leaves as `A`, goes through `B`, comes back
as `A` — what does not come back is what `B` could not carry.

The measurement is exact only thanks to a property that the harness of #338
holds elsewhere: **a file of the corpus is already what our writing produces**,
byte for byte. Without it, a difference could come from the layout rather than
from the passage.

The corpus is that of `src/test/data/` — `valides/`, ten files that look
nothing alike, and `formats/`, the same scene written nine times. The private
corpus is not read: absent from one machine in two, it would make the same gate
say two things.

<!-- generated record: do not edit by hand -->

    intact round trips: 58/126

Recorded on version 0.9.22, on 2026-09-09.

| From \ Through | `ass` | `lrc` | `microdvd` | `mpl2` | `srt` | `ssa` | `subviewer2` | `tmplayer` | `vtt` |
| :--- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| `ass` | — | 0/1 | · | 0/1 | 0/1 | 0/1 | 0/1 | 0/1 | 0/1 |
| `lrc` | 1/1 | — | · | 1/1 | 1/1 | 1/1 | 1/1 | 1/1 | 1/1 |
| `microdvd` | · | · | — | · | · | · | · | · | · |
| `mpl2` | 1/1 | 0/1 | · | — | 1/1 | 1/1 | 1/1 | 0/1 | 1/1 |
| `srt` | 5/8 | 0/8 | · | 4/8 | — | 5/8 | 5/8 | 0/8 | 6/8 |
| `ssa` | 0/1 | 0/1 | · | 0/1 | 0/1 | — | 0/1 | 0/1 | 0/1 |
| `subviewer2` | 0/1 | 0/1 | · | 0/1 | 0/1 | 0/1 | — | 0/1 | 0/1 |
| `tmplayer` | 1/1 | 0/1 | · | 1/1 | 1/1 | 1/1 | 1/1 | — | 1/1 |
| `vtt` | 3/4 | 0/4 | · | 3/4 | 3/4 | 3/4 | 3/4 | 0/4 | — |

| File | Through | First line that does not come back |
| :--- | :------ | :---------------------------------- |
| `valides/balises.srt` | `vtt` | `<b>ATTENTION</b> — <font color="#ffff00">zone interdite</font>` |
| `valides/balises.srt` | `mpl2` | `<b>ATTENTION</b> — <font color="#ffff00">zone interdite</font>` |
| `valides/balises.srt` | `tmplayer` | `00:00:01,000 --> 00:00:03,000` |
| `valides/balises.srt` | `lrc` | `00:00:01,000 --> 00:00:03,000` |
| `valides/cadence.srt` | `subviewer2` | `01:00:00,017 --> 01:00:02,000` |
| `valides/cadence.srt` | `ssa` | `01:00:00,017 --> 01:00:02,000` |
| `valides/cadence.srt` | `ass` | `01:00:00,017 --> 01:00:02,000` |
| `valides/cadence.srt` | `mpl2` | `00:00:01,010 --> 00:00:02,020` |
| `valides/cadence.srt` | `tmplayer` | `00:00:01,010 --> 00:00:02,020` |
| `valides/cadence.srt` | `lrc` | `00:00:01,010 --> 00:00:02,020` |
| `valides/complet.vtt` | `srt` | `WEBVTT - Dialogue` |
| `valides/complet.vtt` | `subviewer2` | `WEBVTT - Dialogue` |
| `valides/complet.vtt` | `ssa` | `WEBVTT - Dialogue` |
| `valides/complet.vtt` | `ass` | `WEBVTT - Dialogue` |
| `valides/complet.vtt` | `mpl2` | `WEBVTT - Dialogue` |
| `valides/complet.vtt` | `tmplayer` | `WEBVTT - Dialogue` |
| `valides/complet.vtt` | `lrc` | `WEBVTT - Dialogue` |
| `valides/coordonnees.srt` | `vtt` | `00:00:01,000 --> 00:00:03,000  X1:040 X2:600 Y1:020 Y2:460` |
| `valides/coordonnees.srt` | `subviewer2` | `00:00:01,000 --> 00:00:03,000  X1:040 X2:600 Y1:020 Y2:460` |
| `valides/coordonnees.srt` | `ssa` | `00:00:01,000 --> 00:00:03,000  X1:040 X2:600 Y1:020 Y2:460` |
| `valides/coordonnees.srt` | `ass` | `00:00:01,000 --> 00:00:03,000  X1:040 X2:600 Y1:020 Y2:460` |
| `valides/coordonnees.srt` | `mpl2` | `00:00:01,000 --> 00:00:03,000  X1:040 X2:600 Y1:020 Y2:460` |
| `valides/coordonnees.srt` | `tmplayer` | `00:00:01,000 --> 00:00:03,000  X1:040 X2:600 Y1:020 Y2:460` |
| `valides/coordonnees.srt` | `lrc` | `00:00:01,000 --> 00:00:03,000  X1:040 X2:600 Y1:020 Y2:460` |
| `valides/crlf-bom.srt` | `tmplayer` | `00:00:01,000 --> 00:00:03,000` |
| `valides/crlf-bom.srt` | `lrc` | `00:00:01,000 --> 00:00:03,000` |
| `valides/heures.vtt` | `tmplayer` | `00:00:01.000 --> 00:00:03.000` |
| `valides/heures.vtt` | `lrc` | `00:00:01.000 --> 00:00:03.000` |
| `valides/mentions.srt` | `tmplayer` | `00:00:01,000 --> 00:00:03,000` |
| `valides/mentions.srt` | `lrc` | `00:00:01,000 --> 00:00:03,000` |
| `valides/minimal.srt` | `tmplayer` | `00:00:01,000 --> 00:00:03,500` |
| `valides/minimal.srt` | `lrc` | `00:00:01,000 --> 00:00:03,500` |
| `valides/minimal.vtt` | `tmplayer` | `00:01.000 --> 00:03.500` |
| `valides/minimal.vtt` | `lrc` | `00:01.000 --> 00:03.500` |
| `valides/trois.srt` | `subviewer2` | `00:00:05,001 --> 00:00:07,000` |
| `valides/trois.srt` | `ssa` | `00:00:05,001 --> 00:00:07,000` |
| `valides/trois.srt` | `ass` | `00:00:05,001 --> 00:00:07,000` |
| `valides/trois.srt` | `mpl2` | `00:00:05,001 --> 00:00:07,000` |
| `valides/trois.srt` | `tmplayer` | `00:00:01,000 --> 00:00:03,000` |
| `valides/trois.srt` | `lrc` | `00:00:01,000 --> 00:00:03,000` |
| `formats/scene.ass` | `srt` | `Title: The scene, in nine formats` |
| `formats/scene.ass` | `vtt` | `Title: The scene, in nine formats` |
| `formats/scene.ass` | `subviewer2` | `Title: The scene, in nine formats` |
| `formats/scene.ass` | `ssa` | `Title: The scene, in nine formats` |
| `formats/scene.ass` | `mpl2` | `Title: The scene, in nine formats` |
| `formats/scene.ass` | `tmplayer` | `Title: The scene, in nine formats` |
| `formats/scene.ass` | `lrc` | `Title: The scene, in nine formats` |
| `formats/scene.mpl2.txt` | `tmplayer` | `[10][30]The harbour wakes before\|the town does.` |
| `formats/scene.mpl2.txt` | `lrc` | `[10][30]The harbour wakes before\|the town does.` |
| `formats/scene.srt` | `tmplayer` | `00:00:01,000 --> 00:00:03,000` |
| `formats/scene.srt` | `lrc` | `00:00:01,000 --> 00:00:03,000` |
| `formats/scene.ssa` | `srt` | `Title: The scene, in nine formats` |
| `formats/scene.ssa` | `vtt` | `Title: The scene, in nine formats` |
| `formats/scene.ssa` | `subviewer2` | `Title: The scene, in nine formats` |
| `formats/scene.ssa` | `ass` | `Title: The scene, in nine formats` |
| `formats/scene.ssa` | `mpl2` | `Title: The scene, in nine formats` |
| `formats/scene.ssa` | `tmplayer` | `Title: The scene, in nine formats` |
| `formats/scene.ssa` | `lrc` | `Title: The scene, in nine formats` |
| `formats/scene.subviewer2.sub` | `srt` | `[TITLE]The scene, in nine formats` |
| `formats/scene.subviewer2.sub` | `vtt` | `[TITLE]The scene, in nine formats` |
| `formats/scene.subviewer2.sub` | `ssa` | `[TITLE]The scene, in nine formats` |
| `formats/scene.subviewer2.sub` | `ass` | `[TITLE]The scene, in nine formats` |
| `formats/scene.subviewer2.sub` | `mpl2` | `[TITLE]The scene, in nine formats` |
| `formats/scene.subviewer2.sub` | `tmplayer` | `[TITLE]The scene, in nine formats` |
| `formats/scene.subviewer2.sub` | `lrc` | `[TITLE]The scene, in nine formats` |
| `formats/scene.tmplayer.txt` | `lrc` | `00:00:01:The harbour wakes before\|the town does.` |
| `formats/scene.vtt` | `tmplayer` | `00:01.000 --> 00:03.000` |
| `formats/scene.vtt` | `lrc` | `00:01.000 --> 00:03.000` |

**26 pair(s) the conversion refuses to make** — a timed file to a frame-based format, with no grid and no frame rate given. A refusal is not a loss, and so does not enter the count above.

<!-- end of record -->

## The column that does not fill, and what it says

**There is no coming back to MicroDVD.** Its column is empty, and so is its
row, because a round trip that leaves it cannot get back in: the intermediate
file has lost the frame rate, and the conversion refuses to invent one —
`convert --to microdvd` then asks for `--frame-rate`.

It is **the clearest declared loss of the nine formats**, and the only one that
is not counted in losses. A lost header leaves a file; a lost frame rate leaves
an impossible conversion.

These pairs are therefore counted separately. **A refusal is not a loss**:
counting them as such would say the opposite of what they are, and inflate the
denominator of a figure that would no longer measure anything.

## What the number is worth, and what it is not

**It knowingly mixes two things**: what a format *cannot* carry, and what our
conversion would lose without being forced to. Separating them would require
declaring the seventy-two cells by hand — and a declaration written by hand
goes stale in silence, which #289 corrected for the corpus and which nothing
would force us to start again here.

What it guarantees is narrower, and stands on its own: **a loss does not grow
without anyone seeing it.** What it is worth in principle reads below, in prose
— and it is there, not in the number, that the degradation policy is decided.

**And it falls once, with the arrival of TMPlayer and LRC**: 47/80 becomes
60/126, a share that drops from three fifths to less than half. Nothing
regressed. Two formats that carry no end entered the matrix, and **every pair
that crosses them loses its ends by construction** — sixteen columns and two
rows more, nearly all losing in advance. The record was therefore re-recorded
knowingly, which is what the check asks to do rather than suffer.

It is also what shows the limit of the single figure, and why the matrix is
there: **a row can be read, a total cannot.** The `lrc` and `tmplayer` rows are
full — one leaves those formats without losing anything, since they have
nothing more to lose. It is their columns that are empty.

## It falls a second time, and this time it is progress

**60/126 becomes 58/126 the day the tag pivot exists**, and the two pairs lost
are the two pairs that had to be lost.

**The byte round trip rewards whoever does nothing**, and this is where it
shows. Before the pivot, converting a coloured `.srt` to WebVTT copied
`<font color="#ffff00">` as it was: the file came back identical, and the
`.vtt` produced carried a tag that WebVTT does not define. The measurement
counted that pair intact because it had translated nothing. The pivot removes
the colour, since WebVTT has none — the `.vtt` is at last WebVTT, and the round
trip no longer comes back. **The intermediate file became correct, and the
measurement fell for that very reason.**

The second pair says the same thing another way. MicroDVD and MPL2 have no tag
that stops: `{y:i}` slants to the end of the line. A line only half of which
was bold is therefore written without bold — rather than with a bold that
spills over the rest, which a copy would have given.

**This is what ADR 0031 calls the soiling, seen from the other side.** Writing
`{\i1}` in a `.srt` makes a file that comes back byte for byte and that the
user reads wrongly; translating it makes a correct file and a round trip that
loses. The measurement prefers the first, and it is wrong — it is the prose
that decides, as this record has said from the start.

**What remains true, and it is all that is asked of it**: the number does not
move on its own. Someone had to come here and write it.

## The declared loss

What follows is **a property of the formats, not of our code**. It is written
before the first line of a reader, and it is what the framing of phase 9 (#337)
decides its degradation policy on.

### What crosses by translation, since the pivot

**Six things**, and they are Gaupol's: bold, italic, underline, colour, font
and size. They are decoded from the starting vocabulary, carried in a model
that lives only as long as the conversion, and rewritten in the target
vocabulary — [ADR 0031](../adr/0031-pivot-de-balises-a-la-conversion.md).

| Vocabulary | Formats | Italic is written |
| :--------- | :------ | :---------------- |
| HTML | SubRip, WebVTT, SubViewer 2 | `<i>…</i>` |
| Sub Station Alpha | SSA, Advanced SSA | `{\i1}…{\i0}` |
| MicroDVD | MicroDVD | `{Y:i}`, to the end |
| MPL2 | MPL2 | `/` at the start of the line |
| none | TMPlayer, LRC | nowhere |

**A shared vocabulary is not a shared power.** SubRip and WebVTT write italic
the same way and do not agree on colour; Sub Station Alpha does not write the
underline that Advanced SSA writes. What each format can say is therefore a
second table, and it is that table that decides the loss.

**Two vocabularies cannot style part of a line**, for want of a tag that stops.
A line whose runs do not agree is written with no style at all in MicroDVD and
in MPL2: better to lose than to spill over.

### What never crosses a format boundary

**The data specific to one subtitle** — SubRip's coordinates, the identifier
and settings of a WebVTT cue, the named style, the margins and the effect of an
SSA event. Each format has its own, and they do not translate:
`align:start position:10%` has no SubRip equivalent, `X1:040 X2:600` has none
in WebVTT.

**The header**, for the same reason. `[Script Info]` is not a WebVTT header,
and the free text that follows `WEBVTT` is not SSA.

**The layout**, which the pivot leaves out — `{\pos(x,y)}`, `{\an8}`,
`<v Marie>`, `<ruby>`, WebVTT's internal timestamps. They cross a round trip in
their own format intact, where nothing is decoded, and disappear on
conversion. Translating it would require a layout model that nothing else in
the project calls for, and most of them have an equivalent nowhere.

They can be read in the measurement: `valides/coordonnees.srt` loses its
coordinates going through WebVTT, `valides/complet.vtt` loses its header, its
`STYLE` block, its note, the identifier of its first cue, its settings **and
its two voice tags** going through SubRip.

### What crosses, except where the target format cannot carry it

| Format | The end of a cue | Line breaks | Italic | Precision |
| :----- | :--------------- | :---------- | :----- | :-------- |
| SubRip | carried | carried | `<i>…</i>` | millisecond |
| WebVTT | carried | carried | `<i>…</i>` | millisecond |
| SubViewer 2 | carried | `[br]` | `<i>…</i>` | hundredth |
| Sub Station Alpha | carried | `\N` | `{\i1}…{\i0}` | hundredth |
| Advanced SSA | carried | `\N` | `{\i1}…{\i0}` | hundredth |
| MicroDVD | carried, **in frames** | `\|` | `{Y:i}` | frame |
| MPL2 | carried | `\|` | `/` at the start of the line | tenth |
| TMPlayer | **absent** | `\|` | **no vocabulary** | second |
| LRC | **absent** | **absent** | **no vocabulary** | hundredth |

**Two formats have no end.** LRC and TMPlayer carry only one position per line;
a reading invents the ends, and the invented end is what comes out of a round
trip. It is the "empty" promise of the table of #338, seen from the other side.

**LRC carries no line break**: a cue of two lines comes back as one, and the
words are joined by a space.

**MicroDVD counts in frames**, and states no grid. A conversion to it assumes a
frame rate that the file will not tell on the next reading — the conditional
promise of #338, and the question the framing has to settle.

**A bound the table does not show, and where we go further than Gaupol.** LRC
writes `mm:ss.cc` and has no hours field; so it is the minutes that carry them,
and `[62:03.00]` is one hour and two minutes. Gaupol's pattern takes exactly two
digits of minutes: beyond 99 minutes 59 it no longer reads the file back, and
its writing, which cuts the `HH:MM:SS` string after the hours, writes
`02:03.00` for the same position — a cue moved by an hour, in silence.

Our reading takes the minutes for what they are, however long they are, and our
writing renders them whole. The round trip is then exact wherever one is in the
film, which is the only thing the format asked for. The sign is accepted on
both sides, as in Gaupol.

**Precision, on the other hand, is measured — and not where it was expected.**
The scene of #338 puts all its positions on the whole second, precisely so that
it is the same scene in the poorest of the nine; it could therefore say nothing
about it. It is `valides/` that exercises it, without having been written for
it: `cadence.srt` carries a position at `01:00:00,017`, and it comes back at
`,020` from a passage through SubViewer 2. The corpus that looks like nothing
finds what the regular corpus cannot find, and that is the reason for keeping
both.

## What did not enter the measurement

Seven of the nine renderings of the scene did not open at the start of phase 9:
the readers were not written. The record counted them separately rather than
keeping quiet about them, and their number went down by one with each format
delivered, to zero. The line disappears from the record when there is nothing to
count; the counter stays, because the question will come up again with the
first corpus added.
