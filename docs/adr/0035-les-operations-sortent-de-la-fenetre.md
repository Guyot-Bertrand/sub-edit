# 0035 — The `Tools` operations leave the window

**Date:** 2026-09-25
**Status:** accepted

Issue [#486](https://github.com/Guyot-Bertrand/sub-edit/issues/486), arising from the analysis of `MainWindow`
requested before phase 11 closed. **Goes back on an alternative that [ADR
0034](0034-trois-collaborateurs-de-la-fenetre.md) had dismissed.**

## Context

ADR 0034 left the `Tools` operations in the window, with this reason: "they are already
thin — a dialog, a core function, `applyOperation` —, and have no state: moving them would
make nothing more testable". Two things have changed since.

- **#461 made the page explicit.** `applyOperation` receives the targeted page and no longer reads `m_page`; the
  target of an operation is read in the selection **of the page** (`ProjectPage::tableSelection`), which is
  the table's when the page is displayed. An operation can therefore run on a page built
  on its own, which was false when 0034 was written.
- **The weight.** After #483, #484 and #485, the thirteen `Tools` operations and the route to the history
  still weighed **about 350 lines** of a 1,851-line `main_window.cpp` — the largest remaining family.
  The user reopened the question by keeping this lead from the analysis.

0034's reason was right on one point: these operations hold almost no state. It no longer was
on the other: without a window, they were not tested at all.

## Decision

**A collaborator `ProjectOperations`**, on the model of 0034, carries:

- the thirteen `Tools` operations — shift, transform, convert, adjust durations, append a file,
  split the project, remove mentions, italics, case, dashes, align, snap to the grid,
  analyse the grid;
- **the single route to the history**, `apply` and `applyQuietly`, with what an operation leaves past the
  end of the film. The `Edit` gestures and search take it too: it has only one path, and that path
  is here;
- the family's only state: the shape of the last duration adjustment, which the settings read and
  write.

**Each operation receives the page** and reads its target in that page's selection. **What belongs to
the screen goes through `ProjectOperations::View`**: the parent of the message boxes, the targeted text, the duration of the page's
film, the status bar's transient message, the file to append and its reading — which stay with
`ProjectFiles` —, the diagnostics panel, the row selection, and opening a project alongside
for the tail of a split.

**The window keeps the `connect` calls**, and what precedes a gesture without a dialog: `commitCellEditor`, which
commits the open cell before italics, case or dashes read their target (#397).

## Alternatives dismissed

- **Leaving `applyOperation` in the window** and taking only the operations out. They would have
  asked for it through the `View`, which would then have carried the heart of the family; and the route they
  all take would not have been tested with them.
- **One class per operation.** Thirteen types for thirteen gestures that share a route, a target and a
  report: the boundary would have become the number of files.
- **Passing the target as an argument** rather than reading it in the page. That was possible, and purer; but
  `Split Project…` needs the selection itself — it shows it during the choice and restores it if the user
  cancels (#462) —, and two ways of finding the target within the same family is one too many.

## Consequences

- **The operations have their tests, without a window** (`project_operations_test.cpp`); those of the window do not
  change.
- `main_window.cpp` goes from 1,851 to 1,506 lines, `main_window.hpp` from 794 to 692; the analysis aimed at
  about 1,450.
- One more interface, and the widest of the four — nine gestures. It reads as the list of what an
  operation cannot do on its own, and that is what justifies each of them.
- **What stays with the window**: the tabs and the list of pages, the `Edit` gestures that manipulate the
  table (insert, remove, merge, split a row, the clipboard), opening files and
  translations, the settings. ADR 0034 said the tabs are its heart; that is still true.
