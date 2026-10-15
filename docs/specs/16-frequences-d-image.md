# Phase 16 — Frame rates: deduction and correction

**Milestone:** `16 — Fréquences d'image : déduction et correction`
**Scheduled between [6](06-lecteur-integre.md) and 7.** Its number is the
first one free when it was added; it identifies it and does not say its
rank.

## Goal

A subtitle file does not declare the frame rate against which its positions
were computed — SubRip has no header, WebVTT's is free
text. This phase **deduces it from the positions themselves**, and brings
the two operations that revolve around it: putting a file back on its grid, and
snapping it to another one.

## What sets it apart

**It goes beyond iso-functionality, and it is the first to do so.** Gaupol deduces
no frame rate, and the chain takes three hops: `framerate_convert.py`
prefills its **two** lists with `page.project.framerate`; `page.py` gives
that value to the project from `conf.editor.framerate` on opening; and
`config.py` sets that default to `FPS_23_976`. **Nothing, anywhere, looks at the
file** — it is a global preference, and it has not moved in twenty years.

**There is therefore nothing to read in `reference/gaupol`**, and the preliminary
analysis step of a phase's course is empty, for the first time. What
replaces it: the fixtures of #198 and what they yield.

**It tests a judgement, not a behaviour.** The previous phases
tested what the software *does* — read, write, shift. This one tests
what it *concludes*: "this file is timed on a grid at 23.976, with a
concentration of 99.9". A judgement is compared with an expectation, and the expectation is
now versioned: `src/test/data/grilles/` and its `LISEZMOI.md`.

## Where the method comes from

**Converting a frame rate assumes that the positions were timed on a
frame grid.** When that is true, each position is `round(n × 1000 / R)`,
and that grid can be measured.

For each of the eight standard frame rates, the phase of each
start on the corresponding grid is computed, and **the concentration of those
phases** is measured — the length of the resultant vector, from 0 to 100. A hundred: a
perfect grid, whatever its phase. Near zero: no structure.

It has **no tolerance parameter**, and it is **insensitive to a
shift**. That second point was paid for: a first version looked for a
grid of zero phase and scored zero a file that is a perfect 24 frames per
second, translated by a millisecond and a half.

## Scope

**In the phase:**

- the deduction, and what it returns;
- two operations — putting a file back on its grid, snapping it to a
  chosen frame rate;
- the cross-reading with the frame rate the video container declares;
- the surfaces: `inspect`, two subcommands, the status bar, an analysis
  dialog, and the prefilling of the conversion dialog.

**Outside the phase:**

| What is deferred | Where |
| :--------------- | :---- |
| finding the pair of a conversion made with the wrong frame rate | phase 10, with the complementary operations — "the most useful and the least certain" |
| marking in the table the positions that fall off the grid | nowhere: **D12** forbids it, and says why |
| batch processing and script-readable output | phase 13 |
| editing in frame mode | phase 9 |

## Applicable decisions

**D1 — the method is phase concentration, without tolerance.** No threshold
enters the computation; thresholds only appear when a
verdict is returned, and only once (**D4**).

**D2 — the set of candidates is closed and small: the eight of
`StandardFrameRate`.** Solving for an arbitrary `R` is an entirely different
problem, and pointless — nobody masters at 26.3 frames per second.
`src/test/data/grilles/grille-absurde.srt` is exactly that file, and the
right answer about it is "I don't know".

**D3 — the starts are the signal; the ends are not measured.** Wherever
a grid exists, the starts are on it without exception; the ends range from 55 to 100,
because a *cue-out* is often computed by a reading-speed rule and
not placed on a frame. Mixing them would lower a clear signal with known
noise.

**D4 — the deduction returns the complete ranking of the eight, and a verdict computed
only once.** The ranking hides nothing; the verdict — `clean`,
`partial`, `silent` — keeps `inspect`, the analysis dialog, the status
bar and the cross-check with `ffprobe` from each reimplementing the thresholds and
ending up no longer saying the same thing.

**D5 — between two harmonic candidates, the lower one.** A grid at 25 is
**included** in a grid at 50: a file timed on 25 comes out at 100 on
both, and the converse is false — `grille-50.srt` gives only 15.8 on 25.
The implication runs only one way, so the lower one is the
parsimonious explanation.

This is not a theoretical precaution. Taking the maximum gets it wrong, and the
fixtures show it:

