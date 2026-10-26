# Phase 5 — Interface: tabular editing

Framed on 2026-08-17, issue [#125](https://github.com/Guyot-Bertrand/sub-edit/issues/125).

## Goal

The heart of the usage: **open a file, edit it in a table, save it, undo.** This
is the first phase whose product is a window, and the first where a user sees
the project other than through a terminal's output.

The four previous phases built a core and a command line. This one adds almost
no capability to the core: it exposes what exists. Three commands — `SetStart`,
`SetEnd` and `Insert` — in fact have **no end-to-end proof** to date, and two
others — `SetText` and `Remove` — are only reached indirectly, through the
removal of annotations that composes them. No subcommand exposes them. The
window is the first surface that calls them for their own sake, and it covers
only two of them: `SetStart` and `SetEnd`, `Insert` being left to phase 7.

## Scope

**What the phase builds:**

- a window, one project at a time, a five-column table — number, start, end,
  duration, text;
- in-place editing of the text, the start and the end;
- open, save, save as;
- undo and redo, wired to the core's history;
- four operations through a dialog — shift, transform, convert the frame rate,
  remove hearing impaired annotations;
- marking out-of-order subtitles.

**What is not in it, and where it goes:**

| Set aside | Where | Why |
| :-------- | :---- | :-- |
| inserting and removing subtitles from the window | phase 7 | editing cells is the heart of the usage; adding a row is not |
| persisted configuration, preferences | phase 7 | the roadmap already puts it there, and nothing persistent is required for the table to work |
| multiple projects in tabs | phase 11 | outside the MVP outline |
| translation column | phase 11 | the model accommodates it, the interface does not build it |
| configurable columns, highlighting of differences | not planned | outside the MVP outline |
| frame-based editing mode | phase 9 | no frame-based format before MicroDVD |
| editing the duration | phase 10 | the core has no `SetDurationCommand`, and phase 2 stopped at "a text, a start, an end" |

**Three pieces of core work come before the window**, and they are in the phase
because it triggers them: the reorganisation of `format/` and `model/`, the
split of the diagnostics, and the compact representation of selections and
changes ([#45](https://github.com/Guyot-Bertrand/sub-edit/issues/45)).

## Applicable decisions

| ADR | What it settles |
| :-- | :-------------- |
| [0001](../adr/0001-cpp20-et-qt6.md) | Qt 6, and a core with no dependency on the interface |
| [0010](../adr/0010-annulation-par-commandes.md) | commands carrying their own inverse |
| [0012](../adr/0012-ordre-des-sous-titres-par-composition.md) | the model does not sort on its own |
| [0014](../adr/0014-registre-d-exigences.md) | requirements cited by a test tag |
| [0018](../adr/0018-vocabulaire-des-formats-dans-le-modele.md) | the format vocabulary in the model, the operations in `format/` |
| [0019](../adr/0019-table-en-adaptateur-mince.md) | a table that reads through the model, a core that returns its changes |

## What Gaupol does, and where we diverge

The analysis covered `gaupol/view.py`, `page.py`, `application.py`,
`renderers/multiline.py`, `config.py` and the operation dialogs.

**It duplicates the table entirely.** `page.py` copies every subtitle into a
`Gtk.ListStore` and keeps it up to date through eight signal handlers. The price
is written in its code: `reload_view_all()` rebuilds everything on opening,
`_on_project_subtitles_removed` disconnects the model from the view beyond fifty
removed rows — "a large series of live updates made directly to the view is
slow" —, and an `iterate_main()` lingers after each refresh. We take the thin
adapter, and [0019](../adr/0019-table-en-adaptateur-mince.md) says why.

**It does not store the number**: a cell function computes it from the row
index. We do the same — the number is a rank, not data.

**It has no configuration migration, and that is a mechanism.** The file is read
option by option; an unknown key is accepted then erased on writing if it is no
longer among the defaults; an unreadable value prints on error output and
leaves the default; every option left at its default value is rewritten
**commented out**, so that a change of default takes effect for anyone who never
overrode it. `general.version` is written and never read back: a trace, not a
trigger. The finding is recorded here for phase 7, which inherits it.

**Its input frame rate comes from a preference.**
`FramerateConvertDialog._init_values()` sets both lists to
`page.project.framerate`, which comes from `conf.editor.framerate` — 23.976 by
default. No file name heuristic, no reading of the video, in twenty years. See
"The origin of the input frame rate" below.

**Its multi-line editing cost it a widget.** `MultilineCellRenderer` builds a
`CellTextView`, a `Gtk.TextView` that implements `Gtk.CellEditable`, with
`Enter` and `Escape` handled by hand and a workaround on focus loss — "removing
the editor there sends GTK's focus handling into an infinite loop". Qt offers
the same gesture through a documented path: a `QStyledItemDelegate` that returns
a `QPlainTextEdit`.

## The core before the window

### 1. The format vocabulary joins the model

`SourceFile` retains everything about the original file except its format. A
window that opens then saves needs it, and adding it there would create the
first `model/ → format/` edge.

[0018](../adr/0018-vocabulaire-des-formats-dans-le-modele.md) settles it:
`format/` keeps only the operations, the vocabulary goes to the model, and what
does not even know the word "subtitle" leaves both — `io/` for the disk, `text/`
for strings. `SourceFile` then carries its format without a new edge.

The move is **mechanical and without any change of behaviour**: 38 files whose
includes change. At the time it made `tidy-scope.sh` fall back to the full
analysis, hence a gate at full price, paid once. That script disappeared with
#269: the analysis now follows the dependencies the compiler has written, and a
move of headers reanalyses what includes them, nothing more.

### 2. The diagnostics split

Three categories of `DiagnosticKind` do not describe what a reading encountered
but what a document is. They separate, and the locator says why: **a line exists
only at the moment of reading, an index survives editing.**

| Stays in `format/diagnostic.hpp`, located by a line | Moves to `model/anomaly.hpp`, located by a `SubtitleIndex` |
| :--- | :--- |
| `IgnoredLine`, `MalformedTimestamp`, `MissingNumbering`, `InconsistentNumbering`, `TextBeforeAnyTimestamp`, `UnknownBlock`, `MixedNewlines` | `EndBeforeStart`, `OverlappingSubtitles`, `OutOfOrder` |

The model exposes `scanAnomalies(const Project&)`, which **`inspect` and the
window** read. `Project::outOfOrder()` disappears with `OrderReport`: nothing
read the list any more, and the strict order policy makes do with an
`isInOrder()` that allocates nothing. **`--order-report` goes in the same
move**, for lack of a consumer — its removal was announced for #134, it followed
the function that fed it.

**What it changes for the user**, corrected when doing it: I feared here the
loss of a line locator. It does not happen for disorder — the `order:` line of
`inspect` **already gave a subtitle number**, under the misleading label
"line". The real change is elsewhere: end before start and overlap leave the
diagnostics, where they carried a real file line, to join the anomalies, which
carry a subtitle. It is a less precise locator for whoever opens the file in a
text editor, and it is accepted because the index is the only one that survives
an edit — otherwise no anomaly could be computed after a modification.

The `order:` line becomes `anomalies:`, and its label stops lying.

**The reading of disorder is settled: `Breaks`.** On the starts
`0, 4000, 2000, 3000`, it names `{2}` where the other names `{2, 3}` — yet row 3
is in its place relative to row 2: reporting it points at a row with nothing to
correct. A table that highlights the faulty rows must highlight the ones to be
moved.

**The corpus did not decide, and that must be said.** Phase 3 had exposed both
readings under `--order-report` to "compare them on real files". Both readings
were run on the fifteen files of `src/data/`: **none is out of order**, they
agree trivially everywhere, and eight are not valid UTF-8. The decision is
therefore made by reasoning, for lack of data, and not by measurement.
`--order-report` disappears.

### 3. The cost of an operation on a whole file

[#45](https://github.com/Guyot-Bertrand/sub-edit/issues/45) goes **before** the
table, and that order changes what it must produce.

The table model translates a `Change` into `dataChanged(topLeft, bottomRight)`:
**Qt wants ranges, not indices.** A table written before #45 would unpack four
thousand indices only to glue them back into intervals; written after, it
consumes the compact form as is. The representation that #45 introduces must
therefore **expose directly usable intervals**, and not merely take up less
memory.

Two notes on the measurement and the scope:

- it will be taken on the core benchmarks and not on the window, which does not
  exist yet; the history footprint is already quantified — 32 KB per shift,
  64 KB per transformation — and that is the point of comparison;
- its point 2, the quadratic removal, keeps its reason for being despite the
  absence of a "delete" menu: `removeHearingImpaired` produces a `RemoveCommand`
  that undo reinserts.

## The architecture of the window

### Where the code lives

`src/lib/subedit/gui/`, library `subedit::gui`, on the model of `subedit::cli`.
`src/exe/gui/main.cpp` stays wiring.

This is not a choice of style: `check-architecture.sh` refuses a `main` that
defines a class or exceeds its line limit, and refuses any Qt include under
`core/`. The boundary is held by the gate.

### The table model

`SubtitleTableModel : QAbstractTableModel`, a thin adapter over
`const Project&`.

| Column | Content | Editable |
| :----- | :------ | :------- |
| number | `row + 1`, never stored | no |
| start | `Timestamp::format()` | yes |
| end | `Timestamp::format()` | yes |
| duration | `end − start`, formatted | no |
| text | the text of the main document | yes |

`data()` fetches `project.subtitleAt(index)` and formats on the fly. No subtitle
is copied, and the model never materialises what is not visible.

The duration is not editable: the core has no command for it, and inventing one
here would be core work slipped into an interface phase.

The decimal separator displayed follows the project's format — comma for
SubRip, point for WebVTT — so that what one reads on screen is what will be
written.

### What the window learns from a change

`Session::apply`, `undo` and `redo` return `std::vector<Change>` instead of
`void`. Undo returns it inverted, through a free function `invert(ChangeKind)` —
the indices are the same in both directions.

The model translates:

| What the `Change` says | What the model emits |
| :--------------------- | :------------------- |
| `Positions` | `dataChanged` on the start, end, duration columns |
| `MainText` | `dataChanged` on the text column |
| `Reordering` | `dataChanged` over the full width of the ranges |
| `Insertion`, `Removal` | `beginResetModel` / `endResetModel` |

The reset is the price of the order of things: Qt requires a structural change
to be bracketed **before** it happens, `Session` only reports it afterwards, and
no command can predict it — `removeHearingImpaired` only knows which subtitles
the rule empties once the rule has been applied.
[0019](../adr/0019-table-en-adaptateur-mince.md) carries the decision and its
trigger for re-examination.

### Undo

No `QUndoStack`. `canUndo()` and `canRedo()` drive two `QAction`s; the label
"Undo: …" comes from `CommandKind` translated by a free function of
`subedit::gui` — the enumeration exists for that, its comment says so.

Written as the French label for "Undo: …" at framing time, and corrected at
review: the interface has been in English since
[#145](https://github.com/Guyot-Bertrand/sub-edit/issues/145), and a spec that
keeps the old word makes one doubt the code rather than itself.

**The grouping question left open in phase 2 closes without a mechanism.** A
`QStyledItemDelegate` commits once, on `Enter` or on focus loss: an edited cell
produces **one** command. There is nothing to group, hence nothing to build.

### In-place editing

Three delegates:

- **text** — `QPlainTextEdit`, multi-line, `Enter` commits, `Shift+Enter`
  inserts a line break, `Escape` cancels;
- **start** and **end** — a constrained input field, read by `Timestamp::parse`,
  which is already permissive as real files demand; an unreadable entry leaves
  the cell unchanged.

A commit that changes nothing produces no command: the history has no need to
remember a keystroke followed by `Enter` on an identical text.

## The visible surface

### Opening

`subedit-gui <file>` first — the window is useful and testable as soon as it
takes a path as an argument —, the open dialog afterwards.

Reading goes through `readSubtitles`. A `ReadError` gives a message and nothing
opened. **The reading diagnostics are shown** in a collapsible panel: it is the
first place in the project where they reach a user other than through `-vvv`.

*Replaced at the phase 13 review ([#605](https://github.com/Guyot-Bertrand/sub-edit/issues/605)): the collapsed panel
took a row of the window for rare information, read once. They are now a button
of the status bar and a floating list; `GUI-OPEN-03` does not change.*

### Save, save as

Save rewrites in the original format, line endings, BOM and header —
`SourceFile` carries them all, and will carry the format after the
reorganisation. Writing goes through the existing atomic write, and
`Session::markSaved` resets the modification counter to zero.

"Save as" adds the choice of path and format.

**The `FormatExtras` of the other variant: looked at, and nothing to do.** The
question was open — a project read as WebVTT carries `WebVttExtras` that a
SubRip writer has no use for, and vice versa. Both writers query the variant
through `std::get_if` and fall back on their default values when it belongs to
the other format: `sub_rip_writer.cpp` for the coordinates, `web_vtt_writer.cpp`
for the identifier, the settings, the style and the comment.

What does not belong to the written format is therefore **silently ignored**,
and neither translated at random nor copied as is. It is the right behaviour, it
was already there, and nothing said so: two tests now say it. The extras are not
lost for all that — they remain in the project, and saving again in the original
format rewrites them.

Found in issue [#131](https://github.com/Guyot-Bertrand/sub-edit/issues/131).

Closing or opening another file while `hasUnsavedChanges` is true asks for
confirmation.

### The operations

Four dialogs, and a common target: **the selection, or the whole file**. This is
what phase 4 had sent here — "phase 5, which brings a real selection".

| Dialog | What it asks for |
| :----- | :--------------- |
| shift | a signed duration |
| transform | two reference points, index and corrected position |
| convert the frame rate | input frame rate, output frame rate |
| remove annotations | nothing but a confirmation, and a report |

The removal of annotations requires **one more signature from the core**:
`removeHearingImpaired` currently walks the whole project unconditionally, and
must accept a `Selection`.

An operation that changes nothing — `removeHearingImpaired` returns `nullptr` in
that case — does not enter the history and says so.

### The origin of the input frame rate

**Question closed, and without a heuristic.** The conversion dialog asks for the
input frame rate, pre-filled with the project's.

The file does not carry it and cannot carry it: SubRip has no header, WebVTT's
is free text, and the two MVP formats are time-based. A heuristic drawn from the
file name was **set aside for lack of data** — the fifteen files of `src/data/`
were renamed by hand, their names are the product of a clean-up and not an
observation, and no file with intact names is available. Getting the frame rate
wrong shifts the whole file without reporting anything: silent and global, the
worst failure mode of this operation.

Gaupol does no better and never has; iso-functionality is reached.

**Two leads exist, and neither belongs to this phase.** The video container
*declares* its frame rate, and phase 6 already associates a video with the
project. And the positions themselves *betray* it: converting a frame rate
assumes they were aligned on a frame grid, and that grid can be measured — eight
of the fifteen files of the private corpus give it unambiguously, against a
background noise of a few percent. That is the subject of **phase 16**, born of
this framing.

**Phase 5 depends on neither.** Its dialog asks for the frame rate and
pre-fills it with the project's; the day a measurement exists, it will replace
that pre-filling without the dialog changing shape. Writing the reverse —
waiting for the measurement to build the dialog — would make the MVP depend on
a capability that exists nowhere else.

No dedicated action declares the frame rate: with two time-based formats, it
only serves the conversion, and it is the dialog's input list that declares it.
Gaupol has a menu because Gaupol reads frame-based formats; we have none before
phase 9.

**The frame rate `ChangeKind` stays set aside.** Phase 2 had set the condition —
"if the window displays the current frame rate, it will need to know that it
changed". It does not display it. The enumerator would still have no reader, and
the rule that `DiagnosticKind` states holds: an enumerator that nobody reads is
a promise without a guarantor.

### Disorder on screen

The subtitles that `scanAnomalies` reports are marked in the table — a tinted
background on the position columns, and a tooltip that names the anomaly. The
marking is recomputed after every change of positions.

### What the window does not have

Named so that nobody looks for it: no search bar, no clipboard, no video
preview, no preferences, no tabs, no translation column, no insertion menu.

## Tests

The harness is that of [#119](https://github.com/Guyot-Bertrand/sub-edit/issues/119):
**Catch2, and from Qt only its driving functions.** The binary provides its
`main`, the `QApplication` lives on its stack, and `QT_QPA_PLATFORM=offscreen`
is set by default.

| Level | What is tested |
| :---- | :------------- |
| unit, without widgets | the translation of a `Change` into model signals; `invert`; the label of a `CommandKind`; `scanAnomalies` |
| table model | `rowCount`, `columnCount`, `data` on each column, `setData` producing one command and only one |
| window, driven by `QTest` | open a file, edit a cell, undo, redo, save, each operation dialog |

**What an interface test must not become:** a pixel capture. We assert on what
the model returns and on what the session contains, never on a rendering.

The file corpus of `src/test/data/` serves for opening; **`src/data/` is read by
no test**, the project's rule not changing here.

**A limit, written at review because it was nowhere.**
`src/exe/gui/main.cpp` is only proven by `--version`. The "a file as argument"
path, and the message the manual quotes when it fails, are checked by nothing —
and **no end-to-end test can reach them**: once past the argument, the binary
shows a window and enters `QApplication::exec()`, which it leaves only when a
window that nobody closes is closed. What the window does is therefore tested
one notch lower, against `MainWindow`, as the #119 harness had planned. What
remains without proof is the wiring of `main`, some fifteen lines that
`check-architecture.sh` keeps trivial — and it is precisely for that reason that
it keeps them trivial.

The spec also announced that the versioned corpus served for opening, and no
window test read it: they all went through `InMemoryFileSystem`. Corrected in
[#153](https://github.com/Guyot-Bertrand/sub-edit/issues/153), which opens a
corpus file through a real file system.

## Measurements

`make bench` runs on every issue, as everywhere. Two new measurements arrive
with the code that makes them possible:

- **#45** — history footprint and cost of a removal, before and after, in
  `Release`. Point of comparison: 270 B per text edit, 32 KB per shift, 64 KB
  per transformation.
- **the table model** — build a model on a project of four thousand subtitles,
  and measure `data()` over a window of visible rows. It is the measurement that
  says whether the thin adapter keeps its promise.

There is no interface benchmark in the sense of a driven window: #116 set it
aside, and its reason still holds — the measurement comes with the code it
measures.

**The footprint was not measured, and it could not be.** The project has no
memory measurement instrument: `make bench` times, and nothing else. #45
therefore returned times — removing one subtitle out of two went from 9.09 ms
to 168 µs, removing annotations from 5.34 ms to 1.15 ms — and never the bytes
this line promised. Found at review, and written here rather than left to be
believed.

What can be asserted is read from the structures, and it is a derivation, not a
measurement:

| Operation on 4000 subtitles | Before | After |
| :-------------------------- | -----: | ----: |
| shift | 32 KB | **two bounds** |
| transformation | 64 KB of positions + 32 KB of selection | 64 KB of positions |
| text edit | 270 B | 270 B |

The shift was **entirely** its selection: `ShiftCommand` retains only a
`Duration`, its inverse being the same shift reversed, exact and without
rounding. The transformation keeps one position per subtitle and will keep them
— scaling rounds, so it is not invertible without them; the saving of
[ADR 0010](../adr/0010-annulation-par-commandes.md) only applies to commands
that carry their own inverse. It therefore gained only the 32 KB of its
selection, that is a third.

**A footprint measurement remains to be built the day a decision depends on
it.** None depends on it today: the history ceiling is a thousand entries, and
the heaviest command of the MVP weighs 64 KB.

**A new measurement was born of the review**: the model reset on a structural
change, which [ADR 0019](../adr/0019-table-en-adaptateur-mince.md) asked to
quantify before phase 7 built on it. Answer, on an idle machine: **8.15 µs**
after a removed row, against 8.25 µs to refresh a shift of four thousand
subtitles and 8.38 µs to build the model. **All three are the same figure**, to
within a twentieth, and it is the `scanAnomalies` they share — on the model
side, resetting costs nothing more than refreshing. The ADR's trigger will
therefore not find its argument there, but on the view side, which a benchmark
binary without a `QApplication` cannot reach. The measurement does not answer
phase 7's question: it moves it, and says where it lives.

## Requirements

Registered in the `planned` state, as the rule requires, before the code.

| ID | Requirement |
| :- | :---------- |
| `GUI-VERSION-01` | `subedit-gui --version` writes `subedit <version>`, code 0 |
| `GUI-OPEN-01` | `subedit-gui <file>` opens the file and displays its subtitles |
| `GUI-OPEN-02` | an unreadable file gives a message and leaves the window empty |
| `GUI-OPEN-03` | the reading diagnostics are shown, with their line |
| `GUI-TABLE-01` | the five columns display number, start, end, duration and text |
| `GUI-TABLE-02` | the subtitles with an anomaly are marked and named |
| `GUI-EDIT-01` | editing a text cell modifies the subtitle and nothing else |
| `GUI-EDIT-02` | editing a start or an end reads a permissive timestamp |
| `GUI-EDIT-03` | a commit that changes nothing does not enter the history |
| `GUI-UNDO-01` | undo restores the previous state, the action names the operation |
| `GUI-UNDO-02` | redo redoes what was just undone |
| `GUI-SAVE-01` | save rewrites the file in its original form |
| `GUI-SAVE-02` | save as chooses path and format |
| `GUI-SAVE-03` | closing with unsaved changes asks for confirmation |
| `GUI-SHIFT-01` | the shift dialog shifts the target |
| `GUI-TIME-01` | a field that sets a time is typed over, not inserted into: only digits (and `-` where a negative time makes sense) are taken, and the cursor skips the separators by itself, in the shift, the transformation, the range of a transcription and the position cells of the table |
| `GUI-LOOK-01` | a read-only text area in a dialog is delimited — a thin frame and a background of its own — in both palettes |
| `GUI-TRANSFORM-01` | the transformation dialog corrects by two reference points |
| `GUI-SCOPE-01` | an operation dialog offers the choice between the selection and the whole project when only part is selected |
| `GUI-PREVIEW-01` | an operation dialog shows, before applying, the first subtitles it would change |
| `GUI-SORT-01` | `Sort Subtitles` puts the project in order of starts, undoably, and says how many subtitles changed place |
| `GUI-SORT-02` | a file opened out of order offers to be sorted, and is sorted only on a yes |
| `GUI-SORT-03` | the list of diagnostics counts by kind, reading ones as well as subtitle anomalies, follows the corrections, and leads to the first subtitle concerned |
| `GUI-SORT-04` | an indeterminate bar in the status bar says that a sort is in progress, and goes away with it |
| `GUI-DISJOINT-01` | `Make Ranges Disjoint…` cuts the overlapping subtitles over the whole project, undoably, selects the rows written and says what it did; out of order, it is refused with the reason |
| `GUI-DISJOINT-02` | a file opened with correctable defects (repeated, out of order, overlaps) offers them in a single box, one checkbox per correction counted where it is made, and applies only what is checked, in the order join, sort, cut |
| `GUI-JOIN-01` | `Join Repeated Subtitles` makes a single subtitle of the consecutive pieces of the same text, without depending on the file's order, undoably, and says how many it joined |
| `GUI-JOIN-02` | `Repair Subtitles…` chains the joining of repeated subtitles, the sort and the cutting of ranges in one go, undoably, and reports a single account of it |
| `GUI-FRAMERATE-01` | the conversion dialog re-times the target |
| `GUI-HEARING-01` | the removal of annotations applies to the selection or to the file |
| `GUI-HEARING-02` | a removal that changes nothing says so and does not enter the history |
| `CLI-INSPECT-04` | a document's anomalies are reported by subtitle number |
| `CLI-INSPECT-08` | `inspect` counts the anomalies by kind, one per line with `--detail`, and keeps `none` on the `anomalies` line when there are none |

`GUI-VERSION-01` was added to this table at review: it was born with the #119
harness, before this list was written, and the registry carried it without the
spec counting it. Nineteen, therefore, and not eighteen.

`CLI-INSPECT-04` was added at the phase 7 review, and for the same reason:
phase 5 created it, the spec named it in the sentence below and never in the
table. **Twenty, therefore.** It is the command-line row that an interface phase
gave birth to — anomalies become a core notion here, and `inspect` reports them
at the same stroke.

One registry row moves besides: `CLI-INSPECT-03` moves to `abandoned` —
`--order-report` disappears.

## Manual

`docs/manual/subedit-gui/`, **in prose**. #116 set aside generated screenshots
and a generated description of the widget tree, with their reasons;
`make manual` will not maintain this manual, and its correctness rests on the
end-of-phase review. It is written here so that nobody believes otherwise.

Each issue updates the section it changes, at step 7 of the project's order —
after the gate, which does not read the manual.

## Split into issues

| Order | Issue | Depends on |
| :---: | :---- | :--------- |
| 1 | [#126](https://github.com/Guyot-Bertrand/sub-edit/issues/126) — tidy up the format vocabulary, the operations, and the rest | — |
| 2 | [#127](https://github.com/Guyot-Bertrand/sub-edit/issues/127) — split reading diagnostics and a document's anomalies | #126 |
| 3 | [#45](https://github.com/Guyot-Bertrand/sub-edit/issues/45) — cost of an operation on a whole file | #126 |
| 4 | [#128](https://github.com/Guyot-Bertrand/sub-edit/issues/128) — the window and its table model | #127, #45 |
| 5 | [#129](https://github.com/Guyot-Bertrand/sub-edit/issues/129) — editing a cell: multi-line text, start, end | #128 |
| 6 | [#130](https://github.com/Guyot-Bertrand/sub-edit/issues/130) — undo, redo, and the action's label | #129 |
| 7 | [#131](https://github.com/Guyot-Bertrand/sub-edit/issues/131) — open, save, save as | #130 |
| 8 | [#132](https://github.com/Guyot-Bertrand/sub-edit/issues/132) — shift, transform, convert the frame rate | #131 |
| 9 | [#133](https://github.com/Guyot-Bertrand/sub-edit/issues/133) — remove annotations, on a selection | #132 |
| 10 | [#134](https://github.com/Guyot-Bertrand/sub-edit/issues/134) — mark the anomalies in the table | #128, #127 |
| 11 | [#135](https://github.com/Guyot-Bertrand/sub-edit/issues/135) — end-of-phase 5 review | all |

**Three issues were born during the phase**, and this table ignored them:
[#145](https://github.com/Guyot-Bertrand/sub-edit/issues/145), which put the
interface back in English — the language rule did not cover what the binary
writes, and the silence cost a whole window in French;
[#152](https://github.com/Guyot-Bertrand/sub-edit/issues/152), a coverage tree
made stale by the move of #126; and
[#153](https://github.com/Guyot-Bertrand/sub-edit/issues/153), what the review
found in the code.

The order 5 → 6 → 7 deserves a word: undo comes **after** editing because there
must be something to undo, and opening **after** undo because a path as argument
suffices until then. Each issue leaves a binary that is good for something.

## Deferrals

Every "later" of this spec lands on a named phase.

| What is deferred | Where |
| :--------------- | :---- |
| reading the frame rate from the video container | phase 6 |
| deducing the frame rate from the positions, and correcting | phase 16 |
| reporting positions that fall off the frame grid | phase 16, which says why not here |
| insertion and removal from the window | phase 7 |
| persisted configuration, preferences, theme | phase 7 |
| editing the duration | phase 10 |
| frame-based editing mode | phase 9 |
| multiple projects, translation column | phase 11 |
| a `Session` that announces a structural change before making it | phase 7, if the measurement calls for it |

**All nine land, two of them only since the review.** Phase 9 said nothing about
the frame-based editing mode, and phase 10 carried the adjustment of durations
without saying that the `Duration` column would become editable with the
command it lacks. A deferral to a phase that does not mention it is not a
deferral: it is an oversight that looks tidied away.

## What the review found

[#135](https://github.com/Guyot-Bertrand/sub-edit/issues/135), on the spec, the
registry, the manual, the thirty files of `src/lib/subedit/gui/`, the tests and
the two measurement records.

**A defect visible on screen**, and only one. The four operation dialogs
received the project's count where the operation applied to the target: two
rows selected out of four gave "Applies to: 4 subtitles" and two subtitles
changed. The only test of the label built the dialogs by hand, without going
through the window — **the guarantee was exactly where the defect was not.**
Corrected in [#153](https://github.com/Guyot-Bertrand/sub-edit/issues/153).

**A language lost from sight.** Eleven of the thirty files of `gui/`, two of
`cli/` written during this phase, twelve of the fourteen interface test files:
their comments were in French, against the project's rule, and zero everywhere
else in the core. The same silence as the one that had given the window in
French in #145 — except that the rule, this time, existed. Corrected in #153,
four hundred and forty lines.

**Three axes, and two issues.** #153 carried the rest of the code: modal boxes
that were parented to no window, three comments describing a bygone state, the
reset benchmark, opening a corpus file.
[#154](https://github.com/Guyot-Bertrand/sub-edit/issues/154) carries what
could not fit into an end-of-phase gate: `gui/opening.cpp` and `gui/saving.cpp`
contain not a single line of Qt, the recipe they hold is written four times in
the repository, and the window crushes four causes of opening failure into a
single message where the command line distinguishes them.

**What holds.** The nineteen `GUI-…` requirements are cited by at least one
test, tag by tag. The boundary of ADR 0018 has not moved: `format/` contains
only readers, writers, a detection and the types they produce.
`OperationDialog` carries what is common to the four dialogs and nothing more —
the defect there was a badly fed parameter, not a repetition. And the three
guards that no user can reach are traversed by tests, which is the right answer
to "a guard that no test traverses is a promise that nobody checks".

## Added by #805: typing a time, and the look of a read-only area

Two usability points seen in use, **generalised** rather than fixed where they were seen.

- **A time is typed over, as in Gaupol** (`GUI-TIME-01`). `TimestampEdit` is a `QLineEdit` whose text is the time
  `Timestamp::parse` reads, in a fixed shape `HH:MM:SS,mmm`: a digit **replaces** the digit under the cursor and the
  cursor moves on, **skipping `:` and `,`** itself; **only digits** are taken, and `-`/`+` where a negative time makes sense;
  Backspace and Delete put a zero back, the arrows move from digit to digit. Three shapes: a position (signed, with
  milliseconds: shift, transformation and the position cells of the table), a duration (unsigned, the `Duration`
  cells), and a range bound (no milliseconds, **empty allowed** — empty means "no bound"). A field that must hold a time begins as
  zeros, so that there is a shape to overwrite. **A text given to the field** — a value to start from, a paste, a test —
  **is put into shape when the time it holds is readable** (`1:02` becomes `00:01:02,000`) and is otherwise left alone, so what
  reads `text()` keeps reading it. The frame-number cells keep their own field (a whole number, not a time). The grammar of
  the transcription range (`2:00`, seconds) is still what the command line reads; the window's field is the shaped one.
- **A read-only text area is delimited** (`GUI-LOOK-01`). `styleAsReadOnly` gives it a thin sunken frame and the palette's
  `AlternateBase` as background, in both palettes; the two references of the transformation dialog use it, and a future
  read-only area is given the look by the same call.

## Completion criteria

- [x] The window opens, edits, saves, undoes and redoes
- [x] The four operations are accessible, on a selection or the whole file
- [x] The core is reorganised, the diagnostics split, #45 done and measured — in
      time; its footprint is derived and not measured, and the "Measurements"
      section says why
- [x] `--order-report` has disappeared, from the binary, the manual and the
      registry
- [x] Every `GUI-…` requirement is `implemented` and cited by a test
- [x] `docs/manual/subedit-gui/` describes what the window does, and nothing
      more
- [x] The benchmarks are rerun and the measurements recorded
- [x] The end-of-phase review has taken place
