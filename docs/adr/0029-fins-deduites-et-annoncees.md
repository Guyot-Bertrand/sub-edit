# 0029 — Deduce the ends the file does not carry, and announce it

**Date:** 2026-09-07
**Status:** accepted

Decided while scoping phase 9, issue #337.

## Context

**Two of the nine formats carry only one position per line.** LRC writes
`[00:12.34]text`, TMPlayer writes `00:00:12:text`: the start of a cue, and
nothing else. Neither has a field for the end.

Gaupol reads these files by setting a cue's end at the start of the next one,
and by giving the last one five seconds — `subtitles[-1].duration_seconds = 5`,
the same line in `lrc.py` and in `tmplayer.py`.

Our `Subtitle` carries `start` **and** `end`, both non-optional, since phase 1.
The question is whether a file that does not state the end has the right to
make the model carry a value it has not read.

It is not rhetorical: the model has so far refused to invent. It tolerates an
`end` earlier than the `start` rather than correcting it, it refuses to sort on
its own ([0012](0012-ordre-des-sous-titres-par-composition.md)), and its rule on
opening is to report rather than repair
([0008](0008-lecture-au-mieux-avec-diagnostics.md)).

## Decision

**Ends are deduced on reading** — a cue's end is the start of the next one, the
last one receives five seconds — **and reading emits a diagnostic that says
so.**

The model does not change. `Subtitle::end` remains a `Timestamp`.

## Alternatives dismissed

### Making `end` optional

It is the route that tells the truth, and that is why it was examined before
being dismissed.

The count is not what decides, but it gives the scale: **twenty-eight places**
in `src/lib` and `src/exe` read or write `end` directly, six call
`duration()`, and `position(Boundary::End)` returns a reference that can no
longer be one.

**What decides is that there is nothing to decide twenty-eight times.** What
does a shift do to a subtitle without an end? Nothing. What does the Duration
column show? Nothing. What does an overlap mean when one of the two has no end?
The question does not arise. Each of these answers is a void that would have to
be written, tested and maintained — and not one teaches the user anything.

**And the state survives nothing.** It arises from two formats out of nine, and
disappears as soon as the document is saved in another: SubRip, WebVTT,
SubViewer 2, SSA, ASS, MicroDVD and MPL2 all require an end. Making the whole
core carry a case that seven formats out of nine cannot represent is a bad
trade.

### Refusing both formats

Contradicts iso-functionality, which is the purpose of the project. Dismissed
without further ado.

### Deducing silently, as Gaupol does

It is Gaupol's route, and **the only thing this decision changes in it** is
the silence. A user who opens an LRC sees an End column filled with
plausible values, and nothing tells them that none of them comes from their
file. If they correct one, they are correcting an invention.

[ADR 0008](0008-lecture-au-mieux-avec-diagnostics.md) already settles this
kind of case the other way, and the vocabulary has existed since phase 1: a
`Diagnostic` carried by reading, shown by the window and by `--verbose` on the
three subcommands.

## Consequences

**The byte round trip stays exact, and it proves nothing.** Writing returns only
starts, so everything that was deduced is deduced again on the next read and the
file comes back identical — **whatever the invented value.** A test that
compared only bytes would stay green if the deduction put the ends anywhere at
all.

That is what #338's table of promises calls **the vacuous byte check**, and why
these two formats carry in it their four expected ends written by hand. The
property that covers them is not the byte comparison, it is that expectation.

**The last cue's five seconds are arbitrary, and stay so.** The choice is
Gaupol's, kept for iso-functionality. A cleverer rule — the file's median
duration, a reading speed — would be less predictable without being any truer,
and phase 10 brings duration adjustment anyway to whoever wants better.

**Conversion, on the other hand, really loses.** Converting a SubRip to LRC
throws the ends away, and that loss is not deduced but suffered: it is the
phase's degradation policy, and `docs/mesures/conversion.md` declares it.