| Fixture | Timed on | What the maximum gives |
| :------ | :------- | :--------------------- |
| `grille-25.srt` | 25 | 25 at 100.0 — **and 50 at 100.0**: heads or tails |
| `grille-30.srt` | 30 | 30 at 99.9, 60 at 99.5 — right by luck |
| `melange-groupe.srt` | 29.97 | **59.94 at 66.4 against 65.1 — wrong** |

The only ambiguous pairs of the standard set are those where one is an
integer multiple of the other: 25 and 50, 29.97 and 59.94, 30 and 60. No others —
24 and 60 are in a ratio of two and a half, so only one frame in two
coincides. **The ambiguity is stated, never hidden**: the deduction names the
candidate set aside.

**When does the pair count as ambiguous?** Clarified by #203, which had to
write it: the **bands** are compared, not a gap. A file truly timed
on the higher frame rate makes the lower one collapse towards noise, so the two
fall into different bands; a file timed on the lower one puts them in
the same band, whatever rounding did to their order. The rule then requires
no new threshold — it reuses those of the verdict — and it holds on the eight
cases the fixtures offer:

| Fixture | The lower | The higher | Same band? | Retained |
| :------ | --------: | ---------: | :--------- | :------- |
| `grille-25.srt` | 100.0 | 100.0 | yes | 25 |
| `grille-30.srt` | 99.9 | 99.5 | yes | 30 |
| `grille-29-97.srt` | 99.8 | 99.4 | yes | 29.97 |
| `grille-50.srt` | 15.8 | 100.0 | no | 50 |
| `grille-60.srt` | 1.3 | 99.5 | no | 60 |
| `grille-59-94.srt` | 4.8 | 99.4 | no | 59.94 |
| `melange-groupe.srt` | 65.1 | 66.4 | yes | 29.97 |
| `melange-disperse.srt` | 75.4 | 84.9 | yes | 25 |

A numeric gap would not have held: the last two rows are separated by
1.3 and 9.5 points, and the fourth by 84. No margin fits between 9.5
and 84 without being chosen after the fact for that purpose.

**D6 — the span is returned, and the candidates it does not separate are
named.** 23.976 and 24 drift apart by one millisecond per second of film, and a
frame lasts 41.7 ms: the phase on the wrong candidate makes a full turn
every forty-two seconds. Over a feature film, they separate without
question; over ten seconds, they do not — `grille-24-courte.srt` gives 24 at 99.9 and 23.976
at 92.7. Returning "24, confidence 99.9" would be lying by omission.

**D7 — a partial file returns which starts are off the grid.** Saying "two
thirds of your starts are on a grid at 29.97" is true and helps nobody.
The deduction therefore returns, for the retained candidate, the indices of the starts that
deviate from it by more than one millisecond, and the presentation derives the shape from them.
The number of consecutive runs is enough to tell the two causes apart, and it
costs only one pass:

| Fixture | Off grid | Runs | Longest |
| :------ | -------: | ---: | ------: |
| `melange-disperse.srt` — editing by hand | 31 / 171 | **31** | **1** |
| `melange-groupe.srt` — a retimed section | 52 / 168 | **5** | **27** |

**D8 — the phase is returned, in milliseconds, and is never corrected
silently.** A file whose positions are on the grid up to a constant
has been shifted, and that constant can be measured. Correcting it is an operation
the user asks for (**D10**), not a side effect of reading.

**D9 — the deduction runs when the file is opened.** That is what lets the
status bar, the conversion dialog and `inspect` show it without
the user asking for it. It is therefore **on the reading path**, which costs
2.5 ms for 4000 subtitles: its budget is **a fifth of reading**,
checked by the benchmark and not hoped for. Beyond that, it becomes lazy.

**Measured by #203: 374 µs** in the record of version 0.6.3, against 2.32 ms for
reading in the same record — **16 %, the budget is held.**

**The window goes beyond "on opening", and that is intended.** It recomputes on
every change of the document, because a snap changes the grid and the
status bar must not keep announcing the old one. That is what
[ADR 0021](../adr/0021-analyse-du-document-a-l-ouverture.md) makes possible by
refusing to keep a derived value: there is no invalidation to maintain, and
the cost remains that of a fraction of a millisecond.

It was not so at the first attempt. The gain comes from a property and not from a
trick: a position is a whole number of milliseconds, so on an integer
frame rate the phase takes only a thousand values, and five of the eight
candidates are then answered by an **exact** table, not an approximate one. Measured
back to back, it halves the cost.

**D10 — two operations, and they do not do the same thing.**

