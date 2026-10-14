# Phase 2 — Editing operations

**State:** design approved, ready to be split
**Milestone:** `2 — Opérations d'édition`

## Goal

Give the core the operations that editing in a table presupposes, and the three
position operations listed as priorities. All undoable, all written as commands
on the foundation of phase 1.

At the end of this phase, one can modify a subtitle, insert some, remove some,
shift, transform and convert the frame rate — with no interface, which belongs
to phase 5.

## Scope

**Included** — modifying a text, a start, an end; inserting, removing;
shifting, transforming by two reference points, converting the frame rate;
sorting; the target of an operation; the type that brings a project and its
history together.

**Excluded, by phase** — duration adjustment, case, italics, dialogue dashes,
merging, splitting, find and replace, clipboard (phase 10); removal of hearing
impaired annotations (phase 4); any interface (phases 3 and 5).

## Applicable decisions

| ADR | Decision |
| :-- | :------- |
| [0006](../adr/0006-positions-en-millisecondes.md) | positions in whole milliseconds, strong types |
| [0010](../adr/0010-annulation-par-commandes.md) | commands carrying their own inverse |
| [0011](../adr/0011-numero-d-image-en-type-fort.md) | frame number as a strong type |
| [0012](../adr/0012-ordre-des-sous-titres-par-composition.md) | never sort on its own, strict mode by composition |
| [0013](../adr/0013-mise-a-l-echelle-exacte-des-positions.md) | scaling by an exact rational, rounded once |

The last two are specific to this phase and were written before it.

## What the core lacks

Taking stock, done before writing this spec, found four gaps and one
inconsistency. They drive the split.

| Gap | Consequence |
| :-- | :---------- |
| no scaling of a position | transformation and frame rate conversion cannot be expressed |
| `Project` can neither insert nor remove | two operations out of seven without support |
| nothing brings a project and its history together | every operation needs both |
| a command has no name | phase 5 would have nothing to display |

The inconsistency: ADR 0010 states that modifying a project outside a command is
a defect, whereas `setSubtitles`, `setFrameRate` and mutable access to a
subtitle are public and unguarded. Dealt with below.

## Scaling positions

```cpp
class Ratio;   // exact rational, signed, reduced

Timestamp Timestamp::scaledBy(Ratio) const;
Duration  Duration::scaledBy(Ratio) const;
```

A named method rather than an operator: scaling a position is a **rounding
decision**, and ADR 0006 wants none of those to be made implicitly.

**The round trip through frames does not do.** One might think that converting
a frame rate is written `fromFrame(toFrame(t, input), output)`. It is wrong:
that path quantises every position onto the input frame grid, a loss the
phase 1 spec already documents — `ms → frame → ms` is not the identity.
Measured from 25 to 24000/1001, the difference from the exact calculation
reaches **21 ms**, that is half a frame:

| position | through frames | exact | difference |
| -------: | -------------: | ----: | ---------: |
| 1010 ms | 1043 | 1053 | 10 ms |
| 1020 ms | 1084 | 1064 | 20 ms |

The rule is therefore: **a single rounding, on an exact rational.** The
intermediate calculations fit in a `std::int64_t` — a plausible position does
not exceed `3.6 × 10⁸` ms, and a reduced rational keeps a numerator of the same
order.

## Target of an operation

```cpp
class Selection;   // sorted indices, no duplicates

Selection::all(const Project&);
Selection::of(std::span<const SubtitleIndex>);
Selection::range(SubtitleIndex first, SubtitleIndex last);
```

Gaupol repeats in every signature an `indices` parameter that is `None` for
"the whole project". An absent value that means "all" is exactly the kind of
convention the compiler should carry: `Selection::all` says it.

Sorted and without duplicates **by construction**, which insertion and removal
need in order to be correct.

## Session

```cpp
enum class OrderPolicy { Lenient, Strict };

class Session {
    Project     m_project;
    History     m_history;
    OrderPolicy m_policy = OrderPolicy::Lenient;
};
```

