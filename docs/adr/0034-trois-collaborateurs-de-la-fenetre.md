# 0034 — Three collaborators of the window, which receive the page they target

**Date:** 2026-09-24
**Status:** accepted

Issue [#460](https://github.com/Guyot-Bertrand/sub-edit/issues/460), opened by the end-of-phase-11
review ([#441](https://github.com/Guyot-Bertrand/sub-edit/issues/441)).

## Context

`MainWindow` weighed **1,951 lines** when phase 11 opened, and **2,968** at its review. [ADR
0033](0033-un-projet-est-une-page.md) took out of the window what a project **keeps** — `ProjectPage` —,
not what a project **undergoes**: everything the third batch learned to do was added to the
window, and each of those methods reads `m_page`, the page of the current tab.

Two consequences, and the second is the costlier:

- a single class of 94 methods, where what opens a file sits next to what moves a column;
- **none of it can be tested without building the whole window** — menus, player, status bar —,
  because nothing can target a project other than by making it the current tab.

## Decision

**Three families leave the window, each in a type of its own.** The criterion for the cut: a family
holds a state of its own, and talks to the rest of the window only through a small number of named gestures.

| Type | What it carries | What it keeps |
| :--- | :-------------- | :------------ |
| `ProjectFiles` | open, save, save as, the question about unsaved changes on one or several projects, `Save All`, reading a translation, sorting a drop | the directory of the last file opened or saved |
| `ProjectSearch` | the search dialog, the target, find and replace in one project or in all of them | the two options, and the dialog once opened |
| `TableColumns` | the entries of `View ▸ Columns`, the translation column, hiding, the order, the widths, the text the current cell targets | the widths of hidden columns |

**Each receives the page it targets** — a `ProjectPage&`, or the list of pages —, and never reads the
current page. That is what makes them testable without a window: a page builds on its own
(`ProjectPage::make`), and a test passes one in.

**What they ask of the window goes through one interface per collaborator**, and only what they
cannot do themselves because it belongs to the screen:

- `ProjectFiles::View` — bring a page's tab to the front (a dialog must open on the
  project it concerns), say that a document has just been written (title, film, actions), and the widget
  on which to place the message boxes;
- `ProjectSearch::View` — the pages in tab order and the one being shown, bring one to the
  front, the targeted text and the target the selection indicates, go to a match,
  apply a command to a page;
- `TableColumns` has none: it knows only the table.

The window implements them; a test passes in a double.

**What stays with the window**: the tabs and the list of pages, the menus and their actions, the
player, the status bar, the title, and the operations of `Tools` and `Edit`, which all go through
`applyOperation` and hold no state of their own.

> **Completed on 2026-09-25, [#483](https://github.com/Guyot-Bertrand/sub-edit/issues/483): the actions
> leave in turn**, into `WindowActions`. It builds the thirty-nine actions, their shortcuts and
> their initial state, and places them in the menus and the toolbar; **it connects nothing** — what
> each gesture does remains a slot of the window, which wires it — and does not decide when an action
> is disabled, which `refreshActions` recomputes after each operation. The boundary is therefore not that
> of the three collaborators above: no state of its own, no targeted page, no `View` interface. What
> it carries is what no longer changes once built, and that is what can be tested without a window — on
> a bare `QMainWindow`. An aggregate of pointers read by name (`m_actions->undo`) rather than a
> class with accessors: the window's test accessors already exist and return those pointers,
> and no window test was rewritten. The window's constructor goes from 548 to 269 lines,
> `main_window.cpp` from 2,531 to 2,216, `main_window.hpp` from 908 to 872.

> **Completed on 2026-09-25, [#484](https://github.com/Guyot-Bertrand/sub-edit/issues/484): the video
> leaves, and it does belong to the family of the first three.** `VideoPane` owns what the video holds
> of its own — the shared player, the native surface and the `Select Video…` strip, the timer, the page
> for which the player is playing — and each gesture receives the page it targets. It reads the table it is
> given (the selection, the current row, the open editor) and goes through `Prompts` for its message boxes; its
> `VideoPane::View` interface asks only two things of the window: the targeted text, which the cue
> shows, and to enable or disable `Play / Pause`. The status bar stays with the window, which paints it
> after `watch` — the order of #323 —, pending #485. The fixes #469, #470 and #471 followed the
> code without changing: a tab's remembered place (`leave`), releasing the player before the surface
> (`release`, and the pane destroyed before the window's widgets), the picture's place in the
> splitter (`showPicture`). `main_window.cpp` goes from 2,216 to 1,923 lines, `main_window.hpp` from
> 872 to 815, without a window test being rewritten.

> **Completed on 2026-09-25, [#485](https://github.com/Guyot-Bertrand/sub-edit/issues/485): the status
> bar leaves, without an interface.** `StatusLine` owns the four permanent labels — targeted text,
> encoding, grid, film — and places them on the bar; each refresh receives the displayed `core::Project`,
> or the targeted text (`showTarget`). It asks nothing of the window and keeps nothing of a project,
> like `TableColumns`; nor does it decide **when** to refresh, which stays with `refreshActions`,
> `refreshVideo` and `refreshTarget`. The words come from `core/wording/`, where the command line
> reads them too, and its tests check it against them. Transient messages (`showMessage`) stay with
> the window. `rateReadInFrames` becomes a free function, since `Convert Frame Rate…` reads it too.
> `main_window.cpp` goes from 1,923 to 1,851 lines, `main_window.hpp` from 815 to 794, without a
> window test being rewritten.

## Alternatives dismissed

- **A single interface for all three**, a "window as seen by a collaborator". Shorter to write,
  it would have given each one all the gestures of the other two: search could have brought a
  document to the front to save it. The boundary is read in what an interface does not
  allow.
- **Qt signals rather than interfaces.** They say what happened, not what must
  happen: `mayDiscard` must know that a tab is at the front **before** opening `Save As…`, and a
  signal does not answer.
- **Cutting by menu** — `File`, `Edit`, `Tools`. Menus organise for the user, not for the code:
  `Find and Replace…` is under `Edit` and has nothing in common with `Cut`, `Columns` is under `View` and holds
  the same table as `targetDocument`.
- **Taking the `Tools` operations out as well.** They are already thin — a dialog, a core function,
  `applyOperation` —, and have no state: moving them would make nothing more testable.

  > **Revisited on 2026-09-25, [ADR 0035](0035-les-operations-sortent-de-la-fenetre.md) (#486):** since #461,
  > an operation reads its target in the page it is given, and can therefore be tested without a window. They leave
  > into `ProjectOperations`, along with `applyOperation`.

## Consequences

- **The collaborators have their tests, without a window**; the window's tests do not change, and that is
  what proves the cut moved nothing of the behaviour.
- **Targeting a page without making it the current tab becomes possible.** This ADR does not do it: so as
  to change nothing, gestures over several projects still ask the window to bring each page to the
  front. [#461](https://github.com/Guyot-Bertrand/sub-edit/issues/461) will remove those passages, except before
  a dialog that must show its project.

  > **Done in #461.** `View::apply` receives the page, `View::show` now serves only before a `Save As…`,
  > the question about a single document and the display of a match found in another project.
- One more interface per collaborator, to be kept up to date; that is the price of a boundary that can be
  read.