| Operation | What it does | What it preserves |
| :-------- | :----------- | :---------------- |
| **put back on the grid** | shifts the whole file by the measured phase | the relative timing, **exactly** |
| **snap to a frame rate** | moves each timestamp — start *and* end — to the nearest frame of the chosen frame rate | the position of each line, to within half a frame |

On a perfectly timed file, both give the same result: if the
phase is less than half a frame, moving each timestamp to the nearest
frame amounts to shifting the whole by the same amount. **They diverge
on a partial file**, and that is where their reason for being lies: the first
leaves the deviations where they are, the second absorbs them. `melange-disperse.srt`
is the file that shows the difference.

**D11 — snapping is not converting, and that is the trap of this phase.**

| | Convert | Snap |
| :--- | :------ | :--- |
| formula | `t' = t × R_in / R_out` | `t' = round(t × R / 1000) × 1000 / R` |
| displacement | proportional to time — **several seconds** over a feature film | half a frame at most, **everywhere** |
| for which file | a file whose timing is **wrong** | a file whose timing is **right** and whose grid is wrong |

The use case of snapping, spelled out: a video at 25 frames per
second, a subtitle file written on a grid at 24, lines already
roughly in place to within a few milliseconds. There is nothing to
retime — only to set the timestamps back on the right grid.

Mistaking one for the other is the worst failure mode of the phase: both
accept the same arguments and one shifts the whole file without
saying anything. **The manual and the online help contrast them explicitly**, each in
the other's section.

**D12 — the deduction never marks a row.** Phase 5 forbids itself
from using the grid to mark anything in the table, and that
prohibition holds. As soon as a user corrects a position by hand, it
stops being aligned: a naive detector would flag their work as an
anomaly.

**A row tint has existed since #211**, however, and it does not come
from here: it is the line the film shows at that instant. It is transient,
it follows playback and not a measurement, and an anomaly overrides it. D12
forbids the *deduction* from marking a row, and that remains true. `melange-disperse.srt` is precisely that file, and accusing its
thirty-one rows would be accusing the user. **The deduction speaks of the
document, never of a row** — except in the analysis dialog, which
the user opens for that purpose.

**D13 — a disagreement between the two sources is shown, and nothing is chosen.**
The container *declares* a frame rate (phase 6, `ffprobe`), the positions
*betray* another. They are not the same datum: the first says at what
rate the film runs, the second on which grid the file was written. A
file at 24 on a film at 25 is not a contradiction, it is the use case
of snapping. Phase 6 had laid down the rule in advance — "a source that
takes precedence forbids cross-checking" (D6 of its spec) — and it applies here to the
third source.