The missing type. Issue #4 ruled out putting `History` in `Project` — `model`
would depend on `command`, and above all a `Project` would stop being copyable.
`Session` brings them together without mixing them: it is what an application
holds per open file, and what the command line of phase 3 as well as the window
of phase 5 will handle.

It lives in a fifth module, `core/edit/`, which also hosts the operations.

**`Session::project()` returns a const reference.** It is the answer to the
inconsistency found above: inside a session, the only path to a modification is
a command, and the compiler enforces it. A bare `Project` remains modifiable —
the reader does have to fill it — but whoever holds one is building it, not
editing it. That is how ADR 0010 is to be understood, and the boundary is
checkable instead of being promised.

## Grouping actions — a closed question

**Closed by phase 5, and without a mechanism.** The roadmap raised the question
here with typing in mind: twenty characters typed into a cell must not make
twenty undo entries, and a way to bring them together seemed needed.

The way did not have to be built, because typing produces nothing. A
`QStyledItemDelegate` returns its value only once — on `Enter` or on focus
loss —, never keystroke by keystroke. **An edited cell produces one command**,
so there is nothing to group.

The mechanism that would have served exists anyway, written for another reason:
`CompositeCommand` already fits an operation and the sort the strict policy
adds to it into a single entry. It will wait for a real need — the find and
replace of phase 10 is a candidate, as it touches several subtitles in one
gesture.

