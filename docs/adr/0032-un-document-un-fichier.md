# 0032 — Keep one file per document

**Date:** 2026-09-20
**Status:** accepted

Decided while scoping phase 11, issue #426.

## Context

Since phase 1, a subtitle has carried **both texts and a single pair of positions**: a
translation has no timing of its own. The model is ready to welcome a second document, and the
history already knows it — it keeps its changes per document, and `Session` can tell whether one
or the other is modified, or has just been saved.

**What is missing is what comes *in*.** `Project` keeps only **one** `SourceFile`: a path, a
format, line endings, an encoding, a header, what the file declares about itself
([ADR 0030](0030-ce-qu-un-document-retient-de-son-fichier.md)). Yet a translation is a file
like the other, and **nothing forces the two to resemble each other**: a main file in SubRip and a
translation in SSA, one in UTF-8 and the other in Windows-1252. Gaupol knows it: it keeps a
`main_file` and a `tran_file`, and reads the tags **of the document's format** when it italicises.

`sourceFile()` is read in twenty-nine places. **Twelve are core operations** that look in it for the
markup dialect — `italics_command` (2), `dialogue_dashes_command` (2), `clipboard` (2),
`search` (3), and `letter_case_command`, `hearing_impaired_removal` and `duration_adjustment` (1 each) —
and all of them read the main file today. That is true as long as there is only one document, and false
the day a translation is in SSA: **two of them already receive a `Document`** and yet read the main
file's format.

## Decision

**`Project` keeps a second `SourceFile`, the translation's, optional.** An accessor takes
a `Document`: `sourceFile(Document)`, and its counterpart for writing.

**Without a translation file, the translation follows the main file's format**: a translation typed by
hand into an empty column will be written like the main file, until a format is chosen for it.

**Eleven of the twelve core sites read the format of the document they are given**, no longer that of
the main file. **The twelfth, duration adjustment, stays on the main file, deliberately**: its reading
speed is computed on `mainText`. The others — the window, opening, saving, the command line — stay as
they are until an issue makes them touch the second document.

## Alternatives dismissed

- **A translation file the project ignores, held by the window.** That is what
  [ADR 0018](0018-vocabulaire-des-formats-dans-le-modele.md) already refused for the format: the
  window would hold, next to its `Session`, a field that `SourceFile` is precisely made for, and
  the question would come back with the second consumer — the command line of phase 13, which has
  no window.
- **A translation that is read, then forgotten as a source.** That would be simpler, and we could no
  longer save it to its path, nor in its format, nor in its encoding: the promise of a faithful round
  trip, the core's only strong promise, would hold only for the main file.
- **A second `Project`, linked to the first.** A translation has no positions of its own, and a second
  project would have some: they would have to be kept in agreement, and **that is exactly the defect
  that "both texts, one pair of positions" was written to avoid**.
- **An array of files indexed by `Document`.** The most general form, and it has only one more use
  case than the chosen form: there are only two documents. The trigger is further down.

## Consequences

**Easy**: saving the translation to its path, in its format; italicising an SSA translation with
`{\i1}`; knowing, per document, whether it is modified — the history already does it.

**Hard, and to be watched**: `sourceFile()` is still read in seventeen places outside those twelve,
and **all of them assume the main file**. That is not a defect as long as they are sites of the main
file — the window, opening a project —; it becomes one the day one of them turns into a text operation
without anyone having reviewed it. The end-of-phase review checks it.

**What the decision costs.** A translation's **per-subtitle** settings — an ASS style, the position
of a WebVTT subtitle — **are not preserved**: a subtitle carries a single set of `FormatExtras`,
the main file's, and aligning a translation line copies only its text. What is lost belongs to a file
whose formatting is almost never reread, but it is a loss, and it is named here rather than
discovered.

**What it would cost to undo it**: little, as long as the accessor takes a `Document` — the day the
second file becomes an array, only its body changes.

**The trigger** that would reopen the question is a **third document** — a reference text, a
second language. Two named documents are no longer a form; at three they are a list.
