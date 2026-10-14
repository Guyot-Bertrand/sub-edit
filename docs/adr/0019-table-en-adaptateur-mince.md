# 0019 — Read through the core's model, which reports its changes

**Date:** 2026-08-17
**Status:** accepted

## Context

Phase 5 puts a `QAbstractTableModel` on top of `Project`. Two shapes are on
offer: a **thin adapter**, whose `data()` fetches the data from the project on
every call, or a **model of its own** that holds its copy and resynchronises on
signals.

Gaupol took the second, and its code shows the price. `gaupol/page.py` copies
every subtitle into a `Gtk.ListStore` and keeps it up to date through eight
signal handlers. One reads there:

- `reload_view_all()`, which empties and repopulates everything on every
  opening;
- in `_on_project_subtitles_removed`, `if len(rows) > 50:` **unplugs the model
  from the view**, with this comment: "a large series of live updates made
  directly to the view is slow";
- a `gaupol.util.iterate_main()` after each refresh, to hand control back to
  the event loop.

These are not blunders: it is what duplication costs, paid by the reference,
written in its code.

The core, for its part, **knows no signal mechanism**, and that is deliberate —
`Change` says so in its own comment: "the core returns the information and the
caller does what it wants with it". But `Session::apply`, `undo` and `redo`
return `void`. A window plugged into them has no way of knowing which rows to
refresh, and would only have the choice of redrawing everything.

Finally, Qt offers `QUndoStack`. The core has its own, and it is the one that
is authoritative: the phase 3 command line depends on it too.

## Decision

**The table is a thin adapter.** `data()` reads `project.subtitleAt(index)` and
formats on the fly; no subtitle is copied. The number column is not data but
`row + 1`.

**`Session::apply`, `undo` and `redo` return what they changed** — a
`std::vector<Change>`, the one `describe()` already produces. Undo returns it
**inverted**: undoing an insertion is a removal, and a free function
`invert(ChangeKind)` suffices, the indices being the same in both directions.
No observer, no signal enters the core.

**The core's history is the only one.** `canUndo()` and `canRedo()` drive the
window's two actions; there is no `QUndoStack`.

**A structural change goes through a model reset.** Qt requires an insertion or
removal of rows to be bracketed *before* it takes place — `beginRemoveRows` —
whereas `Session` only returns its report afterwards. Hence:

| What changes | What the model emits |
| :----------- | :------------------- |
| text, positions, reordering | `dataChanged` on the affected ranges |
| rows added or removed | `beginResetModel` / `endResetModel` |

## Alternatives dismissed

- **A synchronised model of its own,** like Gaupol. It duplicates every
  subtitle and makes resynchronisation the place where defects live. Dismissed
  on evidence, not on taste: the three scars quoted above are in `page.py`.

- **`QUndoStack`.** A second undo stack, which would have to be kept in
  agreement with the first. Two sources of truth for the same question, only
  one of which is visible from the command line.

- **An observer in the core** — `Session::setObserver`. It would solve the
  bracketing of structural changes by notifying before and after. It adds to
  the core a mechanism the CLI has no use for, and brings the order of
  notifications into its contract. To be reconsidered if the bracketing becomes
  genuinely bothersome, not before.

- **Describing the command before applying it,** to know in advance the
  structure touched. Impossible in general: `removeHearingImpaired` only knows
  which subtitles the rule empties once the rule has been applied. A solution
  that works for some commands and not for others is not one.

## Consequences

**Made easy.** Refreshing what moves and nothing else, over several thousand
rows. A single authority for undo, shared with the command line. A core that
stays free of Qt, which `check-architecture.sh` already checks.

**Made hard.** A structural change costs a rebuild of the view and loses the
selection. That is tenable as long as its only producer is the removal of
hearing-impaired mentions — global, rare, and followed by a refresh anyway.

**What it costs to undo.** The thin adapter, almost nothing: the model is thin
by definition. The report returned by `Session`, more — three public
signatures that the CLI compiles too.

**Trigger for a review.** Phase 7 brings inserting and removing subtitles from
the window. A full reset for every row added would then be ridiculous, and that
is where `Session` will have to announce a structural change before making it.
Measure it before building it.

> **The trigger has fired, and the answer is no — 2026-08-30,
> [#264](https://github.com/Guyot-Bertrand/sub-edit/issues/264).** Phase 7
> delivered insertion and removal, decision D8 of its spec examined the
> question, and the end-of-phase review closed it.
>
> "Measure it before building it" was followed to the letter, and the
> measurement concluded: resetting the model after a removed row is of the
> order of **10 µs on four thousand subtitles**, against a human gesture. The
> view's share is not measurable on the bench — it has no `QApplication`.
> **Building an optimisation whose benefit is not measurable is exactly what
> this ADR asked to avoid**, so it is not built, and the adapter stays thin.
>
> What would reopen the question is no longer a measurement but a need: a
> producer that changes the structure faster than a human clicks — a batch
> correction, an import. None exists; the day one exists, it will carry the
> question with it.
