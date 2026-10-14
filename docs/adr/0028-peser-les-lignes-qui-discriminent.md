# 0028 — Detection weighs only the lines that discriminate

**Date:** 2026-09-05
**Status:** accepted

Decided while measuring detection, issue #310.

## Context

[ADR 0027](0027-icu-pour-les-encodages.md) chose ICU to detect and to convert,
on a test bench where it scored 9/9. That bench held only **monolingual**
files: every fixture is written from end to end in a single script.

A real subtitle file is not always so. A subtitled Russian film carries English
title cards; a fansub carries a translator's note; an English subtitle track
carries three typographic apostrophes left by a word processor. In all these
cases, **almost the whole file is ASCII**, and the little that is not is what
carries the whole question.

Issue #310 asked to measure where detection stops being reliable. The
measurement — `score-encoding-detection.py --par-longueur`, and the record in
[`docs/mesures/detection-d-encodage.md`](../mesures/detection-d-encodage.md) —
answered something other than the question asked.

## What was measured

On generated SubRip files, sixty draws per row:

| Written in | Cues | Accented | Bytes | Exact | Script preserved |
| :--------- | ---: | -------: | ----: | ----: | ---------------: |
| KOI8-R | 200 | 3 | 13,195 | **0 %** | **0 %** |
| KOI8-R | 600 | 10 | 39,724 | **0 %** | **0 %** |
| CP1250 | 600 | 1 | 39,815 | 0 % | 63 % |

Sixty times out of sixty, a forty-kilobyte file of which ten cues are Cyrillic
came back as `ISO-8859-1`: the Russian as accented Latin, in full, and the
diagnostic "an encoding nothing declared" announcing `ISO-8859-1` with the
assurance it would have had announcing the right encoding.

**It was not a limit of the available information**, and that is what turns an
inevitability into a defect. On the same file:

| Detector | Answer |
| :------- | :----- |
| ours, ICU | `ISO-8859-1` |
| `uchardet`, control | `KOI8-R` |

ICU's ranking explains the rest. It puts `ISO-8859-1` at a confidence of 66,
labelled "en", and `KOI8-R` at 2, last of the six candidates. Its detector
scores letter statistics over all the bytes: thirteen thousand bytes of English
weigh in it, and the hundred and ninety bytes of Russian weigh nothing. The rule
"the first candidate that actually decodes" can do nothing about it — a
single-byte encoding decodes almost anything, so the first in the ranking
always wins.

## Decision

**ICU receives only the lines that carry a non-ASCII byte.** The rest of the
file is withheld from it before it weighs anything.

The reasoning fits in one sentence: **a line of pure ASCII reads the same way
under every candidate, so it cannot tell them apart.** What it can do, and what
it did, is weigh in a statistic where it has nothing to say. It is not one more
heuristic; it is the removal of what is not information.

The same file, with its ASCII lines removed, puts `KOI8-R` at 54 and no longer
proposes `ISO-8859-1` at all.

**What is weighed is not what is decoded.** The ranking is made on the retained
lines; each candidate is then tested **against the whole file**, because an
encoding that does not read all of it is not an answer. That rule does not
move.

**Two exits return the bytes untouched**, and each closes off a way of harming
rather than helping:

- **a NUL anywhere.** It is a wide encoding — UTF-16 above all — where `0x0a`
  is half a code unit, so cutting there slices a character and not a line end;
  and where a line of pure ASCII is a run of `X\0X\0` that carries no high byte
  and would vanish entirely. Measured: without that exit, a UTF-16LE file
  without a BOM was returned as `UTF-16BE`.
- **nothing removed.** A file accented from end to end has no mass of ASCII to
  remove, and rebuilding it would only normalise its line endings for nothing.

## What the decision costs, written rather than left unsaid

**One more pass over the bytes, and a copy.** It happens only in the branch
where the bytes are not valid UTF-8, that is, never for the vast majority of
files. The bench does not see it.

**Short files lose their timestamps, which were bulk.** On fully accented files
of fewer than six cues, removing the index and time lines divides the weighed
size by three, and CP1250's accuracy rate drops by a few points — 92 % to 85 %
on five cues, within the noise of sixty draws but in the wrong direction. **The
script, however, stays 100 % right**: what drops is the choice between two
Latin encodings, never the choice of alphabet. That is the price, and it is
paid where it costs least.

**The labelled corpus cannot by itself measure what this decision changes.**
`koi8-r-rare.srt` was added for that — twenty English cues and one Russian, the
only fixture whose answer moves from before to after.

## Alternatives dismissed

**Replacing ICU with `uchardet`.** It answers correctly here, and ADR 0027 had
already noted that it detects as well. Dismissed for the reason that had
already dismissed it: it does not convert, so it would be added to ICU instead
of replacing it — one more dependency for a case thirty lines handle.

**Setting a length threshold, and returning `nullopt` below it.** That is what
#310 envisaged, and the measurement dismisses it twice. The real defect is on
forty-kilobyte files, which no lower bound on size sees. And detection is
consulted only when the bytes are **not** valid UTF-8: returning `nullopt`
makes reading fall back on UTF-8, which fails by construction, so the file
would no longer open at all. An imperfect answer that a diagnostic announces is
better than a refusal.

**Weighing both and keeping the better confidence.** ICU returns a confidence
per candidate, and the ranking of the whole file could be compared with that of
the retained lines. Measured: on the bilingual file, `ISO-8859-1` over the whole
file is worth 66 and `KOI8-R` over the retained lines is worth 54. Confidence
rises with the volume weighed, so that comparison would give back precisely the
wrong answer. Two figures computed on two different samples are not
comparable.

**Removing ASCII character by character rather than line by line.** More
aggressive, and wrong: a high letter isolated from its word loses the context
ICU's bigram statistics use. The line is the unit that keeps the whole word
while throwing away what is not one.

## Consequences

**ADR 0027 is not superseded, it is complemented.** ICU remains the detector and
the converter; what changes is what it is given.

**The labelled corpus now carries a bilingual case**, and the score is replayed
on every pull request — issue #311. A regression on this point can no longer go
unnoticed: it makes `make score` fail.
