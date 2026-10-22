# Roadmap

Twenty-five phases, each tied to a GitHub milestone. Nine lead to a
**shippable MVP** — the first eight, plus [16](#16--frame-rates-inference-and-correction)
slotted in before 7; the following ones, up to 15, complete feature parity
with Gaupol; the last eight, from
[17](#17--additional-text-formats) to [24](#24--workflow-and-integrations),
go [beyond Gaupol](#part-three--beyond-gaupol).

**A phase's number identifies it; it does not give its rank.** The order is the
order of this document, and that alone. A phase added along the way takes the
first free number rather than shifting the following ones — this is already the
rule for requirement identifiers, and for the same reason: a reference that
changes referent misleads more than a number out of order. Six ADRs, seven files
in `src/` and nine milestones cite phase numbers; shifting them would make all of
them silently wrong.

This document holds the upstream scoping: what has to be analysed, what has to
be decided, and what will be hard. It is revised as the project goes — a
finished phase sees its scoping replaced by its spec in [`specs/`](specs/).

## The outline of the MVP

The priorities come from the end user, given on 2026-08-05:

- **open** — mostly SubRip (`.srt`) and WebVTT (`.vtt`); more rarely
  SubViewer (`.sub`), Sub Station Alpha (`.ssa`), Advanced SSA (`.ass`);
- **save as** — mostly `.srt` and `.vtt`, in UTF-8, with Unix line
  endings;
- **edit subtitles** in the central table;
- **shift positions**, **transform positions**, **convert the frame
  rate**;
- **remove texts for the hearing impaired** — sounds in brackets and in
  parentheses.

Phases 1 to 7 are therefore **restricted to this outline**, interface included.
They change neither identity nor order: they cover less ground.
[16](#16--frame-rates-inference-and-correction), for its part, widens it — the
only one to do so, and the only one to go beyond feature parity with Gaupol
before part three.

Three consequences that cannot be read directly from the list:

- **The markup model is needed from phase 1.** SubRip carries `<i>`,
  `<b>`, `<font>`, WebVTT its own. It must moreover be designed to
  accommodate ASS in phase 9, otherwise that phase would force it to be redone.
- **The undo architecture is laid down in phase 1.** It is not in the list,
  but editing in a table implies it, and it is precisely what cannot be added
  after the fact.
- **The translation document is not settled.** The data model is therefore
  designed to accommodate it, without the interface building it: adding it to
  the model later would be expensive, adding it to the interface costs nothing.

## How a phase unfolds

Three issues frame each phase — one to equip it, one to scope it, one to review
it — and the implementation issues come between the last two.

### 1. Initialisation — the tooling (`type:task`)

**Before the scoping.** The question it asks: *what, in this phase, will be
checked by hand for lack of a tool?* What checks itself need not be checked by
hand, and a tool costs less built at the start of the phase than at the end.

It produces one decision per tool: those retained have their issue, those
set aside have **their reason written down** — a tool set aside without a trace
comes back at every phase.

The tooling issues it opens come **before the first implementation issue**.
This is an ordering constraint on the phase and not a task of the
initialisation ticket: when that ticket ends, nothing is in place yet, by
construction. Writing it as an exit criterion would give a box that can never
be ticked.

### 2. Scoping (`type:task`)

1. **Preliminary analysis** — targeted reading of the corresponding Gaupol code,
   in `reference/gaupol`, to understand what is done and why.
2. **Discussion of architecture choices** — the questions listed below are the
   starting point, not the complete list.
3. **Writing the spec** in `docs/specs/NN-<topic>.md`, and ADRs for decisions
   that are expensive to go back on.
4. **Splitting into implementation issues**, attached to the milestone.

### 3. End-of-phase review (`type:task`)

**After the last implementation issue, before closing.** It produces no
feature: it checks that the phase holds.

- **Compare the spec with what was built.** A gap is either fixed or recorded;
  an unrecorded gap is a forgotten gap.
- **Carry deferrals forward.** Every "to be seen later" must land on a named
  phase or issue. Without this a promise ends up pointing at a phase already
  past — it happened in phase 2, where the history bound promised to be
  measured "in phase 2" and was not.
- **Read the manual in one go.** It is updated ticket by ticket, hence in
  pieces, and by people who already know the answer. It is the only moment
  where one sees what it gives for someone discovering it: what is missing,
  what has aged, what is accurate but filed where nobody will look for it.
- **A critical look at the phase's code as a whole**, and not file by file:
  duplication, missing or superfluous abstractions, tests that promise more
  than they prove. What comes out of it **is not fixed there**: each axis is
  discussed and becomes an issue if retained.

A phase is closed only when its spec, its tests, its benchmarks, its manual
section, its CHANGELOG entry and its review exist.

**The analysis is done when the phase concerned starts, not now.** The
questions and difficult points listed below are landmarks noted during the
initial exploration of Gaupol: they serve to avoid starting from a blank page
and to avoid discovering a known obstacle late. They are neither conclusions
nor a closed list.

---

# Part one — towards the MVP

## 0 — Foundations

**Finished on 2026-08-05.** See [`specs/00-fondations.md`](specs/00-fondations.md).

`lib`/`exe`/`test` structure, CMake and presets, `make` facade, a five-step
`make check` gate proven both ways by `make verify-gates`, CI that runs nothing
but this gate, git hooks, generated CHANGELOG, five ADRs, and repository
lock-down.

---

## 1 — Core: data model and formats

**Finished on 2026-08-08.** See [`specs/01-noyau.md`](specs/01-noyau.md).

Subtitle, document, project, positions, SubRip and WebVTT, markup, and the
architecture of reversible commands.

**Restricted to the MVP outline:** two formats out of nine, UTF-8 only, Unix
line endings only, no automatic encoding detection.

**Preliminary analysis** — `aeidon/`: `subtitle.py`, `position.py`,
`calculator.py`, `project.py`, `containers.py`, `revertable.py`, `file.py`,
`files/{subrip,webvtt}.py`, `markup.py`, `markups/{subrip,webvtt}.py`,
`parser.py`.

**Architecture questions**

- **Representation of positions.** Integer milliseconds internally, frames
  derived from the frame rate? Frame rate conversion is among the priorities,
  so round trips must not drift. Which rounding policy, and which guarantee on
  `frames → ms → frames`?
- **Error handling.** Exceptions, return codes, or a result type.
  `std::expected` is available with the installed GCC 13, in `-std=c++23`:
  choosing the result type would mean moving the project to C++23, without
  changing compiler. Decision by ADR; it permeates the whole API.
- **Undo architecture.** Gaupol stores in each action an *inverse function*
  and its arguments (`RevertableAction.revert_function`). Alternative:
  commands that capture the prior state. The first is memory-efficient but
  requires every operation to know how to invert itself exactly; the second is
  robust but expensive on a global replacement. To be decided by measurement.
- **Markup model.** SubRip and WebVTT are enough for the MVP, but the model must
  accommodate ASS in phase 9 — positioning, named styles, effects — without
  being redone. Where does the boundary lie between what is common and what is
  specific to a format?
- **Parsing tolerance.** Real files are malformed. Clean failure, or recovery
  with a diagnostics report? The choice determines the signature of every
  reading function.

**Difficult points**

- The *project* model carries **two documents**, main and translation, which
  **share positions**. The translation has no timings of its own. The model
  must accommodate it even if the interface does not expose it yet.
- A single format interface must accommodate: with or without a header, times
  or frames, disjoint markup sets. If an implementation has to raise "not
  supported", the split is wrong. The risk is low with two similar formats,
  and that is precisely the trap: the design must hold with nine.

---

## 2 — Editing operations

**Finished on 2026-08-09.** See
[`specs/02-operations-d-edition.md`](specs/02-operations-d-edition.md).

What editing in the table requires, plus the three position operations listed
as priorities.

**Restricted to the MVP outline:** change a text, a start, an end;
insert, delete; shift, transform, convert the frame rate. Deferred to
phase 10: duration adjustment, case, italics, dialogue dashes, merge, split,
find and replace, clipboard.

**Preliminary analysis** — `aeidon/agents/`: `set.py`, `edit.py`, `position.py`.

**Architecture questions**

- Is every operation a first-class undoable command, including typing in a
  cell? How is the **grouping** of actions into a single undo entry done? —
  *closed, without a mechanism: a delegate commits once, so an edited cell
  produces one command. See
  [the phase 2 spec](specs/02-operations-d-edition.md#grouping-actions--a-closed-question).*
- **Target** model: selection, range, whole project. Gaupol handles it with a
  `target` parameter repeated in every signature — we can do better.
- Where is the boundary between a core operation and interface logic?

**Difficult points**

- Affine transformation of positions from two reference points, with the
  accumulated rounding errors that implies.
- Frame rate conversion applies to time-based subtitles: it rescales every
  position. Check at scoping what the user expects exactly — resynchronising a
  file timed for 23.976 to 25 frames per second is the common case.

---

## 3 — CLI

**Finished on 2026-08-14.** See [`specs/03-cli.md`](specs/03-cli.md).

The command line does not appear in the user's needs: it serves as a
**validation and measurement harness** for the core, before a window exists.
Subcommands meant for real use belong to phase 13.

**The scope announced here was widened at scoping.** It said inspection,
conversion and shifting; the spec adds two-point transformation and frame rate
conversion. The reason fits in one sentence: the core already implements them,
and a harness that does not expose them does not validate them — they would
remain without any end-to-end test until phase 13.

Two questions left open here are settled there: CLI11 for argument parsing
([ADR 0016](adr/0016-cli11-pour-l-analyse-d-arguments.md)), and four exit
codes distinguishing total failure from partial failure on a batch.

**What `Project::outOfOrder()` must return remains open, deliberately.** Phase
2 compares each subtitle with its immediate predecessor; the other reading
compares with the largest start met so far. On the starts `0, 4000, 2000, 3000`,
the first returns `{2}`, the second `{2, 3}`. The two always agree on whether
there is disorder and differ only on the list.

Inspection is the first caller to consume this list. Rather than deciding
without data, it **exposes both** behind an option, long enough to compare them
on real files. **Phase 5 inherits the choice and removes the option** — that is
its trigger, recorded as such in the spec.

---

## 4 — Removing hearing-impaired texts

**Scoped.** See [`specs/04-mentions-pour-malentendants.md`](specs/04-mentions-pour-malentendants.md).

**Heavily restricted.** The complete correction engine — per-language patterns,
line breaking, spell checker — belongs to phase 12. Here, only the two requested
patterns:

```
Sound in brackets      \[.*?\]    → empty string
Sound in parentheses   \(.*?\)    → empty string
```

No backreferences: **the choice between PCRE2 and RE2 does not block this
phase**, and can be deferred to phase 12, where it will really arise.

**Preliminary analysis** — `aeidon/`: `pattern.py`, `patternman.py`,
`agents/text.py` (method `remove_hearing_impaired`), and
`data/patterns/Latn.hearing-impaired`.

**What the reading yielded**, noted at issue #88 so as not to redo it:

| Finding | Detail |
| :------ | :----- |
| five patterns, not two | brackets, parentheses, lyrics between `#`, lyrics on a line, speaker name before a colon — the last three belong to phase 12 |
| sections with the same name | `[Hearing Impaired Pattern]` repeats, which no standard INI reader accepts |
| `Flags=DOTALL;MULTILINE;` | the pattern crosses the line break, and it has to: in real files, a mention is often split by one |
| `Replacement=\0` | the `\0` protects the value, empty here; `-\040` gives a dash and a space |
| XML `.conf` | `enabled="false"` for every pattern — nothing is active by default |

**The pattern data is not taken over.** Gaupol is GPL-3 like this project, so
copying would be lawful; it is not useful. The phase's two patterns fit in two
expressions, the specification written in `src/test/data/textes/mentions.cas`
already says much more — backreferences, empty brackets, dialogue dashes,
mentions straddling lines — and the format of pattern files remains a scoping
decision. Taking over data in a format we did not choose would be committing
before having decided.

**The two architecture questions asked here are settled at scoping**, and the
spec develops them.

*Take over the INI format of the patterns, or define our own?* **Neither: no
pattern file in this phase.** The transformation is decided, not configurable;
the files, their `.conf` and activation by name come with the phase 12 engine,
which will choose the format knowing what it must carry.

*How is the engine designed so that phase 12 extends it?* **There is no
engine** — [ADR 0017](adr/0017-analyseur-de-mentions-ecrit-a-la-main.md). The
project's rule is not a substitution: it leaves *exactly one space between what
surrounded the mention*, which is decided at the removal site, whereas a regular
expression pass is global and rewrites text it was not asked to touch. Gaupol
pays for it in seven catch-up passes. A hand-written scan holds the rule
exactly, and phase 12 remains free in its choice of engine.

**The difficult point is settled too.** Removing `[Footsteps]` does not
leave an empty line: a line emptied by the removal disappears, and a fully
emptied text takes its subtitle with it — without an option, where Gaupol offers
`remove_blank`. Emptiness ignores formatting markup, because nine real
subtitles in the corpus are written `<i>[PEOPLE SCREAMING]</i>` and would
otherwise leave an `<i></i>` on screen.

---

## 5 — Interface: table editing

**Scoped.** See [`specs/05-interface-tabulaire.md`](specs/05-interface-tabulaire.md).

The heart of the use: open, edit in the table, save as, undo.

**Restricted to the MVP outline,** and tightened at scoping: a single window, a
single project at a time, number / start / end / duration / text columns,
dialogs for the operations of phases 2 and 4. Deferred: multiple projects in
tabs, translation column, configurable columns, difference highlighting —
and, decided at scoping, **inserting and deleting subtitles** as well as
**persisted configuration**, both in phase 7.

**Preliminary analysis** — `gaupol/`: `view.py`, `page.py`, `application.py`,
`renderers/*.py`,
`dialogs/{open,save,position_shift,position_transform,framerate_convert}.py`.

**What the reading yielded**, noted so as not to redo it:

| Finding | Detail |
| :------ | :----- |
| the table is fully duplicated | `page.py` copies every subtitle into a `Gtk.ListStore` and resynchronises it through eight signal handlers |
| and the price is written in the code | `reload_view_all()` on every open, the model **disconnected** beyond 50 removed rows, an `iterate_main()` after every refresh |
| the number is not data | a cell function computes it from the row index |
| no configuration migration, ever | an unknown key is erased on write, an unreadable value leaves the default, an option at its default is rewritten **commented out**; `general.version` is written and never read back |
| the input rate comes from a preference | `conf.editor.framerate`, 23.976 by default — no file-name heuristic, no reading of the video, in twenty years |
| multi-line editing costs a widget | a `Gtk.TextView` implementing `Gtk.CellEditable`, `Enter` and `Escape` by hand, a workaround for focus loss |

**The architecture questions asked here are settled at scoping**, and the spec
develops them.

*Thin adapter or synchronised model of its own?* **Thin adapter** —
[ADR 0019](adr/0019-table-en-adaptateur-mince.md). The choice is not a matter
of taste: the three scars above are what duplication costs, and they are in the
reference's code.

*How does the interface plug into the core's history without keeping a second
one?* `Session::apply`, `undo` and `redo` **return what they changed**, instead
of `void`. No signal enters the core. No `QUndoStack`. And the grouping question
left open in phase 2 closes without a mechanism: a delegate commits once, so an
edited cell produces one command.

*Typed and persisted configuration?* **Not here.** Phase 7 already carries it,
and nothing persistent is required for the table to work.

*A `ChangeKind` for the frame rate?* **Still no.** Phase 2 had set the
condition — "if the window displays the current rate". It does not display it,
so the enumerator would still have no reader.

*Where does the input rate of a conversion come from?* **From the user, and
without a heuristic.** The dialog asks for it, pre-filled with the project's.
The file-name heuristic is set aside for lack of data: the files in `src/data/`
were renamed by hand, no file with intact names is available, and getting the
rate wrong shifts the whole file without reporting anything. Gaupol does no
better.

**The question did however find its answer during this scoping, and it gave
birth to a phase.** Converting a rate assumes the positions were aligned on a
frame grid; when that is the case, the grid can be measured — eight of the
fifteen files of the private corpus give it unambiguously, against a background
noise of a few percent on the rates they do not have.
The file does not *declare* its rate, but it *betrays* it. This is data and not
a correlation, and it is the subject of
[phase 16](#16--frame-rates-inference-and-correction), scheduled right after
6. Phase 5 does not depend on it: its dialog asks for the rate, and will
receive the measured proposal when it exists.

**The scoping also opened up the core,** which was not planned:

- **the format vocabulary joins the model** — [ADR 0018](adr/0018-vocabulaire-des-formats-dans-le-modele.md).
  `SourceFile` did not keep the file's format, and a window that opens and then
  saves needs it. `format/` keeps only operations; `io/` and `text/` take in what
  does not even know the word "subtitle";
- **diagnostics split** — what a read encountered is located by a line, what a
  document is is located by an index. A line exists only at reading time; an
  index survives editing, and **a table highlights rows**. `inspect` will
  therefore point to a subtitle number where it gave a line number;
- **the disorder reading is settled: `Breaks`**, and `--order-report`
  disappears. The corpus did not decide between them — none of its fifteen
  files is out of order, and the two readings agree trivially on it. The
  decision is made by reasoning, and the spec says so rather than letting one
  believe in a measurement.

**The difficult point is settled too.** Staying smooth over several thousand
rows depends on the model never materialising what is not visible — the thin
adapter ensures it by construction — and on fine-grained rather than global
signals, which `Change` already carries. One reservation remains and it is
written down: **a structural change goes through a model reset**, because Qt
requires bracketing before and `Session` reports only after. Tenable as long as
its only producer is the removal of mentions; to be revisited in phase 7, which
brings insertion and deletion.

[#45](https://github.com/Guyot-Bertrand/sub-edit/issues/45) comes **before** the
table, not after: the model translates a `Change` into `dataChanged(topLeft,
bottomRight)`, so Qt wants ranges and not indices. The compact representation
must expose intervals usable as they are.

---

## 6 — The built-in player

**Scoped.** See [`specs/06-lecteur-integre.md`](specs/06-lecteur-integre.md).

**Re-scoped twice, and the second cancels the first.** It was first the whole
video player; then restricted to a preview through an external player, the
built-in one moving to phase 14 as "the most expensive part of the project";
then given back to the built-in player at scoping.

What the first restriction had missed: **Gaupol has both.**
`gaupol/player.py` is an embedded GStreamer player, distinct from
`aeidon/agents/preview.py`. The roadmap had kept the second and forgotten the
first. The external player disappears; the video is watched in the window,
with the current line drawn on top, and the backend is libmpv — see
[ADR 0020](adr/0020-libmpv-pour-le-lecteur-integre.md).

What follows describes the abandoned scoping, kept because it shows where we
started from.

**Preliminary analysis** — `aeidon/agents/preview.py`, `aeidon/enums.py` (the
commands of the three players), `gaupol/agents/preview.py`.

**Architecture questions — all settled at scoping**, and the spec carries the
answers. What follows stays written as it was before, because the value of
these lines is to show where we started from.

- Detection of the available player, and customisable command.
- Associating a video file with a project: by naming convention, like
  Gaupol (`find_video`), or explicit choice?
- **Read the frame rate from the associated video?** Phase 2's rate conversion
  needs an input rate that the subtitle file does not *declare*. The video
  container, for its part, announces it, and a video is already associated with
  the project here, before the built-in player of phase 14.

  **This is the second source of the same data.**
  [Phase 16](#16--frame-rates-inference-and-correction) infers it from the
  positions themselves, without a video. Two independent measurements make a
  cross-check — and a disagreement between them is information, provided one
  knows how to present it.

  To check at scoping: what this reading costs. The preview launches an
  external player and therefore has no video library in memory; reading
  metadata would require `ffprobe` — one more executable to detect — or a
  dependency. The gain is real, the price remains to be measured.
- **Validate operations against the video's duration.** Same mechanism as the
  previous question, and therefore same price: the duration is container
  metadata, read at the same time as the rate or not at all.

  What it would allow: refusing, or at least reporting, an operation that
  pushes subtitles beyond the end of the film. Today the core can check only one
  bound, zero — a too large negative shift makes a position negative, and that
  is all it can say. The upper bound does not exist for it, for lack of knowing
  where the film stops.

  Shifting is the clearest case. The other operations are concerned **in other
  forms**: a transformation whose second reference point falls after the end, a
  rate conversion that stretches everything beyond it. Each needs its own
  formulation, and none follows from the shift's.

  Noted at the scoping of phase 3, where nothing could be done about it: the CLI
  has no notion of video, and giving it one before the project has one would be
  building the check before the data.
- Temporary file: lifetime, encoding forced to UTF-8.

---

## 16 — Frame rates: inference and correction

**Scoped.** See [`specs/16-frequences-d-image.md`](specs/16-frequences-d-image.md)
and [ADR 0021](adr/0021-analyse-du-document-a-l-ouverture.md).

**In the MVP, scheduled between [6](#6--the-built-in-player) and
[7](#7--finishing-touches-and-first-release).** Its number is the first that
was free when it was added; it identifies it and does not give its rank.

**It goes beyond feature parity, deliberately** — the first one to do so.
Gaupol infers no frame rate: its conversion dialog pre-fills its two lists with
`conf.editor.framerate`, a global preference at 23.976, and has never done
better in twenty years. What follows therefore has no counterpart to read in
`reference/gaupol`.

### Where it comes from

The scoping of phase 5 stumbled on a question the roadmap has carried since the
start: **where does the input rate of a conversion come from?** The file does
not declare it — SubRip has no header, WebVTT's is free text — and a heuristic
drawn from the file name had been set aside as a correlation, not data.

The observation that opens this phase lies elsewhere: **converting a rate
assumes the positions were aligned on a frame grid.** When that is true, every
position is `round(n × 1000 / R)`, and this grid can be measured.

### What the measurement has already yielded

Noted at the scoping of phase 5, then **redone** after the example sets showed
that the first method was the wrong one.

**None of these figures can be reproduced elsewhere.** `src/data/` is ignored
by git — each machine's private corpus, just as `reference/gaupol` is a private
clone. The readings that follow are therefore recorded observations, not
measurements a third party can replay, and **no test of this phase will be able
to read these files.** It will need versioned fixtures; none exist.

#### The method: look for a grid *and its phase*

For each of the eight standard rates, we compute the phase of each start on the
corresponding frame grid, and measure **the concentration of these phases** —
the length of the resultant vector. One hundred percent: a perfect grid,
whatever its phase. Near zero: no structure.

It has **no tolerance parameter**, and it is insensitive to a shift.

**An example set is what imposed it.** A first version looked for a grid of
zero phase, and scored at zero a file that is a perfect 24 frames per second,
shifted by one and a half thousandths of a second. Its one thousand one hundred
and seventeen starts fall into exactly **three** residue classes — what a
24-frame grid written in whole milliseconds produces — and the shift applied by
Gaupol translates all three by the same amount.

#### What Gaupol's operations do to the grid

Ground truth: example sets 004, 005 and 006 of the private corpus are files
produced by Gaupol, the operation applied being recorded alongside.

| Operation | What it does to the grid |
| :-------- | :----------------------- |
| rate conversion, 25 → 24 and 25 → 23.976 | **transposes it** — each output announces its new rate at 100, and the old one falls back to noise |
| shift, by −7.001 s and by +2.999 s | **preserves it**, only the phase changes — 100 before as after, both ways |
| two-point transformation | **destroys it** — 100 before, 3 after |

Conversion and shifting are therefore reversible from the inference's point of
view; transformation is not. It is a useful property: it says what one can hope
to recover from a file and what is lost.

#### On the fifteen files of the private corpus

| Verdict | Files | Rates found |
| :------ | ----: | :---------- |
| clear grid — concentration ≥ 90 | **8** | five at 23.976, one at 24, one at 25, one at 29.97 |
| partial hint — from 50 to 90 | 4 | three at 29.97, one at 23.976 |
| silent — under 50 | 3 | — |

**Failure is loud.** A file written in milliseconds without a grid stays under
a few percent on all eight candidates: it is an unequivocal "I don't know", and
not a wrong answer. That is the property that makes it usable, where a
file-name heuristic would never have offered it.

**Harmonic ambiguity is real and appears from the first file.** A 25 grid is
included in a 50 grid: two files come out at 100 on both. In the standard set,
the only ambiguous pairs are those where one is an integer multiple of the
other — **25 and 50, 29.97 and 59.94, 30 and 60**. No others: 24 and 60 are in a
ratio of two and a half, so only one frame in two coincides.

**Starts are the signal, ends corroborate.** Wherever a grid exists, starts are
at 100 without exception; ends range from 55 to 100. The *cue-in* is set on a
frame, the *cue-out* is often computed by a reading-speed rule.

### Architecture questions

- **What does the inference return?** A ranking of candidates with their
  concentration, or an answer and a confidence? The second is simpler to
  consume, the first hides nothing — and a partial file is neither one nor the
  other.
- **The set of candidates must be closed and small** — the eight standard
  rates. Solving for an arbitrary `R` is a completely different problem, and
  pointless: nobody masters at 26.3 frames per second.
- **The phase is information, not waste.** A file whose positions are on the
  grid up to a constant has been shifted, and this constant can be measured.
  Should it be returned? Corrected?
- **Where does the function live?** It is pure and depends only on positions:
  `core/time/`, or a `core/analysis/` that would also take in a document's
  anomalies.
- **What does "correct" cover?** Three distinct mechanisms, to be sorted out:
  bringing the phase back to zero; refusing or reporting a conversion whose
  input rate contradicts the measurement; recovering the pair of a conversion
  made with the wrong rate, by looking for the rational that puts the file back
  on a standard grid. The third is the most useful and the least certain.
- **Exposed surface.** A command-line subcommand, or one more reading in
  `inspect`? And in the window: the conversion dialog pre-fills its input rate
  **by showing its measurement**, never by applying it silently.
- **Fit with [phase 6](#6--the-built-in-player),** which asks the same question
  from the other end: the associated video *declares* its rate. Two independent
  sources for the same data, hence a free cross-check — and a disagreement one
  must know how to present.

### Difficult points

- ~~**The test files do not exist.**~~ **Resolved.** The two corpora that
  served for everything above are ignored by git, and the richest of the
  versioned fixtures carried six timestamps over fifteen seconds. It was the
  first question of the initialisation ticket, and it is the only tool it
  retained: `src/scripts/subtitle-fixtures.py` generates fifteen files on known
  grids — the eight standard rates, an absurd rate, a shifted grid, an
  insufficient span, the two faces of the partial file, ends computed by a
  reading-speed rule, and a file written in milliseconds without any grid. The
  last two were missing from phase 16 and were written in phase 10, issue #373.
  `src/test/data/grilles/LISEZMOI.md` records what each one is and what each one
  yields.
- **Manual editing leaves the grid.** As soon as a user corrects a position in
  the table, it stops being aligned. A naive detector would report the user's
  work as an anomaly. That is why phase 5 forbids itself from using the grid to
  mark anything.
- **Telling 23.976 from 24 requires span.** They diverge by 3.6 s per hour:
  overwhelming on a two-hour film, invisible on a thirty-second excerpt. The
  inference must return the span it had in view, and not only its
  concentration.
- **Partial files are the interesting case and the hardest.** Neither grid nor
  noise — four of the fifteen in the private corpus, between 50 and 83. Saying
  "two thirds of your starts are on a 29.97 grid" is true and helps nobody;
  saying *which ones* and *where they cluster* helps, and requires deciding what
  counts as a block. The observation made at scoping: isolated deviations look
  like one-off anomalies, clustered deviations like a retimed section or an
  assembled file.

---

## 7 — Finishing touches and first release

**Shipped in `v0.8.0`.** See
[`specs/07-finitions-et-premiere-livraison.md`](specs/07-finitions-et-premiere-livraison.md),
[ADR 0022](adr/0022-configuration-au-noyau-et-tolerance-par-option.md) and
[ADR 0023](adr/0023-deb-et-rpm-pour-la-premiere-livraison.md).

What is missing for a third party to install and use the tool.

Persisted preferences, light and dark theme following the system, complete user
manual for the shipped outline, Linux packaging.

**The theme does not follow the system, and D3 says so.** Qt 6.4 has no
colour scheme API — `QStyleHints::colorScheme` arrived in 6.5 —
so "system" leaves the palette to the platform theme instead of reading it.
That is what Gaupol does under GTK earlier than 4.20, and it is what makes the
other two values testable.

**A deferral from phase 16 awaits its content here:** the window's
`Help ▸ Manual` entry exists and has been disabled since #211. The manual it
will open and the place where it will be installed are decided with packaging,
not before.

**Three deferrals from the scoping of phase 5 land here**, and the first was not
planned:

- **inserting and deleting subtitles from the window.** The commands have
  existed in the core since phase 2 and are exposed nowhere — neither by the
  command line nor by the table. Phase 7 is their first surface, and the first
  moment they will have end-to-end proof;
- **persisted configuration itself**, which phase 5 left whole: it needs nothing
  persistent for its table to work. What it would have wanted to remember
  therefore arrives here — window geometry, column widths. **The default frame
  rate, however, did not come and will not come**: nothing would read it, and a
  preference nobody uses is a box that lies. Investigated then set aside in
  [#267](https://github.com/Guyot-Bertrand/sub-edit/issues/267);
- **a `Session` that announces a structural change before making it**, if
  measurement calls for it. Phase 5 resets the model for any insertion or
  deletion of rows; its only producer there is global and rare, which will no
  longer be true as soon as a menu adds one row at a time.
  [ADR 0019](adr/0019-table-en-adaptateur-mince.md) carries the trigger.

**What reading `gaupol/config.py` yielded**, noted at the scoping of phase 5 so
as not to redo it: **there is no migration, and it is a design.** The file is
read option by option; an unknown key is accepted then erased on write if it is
no longer among the defaults; an unreadable value prints to standard error and
leaves the default in place; any option left at its default value is rewritten
**commented out**, so that a change of default takes effect for whoever never
overrode it. `general.version` is written and never read back: a trace, not a
trigger. The "file format and migration" question below therefore arises with a
possible answer already on the table — per-option tolerance, rather than a
versioned migration.

**Preliminary analysis** — `gaupol/config.py`, `gaupol/style.py`, `data/`,
`PACKAGING.md`, `flatpak/`.

**What the phase left behind, and where each thing lands.** The end-of-phase
review gave a place to four deferrals that had none, and seven axes came out of
its critical look:

**Three of them were investigated and closed right away**, and that is a result
rather than a failure: a written "no" does not get proposed again.

| What was settled | The decision |
| :--------------- | :----------- |
| the trigger of ADR 0019 | **abandoned** — [#264](https://github.com/Guyot-Bertrand/sub-edit/issues/264): D8 is enough, and the missing measurement would change nothing |
| Flatpak and AppImage | **set aside** — [#265](https://github.com/Guyot-Bertrand/sub-edit/issues/265): `.deb` and `.rpm`, and elsewhere one builds from source |
| the default frame rate | **set aside** — [#267](https://github.com/Guyot-Bertrand/sub-edit/issues/267): no reader needed it |

| What remains | Where |
| :----------- | :---- |
| a Fedora container to test the `.rpm` | [#266](https://github.com/Guyot-Bertrand/sub-edit/issues/266) |
| check with the tool that matters — the manual, the man page | [#268](https://github.com/Guyot-Bertrand/sub-edit/issues/268) |
| the `Makefile` forces the full analysis for a comment | [#269](https://github.com/Guyot-Bertrand/sub-edit/issues/269) |
| the load threshold of the measurement journal | [#270](https://github.com/Guyot-Bertrand/sub-edit/issues/270) |
| `check-installation.sh` has tripled | [#271](https://github.com/Guyot-Bertrand/sub-edit/issues/271) |
| `subedit-gui` without a man page | [#272](https://github.com/Guyot-Bertrand/sub-edit/issues/272) |
| the language of comments and test titles | [#273](https://github.com/Guyot-Bertrand/sub-edit/issues/273) |
| two shortcuts Qt does not set under X11 | [#274](https://github.com/Guyot-Bertrand/sub-edit/issues/274) |

**Architecture questions**

- Configuration file format and migration between versions.
- Packaging: Flatpak, `.deb`, AppImage — which one for a first release?
  **Answered: `.deb` and `.rpm`**, the other two set aside — ADR 0023 and #265.
- **`install()` rules in CMake, and `install` / `uninstall` targets.** The
  project has none: the tool runs from the build tree, and
  `docs/manual/subedit-cli/installation.md` says so. Noted at the scoping of
  phase 3 and left here on purpose — a target written before the packaging
  format is settled would prejudge the answer.

  Two consequences to handle at the same time: the manual's examples show
  `$ subedit-cli` as if the tool were on the `PATH`, which will only become
  true here; and `make manual` runs it from `build/dev/bin`, which will remain
  the right choice for generation even once installation is possible.

**At the end of this phase, the MVP is shippable.**

---

# Part two — full coverage

These phases are cross-cutting: each one touches the library, the command line
and the interface. Their order is indicative and will be reviewed with the user
once the MVP is in service — it is real use that must determine it, not a
forecast made now.

## 8 — Encodings and line endings

Automatic encoding detection, full set of encodings, Windows and Mac line
endings, forcing on save.

**Scoped** — [`docs/specs/08-encodages.md`](specs/08-encodages.md), issue #288.

**The open question is settled, and by measurement.** It asked for a C++
equivalent of `charset-normalizer`, among ICU, `uchardet` and
`compact_enc_det`. The three routes were run on the same labelled corpus before
any was chosen — ICU 9/9, `uchardet` 9/9, a prototype with hand-written tables
6/9 — and **what decided was not detection but conversion**: ICU is the only one
of the three that converts, and feature parity calls for Gaupol's ninety-seven
encodings, which are not written by hand.
[ADR 0027](adr/0027-icu-pour-les-encodages.md).

**Two things the scoping found and that this paragraph promised badly:**

- **line endings are already shipped.** `Newline` has carried `Lf`, `CrLf` and
  `Cr` since phase 1, `--line-endings unix|windows|mac` has existed since phase
  3, and a file that was read is rewritten with its own. Half of the title was
  done;
- **the window offers none of this.** `Save As…` chooses a path and a format;
  no encoding, no line ending, no BOM. The command line can do what the window
  does not offer, and this gap enters the scope.

**Shipped**, in six implementation issues — #294 to #299 — and closed by
`v0.9.0`. What was done: the encoding enters the model and the BOM becomes its
variant, reading in a given encoding, detection, writing, `--encoding` and
`--to-encoding` on the command line, and `Save As…` choosing the encoding, the
line ending and the mark. Ten requirements, ten in the registry.

**Four gaps between the spec and what was built**, all recorded in
[the spec](specs/08-encodages.md) rather than fixed in the code — what was built
was right all four times:

| The spec said | What was built |
| :------------ | :------------- |
| D6: "`--encoding` on read **and on write**" | two options, because a single invocation does both |
| "the phase does not ship batch conversion" | `--to-encoding` with `--output-dir` re-encodes a directory |
| D4: "**the chosen encoding** enters the diagnostics" | only if it was guessed and is not UTF-8 |
| "UTF-32: ICU will read it if asked to" | **it will not** — measured: its mark is taken for UTF-16LE's |

**What the review settled.** The bench load threshold, left open by #270, is
settled: **it is neither the threshold nor the delay**, but where and when the
load is read. Two changes, the second found because the first failed on its
first run — `make check-local` comes before `make check`, and `bench.sh` reads
the load **before** building the Release tree instead of after, that is, before
having itself occupied the machine it is asking about.
[ADR 0015](adr/0015-memoire-des-mesures.md).

**What the phase leaves behind, and where each thing lands.** Four deferrals had
a "later" without a referent; six axes come out of the critical look.

| What was deferred | Where |
| :---------------- | :---- |
| a read decodes the file twice | [#314](https://github.com/Guyot-Bertrand/sub-edit/issues/314) |
| `Encoding::name()` invents `UTF-16LE-sig` | [#315](https://github.com/Guyot-Bertrand/sub-edit/issues/315) |
| fourteen encodings in the menu against ninety-seven | [#316](https://github.com/Guyot-Bertrand/sub-edit/issues/316) |
| `file.write-encoding` only serves the document without a file | [#317](https://github.com/Guyot-Bertrand/sub-edit/issues/317) |

| What the critical look brought out | Where |
| :--------------------------------- | :---- |
| an encoding whose name does not fix the byte order writes its mark, and `--no-bom` is disobeyed | [#308](https://github.com/Guyot-Bertrand/sub-edit/issues/308) |
| a failing `Save As…` moves the document anyway | [#309](https://github.com/Guyot-Bertrand/sub-edit/issues/309) |
| detection gets the script wrong on a short text | [#310](https://github.com/Guyot-Bertrand/sub-edit/issues/310) |
| nobody replays the detection score | [#311](https://github.com/Guyot-Bertrand/sub-edit/issues/311) |
| the language of comments rests on a wrong measurement | [#312](https://github.com/Guyot-Bertrand/sub-edit/issues/312) — **settled**: C++ in English, a ratchet holds it, the translation is [#325](https://github.com/Guyot-Bertrand/sub-edit/issues/325) |
| the window states the encoding only when it was guessed | [#313](https://github.com/Guyot-Bertrand/sub-edit/issues/313) |
| writing UTF-8 goes through a conversion that does nothing — a third more, invisible for two versions | [#318](https://github.com/Guyot-Bertrand/sub-edit/issues/318) |

**The heaviest is #312, and it corrects an issue from the previous
review.** #273 had reversed the comment language rule by concluding that "the
comments of this repository are French everywhere, without exception". Its
measurement looked for files carrying *at least one* accented character, so it
could only return "there is French everywhere" — that is the flaw of #268,
checking with the tool that does not matter, committed one issue after being
recorded.

**Counted line by line, the boundary is the file's**: 6,460 English comment
lines against 1,161 in the C++, and the exact reverse elsewhere — 1,971 French
against 6 in the scripts, 386 against 4 in the build system. The share of French
in C++ comments, by week of writing, gives the drift a clear date: 0.0 %,
0.3 %, 2.7 %, then 30.2 % and 32.8 % — three weeks of English, then the
interface phases.

**C++ is therefore commented in English**, a ratchet holds it
(`check-comment-language.py`, called by the gate), and the 1,161 remaining lines
are issue [#325](https://github.com/Guyot-Bertrand/sub-edit/issues/325).

> **Held, reviewed at the end of phase 12.** #325 was closed on 2026-09-06 and the ratchet has not moved since: the
> C++ no longer carries **any** French comment line. The 1,161 lines above are those of the day of the
> review, not something left to translate.

## 9 — Additional formats and rich markup

SubViewer 2, Sub Station Alpha, Advanced SSA — the three formats cited as
secondary — then MicroDVD, MPL2, TMPlayer and LRC.

**Scoped, and its spec is in [`specs/09-formats.md`](specs/09-formats.md)** —
issue #337. Three ADRs come out of it: the inferred ends for the two formats
that carry none ([0029](adr/0029-fins-deduites-et-annoncees.md)), the frame rate
kept by the document for MicroDVD
([0030](adr/0030-ce-qu-un-document-retient-de-son-fichier.md)), and the markup
pivot that ADR 0009 had promised since phase 1
([0031](adr/0031-pivot-de-balises-a-la-conversion.md)).

**The deferral from the scoping of phase 5 — the frame editing mode — goes back
to phase 14**, better framed than it arrived. Phase 5 had deferred it for lack of
a frame-based format; one arrives here. What remains true is that MicroDVD
carries frame numbers **and no frame rate**: the toggle would show the file's
figures for a MicroDVD, and a conversion against a chosen rate for the other
eight.

**Difficult point** — ASS is not a timing format but a structured format with
sections, named styles and typed events. Conversion to SubRip is
**structurally lossy**: which degradation policy, and is the user told? This is
where the soundness of the markup model designed in phase 1 is tested.

## 10 — Additional operations

**Scoped.** See [`specs/10-operations.md`](specs/10-operations.md), which carries
the eight decisions and the split — and which corrects two claims of this
scoping: Gaupol's clipboard carries only texts, and its menu offers only one
style toggle, italics.

Duration adjustment, case, italics, dialogue dashes, merge, split, find and
replace, clipboard.

**A deferral from the scoping of phase 5 landed here: editing the duration in
the table.** The `Duration` column was displayed without being editable, for lack
of a `SetDurationCommand` in the core. It was born in #381, and **it moves the
end** — decision D3 of the spec.

**Two deferrals from phase 16 were placed here, and only one was kept.**

- **Two missing grid fixtures**, written by issue #373: ends computed by a
  reading-speed rule rather than set on a frame, and a file written in
  milliseconds **without any grid** — `grille-absurde.srt` being regular, the
  pure-noise case had none.
- **Recovering the pair of a conversion made with the wrong rate** was
  **deferred to phase 14**, by that phase's scoping — issue
  [#386](https://github.com/Guyot-Bertrand/sub-edit/issues/386). Nothing phase
  10 brings makes this mechanism more reliable.

**Difficult points**

- **Duration adjustment**: simultaneous constraints of minimum and maximum
  duration, minimum gap between subtitles and reading speed in characters per
  second. They are **potentially contradictory**; the resolution order must be
  specified.
- **Searching marked-up text**: searching in the visible text while replacing
  in the source text, without breaking markup that overlaps the match.

## 11 — Translation and multiple projects

Second document alongside the main one, alignment of the translation file by
number or by position, tabs, grouped save and close, splitting a project,
appending one file after another.

**Scoped.** See [`specs/11-traduction.md`](specs/11-traduction.md), which carries
nine decisions, two ADRs ([0032](adr/0032-un-document-un-fichier.md), accepted,
and [0033](adr/0033-un-projet-est-une-page.md), **accepted**) and the split into
**three slices** — translation, appending a file, multiple projects. **The
reservation below is lifted**: the gate
[#435](https://github.com/Guyot-Bertrand/sub-edit/issues/435) decided in favour
of multiple projects on 2026-09-22, and the third slice is being shipped.

**A deferral from phase 6 lands here: the player for a translation document.**
The line drawn on the picture comes from the main document, and `Subtitle` has
carried both texts for a single pair of positions since phase 1 — so it will be
necessary to say which of the two is displayed, and whether the choice is a
setting or follows the active tab. The end-of-phase 6 review found that this
deferral landed nowhere; it lands here.

**A deferral from phase 10 lands here: searching beyond the current document.**
Gaupol searches in "all open projects" and in the translation text; phase 10
reduced the scope to the usual target — the selection or the document — and to
the main text only, for lack of a second project and of a translation column on
screen. It will be necessary to say whether the scope widens with tabs, and
whether the searched field follows the visible column.

**Reservation lifted by #435, on 2026-09-22.** The phase 1 data model already
accommodated multiple projects without committing to them; the decision commits
to them.

**Shipped, reviewed by [#441](https://github.com/Guyot-Bertrand/sub-edit/issues/441).** The three slices,
and two more issues: hideable and reorderable columns
([#442](https://github.com/Guyot-Bertrand/sub-edit/issues/442)), which no phase carried, and
drag and drop ([#453](https://github.com/Guyot-Bertrand/sub-edit/issues/453)), which #437 had left.
**The two deferrals above have landed**: the line follows the current column, without a setting, and
search covers, at the user's choice, all open projects, in the text of the current column — D8 of the
spec. Those the phase emits each have their recipient: the command line for translation in
phase 13, detection of a shifted translation in phase 14. **The review opened four issues, #460
to #463**, which keep the phase open until closing in 0.12.0.

**Closed in 0.12.0, on 2026-09-26.** After the review, two more waves, both closed before the
closing:

- **feedback from use on a real desktop** — [#468](https://github.com/Guyot-Bertrand/sub-edit/issues/468)
  to [#474](https://github.com/Guyot-Bertrand/sub-edit/issues/474) and
  [#477](https://github.com/Guyot-Bertrand/sub-edit/issues/477): a second film that would not load,
  a zero-pixel picture, X11 errors on quitting, the playback position per tab, the tabs' close button and
  "+", the toolbar, the blank project replaced on opening;
- **slimming down `MainWindow`**, requested at the merge of #460 and analysed before closing:
  [#483](https://github.com/Guyot-Bertrand/sub-edit/issues/483) to
  [#486](https://github.com/Guyot-Bertrand/sub-edit/issues/486) move out the actions and menus
  (`WindowActions`), the video (`VideoPane`), the status bar (`StatusLine`) and the `Tools` operations
  (`ProjectOperations`, [ADR 0035](adr/0035-les-operations-sortent-de-la-fenetre.md)). `main_window.cpp`
  goes from 2,531 to 1,506 lines, without a single window test being rewritten.

**The phase's bench** remains thin: **five readings for thirty-two versions** — 0.11.1, 0.11.7,
0.11.21, 0.11.25 and 0.11.30 —, the machine having found the load threshold exceeded every other time.
The 0.12.0 reading is due at closing — issue [#270](https://github.com/Guyot-Bertrand/sub-edit/issues/270).

## 12 — Complete correction engine

Declarative patterns by script, language and country — common errors classed
Human and OCR, capitalisation, remaining hearing-impaired mentions —
line breaking and spell checker, with the word joining and splitting it
enables — deferral from phase 10.

**Scoped.** See [`specs/12-correction.md`](specs/12-correction.md), which carries
ten decisions, two ADRs — [0036](adr/0036-icu-pour-les-motifs-de-correction.md),
**ICU** to apply the patterns, and [0037](adr/0037-lire-les-motifs-de-gaupol-tels-quels.md),
Gaupol's files **read as they are** — and the split into **four slices**: the
engine, line breaking, the assistant, the spell checker ([#498](https://github.com/Guyot-Bertrand/sub-edit/issues/498) to [#509](https://github.com/Guyot-Bertrand/sub-edit/issues/509)).
**No gate**: no reservation is written about the need, and the spell checker,
the only piece that adds a dependency, comes last.

**The engine question, below, is settled, and not as it was asked.**
The candidate retained is neither PCRE2 nor RE2 but ICU, already in the core and
already the search engine. Measurement put it at 0.13 s per film against 0.05 s
for PCRE2 compiled on the fly — a difference nobody will see —, and it found
something more important than time: **neither of them reads `\w` like Python**
as soon as a letter is decomposed, PCRE2 since its 10.43. Rewriting `\w` is due
whatever the engine, and the compatibility argument falls with it. The spell
checker is **Enchant**, and not hunspell: a Gaupol user's personal word list is
the same.

**Phase 10 left a markup-aware parser here** —
`core/text/markup_parser` — which case, dashes, search and duration adjustment
borrow. The correction patterns will apply to the source text and will meet
there the flaw that ADR 0009 describes; the parser is the piece made to avoid
it, and the phase 4 mention removal does not go through it yet.

**Architecture questions**

- **Regular expression engine.** This is where the choice arises. The
  patterns are written in Python syntax and make heavy use of
  **backreferences** (`\1 \2`), which RE2 does not handle; PCRE2 is compatible
  but can blow up in time on some patterns. Measure before deciding.

  **Phase 4 did without one, and its [ADR 0017](adr/0017-analyseur-de-mentions-ecrit-a-la-main.md)
  is to be read before deciding here.** It explains why an engine would not
  have been enough for its two patterns — the stitching rule is local to the
  removal site, a substitution is global — and sets its own reopening trigger:
  "the third pattern requested, whatever it is". That third pattern is
  requested by this phase.
- **Measuring text length.** Major point of attention: Gaupol measures lines in
  *ems*, and does so by asking **a GTK widget to measure the rendered text**
  (`gaupol/ruler.py`). The line-breaking algorithm therefore depends on the
  toolkit. Here, it must be an injected abstraction: a trivial per-character
  implementation for the CLI and tests, a Qt implementation for the interface.
- Spell checker: hunspell, nuspell, or a system service. **Settled by the spec:
  Enchant**, whose personal word list is Gaupol's — D6.

**Difficult point** — line breaking is a variant of Knuth–Plass with boxes,
penalties and demerits, where the penalties come from the per-language
`line-break` patterns. Expensive, subjective, and central to perceived quality.
Applying dozens of patterns to thousands of subtitles is **the** reference
benchmark of the project.

**Shipped, reviewed by [#523](https://github.com/Guyot-Bertrand/sub-edit/issues/523).** Twelve issues, #498 to
#509, in fifteen versions: Gaupol's patterns read as they are and applied by ICU, common errors,
capitalisation, the remaining mentions, line breaking in characters and in *ems*, the
`Correct Texts…` assistant and its confirmation, word joining and splitting, and `Check Spelling…`. **The four
deferrals it received have landed**: the three mention patterns and the choice of engine, from phase 4;
PCRE2 or RE2 and hunspell, from phase 0; joining, splitting and common errors, from phase 10;
the markup-aware parser, which phase 10 had written for that purpose. **Twelve requirements, all
`implemented` and cited** — fourteen at closing, with `GUI-SPELL-04` and `GUI-EDIT-04`; **twenty-one differences from Gaupol**, ten of which the spec's table did not
carry. Those the phase emits each have their recipient: the command line for correction in
phase 13, translation of pattern names in phase 15. **Two deferrals were left without a phase**; the review submitted them for decision, and they are shipped within the phase: spell checking as you type by [#525](https://github.com/Guyot-Bertrand/sub-edit/issues/525) (`GUI-SPELL-04`, thirteenth requirement), the length of lines shown in the cells by [#526](https://github.com/Guyot-Bertrand/sub-edit/issues/526). The spec keeps count of them, at the end.

**Closed in 0.13.0, on 2026-10-01.** The review opened seven issues, all closed before closing:

- **the two Gaupol features it had deferred** — [#525](https://github.com/Guyot-Bertrand/sub-edit/issues/525),
  spell checking as you type in the cell editor, and
  [#526](https://github.com/Guyot-Bertrand/sub-edit/issues/526), the length of each line in the cells and
  in a margin of the editor (three `editor.*` settings, in `Preferences…`);
- **the flaws it had seen in the code** — [#527](https://github.com/Guyot-Bertrand/sub-edit/issues/527),
  the length cache finally placed in front of the assistant;
  [#528](https://github.com/Guyot-Bertrand/sub-edit/issues/528), two checkboxes in the pattern list that did
  nothing; [#529](https://github.com/Guyot-Bertrand/sub-edit/issues/529), a pattern name with a comma that lost
  all the overrides; [#530](https://github.com/Guyot-Bertrand/sub-edit/issues/530), two of the four robustness
  findings of #509, the other two set aside with their reason;
- **a GitHub flaw, not the repository's** — [#531](https://github.com/Guyot-Bertrand/sub-edit/issues/531):
  pull requests of 30 September did not link their `Closes #N`, then it stopped without any difference in
  form being found. Closed without an added check.

**The phase's bench**, like that of the previous one, remained thin: the machine found the load threshold
exceeded almost every time. The 0.13.0 reading, taken in calm conditions at closing, is added to the journal.

## 13 — Complete CLI

Subcommands meant for real use: conversion, shifting, transformation,
alignment on a grid, duration adjustment, correction, inspection.
Batch processing, human-readable output and script-consumable output.

Gaupol has no equivalent: it is a new design, and a real functional gain.

**A deferral from phase 10 lands here: cross-checking duration adjustment
against what the measurement script predicted for it.**
[`measure-duration-constraints.py`](../src/scripts/measure-duration-constraints.py)
(issue [#371](https://github.com/Guyot-Bertrand/sub-edit/issues/371)) lists,
before any adjustment, the subtitles for which no end satisfies two of the
duration constraints; `adjustDurations`, for its part, declares what it
sacrificed. The second mode announced by #371 — apply the adjustment, then
count — was not written: a Python script cannot call the core, the command line
does not expose adjustment before this phase, and a tool linked to the core
would serve only this one cross-check. **Once adjustment is exposed**, the two
counts will be taken through the command line on the same files and the same
settings; if they differ, one of the two is wrong. The script counts, per pair
of constraints, the subtitles without a satisfying end; `adjustDurations`
counts, per constraint, those where it remains violated: saying which counts
compare with which is part of the work. Issue
[#407](https://github.com/Guyot-Bertrand/sub-edit/issues/407).

**This deferral is kept** ([#560](https://github.com/Guyot-Bertrand/sub-edit/issues/560)). The
counts compare as spec D7 now says: `sacrificed.minimum` with "minimum against
gap", `sacrificed.speed` with **the union** of "speed against gap" and "speed against
maximum", `sacrificed.gap` with negative room. The review found **two errors on the
script's side, none on the core's side**: it compared the reading need in floating point where the core
rounds it to the millisecond, and negative room had no column (the scoping wrongly assumed
it was "off topic"). The proof is a versioned fixture (`src/test/data/durees/`,
nine files) whose counts are written by hand, and against which the core (end-to-end case)
and the script (`--check-fixtures`, in `make check-local`) are checked. **Observation on the
private corpus**, replayed with `--crosscheck`: **the two counts agree on the seventy-one
SubRip and WebVTT files, under four different settings** (Gaupol's defaults with a maximum
of 6 s, then three variants of the gap, the minimum, the speed and the maximum). Nothing diverged,
so open point 9 of the spec — knowing *which* subtitles are sacrificed — stays closed.

**A deferral from phase 11 lands here: translation on the command line.**
`-t/--translation-file` exists in Gaupol, and the phase 4 spec had deferred
"cleaning the translation document from the command line" to phase 11.
The scoping of phase 11 ([`specs/11-traduction.md`](specs/11-traduction.md), D9)
hands them to this one: **nothing phase 11 writes is specific to the
window** — opening a translation, its alignment and the report sentence live in
the core and in `core/wording/` —, and all that remains here is a grammar to
write.

**A deferral from phase 12 lands here: correction on the command line.** The computation of changes, the
patterns, the spell checker and the report sentences live in the core ([`specs/12-correction.md`](specs/12-correction.md),
D8); all that remains is a grammar — `correct`, with its tasks, its language and its line-breaking settings **in
characters**, measurement in *ems* being the window's. Word joining and splitting join it, the
spell checker being in the core and the dependency on Enchant already pulled in by `subedit-cli`. **Confirmation has no
equivalent**: the spec leaves it to this phase to say whether a mode that writes the proposed changes without
applying them replaces it.

**Scoped by [#542](https://github.com/Guyot-Bertrand/sub-edit/issues/542)** — the spec is
[`specs/13-cli.md`](specs/13-cli.md), and three ADRs carry its decisions that are expensive to undo:
[0038](adr/0038-sortie-json-lines-versionnee.md) (script-readable output),
[0039](adr/0039-le-lot-collisions-dossiers-et-arborescence.md) (batches) and
[0040](adr/0040-correct-ecrit-directement-dry-run-propose.md) (write directly, propose with
`--dry-run`). What the scoping settled, and the spec develops:

- **Script-readable output: `--format json`**, opt-in, a global option. On standard output,
  **one JSON object per input file** (JSON Lines), failure included, versioned by a `schema` field; text
  remains the default, narration stays on standard error. It is proven by **versioned expected
  outputs**, written by hand, which `MatchesFile` (#543) compares byte for byte. No floating-point
  number: whole milliseconds, frame rates as strings.
- **A safe batch.** Two inputs with the same destination, or a destination that is an input, are **refused
  before anything is written** (code `1`); the output directory is **created**; an existing destination is
  **overwritten** — and the manual says so; **`--recursive`** takes directories and **preserves the tree**
  under `--output-dir`. The "cannot be read" wording of a write that fails is corrected.
- **`correct` writes directly; `--dry-run` proposes.** It writes nothing and prints the text before and after,
  per subtitle; it is accepted by **every** subcommand that writes. It is the "mode that proposes without
  applying" that phase 12 left here: it **replaces** the confirmation page.
- **Seven more subcommands**: `adjust`, `correct` (named tasks, line breaking **in characters**, word joining
  and splitting), `replace`, `case`, `italics`, `dialogue-dashes`, `sort`; plus `--list-encodings` and
  translation as options (`-t`, `--document`, `--align-method`). **Three others — `append`,
  `split-file`, `pair` — are behind a gate**, as multiple projects were in phase 11: the use is
  real, no caller is known. **Set aside, with their reason**: merging and splitting a
  subtitle, the clipboard, editing by hand, interactive spell checking.
- **An invocation is self-sufficient**: no setting is read, nor the system language; the language, the
  pattern code and the line length **are named**. Only the shipped patterns, the user's
  and Enchant's dictionary are read.
- **The deferral of #407** is made precise: `sacrificed.minimum` compares with "minimum against gap",
  `sacrificed.speed` with **the union** of "speed against gap" and "speed against maximum",
  `sacrificed.gap` with negative room; `adjust --format json --dry-run` exposes the counts and the
  constraints used so that the comparison can be made, on versioned fixtures first.

**Twenty-one issues, in five slices** — the foundations (batch, format, `--dry-run`, `--range`), the
subcommands for which the core is ready, translation, correction, the gate —, the end-of-phase
review included. **A finding of the analysis**: resolving the locations of patterns, which ADR 0037
said was outside `gui`, stayed there and calls Qt; correction on the command line starts by
rewriting it without Qt.

**Shipped, reviewed by [#573](https://github.com/Guyot-Bertrand/sub-edit/issues/573).** Twenty issues, #553 to #572, in
twenty-eight versions, from 0.13.1 to 0.13.28: **seventeen subcommands** — the original seven, plus `adjust`,
`replace`, `case`, `italics`, `dialogue-dashes`, `sort`, `correct` and, behind the gate, `append`, `split-file`
and `pair` —, the safe batch and `--recursive`, `--format json`, `--dry-run` on every subcommand that writes,
`--range`, `-t`, `--document`, `--align-method` and `--list-encodings`. **Sixty-six `CLI-*` requirements, all
`implemented` and cited by a test**, none `abandoned`; three ADRs ([0038](adr/0038-sortie-json-lines-versionnee.md),
[0039](adr/0039-le-lot-collisions-dossiers-et-arborescence.md),
[0040](adr/0040-correct-ecrit-directement-dry-run-propose.md)); **eleven differences from Gaupol**, two more than at
scoping (`--list-encodings`, and the window's three gestures turned into subcommands). **The D12 gate opened
on 5 October 2026, for all three**: the user wanted them, one issue each, and each with the grammar its
arity calls for — N inputs one output, one input two outputs, two inputs one output. **The eleven open points of the
spec are all settled**, three of them at this review (pairing within a batch, `--range` on the original
seven, `--include`), each with its trigger. **The deferral of #407 is kept** (#560): the two counts
agree.

**The review found no fundamental flaw, and corrected the manual.** The table of what the command line
does not do said "no" on five rows of things it does; `invocation.md` gave three values for the number of
subcommands that write and stated neither the `counts` of `append`, `split-file` and `pair`, nor that these three
write **one object per run** and not per input; six pages had no JSON section; `lots.md` and the page
on multiple files said "all". **The critical look at the code** opened three issues, in the
milestone, nothing having been fixed in the review:
[#598](https://github.com/Guyot-Bertrand/sub-edit/issues/598), a single preparation of options for seventeen
subcommands (validation is written there seventeen times, opening a file five times, the "destination =
input" guard in two wordings);
[#599](https://github.com/Guyot-Bertrand/sub-edit/issues/599), decision logic left in `src/exe/cli`
(`pairingOf`, the preparation of `correct`, the list of formats of `convert` that duplicates the core);
[#600](https://github.com/Guyot-Bertrand/sub-edit/issues/600), tests that promise more than they prove
(`--dry-run` is tested with a destination to create for only six subcommands out of seventeen). **`application.cpp`
stayed thin.** **Sentence parity holds**: no sentence of `core/wording/` is copied into `cli/`, and the sentence
refusing a split, which the window wrote inline, moved there (#571).

**The phase's bench** remained thin, like that of the two previous ones: readings at 0.13.0, 0.13.10, 0.13.13,
0.13.14, 0.13.21, 0.13.25 and 0.13.27, and **none for 0.13.28 nor for the review** — the machine found the load
threshold exceeded. **What is missing**: a bench specific to `append`, `split-file` and `pair`, which have none — they rest
on core operations already measured, and `make bench` measures only the batch (walking two hundred files,
shifting a file of four thousand subtitles). Replaying `make bench` in calm conditions remains due.

**Closed in 0.14.0, on 2026-10-06.** The three issues of the review were shipped, one more along with them, and the
milestone — thirty-two issues, from #541 to #605 — closes with nothing open:

- **#598** (0.13.30): a single preparation of options, `prepare` and `prepareWriting`; opening a file,
  narrating a rewritten file and the "destination = input" guard are no longer written five, four and two
  times; **a single wording** for the destination that is an input, which no longer points to an option that `append`
  and `split-file` do not have;
- **#599** (0.13.31): what `src/exe/cli` decided alone — the shared options and the `-t` rule, the preparation
  of `correct`, the shape of a conversion, the amount of a shift, the list of formats of `--to`, derived from the core
  — lives in `subedit/cli`, where unit tests reach it;
- **#600** (0.13.32): `--dry-run` tested for the sixteen subcommands that write and not six, and the tests that
  promised "nothing is written" observe it;
- **#605** (0.13.33), **opened by usage feedback** and not by the review: the read diagnostics
  panel, which took a row of the window for rare information, becomes a status bar button and
  a floating list. While removing the row, a test revealed an old flaw — the table's share, read and set
  by rounding down, dropped by one point at each re-read —, fixed and proven on eight heights.

**The phase's bench** has no reading for 0.14.0: the machine found the load threshold exceeded, and the
closing measurement was not retaken. The last reading added is that of 0.13.33. **What remains due** is
stated above: a bench specific to `append`, `split-file` and `pair`, and a `make bench` in calm conditions.

## 14 — Fine timing

**Reduced, but not emptied.** The player itself moved to phase 6, which took it
back at scoping: associate a film, open it in the window, play, stop,
seek, draw the line on the picture. What remains here is what phase 6 does not
do — and that goes beyond timing proper, because **phase 6 ships only play and
stop as playback control.**

The outline was recounted against Gaupol's **Video** menu and its player bar,
item by item, after a review showed that half of it was missing from the
scoping. Three families, and nothing else.

**Playback control.** What a player bar carries, and what phase 6 leaves to
whoever has only `Ctrl+P`:

| Command | What it does | In Gaupol |
| :------ | :----------- | :-------- |
| position | a bar one reads and drags | `Gtk.Scale` of the player bar |
| seek forward, seek backward | by an adjustable step | `Seek Forward`, `Seek Backward` |
| previous, next subtitle | go to its start | `Seek Previous`, `Seek Next` |
| start, end of the selection | go there, slightly ahead | `Seek Selection Start`, `Seek Selection End` |
| play the selection | and stop at its end | `Play Selection` |
| volume | up, down, a setting | `Volume Up`, `Volume Down`, `Gtk.VolumeButton` |

**Timing itself**, which is the heart of *timing* work:

| Command | What it does |
| :------ | :----------- |
| start, end from the video position | set a mark on what one is watching |
| insert a subtitle at the video position | likewise, for a line that does not exist yet |
| step forward, backward frame by frame | what ADR 0020 chose libmpv for |
| shift start or end by small increments | fine adjustment, from the keyboard |

**What is displayed**: timecode overlay, audio track selection, and
**the line drawn without its raw markup** — deferral from the end-of-phase 10
review, issue [#408](https://github.com/Guyot-Bertrand/sub-edit/issues/408).
The player's manual promised it "with the rich formats", a phase already
past; the parser and the pivot now exist, and nothing is missing from the core.

**What phase 6 has already shipped, and which is therefore not here**: associate a
film, open it in the window, play and stop, the line drawn from
the model, the current row that follows playback, exact seeking to the
selected subtitle.

**The backend is decided** — libmpv, see
[ADR 0020](adr/0020-libmpv-pour-le-lecteur-integre.md). And it is this phase
that decided it: both candidates could serve phase 6, only libmpv can
step one frame.

**A deferral from the scoping of phase 9 lands here: the frame editing
mode.** It comes from phase 5, which had deferred it for lack of a frame-based
format, and phase 9 passes it on having framed it better — MicroDVD carries frame
numbers and no frame rate, so the toggle would show the file's figures
for it, and a conversion against a chosen rate for the other eight.
It is a question of fine editing, and this phase already carries "step forward,
backward frame by frame".

**A deferral from phase 10, coming from phase 16, lands here: recovering the
pair of a conversion made at the wrong rate** — issue
[#386](https://github.com/Guyot-Bertrand/sub-edit/issues/386). When the inferred
grid is none of the eight, it is sometimes because a conversion was made with
a wrong input rate, and the rational that would put the file back on a
standard grid can be searched for. The phase 16 spec called it "the most useful
and the least certain" of the three correction mechanisms; neither how far to
search nor how to state its confidence has an answer yet.

**A deferral from phase 11 lands here: recognising a shifted translation.**
A translation timed two seconds too late aligns with the wrong subtitles
without anything distinguishing it from a correct attachment — the
`positions-decalees` case of `src/test/data/paires/` shows it. Gaupol does not
detect it either. Phase 11 makes the shift **visible**, through the opening
count (three lines out of four give birth to a subtitle there), not
**correctable**: it is a synchronisation question, and it arises here.

**Clarified by the user, on 2026-09-30: the table follows playback, but does not
centre.** The current subtitle is highlighted during playback, and
the table does scroll to it — a first observation said the opposite,
corrected since. What remains: **the table does not centre on the current
subtitle**, it only brings it back into the visible area. Phase 14 centres it.
The point to settle at scoping is the user's gesture — scrolling by
hand during playback must not be undone at every frame, and one must say
when following resumes.

**Difficult point** — **positioning accuracy.** Timing a subtitle requires
a frame-exact `seek`; most backends guarantee it only to the nearest
keyframe. It is the most demanding feature of the whole
project, and the one that decides the tool's quality for timing work.
Phase 6 already lays down exact seeking, which makes it testable before
arriving here.

**Scoped by [#609](https://github.com/Guyot-Bertrand/sub-edit/issues/609)** — the spec is
[`specs/14-calage-fin.md`](specs/14-calage-fin.md), and [ADR 0041](adr/0041-afficher-la-video-par-le-rendu-logiciel.md)
carries the decision that is expensive to undo. What the scoping settled, and the spec develops:

- **The picture is rendered by libmpv's software API, in the Qt window** — decided by the user on 2026-10-06,
  and what closes [#190](https://github.com/Guyot-Bertrand/sub-edit/issues/190). Measured: 2 to 3.4 ms per frame for
  1080p H.264, without a dropped frame; Wayland and X11 alike, **the picture becomes capturable and readable without
  a screen**. Zero-copy hardware decoding no longer exists, and the ADR states its trigger.
- **The accuracy of `seek` is measured, and stated**: mpv places the seek on **the nearest frame** to a position, and
  `position()` is **the start of the displayed frame** — a mark set from the video on a frame returns to it. Millisecond
  granularity is not at issue: the 24 frames tried at 23.976 start exactly.
- **The player interface gains five commands** (frame step, volume, tracks, `playUntil`), **no notification**: following
  remains a 100 ms probe, which tests drive by hand.
- **Timing replays Gaupol's five mark gestures on commands that exist**, and adds what Gaupol does not
  have: frame by frame and shifting an edge by one frame, whose duration comes from the video, then from the document, then
  from the grid — and with none, the gesture refuses.
- **The table centres and pauses on a manual scroll**; it resumes on a player gesture, **without a timer**.
- **Two detections, proposed and never applied**, both within the phase — decided by the user: a
  translation shifted by a constant amount, **judged by what the attachment would say afterwards**, and the pair of a conversion
  at the wrong rate, **within the closed set of fifty-six pairs** and judged by the phase 16 inference.
  They stay silent when two candidates are equal, when the gap is not constant, or when the file is clean.

**Fourteen issues** — three for tooling (#610 to #612), ten for implementation (#613 to #622), the end-of-phase review
(#623) —, with #386 and #408, which are older. **Twenty-eight `GUI-*` requirements**, all `planned`. **A finding of
the analysis**: `VideoPlayer` says nowhere that `seek` returns the nearest frame; the header said "the frame at its
own start".

**Shipped, reviewed by [#623](https://github.com/Guyot-Bertrand/sub-edit/issues/623).** Fifteen issues, #610 to #622 with
#386 and #408, in eighteen versions, from 0.14.1 to 0.14.18: libmpv software rendering in the window
([ADR 0041](adr/0041-afficher-la-video-par-le-rendu-logiciel.md), which closes #190), the player interface and its playback
bar, the audio track, the marks, frame by frame and shifting an edge by one frame, the table that follows
playback, the line without raw markup, positions in frames — in the table and on the bar —, and the two
detections. **Twenty-eight `GUI-*` requirements, all `implemented` and cited by a test**, none `abandoned`; **the
spec's nine open points are all settled**, four of them at this review (resuming following, drift,
4K and hardware decoding); **ten differences from Gaupol**, two more than at scoping, born of user requests
(a `Follow` button, positions in frames on the bar).

**Two things the spec had not foreseen.** The conversion that puts a file back on a grid (#622, #386) almost
never has a unique repair: out of the 320 combinations of a grid and a wrong ratio, only 28 do, and
a tie — stated with its two pairs, without proposing anything — is the ordinary case. And **the bench did not respect the
gate's two-core limit**: `gate/bench.sh` launched `subedit_bench` bare, and libmpv's threads exceeded it.
`limit-cores.sh` now bounds it, and `check-parallelism.sh` refuses a bare launch, with two more proofs. **The
bench readings are therefore no longer quite comparable with those before 0.14.19**: the player entries are 1.2 to
1.9 times slower there. **The probable cause is affinity**, but it is not verified: the same measurement without a limit has not
been replayed, and calling it a code regression or not would be guessing.

**The review corrected the manual and opened four issues.** `lecteur.md` still said that frame by frame and
marks do not exist; the page order placed timing before the player it assumes; the "current state" ignored
the playback bar, the audio track and the two detections. **The critical look at the code**, nothing being fixed in
the review: [#644](https://github.com/Guyot-Bertrand/sub-edit/issues/644), position rules that live in
`video_pane.cpp` and `mpv_player.cpp` and cannot be tested without a window, including a second floating-point frame
arithmetic next to the rational one; [#645](https://github.com/Guyot-Bertrand/sub-edit/issues/645), the sequence "resume
following, move, follow" written five times, and the "every gesture" test that omits `nudge` and `seekToSelection`;
[#646](https://github.com/Guyot-Bertrand/sub-edit/issues/646), clock-based waits and five writings of the same
wait. **The player on a real film** is measured, under the two-core limit: 1080p H.264 keeps real time at
2.5 times, **a heavy 4K HEVC does not** (0.62 times in 8 bits, 0.92 in 10 bits) and its one-frame step costs 0.4 to
5 s; the machine's hardware decoding brings it to 2.1 times, hence
[#647](https://github.com/Guyot-Bertrand/sub-edit/issues/647), **shipped**: `hwdec=auto-copy` and a setting to turn it off,
for a gain of 1.1 to 3.2 times through libmpv (ADR 0041, addendum). HDR, AV1 and continuous playback with dropped-frame
counting remain unmeasured.

**The bench** has a reading for 0.14.19 (load 1.35), the first since 0.14.7; three new measurements — centring a row,
the conversion search (about 2.4 ms for 4000 subtitles) and translating a marked-up line — will enter
at the next calm reading. The `seeking and stepping on a real film` bench abstains without `SUBEDIT_BENCH_REAL_FILM`.

**Closed in 0.15.0, on 2026-10-09.** The four issues of the review were shipped, and the milestone — twenty-five
issues, from #190 to #647 — closes with nothing open:

- **#644** (0.14.20): the player's position rules — neighbour, seek, lead-in, frame step, nearest
  rate — live in `core/video/seeking`, in rational arithmetic, and are tested without a window;
- **#645** (0.14.21): a single path, `goTo`, for the player's gestures, and a test of all gestures that no longer omits
  any;
- **#646** (0.14.22): the player tests no longer wait on a clock — `waitUntil` and the projector are shared;
- **#647** (0.14.23): `hwdec=auto-copy` and its setting; from 1.1 to 3.2 times faster through libmpv depending on the film,
  less than the 2.1 times of ffmpeg alone, and a heavy 4K far from the keyframe remains slow.

**The phase's bench** has its reading for 0.15.0 (load 1.26), which receives the three new measurements: centring a row,
the conversion search (364 µs for 4000 subtitles) and aligning a translation. The bench on a real film remains
a manual measurement. **What remains unmeasured**: HDR, AV1, continuous playback with dropped-frame counting, a
machine without a graphics card.

## 15 — Internationalisation

Gaupol's 20 locales are under the GPL, hence reusable.

**Open question** — Qt Linguist or gettext? Converting `.po` to `.ts`
is faithful only if the strings match, which will not be the case
everywhere. Assess the real gain before committing.

**A deferral from phase 12 lands here: the names and descriptions of correction patterns.** Gaupol
translates them through gettext; the pattern files are read as they are
([ADR 0037](adr/0037-lire-les-motifs-de-gaupol-tels-quels.md)) and their labels stay in English as long as
the interface does. They go along with the rest of the interface, and the question of converting `.po` to `.ts`
arises for them too.

**Clarified by the user, on 2026-09-30: the embedded documentation comes here,
internationalised.** Two deliverables, both in the interface's languages:

- **The window's help page.** The manual that `Help` opens today
  (`docs/manual/`, in French) becomes **internationalised** help: it
  explains the various features the program offers, and follows the chosen
  language like the rest of the interface. What the manual already says is its
  material; what the phase changes is that it is translated with the interface,
  and not after it.
- **Manual pages (`man`) for the command line**, internationalised as well:
  one page for `subedit-cli` and one for each subcommand whose use calls for
  it, installed with the packages (`.deb`, `.rpm`) where `man` looks for them.

At scoping: where the pages come from (a single source, the current manual, from
which the window's help and the `man` pages are drawn, or two sources), how their
translation stays up to date when the manual moves, and whether the manual's
screenshots are taken per language. **A single source** is the route that keeps
two texts from diverging; it is also the most expensive to tool.

**Scoped on 2026-10-09** ([#655](https://github.com/Guyot-Bertrand/sub-edit/issues/655), spec
[15-internationalisation.md](specs/15-internationalisation.md), ADRs 0042 to 0044): gettext, read by our own `.mo` reader
in the core (the core has no Qt, and Gaupol's catalogues are `.po`); **English becomes the language of the whole repository** and
French one language among others; **the whole repository is translated** (about 220,000 words), by Claude's hand and reviewed by
the maintainer; a single source for the manual, the window's help and the `man` pages. The phase ships two languages,
English and French, and the tooling for Gaupol's twenty, the others of which remain uninstalled seeds. Twenty-seven
issues, [#658](https://github.com/Guyot-Bertrand/sub-edit/issues/658) to
[#684](https://github.com/Guyot-Bertrand/sub-edit/issues/684), written in English; closing in 0.16.0.

**Folded in on 2026-10-12: the settings shared by the window and the command line**
([#699](https://github.com/Guyot-Bertrand/sub-edit/issues/699), [ADR 0045](adr/0045-one-settings-file-two-parts.md)).
Framed first as a standalone spec, it is not a phase of its own: the maintainer placed it in this phase and its milestone.
The settings file gets an operation part, which the command line reads by default (`--no-config`, `--config FILE`), and a
display part, which stays the window's; `subedit-cli config list|show|set|unset` documents, shows and changes the
operation part. Its decisions are S1 to S8 of the [phase spec](specs/15-internationalisation.md).

**Closed in 0.16.0, on 2026-10-21** ([#684](https://github.com/Guyot-Bertrand/sub-edit/issues/684)). The review found the
two ratchets of the phase not yet at zero, and the closure brought them there: the 142 untranslated literals of 28 files
(reader and file failures, the text engine's reasons, the settings table of `config list`) went through the translation
API, the rest being exempted with a reason each (`deferred()` marks a message of a constant table, translated when it is
shown); the 61 files of `.french-allowlist` were translated, the list is empty, and the benchmark names were translated in
the code and in the journal together, so that the records still compare. The 85 new messages are translated into the
twenty installed languages. **The cost of translation at start-up**, measured in Release at 0.16.0 (load 1.49): reading a
shipped catalogue takes 338 µs for French and 359 µs for Russian, and finding a message in it 45.5 ns.

# Part three — beyond Gaupol

These phases are no longer feature parity: they add what Gaupol
does not do. They were born of a scoping exchange, on 2026-10-06, and **their
order is the one the user chose**; it does not rest on an effort
forecast.

**Intended audience: series and film enthusiasts looking to automate
their workflow.** The command line therefore ranks equal to the interface, and
"I run a command and the subtitle lands right" is the criterion to hold.

**These phases are drafts.** Each will be refined at its scoping — that is
where the exact scope, the requirements and the split into issues are settled.
What follows fixes the order, the intent and the decisions to anticipate, nothing
more.

```
17 ─► 18 ─► 19 ─► 20 ─► 21
                   │
                   └────────► 22 ─► 23 ─► 24
```

**What the phases hand on to each other.** 19 lays down the subprocess
mechanism and the detection of an external tool, which 21, 22 and 24 reuse. 20
lays down OCR, which 21 plugs into containers. 22 lays down the audio engine,
which 23 reuses for alignment.

**One rule for all external tools** (`mkvmerge`, `ffmpeg`, Tesseract,
whisper.cpp), extending the tolerance to the absence of `ffprobe`:
**their absence breaks nothing.** The feature that depends on them disables
itself with a clear message, or the user provides the data themselves. None of
these tools is shipped with the packages.

## 17 — Additional text formats

The text subtitle formats that neither Gaupol nor phases 1 and 9
cover. The list is to be drawn up at scoping, according to the real use of the
intended audience.

Each format moves the three usual measurements of phase 9: a row of the
promise table, the detection score, the conversion matrix.

## 18 — Advanced text formats

Named styles, positioning, karaoke: reading, writing and conversion between
the formats that carry them (ASS, TTML, WebVTT…).

**This is the phase that touches the most existing code.** The entry model is
marked-up text today; it must accommodate styles, a position per
entry and timed segments. That runs through the core and every format already
written, and calls for an ADR.

**To be settled at scoping:**

- the shape of the style and position model, and its place in the core;
- what becomes of information the target format does not carry: reported
  loss, refusal, or fallback — extending the conversion matrix;
- what the interface shows of it and allows editing, or whether this phase
  first limits itself to the library and the command line.

**A heavy phase, to be split at scoping** if measurement calls for it.

## 19 — Embedded text tracks

List, extract and inject text subtitle tracks in MKV and MP4 containers, with
their metadata: language, "forced" and "hearing impaired" flags, default
track.

**To be settled at scoping — and the decision holds for phases 21, 22 and 24:**
link a library (libavformat) or call `mkvmerge` / `ffmpeg` as a
subprocess. **The subprocess is the expected direction**: it keeps the
executables minimal and leaves the user in control of their tools. It assumes
detecting the tool's presence, and tolerance to its absence.

Other questions: open a container directly from the window and the
command line, or only extract; rewrite the container (remux) or
export alongside.

## 20 — Image formats and OCR

Read image subtitles — **VobSub** (`.idx` / `.sub`), **PGS** (`.sup`,
Blu-ray) and **DVB-sub** (TV streams, recorded `.ts`) —, run them through
OCR, and produce text subtitles.

**Two parts, which do not move at the same pace:** reading image formats,
which is binary parsing and is tested on fixtures, and OCR proper.

**To be settled at scoping:**

- the OCR engine — Tesseract as a library or as an executable — and the
  origin of its language data, which the user downloads;
- **the human correction interface**, which is not a detail: OCR output gets
  proofread. It relies on the "OCR" patterns of the correction engine of
  [phase 12](#12--complete-correction-engine);
- the command-line version: OCR without proofreading, with a report
  of doubtful entries.

**A heavy phase, to be split at scoping**: reading formats on one side, OCR and
the proofreading interface on the other.

## 21 — Embedded image tracks

Extract image tracks from containers and chain them to the OCR of phase 20,
end to end: from the video file to text subtitles.

It adds almost nothing new — it is the junction of 19 and 20 —, and
that is precisely what makes it short.

## 22 — Audio transcription

Produce subtitles, with their positions, from a video's audio.

**Expected engine: whisper.cpp**, which runs on an ordinary processor,
without a graphics card; a graphics card only speeds up processing. The models
weigh from a few hundred MB to several GB: **they are not shipped**, and
the user fetches them, through a dedicated command or a path to fill in.

**To be settled at scoping:**

- executable as a subprocess or library — same direction as for
  phase 19: the executable, which leaves the choice of hardware variant;
- the build shipped in the packages: CPU only by default;
- the output: timed sentences, or timed words, which phase 23 reuses;
- detecting the language of the audio.

## 23 — Automatic alignment

Re-time an existing subtitle on the audio: from the text and the voice,
find where each sentence is actually spoken. This is the case of the `.srt`
fetched online, shifted at the start and at the end, which two-point correction
does not catch.

**It depends on phase 22**, whose engine and timed words it reuses, and remains
optional like it. It is shipped on the command line
and in the interface.

**To be settled at scoping:** the behaviour when alignment is doubtful
(entries not found, progressive drift) and how to report it to
the user.

## 24 — Workflow and integrations

What makes it possible to chain the previous phases without replaying them by hand:
batch processing, watched folder, command chaining, stable JSON output
for scripts.

**A phase to scope last**, because it automates what the others
ship: its exact scope depends on what use will have shown during
phases 17 to 23. The `--recursive` of [phase 13](#13--complete-cli) is its
starting point.

## For a v3

Kept but set aside for now, on 2026-10-06:

- **server mode** (`subedit serve`);
- **the interactive waveform** — the drawing of the sound under the video, with
  subtitle blocks dragged with the mouse to time them. The automatic
  alignment of phase 23 covers most of the need without it;
- **subtitles burned into the picture** ("hardsubs"), by OCR on the
  video;
- **machine translation** and **quality**: compliance with broadcaster
  standards, translation memory, merging versions;
- **shot change detection.**
