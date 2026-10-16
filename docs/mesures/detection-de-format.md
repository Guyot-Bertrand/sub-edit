# Format detection

What the project answers when asked which format a file is in, and **where it
stops knowing**.

**A rate, not "detection works".** This is the statement of #290 for the
encoding, carried over by #340 to the other unknown of phase 9: a detection is
a classifier, it has a success rate, not a result that is right or wrong.

**With two formats, the question was closed in advance.** A file carries the
`WEBVTT` signature or an arrow with a comma, and nothing else comes close.
With nine, it is a classification, and the collisions are real:

| What looks alike | What it stands for |
| :--------------- | :----------------- |
| `.sub` | MicroDVD **and** SubViewer 2 |
| `.txt` | TMPlayer, MPL2, and everything else in the world |
| `{0}{25}` and `[0][25]` | the same grammar, but for the shape of the brackets |
| SSA and ASS | the same header, but for a version and a section name |

## How to replay it

```console
$ make score-format
```

It is a step of `check-local`: the measurement is replayed on every pull
request, and a departure from the record shows. A rate that falls fails, a rate
that rises invites `make score-format-record`, an unchanged rate stays silent.
It is not a threshold: the number compared is the one of the last measurement.

**The detector is an argument**, as for the encoding — the framing (#337) will
have routes to choose between, and the choice between them is a choice of rate.
Measuring it after choosing is choosing without measuring.

```console
$ cmake --build build/dev --target subedit_detect_format
$ ./src/scripts/score-format-detection.py \
      --detector './build/dev/bin/subedit_detect_format {}' --private
```

`--private` adds the private corpus, absent from any machine that does not have
it. No gate reads it.

## Three outcomes, and two of them are not equal

A **right** answer. A **wrong** answer, which opens a file as what it is not. A
**refusal**, which says it does not know.

**The refusal is the project's rule applied** — nothing rather than a guess.
**Confusion is what must be avoided at any price**, and it fails on its own,
whatever the rate: a rate that rises while a confusion appears must not buy it
back.

## The file name is not part of the question

The fixtures are called `scene.microdvd.sub`: their name **carries the
answer**, since a corpus has to be labelled. It is removed before the detector
sees them — each file is handed to it under a neutral name, **keeping its real
extension**, so that `.sub` stays as ambiguous as it is in the wild.

The day the framing decides that the extension is admissible evidence, that is
where it will have to be said, and deliberately.

## Labelled corpus

`src/test/data/formats/`, the same scene in the nine formats — the label comes
with the file, by construction — and `src/test/data/valides/`, ten files that
their extension is enough to name.

<!-- generated record: do not edit by hand -->

    recognised formats: 19/19

Recorded on version 0.9.21, on 2026-09-09.

| | Files |
| :--- | -------: |
| recognised | 19 |
| refused, for want of a reader | 0 |
| **confused** | **0** |

**No confusion**: the detection refuses, it does not get it wrong.

<!-- end of record -->

**Neither refusal nor confusion.** The figure rose by one rendering with each
format delivered, and it was the only one in the repository that measured the
progress of phase 9 rather than a property of the code. It no longer measures
that: the nine formats are there, the labelled corpus is recognised in full,
and what the measurement keeps is what it had that mattered most from the first
day — **a confusion fails on its own.** A full score is not relaxed; it is
watched.

## Private corpus — without labels

`src/data/`, seventy-eight real files. **It gives no rate**: there is no truth
to hold it against, and inventing one would be worse than having none. What it
gives is the distribution of the answers.

| Answer | Files |
| :----- | ----: |
| WebVTT | 54 |
| SubRip | 17 |
| MicroDVD | 1 |
| Advanced SSA | 1 |
| refused | 5 |

**The last two refusals on a real subtitle became recognitions**, a MicroDVD
and an Advanced SSA, both real, both named by the readers of the phase.
It confirms what the initialisation of the phase (#336) had ended up finding
after getting it wrong: the private corpus carries something other than `.srt`
and `.vtt`, and the record that said otherwise filtered its walk on those two
extensions.

**The five refusals are deserved, and they entered the record at the review of
the phase.** They are notes and a text excerpt, carrying the `.txt` extension;
the detection says it does not know, which is the right answer. A refusal is
not a confusion, and only confusion makes it fail.

**They were not there, and their absence was a filter.** The probe did not walk
the `.txt` files, on the grounds that the only ones in the corpus were those
notes. The argument held on the files of one day; what it cost was greater —
`.txt` names **MPL2 and TMPlayer**, so the probe could not see a file of those
two formats, and the sentence below was guaranteed by the filter rather than
measured. It is the exact shape of the defect of #268 and #273: checking with
the tool that does not count. In the only record whose whole value rests on
**the extension playing no part in the decision**, a filter by extension had no
place.

**The private corpus says nothing of TMPlayer or LRC**: it carries none — and
this has been measured since the probe looks at the `.txt` files. What those
two formats have going for them is the scene of #338, and nothing real.

## What the first version of the probe taught

**It handed the raw bytes to `detectFormat`**, and the private corpus answered
nine refusals instead of two: seven perfectly valid WebVTT files were counted
as detection failures.

The cause was not the detection but the order. **`detectFormat` works on
decoded text**, after the encoding has been settled and the mark removed; a
UTF-16 file does not begin with the bytes of `WEBVTT`. The probe therefore opens
the file as the product opens it, rather than calling the classifier beside the
path that feeds it.

It is the defect of #268 in another form — **checking with the tool that does
not count the same thing** — and it is the private corpus that caught it, on
files that no fixture replaced.
