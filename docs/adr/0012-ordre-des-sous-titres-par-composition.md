# 0012 — Never sort unprompted, and make strict mode a composition

**Date:** 2026-08-08
**Status:** accepted

## Context

A subtitle whose start is corrected can end up placed before the one that
precedes it. The question arises before writing the first editing operation:
is "sorted by start" an invariant of the project?

Gaupol answers yes, and holds it in two places. On opening, it sorts, counts
the subtitles moved and opens a dialog — *"Open unsorted file? The order of N
subtitles needs to be changed"* — which the user may decline. On editing,
`set_start` calls `_move_if_needed`, which moves the row.

**This invariant is not one.** `replace_positions`, through which shifting,
transforming and frame-rate conversion go, does not sort: shifting part of a
selection can put the file out of order without anything noticing. And the
count on opening counts in frames what the sort compares in seconds; two
subtitles inverted by less than a frame are therefore reordered without a
word, precisely in the case hardest to spot by eye.

Our reader, for its part, does not sort and guarantees a byte-for-byte faithful
round trip.

## Decision

**The core never sorts on its own.** The order is the file's, an edit moves no
row, and disorder produces a diagnostic on reading — `OutOfOrder` — like any
other anomaly.

Two modes are offered to the user, carried by the session:

- **lenient**, by default — nothing moves unless asked;
- **strict** — after an operation that may break the order, the session chains
  a **sort command into the same composite**.

Strict mode is thus a **composition policy**, not an invariant of the model.
Sorting remains an ordinary, undoable command, counting as a single history
entry together with the operation that triggered it.

## Alternatives dismissed

- **An invariant held by `Project`**, the Gaupol way. Dismissed on cost:
  indices would move under the caller's feet, `Change` would have to express a
  move and undo restore an *order* rather than a value. Every command would pay
  that price, permanently, for a rare case.
- **Two invariants depending on the mode.** That would not be a setting but two
  data models: every command would have to ask which one it runs under, and
  code written under one would be wrong under the other.
- **Sort on opening, with a warning.** That is Gaupol's answer. Dismissed for
  two reasons: it would break the byte-for-byte round trip, the library's only
  strong promise today; and a dialog is an *event*, which does not survive its
  closing, whereas a diagnostic is a *state* that persists in the reading
  result.
- **Refuse to open a file that is too disordered**, which Gaupol offers.
  Dismissed: the threshold would be arbitrary, and ADR 0008 states that the
  core opens as best it can and reports.

## Consequences

**Indices remain stable within a command.** That is the load-bearing property:
`Change` carries `SubtitleIndex` values, the history replays commands, the
interface will refresh by index. No operation has to wonder whether it moved
something.

**Sorting becomes undoable**, which it is not in Gaupol, where it precedes the
existence of the history.

**The modification counter tells the truth.** Opening a disordered file in
strict mode marks the document modified — which it is, since it differs from
the disk. Gaupol modifies in memory with nothing showing it until saving.

In return, a subtitle may end up displayed out of order in lenient mode. The
interface must make that visible, and `Project::outOfOrder()` gives it the
means without the rule living in the interface.

When the sort triggers, the composite moves rows and its `Change` must express
that. The difference from an invariant is that this case comes from **one
identified command**, handled once, instead of weighing on all of them.

Undoing this decision would mean making the order invariant, hence reworking
every command. The trigger would be a use showing that users expect automatic
moving — in which case strict mode, already there, would become the default
before an invariant is considered.
