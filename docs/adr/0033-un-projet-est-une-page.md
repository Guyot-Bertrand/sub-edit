# 0033 — Make a project a page of the window

**Date:** 2026-09-22
**Status:** accepted

Proposed while scoping phase 11, issue #426. **Accepted by #435**, the gate that preceded the third
batch of the phase: multi-project support ships, at the price described below and knowing it.

## Context

The roadmap promises **tabs**, and says of the phase that carries them that "the need is not
confirmed". This ADR describes what they would cost, so that #435 decides knowing it; **it does not
decide that they are built.**

`MainWindow` is 1,951 lines long and keeps, in a single object, **two kinds of state**:

| To the window | To a project |
| :------------ | :----------- |
| the menus and their actions, the status bar, the title | the session and its history |
| the player, the video view, the timer | the model and the table |
| the theme, the directories, the adjustment settings | the row where playback was placed |
| the clipboard, the search box | the associated video, the cue the picture carries |
| | the search match |

Today the window has **one** project, and the distinction never had to be made. Gaupol has
tabs in a window, each one a `Page`, and that is where its project state lives.

## Decision

**A project is a page.** `ProjectPage` carries the right-hand column — the session, the model, the table,
the playback and search state —, and **the window owns one or more of them**. When the current page
changes, the window **recomputes its menus, its status bar, its title and its film**.

**The player is not duplicated**: a single one, whose film changes with the page. What the window can
already do — `syncVideo` compares the wanted association with the one that is open, and reopens if they
differ — therefore serves to switch tabs. **The playback position does not survive a tab switch**, and the
manual says so.

> **Amended on 2026-09-25, [#471](https://github.com/Guyot-Bertrand/sub-edit/issues/471).** In use, on a
> real desktop, the user judged this price too heavy. The player stays single; **the position, however,
> survives**: each page keeps it when its tab is left (`ProjectPage::resumeAt`), and the player goes back
> to it on return, paused. It belongs to the film the tab was left on: another film chosen in the meantime
> starts again from the beginning.

**The clipboard stays with the window**: copying in one project and pasting in another is what one opens
two for.

**The extraction comes first, alone**, in the form of a window that owns a single page, without any
new behaviour. It proves the separation before a second project puts it to the test.

## Alternatives dismissed

- **One window per project.** Cheaper today: `MainWindow` as it is, and an application
  registry. Dismissed because "close all", "save all" and "next project" then become gestures
  **between** windows, with a menu per window that must know what the others do; and
  because the roadmap says "tabs", Gaupol has them, and one window per project is a different
  interface. *If #435 confirms the need for several projects but not for tabs, this is the fallback.*
- **A `QTabWidget` whose window state is swapped at each tab switch.** Without a page object,
  the window remains the place of everything, and **each tab switch is a copy of state in both directions** —
  precisely where one project's leaks into another live: a search match, a placed row, a film that was
  not changed.
- **One player per page.** One mpv process per tab: memory for films nobody is watching, and
  two possible sounds at once. The price of a single player, the position lost on tab switch, is
  lower. *(This price was lifted by #471: see the box above.)*

## Consequences

> **Completed on 2026-09-25, [#477](https://github.com/Guyot-Bertrand/sub-edit/issues/477).** Always keeping a
> project open has an effect the decision had not foreseen: started without a file, the window shows a
> blank project, and an opened file was added to it in a second tab, next to a useless project. Gaupol does
> not meet the case, since it starts with no project at all. **A blank project now gives up its place** to
> the file opened over it, in the same tab (`isBlank` in `project_page.hpp`). Nothing is asked,
> since a blank project has nothing to lose.

**Easy**: one more project is one more page; undo is already per session, hence per tab.

**Hard**: it is **the biggest rewrite of the phase** — `main_window.cpp` is crossed by it from end to end —,
and the only piece that is not easily undone. The criterion that makes it safe is written in the
extraction issue: **the window tests pass without rewriting any of them.** If one of them has to change,
the separation was not the one we thought, and the ADR reopens.

**#435 decided for multi-project support, on 2026-09-22.** Phase 11 had built the data model to
accommodate it from the first batch without ever committing to the rewrite; the decision commits to it
now, knowing the price written above. The five issues of the third batch — #436 to
#440 — leave `blocked`, and the extraction of [#436](https://github.com/Guyot-Bertrand/sub-edit/issues/436)
is the first to open: it sets up `ProjectPage` alone, without tabs, before a second project
puts the separation to the test.
