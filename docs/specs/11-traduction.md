# Phase 11 — Translation and multiple projects

Framing of issue [#426](https://github.com/Guyot-Bertrand/sub-edit/issues/426),
after the initialisation [#425](https://github.com/Guyot-Bertrand/sub-edit/issues/425).

## What the roadmap promises, and what is already there

A second document alongside the main one, alignment of the translation file by
number or by position, tabs, grouped saving and closing, splitting a project,
appending one file after another — plus two deferrals: the player for a
translation document, from phase 6, and search beyond the current document,
from phase 10.

**And this reservation, which the roadmap writes of this phase and of no
other: "the need is not confirmed".** A framing cannot confirm it in place of
whoever wrote it; it can build the phase so that the question is asked at the
right time. That is decision D1.

**The model was written for this very phase, and the core already accommodates
it.** What is missing is what *comes in* — a second file — and what the window
does with it.

| What is already there | Where |
| :-------------------- | :---- |
| a subtitle carries **both texts and a single pair of positions** — "a translation has no timing of its own" | `Subtitle`, phase 1 |
| `Document` names both, and **the commands take it**: the text, merging and splitting, degradation on conversion | phases 2, 4, 9, 10 |
| **the history keeps its changes per document**: a change of positions dirties both, a change of text its own | `affects(ChangeKind, Document)` |
| `Session::hasUnsavedChanges(Document)` and `markSaved(Document)` | `Session` |
| **the writer writes one text or the other** | `WriteRequest::document` |
| a read that returns the diagnostics, including `OutOfOrder`, and a core that never sorts on its own | ADR 0008, 0012 |
| eight main/translation pairs, **with the expected alignment of each method**, observed on Gaupol itself | [#427](https://github.com/Guyot-Bertrand/sub-edit/issues/427), `src/test/data/paires/` |

**What is not there.** `Project` keeps only **one** `SourceFile` — a path, a
format, an encoding —, and `sourceFile()` is read in twenty-nine places, twelve of
them in core operations that look up the markup dialect there. The window writes
`Document::Main` hard-coded in fourteen places. And `MainWindow` keeps, in a single
1,951-line object, what belongs to the window and what belongs to a project.

## Preliminary analysis — what Gaupol does

Read in `aeidon/agents/open.py`, `gaupol/agents/open.py` and `close.py`,
`gaupol/dialogs/split.py` and `multi_close.py`, `gaupol/actions/`, and **executed**
where reading was not enough: the eight cases of #427 were run against the
`aeidon` package, copied out of the clone. Five findings govern what follows.

**The translation is a second file laid on the same list of subtitles.**
It has no timing of its own: its lines are *attached* to the subtitles of the
main document, by number or by position, and those that find no place give birth
to new ones. `open_translation` **first clears all existing translations**, then
aligns.

**A text operation targets the document of the column that has the focus.**
`text_column_to_document(col)`: case, dashes, italics apply to the current text
column, and the action is disabled if the focus is not in a text column. Italics
even reads the tags **of that document's format**.

**Opening announces nothing, except sorting.** The alignment does not say how
many lines found their place nor how many gave birth to a subtitle; on the other
hand, if the file had to be sorted, a dialog asks whether to open it — the
behaviour that [ADR 0012](../adr/0012-ordre-des-sous-titres-par-composition.md)
has already read and refused to copy.

**Closing several projects asks a single question.** A dialog lists the modified
documents — the main document and the translation of the same project are two
rows —, a checkbox each, and answers save, close without saving, or cancel;
cancel closes nothing. With a single document to confirm, it is a simple dialog.
**A file gone from the disk counts as modified**: it is the only copy of what the
project contains.

**Appending and splitting are two inverse shifts.** Appending shifts by the end of
the last subtitle; splitting shifts the new project by *minus* the end of the last
subtitle left in the original. Both are a single history entry.

> **A sentence of the framing was wrong, and reading corrected it.** #426
> wrote that Gaupol sorts the translation "without the window saying so other
> than by a returned number". It asks for confirmation — which ADR 0012 already
> said, and which the framing did not reread before writing.

## D1 — The phase is delivered in three slices, and a gate precedes the last

The roadmap writes that the need is not confirmed. The six pieces — translation,
tabs, grouped saving, grouped closing, splitting, appending — **do not carry the
same need**, and delivering them together because Gaupol delivers them together is
a choice that was not made.

| Slice | What it delivers | What it touches |
| :---- | :--------------- | :-------------- |
| **1 — translation** | a second document alongside the main one, aligned, edited, saved; the closing question with a rich answer | the model, the table, the text operations |
| **2 — appending a file** | appending one file after another | a menu, a command |
| **3 — multiple projects** | tabs, grouped saving and closing, splitting, searching everywhere | **the whole window** |

**The first two stand without the third**, and do not depend on it. The third
makes `MainWindow` one page among others: it is the only piece of the phase that
is not easily undone, and it is the one the roadmap doubts. **A gate,
[#435](https://github.com/Guyot-Bertrand/sub-edit/issues/435), therefore precedes
the last slice**: its five issues are `blocked` until the decision is made, and
written, when the window knows what a translation and an append bring it.

**Neither answer is a failure.** If multiple projects are delivered, [ADR
0033](../adr/0033-un-projet-est-une-page.md) moves from "proposed" to "accepted".
If they are not, its five issues are closed with their reason, the ADR is
abandoned, and the phase closes on its review with the translation.

**#435 decided for multiple projects, on 2026-09-22.** ADR 0033 is accepted, and
the five issues of the third slice leave `blocked` — #436, the extraction of
`ProjectPage`, first.

## D2 — A project is a page, and the page is a tab

**To be decided by [#435](https://github.com/Guyot-Bertrand/sub-edit/issues/435), and
described here so that the decision is made knowingly.** [ADR 0033](../adr/0033-un-projet-est-une-page.md),
proposed.

> **Corrected at the end-of-phase review.** ADR 0033 has been **accepted** since #435, on 2026-09-22 —
> D1 says so, this line did not. The paragraph below has stayed in the conditional of the framing;
> what the separation became is written further down, by #436 and #437.

`MainWindow` keeps, in one and the same object, **two kinds of state**:

| Belongs to the window | Belongs to a project |
| :-------------------- | :------------------- |
| the menus and their actions, the status bar, the title | the session and its history (`m_session`) |
| **the player**, the video view, the timer, **the table (`m_table`) — #436** | the model (`m_model`) |
| the theme, the directories, the adjustment settings | the row where playback was placed (`m_placedAt`) |
| **the clipboard**, the search dialog | the associated video (`m_associated`, `m_watching`) |
| | the line the picture carries (`m_shown`) |
| | the search match (`m_match`, `m_searchTarget`) |

**Tabs are this separation made, and the window owning several of them.** One
object, `ProjectPage`, carries the right-hand column; the window keeps one or
more, and **recomputes its menus, its status bar, its title and its film when the
current page changes**. The player is not duplicated: a single one, whose film is
changed — `syncVideo` already knows how to do that when the association changes —
and whose **playback position does not survive a tab change**. The manual says
so; it is the price of a single mpv process.

> **Revised by [#471](https://github.com/Guyot-Bertrand/sub-edit/issues/471), at the user's request.**
> The price turned out too heavy in use: each return to a tab sent its film back to the beginning.
> The player stays single, but **each page keeps the position of its film** when its tab is left
> (`ProjectPage::resumeAt`) and the player goes back to it on return, paused. Another film chosen in the
> meantime starts from the beginning.

**The clipboard stays with the window**: copying in one project and pasting in the
other is what one opens two for.

**The extraction is done first, alone** ([#436](https://github.com/Guyot-Bertrand/sub-edit/issues/436)):
a refactoring that has a single page, and whose window tests pass without
rewriting any of them. That is what proves the separation before a second project
puts it to the test.

**A correction #436 had to make: `m_table` stays with the window, not the page.**
The table above said so together with `m_model`, as a single thing; `ProjectPage`
carries only the latter. The Qt view itself — building the widget, its parent, its
place in `QSplitter` — is a tab-sharing question still to be settled (a stack of
tables, or one table that changes model), and settling it now would have been
deciding a question of #437 in an issue whose only criterion is to change nothing.
`MainWindow::table()` returns the same widget as before: no test had to know.

**What the decision rules out** is in the ADR: one window per project, cheaper
today, but which makes "close all" an application matter and the player an object
per window; and a `QTabWidget` whose state would be swapped at each tab change,
where the bugs of state leaking from one project to another live.

**#437 settled the question #436 had left open: a single table, whose model
changes.** Not a stack of tables per page — the option #436 named without
choosing. `QTabBar` alone, not `QTabWidget`: nothing to stack, one label per page
and a signal when another is chosen.

**Two things Qt does on its own, and which had to be taken back.**
`QAbstractItemView::setModel` throws away the selection model it had and makes a
new one at each call — including to return to a model already shown. Without care,
each page would therefore have forgotten its selection at the first tab change.
`ProjectPage` now carries its own (`tableSelection`), made once, handed back to the
table at each page — ADR 0033 gains this line, which the table above did not count.

**The shared player has the same trap, one line higher in the code.**
`watchAssociatedVideo` does nothing when a page's association has not changed since
the last time it was synchronised — the right rule for a single project, wrong for
several: returning to a page whose association has not moved leaves the player on
the film of the page just left. A second thing to compare, `m_playingPage`, which
says which page drives the player at the moment — not only which one the window
shows.

**Two entries the issue promised, and that this PR does not deliver**: dragging a
file onto the window, and a `Projects` menu listing the tabs. Neither is in the
completion criteria of #437 — only Gaupol's prose named them.

**Drag and drop got its own issue**, [#453](https://github.com/Guyot-Bertrand/sub-edit/issues/453),
and its five questions were settled before writing. **The whole window** accepts the drop. **A
subtitle file follows the route of `Open…`** — one tab each, a file already open gives the focus back to its
tab without being reread: `openFile` is now what the two gestures share. **A video goes to the current
tab**, like `Select Video…`, once the subtitles of the same drop are open. Several files: Gaupol's sorting
(`isVideoFile`, the closed list of extensions), with two deviations — **two or more videos are refused
with a message** rather than silently, and **an unreadable file does not stop the others**; what could
not be done is said in a single dialog, one line each.
**The `Projects` menu has none**: it duplicates what the tab bar already does, and
`Ctrl+PageUp`/`Ctrl+PageDown` cover the keyboard — to be revisited if use shows the
need, rather than a deferral for a redundant gesture.

> **Clarified at the end-of-phase review.** "The `Projects` menu" here means **the list of tabs**
> that Gaupol puts there, with `Previous` and `Next`. A `Projects` menu has existed since #438, for the two
> gestures that apply to all projects at once — `Save All` and `Close All` — and **without** that list:
> it is the list, and the list alone, that has no deferral.

## D3 — One document, one file

[ADR 0032](../adr/0032-un-document-un-fichier.md), accepted.

**`Project` keeps a second `SourceFile`, that of the translation, optional.**
Path, format, line endings, encoding, header, what the file declares about itself
(ADR 0030): everything a document keeps from its file to rewrite itself identically,
and the translation is no less a file than the main document. A translation can be
in SSA when the main document is in SubRip, and in Windows-1252 when the latter is
in UTF-8.

**Without a translation file, it follows the main document's format**: a translation
typed into an empty column will be written like the main one, until a format is
chosen for it.

**Eleven of the twelve core sites that read `sourceFile()` to choose a dialect
read that of the target document**: `italics_command` (2), `dialogue_dashes_command`
(2), `clipboard` (2), `search` (3), `letter_case_command` (1) and
`hearing_impaired_removal` (1). Today they all read the main document — the last two
**already receive a `Document`** and yet read the main document's format —, which is
wrong only on the day a translation exists. **The twelfth, `duration_adjustment`,
stays with the main document, on purpose**: reading speed is computed on `mainText`,
and the adjustment is a positions operation.

**They do not all change at the same time.** #429 changes eight — all those that
already receive a `Document`. The three in `search` **were waiting**: search did not
take a `Document`, it read `mainText`, and it is #433 that gave it one. They named the
main document (`Document::Main`), and a test held it; that test now states both
directions, one search per document.

**What the decision rules out** is in the ADR: a translation file the project would
ignore — the window would keep it next to its `Session`, which ADR 0018 has already
refused; a translation read then forgotten as a source, which could no longer be saved
to its path or in its format; and a second `Project` linked to the first, which would
duplicate positions a translation does not have.

**What the decision costs, and the ADR says so**: the per-subtitle settings of a
translation — an ASS style, the position of a WebVTT subtitle — **are not kept**.
A subtitle carries only one set of them, the main document's.

First implementation issue: [#429](https://github.com/Guyot-Bertrand/sub-edit/issues/429).

## D4 — Opening a translation

Two methods, by position and by number, like Gaupol; **position is the default**,
and the eight cases of `src/test/data/paires/` say why: on
`une-ligne-de-moins-au-milieu`, a single missing line shifts **all** the following
ones by number, whereas by position only one is left without a translation.

**Six rules, four of which depart from Gaupol, and each of these departures has its
reason.**

- **Both methods go through the translation in time order, and sort neither
  document.** Gaupol sorts, counts, and asks for confirmation; [ADR
  0012](../adr/0012-ordre-des-sous-titres-par-composition.md) refused that path for
  the main document, and the core never sorts on its own. **The count of lines that
  were out of order is stated**, in the report of the opening — the read already
  returns `OutOfOrder`. The case `traduction-dans-le-desordre` gives, for both
  methods, what the control case gives.
- **A subtitle born from a translation line keeps that line's positions, whatever
  the method.** By number, Gaupol invents three seconds after the last subtitle —
  `_insert_blank_subtitles` — when the translation file **states** its own.
  Inventing what is written is a loss, and the case `une-ligne-de-plus-a-la-fin`
  shows it: 15–18 s invented on one side, 16–18 s stated on the other.
- **Opening clears the existing translations**, like Gaupol, **and enters the
  history as a single entry** — which Gaupol does not do (`register=None`). Undo
  gives everything back: the texts, the born subtitles removed, the translation file
  detached. Gaupol leaves the subtitles born from a first alignment when a second is
  opened; here, one undoes the first before opening with the other method.
- **Opening says what happened.** ADR 0008: say rather than stay silent. The count is
  that of `TranslationOutcome` — lines attached to their subtitle, lines that gave
  birth to a subtitle, subtitles left without a translation, out-of-order lines — and
  the sentence that says it lives in `core/wording/translation.hpp`, the place of the
  words the command line will take up. **In the status bar when everything was
  attached and nothing was left alone, in a dialog to close otherwise**: the rule of
  #398, for a gesture that has something to say.
- **A translation modified and not saved is offered for saving** before being
  replaced: the question Gaupol asks (`_show_translation_warning_dialog`).
- **The file already open as the main document is refused**, and the window says so.

**What the read owes to what exists**: a translation in frames (MicroDVD) is read at
the project's frame rate, a translation without ends (LRC, TMPlayer) with the ends the
read invents — ADR 0029 —, and **it is on those that the middle of a line is
computed**.

**`positions-decalees` is the case the spec does not correct, and it names it.**
Gaupol has no offset detection: a translation timed two seconds too late is not
recognised for what it is, and its first line sticks **onto the wrong subtitle**
with nothing to distinguish it from a correct attachment. The count of the opening
is what lets the user see it: three lines out of four give birth to a subtitle there.

**What the window made of it** (#432): two questions one after the other rather than
a single dialog — the file chooser of `Open…`, then a small dialog of our own for the
method, which goes through `Prompts::run` and is tested without a modal loop. **The
encoding is not asked for**, as it is not when opening the main document: it is
detected, and the window has never offered to choose it on reading. The read is done
**before** the method question: a file that does not open, or that is the main
document, is not worth asking how to align it. What the read encountered goes to the
diagnostics panel, that of the last read. A translation just opened **is not
modified** — it is the main document that is, when subtitles were born.

Issue: [#430](https://github.com/Guyot-Bertrand/sub-edit/issues/430) for the core,
[#432](https://github.com/Guyot-Bertrand/sub-edit/issues/432) for the window.

## D5 — The translation column, and the document the operations target

**A sixth column, `Translation`, to the right of `Text`.** Absent from a project
without a translation — which is what Gaupol does, removing it when a main document
is opened and when a project is created —, it appears when the project receives one,
and an entry toggles it. **That entry belongs to a `View` menu the window does not
have** — it has `File`, `Edit`, `Video`, `Tools` and `Help` — **and which is born
with it**, as in Gaupol; it is also where the rest of the columns would one day go
([#442](https://github.com/Guyot-Bertrand/sub-edit/issues/442)). Its cells are edited
like those of the text.

> **Written at the end-of-phase review.** That "one day" came within the phase itself: #442 was handled
> after the third slice, and the translation entry moved into a submenu,
> `View ▸ Columns`, with those of the four other columns that can be hidden — see the end of this section.

**The target document is that of the current column**: the translation if the current
cell is in its column, the main document elsewhere. That is Gaupol's rule, with one
difference: **nothing is disabled.** Gaupol disables the text operations outside a
text column; here, without a translation column, the target document is always the
main one, and the window behaves as before. One more disabled state would make the
phase a regression for whoever never opens a translation.

| What the operation does | What it targets |
| :---------------------- | :-------------- |
| **positions** — shifting, transforming, converting the frame rate, aligning, adjusting durations | both: positions are shared, and the commands already move both |
| **structure** — inserting, removing, merging, splitting | both: a subtitle is a whole, and merging and splitting already join both texts back |
| **text** — removing mentions, italics, case, dashes, copying, cutting, pasting, editing a cell | the target document |
| **search** | the target document — D8 |

**The status bar states the target document when there are two**, and says nothing
otherwise.

**Removing mentions, targeted at the translation, never removes a subtitle.** On the
main document, a text the rule empties takes its subtitle with it: a subtitle without
text is not one. On the translation, the same gesture would have destroyed the main
text, which was not targeted — that is the question #429 brought up, and **Gaupol does
not distinguish the two**: its `remove_hearing_impaired` removes any subtitle whose
target text becomes empty, whatever the document. Here the translation is **emptied**
and the subtitle stays; the report counts it as cleaned
(`1 subtitle cleaned, 0 removed`), and undo gives its text back. Refusing the gesture would have protected
nothing more, and left the user with a translation full of mentions and no recourse.
It is deviation no. 8.

**The clipboard carries no document**: it is a list of texts. One can therefore copy
a cell of the main document and paste it into the translation column, which Gaupol
allows.

**What #442 added: the rest of the columns.** The deferral above had a recipient
without a phase; it was handled after slice 3, on three answers given before writing:

- **All can be hidden except `Text`**, under `View ▸ Columns` — a submenu, as in
  Gaupol; the translation entry moved there. Without the main text there is nothing
  left to edit.
- **All can be moved**, by dragging the header, and **the order as well as the removed
  columns are remembered** in the preferences (`table.order`, `table.hidden`,
  ADR 0022), with the widths. The translation is not among them: its visibility keeps
  the rule above.
- **A current cell in a column being removed moves to `Text`**, same row, selection
  kept — the rule `targetDocument` already applied to a hidden translation, extended
  to the cell.

Two details the implementation had to settle. A hidden column measures zero, and the
preferences reader refuses a zero width — the whole line would have been lost: the
window keeps the width it had when leaving. And the order survives a tab change
without anything being done: `setModel` keeps the sections when their number does not
change.

Issues: [#431](https://github.com/Guyot-Bertrand/sub-edit/issues/431),
[#442](https://github.com/Guyot-Bertrand/sub-edit/issues/442).

## D6 — Appending a file, and splitting a project

**Appending is independent of everything else**, and that is its slice.
`Tools ▸ Append File…`, in the menu where Gaupol puts it.

- The positions of the appended file are **shifted by the end of the project's last
  subtitle**, like Gaupol. Nothing is inserted between the two. **Appending is
  disabled on a project without subtitles**, as in Gaupol (`len(subtitles) > 0`):
  there is no end to shift by.
- **The appended file goes through conversion.** Its tags are translated into the
  project's dialect and the loss is stated — the rule and the words of pasting from
  another format, phase 10 —, and it is `convertProjectFor` that does the work. The
  appended subtitles have no translation.
- A single history entry, "Appending file"; the selection is that of the appended
  subtitles.
- **A file already open is allowed**: Gaupol allows it, and appending a file to
  itself is legitimate.

**One thing this framing promised and #434 withdrew: the file chooser does not offer
the encoding.** The same correction as in #432 applies here: the window offers one
nowhere on reading, `Open…` included, and adding one for this chooser alone would
have been one more inconsistency rather than a richness.

**What #434 had to settle, and this framing did not say.** Appending becomes one more
`CommandKind`, and two existing exhaustive switches depend on it: ordering
(`mayBreakOrder`) classes it with inserting and pasting — what it lays down may be out
of order —, and overrunning the film (`movesPositions`) classes it **with shifting and
not with inserting**: what it appends is shifted by everything the project already
carries, and that is exactly what the overrun warning exists to catch.

**Splitting is the exact inverse of appending**, `Tools ▸ Split Project…`, disabled
under two subtitles as in Gaupol. The rest is copied into a new project, opened in a
new tab, and removed from the original; the new project is **shifted by minus the end
of the last subtitle left**, so that splitting then appending gives back the original
project. Its two texts follow their subtitles, and it inherits the format and the
encoding, without a path.

**What reading Gaupol did not say, and the framing settles.** `Calculator.add` does
not clamp at zero, and `seconds_to_time` even writes a `-` sign: if the two halves of a
project overlap at the cut, Gaupol produces a negative position. `subedit` can
represent it and no file can write it — that is `firstBeforeOrigin`. **Splitting is
therefore refused, it names the subtitle that would fall before the origin, and offers
to cut elsewhere**: the rule of shifting, not one more rule.

**What #439 delivered.** `splitProject` returns a
`std::expected<SplitProject, SplitRefusal>`: the new project and a command — a
`RemoveCommand` under a new `CommandKind::SplitProject`, which ordering and the film
overrun class with removal, nothing moves in the original. The cut is bounded from the
second subtitle to the last (`std::out_of_range` otherwise: the dialog does not offer
it).

- **The new project inherits the translation when the original has one** — a
  translation file without a path, same format, same encoding —, without which its
  translation texts would have neither a column nor a modified state.
- **It is born modified.** No file holds it: closing it without asking would lose the
  subtitles. `Session::markUnsaved` is the counterpart of `markSaved`, and is called
  only there.
- **The refusal names the subtitle**, through `firstBeforeOrigin`, numbering as the
  table does.
- **The dialog opens on the current row.** Gaupol did not say so; it is "from here".

Issues: [#434](https://github.com/Guyot-Bertrand/sub-edit/issues/434) for appending,
[#439](https://github.com/Guyot-Bertrand/sub-edit/issues/439) for splitting.

## D7 — Saving, and closing several documents

**Each document is saved separately.** `Save Translation` and `Save Translation As…`
write the translation in its format and its encoding, with the dialog and the loss
warning of `Save As…`; the modified state is per document, as the history already has
it, and the window title shows it if either of the two is modified.

**Closing, or quitting, with several modified documents asks a single question.**
`Prompts` receives the list of modified documents, a checkbox each, and answers save
those checked, close without saving, or cancel; **with a single modified document, the
question stays today's.** It is Gaupol's dialog, and not one question per project,
simpler to write and more tedious to answer: three modified projects would make three
dialogs that can no longer be cancelled in one go.

**This richer answer is what the first slice must deliver**, before the tabs: as soon
as a project has a translation, it has two documents to close. That is why it is in
[#432](https://github.com/Guyot-Bertrand/sub-edit/issues/432) and not in grouped
closing. `FakePrompts` will take it on at that point: its `nextUnsavedChoice`, which a
single test file uses, is no longer enough.

**A file gone from the disk counts as modified**, and the list says so.
`FileSystem::exists` already knows how to answer.

**`Save All`, `Close All` and leaving the window belong to the third slice**: they are
the same questions asked over N tabs. `Save All` writes each modified document that has
a file; **a document without a file opens `Save As…`, one at a time**, and an abandon
stops the rest, saying what was written. **`Save All As…` is not delivered**: it is a
series of dialogs that `Save All` already does, one at a time, for those without a
name.

**What #432 delivered**: the richer answer goes through `Prompts::run`, like every
dialog of our own — an `UnsavedDocumentsDialog` from which `choice()` and `toSave()`
are read —, so that `Prompts` gained no method; `aboutUnsavedChanges`, on the other
hand, takes the document, to say which of the two is meant. **Two modified documents
open the list, a single one asks the usual question.** Writing the translation reads
**the translation's file** — path, format, encoding, line endings, header —, and the
announced loss is that **of the document written**: `convertFor` no longer goes through
both texts with the main document's format. Opening another file replaces the
translation, so it asks the same question.

**What #438 delivered.** A `Projects` menu, **with these two entries only**: `Save All`
and `Close All`. Gaupol's also lists the tabs and carries `Save All As…`; the former
duplicates the tab bar (D2), the latter is ruled out above.

- **`Close All` is closing the window.** It has no method of its own: the action is
  connected to `close()`, and `closeEvent` asks the question. The window always keeps
  a project — closing all projects and closing the window are a single act, and there
  is no intermediate state to write. The criterion "quitting goes through the same
  question" is true by construction.
- **The list is that of #432, over all tabs.** `mayDiscardAllChanges` gathers the
  modified documents of each page, a vector of page indices in parallel. **`Save` reads
  the checkboxes**, not `toSave()`: the latter names documents, and two projects each
  have a main document. Names are not qualified by project — nor are they in Gaupol's
  list. A single modified document, whatever the tab, asks the simple question, the
  tab brought to the front.
- **`Save All` reuses `saveDocument`**, which already opens `Save As…` for a document
  without a file. An abandon or a failure stops the loop; the dialog says how many were
  written out of how many.
- **A tab's label carries an asterisk** while one of its documents is modified, updated
  in the same place as the window's asterisk (`refreshActions`).

Issues: [#432](https://github.com/Guyot-Bertrand/sub-edit/issues/432),
[#438](https://github.com/Guyot-Bertrand/sub-edit/issues/438).

## D8 — The player and search

**Both deferrals land here, and they take the rule of D5** rather than each inventing
its own.

- **The line drawn on the picture is that of the target document**, or of the main
  document when there is no translation. **No setting**: a single rule to say which
  text, across the whole window. It is the deferral of phase 6 — "it will be necessary
  to say which of the two is displayed, and whether the choice is a setting or follows
  the active tab": it follows the active column, and it is not a setting.
- **Search works on the target document.** **The `Document` is a parameter** of
  `findNext`, `findPrevious`, `replaceMatch` and `replaceAll`, next to the target,
  **and not a field of `SearchOptions`** — the first draft of this decision said the
  opposite, and #433 corrected it: `SearchOptions` is what the preferences file keeps
  from one session to the next (`search.regex`, `search.ignore-case`), and the target
  text is not a preference, it follows the current column. The dialog says in which
  field it searches (`Searching in: Translation`, absent when there is only one
  text); **replacing writes into that document's source text**, the tags of its
  format respected. That is half of the deferral of phase 10. Gaupol searches the main
  document, the translation or both; **"both" is not delivered**, and writing down why
  is the point: searching two texts at once would make each match a pair (subtitle,
  document), and the table that moves on its own would lose its meaning there.
- **The "all open projects" scope** belongs to the third slice: it only makes sense
  with several projects. `Replace All` there is **one history entry per project
  touched**, so that undoing in one tab only undoes what that tab received.

**What #440 delivered.** An `All open projects` checkbox in the search dialog — not a
three-state selector: the two scopes from before, the selection or the document, are
not chosen, they are read from the table, and the new one is the only one that asks
for a gesture. **Disabled with one project, unchecked automatically when only one is
left**; checked, it ignores the selection and takes each project whole.

- **The algorithm belongs to the window, not the core**: the core searches within one
  project and wraps around on itself, the window knows what a tab is. The project shown
  first; if its next match is in fact the wrap-around of the same project, the other
  tabs are visited in order (in reverse when going up), each from its beginning; the
  wrap is reported as `Search wrapped around` when it passes the end again.
- **A match a tab keeps from its last visit is of no use**: the other projects are
  searched from their beginning, never from their old match, and a change of pattern or
  option forgets them all.
- **`Replace All` goes through each page**: `switchToPage` then `applyOperation`, like
  `Save All`, which makes one history entry per project touched without inventing
  anything.

  > **Corrected by #461.** `applyOperation` now receives the page it targets: `Replace
  > All` gives each page its command behind its tab, without `switchToPage` — one
  > history entry per project touched, still, but neither flicker nor reloaded film.
  > The count, `replaced N matches in M projects`, is
  > `noticeOfReplaceAll(count, projects)`.
- **On the target document of each project**: the column is that of the table, a
  single one for the whole window.

**Two things the decision did not say, and that #433 had to settle.**

- **The table stays in the column it was in.** `selectRows` put the current cell in
  column 0 — hence in the main document — each time it selected rows: a search started
  in the translation continued in the main document from the first match. The current
  column is kept, and that is what makes the rule "the target text is that of the
  current column" true for an operation that moves the selection.
- **A match is a place in a text.** When the current column moves from one text to the
  other, the remembered match is forgotten: a subtitle's translation can read exactly
  what the main document says, and a `Replace` would rewrite it without the user having
  seen it in that text.

**The line draws the text as the model carries it, tags included** — ADR 0009, which
the issue did not change. Removing the tags "according to the document's format" is
not delivered: it would change what someone who never opens a translation sees, and
the issue's criterion is that nothing changes without a translation.

Issues: [#433](https://github.com/Guyot-Bertrand/sub-edit/issues/433),
[#440](https://github.com/Guyot-Bertrand/sub-edit/issues/440).

## D9 — The command line waits for phase 13

`-t/--translation-file` exists in Gaupol, and the phase 4 spec deferred to here
"cleaning the translation document from the command line". **Neither enters this
phase**, and the deferral changes recipient: phase 13, that of the complete command
line.

What makes the deferral honest: **nothing this phase writes is specific to the
window.** Opening a translation, its alignment and the count it returns live in the
core; the sentence that states it lives in `core/wording/translation.hpp`, the place of
shared words. Phase 13 will only have a grammar to write. And `Document::Translation`
still appears nowhere in `src/lib/subedit/cli` — a `--document` option would be dead
grammar, until there is something to target.

## What the phase does not deliver

Each with a phase or an issue, because a deferral without a recipient ends up pointing
to a phase already past.

- **The command line for the translation** — phase 13, D9.
- ~~**Hiding and reordering the table's columns.**~~ **Delivered all the same, by
  [#442](https://github.com/Guyot-Bertrand/sub-edit/issues/442)**, left without a
  milestone on purpose and handled after the third slice, once its three questions were
  settled — see D5.
- **`Save All As…`** — ruled out, and not deferred: D7.
- **Searching the main document and the translation at once** — ruled out, and not
  deferred: D8.
- **The per-subtitle settings of a translation** — an ASS style, a WebVTT position —
  **ruled out**: a subtitle carries only one set of them, the main document's.
  [ADR 0032](../adr/0032-un-document-un-fichier.md).
- **Detecting that a translation is offset.** Gaupol does not do it, and the case
  `positions-decalees` shows what that costs; the count of the opening makes it
  visible, not correctable. It would be synchronisation correction: phase 14, fine
  timing.

## Deviations from Gaupol

**Eight, each with its reason**, and the end-of-phase review checks that what was
delivered holds them and that no other slipped in.

> **Completed at the end-of-phase review.** The eight are held. **Nine others had
> slipped in**, all in the third slice and in the two issues that followed it, each
> stated in its section but none in this table; they are added below the rule. Only one
> was wanted by nobody — the split that says nothing — and it becomes an issue rather
> than a deviation.

| What Gaupol does | What `subedit` does | Why |
| :--------------- | :------------------ | :-- |
| by number, a surplus line gives birth to a subtitle at **invented positions** (3 s) | **the line's positions**, whatever the method | the file states them — D4 |
| opening a translation **does not enter** the history | **a single entry**, undoable | ADR 0010, and reopening with the other method — D4 |
| **sorts** the translation and asks for confirmation | **sorts none**, goes through in time order, and states the count | ADR 0012 — D4 |
| the alignment **announces nothing** | **the count**: status bar or dialog | ADR 0008 — D4 |
| the text operations are **disabled** outside a text column | **nothing is disabled**, the target document is the main one without a translation column | not regressing for whoever does not open a translation — D5 |
| removing mentions removes the **whole subtitle** when the target text becomes empty, translation included | **empties the translation** and keeps the subtitle | the main text was not targeted — D5 |
| `Save All As…`, searching "both" texts | **not delivered** | D7, D8 |
| `-t/--translation-file` | **phase 13** | D9 |
| *added at the end-of-phase review* | | |
| `Close All` closes the projects and **leaves the window empty** | `Close All` **closes the window** | the window always keeps a project: closing all projects and closing it are a single act — D7, #438 |
| the `Projects` menu **lists the tabs** and carries `Previous`, `Next` | **without the list** or those two entries | the tab bar and `Ctrl+PageUp`/`Ctrl+PageDown` already do it — D2 |
| splitting **writes** a negative position when the halves overlap | **refused**, the subtitle named | no file can write it: the rule of `firstBeforeOrigin` — D6, #439 |
| the project born from a split **inherits nothing**: default format and encoding | it **inherits the format, the encoding and the frame rate**, without a path | the exact inverse of appending, which converts to the project's format — D6, #439 |
| the project born from a split has **one history entry**, the insertion of its subtitles | **an empty history**, and the project **marked modified** | undoing that insertion would empty the project just created; what matters is that it asks before being lost — #439 |
| all columns can be hidden, `Text` included | **`Text` cannot be hidden**; the translation's visibility is not remembered | without the main text there is nothing left to edit; the translation follows its own rule — D5, #442 |
| dropping several videos: **all ignored, without a word** | **ignored with a message** | say rather than stay silent, ADR 0008 — D2, #453 |
| dropping an unreadable file **stops** the following ones | **the others open**, a single dialog states the failures | a drop is one gesture, and ten files do not make ten dialogs — D2, #453 |

> **Corrected by [#462](https://github.com/Guyot-Bertrand/sub-edit/issues/462).** The row "splitting
> says so" has left the table: it is no longer a deviation. The status bar says `split N subtitles into a new
> project` — without the project's name, which it does not have yet — and the dialog selects the row it
> designates at each change of the number, like Gaupol; cancelling it gives back the previous selection, which
> Gaupol does not do.

> **Added by [#474](https://github.com/Guyot-Bertrand/sub-edit/issues/474): the toolbar.** It
> carried only `Open…`, `Save`, `Undo`, `Redo` and `Italic`, and the user read "a single operation, and not
> the most frequent one" as an oversight. Compared with Gaupol's bar (`gaupol/application.py`,
> `_init_header_bar`), it now carries:
>
> | Gaupol | `subedit` | Why |
> | :----- | :-------- | :-- |
> | `Open` and a menu of recent files | `Open` | `subedit` has no list of recent files: nothing to put there |
> | `Save` | `Save` | — |
> | `Undo`, `Redo` | `Undo`, `Redo` | — |
> | `Find and Replace` | `Find` | — |
> | `Preview`: an **external** player from the chosen row | `Play`: the **built-in** player | the film plays in the window (ADR 0020); Gaupol also gives its own a separate bar, shown with the film, and `subedit` carries only the button that serves most |
> | — | `New` | opening a project is a gesture people look for, #473 |
> | — | `Insert`, `Remove` | the most frequent editing gestures, requested by the user; opening a dialog is no reason to stay off the bar |
> | — | `Italic` | already on the bar, the only operation without a setting; removing it would have been a regression |
>
> The buttons read a short word (`iconText`), the menus keep the whole entry. A test freezes the content and
> the order of the bar.

## Requirements

**Fourteen, all `planned`** — the registry is fed at the start of each issue. The five of
the third slice stay `planned` until the decision of #435; if it is negative, they are
`abandoned` with their reason.

> **Corrected at the end-of-phase review.** **Eighteen, all `implemented`.** Four were born
> along the way: `GUI-TABS-03` and `GUI-SAVE-04` with #438, `GUI-TABLE-03` with #442, `GUI-TABS-04`
> with #453. `GUI-TABS-02` was narrowed by #438 to closing, `Save All` having received its own.

| ID | Requirement |
| :- | :---------- |
| `GUI-TRANS-01` | opening a translation file aligns it with the main document, by position or by number |
| `GUI-TRANS-02` | opening says what happened: lines attached, subtitles born, subtitles without a translation, out-of-order lines |
| `GUI-TRANS-03` | opening a translation is a single history entry, and undo gives the document back as it was |
| `GUI-TRANS-04` | the translation column appears with the translation, and can be edited |
| `GUI-TRANS-05` | a text operation targets the document of the current column, and the status bar says so |
| `GUI-TRANS-06` | the translation is saved separately from the main document, each with its modified state |
| `GUI-CLOSE-01` | closing with several modified documents asks a single question, one checkbox per document |
| `GUI-SEARCH-03` | search works on the target document, and replacing writes into its source text |
| `GUI-PLAYER-04` | the line drawn on the picture is that of the target document |
| `GUI-APPEND-01` | appending a file shifts by the end of the last subtitle, states what the conversion loses, and is undone in one go |
| `GUI-TABLE-03` | the columns, except the text, can be hidden; all can be moved; the table remembers them |
| `GUI-TABS-01` | several projects open in tabs, each with its history, its selection and its video |
| `GUI-TABS-02` | closing all, or quitting, with a single question for all projects |
| `GUI-TABS-03` | a tab's label says that its project is modified |
| `GUI-TABS-04` | dragging files onto the window opens them: one tab per subtitle file, a video to the current tab |
| `GUI-SAVE-04` | saving all writes each modified document, tab by tab, and stops at an abandon, saying what was written |
| `GUI-PSPLIT-01` | splitting a project in two, and appending the rest gives back the original project |
| `GUI-SEARCH-04` | search works, optionally, on all open projects |

**The alignment core has no requirement**, and that is the registry's rule: a requirement
is what the binary shows. `GUI-TRANS-01` and `GUI-TRANS-02` are what the window shows of
it; the eight cases of `paires/` hold the core.

## Split into issues

The order goes from the foundational to the costly. **Translation comes before
everything**, because it is the only piece the phase cannot do without, and the other two
slices add to it.

| Issue | What it does | What it depends on |
| :---- | :----------- | :----------------- |
| [#429](https://github.com/Guyot-Bertrand/sub-edit/issues/429) | a second file per project, and a format per document | — |
| [#430](https://github.com/Guyot-Bertrand/sub-edit/issues/430) | opening a translation: aligning it with the main document | #429, #427 |
| [#431](https://github.com/Guyot-Bertrand/sub-edit/issues/431) | the translation column, and the document the operations target | #429 |
| [#432](https://github.com/Guyot-Bertrand/sub-edit/issues/432) | opening and saving a translation, and closing with two modified documents | #430, #431 |
| [#433](https://github.com/Guyot-Bertrand/sub-edit/issues/433) | search and the line follow the target document | #431 |
| [#434](https://github.com/Guyot-Bertrand/sub-edit/issues/434) | appending one file after another | — |
| [#435](https://github.com/Guyot-Bertrand/sub-edit/issues/435) | **deciding whether the phase delivers multiple projects** | #432, #433, #434 |
| [#436](https://github.com/Guyot-Bertrand/sub-edit/issues/436) | extracting the project page from the window | #435 |
| [#437](https://github.com/Guyot-Bertrand/sub-edit/issues/437) | tabs: several projects in one window | #436 |
| [#438](https://github.com/Guyot-Bertrand/sub-edit/issues/438) | save all, close all | #437, #432 |
| [#439](https://github.com/Guyot-Bertrand/sub-edit/issues/439) | splitting a project in two | #437, #434 |
| [#440](https://github.com/Guyot-Bertrand/sub-edit/issues/440) | searching all open projects | #437, #433 |

The phase closes on [#441](https://github.com/Guyot-Bertrand/sub-edit/issues/441), the
end-of-phase review.

> **Written at the end-of-phase review.** Two issues were added to this table, after #440:
> [#442](https://github.com/Guyot-Bertrand/sub-edit/issues/442), the columns, without a milestone, and
> [#453](https://github.com/Guyot-Bertrand/sub-edit/issues/453), the drag and drop #437 had left,
> within the phase. And #437 took a documentation patch of its own, #454, which created #453.

**#429 and #434 depend on nothing** and can be done at any time; #429 is placed first
because three issues are waiting for it. **#432 is the biggest of the first slice**, and
for two opposite reasons: it carries an opening dialog with three choices, a separate
save, and the richer answer of closing — which serves all three slices.

**#436 is a refactoring with no new behaviour**, and that is what makes it risky: all the
window tests must pass without rewriting any of them. If one of them has to change, the
separation was not the one we believed.

## The issues opened by the review

End-of-phase review, [#441](https://github.com/Guyot-Bertrand/sub-edit/issues/441), on
2026-09-24. **Four lines retained**, each an issue of the milestone; the phase closes after
them.

| Issue | What it says |
| :---- | :----------- |
| [#460](https://github.com/Guyot-Bertrand/sub-edit/issues/460) | `MainWindow`: 1,951 lines at the opening, 2,968 at the review. `ProjectPage` handed back the state, not the behaviour — all the gestures of the third slice were added to the window |
| [#461](https://github.com/Guyot-Bertrand/sub-edit/issues/461) | the operations target the page of the current tab; `Save All`, the closing question and `Replace All` over all projects **therefore change tab to target a page** — the screen flickers and the player changes film at each pass |
| [#462](https://github.com/Guyot-Bertrand/sub-edit/issues/462) | `Split Project…` says nothing and does not show where it cuts — the only deviation from Gaupol nobody wanted |
| [#463](https://github.com/Guyot-Bertrand/sub-edit/issues/463) | dropping files **onto the film** — a native window that libmpv draws — can only be checked on a real desktop |

> **#460 delivered**, [ADR 0034](../adr/0034-trois-collaborateurs-de-la-fenetre.md): three collaborators —
> `TableColumns`, `ProjectSearch`, `ProjectFiles` — which receive the page they target and are tested
> without a window. `main_window.cpp` went from 2,968 to 2,319 lines, without a window test being
> rewritten. The gestures over several projects still change tab to target a page: that is #461.
>
> **#461 delivered**: `applyOperation` and `ProjectSearch::View::apply` receive the target page, each page
> keeps its own tab up to date (`refreshTabOf`), and `Save All`, the closing question and `Replace
> All` bring a tab to the front only before a `Save As…`. Two window tests prove it — the tab does not
> move, the player opens no film — without an existing window test being rewritten.
>
> **#483 delivered**, ADR 0034 completed: `WindowActions` builds the actions, their shortcuts, the
> menus and the toolbar; the window keeps it as a single member and connects each action to its
> slot. `main_window.cpp` goes from 2,531 to 2,216 lines, and no window test is rewritten.
>
> **#484 delivered**, ADR 0034 completed: `VideoPane` owns the player, the surface, the strip, the
> timer and the page it plays for, and receives the target page. `main_window.cpp` goes from 2,216 to
> 1,923 lines, and no window test is rewritten.
>
> **#485 delivered**, ADR 0034 completed: `StatusLine` owns the four permanent labels of the status
> bar and refreshes itself from the displayed project. `main_window.cpp` goes from 1,923 to 1,851 lines,
> and no window test is rewritten.
>
> **#486 delivered**, [ADR 0035](../adr/0035-les-operations-sortent-de-la-fenetre.md): `ProjectOperations`
> carries the thirteen operations of `Tools` and the single route to the history; each operation receives the
> page and reads its target from that page's selection. `main_window.cpp` goes from 1,851 to 1,506 lines, and
> no window test is rewritten.

**Ruled out, with their reason.**

- **A document's name — its file, or `untitled` — is computed in four places** (`titleFor`,
  `tabLabelFor`, `modifiedDocuments`, `mayReplaceTranslation`). Four lines each, which do not
  diverge: one more function would make nothing safer. If #460 splits the window, the
  question will come up again on its own.
- **The closing list does not say which project a document belongs to**: two modified
  `untitled` read the same there. Neither does Gaupol's list; and the checkboxes stay right,
  since they are read in the order of the list and not by their name.
- **`Document::Main` hard-coded**: none is left wrongly. In the window, they are `Save`, the
  loops over both documents and the default of the target document; in the core, the "main"
  overloads and the duration adjustment, which stays with the main document on purpose (D3);
  in the command line, the three that D9 leaves to phase 13.

**The benchmark.** Two records for the whole phase, 0.11.1 and 0.11.7: each following
version found the machine above the threshold, and **the third slice has no measurement**.
The review did not find it any calmer — load 5 for a threshold of 1.5. The record is due at
closing; issue [#270](https://github.com/Guyot-Bertrand/sub-edit/issues/270).