Observed in issue [#129](https://github.com/Guyot-Bertrand/sub-edit/issues/129),
which put the editing delegates in place.

## Order policy

**`Lenient` by default.** The core never sorts on its own: it reports and does
not repair, as ADR 0008 lays down for reading. An opened file keeps the order of
its file, and an edit moves no row.

**`Strict` is a composition policy, not a model invariant.** After an operation
that may break the order, the session chains a sort command into the same
composite: a single history entry, a single undo.

This choice, rather than an invariant held by `Project`:

- **indices stay stable inside a command.** `Change` carries `SubtitleIndex`
  values, the history replays commands, the table will refresh by index. If an
  edit could silently move a row, *every* command would become "and maybe a
  move";
- **a single data model.** Two invariants depending on a mode would not be a
  setting but two models, and every command would have to ask which one;
- **sorting becomes undoable**, which it is not in Gaupol, where it predates
  the existence of the history;
- **the modification counter tells the truth.** Opening an unordered file in
  strict mode marks the document modified — which it is, since it differs from
  the disk.

Accepted price: when the sort kicks in, the composite moves rows and its
`Change` must express that. The difference is that this case comes from an
identified command, not from any command at all.

### What Gaupol does, and why we depart from it

Gaupol maintains the order in two places only: on opening, where it sorts and
warns through a dialog box; and on `set_start`. `replace_positions` — through
which shifting, transformation and frame rate conversion go — does not sort.
Shifting part of a selection can therefore leave the file out of order there
without anything noticing: **the invariant is not one.**

Its counting has a hole too. It counts in frames what it sorts in seconds: two
subtitles inverted by less than a frame are not counted, no dialog box opens,
and yet the file is reordered.

Our diagnostics model meets this need better than a dialog: a `Diagnostic` is a
**state** that persists in the `ReadResult`, not an event that disappears once
the window is closed.

### What reading must learn to say

`DiagnosticKind` gains `OutOfOrder` — a subtitle whose start precedes that of
the previous subtitle. We already report overlap, which is weaker. Like every
value of this enumeration, it is emitted by a reader: a value nobody produces is
a broken promise.

`Project` gains a pure query, `outOfOrder()`, which returns the indices
concerned. The interface uses it to show the disorder without the rule living
in the interface.

## The operations

All of them are commands. Each captures at construction what it needs to undo
itself, and nothing more.

| Operation | What the inverse retains |
| :-------- | :----------------------- |
| modify a text | the old text, the index, the document |
| modify a start, an end | the old position |
| insert | the inserted indices |
| remove | the removed subtitles and their indices |
| shift | the offset, to be applied in reverse |
| transform | the previous positions of the selection |
| convert the frame rate | the previous positions, and the old frame rate |
| sort | the previous order |

Shifting retains a `Duration` and not the previous positions: its inverse is
exact, and that is the economy ADR 0010 calls for. Transformation and
conversion, on the other hand, round — their inverse cannot be a second
calculation and must retain the positions.

### Insertion

Inserting without content creates empty subtitles, in Gaupol's manner: they
share the free window before the next subtitle, or last three seconds if they
are added at the end.

### Transformation by two reference points

The user designates two subtitles and the position their start must take. The
correction is affine:

```
t′ = round( (t − x₁) × r ) + y₁        with r = (y₂ − y₁) / (x₂ − x₁)
```

Written this way rather than `round(r × t) + constant` — Gaupol's form — for two
reasons: **a single rounding**, and above all **the two reference points land
exactly where the user asked for them**, since `(x₂ − x₁) × r` equals `y₂ − y₁`
exactly. Gaupol's form does not guarantee it.

`r` is a `Ratio`, hence exact. A zero denominator — two reference points on the
same subtitle — is refused at construction.

### Frame rate conversion

```
t′ = round( t × (input / output) )
```

These are Gaupol's semantics: a film mastered at 23.976 and played at 25 frames
per second runs faster, so the positions move earlier. The command also changes
the project's frame rate, and its inverse restores both.

## Name of a command

```cpp
enum class CommandKind { SetText, SetStart, SetEnd, Insert, Remove,
                         Shift, Transform, ConvertFrameRate, Sort };

CommandKind Command::kind() const;
```

An enumeration and not a string, for the reason that already holds for
`DiagnosticKind`: it can be translated, and a test can hook onto it without
comparing prose that will be reworded. It is what phase 5 will display in
"Undo: shift positions".

## What `Project` gains

```cpp
void insert(SubtitleIndex at, std::span<const Subtitle>);
std::vector<Subtitle> remove(const Selection&);   // returns what was removed
std::vector<SubtitleIndex> outOfOrder() const;
```

`remove` returns the removed subtitles: it is exactly the state the inverse
command must retain, and returning it avoids reading it again before erasing
it.

## Tests

For each of the eight operations: applying then undoing restores the **exact**
state, and redoing reproduces it. It is the property issue #4 proved on dummy
commands; here it is checked on the real ones.

Cases that cannot be guessed and must be covered:

- the **two reference points of a transformation land exactly** on the
  requested positions;
- a frame rate conversion gives the **same result as the exact rational
  calculation**, and not that of the round trip through frames;
- a **partial shift** that breaks the order: reported under `Lenient`, sorted
  under `Strict`, and undoable in both cases;
- a removal followed by an undo restores the subtitles **at their original
  indices**, including for a discontinuous selection;
- the **modification counter** comes back to zero when undoing back to the save
  point, through a composite.

The frame rate conversion benchmark set up in issue #9 measures the round trip
through frames, that is an operation this phase will not implement. It has to
be redone.

## Split into issues

In order of processing.

| Work | Nature |
| :--- | :----- |
| `Ratio`, scaling of `Timestamp` and `Duration` | structuring |
| `Selection`, `Project::insert` / `remove` / `outOfOrder`, `OutOfOrder` diagnostic | structuring |
| `Session`, `CommandKind`, order policy, sort command | **checkpoint** |
| modify a text, a start, an end | mechanical |
| insert, remove | mechanical |
| shift | mechanical |
| transform by two reference points | mechanical |
| convert the frame rate | mechanical |
| benchmarks redone, manual section | mechanical |

The third is the checkpoint: it sets the boundary between the model and the
policy, and everything that follows builds on it.

## Completion criteria

- [ ] The eight operations are undone and redone identically
- [ ] The two reference points of a transformation land exactly on their target
- [ ] The frame rate conversion equals the exact rational calculation
- [ ] A partial shift breaking the order is reported in lenient mode, sorted in
      strict mode
- [ ] `Session::project()` is const: no modification outside a command
- [ ] `make check` passes, coverage ratchet held
- [ ] Benchmarks redone, manual section and CHANGELOG entry up to date