**D14 — the deduction lives in `core/analysis/`.** It is not a time
primitive like `FrameRate`, `Timestamp` or `Ratio`: it is an observation about a
document. The directory is new and will later host the anomalies of
`core/model/anomaly.hpp`, which are the same family — that move is not part of
this phase. *(Done since, outside the phase, by #227.)*

## The core

### `core/analysis/frame_rate_deduction.hpp`

A pure function on the positions, without state. It depends only on
`core/time/` — `Duration`, `FrameRate` — and on `SubtitleIndex`, which is a value
type of the model and not an operation. **It knows neither `Project`, nor
`Session`, nor the history**: it receives positions and returns a judgement, which
is exactly what makes it testable on fifteen files and a table of
constants.

```cpp
struct GridFit {          // a candidate, and what it is worth
    FrameRate rate;
    double concentration; // 0 to 100
    Duration phase;       // the constant shift, in milliseconds
};

enum class GridVerdict { Clean, Partial, Silent };

struct FrameRateDeduction {
    std::array<GridFit, 8> ranked;         // descending, D4
    GridFit retained;                      // the answer, after D5
    GridVerdict verdict;                   // D4
    std::optional<FrameRate> harmonic;     // the candidate set aside by D5
    std::vector<FrameRate> notSeparated;   // what the span does not separate, D6
    Duration span;                         // the span seen, D6
    std::size_t starts;                    // how many starts produced it
    std::vector<SubtitleIndex> strays;     // the starts off the grid, D7
};
```

**`retained` is not `ranked.front()`, and both are exposed.** That is what
D5 imposes: the ranking stays the honest order of the concentrations, the answer
is the one the harmonic rule designates. Conflating them would force either
lying about the order or redoing the rule in every caller.

The thresholds — 90 for `Clean`, 50 for `Partial` — are written **only once**,
here, and nowhere else. That is the conclusion of #197: the score of a
file × frame rate pair is a test constant, not a matcher.

### The two operations

They enter the history like the others, through `Session::apply`, and are
undoable.

**Putting back on the grid** is a `ShiftCommand` whose amount comes from the
measurement rather than from the user. No new command: it is a shift,
and writing it otherwise would duplicate a behaviour already tested — including
the warning about going past the end of the film (phase 6, D4 of its spec). It
**is not available when the verdict is `Silent`**: there is no grid
to rejoin.

**Snapping to a frame rate** is a new command, `SnapCommand`. It moves
each timestamp — start and end — to the nearest frame of the given
frame rate. The frame rate is chosen by the user and **prefilled with the one
the video declares**, not with the one the deduction finds: the intent is
to rejoin the film's grid, and the deduction says the one being left.

Two properties to hold, and to test:

- **idempotence** — applying the same frame rate twice changes nothing the
  second time;
- **order is preserved** — quantisation is monotonic, so two starts
  at least one frame apart stay so. Two starts closer than one
  frame may merge, and that is an anomaly the core already knows how to
  name.

## The command line

`inspect` gains a section, and two subcommands appear.

| Surface | What it carries |
| :------ | :-------------- |
| `inspect` | the retained candidate, its concentration, the span, the phase, the verdict, and the harmonic ambiguity if there is one |
| `shift --to-grid` | put back on the grid; the amount comes from the measurement, and it is written before being applied |
| `snap --rate <R>` | snap to a frame rate; writes how many positions moved and by how much at most |

`--rate` accepts the six forms already accepted by `framerate` (phase 2), and
**any valid rate**, not only the eight standard ones.

> **Correction, by #207.** This line said the opposite — "`snap` refuses a
> frame rate outside the eight standard ones, like `framerate`" — and was wrong twice.
> `framerate` does not refuse them; and the closed set of eight belongs to
> the **deduction**, which must choose among candidates, not to an operation
> whose target the user names themselves. Restricting `snap` would have added
> a rule with no benefit and inconsistent with the operation next to it.
>
> The requirement `CLI-SNAP-03` that followed from it is therefore **withdrawn** before having
> been written, rather than made true by force.

A silent file makes `inspect` say that it found no grid, **and not
a low-confidence frame rate**: that is the meaning of D2, and it is the property
that makes it legitimate to use the deduction.

## The window

| Surface | What it carries |
| :------ | :-------------- |
| status bar | the verdict, next to `Video: film.mkv` — for example `Grid: 23.976 fps` or `No grid` |
| `Frame Rate Analysis…` | the analysis dialog: the ranking of the eight, the span, the phase, the starts off the grid and their shape |
| `Convert Frame Rate…` | **the input is now prefilled with the measurement**, and its origin is stated |
| `Shift Whole File onto Grid` | the operation of D10, amount **and scope** shown before being applied |
| `Snap to Frame Rate…` | the operation of D10, frame rate prefilled with the one the video declares |

### An operation works on the selection, the grid speaks of the document — issue #324

**Two rules of this window are each right on their own side, and they do not
apply to the same thing.** An operation takes the selection; the grid
speaks of the document. Seen side by side, they read like a missed
refresh: snapping five rows out of a hundred and seventy-six makes the
timestamps move in the table while the status bar and the analysis say
nothing. They have nothing to say — the document is still read on the grid that
the other hundred and seventy-one hold.

**The gesture is not harmless for all that, and it is measured.** On the eight
grid fixtures, snapping the whole file brings the deduced rate to the
target and its concentration to 100 %; snapping five rows never moves the
deduced rate and **lowers** the concentration in seven cases out of eight — 99.9 to
96.6 at worst. The eighth is 50 to 25, where the target grid divides that of the
file. Snapping part of a file to a rate the rest does not share
is almost always unintended.

**A notice says so, once the operation is done** — `noticeOf(PartialAlignment)`
and `partialAlignment`, the same form as what D4 gives to going past the end
of the film: an analysis returns what there is to say or nothing, and the wording is
elsewhere. It carries the scope and the deviation, because neither alone
is enough — the scope links the gesture to its effect, the deviation is the news.

It stays silent when the selection covers everything, when no grid is deduced,
and when the document is now read on the requested rate.

**Set aside: a confirmation dialog before applying.** It would stop the
unintended gesture and block the deliberate one, whereas snapping part
of a file can be intended. The project's rule is to say rather than to
stay silent, not to prevent — ADR 0008.

**Set aside: stating permanently in the status bar what the grid
looks at.** It would clutter a line read at every instant for a case that only
arises after an operation.

**And `Shift onto Grid` becomes `Shift Whole File onto Grid`.** It is the only
operation of the menu that ignores the selection — a grid is a property of the
document, and half a document has no grid of its own. The manual
said so, the window did not, so the only entry that behaves differently
looked like all the others. It already carried its amount in its
label; it now carries its scope there too.

The conversion dialog changes in nature, and the interface manual already
announces it: it writes today that the dialog prefills "the output, never
the input", because "the file was timed against a frame rate that only it
knows". That is exactly what this phase brings. **The line therefore becomes
a measurement, not a preference** — and it stays editable, by D13.

When the measurement and what the video declares diverge, **both are displayed**:
the input carries the measurement, the output carries what the film announces, and nothing is
chosen in the user's place.

**Cosmetic fixes to the window will be included in this phase**,
listed before the closing. They have nothing to do with the deduction and are
grouped here because the phase already touches the interface.

## Tests

The fixtures exist: `src/test/data/grilles/`, fifteen files, and their
`LISEZMOI.md` carries the two tables — what each one is, and what each one
gives on the eight candidates.

| What is tested | On what |
| :------------- | :------ |
| the right candidate comes out | the eight `grille-<R>.srt` |
| failure is loud | `grille-absurde.srt` — under 16 everywhere |
| the phase is not waste | `grille-24-decalee.srt` — 99.9 despite 2 999 ms |
| the harmonic ambiguity and D5 | `grille-25.srt`, `grille-30.srt`, `melange-groupe.srt` |
| the span and D6 | `grille-24-courte.srt` — 23.976 at 92.7 against 99.9 |
| the partial file, its two causes and D7 | `melange-groupe.srt`, `melange-disperse.srt` |
| idempotence of snapping | all of them, applied twice |
| snapping is not converting | `grille-24.srt` snapped to 25 moves by less than half a frame; converted, by several seconds |
| the starts are enough | `grille-24-fins-calculees.srt` — 99.9 on the starts, 52.4 on the ends |
| no grid at all | `sans-grille.srt` — 12.5 at best, silent verdict |

The expected concentrations are **constants in the test code**,
taken from `LISEZMOI.md`. No matcher: nine fixtures and eight candidates make
a table of numbers, and a table reads better than a verb.

The last two rows were **added in phase 10**, issue #373: this
spec named them as what was missing "if a test asks for it", and a test
asked for them. `grille-24-fins-calculees.srt` keeps the deduction honest the day
someone makes it read the ends; `sans-grille.srt` turns "failure is
loud" from an observation on the private corpus into a versioned property.

## Measurements

| Measurement | Why |
| :---------- | :-- |
| the deduction on 4000 subtitles | the budget of D9, a fifth of reading |
| snapping on 4000 subtitles | one more operation, to compare with the four existing ones |

`lecture de 4000 sous-titres` already carries the path on which D9 places the
deduction: that is the record that will say whether the budget is held.

## Requirements

| ID | Requirement |
| :- | :---------- |
| `CLI-INSPECT-05` | `inspect` writes the deduced grid, its concentration, the span and the verdict |
| `CLI-INSPECT-06` | a file without a grid says so, and names no frame rate |
| `CLI-INSPECT-07` | a harmonic ambiguity is named, and the lower one is retained |
| `CLI-SHIFT-03` | `shift --to-grid` shifts by the measured amount, and writes it |
| `CLI-SHIFT-04` | `shift --to-grid` on a file without a grid refuses and says why |
| `CLI-SNAP-01` | `snap --rate R` moves each position to the nearest frame of R |
| `CLI-SNAP-02` | `snap` writes how many positions moved, and by how much at most |
| `GUI-GRID-01` | the status bar carries the verdict of the deduction |
| `GUI-GRID-02` | the analysis dialog shows the eight candidates, the span and the deviations |
| `GUI-FRAMERATE-03` | the conversion dialog prefills the input with the measurement, and says so |
| `GUI-FRAMERATE-04` | a disagreement between the measurement and what the video declares is shown without being arbitrated |
| `GUI-SNAP-01` | `Snap to Frame Rate…` snaps the document, and the operation can be undone |
| `GUI-GRID-03` | putting back on the grid shows its amount, and is disabled without a grid |

**Twelve, not thirteen.** `CLI-SNAP-03` was withdrawn by #207 before being
written — the reason is above — and `GUI-GRID-03` was born in #210, when
the menu entry had to state its amount before applying it. Both deviations
are recorded here rather than silently corrected, and the end-of-phase
review (#222) confronted this table with the registry row by row.

**They enter the registry at the start of the issue, in the `planned` state.** The framing of
phase 6 deferred the question here: the rule says so, practice has
never followed it over a whole phase, and it had to be settled. **The rule is
kept**, because this phase is the first where it really serves: the
thirteen rows above are written *before* the code and decide what will be
done, whereas the seven of phase 6, entered at the end, only described.
If it is not held this time either, it is the rule that will have to be withdrawn
and not the practice that will have to be excused.

## Manual

| Section | What changes |
| :------ | :----------- |
| `subedit-cli/inspect.md` | the deduction section, with a real example |
| `subedit-cli/shift.md` | `--to-grid` |
| `subedit-cli/snap.md` | the subcommand, **and its explicit contrast with `framerate`** (D11) |
| `subedit-cli/framerate.md` | the symmetrical pointer to `snap` |
| `subedit-gui/operations.md` | the two operations, and the conversion dialog that changes in nature |
| `subedit-gui/table.md` or a new section | the analysis dialog and the status bar |

**One manual question remains open, deferred by #198:** an example of
`inspect` needs a file timed on a grid, and the examples run
in an empty temporary directory where the input comes from a `printf`. Two
lines fit there, a thousand do not. Two ways — a shell loop in the
example, self-sufficient but showing a generator before showing a
command, or a versioned fixture, which shows a command on a file that
the reader does not have. **To be settled in the issue that writes the section**, with the
section in front of one's eyes.

## Split into issues

| Order | Issue | What it carries |
| ----: | :---- | :-------------- |
| 1 | #203 — Core: the deduction, its ranking and its verdict | D1 to D8 and D14, `core/analysis/`, the thresholds written once |
| 2 | #204 — Core: snapping the positions to a frame rate | `SnapCommand`, D10 and D11, idempotence and preserved order |
| 3 | #205 — Core: putting a file back on its grid | the shift whose amount comes from the measurement |
| 4 | #206 — CLI: `inspect` states the file's grid | D2, D5, D6, and the silent file that says so |
| 5 | #207 — CLI: snap, put back on the grid | the two subcommands, and the contrast with `framerate` |
| 6 | #208 — Interface: the verdict and the analysis dialog | the status bar, D7 in readable form, D12 |
| 7 | #209 — Interface: the measurement in the conversion dialog | D13, and the disagreement shown |
| 8 | #210 — Interface: snap, put back on the grid | the menu entries, undo |
| 9 | #211 — Interface: cosmetic fixes | to be listed before the closing |

#203 precedes all the others. #204 and #205 depend only on it and can
go in parallel; #206 and #207 depend on the three; #208, #209 and #210
depend on everything before. **#208 is the one that makes the phase visible** —
before it, the deduction exists only for whoever reads terminal output. #211
depends on nothing and fills up along the way.

## Deferrals

**Carried over by the end-of-phase review (#222)**, which checked that each one
designates something real — a section of the roadmap or an open
issue — rather than an intention.

| What is deferred | Where |
| :--------------- | :---- |
| finding the pair of a conversion made with the wrong frame rate | phase 10, whose section now names it |
| fixtures with realistic ends, and a file with no grid at all | phase 10, same section |
| moving the anomalies from `core/model/` to `core/analysis/` | #227 — a condition, not a date: the day a second analysis exists |
| batch processing for `snap` and script-readable output | phase 13, whose list now names snapping |
| the `Help ▸ Manual` entry, present and disabled | phase 7, with the packaging that decides where the manual lives |
| the window geometry, remembered from one session to the next | phase 7, with the persisted configuration |

**And four lines of work that came out of the critical review**, which are not framing
deferrals but observed debts:

| What was seen | Issue |
| :------------ | :---- |
| three callers gather the starts before deducing | #223 |
| the list of the eight frame rates is written twice | #224 |
| the thresholds are not tested at their boundary | #225 |
| the gate does not see the files the tests leave behind | #226 |

## Completion criteria

- [ ] The nine issues are closed
- [ ] The registry's requirements are `implemented` and cited by a test
- [ ] The manual sections exist and describe what exists
- [ ] The benchmarks are rerun, and the budget of D9 is held or renegotiated
- [ ] The end-of-phase review is done
