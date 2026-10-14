# 0030 — What a document keeps from its file, frame rate included

**Date:** 2026-09-07
**Status:** accepted

Decided while scoping phase 9, issue #337.

## Context

A `subedit` document already keeps from its file what no subtitle carries: its
**format**, its **encoding**, its **byte order mark**, its **line endings** and
its **header**, a free-form string. That is what makes the byte round trip
possible — writing puts back what reading found.

The seven formats of phase 9 add three things to this list, and the third is
the heaviest.

**The order of SSA event fields.** An SSA file declares its columns:
`Format: Marked, Start, End, Style, Name, MarginL, MarginR, MarginV, Effect, Text`.
Gaupol keeps it in `event_fields` because the order varies from one producer to
another. Forgetting it would rewrite the whole file with other columns.

**A shape setting, `two_digit_hour`.** TMPlayer is written `0:00:12:` or
`00:00:12:` depending on the file, and Gaupol remembers which.

**MicroDVD's frame rate.** It is of another nature, and it is the heart of this
decision. MicroDVD does not count in time: its positions **are** frame numbers —
`{1194}{1285}`. The model, for its part, counts in whole milliseconds since
[ADR 0006](0006-positions-en-millisecondes.md). Reading a MicroDVD therefore
requires a frame rate, and **no MicroDVD file declares one.**

Phase 16's deduction is of no help: `deduceFrameRate` looks for a grid in
milliseconds, and consecutive frame numbers fall on every grid at once.

## Decision

**One `variant` per format at the file level**, the twin of `FormatExtras` at
the subtitle level, carries what a file declares about itself and no subtitle
carries. The free-form header remains the string it is.

**The frame rate of a frame-based file goes into it**, and writing takes it back:
that is what makes the round trip exact whatever its value.

**It has three sources, in this order**, and reading always says which one was
used:

1. what the user says — `--frame-rate`, or the window's dialog;
2. what the associated video declares, when there is one;
3. **a declared default, 23.976**, announced by a diagnostic.

## Alternatives dismissed

### Keeping frames in the model

That would be Gaupol's `modes.FRAME`, and it flatly contradicts
[ADR 0006](0006-positions-en-millisecondes.md), on which the whole core depends:
every operation, every undo command, every anomaly and the entire table reason
in milliseconds. A second position mode would double the surface of the model
for one format out of nine.

[ADR 0011](0011-numero-d-image-en-type-fort.md) already settled the form a
frame takes here: a strong type `Frame`, which passes through a conversion and
does not settle in the model.

### Refusing to open for lack of a frame rate

It is the most honest route on paper — we invent nothing — and it was dismissed
on usage. A file that has just been double-clicked must open. An announced value
that can be corrected in one action is better than a refusal that leaves the
window empty and teaches nothing.

It would also contradict the house rule: reading opens as best it can and
reports ([0008](0008-lecture-au-mieux-avec-diagnostics.md)). Refusal is reserved
for what is not readable at all.

### Guessing the frame rate from the content

There is nothing to guess. A `{1194}{1285}` is compatible with 23.976 as well as
with 60; what we would read is not a frame rate but a preference.

### A default of 25

25 is the PAL frame rate, Europe's, and probably the most frequent on the files
this project will encounter. **23.976 is Gaupol's**, and iso-functionality is
the only reason that decides between two equally arbitrary figures. A file
opened by both tools must give the same timestamps, otherwise the comparison
that underpins the whole project no longer means anything.

### Putting everything in the header

`header` is a string, and it is enough for what a round trip must give back,
since nothing needs to understand it. It is not enough for `event_fields`:
writing must **read back** the column order to compose each `Dialogue:` line,
and therefore rebuild it by parsing the string on every write. That would be
storing structured data in free text only to parse it again right away.

## Consequences

**`FormatExtras` does not become a nine-branch variant.** Five of the nine
formats carry nothing per subtitle — SubViewer 2, MicroDVD, MPL2, TMPlayer and
LRC stay on `std::monostate` — and SSA and ASS share a branch. It goes from
three to four. The compiler guard its comment claims holds without changing
shape.

**A frame-based document has a frame rate, a time-based document has none.** The
distinction is carried by the variant, hence by the compiler: asking a SubRip
document for its frame rate does not compile.

**Converting to a frame-based format requires a frame rate that does not exist
in the source.** It then comes from `--frame-rate`, or from the deduced grid —
and it is the only place in the project where phase 16 serves something other
than informing: the grid of a time-based file *is* the frame rate it was
timed to. Without either, the conversion refuses rather than inventing a figure
that would move every cue.
