# Performance journal

What the benchmarks measured, version by version. Replayed in `Release`, the
shipped mode — it is the only one whose figures mean anything to a user.

**No alert threshold, deliberately.** The natural variance of the measurements
is not known: the same sort gave 240 µs then 456 µs an hour apart, on the same
machine and the same binary. Setting a ratchet on figures like that would
guarantee false alerts and the habit of ignoring them. The table of extremes
below exists so that this variance becomes known; the threshold will come when
it is. See [ADR 0015](../adr/0015-memoire-des-mesures.md).

## The machine's load, and why it is written down

A measurement taken while a browser compiles JavaScript says nothing about the
code: it says the state of the machine. Each record therefore carries the
**one-minute load average** read just before, in its header, and the rule that
follows is mechanical:

> **A record taken above `BENCH_MAX_LOAD` — one and a half, by default — does
> not enter the journal.** The benchmarks run, their figures are displayed, and
> nothing is written.

> **And a measurement more dispersed than usual sets no extreme**, even on a
> quiet machine: beyond **1.25 times its median spread**, it enters the record
> and stays out of the table.

### Two criteria, because one alone does not read the measurement — #202

The load threshold reads **the machine**. It does not read **the measurement**,
which is nevertheless the only thing the bench can say about its own
reliability — and it is there, in the "Standard deviation" column of each
record.

A record taken at load 1.44 — so admitted — set a permanent maximum on a ticket
that changed no `.cpp`: 3.4 ms for reading 4000 subtitles, against 3.28. Its
standard deviation was **26 % of its mean**, where that measurement usually sits
around 10 %. The measurement itself said it was worth nothing.

**The criterion is relative to each measurement, and it had to be.** The median
spread of the journal is 17 %, and it ranges from 3.8 % for scaling by a
rational to 43.6 % for a shift of four thousand subtitles. A single threshold at
10 % would refuse more than one measurement in two. What matters is not that a
measurement is dispersed, it is that it is **more dispersed than usual**.

**The factor is a quarter above the median**, and that is where the curve
turns. Measured on the 87 records of the journal, counting the maxima set and
those that the next five records contradict:

| Criterion | Maxima set | Contradicted |
| :-------- | ---------: | -----------: |
| load alone — the previous rule | 134 | **11** (8.2 %) |
| load, and spread ≤ 1.0 × its median | 119 | 4 |
| **load, and spread ≤ 1.25 × its median** | **126** | **4** (3.2 %) |
| load, and spread ≤ 1.5 × its median | 121 | 5 |
| load, and spread ≤ 2.0 × its median | 131 | 6 |

**The spike was not the right lens.** The instrument counted what exceeds its
neighbours by a half; a maximum is set by exceeding the previous one by a hair,
and it is permanent. The record of #202 was at ×1.15: never a spike, and yet an
envelope skewed forever. `analyse-bench-journal.py` now counts both.

**The second criterion adds to the first, it does not replace it.** Spread alone
is clearly worse — 24 spikes admitted against 5 — and the load is read
**before** measuring, which allows giving up; the spread is only known after.

**Below five records, a measurement has no median** and the criterion does not
apply: the load remains the only judge, and the rule of #189 already keeps new
measurements out of the table until the first quiet record.

### What the rule repaired behind it

A rule that tightens does not repair the past on its own: **a maximum only falls
before a higher maximum.** Twenty-four of the sixty extremes in the table had
been set by a record that both criteria refuse, and they were recomputed —
`src/scripts/recompute-bench-extremes.py`, which touches only an extreme whose
**originating record** is refused, and leaves those whose source has been
pruned.

Seven are in that case: neither judgeable nor replaceable, left as they are.
Saying so is better than recomputing on a truncated history.

`make bench` first waits for the load to come down, **thirty seconds at most**:
the most frequent cause is the target that precedes it, and the one-minute
average takes a minute to forget it. Past that delay it measures anyway — the
figures remain useful to whoever just ran the command — but it records nothing,
and says so.

### Why thirty seconds, and why write nothing — #270

Phase 7 gave thirteen records of which **six were above the threshold**. The
question asked was: is the threshold tenable in the prescribed sequence?

**What was measured, and what it corrected.** First idea, wrong: that the load
came from the two steps that precede `bench` in `check-local`. Read at the exit
of `install-check`, every five seconds, it peaks at 1.78 five seconds after the
end — the one-minute average lags behind what has just finished — and drops
back under the threshold in about twenty seconds. `e2e` and `install-check` are
short; they load almost nothing.

**What loads is what precedes the whole.** The prescribed sequence chains
`make check` then `make check-local`: a quarter of an hour of clang-tidy, of
tests under sanitizers and of coverage, immediately before. It is that
inheritance that `bench` finds, and not that of its two neighbours. Four real
runs, on the same day and on the same machine:

| Context of the run | Load at `bench` | Outcome |
| :----------------- | --------------: | :------ |
| `check-local` straight after `make check` | 2.78 | refused |
| same, a second time | 1.84 | refused |
| `check-local` alone, machine at rest | 1.38 | **recorded** |
| `check-local` during an active desktop session | 2.88 | refused |
| same, more loaded | 6.56 | refused |

The record of version `0.8.6` is the one in the third row, and it survived the
two refused runs that followed it: **a refusal does not write, so it does not
erase either** the clean measurement already in place.

The last two are not cases of the project: the processes at the top of `ps` did
not belong to this repository — a browser, a development environment, two
servers — and the load fell back to 0.64 as soon as the session calmed down,
without anything in the project having changed.

**The threshold is therefore tenable, and it is not what needed looking at.** A
measurement taken outside the tail of `make check`, on a machine that is ours
alone, passes. The six failures of phase 7 were one or the other of the two
causes above, and neither is cured by waiting longer: the tail of a quarter of
an hour of compilation and the desktop session both last longer than any delay
one would accept to pay on every run.

**Hence thirty seconds.** What can be caught up is caught up in twenty; what
cannot be caught up is not caught up in three minutes either. Waiting beyond
that was paying the delay only for the cases where it could do nothing.

**And recording without the right to set extremes was a half-measure in the
literal sense.** The journal has a section per version: a section that cannot
be compared to anything — since one does not know what, in it, comes from the
code — takes the place of the one that is missing, and **hides** the fact that
there was no measurement. A version without a section says so. Almost one
record in two of phase 7 was in that case.

### The bench's place in the sequence, settled — #307

#270 left this question open, saying it went beyond it: the bench is chained by
`check-local`, so just behind the only step that really heats the machine.
**Phase 8 measured what that costs**, and the end-of-phase review settled it.

| Phase | Records written | Refused for load |
| :---- | --------------: | ---------------: |
| 7 | 7 out of 13 | 6 |
| 8 | 3 out of 6 | 3 — loads 1.59 · 2.08 · 10.64 |

The first two loads of phase 8 are the tail of `make check`, the one #270 had
measured at 2.78 then 1.84 under the same conditions. The third is a desktop
session, which nothing in the project can cure.

**First decision: swapping the two targets**, `make check-local` first,
`make check` after — the bench finds a machine that nothing has heated, and it
is also the order that the principle "from the cheapest to the most expensive"
already wanted.

**It is not enough, and its first run said so.** Machine starting from 0.63,
new order: the bench read **2.21** and recorded nothing. The tail of
`make check` was therefore not the whole cause.

### The guard read the load after creating it

Two runs a minute apart, on the same day and on the same machine:

| Run | Load read | Outcome |
| :-- | --------: | :------ |
| full `check-local`, with a Release rebuild | 2.21 | refused |
| `make bench` alone, Release tree already up to date | **1.32** | **recorded** |

**The second started from a more loaded machine, and was recorded.** What
separates them is the Release build that the bench step launches itself, just
before asking whether the machine is free.

`/proc/loadavg` gives an average over **one minute**, that is, the past. Read
after a compilation, it answers about that compilation. **The threshold was
never wrong, nor the delay: the question was being asked at the only moment
when the guard could not answer.**

The reading therefore comes before the build. The guard now answers "was the
machine ours when we started", which is the right question: a competing load —
browser, development environment, the desktop session at 10.64 — is there
before as well as during. What it stops seeing is a load that would start
during the build, and it is the spread criterion that sees it, after the fact.

**Neither the threshold nor the delay moves.** No measurement calls them into
question. See [ADR 0015](../adr/0015-memoire-des-mesures.md) and `CLAUDE.md`.

The rule of the table of extremes does not move either: it still holds for the
only record that can still arrive with an unqualified load, that of a machine
without `/proc/loadavg`.

Without that rule, a maximum set by noise is **permanent** — the table is never
pruned — and makes the measurement blind to any regression smaller than that
noise.

**It happened.** The record of version `0.2.15` was taken under a load of 7.2
and set thirteen maxima at once, one of them at 3.4 times its usual value, for a
ticket that touched no measured path. The table was recomputed without it, from
the kept records — all the extremes it cited came from them, so nothing was lost
but a little precision: the records display only three significant digits.

**It had happened again for a new measurement, through another door**, and that
door has been closed since #189. A measurement without history set its two
extremes on the first record that carried it, **even taken above the
threshold**: there was nothing to compare, so nothing to refuse, and the rule
above never applied to what had just been born. The four measurements of
version `0.5.13` were born that way, under a load of 5.73 — then the record was
replayed at 2.75, replacing the version section without touching the table. The
extremes then cited, for a version present in the journal, figures it no longer
showed there: a maximum of 27.8 ms for opening a video where the kept record
shows 10.9. The table was corrected by hand, as it had been for `0.2.15`.

**Two rules came out of it**, and `verify-gates.sh` holds them:

- **a new measurement enters the table only on a clean record.** Taken on a busy
  machine, it is logged in the record and left out of the envelope; it will
  enter it on the first quiet record, and the script says so;
- **an extreme set by the record being replaced is taken from the current
  record**, better or worse. Its section disappears from the file; leaving it as
  it is would cite figures this version no longer shows there. When the current
  record is not clean, the extreme stays as it is and the script reports that it
  is no longer verifiable.

**Practical consequence: the envelope grows only on a free machine.** A record
taken while working on something else on the same machine is logged, dated,
comparable — but it does not touch the table. It is not a failure, and
`make check-local` is not troubled by it. To feed the envelope, run
`make bench` when the machine is not in use.

**The threshold itself is a heuristic**, and the load column exists to refine
it. It was in fact lowered from two to one and a half on the day it was set: the
record of version `0.3.5`, taken at 1.88 and so admitted, set a maximum of
853 µs for writing 4000 subtitles where the seventeen previous records ranged
from 492 to 641. Those taken under 1.4 set only minima.

This file is written by `make bench`: do not edit it by hand — except this
preface, which the script copies as it is.

## The text of the editing fixture changed once, in 0.3.9

The `core/edit` measurements bear on a generated document of four thousand
subtitles. **Its positions have never moved and will not move** — that is what
makes each figure comparable to its own past. Its **text** changed once: up to
version `0.3.8` the four thousand cues were the same string of fifty-three
bytes; since `0.3.9`, the text varies and one subtitle in five carries a
hearing-impaired mention, in the proportions real files give. The why and the
figures are in `src/test/bench/full_length_project.hpp`; what matters here is
that the document now weighs 122 KB of text instead of 212 KB.

**The effect on the measurements that do not bear on the text is below the
noise.** The two fixtures were measured on the same day, an hour apart, on a
machine at 0.6 load: reading 4000 subtitles, whose fixture did not move by a
byte, varied by 14 % between the two runs — more than any editing measurement.
The shift, measured at 11.3 then 8.67 µs, came back to 9.01 in the record of
`0.3.9`: 0.02 µs from its historical minimum.

**One measurement is the exception, and it is the only one that copies text.**
Removing one subtitle in two keeps what it removes, so it can be undone:
`Project::remove` copies each of the two thousand subtitles taken out, so their
strings. Half as many bytes to copy shows — its minimum went from 9.92 to
8.95 ms. It is not an improvement of the code, it is a lighter fixture: for this
measurement, the records before `0.3.9` and those after compare only to within a
tenth.

Nothing was pruned from the table for all that: the earlier envelope remains
true of what it measured.

## The table model scans for anomalies, in 0.4.18

Three measurements jump, and the jump is expected rather than suffered:

| Measure | Before | After |
| :------ | -----: | ----: |
| building the model over 4000 subtitles | 59 ns | ~8 µs |
| rafraîchir après un shifting 4000 subtitles | 26 ns | ~9 µs |
| editing a position cell | 579 ns | ~10 µs |

**The third row was added at the end-of-phase review**, which found it by
comparing the records: it had jumped by a factor of seventeen between `0.4.17`
and `0.4.18` without anything saying so, and this note announced two.

Issue #134 has the table mark the subtitles whose positions do not hold
together. The computation is `scanAnomalies`, which walks the whole document;
the model redoes it on construction and after **every change of position** —
that is what guarantees that a marking is never stale, including after an undo.

**The ratio is a hundred, the sum is eight microseconds.** The computation
happens once per operation, never per cell: opening a file costs 2.4 ms of
reading, in which these 8 µs weigh three thousandths; shifting four thousand
subtitles cost 7.7 µs and now costs twice that, which no eye tells apart from a
gesture.

**The third is the only one on the path of a keystroke**, and it deserves to be
looked at for itself: validating a position rescans the whole document, so a
cell edited in a file of four thousand subtitles now costs ten microseconds
instead of half of one. It is still a thousand times less than what a finger can
feel, and it is the price of a marking that is never stale. Editing a text has
not moved: a text enters no anomaly, and nothing is recomputed.

What could have been saved, and why it was not: making the computation lazy —
mark dirty, recompute at the first display — would bring these two measurements
back to their earlier value without changing anything in the program, since
the display always follows the operation. That would be optimising the test
bench.

What was saved: `data()` looks up the anomalies of a row by bisection and not
by scanning. No benchmark shows it — their fixture is sound, so the list is
empty — but on a badly damaged file, a scan would make each cell pay for the
number of anomalies in the document.

## Resetting costs what refreshing costs, in 0.4.20

A new measurement, asked for by
[ADR 0019](../adr/0019-table-en-adaptateur-mince.md) before phase 7 built on
it: **a structural change resets the model**, and the ADR sets as its trigger
for re-examination the moment a menu adds rows one by one — "a complete reset
for every row added would then be ridiculous".

The figure does not say what was expected:

| Measure | Order of magnitude |
| :------ | -----------------: |
| resetting the model after a row is removed | ~13 µs |
| rafraîchir après un shifting 4000 subtitles | ~12 µs |
| building the model over 4000 subtitles | ~10 µs |

**The three are the same figure**, and it is the `scanAnomalies` they share. On
the model side, resetting costs nothing more than refreshing: the
`beginResetModel` / `endResetModel` are two signals, the rest is the walk of the
document that every position operation already pays.

**What the measurement does not contain, and cannot contain: the view's
share.** A `QTableView` needs a `QApplication`, and the benchmark binary has
none — it takes Catch2's `main`, and the models it measures are `QObject`s. Yet
it is precisely the view that rebuilds everything, loses the selection and
redoes its layout.

The ADR's trigger will therefore not find its argument here. The measurement
does not answer the question: **it moves it, and says where it lives.** That is
the most honest thing one can expect of it before having a screen at hand.

## Two conversions that did nothing, in 0.9.5

Issues #314 and #318, in the same gesture: ICU converted in two cases where
there was nothing to convert.

**On writing, when the target is UTF-8**, the two converters are the same: the
text made the round trip through UTF-16 to come back as it had left. It is the
ordinary case — the great majority of files written.

| | before | after |
| :--- | ----: | ----: |
| writing 4000 subtitles | 852 µs | **~610 µs** |

**Nothing is lost by skipping this passage**, and it had to be checked rather
than assumed: the round trip was not a safety net. The stop callback is set on
the output converter, which refuses a character the target cannot write;
nothing was set on the input one, so that malformed incoming UTF-8 was replaced
by U+FFFD and written — silently. Copying it is no worse, and the model carries
only UTF-8 anyway.

**On reading, the file was decoded twice**: once by the detection, which cannot
answer without reading, and once by the reading itself. The detection now
returns the text it produced.

That gain **does not show on the bench**, and that is what must be known about
it: it is worth one decoding of the whole file, measured separately because the
spread of the reading record is larger than it at this size.

| Size | One decoding |
| :--- | -----------: |
| 4 000 subtitles, 359 KB | **212 µs** out of 2.7 ms of reading, i.e. 8 % |
| 40 000 subtitles, 3.6 MB | **2 195 µs** |

It is therefore proportional to the file, and that is the only thing that makes
it worth attention: at four thousand cues it disappears in the noise, at forty
thousand it is worth two milliseconds. #314 asked precisely that the question be
reopened by a measurement rather than by memory; here it is.

## Extremes

The minimum and the maximum ever recorded for each measurement. This table is
never pruned — it is what keeps the envelope when the records are erased. A
renamed or removed measurement keeps its extremes there indefinitely: nothing
here distinguishes a live entry from an orphan — pruning the orphans is not the
subject of this ticket.

<!-- extremes -->
| Measure | Minimum | Recorded on | Maximum | Recorded on |
| :----- | ------: | :-------- | ------: | :-------- |
| versionString | 29.2 ns | 0.5.9 — 2026-08-23 | 55.1 ns | 0.4.4 — 2026-08-17 |
| parse | 29.9 ns | 0.2.6 — 2026-08-13 | 67.3 ns | 0.9.18 — 2026-09-08 |
| format | 29.8 ns | 0.3.11 — 2026-08-15 | 68 ns | 0.9.17 — 2026-09-08 |
| position to frame | 6.32 ns | 0.12.23 — 2026-09-30 | 8.03 ns | 0.10.1 — 2026-09-11 |
| frame to position | 6.32 ns | 0.13.25 — 2026-10-05 | 12.7 ns | 0.10.14 — 2026-09-15 |
| scaling by an exact rational | 6.71 ns | 0.9.19 — 2026-09-08 | 8.3 ns | 0.10.14 — 2026-09-15 |
| reading 4000 subtitles | 1.99 ms | 1.0.0 — 2026-10-22 | 3.28 ms | 0.5.3 — 2026-08-22 |
| writing 4000 subtitles | 488 µs | 0.3.10 — 2026-08-15 | 825 µs | 0.9.3 — 2026-09-05 |
| shifting 4000 subtitles | 6.56 µs | 0.13.25 — 2026-10-05 | 11 µs | 0.9.18 — 2026-09-08 |
| shifting then undoing | 12.7 µs | 0.13.25 — 2026-10-05 | 20.7 µs | 0.2.6 — 2026-08-13 |
| transforming 4000 subtitles | 67.5 µs | 0.13.13 — 2026-10-02 | 94.5 µs | 0.9.18 — 2026-09-08 |
| frame rate conversion over 4000 subtitles | 66.2 µs | 0.11.7 — 2026-09-21 | 100 µs | 0.2.12 — 2026-08-14 |
| sorting 4000 subtitles in reverse | 196 µs | 0.3.2 — 2026-08-15 | 352 µs | 0.9.18 — 2026-09-08 |
| removing every other subtitle | 138 µs | 0.4.14 — 2026-08-21 | 12.5 ms | 0.3.3 — 2026-08-15 |
| inserting 100 empty subtitles in the middle | 48.2 µs | 0.7.5 — 2026-08-29 | 97.6 µs | 0.8.13 — 2026-09-02 |
| editing a text, through a session | 114 ns | 0.2.14 — 2026-08-14 | 228 ns | 0.8.6 — 2026-08-31 |
| removing mentions over 4000 subtitles | 853 µs | 0.4.21 — 2026-08-22 | 5.68 ms | 0.4.4 — 2026-08-17 |
| removal then undo | 213 µs | 0.4.14 — 2026-08-21 | 314 µs | 0.10.15 — 2026-09-16 |
| building the model over 4000 subtitles | 59.3 ns | 0.4.14 — 2026-08-21 | 16.4 µs | 0.9.13 — 2026-09-07 |
| a window of 40 rows, five columns | 14.7 µs | 0.4.15 — 2026-08-21 | 28.9 µs | 0.6.8 — 2026-08-26 |
| rafraîchir après un shifting 4000 subtitles | 26.4 ns | 0.4.15 — 2026-08-21 | 13.2 µs | 0.11.25 — 2026-09-25 |
| editing a text cell | 371 ns | 0.5.9 — 2026-08-23 | 496 ns | 0.9.22 — 2026-09-10 |
| editing a position cell | 8.09 µs | 0.12.7 — 2026-09-27 | 14.8 µs | 0.8.8 — 2026-09-01 |
| resetting the model after a row is removed | 7.23 µs | 0.9.20 — 2026-09-08 | 13.6 µs | 0.11.25 — 2026-09-25 |
| the current line, among 4000 subtitles | 4.71 µs | 0.8.13 — 2026-09-02 | 14.1 µs | 0.11.25 — 2026-09-25 |
| composing a two-line line | 108 ns | 0.15.20 — 2026-10-12 | 284 ns | 0.13.33 — 2026-10-06 |
| opening a video | 8.82 ms | 0.13.33 — 2026-10-06 | 223 ms | 0.15.0 — 2026-10-09 |
| seeking a position | 538 µs | 0.6.13 — 2026-08-27 | 5.64 ms | 0.14.19 — 2026-10-08 |
| frame rate deduction over 4000 subtitles | 366 µs | 0.7.15 — 2026-08-30 | 459 µs | 0.15.7 — 2026-10-09 |
| alignment over 4000 subtitles | 123 µs | 0.13.25 — 2026-10-05 | 160 µs | 0.15.18 — 2026-10-12 |
| italicising 4000 subtitles | 885 µs | 0.12.8 — 2026-09-27 | 1.39 ms | 0.10.10 — 2026-09-13 |
| replacing a frequent word over 4000 subtitles, plain text | 3.82 ms | 1.0.0 — 2026-10-22 | 4.78 ms | 0.12.19 — 2026-09-30 |
| replacing a frequent word over 4000 subtitles, regular expression | 3.97 ms | 1.0.0 — 2026-10-22 | 5.13 ms | 0.14.5 — 2026-10-06 |
| search with no result over 4000 subtitles | 2.59 ms | 1.0.0 — 2026-10-22 | 8.01 ms | 0.13.10 — 2026-10-01 |
| adjusting the durations of 4000 subtitles, four constraints | 1.2 ms | 0.10.17 — 2026-09-19 | 1.47 ms | 0.15.18 — 2026-10-12 |
| title case over 4000 subtitles | 17.6 ms | 0.15.15 — 2026-10-12 | 22 ms | 0.11.25 — 2026-09-25 |
| pasting 4000 texts | 964 µs | 0.10.17 — 2026-09-19 | 1.26 ms | 0.11.1 — 2026-09-19 |
| aligning a 4000-line translation, by position | 803 µs | 0.13.33 — 2026-10-06 | 1.1 ms | 0.11.25 — 2026-09-25 |
| aligning a 4000-line translation, by number | 782 µs | 0.12.7 — 2026-09-27 | 995 µs | 0.12.19 — 2026-09-30 |
| common French errors over 4000 subtitles | 152 ms | 1.0.0 — 2026-10-22 | 187 ms | 0.12.19 — 2026-09-30 |
| common English errors over 4000 subtitles | 169 ms | 0.15.13 — 2026-10-12 | 207 ms | 0.12.19 — 2026-09-30 |
| splitting 4000 subtitles, without a length cache | 107 ms | 1.0.0 — 2026-10-22 | 132 ms | 0.12.19 — 2026-09-30 |
| splitting 4000 subtitles, with a length cache | 111 ms | 0.15.37 — 2026-10-17 | 138 ms | 0.12.19 — 2026-09-30 |
| splitting 4000 subtitles, in ems, with a length cache | 175 ms | 0.15.15 — 2026-10-12 | 213 ms | 0.15.35 — 2026-10-16 |
| walk of 200 files in a tree of depth 3 | 552 µs | 0.15.43 — 2026-10-19 | 659 µs | 0.14.3 — 2026-10-06 |
| shift of 200 files of 20 subtitles | 31.5 ms | 0.15.24 — 2026-10-13 | 38.5 ms | 0.15.18 — 2026-10-12 |
| shift of one file of 4000 subtitles | 2.84 ms | 1.0.0 — 2026-10-22 | 3.6 ms | 0.13.10 — 2026-10-01 |
| rendering a frame of 640×360 | 208 µs | 0.14.5 — 2026-10-06 | 378 µs | 0.15.0 — 2026-10-09 |
| rendering a frame of 1280×720 | 724 µs | 0.14.7 — 2026-10-07 | 1.35 ms | 0.14.19 — 2026-10-08 |
| rendering a frame of 1920×1080 | 1.36 ms | 0.15.39 — 2026-10-18 | 2.66 ms | 0.15.0 — 2026-10-09 |
| seeking frame 0 (720p video) | 6.17 ms | 0.15.10 — 2026-10-10 | 7.81 ms | 0.14.19 — 2026-10-08 |
| seeking frame 100 (720p video) | 36.3 ms | 0.15.39 — 2026-10-18 | 53.2 ms | 0.14.19 — 2026-10-08 |
| seeking frame 249 (720p video) | 69.4 ms | 0.15.45 — 2026-10-20 | 125 ms | 0.14.19 — 2026-10-08 |
| step forward to frame 100 (720p video) | 36.1 ms | 0.15.39 — 2026-10-18 | 52.4 ms | 0.14.19 — 2026-10-08 |
| step forward to frame 249 (720p video) | 69.8 ms | 0.15.45 — 2026-10-20 | 120 ms | 0.15.35 — 2026-10-16 |
| step back to frame 99 (720p video) | 35.7 ms | 0.15.39 — 2026-10-18 | 49.5 ms | 0.14.19 — 2026-10-08 |
| step back to frame 248 (720p video) | 69.2 ms | 0.15.45 — 2026-10-20 | 115 ms | 0.15.18 — 2026-10-12 |
| converting a tagged line for the image | 815 ns | 0.15.5 — 2026-10-09 | 1.17 µs | 0.14.19 — 2026-10-08 |
| centring a row among 4000 subtitles | 1.06 µs | 0.15.5 — 2026-10-09 | 1.3 µs | 0.14.19 — 2026-10-08 |
| conversion search over 4000 subtitles | 361 µs | 0.15.5 — 2026-10-09 | 448 µs | 0.15.7 — 2026-10-09 |
| loading the French catalogue | 309 µs | 1.0.0 — 2026-10-22 | 338 µs | 0.16.0 — 2026-10-21 |
| loading the Russian catalogue | 349 µs | 1.0.0 — 2026-10-22 | 359 µs | 0.16.0 — 2026-10-21 |
| looking a message up in the French catalogue | 41.8 ns | 1.0.0 — 2026-10-22 | 45.5 ns | 0.16.0 — 2026-10-21 |

<!-- versionString min=29.1524 max=55.1 -->
<!-- parse min=29.9 max=67.348 -->
<!-- format min=29.8143 max=68.0218 -->
<!-- position to frame min=6.31843 max=8.0301 -->
<!-- frame to position min=6.31692 max=12.7175 -->
<!-- scaling by an exact rational min=6.70967 max=8.3022 -->
<!-- reading 4000 subtitles min=1987840.0 max=3280000.0 -->
<!-- writing 4000 subtitles min=488279.0 max=825490.0 -->
<!-- shifting 4000 subtitles min=6556.44 max=11029.2 -->
<!-- shifting then undoing min=12695.6 max=20700.0 -->
<!-- transforming 4000 subtitles min=67454.9 max=94496.9 -->
<!-- frame rate conversion over 4000 subtitles min=66238.2 max=100000.0 -->
<!-- sorting 4000 subtitles in reverse min=196000.0 max=351534.0 -->
<!-- removing every other subtitle min=137974.0 max=12500000.0 -->
<!-- inserting 100 empty subtitles in the middle min=48185.9 max=97640.4 -->
<!-- editing a text, through a session min=114.0 max=227.528 -->
<!-- removing mentions over 4000 subtitles min=853000.0 max=5676890.0 -->
<!-- removal then undo min=213280.0 max=314434.0 -->
<!-- building the model over 4000 subtitles min=59.2521 max=16365.4 -->
<!-- a window of 40 rows, five columns min=14699.7 max=28872.4 -->
<!-- rafraîchir après un shifting 4000 subtitles min=26.4 max=13151.9 -->
<!-- editing a text cell min=370.791 max=496.387 -->
<!-- editing a position cell min=8090.8 max=14758.2 -->
<!-- resetting the model after a row is removed min=7229.3 max=13625.8 -->
<!-- the current line, among 4000 subtitles min=4714.13 max=14131.7 -->
<!-- composing a two-line line min=108.361 max=284.466 -->
<!-- opening a video min=8822490.0 max=222615000.0 -->
<!-- seeking a position min=537819.0 max=5643350.0 -->
<!-- frame rate deduction over 4000 subtitles min=366479.0 max=458920.0 -->
<!-- alignment over 4000 subtitles min=122556.0 max=160422.0 -->
<!-- italicising 4000 subtitles min=885492.0 max=1386260.0 -->
<!-- replacing a frequent word over 4000 subtitles, plain text min=3819870.0 max=4776230.0 -->
<!-- replacing a frequent word over 4000 subtitles, regular expression min=3968110.0 max=5128860.0 -->
<!-- search with no result over 4000 subtitles min=2585270.0 max=8011860.0 -->
<!-- adjusting the durations of 4000 subtitles, four constraints min=1200330.0 max=1466530.0 -->
<!-- title case over 4000 subtitles min=17551600.0 max=21973100.0 -->
<!-- pasting 4000 texts min=964212.0 max=1259920.0 -->
<!-- aligning a 4000-line translation, by position min=802764.0 max=1096980.0 -->
<!-- aligning a 4000-line translation, by number min=781546.0 max=995207.0 -->
<!-- common French errors over 4000 subtitles min=152117000.0 max=186776000.0 -->
<!-- common English errors over 4000 subtitles min=168581000.0 max=206664000.0 -->
<!-- splitting 4000 subtitles, without a length cache min=106832000.0 max=131622000.0 -->
<!-- splitting 4000 subtitles, with a length cache min=111172000.0 max=138172000.0 -->
<!-- splitting 4000 subtitles, in ems, with a length cache min=174535000.0 max=212846000.0 -->
<!-- walk of 200 files in a tree of depth 3 min=552440.0 max=658528.0 -->
<!-- shift of 200 files of 20 subtitles min=31494800.0 max=38497100.0 -->
<!-- shift of one file of 4000 subtitles min=2838880.0 max=3596300.0 -->
<!-- rendering a frame of 640×360 min=208485.0 max=378356.0 -->
<!-- rendering a frame of 1280×720 min=723665.0 max=1349710.0 -->
<!-- rendering a frame of 1920×1080 min=1355130.0 max=2658770.0 -->
<!-- seeking frame 0 (720p video) min=6173300.0 max=7811380.0 -->
<!-- seeking frame 100 (720p video) min=36297400.0 max=53241100.0 -->
<!-- seeking frame 249 (720p video) min=69371500.0 max=124818000.0 -->
<!-- step forward to frame 100 (720p video) min=36118900.0 max=52446900.0 -->
<!-- step forward to frame 249 (720p video) min=69813100.0 max=119739000.0 -->
<!-- step back to frame 99 (720p video) min=35699900.0 max=49479600.0 -->
<!-- step back to frame 248 (720p video) min=69163900.0 max=114941000.0 -->
<!-- converting a tagged line for the image min=815.476 max=1172.95 -->
<!-- centring a row among 4000 subtitles min=1064.1 max=1300.8 -->
<!-- conversion search over 4000 subtitles min=361110.0 max=447822.0 -->
<!-- loading the French catalogue min=308806.0 max=338186.0 -->
<!-- loading the Russian catalogue min=349225.0 max=359166.0 -->
<!-- looking a message up in the French catalogue min=41.7997 max=45.4663 -->

## Records

One section per version. Records older than a month are pruned; their
extremes survive in the table above.

<!-- records -->

### 1.0.0 — 2026-10-22 — Release — load 1.44 — pace ×1.00

| Measure | Mean | Standard deviation |
| :----- | ------: | ---------: |
| the current line, among 4000 subtitles | 5.86 µs | 1.46 µs |
| converting a tagged line for the image | 818 ns | 38 ns |
| composing a two-line line | 136 ns | 1.67 ns |
| opening a video | 220 ms | 1.88 ms |
| seeking a position | 5.44 ms | 95.5 µs |
| rendering a frame of 640×360 | 307 µs | 20.6 µs |
| rendering a frame of 1280×720 | 822 µs | 43 µs |
| rendering a frame of 1920×1080 | 2.05 ms | 111 µs |
| seeking frame 0 (720p video) | 6.22 ms | 254 µs |
| seeking frame 100 (720p video) | 44.9 ms | 2.47 ms |
| seeking frame 249 (720p video) | 105 ms | 6.03 ms |
| step forward to frame 100 (720p video) | 46.9 ms | 3.39 ms |
| step forward to frame 249 (720p video) | 104 ms | 6.19 ms |
| step back to frame 99 (720p video) | 44.5 ms | 2.47 ms |
| step back to frame 248 (720p video) | 108 ms | 6.94 ms |
| building the model over 4000 subtitles | 9.62 µs | 3.3 µs |
| a window of 40 rows, five columns | 17.7 µs | 540 ns |
| rafraîchir après un shifting 4000 subtitles | 9.56 µs | 2.73 µs |
| resetting the model after a row is removed | 8.45 µs | 5.21 µs |
| editing a text cell | 458 ns | 66.4 ns |
| editing a position cell | 9.13 µs | 906 ns |
| centring a row among 4000 subtitles | 1.08 µs | 249 ns |
| splitting 4000 subtitles, in ems, with a length cache | 184 ms | 12 ms |
| versionString | 40 ns | 2.72 ns |
| parse | 36.5 ns | 3.33 ns |
| format | 38.2 ns | 9.1 ns |
| position to frame | 6.33 ns | 0.0781 ns |
| frame to position | 6.47 ns | 0.0682 ns |
| scaling by an exact rational | 6.72 ns | 0.0709 ns |
| splitting 4000 subtitles, without a length cache | 107 ms | 562 µs |
| splitting 4000 subtitles, with a length cache | 112 ms | 2.73 ms |
| common French errors over 4000 subtitles | 152 ms | 2.48 ms |
| common English errors over 4000 subtitles | 169 ms | 3.11 ms |
| loading the French catalogue | 309 µs | 2.41 µs |
| loading the Russian catalogue | 349 µs | 15.4 µs |
| looking a message up in the French catalogue | 41.8 ns | 1.19 ns |
| reading 4000 subtitles | 1.99 ms | 39.3 µs |
| writing 4000 subtitles | 506 µs | 20.6 µs |
| shifting 4000 subtitles | 6.78 µs | 3.08 µs |
| shifting then undoing | 16.6 µs | 10.2 µs |
| transforming 4000 subtitles | 81.5 µs | 6.56 µs |
| frame rate conversion over 4000 subtitles | 71.6 µs | 5.45 µs |
| alignment over 4000 subtitles | 130 µs | 6.04 µs |
| sorting 4000 subtitles in reverse | 310 µs | 15.6 µs |
| removing every other subtitle | 165 µs | 24.8 µs |
| removal then undo | 247 µs | 34.7 µs |
| inserting 100 empty subtitles in the middle | 62.2 µs | 23 µs |
| editing a text, through a session | 183 ns | 14.3 ns |
| removing mentions over 4000 subtitles | 1.17 ms | 28.8 µs |
| italicising 4000 subtitles | 921 µs | 50.2 µs |
| replacing a frequent word over 4000 subtitles, plain text | 3.82 ms | 117 µs |
| replacing a frequent word over 4000 subtitles, regular expression | 3.97 ms | 85.1 µs |
| search with no result over 4000 subtitles | 2.59 ms | 67.9 µs |
| adjusting the durations of 4000 subtitles, four constraints | 1.23 ms | 55.7 µs |
| title case over 4000 subtitles | 19.8 ms | 1.93 ms |
| pasting 4000 texts | 1.15 ms | 70.1 µs |
| aligning a 4000-line translation, by position | 977 µs | 80.7 µs |
| aligning a 4000-line translation, by number | 899 µs | 185 µs |
| frame rate deduction over 4000 subtitles | 426 µs | 11.3 µs |
| conversion search over 4000 subtitles | 377 µs | 38.5 µs |
| walk of 200 files in a tree of depth 3 | 565 µs | 28.9 µs |
| shift of 200 files of 20 subtitles | 33 ms | 1.89 ms |
| shift of one file of 4000 subtitles | 2.84 ms | 61.4 µs |

### 0.16.0 — 2026-10-21 — Release — load 1.49 — pace ×1.01

| Measure | Mean | Standard deviation |
| :----- | ------: | ---------: |
| the current line, among 4000 subtitles | 5.87 µs | 2.5 µs |
| converting a tagged line for the image | 816 ns | 21.3 ns |
| composing a two-line line | 155 ns | 2.7 ns |
| opening a video | 218 ms | 1.96 ms |
| seeking a position | 5.41 ms | 118 µs |
| rendering a frame of 640×360 | 320 µs | 26.1 µs |
| rendering a frame of 1280×720 | 884 µs | 49.1 µs |
| rendering a frame of 1920×1080 | 2.14 ms | 44.4 µs |
| seeking frame 0 (720p video) | 6.25 ms | 256 µs |
| seeking frame 100 (720p video) | 47.6 ms | 3.5 ms |
| seeking frame 249 (720p video) | 110 ms | 4.25 ms |
| step forward to frame 100 (720p video) | 46.8 ms | 3.58 ms |
| step forward to frame 249 (720p video) | 111 ms | 3.26 ms |
| step back to frame 99 (720p video) | 46.2 ms | 2.36 ms |
| step back to frame 248 (720p video) | 110 ms | 2.64 ms |
| building the model over 4000 subtitles | 12.7 µs | 5.05 µs |
| a window of 40 rows, five columns | 17.7 µs | 450 ns |
| rafraîchir après un shifting 4000 subtitles | 10.5 µs | 420 ns |
| resetting the model after a row is removed | 10.5 µs | 523 ns |
| editing a text cell | 416 ns | 220 ns |
| editing a position cell | 11.6 µs | 1.02 µs |
| centring a row among 4000 subtitles | 1.07 µs | 78.9 ns |
| splitting 4000 subtitles, in ems, with a length cache | 183 ms | 10.6 ms |
| versionString | 39.3 ns | 0.582 ns |
| parse | 37.3 ns | 2.91 ns |
| format | 36.6 ns | 4.3 ns |
| position to frame | 6.32 ns | 0.0776 ns |
| frame to position | 6.47 ns | 0.0677 ns |
| scaling by an exact rational | 6.71 ns | 0.0544 ns |
| splitting 4000 subtitles, without a length cache | 110 ms | 4.56 ms |
| splitting 4000 subtitles, with a length cache | 122 ms | 9.9 ms |
| common French errors over 4000 subtitles | 157 ms | 7.16 ms |
| common English errors over 4000 subtitles | 173 ms | 7.13 ms |
| loading the French catalogue | 338 µs | 37.4 µs |
| loading the Russian catalogue | 359 µs | 28.5 µs |
| looking a message up in the French catalogue | 45.5 ns | 7.47 ns |
| reading 4000 subtitles | 2.2 ms | 67.5 µs |
| writing 4000 subtitles | 659 µs | 194 µs |
| shifting 4000 subtitles | 7.74 µs | 2.9 µs |
| shifting then undoing | 14.9 µs | 6.53 µs |
| transforming 4000 subtitles | 78.3 µs | 18.6 µs |
| frame rate conversion over 4000 subtitles | 78.2 µs | 17 µs |
| alignment over 4000 subtitles | 155 µs | 13 µs |
| sorting 4000 subtitles in reverse | 312 µs | 17.7 µs |
| removing every other subtitle | 169 µs | 27.3 µs |
| removal then undo | 285 µs | 26 µs |
| inserting 100 empty subtitles in the middle | 63.9 µs | 13 µs |
| editing a text, through a session | 210 ns | 16.3 ns |
| removing mentions over 4000 subtitles | 1.27 ms | 99.1 µs |
| italicising 4000 subtitles | 1.01 ms | 84.5 µs |
| replacing a frequent word over 4000 subtitles, plain text | 3.97 ms | 239 µs |
| replacing a frequent word over 4000 subtitles, regular expression | 4.09 ms | 199 µs |
| search with no result over 4000 subtitles | 2.61 ms | 57.3 µs |
| adjusting the durations of 4000 subtitles, four constraints | 1.31 ms | 84.6 µs |
| title case over 4000 subtitles | 18.6 ms | 987 µs |
| pasting 4000 texts | 984 µs | 47.7 µs |
| aligning a 4000-line translation, by position | 843 µs | 93 µs |
| aligning a 4000-line translation, by number | 811 µs | 41.3 µs |
| frame rate deduction over 4000 subtitles | 380 µs | 27.1 µs |
| conversion search over 4000 subtitles | 367 µs | 16.8 µs |
| walk of 200 files in a tree of depth 3 | 558 µs | 34 µs |
| shift of 200 files of 20 subtitles | 34.5 ms | 1.42 ms |
| shift of one file of 4000 subtitles | 3.27 ms | 103 µs |

### 0.15.45 — 2026-10-20 — Release — load 1.40 — pace ×0.99

| Measure | Mean | Standard deviation |
| :----- | ------: | ---------: |
| the current line, among 4000 subtitles | 7.45 µs | 3.22 µs |
| converting a tagged line for the image | 822 ns | 42 ns |
| composing a two-line line | 158 ns | 4.94 ns |
| opening a video | 216 ms | 20.6 ms |
| seeking a position | 5.48 ms | 91.6 µs |
| rendering a frame of 640×360 | 258 µs | 28.4 µs |
| rendering a frame of 1280×720 | 657 µs | 81.8 µs |
| rendering a frame of 1920×1080 | 1.37 ms | 307 µs |
| seeking frame 0 (720p video) | 6.35 ms | 341 µs |
| seeking frame 100 (720p video) | 31.7 ms | 3.65 ms |
| seeking frame 249 (720p video) | 69.4 ms | 4.19 ms |
| step forward to frame 100 (720p video) | 32.4 ms | 3.48 ms |
| step forward to frame 249 (720p video) | 69.8 ms | 4.29 ms |
| step back to frame 99 (720p video) | 31.8 ms | 3.75 ms |
| step back to frame 248 (720p video) | 69.2 ms | 3.65 ms |
| building the model over 4000 subtitles | 8.35 µs | 4.03 µs |
| a window of 40 rows, five columns | 18.9 µs | 4.56 µs |
| rafraîchir après un shifting 4000 subtitles | 8.27 µs | 2.35 µs |
| resetting the model after a row is removed | 8.16 µs | 2.28 µs |
| editing a text cell | 401 ns | 60.9 ns |
| editing a position cell | 9.18 µs | 1.24 µs |
| centring a row among 4000 subtitles | 1.26 µs | 484 ns |
| splitting 4000 subtitles, in ems, with a length cache | 183 ms | 6.71 ms |
| versionString | 54.6 ns | 1.28 ns |
| parse | 44.3 ns | 3.17 ns |
| format | 42.3 ns | 1.78 ns |
| position to frame | 6.32 ns | 0.073 ns |
| frame to position | 7.42 ns | 0.0772 ns |
| scaling by an exact rational | 6.72 ns | 0.121 ns |
| splitting 4000 subtitles, without a length cache | 115 ms | 7.06 ms |
| splitting 4000 subtitles, with a length cache | 114 ms | 4.46 ms |
| common French errors over 4000 subtitles | 158 ms | 6.31 ms |
| common English errors over 4000 subtitles | 174 ms | 5.27 ms |
| reading 4000 subtitles | 2.14 ms | 165 µs |
| writing 4000 subtitles | 540 µs | 60.8 µs |
| shifting 4000 subtitles | 7.63 µs | 6.21 µs |
| shifting then undoing | 13.8 µs | 3.6 µs |
| transforming 4000 subtitles | 75.8 µs | 16.4 µs |
| frame rate conversion over 4000 subtitles | 79.4 µs | 18.6 µs |
| alignment over 4000 subtitles | 139 µs | 17.3 µs |
| sorting 4000 subtitles in reverse | 310 µs | 18.5 µs |
| removing every other subtitle | 180 µs | 21.2 µs |
| removal then undo | 284 µs | 53.1 µs |
| inserting 100 empty subtitles in the middle | 52.3 µs | 6.06 µs |
| editing a text, through a session | 192 ns | 45.5 ns |
| removing mentions over 4000 subtitles | 1.28 ms | 89.2 µs |
| italicising 4000 subtitles | 989 µs | 76.8 µs |
| replacing a frequent word over 4000 subtitles, plain text | 3.86 ms | 150 µs |
| replacing a frequent word over 4000 subtitles, regular expression | 4.05 ms | 171 µs |
| search with no result over 4000 subtitles | 2.78 ms | 219 µs |
| adjusting the durations of 4000 subtitles, four constraints | 1.27 ms | 88.2 µs |
| title case over 4000 subtitles | 18.1 ms | 960 µs |
| pasting 4000 texts | 993 µs | 58.4 µs |
| aligning a 4000-line translation, by position | 829 µs | 57.2 µs |
| aligning a 4000-line translation, by number | 813 µs | 51.5 µs |
| frame rate deduction over 4000 subtitles | 381 µs | 27.7 µs |
| conversion search over 4000 subtitles | 372 µs | 26.2 µs |
| walk of 200 files in a tree of depth 3 | 566 µs | 41.8 µs |
| shift of 200 files of 20 subtitles | 34.8 ms | 1.41 ms |
| shift of one file of 4000 subtitles | 3.19 ms | 348 µs |

### 0.15.43 — 2026-10-19 — Release — load 1.46 — pace ×0.99

| Measure | Mean | Standard deviation |
| :----- | ------: | ---------: |
| the current line, among 4000 subtitles | 7.7 µs | 3.6 µs |
| converting a tagged line for the image | 916 ns | 300 ns |
| composing a two-line line | 158 ns | 2.42 ns |
| opening a video | 219 ms | 1.18 ms |
| seeking a position | 5.41 ms | 131 µs |
| rendering a frame of 640×360 | 210 µs | 30.6 µs |
| rendering a frame of 1280×720 | 765 µs | 140 µs |
| rendering a frame of 1920×1080 | 1.36 ms | 231 µs |
| seeking frame 0 (720p video) | 6.74 ms | 869 µs |
| seeking frame 100 (720p video) | 38.6 ms | 2.88 ms |
| seeking frame 249 (720p video) | 83.5 ms | 7.83 ms |
| step forward to frame 100 (720p video) | 37.7 ms | 2.03 ms |
| step forward to frame 249 (720p video) | 82.5 ms | 9.9 ms |
| step back to frame 99 (720p video) | 38.1 ms | 6.02 ms |
| step back to frame 248 (720p video) | 81.3 ms | 7.79 ms |
| building the model over 4000 subtitles | 7.35 µs | 2.45 µs |
| a window of 40 rows, five columns | 20.9 µs | 1.29 µs |
| rafraîchir après un shifting 4000 subtitles | 7.44 µs | 2.67 µs |
| resetting the model after a row is removed | 8.18 µs | 3.25 µs |
| editing a text cell | 451 ns | 193 ns |
| editing a position cell | 9.48 µs | 3.86 µs |
| centring a row among 4000 subtitles | 1.09 µs | 226 ns |
| splitting 4000 subtitles, in ems, with a length cache | 187 ms | 6.33 ms |
| versionString | 44.9 ns | 0.497 ns |
| parse | 37.2 ns | 2.56 ns |
| format | 36.5 ns | 0.511 ns |
| position to frame | 7.42 ns | 1.12 ns |
| frame to position | 7.83 ns | 3.47 ns |
| scaling by an exact rational | 6.73 ns | 0.144 ns |
| splitting 4000 subtitles, without a length cache | 119 ms | 11.6 ms |
| splitting 4000 subtitles, with a length cache | 121 ms | 6.09 ms |
| common French errors over 4000 subtitles | 164 ms | 6.72 ms |
| common English errors over 4000 subtitles | 180 ms | 11.5 ms |
| reading 4000 subtitles | 2.12 ms | 316 µs |
| writing 4000 subtitles | 574 µs | 44.1 µs |
| shifting 4000 subtitles | 6.92 µs | 3.1 µs |
| shifting then undoing | 13.7 µs | 3.95 µs |
| transforming 4000 subtitles | 72.5 µs | 11 µs |
| frame rate conversion over 4000 subtitles | 81.3 µs | 10.1 µs |
| alignment over 4000 subtitles | 141 µs | 19.4 µs |
| sorting 4000 subtitles in reverse | 272 µs | 26.3 µs |
| removing every other subtitle | 163 µs | 14.4 µs |
| removal then undo | 245 µs | 36.1 µs |
| inserting 100 empty subtitles in the middle | 53.3 µs | 9.22 µs |
| editing a text, through a session | 184 ns | 35.3 ns |
| removing mentions over 4000 subtitles | 1.23 ms | 34.7 µs |
| italicising 4000 subtitles | 914 µs | 36.2 µs |
| replacing a frequent word over 4000 subtitles, plain text | 3.89 ms | 116 µs |
| replacing a frequent word over 4000 subtitles, regular expression | 4.11 ms | 303 µs |
| search with no result over 4000 subtitles | 2.71 ms | 288 µs |
| adjusting the durations of 4000 subtitles, four constraints | 1.26 ms | 172 µs |
| title case over 4000 subtitles | 18.2 ms | 996 µs |
| pasting 4000 texts | 1.01 ms | 165 µs |
| aligning a 4000-line translation, by position | 984 µs | 114 µs |
| aligning a 4000-line translation, by number | 988 µs | 174 µs |
| frame rate deduction over 4000 subtitles | 371 µs | 18.5 µs |
| conversion search over 4000 subtitles | 423 µs | 37.9 µs |
| walk of 200 files in a tree of depth 3 | 552 µs | 34.7 µs |
| shift of 200 files of 20 subtitles | 33.1 ms | 1.78 ms |
| shift of one file of 4000 subtitles | 3.02 ms | 238 µs |

### 0.15.39 — 2026-10-18 — Release — load 1.48 — pace ×0.96

| Measure | Mean | Standard deviation |
| :----- | ------: | ---------: |
| the current line, among 4000 subtitles | 4.96 µs | 1.72 µs |
| converting a tagged line for the image | 821 ns | 37.6 ns |
| composing a two-line line | 138 ns | 2 ns |
| opening a video | 219 ms | 1.35 ms |
| seeking a position | 5.42 ms | 87.1 µs |
| rendering a frame of 640×360 | 209 µs | 26.9 µs |
| rendering a frame of 1280×720 | 721 µs | 62.4 µs |
| rendering a frame of 1920×1080 | 1.36 ms | 94.5 µs |
| seeking frame 0 (720p video) | 6.52 ms | 632 µs |
| seeking frame 100 (720p video) | 36.3 ms | 1.85 ms |
| seeking frame 249 (720p video) | 77.8 ms | 4.32 ms |
| step forward to frame 100 (720p video) | 36.1 ms | 2.27 ms |
| step forward to frame 249 (720p video) | 77 ms | 2.62 ms |
| step back to frame 99 (720p video) | 35.7 ms | 2.17 ms |
| step back to frame 248 (720p video) | 86.7 ms | 22.8 ms |
| building the model over 4000 subtitles | 10.6 µs | 4.4 µs |
| a window of 40 rows, five columns | 21.7 µs | 4.35 µs |
| rafraîchir après un shifting 4000 subtitles | 10.2 µs | 6.65 µs |
| resetting the model after a row is removed | 9.02 µs | 3.46 µs |
| editing a text cell | 404 ns | 57.7 ns |
| editing a position cell | 9.07 µs | 306 ns |
| centring a row among 4000 subtitles | 1.17 µs | 505 ns |
| splitting 4000 subtitles, in ems, with a length cache | 182 ms | 5.37 ms |
| versionString | 44.2 ns | 0.415 ns |
| parse | 36.1 ns | 2.87 ns |
| format | 36.8 ns | 0.268 ns |
| position to frame | 6.65 ns | 1.47 ns |
| frame to position | 6.47 ns | 0.0655 ns |
| scaling by an exact rational | 6.72 ns | 0.0698 ns |
| splitting 4000 subtitles, without a length cache | 113 ms | 4.2 ms |
| splitting 4000 subtitles, with a length cache | 152 ms | 43.5 ms |
| common French errors over 4000 subtitles | 248 ms | 90.8 ms |
| common English errors over 4000 subtitles | 192 ms | 17.3 ms |
| reading 4000 subtitles | 2.54 ms | 127 µs |
| writing 4000 subtitles | 676 µs | 151 µs |
| shifting 4000 subtitles | 6.7 µs | 3.29 µs |
| shifting then undoing | 13.2 µs | 3.64 µs |
| transforming 4000 subtitles | 71.7 µs | 10.6 µs |
| frame rate conversion over 4000 subtitles | 77.2 µs | 9.19 µs |
| alignment over 4000 subtitles | 135 µs | 18 µs |
| sorting 4000 subtitles in reverse | 289 µs | 29.1 µs |
| removing every other subtitle | 178 µs | 23.6 µs |
| removal then undo | 246 µs | 31.4 µs |
| inserting 100 empty subtitles in the middle | 60.9 µs | 21.9 µs |
| editing a text, through a session | 184 ns | 17.5 ns |
| removing mentions over 4000 subtitles | 1.27 ms | 64.5 µs |
| italicising 4000 subtitles | 957 µs | 92.9 µs |
| replacing a frequent word over 4000 subtitles, plain text | 4.14 ms | 250 µs |
| replacing a frequent word over 4000 subtitles, regular expression | 4.59 ms | 468 µs |
| search with no result over 4000 subtitles | 2.78 ms | 156 µs |
| adjusting the durations of 4000 subtitles, four constraints | 1.28 ms | 61.1 µs |
| title case over 4000 subtitles | 18.8 ms | 929 µs |
| pasting 4000 texts | 1.08 ms | 79.2 µs |
| aligning a 4000-line translation, by position | 871 µs | 67.4 µs |
| aligning a 4000-line translation, by number | 910 µs | 36.5 µs |
| frame rate deduction over 4000 subtitles | 370 µs | 19.8 µs |
| conversion search over 4000 subtitles | 367 µs | 23.4 µs |
| walk of 200 files in a tree of depth 3 | 561 µs | 37.9 µs |
| shift of 200 files of 20 subtitles | 34.7 ms | 1.49 ms |
| shift of one file of 4000 subtitles | 3.56 ms | 423 µs |

### 0.15.38 — 2026-10-17 — Release — load 1.44 — pace ×1.04

| Measure | Mean | Standard deviation |
| :----- | ------: | ---------: |
| the current line, among 4000 subtitles | 7.38 µs | 1.63 µs |
| converting a tagged line for the image | 826 ns | 59.6 ns |
| composing a two-line line | 137 ns | 3.45 ns |
| opening a video | 220 ms | 2.22 ms |
| seeking a position | 5.39 ms | 235 µs |
| rendering a frame of 640×360 | 307 µs | 20.9 µs |
| rendering a frame of 1280×720 | 828 µs | 38.3 µs |
| rendering a frame of 1920×1080 | 2.33 ms | 255 µs |
| seeking frame 0 (720p video) | 6.54 ms | 623 µs |
| seeking frame 100 (720p video) | 51.2 ms | 4.41 ms |
| seeking frame 249 (720p video) | 112 ms | 8.38 ms |
| step forward to frame 100 (720p video) | 48.8 ms | 4.1 ms |
| step forward to frame 249 (720p video) | 110 ms | 8.18 ms |
| step back to frame 99 (720p video) | 46.7 ms | 4.32 ms |
| step back to frame 248 (720p video) | 112 ms | 14.8 ms |
| building the model over 4000 subtitles | 8.46 µs | 5.35 µs |
| a window of 40 rows, five columns | 18 µs | 1.21 µs |
| rafraîchir après un shifting 4000 subtitles | 8.92 µs | 3.13 µs |
| resetting the model after a row is removed | 8.22 µs | 3.02 µs |
| editing a text cell | 392 ns | 23.3 ns |
| editing a position cell | 9.67 µs | 3.56 µs |
| centring a row among 4000 subtitles | 1.26 µs | 264 ns |
| splitting 4000 subtitles, in ems, with a length cache | 193 ms | 14.7 ms |
| versionString | 51.5 ns | 0.669 ns |
| parse | 36.9 ns | 4.73 ns |
| format | 38.7 ns | 9.47 ns |
| position to frame | 9.26 ns | 2.8 ns |
| frame to position | 6.64 ns | 1.06 ns |
| scaling by an exact rational | 7.7 ns | 0.113 ns |
| splitting 4000 subtitles, without a length cache | 113 ms | 8.32 ms |
| splitting 4000 subtitles, with a length cache | 124 ms | 7.87 ms |
| common French errors over 4000 subtitles | 162 ms | 13.3 ms |
| common English errors over 4000 subtitles | 171 ms | 10.1 ms |
| reading 4000 subtitles | 2.17 ms | 285 µs |
| writing 4000 subtitles | 518 µs | 24.9 µs |
| shifting 4000 subtitles | 6.96 µs | 3.24 µs |
| shifting then undoing | 13.5 µs | 3.44 µs |
| transforming 4000 subtitles | 71.9 µs | 9.66 µs |
| frame rate conversion over 4000 subtitles | 76.9 µs | 11.3 µs |
| alignment over 4000 subtitles | 156 µs | 23.4 µs |
| sorting 4000 subtitles in reverse | 270 µs | 16.1 µs |
| removing every other subtitle | 167 µs | 31.6 µs |
| removal then undo | 295 µs | 64.8 µs |
| inserting 100 empty subtitles in the middle | 61.3 µs | 17.7 µs |
| editing a text, through a session | 204 ns | 44.1 ns |
| removing mentions over 4000 subtitles | 1.22 ms | 26.2 µs |
| italicising 4000 subtitles | 1.13 ms | 136 µs |
| replacing a frequent word over 4000 subtitles, plain text | 4.47 ms | 373 µs |
| replacing a frequent word over 4000 subtitles, regular expression | 4.45 ms | 385 µs |
| search with no result over 4000 subtitles | 2.62 ms | 100 µs |
| adjusting the durations of 4000 subtitles, four constraints | 1.34 ms | 96.2 µs |
| title case over 4000 subtitles | 19.4 ms | 1.84 ms |
| pasting 4000 texts | 982 µs | 37.9 µs |
| aligning a 4000-line translation, by position | 1.02 ms | 108 µs |
| aligning a 4000-line translation, by number | 952 µs | 120 µs |
| frame rate deduction over 4000 subtitles | 368 µs | 14.3 µs |
| conversion search over 4000 subtitles | 366 µs | 25.1 µs |
| walk of 200 files in a tree of depth 3 | 628 µs | 95.2 µs |
| shift of 200 files of 20 subtitles | 34.4 ms | 3.16 ms |
| shift of one file of 4000 subtitles | 3.58 ms | 522 µs |

### 0.15.37 — 2026-10-17 — Release — load 1.40 — pace ×0.87

| Measure | Mean | Standard deviation |
| :----- | ------: | ---------: |
| the current line, among 4000 subtitles | 5.62 µs | 408 ns |
| converting a tagged line for the image | 831 ns | 32.3 ns |
| composing a two-line line | 139 ns | 1.6 ns |
| opening a video | 219 ms | 1.58 ms |
| seeking a position | 5.34 ms | 75.7 µs |
| rendering a frame of 640×360 | 316 µs | 24 µs |
| rendering a frame of 1280×720 | 913 µs | 42.9 µs |
| rendering a frame of 1920×1080 | 2.03 ms | 128 µs |
| seeking frame 0 (720p video) | 6.2 ms | 246 µs |
| seeking frame 100 (720p video) | 43.5 ms | 2.1 ms |
| seeking frame 249 (720p video) | 100 ms | 6.04 ms |
| step forward to frame 100 (720p video) | 43.5 ms | 2.49 ms |
| step forward to frame 249 (720p video) | 102 ms | 6.59 ms |
| step back to frame 99 (720p video) | 43.4 ms | 2.12 ms |
| step back to frame 248 (720p video) | 101 ms | 5.85 ms |
| building the model over 4000 subtitles | 10.5 µs | 531 ns |
| a window of 40 rows, five columns | 17.9 µs | 244 ns |
| rafraîchir après un shifting 4000 subtitles | 10.4 µs | 133 ns |
| resetting the model after a row is removed | 10.4 µs | 339 ns |
| editing a text cell | 400 ns | 29.2 ns |
| editing a position cell | 11.5 µs | 178 ns |
| centring a row among 4000 subtitles | 1.08 µs | 149 ns |
| splitting 4000 subtitles, in ems, with a length cache | 175 ms | 6.8 ms |
| versionString | 45.2 ns | 0.509 ns |
| parse | 35.9 ns | 2.84 ns |
| format | 36.7 ns | 0.451 ns |
| position to frame | 6.32 ns | 0.0674 ns |
| frame to position | 6.47 ns | 0.0679 ns |
| scaling by an exact rational | 6.71 ns | 0.0524 ns |
| splitting 4000 subtitles, without a length cache | 107 ms | 1.3 ms |
| splitting 4000 subtitles, with a length cache | 111 ms | 5.08 ms |
| common French errors over 4000 subtitles | 153 ms | 3.32 ms |
| common English errors over 4000 subtitles | 171 ms | 6.32 ms |
| reading 4000 subtitles | 2.05 ms | 54.6 µs |
| writing 4000 subtitles | 518 µs | 12 µs |
| shifting 4000 subtitles | 8.11 µs | 3.38 µs |
| shifting then undoing | 15.4 µs | 3.32 µs |
| transforming 4000 subtitles | 76 µs | 11.7 µs |
| frame rate conversion over 4000 subtitles | 77.6 µs | 6.49 µs |
| alignment over 4000 subtitles | 135 µs | 5.53 µs |
| sorting 4000 subtitles in reverse | 282 µs | 27.8 µs |
| removing every other subtitle | 161 µs | 15.7 µs |
| removal then undo | 246 µs | 26.4 µs |
| inserting 100 empty subtitles in the middle | 55.7 µs | 14.8 µs |
| editing a text, through a session | 184 ns | 22.9 ns |
| removing mentions over 4000 subtitles | 1.26 ms | 69.5 µs |
| italicising 4000 subtitles | 910 µs | 25.2 µs |
| replacing a frequent word over 4000 subtitles, plain text | 3.86 ms | 89.5 µs |
| replacing a frequent word over 4000 subtitles, regular expression | 4.03 ms | 71.6 µs |
| search with no result over 4000 subtitles | 2.93 ms | 388 µs |
| adjusting the durations of 4000 subtitles, four constraints | 1.24 ms | 35.6 µs |
| title case over 4000 subtitles | 17.8 ms | 252 µs |
| pasting 4000 texts | 1.04 ms | 31.3 µs |
| aligning a 4000-line translation, by position | 829 µs | 34.7 µs |
| aligning a 4000-line translation, by number | 807 µs | 34.5 µs |
| frame rate deduction over 4000 subtitles | 367 µs | 1.09 µs |
| conversion search over 4000 subtitles | 362 µs | 19.4 µs |
| walk of 200 files in a tree of depth 3 | 563 µs | 17.7 µs |
| shift of 200 files of 20 subtitles | 31.8 ms | 1.25 ms |
| shift of one file of 4000 subtitles | 2.9 ms | 53.8 µs |

### 0.15.35 — 2026-10-16 — Release — load 1.47 — pace ×1.48

| Measure | Mean | Standard deviation |
| :----- | ------: | ---------: |
| the current line, among 4000 subtitles | 7.44 µs | 5.11 µs |
| converting a tagged line for the image | 1.02 µs | 49.7 ns |
| composing a two-line line | 161 ns | 42.8 ns |
| opening a video | 226 ms | 25.3 ms |
| seeking a position | 5.33 ms | 552 µs |
| rendering a frame of 640×360 | 395 µs | 106 µs |
| rendering a frame of 1280×720 | 1.02 ms | 153 µs |
| rendering a frame of 1920×1080 | 2.41 ms | 204 µs |
| seeking frame 0 (720p video) | 7.07 ms | 1.78 ms |
| seeking frame 100 (720p video) | 52 ms | 3.57 ms |
| seeking frame 249 (720p video) | 123 ms | 14.7 ms |
| step forward to frame 100 (720p video) | 53.5 ms | 6.45 ms |
| step forward to frame 249 (720p video) | 120 ms | 7.61 ms |
| step back to frame 99 (720p video) | 74.3 ms | 65 ms |
| step back to frame 248 (720p video) | 142 ms | 84.5 ms |
| building the model over 4000 subtitles | 13.2 µs | 3.39 µs |
| a window of 40 rows, five columns | 24 µs | 8.92 µs |
| rafraîchir après un shifting 4000 subtitles | 12.4 µs | 3.76 µs |
| resetting the model after a row is removed | 13 µs | 2.9 µs |
| editing a text cell | 474 ns | 69.5 ns |
| editing a position cell | 14.6 µs | 2.32 µs |
| centring a row among 4000 subtitles | 1.32 µs | 472 ns |
| splitting 4000 subtitles, in ems, with a length cache | 213 ms | 5.16 ms |
| versionString | 52.4 ns | 4.03 ns |
| parse | 43.3 ns | 2.51 ns |
| format | 45.1 ns | 12.8 ns |
| position to frame | 9.75 ns | 4.81 ns |
| frame to position | 7.64 ns | 0.21 ns |
| scaling by an exact rational | 8.16 ns | 0.571 ns |
| splitting 4000 subtitles, without a length cache | 131 ms | 10.6 ms |
| splitting 4000 subtitles, with a length cache | 132 ms | 7.36 ms |
| common French errors over 4000 subtitles | 185 ms | 13.2 ms |
| common English errors over 4000 subtitles | 207 ms | 13.9 ms |
| reading 4000 subtitles | 2.61 ms | 450 µs |
| writing 4000 subtitles | 847 µs | 322 µs |
| shifting 4000 subtitles | 28.6 µs | 20.8 µs |
| shifting then undoing | 47.2 µs | 49.9 µs |
| transforming 4000 subtitles | 294 µs | 474 µs |
| frame rate conversion over 4000 subtitles | 184 µs | 122 µs |
| alignment over 4000 subtitles | 361 µs | 125 µs |
| sorting 4000 subtitles in reverse | 1.18 ms | 756 µs |
| removing every other subtitle | 502 µs | 186 µs |
| removal then undo | 1.17 ms | 944 µs |
| inserting 100 empty subtitles in the middle | 314 µs | 416 µs |
| editing a text, through a session | 647 ns | 256 ns |
| removing mentions over 4000 subtitles | 6.51 ms | 3.73 ms |
| italicising 4000 subtitles | 4.15 ms | 1.62 ms |
| replacing a frequent word over 4000 subtitles, plain text | 11.5 ms | 1.29 ms |
| replacing a frequent word over 4000 subtitles, regular expression | 12.3 ms | 1.96 ms |
| search with no result over 4000 subtitles | 9.16 ms | 3.91 ms |
| adjusting the durations of 4000 subtitles, four constraints | 2.23 ms | 849 µs |
| title case over 4000 subtitles | 41.9 ms | 18.6 ms |
| pasting 4000 texts | 1.51 ms | 421 µs |
| aligning a 4000-line translation, by position | 1.22 ms | 201 µs |
| aligning a 4000-line translation, by number | 1.84 ms | 474 µs |
| frame rate deduction over 4000 subtitles | 743 µs | 167 µs |
| conversion search over 4000 subtitles | 564 µs | 57.4 µs |
| walk of 200 files in a tree of depth 3 | 1.05 ms | 330 µs |
| shift of 200 files of 20 subtitles | 53.8 ms | 11.7 ms |
| shift of one file of 4000 subtitles | 3.77 ms | 757 µs |

### 0.15.24 — 2026-10-13 — Release — load 1.43 — pace ×0.99

| Measure | Mean | Standard deviation |
| :----- | ------: | ---------: |
| the current line, among 4000 subtitles | 5.68 µs | 1.56 µs |
| converting a tagged line for the image | 830 ns | 27.3 ns |
| composing a two-line line | 110 ns | 2.36 ns |
| opening a video | 219 ms | 1.6 ms |
| seeking a position | 5.41 ms | 95.5 µs |
| rendering a frame of 640×360 | 311 µs | 23.2 µs |
| rendering a frame of 1280×720 | 855 µs | 53.6 µs |
| rendering a frame of 1920×1080 | 2.08 ms | 108 µs |
| seeking frame 0 (720p video) | 6.34 ms | 370 µs |
| seeking frame 100 (720p video) | 45.6 ms | 3.14 ms |
| seeking frame 249 (720p video) | 104 ms | 6.41 ms |
| step forward to frame 100 (720p video) | 45.3 ms | 2.81 ms |
| step forward to frame 249 (720p video) | 105 ms | 6.38 ms |
| step back to frame 99 (720p video) | 44.1 ms | 2.51 ms |
| step back to frame 248 (720p video) | 105 ms | 6.06 ms |
| building the model over 4000 subtitles | 8.15 µs | 1.3 µs |
| a window of 40 rows, five columns | 17.9 µs | 880 ns |
| rafraîchir après un shifting 4000 subtitles | 8.03 µs | 406 ns |
| resetting the model after a row is removed | 8.3 µs | 3.19 µs |
| editing a text cell | 386 ns | 13.9 ns |
| editing a position cell | 9.05 µs | 450 ns |
| centring a row among 4000 subtitles | 1.13 µs | 315 ns |
| splitting 4000 subtitles, in ems, with a length cache | 178 ms | 5.42 ms |
| versionString | 45.3 ns | 1.08 ns |
| parse | 43.2 ns | 1.15 ns |
| format | 37 ns | 6.55 ns |
| position to frame | 6.77 ns | 1.57 ns |
| frame to position | 6.32 ns | 0.0643 ns |
| scaling by an exact rational | 6.72 ns | 0.0719 ns |
| splitting 4000 subtitles, without a length cache | 110 ms | 4.13 ms |
| splitting 4000 subtitles, with a length cache | 113 ms | 4.82 ms |
| common French errors over 4000 subtitles | 155 ms | 4.12 ms |
| common English errors over 4000 subtitles | 171 ms | 4.32 ms |
| reading 4000 subtitles | 2.04 ms | 49.3 µs |
| writing 4000 subtitles | 562 µs | 64.6 µs |
| shifting 4000 subtitles | 6.84 µs | 3.55 µs |
| shifting then undoing | 13.2 µs | 3.54 µs |
| transforming 4000 subtitles | 75.5 µs | 10.8 µs |
| frame rate conversion over 4000 subtitles | 75 µs | 7.65 µs |
| alignment over 4000 subtitles | 133 µs | 9.7 µs |
| sorting 4000 subtitles in reverse | 268 µs | 20.7 µs |
| removing every other subtitle | 162 µs | 19.5 µs |
| removal then undo | 245 µs | 33.6 µs |
| inserting 100 empty subtitles in the middle | 53.2 µs | 14.9 µs |
| editing a text, through a session | 184 ns | 83.9 ns |
| removing mentions over 4000 subtitles | 1.29 ms | 92 µs |
| italicising 4000 subtitles | 911 µs | 57.6 µs |
| replacing a frequent word over 4000 subtitles, plain text | 3.98 ms | 234 µs |
| replacing a frequent word over 4000 subtitles, regular expression | 4.15 ms | 228 µs |
| search with no result over 4000 subtitles | 2.73 ms | 330 µs |
| adjusting the durations of 4000 subtitles, four constraints | 1.24 ms | 55.6 µs |
| title case over 4000 subtitles | 17.8 ms | 519 µs |
| pasting 4000 texts | 983 µs | 38.7 µs |
| aligning a 4000-line translation, by position | 817 µs | 48 µs |
| aligning a 4000-line translation, by number | 809 µs | 46.2 µs |
| frame rate deduction over 4000 subtitles | 374 µs | 24.5 µs |
| conversion search over 4000 subtitles | 367 µs | 14.9 µs |
| walk of 200 files in a tree of depth 3 | 560 µs | 36.9 µs |
| shift of 200 files of 20 subtitles | 31.5 ms | 1.23 ms |
| shift of one file of 4000 subtitles | 3.35 ms | 584 µs |

### 0.15.22 — 2026-10-13 — Release — load 1.39 — pace ×1.12

| Measure | Mean | Standard deviation |
| :----- | ------: | ---------: |
| the current line, among 4000 subtitles | 5.8 µs | 1.64 µs |
| converting a tagged line for the image | 826 ns | 31.2 ns |
| composing a two-line line | 128 ns | 8.09 ns |
| opening a video | 219 ms | 18.9 ms |
| seeking a position | 5.33 ms | 97.3 µs |
| rendering a frame of 640×360 | 352 µs | 58.2 µs |
| rendering a frame of 1280×720 | 962 µs | 106 µs |
| rendering a frame of 1920×1080 | 2.35 ms | 187 µs |
| seeking frame 0 (720p video) | 6.33 ms | 399 µs |
| seeking frame 100 (720p video) | 50.6 ms | 4.12 ms |
| seeking frame 249 (720p video) | 121 ms | 12.9 ms |
| step forward to frame 100 (720p video) | 51 ms | 8.39 ms |
| step forward to frame 249 (720p video) | 135 ms | 27.4 ms |
| step back to frame 99 (720p video) | 51.4 ms | 5.83 ms |
| step back to frame 248 (720p video) | 118 ms | 12.6 ms |
| building the model over 4000 subtitles | 11.4 µs | 4.2 µs |
| a window of 40 rows, five columns | 20.1 µs | 986 ns |
| rafraîchir après un shifting 4000 subtitles | 13.1 µs | 3.27 µs |
| resetting the model after a row is removed | 12.7 µs | 3.44 µs |
| editing a text cell | 514 ns | 339 ns |
| editing a position cell | 14.1 µs | 3.16 µs |
| centring a row among 4000 subtitles | 1.26 µs | 269 ns |
| splitting 4000 subtitles, in ems, with a length cache | 207 ms | 15.2 ms |
| versionString | 54.9 ns | 15.3 ns |
| parse | 44.2 ns | 11.1 ns |
| format | 39.2 ns | 0.58 ns |
| position to frame | 7.25 ns | 0.0692 ns |
| frame to position | 7.42 ns | 0.132 ns |
| scaling by an exact rational | 8.8 ns | 3.06 ns |
| splitting 4000 subtitles, without a length cache | 128 ms | 5.05 ms |
| splitting 4000 subtitles, with a length cache | 127 ms | 8.4 ms |
| common French errors over 4000 subtitles | 178 ms | 10.3 ms |
| common English errors over 4000 subtitles | 195 ms | 10.5 ms |
| reading 4000 subtitles | 2.29 ms | 70.5 µs |
| writing 4000 subtitles | 604 µs | 68.4 µs |
| shifting 4000 subtitles | 9.47 µs | 3.1 µs |
| shifting then undoing | 18.1 µs | 2.47 µs |
| transforming 4000 subtitles | 76.7 µs | 13.3 µs |
| frame rate conversion over 4000 subtitles | 89.4 µs | 10.7 µs |
| alignment over 4000 subtitles | 153 µs | 9.61 µs |
| sorting 4000 subtitles in reverse | 344 µs | 42.4 µs |
| removing every other subtitle | 196 µs | 26.9 µs |
| removal then undo | 280 µs | 18.2 µs |
| inserting 100 empty subtitles in the middle | 61.1 µs | 21.8 µs |
| editing a text, through a session | 225 ns | 34.4 ns |
| removing mentions over 4000 subtitles | 1.4 ms | 53.4 µs |
| italicising 4000 subtitles | 1.02 ms | 68.6 µs |
| replacing a frequent word over 4000 subtitles, plain text | 4.48 ms | 212 µs |
| replacing a frequent word over 4000 subtitles, regular expression | 4.84 ms | 465 µs |
| search with no result over 4000 subtitles | 2.81 ms | 253 µs |
| adjusting the durations of 4000 subtitles, four constraints | 1.41 ms | 78.2 µs |
| title case over 4000 subtitles | 20.1 ms | 1.34 ms |
| pasting 4000 texts | 1.13 ms | 43.5 µs |
| aligning a 4000-line translation, by position | 946 µs | 37.4 µs |
| aligning a 4000-line translation, by number | 954 µs | 46.1 µs |
| frame rate deduction over 4000 subtitles | 424 µs | 50.2 µs |
| conversion search over 4000 subtitles | 421 µs | 11.7 µs |
| walk of 200 files in a tree of depth 3 | 677 µs | 97.4 µs |
| shift of 200 files of 20 subtitles | 37.1 ms | 1.82 ms |
| shift of one file of 4000 subtitles | 3.17 ms | 205 µs |

### 0.15.20 — 2026-10-12 — Release — load 1.49 — pace ×1.00

| Measure | Mean | Standard deviation |
| :----- | ------: | ---------: |
| the current line, among 4000 subtitles | 7.26 µs | 175 ns |
| converting a tagged line for the image | 825 ns | 22.9 ns |
| composing a two-line line | 108 ns | 2.68 ns |
| opening a video | 219 ms | 1.91 ms |
| seeking a position | 5.39 ms | 91 µs |
| rendering a frame of 640×360 | 311 µs | 23.7 µs |
| rendering a frame of 1280×720 | 841 µs | 58.1 µs |
| rendering a frame of 1920×1080 | 2.06 ms | 110 µs |
| seeking frame 0 (720p video) | 6.21 ms | 285 µs |
| seeking frame 100 (720p video) | 47.3 ms | 2.31 ms |
| seeking frame 249 (720p video) | 111 ms | 3.47 ms |
| step forward to frame 100 (720p video) | 47 ms | 3.09 ms |
| step forward to frame 249 (720p video) | 111 ms | 1.96 ms |
| step back to frame 99 (720p video) | 47.2 ms | 1.91 ms |
| step back to frame 248 (720p video) | 111 ms | 2.67 ms |
| building the model over 4000 subtitles | 8.2 µs | 2.26 µs |
| a window of 40 rows, five columns | 19.8 µs | 7.05 µs |
| rafraîchir après un shifting 4000 subtitles | 8.06 µs | 800 ns |
| resetting the model after a row is removed | 8.03 µs | 643 ns |
| editing a text cell | 404 ns | 55.1 ns |
| editing a position cell | 9.15 µs | 1.64 µs |
| centring a row among 4000 subtitles | 1.15 µs | 460 ns |
| splitting 4000 subtitles, in ems, with a length cache | 180 ms | 8.36 ms |
| versionString | 44.9 ns | 0.437 ns |
| parse | 35.8 ns | 3.08 ns |
| format | 32.6 ns | 1.67 ns |
| position to frame | 6.56 ns | 1.24 ns |
| frame to position | 6.46 ns | 0.0486 ns |
| scaling by an exact rational | 7.37 ns | 2.6 ns |
| splitting 4000 subtitles, without a length cache | 111 ms | 3.84 ms |
| splitting 4000 subtitles, with a length cache | 119 ms | 7.78 ms |
| common French errors over 4000 subtitles | 160 ms | 10 ms |
| common English errors over 4000 subtitles | 174 ms | 7.78 ms |
| reading 4000 subtitles | 2.02 ms | 84.7 µs |
| writing 4000 subtitles | 530 µs | 55.2 µs |
| shifting 4000 subtitles | 8.26 µs | 1.94 µs |
| shifting then undoing | 15.4 µs | 2.74 µs |
| transforming 4000 subtitles | 70.9 µs | 5.8 µs |
| frame rate conversion over 4000 subtitles | 75.7 µs | 12.7 µs |
| alignment over 4000 subtitles | 134 µs | 17 µs |
| sorting 4000 subtitles in reverse | 275 µs | 25.8 µs |
| removing every other subtitle | 162 µs | 21.4 µs |
| removal then undo | 246 µs | 31.8 µs |
| inserting 100 empty subtitles in the middle | 61.4 µs | 17.8 µs |
| editing a text, through a session | 183 ns | 17.9 ns |
| removing mentions over 4000 subtitles | 1.25 ms | 88 µs |
| italicising 4000 subtitles | 917 µs | 45.7 µs |
| replacing a frequent word over 4000 subtitles, plain text | 3.94 ms | 260 µs |
| replacing a frequent word over 4000 subtitles, regular expression | 4.16 ms | 328 µs |
| search with no result over 4000 subtitles | 2.72 ms | 169 µs |
| adjusting the durations of 4000 subtitles, four constraints | 1.23 ms | 46.9 µs |
| title case over 4000 subtitles | 17.8 ms | 617 µs |
| pasting 4000 texts | 992 µs | 59.8 µs |
| aligning a 4000-line translation, by position | 814 µs | 44.4 µs |
| aligning a 4000-line translation, by number | 799 µs | 45.9 µs |
| frame rate deduction over 4000 subtitles | 423 µs | 6.52 µs |
| conversion search over 4000 subtitles | 367 µs | 17.9 µs |
| walk of 200 files in a tree of depth 3 | 563 µs | 28.2 µs |
| shift of 200 files of 20 subtitles | 35.4 ms | 1.15 ms |
| shift of one file of 4000 subtitles | 2.87 ms | 91.3 µs |

### 0.15.19 — 2026-10-12 — Release — load 1.45 — pace ×0.99

| Measure | Mean | Standard deviation |
| :----- | ------: | ---------: |
| the current line, among 4000 subtitles | 5.97 µs | 1.81 µs |
| converting a tagged line for the image | 835 ns | 112 ns |
| composing a two-line line | 110 ns | 3.27 ns |
| opening a video | 219 ms | 1.82 ms |
| seeking a position | 5.42 ms | 92.4 µs |
| rendering a frame of 640×360 | 314 µs | 21.7 µs |
| rendering a frame of 1280×720 | 841 µs | 44.5 µs |
| rendering a frame of 1920×1080 | 2.25 ms | 190 µs |
| seeking frame 0 (720p video) | 6.25 ms | 257 µs |
| seeking frame 100 (720p video) | 46.2 ms | 2.53 ms |
| seeking frame 249 (720p video) | 109 ms | 4.56 ms |
| step forward to frame 100 (720p video) | 46.4 ms | 2.39 ms |
| step forward to frame 249 (720p video) | 108 ms | 5.42 ms |
| step back to frame 99 (720p video) | 46.4 ms | 3.93 ms |
| step back to frame 248 (720p video) | 109 ms | 3.55 ms |
| building the model over 4000 subtitles | 8.25 µs | 1.36 µs |
| a window of 40 rows, five columns | 17.6 µs | 379 ns |
| rafraîchir après un shifting 4000 subtitles | 8.08 µs | 875 ns |
| resetting the model after a row is removed | 8.03 µs | 514 ns |
| editing a text cell | 395 ns | 37.7 ns |
| editing a position cell | 9.06 µs | 335 ns |
| centring a row among 4000 subtitles | 1.08 µs | 78.7 ns |
| splitting 4000 subtitles, in ems, with a length cache | 176 ms | 4.79 ms |
| versionString | 45 ns | 0.572 ns |
| parse | 38.5 ns | 9.67 ns |
| format | 31.4 ns | 0.633 ns |
| position to frame | 6.32 ns | 0.06 ns |
| frame to position | 6.47 ns | 0.0664 ns |
| scaling by an exact rational | 6.71 ns | 0.0541 ns |
| splitting 4000 subtitles, without a length cache | 109 ms | 3.65 ms |
| splitting 4000 subtitles, with a length cache | 114 ms | 3.45 ms |
| common French errors over 4000 subtitles | 155 ms | 3.66 ms |
| common English errors over 4000 subtitles | 173 ms | 6.62 ms |
| reading 4000 subtitles | 2.25 ms | 449 µs |
| writing 4000 subtitles | 507 µs | 24.6 µs |
| shifting 4000 subtitles | 6.96 µs | 2.99 µs |
| shifting then undoing | 13.4 µs | 2.94 µs |
| transforming 4000 subtitles | 72.7 µs | 13.6 µs |
| frame rate conversion over 4000 subtitles | 74.3 µs | 6.44 µs |
| alignment over 4000 subtitles | 130 µs | 8.12 µs |
| sorting 4000 subtitles in reverse | 272 µs | 25.7 µs |
| removing every other subtitle | 164 µs | 23.6 µs |
| removal then undo | 244 µs | 24.9 µs |
| inserting 100 empty subtitles in the middle | 53.2 µs | 14.8 µs |
| editing a text, through a session | 184 ns | 11.3 ns |
| removing mentions over 4000 subtitles | 1.22 ms | 48.7 µs |
| italicising 4000 subtitles | 907 µs | 36.1 µs |
| replacing a frequent word over 4000 subtitles, plain text | 4.08 ms | 319 µs |
| replacing a frequent word over 4000 subtitles, regular expression | 4.07 ms | 96 µs |
| search with no result over 4000 subtitles | 2.59 ms | 56.7 µs |
| adjusting the durations of 4000 subtitles, four constraints | 1.23 ms | 46.1 µs |
| title case over 4000 subtitles | 17.8 ms | 710 µs |
| pasting 4000 texts | 1.03 ms | 46.1 µs |
| aligning a 4000-line translation, by position | 824 µs | 49.8 µs |
| aligning a 4000-line translation, by number | 931 µs | 77.4 µs |
| frame rate deduction over 4000 subtitles | 372 µs | 15.3 µs |
| conversion search over 4000 subtitles | 382 µs | 25.6 µs |
| walk of 200 files in a tree of depth 3 | 556 µs | 25 µs |
| shift of 200 files of 20 subtitles | 33.2 ms | 1.95 ms |
| shift of one file of 4000 subtitles | 2.86 ms | 93.4 µs |

### 0.15.18 — 2026-10-12 — Release — load 1.46 — pace ×1.18

| Measure | Mean | Standard deviation |
| :----- | ------: | ---------: |
| the current line, among 4000 subtitles | 6.82 µs | 2 µs |
| converting a tagged line for the image | 1.14 µs | 630 ns |
| composing a two-line line | 128 ns | 10.6 ns |
| opening a video | 221 ms | 1.99 ms |
| seeking a position | 5.35 ms | 112 µs |
| rendering a frame of 640×360 | 356 µs | 23.2 µs |
| rendering a frame of 1280×720 | 1.01 ms | 142 µs |
| rendering a frame of 1920×1080 | 2.35 ms | 201 µs |
| seeking frame 0 (720p video) | 6.34 ms | 254 µs |
| seeking frame 100 (720p video) | 49.1 ms | 2.18 ms |
| seeking frame 249 (720p video) | 115 ms | 3.16 ms |
| step forward to frame 100 (720p video) | 49 ms | 2.03 ms |
| step forward to frame 249 (720p video) | 115 ms | 3.52 ms |
| step back to frame 99 (720p video) | 49.4 ms | 2.47 ms |
| step back to frame 248 (720p video) | 115 ms | 3.76 ms |
| building the model over 4000 subtitles | 12.6 µs | 4.08 µs |
| a window of 40 rows, five columns | 20.9 µs | 1.62 µs |
| rafraîchir après un shifting 4000 subtitles | 11.4 µs | 6.08 µs |
| resetting the model after a row is removed | 12.3 µs | 1.65 µs |
| editing a text cell | 472 ns | 147 ns |
| editing a position cell | 13.4 µs | 1.5 µs |
| centring a row among 4000 subtitles | 1.24 µs | 214 ns |
| splitting 4000 subtitles, in ems, with a length cache | 206 ms | 2.88 ms |
| versionString | 51.7 ns | 0.807 ns |
| parse | 50.4 ns | 22.2 ns |
| format | 38 ns | 0.782 ns |
| position to frame | 8.04 ns | 2.13 ns |
| frame to position | 7.24 ns | 0.067 ns |
| scaling by an exact rational | 7.92 ns | 1.52 ns |
| splitting 4000 subtitles, without a length cache | 127 ms | 1.97 ms |
| splitting 4000 subtitles, with a length cache | 131 ms | 1.8 ms |
| common French errors over 4000 subtitles | 179 ms | 2.27 ms |
| common English errors over 4000 subtitles | 200 ms | 2.42 ms |
| reading 4000 subtitles | 2.47 ms | 239 µs |
| writing 4000 subtitles | 631 µs | 172 µs |
| shifting 4000 subtitles | 9.51 µs | 3.21 µs |
| shifting then undoing | 18.3 µs | 9.61 µs |
| transforming 4000 subtitles | 94 µs | 28.4 µs |
| frame rate conversion over 4000 subtitles | 94.5 µs | 10.2 µs |
| alignment over 4000 subtitles | 160 µs | 16.3 µs |
| sorting 4000 subtitles in reverse | 328 µs | 24 µs |
| removing every other subtitle | 200 µs | 21.9 µs |
| removal then undo | 294 µs | 59 µs |
| inserting 100 empty subtitles in the middle | 66.9 µs | 8.4 µs |
| editing a text, through a session | 213 ns | 27.7 ns |
| removing mentions over 4000 subtitles | 1.38 ms | 110 µs |
| italicising 4000 subtitles | 1.07 ms | 68.4 µs |
| replacing a frequent word over 4000 subtitles, plain text | 4.56 ms | 211 µs |
| replacing a frequent word over 4000 subtitles, regular expression | 4.73 ms | 203 µs |
| search with no result over 4000 subtitles | 3.05 ms | 148 µs |
| adjusting the durations of 4000 subtitles, four constraints | 1.47 ms | 63.1 µs |
| title case over 4000 subtitles | 21.6 ms | 580 µs |
| pasting 4000 texts | 1.16 ms | 79.7 µs |
| aligning a 4000-line translation, by position | 983 µs | 84.4 µs |
| aligning a 4000-line translation, by number | 984 µs | 96.7 µs |
| frame rate deduction over 4000 subtitles | 436 µs | 27.6 µs |
| conversion search over 4000 subtitles | 451 µs | 58 µs |
| walk of 200 files in a tree of depth 3 | 665 µs | 72.3 µs |
| shift of 200 files of 20 subtitles | 38.5 ms | 705 µs |
| shift of one file of 4000 subtitles | 3.58 ms | 423 µs |

### 0.15.15 — 2026-10-12 — Release — load 1.42 — pace ×0.99

| Measure | Mean | Standard deviation |
| :----- | ------: | ---------: |
| the current line, among 4000 subtitles | 5.72 µs | 251 ns |
| converting a tagged line for the image | 828 ns | 41.4 ns |
| composing a two-line line | 111 ns | 2.46 ns |
| opening a video | 219 ms | 1.73 ms |
| seeking a position | 5.38 ms | 101 µs |
| rendering a frame of 640×360 | 313 µs | 24.9 µs |
| rendering a frame of 1280×720 | 849 µs | 39.3 µs |
| rendering a frame of 1920×1080 | 2.02 ms | 82.9 µs |
| seeking frame 0 (720p video) | 6.21 ms | 254 µs |
| seeking frame 100 (720p video) | 44.7 ms | 2.97 ms |
| seeking frame 249 (720p video) | 105 ms | 5.69 ms |
| step forward to frame 100 (720p video) | 45 ms | 2.55 ms |
| step forward to frame 249 (720p video) | 104 ms | 5.99 ms |
| step back to frame 99 (720p video) | 44.7 ms | 2.61 ms |
| step back to frame 248 (720p video) | 105 ms | 6.46 ms |
| building the model over 4000 subtitles | 8.04 µs | 778 ns |
| a window of 40 rows, five columns | 17.8 µs | 1.07 µs |
| rafraîchir après un shifting 4000 subtitles | 8.02 µs | 490 ns |
| resetting the model after a row is removed | 8.23 µs | 2.71 µs |
| editing a text cell | 380 ns | 24.3 ns |
| editing a position cell | 9.01 µs | 440 ns |
| centring a row among 4000 subtitles | 1.07 µs | 126 ns |
| splitting 4000 subtitles, in ems, with a length cache | 175 ms | 4.38 ms |
| versionString | 46 ns | 5.79 ns |
| parse | 36.4 ns | 3.23 ns |
| format | 36.9 ns | 0.365 ns |
| position to frame | 6.48 ns | 0.0604 ns |
| frame to position | 6.32 ns | 0.0662 ns |
| scaling by an exact rational | 6.72 ns | 0.0697 ns |
| splitting 4000 subtitles, without a length cache | 110 ms | 6.82 ms |
| splitting 4000 subtitles, with a length cache | 113 ms | 4.75 ms |
| common French errors over 4000 subtitles | 154 ms | 4.18 ms |
| common English errors over 4000 subtitles | 170 ms | 3.39 ms |
| reading 4000 subtitles | 2.02 ms | 74.1 µs |
| writing 4000 subtitles | 536 µs | 36.5 µs |
| shifting 4000 subtitles | 6.84 µs | 2.17 µs |
| shifting then undoing | 13.4 µs | 3.07 µs |
| transforming 4000 subtitles | 77 µs | 16.8 µs |
| frame rate conversion over 4000 subtitles | 98.7 µs | 23.4 µs |
| alignment over 4000 subtitles | 134 µs | 11.5 µs |
| sorting 4000 subtitles in reverse | 274 µs | 20.2 µs |
| removing every other subtitle | 161 µs | 15.3 µs |
| removal then undo | 245 µs | 26.5 µs |
| inserting 100 empty subtitles in the middle | 55.9 µs | 20.6 µs |
| editing a text, through a session | 173 ns | 8.46 ns |
| removing mentions over 4000 subtitles | 1.2 ms | 28.6 µs |
| italicising 4000 subtitles | 914 µs | 39.1 µs |
| replacing a frequent word over 4000 subtitles, plain text | 3.83 ms | 62 µs |
| replacing a frequent word over 4000 subtitles, regular expression | 4.01 ms | 56.3 µs |
| search with no result over 4000 subtitles | 2.59 ms | 45.4 µs |
| adjusting the durations of 4000 subtitles, four constraints | 1.22 ms | 46.7 µs |
| title case over 4000 subtitles | 17.6 ms | 940 µs |
| pasting 4000 texts | 965 µs | 32.2 µs |
| aligning a 4000-line translation, by position | 807 µs | 44.9 µs |
| aligning a 4000-line translation, by number | 788 µs | 39.3 µs |
| frame rate deduction over 4000 subtitles | 369 µs | 1.6 µs |
| conversion search over 4000 subtitles | 363 µs | 1.3 µs |
| walk of 200 files in a tree of depth 3 | 559 µs | 27 µs |
| shift of 200 files of 20 subtitles | 31.9 ms | 1.5 ms |
| shift of one file of 4000 subtitles | 2.88 ms | 87.1 µs |

### 0.15.13 — 2026-10-12 — Release — load 1.49 — pace ×0.99

| Measure | Mean | Standard deviation |
| :----- | ------: | ---------: |
| the current line, among 4000 subtitles | 7.2 µs | 198 ns |
| converting a tagged line for the image | 837 ns | 24.4 ns |
| composing a two-line line | 119 ns | 29.9 ns |
| opening a video | 219 ms | 1.95 ms |
| seeking a position | 5.38 ms | 88.4 µs |
| rendering a frame of 640×360 | 322 µs | 32.4 µs |
| rendering a frame of 1280×720 | 841 µs | 40.2 µs |
| rendering a frame of 1920×1080 | 2.34 ms | 110 µs |
| seeking frame 0 (720p video) | 6.22 ms | 151 µs |
| seeking frame 100 (720p video) | 46.8 ms | 2.08 ms |
| seeking frame 249 (720p video) | 111 ms | 3.46 ms |
| step forward to frame 100 (720p video) | 46.4 ms | 2.24 ms |
| step forward to frame 249 (720p video) | 112 ms | 9.05 ms |
| step back to frame 99 (720p video) | 46.5 ms | 2.45 ms |
| step back to frame 248 (720p video) | 110 ms | 2.61 ms |
| building the model over 4000 subtitles | 8.36 µs | 3.21 µs |
| a window of 40 rows, five columns | 18.3 µs | 3.3 µs |
| rafraîchir après un shifting 4000 subtitles | 9.61 µs | 3.22 µs |
| resetting the model after a row is removed | 8.41 µs | 2.06 µs |
| editing a text cell | 376 ns | 23.8 ns |
| editing a position cell | 9.14 µs | 440 ns |
| centring a row among 4000 subtitles | 1.08 µs | 210 ns |
| splitting 4000 subtitles, in ems, with a length cache | 179 ms | 9.41 ms |
| versionString | 46.1 ns | 0.469 ns |
| parse | 36 ns | 2.89 ns |
| format | 37 ns | 0.645 ns |
| position to frame | 6.48 ns | 0.0682 ns |
| frame to position | 6.82 ns | 2.08 ns |
| scaling by an exact rational | 6.71 ns | 0.0515 ns |
| splitting 4000 subtitles, without a length cache | 108 ms | 2.87 ms |
| splitting 4000 subtitles, with a length cache | 112 ms | 3.75 ms |
| common French errors over 4000 subtitles | 158 ms | 10.4 ms |
| common English errors over 4000 subtitles | 169 ms | 5.02 ms |
| reading 4000 subtitles | 2.03 ms | 86.3 µs |
| writing 4000 subtitles | 535 µs | 30.5 µs |
| shifting 4000 subtitles | 7.12 µs | 3.56 µs |
| shifting then undoing | 13.6 µs | 3.48 µs |
| transforming 4000 subtitles | 75.6 µs | 9.29 µs |
| frame rate conversion over 4000 subtitles | 74.8 µs | 14.4 µs |
| alignment over 4000 subtitles | 133 µs | 9.89 µs |
| sorting 4000 subtitles in reverse | 273 µs | 23.4 µs |
| removing every other subtitle | 163 µs | 22.9 µs |
| removal then undo | 246 µs | 28.5 µs |
| inserting 100 empty subtitles in the middle | 53.5 µs | 21.3 µs |
| editing a text, through a session | 176 ns | 9.42 ns |
| removing mentions over 4000 subtitles | 1.2 ms | 44.7 µs |
| italicising 4000 subtitles | 928 µs | 50.5 µs |
| replacing a frequent word over 4000 subtitles, plain text | 3.86 ms | 79 µs |
| replacing a frequent word over 4000 subtitles, regular expression | 4.02 ms | 71.9 µs |
| search with no result over 4000 subtitles | 2.62 ms | 65.5 µs |
| adjusting the durations of 4000 subtitles, four constraints | 1.22 ms | 45.4 µs |
| title case over 4000 subtitles | 18 ms | 581 µs |
| pasting 4000 texts | 977 µs | 54.7 µs |
| aligning a 4000-line translation, by position | 910 µs | 49.3 µs |
| aligning a 4000-line translation, by number | 795 µs | 43.1 µs |
| frame rate deduction over 4000 subtitles | 424 µs | 9.87 µs |
| conversion search over 4000 subtitles | 366 µs | 13.5 µs |
| walk of 200 files in a tree of depth 3 | 558 µs | 19.1 µs |
| shift of 200 files of 20 subtitles | 34.9 ms | 1.05 ms |
| shift of one file of 4000 subtitles | 3.01 ms | 183 µs |

### 0.15.10 — 2026-10-10 — Release — load 1.36 — pace ×0.99

| Measure | Mean | Standard deviation |
| :----- | ------: | ---------: |
| the current line, among 4000 subtitles | 5.84 µs | 1.85 µs |
| converting a tagged line for the image | 829 ns | 28.1 ns |
| composing a two-line line | 140 ns | 1.45 ns |
| opening a video | 219 ms | 1.68 ms |
| seeking a position | 5.4 ms | 93.9 µs |
| rendering a frame of 640×360 | 313 µs | 27.9 µs |
| rendering a frame of 1280×720 | 832 µs | 35.7 µs |
| rendering a frame of 1920×1080 | 2.07 ms | 114 µs |
| seeking frame 0 (720p video) | 6.17 ms | 258 µs |
| seeking frame 100 (720p video) | 45.3 ms | 3.07 ms |
| seeking frame 249 (720p video) | 107 ms | 7.42 ms |
| step forward to frame 100 (720p video) | 45.2 ms | 3.58 ms |
| step forward to frame 249 (720p video) | 107 ms | 6.55 ms |
| step back to frame 99 (720p video) | 45 ms | 3.45 ms |
| step back to frame 248 (720p video) | 106 ms | 6.78 ms |
| building the model over 4000 subtitles | 8.03 µs | 256 ns |
| a window of 40 rows, five columns | 20.1 µs | 974 ns |
| rafraîchir après un shifting 4000 subtitles | 8.15 µs | 1.64 µs |
| resetting the model after a row is removed | 8.44 µs | 4.69 µs |
| editing a text cell | 390 ns | 85.6 ns |
| editing a position cell | 9.02 µs | 373 ns |
| centring a row among 4000 subtitles | 1.08 µs | 137 ns |
| splitting 4000 subtitles, in ems, with a length cache | 177 ms | 5.71 ms |
| versionString | 43.9 ns | 0.389 ns |
| parse | 38.8 ns | 3.5 ns |
| format | 36.6 ns | 0.554 ns |
| position to frame | 6.65 ns | 0.0698 ns |
| frame to position | 7.8 ns | 1.48 ns |
| scaling by an exact rational | 6.87 ns | 0.0763 ns |
| splitting 4000 subtitles, without a length cache | 110 ms | 4.62 ms |
| splitting 4000 subtitles, with a length cache | 114 ms | 4.2 ms |
| common French errors over 4000 subtitles | 156 ms | 6.72 ms |
| common English errors over 4000 subtitles | 172 ms | 5.62 ms |
| reading 4000 subtitles | 2.09 ms | 132 µs |
| writing 4000 subtitles | 570 µs | 39.9 µs |
| shifting 4000 subtitles | 6.76 µs | 2.85 µs |
| shifting then undoing | 15.2 µs | 6.94 µs |
| transforming 4000 subtitles | 85.1 µs | 6.12 µs |
| frame rate conversion over 4000 subtitles | 75.9 µs | 6.51 µs |
| alignment over 4000 subtitles | 134 µs | 10.8 µs |
| sorting 4000 subtitles in reverse | 305 µs | 13.2 µs |
| removing every other subtitle | 184 µs | 14.2 µs |
| removal then undo | 258 µs | 28.2 µs |
| inserting 100 empty subtitles in the middle | 55 µs | 15.3 µs |
| editing a text, through a session | 173 ns | 13 ns |
| removing mentions over 4000 subtitles | 1.21 ms | 23.8 µs |
| italicising 4000 subtitles | 917 µs | 37.5 µs |
| replacing a frequent word over 4000 subtitles, plain text | 3.9 ms | 190 µs |
| replacing a frequent word over 4000 subtitles, regular expression | 4.02 ms | 73.1 µs |
| search with no result over 4000 subtitles | 2.59 ms | 48.1 µs |
| adjusting the durations of 4000 subtitles, four constraints | 1.25 ms | 51.2 µs |
| title case over 4000 subtitles | 18.1 ms | 784 µs |
| pasting 4000 texts | 987 µs | 55 µs |
| aligning a 4000-line translation, by position | 1.06 ms | 117 µs |
| aligning a 4000-line translation, by number | 800 µs | 30 µs |
| frame rate deduction over 4000 subtitles | 367 µs | 9.23 µs |
| conversion search over 4000 subtitles | 365 µs | 15.7 µs |
| walk of 200 files in a tree of depth 3 | 563 µs | 38.6 µs |
| shift of 200 files of 20 subtitles | 32.4 ms | 1.98 ms |
| shift of one file of 4000 subtitles | 2.85 ms | 354 µs |

### 0.15.9 — 2026-10-10 — Release — load 1.46 — pace ×0.96

| Measure | Mean | Standard deviation |
| :----- | ------: | ---------: |
| the current line, among 4000 subtitles | 7.71 µs | 1.83 µs |
| converting a tagged line for the image | 856 ns | 174 ns |
| composing a two-line line | 127 ns | 50.7 ns |
| opening a video | 219 ms | 1.4 ms |
| seeking a position | 5.39 ms | 105 µs |
| rendering a frame of 640×360 | 305 µs | 17.3 µs |
| rendering a frame of 1280×720 | 833 µs | 56.1 µs |
| rendering a frame of 1920×1080 | 2.05 ms | 151 µs |
| seeking frame 0 (720p video) | 6.22 ms | 191 µs |
| seeking frame 100 (720p video) | 44.6 ms | 2.78 ms |
| seeking frame 249 (720p video) | 107 ms | 5.81 ms |
| step forward to frame 100 (720p video) | 45.5 ms | 3.14 ms |
| step forward to frame 249 (720p video) | 106 ms | 6.26 ms |
| step back to frame 99 (720p video) | 44.6 ms | 2.88 ms |
| step back to frame 248 (720p video) | 106 ms | 5.77 ms |
| building the model over 4000 subtitles | 8.08 µs | 894 ns |
| a window of 40 rows, five columns | 19.8 µs | 6.97 µs |
| rafraîchir après un shifting 4000 subtitles | 8.24 µs | 2.63 µs |
| resetting the model after a row is removed | 8.18 µs | 2.34 µs |
| editing a text cell | 382 ns | 27.8 ns |
| editing a position cell | 9.01 µs | 294 ns |
| centring a row among 4000 subtitles | 1.25 µs | 338 ns |
| splitting 4000 subtitles, in ems, with a length cache | 178 ms | 6.1 ms |
| versionString | 45.3 ns | 18.8 ns |
| parse | 37.2 ns | 4.58 ns |
| format | 36.7 ns | 5.88 ns |
| position to frame | 6.64 ns | 0.0625 ns |
| frame to position | 6.64 ns | 0.0465 ns |
| scaling by an exact rational | 8.27 ns | 1.92 ns |
| splitting 4000 subtitles, without a length cache | 110 ms | 3.47 ms |
| splitting 4000 subtitles, with a length cache | 114 ms | 4.77 ms |
| common French errors over 4000 subtitles | 156 ms | 4.63 ms |
| common English errors over 4000 subtitles | 174 ms | 6.26 ms |
| reading 4000 subtitles | 2.05 ms | 90.6 µs |
| writing 4000 subtitles | 519 µs | 25.5 µs |
| shifting 4000 subtitles | 6.99 µs | 3.41 µs |
| shifting then undoing | 13.7 µs | 3.79 µs |
| transforming 4000 subtitles | 73.7 µs | 5.98 µs |
| frame rate conversion over 4000 subtitles | 88.8 µs | 13.5 µs |
| alignment over 4000 subtitles | 131 µs | 5.41 µs |
| sorting 4000 subtitles in reverse | 267 µs | 18.6 µs |
| removing every other subtitle | 164 µs | 26.3 µs |
| removal then undo | 243 µs | 25.1 µs |
| inserting 100 empty subtitles in the middle | 53 µs | 14.6 µs |
| editing a text, through a session | 227 ns | 82.9 ns |
| removing mentions over 4000 subtitles | 1.19 ms | 32.6 µs |
| italicising 4000 subtitles | 1.32 ms | 552 µs |
| replacing a frequent word over 4000 subtitles, plain text | 4.36 ms | 382 µs |
| replacing a frequent word over 4000 subtitles, regular expression | 4.55 ms | 451 µs |
| search with no result over 4000 subtitles | 2.99 ms | 90.2 µs |
| adjusting the durations of 4000 subtitles, four constraints | 1.3 ms | 93.8 µs |
| title case over 4000 subtitles | 20.4 ms | 1.14 ms |
| pasting 4000 texts | 1.18 ms | 404 µs |
| aligning a 4000-line translation, by position | 837 µs | 93 µs |
| aligning a 4000-line translation, by number | 956 µs | 88 µs |
| frame rate deduction over 4000 subtitles | 427 µs | 14.9 µs |
| conversion search over 4000 subtitles | 365 µs | 9.54 µs |
| walk of 200 files in a tree of depth 3 | 653 µs | 57.5 µs |
| shift of 200 files of 20 subtitles | 35.7 ms | 1.06 ms |
| shift of one file of 4000 subtitles | 2.93 ms | 206 µs |

### 0.15.7 — 2026-10-09 — Release — load 1.48 — pace ×0.98

| Measure | Mean | Standard deviation |
| :----- | ------: | ---------: |
| the current line, among 4000 subtitles | 5.69 µs | 499 ns |
| converting a tagged line for the image | 990 ns | 104 ns |
| composing a two-line line | 126 ns | 3.45 ns |
| opening a video | 219 ms | 1.74 ms |
| seeking a position | 5.4 ms | 92.3 µs |
| rendering a frame of 640×360 | 310 µs | 26.3 µs |
| rendering a frame of 1280×720 | 942 µs | 35.8 µs |
| rendering a frame of 1920×1080 | 2.06 ms | 120 µs |
| seeking frame 0 (720p video) | 6.19 ms | 153 µs |
| seeking frame 100 (720p video) | 47.8 ms | 2.2 ms |
| seeking frame 249 (720p video) | 117 ms | 14.2 ms |
| step forward to frame 100 (720p video) | 47.9 ms | 3.36 ms |
| step forward to frame 249 (720p video) | 111 ms | 2.67 ms |
| step back to frame 99 (720p video) | 47.3 ms | 3.22 ms |
| step back to frame 248 (720p video) | 112 ms | 6.03 ms |
| building the model over 4000 subtitles | 8.06 µs | 685 ns |
| a window of 40 rows, five columns | 20.1 µs | 1.09 µs |
| rafraîchir après un shifting 4000 subtitles | 8.09 µs | 1.19 µs |
| resetting the model after a row is removed | 8.54 µs | 1.86 µs |
| editing a text cell | 395 ns | 102 ns |
| editing a position cell | 9.27 µs | 1.46 µs |
| centring a row among 4000 subtitles | 1.11 µs | 481 ns |
| splitting 4000 subtitles, in ems, with a length cache | 185 ms | 9.33 ms |
| versionString | 39.2 ns | 0.482 ns |
| parse | 41.8 ns | 3.68 ns |
| format | 36.1 ns | 0.486 ns |
| position to frame | 6.64 ns | 0.0706 ns |
| frame to position | 8.76 ns | 3.58 ns |
| scaling by an exact rational | 6.86 ns | 0.0543 ns |
| splitting 4000 subtitles, without a length cache | 114 ms | 6.09 ms |
| splitting 4000 subtitles, with a length cache | 117 ms | 6.71 ms |
| common French errors over 4000 subtitles | 164 ms | 8.62 ms |
| common English errors over 4000 subtitles | 184 ms | 9.75 ms |
| reading 4000 subtitles | 2.05 ms | 125 µs |
| writing 4000 subtitles | 598 µs | 32.6 µs |
| shifting 4000 subtitles | 7.96 µs | 2.87 µs |
| shifting then undoing | 16.2 µs | 5.24 µs |
| transforming 4000 subtitles | 84.4 µs | 3.64 µs |
| frame rate conversion over 4000 subtitles | 81.5 µs | 6.63 µs |
| alignment over 4000 subtitles | 152 µs | 10.6 µs |
| sorting 4000 subtitles in reverse | 270 µs | 19.6 µs |
| removing every other subtitle | 163 µs | 26.8 µs |
| removal then undo | 282 µs | 34.3 µs |
| inserting 100 empty subtitles in the middle | 60.5 µs | 13.6 µs |
| editing a text, through a session | 199 ns | 21.5 ns |
| removing mentions over 4000 subtitles | 1.29 ms | 96.4 µs |
| italicising 4000 subtitles | 1.09 ms | 90.1 µs |
| replacing a frequent word over 4000 subtitles, plain text | 4.36 ms | 287 µs |
| replacing a frequent word over 4000 subtitles, regular expression | 5.26 ms | 719 µs |
| search with no result over 4000 subtitles | 2.82 ms | 325 µs |
| adjusting the durations of 4000 subtitles, four constraints | 1.86 ms | 503 µs |
| title case over 4000 subtitles | 22.1 ms | 1.71 ms |
| pasting 4000 texts | 1.2 ms | 62.9 µs |
| aligning a 4000-line translation, by position | 1.02 ms | 98.8 µs |
| aligning a 4000-line translation, by number | 1.03 ms | 188 µs |
| frame rate deduction over 4000 subtitles | 459 µs | 15.6 µs |
| conversion search over 4000 subtitles | 448 µs | 12.6 µs |
| walk of 200 files in a tree of depth 3 | 705 µs | 62.8 µs |
| shift of 200 files of 20 subtitles | 36.7 ms | 1.32 ms |
| shift of one file of 4000 subtitles | 3.35 ms | 108 µs |

### 0.15.5 — 2026-10-09 — Release — load 1.42 — pace ×0.91

| Measure | Mean | Standard deviation |
| :----- | ------: | ---------: |
| the current line, among 4000 subtitles | 7.25 µs | 1.44 µs |
| converting a tagged line for the image | 815 ns | 29 ns |
| composing a two-line line | 136 ns | 1.74 ns |
| opening a video | 219 ms | 1.68 ms |
| seeking a position | 5.36 ms | 85.1 µs |
| rendering a frame of 640×360 | 343 µs | 14.2 µs |
| rendering a frame of 1280×720 | 825 µs | 58.8 µs |
| rendering a frame of 1920×1080 | 2.1 ms | 181 µs |
| seeking frame 0 (720p video) | 6.2 ms | 321 µs |
| seeking frame 100 (720p video) | 45.1 ms | 2.65 ms |
| seeking frame 249 (720p video) | 106 ms | 5.48 ms |
| step forward to frame 100 (720p video) | 45.3 ms | 3.46 ms |
| step forward to frame 249 (720p video) | 105 ms | 6.19 ms |
| step back to frame 99 (720p video) | 45.4 ms | 2.7 ms |
| step back to frame 248 (720p video) | 105 ms | 5.73 ms |
| building the model over 4000 subtitles | 11 µs | 2.93 µs |
| a window of 40 rows, five columns | 17.4 µs | 830 ns |
| rafraîchir après un shifting 4000 subtitles | 10.4 µs | 532 ns |
| resetting the model after a row is removed | 10.4 µs | 1.36 µs |
| editing a text cell | 407 ns | 58.4 ns |
| editing a position cell | 11.5 µs | 388 ns |
| centring a row among 4000 subtitles | 1.06 µs | 189 ns |
| splitting 4000 subtitles, in ems, with a length cache | 180 ms | 5.69 ms |
| versionString | 39.2 ns | 0.348 ns |
| parse | 36.9 ns | 3.21 ns |
| format | 44.7 ns | 15.6 ns |
| position to frame | 6.64 ns | 0.0478 ns |
| frame to position | 6.64 ns | 0.0694 ns |
| scaling by an exact rational | 6.86 ns | 0.0565 ns |
| splitting 4000 subtitles, without a length cache | 111 ms | 4.59 ms |
| splitting 4000 subtitles, with a length cache | 115 ms | 4.8 ms |
| common French errors over 4000 subtitles | 159 ms | 12.7 ms |
| common English errors over 4000 subtitles | 172 ms | 5.98 ms |
| reading 4000 subtitles | 2.17 ms | 88.4 µs |
| writing 4000 subtitles | 505 µs | 27.3 µs |
| shifting 4000 subtitles | 7.78 µs | 3.22 µs |
| shifting then undoing | 14.9 µs | 3.39 µs |
| transforming 4000 subtitles | 74.4 µs | 5.52 µs |
| frame rate conversion over 4000 subtitles | 78.7 µs | 8.97 µs |
| alignment over 4000 subtitles | 132 µs | 6.71 µs |
| sorting 4000 subtitles in reverse | 278 µs | 20.3 µs |
| removing every other subtitle | 161 µs | 15.4 µs |
| removal then undo | 245 µs | 25.6 µs |
| inserting 100 empty subtitles in the middle | 55.5 µs | 13.1 µs |
| editing a text, through a session | 184 ns | 8.19 ns |
| removing mentions over 4000 subtitles | 1.22 ms | 46.8 µs |
| italicising 4000 subtitles | 922 µs | 36.8 µs |
| replacing a frequent word over 4000 subtitles, plain text | 3.93 ms | 174 µs |
| replacing a frequent word over 4000 subtitles, regular expression | 4.26 ms | 258 µs |
| search with no result over 4000 subtitles | 2.61 ms | 81.6 µs |
| adjusting the durations of 4000 subtitles, four constraints | 1.25 ms | 65.6 µs |
| title case over 4000 subtitles | 17.9 ms | 697 µs |
| pasting 4000 texts | 994 µs | 48 µs |
| aligning a 4000-line translation, by position | 913 µs | 63.3 µs |
| aligning a 4000-line translation, by number | 816 µs | 31.1 µs |
| frame rate deduction over 4000 subtitles | 369 µs | 12.9 µs |
| conversion search over 4000 subtitles | 361 µs | 8.67 µs |
| walk of 200 files in a tree of depth 3 | 559 µs | 23.1 µs |
| shift of 200 files of 20 subtitles | 32.9 ms | 1.85 ms |
| shift of one file of 4000 subtitles | 2.98 ms | 262 µs |

### 0.15.0 — 2026-10-09 — Release — load 1.26 — pace ×0.99

| Measure | Mean | Standard deviation |
| :----- | ------: | ---------: |
| the current line, among 4000 subtitles | 8.75 µs | 3.43 µs |
| converting a tagged line for the image | 959 ns | 133 ns |
| composing a two-line line | 161 ns | 42 ns |
| opening a video | 223 ms | 3.97 ms |
| seeking a position | 5.4 ms | 115 µs |
| rendering a frame of 640×360 | 378 µs | 54.2 µs |
| rendering a frame of 1280×720 | 1.02 ms | 115 µs |
| rendering a frame of 1920×1080 | 2.66 ms | 681 µs |
| seeking frame 0 (720p video) | 6.87 ms | 791 µs |
| seeking frame 100 (720p video) | 51.2 ms | 6.87 ms |
| seeking frame 249 (720p video) | 122 ms | 17.7 ms |
| step forward to frame 100 (720p video) | 51.9 ms | 11.7 ms |
| step forward to frame 249 (720p video) | 115 ms | 6.06 ms |
| step back to frame 99 (720p video) | 48.9 ms | 3.2 ms |
| step back to frame 248 (720p video) | 114 ms | 6.37 ms |
| building the model over 4000 subtitles | 8.35 µs | 2.04 µs |
| a window of 40 rows, five columns | 20.1 µs | 2.31 µs |
| rafraîchir après un shifting 4000 subtitles | 8.37 µs | 3.94 µs |
| resetting the model after a row is removed | 9.61 µs | 3.78 µs |
| editing a text cell | 497 ns | 148 ns |
| editing a position cell | 9.35 µs | 1.73 µs |
| centring a row among 4000 subtitles | 1.25 µs | 218 ns |
| splitting 4000 subtitles, in ems, with a length cache | 197 ms | 8.81 ms |
| versionString | 44.9 ns | 0.551 ns |
| parse | 42.9 ns | 3.37 ns |
| format | 37 ns | 0.818 ns |
| position to frame | 7.97 ns | 2.18 ns |
| frame to position | 6.8 ns | 1.6 ns |
| scaling by an exact rational | 8.28 ns | 2.53 ns |
| splitting 4000 subtitles, without a length cache | 120 ms | 6.6 ms |
| splitting 4000 subtitles, with a length cache | 128 ms | 6.88 ms |
| common French errors over 4000 subtitles | 173 ms | 8.14 ms |
| common English errors over 4000 subtitles | 190 ms | 6.44 ms |
| reading 4000 subtitles | 2.6 ms | 579 µs |
| writing 4000 subtitles | 572 µs | 37.1 µs |
| shifting 4000 subtitles | 7.55 µs | 3.97 µs |
| shifting then undoing | 15.1 µs | 3.2 µs |
| transforming 4000 subtitles | 82.3 µs | 6.14 µs |
| frame rate conversion over 4000 subtitles | 75.5 µs | 8.03 µs |
| alignment over 4000 subtitles | 149 µs | 8.19 µs |
| sorting 4000 subtitles in reverse | 309 µs | 9.98 µs |
| removing every other subtitle | 166 µs | 28.7 µs |
| removal then undo | 285 µs | 37.9 µs |
| inserting 100 empty subtitles in the middle | 59.3 µs | 28.9 µs |
| editing a text, through a session | 208 ns | 19.3 ns |
| removing mentions over 4000 subtitles | 1.27 ms | 87.1 µs |
| italicising 4000 subtitles | 959 µs | 80 µs |
| replacing a frequent word over 4000 subtitles, plain text | 4.04 ms | 211 µs |
| replacing a frequent word over 4000 subtitles, regular expression | 4.36 ms | 271 µs |
| search with no result over 4000 subtitles | 2.71 ms | 163 µs |
| adjusting the durations of 4000 subtitles, four constraints | 1.4 ms | 29 µs |
| title case over 4000 subtitles | 19.4 ms | 962 µs |
| pasting 4000 texts | 1.04 ms | 56.1 µs |
| aligning a 4000-line translation, by position | 936 µs | 34.3 µs |
| aligning a 4000-line translation, by number | 861 µs | 72.8 µs |
| frame rate deduction over 4000 subtitles | 415 µs | 17.3 µs |
| conversion search over 4000 subtitles | 364 µs | 11.5 µs |
| walk of 200 files in a tree of depth 3 | 573 µs | 39.1 µs |
| shift of 200 files of 20 subtitles | 36.2 ms | 1.82 ms |
| shift of one file of 4000 subtitles | 3.08 ms | 281 µs |

### 0.14.19 — 2026-10-08 — Release — load 1.49 — pace ×1.05

| Measure | Mean | Standard deviation |
| :----- | ------: | ---------: |
| the current line, among 4000 subtitles | 6.78 µs | 1.73 µs |
| converting a tagged line for the image | 1.17 µs | 426 ns |
| composing a two-line line | 169 ns | 38.2 ns |
| opening a video | 222 ms | 26.9 ms |
| seeking a position | 5.64 ms | 519 µs |
| rendering a frame of 640×360 | 373 µs | 17.3 µs |
| rendering a frame of 1280×720 | 1.35 ms | 256 µs |
| rendering a frame of 1920×1080 | 2.65 ms | 540 µs |
| seeking frame 0 (720p video) | 7.81 ms | 679 µs |
| seeking frame 100 (720p video) | 53.2 ms | 7.42 ms |
| seeking frame 249 (720p video) | 125 ms | 10.6 ms |
| step forward to frame 100 (720p video) | 52.4 ms | 6.25 ms |
| step forward to frame 249 (720p video) | 119 ms | 15.9 ms |
| step back to frame 99 (720p video) | 49.5 ms | 2.83 ms |
| step back to frame 248 (720p video) | 114 ms | 6.74 ms |
| building the model over 4000 subtitles | 9.57 µs | 2.82 µs |
| a window of 40 rows, five columns | 20 µs | 1.19 µs |
| rafraîchir après un shifting 4000 subtitles | 9.49 µs | 2.32 µs |
| resetting the model after a row is removed | 9.48 µs | 2.55 µs |
| editing a text cell | 508 ns | 215 ns |
| editing a position cell | 10.7 µs | 1.28 µs |
| centring a row among 4000 subtitles | 1.3 µs | 431 ns |
| splitting 4000 subtitles, in ems, with a length cache | 204 ms | 3.79 ms |
| versionString | 52.3 ns | 12.4 ns |
| parse | 43 ns | 8.93 ns |
| format | 42.7 ns | 8.54 ns |
| position to frame | 7.25 ns | 0.08 ns |
| frame to position | 7.8 ns | 2.4 ns |
| scaling by an exact rational | 7.7 ns | 0.0829 ns |
| splitting 4000 subtitles, without a length cache | 126 ms | 3.22 ms |
| splitting 4000 subtitles, with a length cache | 129 ms | 3.43 ms |
| common French errors over 4000 subtitles | 176 ms | 3.22 ms |
| common English errors over 4000 subtitles | 196 ms | 4.82 ms |
| reading 4000 subtitles | 2.44 ms | 108 µs |
| writing 4000 subtitles | 616 µs | 51.8 µs |
| shifting 4000 subtitles | 8.3 µs | 2.38 µs |
| shifting then undoing | 15.5 µs | 3.67 µs |
| transforming 4000 subtitles | 87.6 µs | 6.31 µs |
| frame rate conversion over 4000 subtitles | 82 µs | 9.92 µs |
| alignment over 4000 subtitles | 150 µs | 8.59 µs |
| sorting 4000 subtitles in reverse | 336 µs | 49.1 µs |
| removing every other subtitle | 289 µs | 79 µs |
| removal then undo | 301 µs | 44.8 µs |
| inserting 100 empty subtitles in the middle | 70.7 µs | 19.6 µs |
| editing a text, through a session | 221 ns | 23.8 ns |
| removing mentions over 4000 subtitles | 1.49 ms | 152 µs |
| italicising 4000 subtitles | 1.07 ms | 51 µs |
| replacing a frequent word over 4000 subtitles, plain text | 4.17 ms | 265 µs |
| replacing a frequent word over 4000 subtitles, regular expression | 4.44 ms | 377 µs |
| search with no result over 4000 subtitles | 3.09 ms | 218 µs |
| adjusting the durations of 4000 subtitles, four constraints | 1.42 ms | 55.6 µs |
| title case over 4000 subtitles | 21.6 ms | 1.4 ms |
| pasting 4000 texts | 1.15 ms | 53.6 µs |
| aligning a 4000-line translation, by position | 936 µs | 40.9 µs |
| aligning a 4000-line translation, by number | 912 µs | 35.8 µs |
| frame rate deduction over 4000 subtitles | 423 µs | 13.2 µs |
| conversion search over 4000 subtitles | 414 µs | 9.24 µs |
| walk of 200 files in a tree of depth 3 | 581 µs | 37.2 µs |
| shift of 200 files of 20 subtitles | 35.9 ms | 995 µs |
| shift of one file of 4000 subtitles | 3.34 ms | 82.4 µs |

### 0.14.7 — 2026-10-07 — Release — load 1.46 — pace ×0.95

| Measure | Mean | Standard deviation |
| :----- | ------: | ---------: |
| the current line, among 4000 subtitles | 5.31 µs | 902 ns |
| composing a two-line line | 185 ns | 32.4 ns |
| opening a video | 218 ms | 1.49 ms |
| seeking a position | 5.39 ms | 103 µs |
| rendering a frame of 640×360 | 216 µs | 44.6 µs |
| rendering a frame of 1280×720 | 724 µs | 88.3 µs |
| rendering a frame of 1920×1080 | 1.41 ms | 187 µs |
| seeking frame 0 (720p video) | 6.65 ms | 500 µs |
| seeking frame 100 (720p video) | 38.6 ms | 1.82 ms |
| seeking frame 249 (720p video) | 85.7 ms | 7.89 ms |
| step forward to frame 100 (720p video) | 40 ms | 5.36 ms |
| step forward to frame 249 (720p video) | 87.6 ms | 10.6 ms |
| step back to frame 99 (720p video) | 40.7 ms | 5.32 ms |
| step back to frame 248 (720p video) | 86.5 ms | 9.59 ms |
| building the model over 4000 subtitles | 8.91 µs | 4.17 µs |
| a window of 40 rows, five columns | 17.9 µs | 2.26 µs |
| rafraîchir après un shifting 4000 subtitles | 8.24 µs | 2.6 µs |
| resetting the model after a row is removed | 8.27 µs | 2.91 µs |
| editing a text cell | 454 ns | 199 ns |
| editing a position cell | 9.62 µs | 2.21 µs |
| splitting 4000 subtitles, in ems, with a length cache | 192 ms | 11.1 ms |
| versionString | 44.7 ns | 0.614 ns |
| parse | 36.6 ns | 2.61 ns |
| format | 42.9 ns | 6.71 ns |
| position to frame | 6.65 ns | 0.0731 ns |
| frame to position | 6.64 ns | 0.046 ns |
| scaling by an exact rational | 6.86 ns | 0.0636 ns |
| splitting 4000 subtitles, without a length cache | 116 ms | 5.85 ms |
| splitting 4000 subtitles, with a length cache | 123 ms | 7.92 ms |
| common French errors over 4000 subtitles | 164 ms | 10 ms |
| common English errors over 4000 subtitles | 183 ms | 11.1 ms |
| reading 4000 subtitles | 2.05 ms | 96 µs |
| writing 4000 subtitles | 555 µs | 42.3 µs |
| shifting 4000 subtitles | 7.56 µs | 3.2 µs |
| shifting then undoing | 15.4 µs | 4.75 µs |
| transforming 4000 subtitles | 72.6 µs | 11.3 µs |
| frame rate conversion over 4000 subtitles | 67.2 µs | 9.06 µs |
| alignment over 4000 subtitles | 127 µs | 12.5 µs |
| sorting 4000 subtitles in reverse | 321 µs | 32.5 µs |
| removing every other subtitle | 199 µs | 35.3 µs |
| removal then undo | 242 µs | 24.6 µs |
| inserting 100 empty subtitles in the middle | 50.9 µs | 7.35 µs |
| editing a text, through a session | 174 ns | 24.6 ns |
| removing mentions over 4000 subtitles | 1.32 ms | 143 µs |
| italicising 4000 subtitles | 947 µs | 130 µs |
| replacing a frequent word over 4000 subtitles, plain text | 4.65 ms | 1.04 ms |
| replacing a frequent word over 4000 subtitles, regular expression | 4.44 ms | 545 µs |
| search with no result over 4000 subtitles | 2.67 ms | 168 µs |
| adjusting the durations of 4000 subtitles, four constraints | 1.24 ms | 59.6 µs |
| title case over 4000 subtitles | 20.1 ms | 2.02 ms |
| pasting 4000 texts | 1 ms | 55.5 µs |
| aligning a 4000-line translation, by position | 922 µs | 55.8 µs |
| aligning a 4000-line translation, by number | 1.02 ms | 193 µs |
| frame rate deduction over 4000 subtitles | 380 µs | 13.9 µs |
| walk of 200 files in a tree of depth 3 | 639 µs | 43.6 µs |
| shift of 200 files of 20 subtitles | 35.6 ms | 3.08 ms |
| shift of one file of 4000 subtitles | 3.45 ms | 263 µs |

### 0.14.5 — 2026-10-06 — Release — load 1.49 — pace ×1.06

| Measure | Mean | Standard deviation |
| :----- | ------: | ---------: |
| the current line, among 4000 subtitles | 4.94 µs | 1.83 µs |
| composing a two-line line | 180 ns | 3.42 ns |
| opening a video | 218 ms | 1.39 ms |
| seeking a position | 5.42 ms | 72.9 µs |
| rendering a frame of 640×360 | 208 µs | 30.3 µs |
| rendering a frame of 1280×720 | 738 µs | 131 µs |
| rendering a frame of 1920×1080 | 1.5 ms | 339 µs |
| seeking frame 0 (720p video) | 7.02 ms | 1.17 ms |
| seeking frame 100 (720p video) | 39.1 ms | 2.79 ms |
| seeking frame 249 (720p video) | 82.5 ms | 2.99 ms |
| step forward to frame 100 (720p video) | 40.8 ms | 6.03 ms |
| step forward to frame 249 (720p video) | 83.3 ms | 5.22 ms |
| step back to frame 99 (720p video) | 39 ms | 4.47 ms |
| step back to frame 248 (720p video) | 82.4 ms | 3.16 ms |
| building the model over 4000 subtitles | 11.3 µs | 6.42 µs |
| a window of 40 rows, five columns | 20.8 µs | 9.41 µs |
| rafraîchir après un shifting 4000 subtitles | 11 µs | 4.04 µs |
| resetting the model after a row is removed | 11 µs | 3.88 µs |
| editing a text cell | 412 ns | 145 ns |
| editing a position cell | 13.8 µs | 883 ns |
| splitting 4000 subtitles, in ems, with a length cache | 197 ms | 11.6 ms |
| versionString | 47.9 ns | 2.36 ns |
| parse | 38.7 ns | 0.889 ns |
| format | 38.5 ns | 13 ns |
| position to frame | 6.64 ns | 0.0523 ns |
| frame to position | 7.75 ns | 0.84 ns |
| scaling by an exact rational | 7.91 ns | 0.388 ns |
| splitting 4000 subtitles, without a length cache | 124 ms | 5.21 ms |
| splitting 4000 subtitles, with a length cache | 126 ms | 2.77 ms |
| common French errors over 4000 subtitles | 174 ms | 8.33 ms |
| common English errors over 4000 subtitles | 198 ms | 14 ms |
| reading 4000 subtitles | 2.37 ms | 275 µs |
| writing 4000 subtitles | 653 µs | 41.7 µs |
| shifting 4000 subtitles | 13.3 µs | 13.5 µs |
| shifting then undoing | 20.6 µs | 10.2 µs |
| transforming 4000 subtitles | 90.8 µs | 14.5 µs |
| frame rate conversion over 4000 subtitles | 84.5 µs | 6.04 µs |
| alignment over 4000 subtitles | 156 µs | 11.3 µs |
| sorting 4000 subtitles in reverse | 336 µs | 31 µs |
| removing every other subtitle | 409 µs | 127 µs |
| removal then undo | 1.24 ms | 862 µs |
| inserting 100 empty subtitles in the middle | 189 µs | 116 µs |
| editing a text, through a session | 202 ns | 23.7 ns |
| removing mentions over 4000 subtitles | 1.46 ms | 50.2 µs |
| italicising 4000 subtitles | 1.34 ms | 747 µs |
| replacing a frequent word over 4000 subtitles, plain text | 5.1 ms | 818 µs |
| replacing a frequent word over 4000 subtitles, regular expression | 5.13 ms | 321 µs |
| search with no result over 4000 subtitles | 4.11 ms | 520 µs |
| adjusting the durations of 4000 subtitles, four constraints | 1.49 ms | 145 µs |
| title case over 4000 subtitles | 22.1 ms | 1.36 ms |
| pasting 4000 texts | 1.1 ms | 134 µs |
| aligning a 4000-line translation, by position | 887 µs | 62.7 µs |
| aligning a 4000-line translation, by number | 936 µs | 136 µs |
| frame rate deduction over 4000 subtitles | 400 µs | 28.3 µs |
| walk of 200 files in a tree of depth 3 | 568 µs | 38.4 µs |
| shift of 200 files of 20 subtitles | 37.1 ms | 1.9 ms |
| shift of one file of 4000 subtitles | 3.7 ms | 614 µs |

### 0.14.3 — 2026-10-06 — Release — load 1.48 — pace ×1.10

| Measure | Mean | Standard deviation |
| :----- | ------: | ---------: |
| the current line, among 4000 subtitles | 7.94 µs | 2.74 µs |
| composing a two-line line | 168 ns | 2.88 ns |
| opening a video | 10.1 ms | 721 µs |
| seeking a position | 644 µs | 103 µs |
| building the model over 4000 subtitles | 9.99 µs | 6.36 µs |
| a window of 40 rows, five columns | 21.4 µs | 1.96 µs |
| rafraîchir après un shifting 4000 subtitles | 13.7 µs | 6.33 µs |
| resetting the model after a row is removed | 9.78 µs | 4.8 µs |
| editing a text cell | 437 ns | 162 ns |
| editing a position cell | 10.6 µs | 1.56 µs |
| splitting 4000 subtitles, in ems, with a length cache | 191 ms | 10.7 ms |
| versionString | 39.1 ns | 0.695 ns |
| parse | 40.7 ns | 1.93 ns |
| format | 33.7 ns | 2.23 ns |
| position to frame | 6.48 ns | 0.0716 ns |
| frame to position | 7.25 ns | 0.131 ns |
| scaling by an exact rational | 7.7 ns | 0.0825 ns |
| splitting 4000 subtitles, without a length cache | 120 ms | 7.79 ms |
| splitting 4000 subtitles, with a length cache | 124 ms | 7.16 ms |
| common French errors over 4000 subtitles | 167 ms | 8.52 ms |
| common English errors over 4000 subtitles | 187 ms | 9.93 ms |
| reading 4000 subtitles | 2.24 ms | 149 µs |
| writing 4000 subtitles | 652 µs | 65.7 µs |
| shifting 4000 subtitles | 10.2 µs | 3.56 µs |
| shifting then undoing | 19.9 µs | 4.71 µs |
| transforming 4000 subtitles | 90.1 µs | 13.1 µs |
| frame rate conversion over 4000 subtitles | 68.5 µs | 11.2 µs |
| alignment over 4000 subtitles | 153 µs | 21.1 µs |
| sorting 4000 subtitles in reverse | 310 µs | 47.6 µs |
| removing every other subtitle | 680 µs | 764 µs |
| removal then undo | 250 µs | 52 µs |
| inserting 100 empty subtitles in the middle | 59.9 µs | 7.53 µs |
| editing a text, through a session | 176 ns | 18 ns |
| removing mentions over 4000 subtitles | 1.47 ms | 111 µs |
| italicising 4000 subtitles | 1 ms | 77.3 µs |
| replacing a frequent word over 4000 subtitles, plain text | 4.16 ms | 326 µs |
| replacing a frequent word over 4000 subtitles, regular expression | 4.43 ms | 489 µs |
| search with no result over 4000 subtitles | 3.18 ms | 421 µs |
| adjusting the durations of 4000 subtitles, four constraints | 1.49 ms | 261 µs |
| title case over 4000 subtitles | 20.4 ms | 1.83 ms |
| pasting 4000 texts | 1.18 ms | 112 µs |
| aligning a 4000-line translation, by position | 925 µs | 45.6 µs |
| aligning a 4000-line translation, by number | 842 µs | 68.3 µs |
| frame rate deduction over 4000 subtitles | 425 µs | 17.9 µs |
| walk of 200 files in a tree of depth 3 | 659 µs | 40.8 µs |
| shift of 200 files of 20 subtitles | 36.8 ms | 2.15 ms |
| shift of one file of 4000 subtitles | 3.27 ms | 213 µs |

### 0.13.33 — 2026-10-06 — Release — load 1.48 — pace ×0.95

| Measure | Mean | Standard deviation |
| :----- | ------: | ---------: |
| the current line, among 4000 subtitles | 5.93 µs | 317 ns |
| composing a two-line line | 284 ns | 5.57 ns |
| opening a video | 8.82 ms | 190 µs |
| seeking a position | 587 µs | 183 µs |
| building the model over 4000 subtitles | 8.36 µs | 1.83 µs |
| a window of 40 rows, five columns | 19.5 µs | 6.89 µs |
| rafraîchir après un shifting 4000 subtitles | 8.64 µs | 2.89 µs |
| resetting the model after a row is removed | 9.16 µs | 3.71 µs |
| editing a text cell | 395 ns | 23.7 ns |
| editing a position cell | 10.6 µs | 2.13 µs |
| splitting 4000 subtitles, in ems, with a length cache | 184 ms | 4.66 ms |
| versionString | 44.6 ns | 0.597 ns |
| parse | 37.6 ns | 8.08 ns |
| format | 38.6 ns | 12.2 ns |
| position to frame | 6.32 ns | 0.0485 ns |
| frame to position | 6.47 ns | 0.069 ns |
| scaling by an exact rational | 7.1 ns | 2.14 ns |
| splitting 4000 subtitles, without a length cache | 112 ms | 4.49 ms |
| splitting 4000 subtitles, with a length cache | 111 ms | 3.9 ms |
| common French errors over 4000 subtitles | 155 ms | 4.84 ms |
| common English errors over 4000 subtitles | 171 ms | 5.66 ms |
| reading 4000 subtitles | 2.01 ms | 59.6 µs |
| writing 4000 subtitles | 527 µs | 24.2 µs |
| shifting 4000 subtitles | 6.78 µs | 2.78 µs |
| shifting then undoing | 13.5 µs | 4.03 µs |
| transforming 4000 subtitles | 73.3 µs | 7.75 µs |
| frame rate conversion over 4000 subtitles | 73.6 µs | 16.3 µs |
| alignment over 4000 subtitles | 126 µs | 10.3 µs |
| sorting 4000 subtitles in reverse | 271 µs | 19.9 µs |
| removing every other subtitle | 165 µs | 25 µs |
| removal then undo | 245 µs | 30.4 µs |
| inserting 100 empty subtitles in the middle | 53.7 µs | 14.8 µs |
| editing a text, through a session | 177 ns | 13.6 ns |
| removing mentions over 4000 subtitles | 1.25 ms | 28.8 µs |
| italicising 4000 subtitles | 905 µs | 32.1 µs |
| replacing a frequent word over 4000 subtitles, plain text | 4 ms | 235 µs |
| replacing a frequent word over 4000 subtitles, regular expression | 4.04 ms | 113 µs |
| search with no result over 4000 subtitles | 2.62 ms | 116 µs |
| adjusting the durations of 4000 subtitles, four constraints | 1.21 ms | 58.8 µs |
| title case over 4000 subtitles | 18.3 ms | 685 µs |
| pasting 4000 texts | 1.14 ms | 133 µs |
| aligning a 4000-line translation, by position | 803 µs | 47.7 µs |
| aligning a 4000-line translation, by number | 828 µs | 66.1 µs |
| frame rate deduction over 4000 subtitles | 371 µs | 24 µs |
| walk of 200 files in a tree of depth 3 | 574 µs | 42.6 µs |
| shift of 200 files of 20 subtitles | 33.2 ms | 1.84 ms |
| shift of one file of 4000 subtitles | 2.89 ms | 114 µs |

### 0.13.27 — 2026-10-05 — Release — load 1.39 — pace ×1.16

| Measure | Mean | Standard deviation |
| :----- | ------: | ---------: |
| the current line, among 4000 subtitles | 7.72 µs | 5.33 µs |
| composing a two-line line | 213 ns | 4.01 ns |
| opening a video | 9.26 ms | 501 µs |
| seeking a position | 574 µs | 83.3 µs |
| building the model over 4000 subtitles | 10.6 µs | 3.89 µs |
| a window of 40 rows, five columns | 20.1 µs | 962 ns |
| rafraîchir après un shifting 4000 subtitles | 9.89 µs | 3.56 µs |
| resetting the model after a row is removed | 10.4 µs | 7.39 µs |
| editing a text cell | 518 ns | 284 ns |
| editing a position cell | 11.4 µs | 798 ns |
| splitting 4000 subtitles, in ems, with a length cache | 209 ms | 2.83 ms |
| versionString | 51.7 ns | 0.625 ns |
| parse | 43.6 ns | 2.97 ns |
| format | 40.3 ns | 0.724 ns |
| position to frame | 7.81 ns | 0.108 ns |
| frame to position | 7.63 ns | 0.171 ns |
| scaling by an exact rational | 8.22 ns | 1.28 ns |
| splitting 4000 subtitles, without a length cache | 128 ms | 1.44 ms |
| splitting 4000 subtitles, with a length cache | 133 ms | 1.83 ms |
| common French errors over 4000 subtitles | 179 ms | 2.45 ms |
| common English errors over 4000 subtitles | 200 ms | 2.2 ms |
| reading 4000 subtitles | 2.44 ms | 149 µs |
| writing 4000 subtitles | 628 µs | 49.4 µs |
| shifting 4000 subtitles | 8.2 µs | 2.54 µs |
| shifting then undoing | 15.9 µs | 5.06 µs |
| transforming 4000 subtitles | 87.3 µs | 16.8 µs |
| frame rate conversion over 4000 subtitles | 81.2 µs | 14.1 µs |
| alignment over 4000 subtitles | 147 µs | 10.5 µs |
| sorting 4000 subtitles in reverse | 321 µs | 43.2 µs |
| removing every other subtitle | 191 µs | 24.3 µs |
| removal then undo | 294 µs | 43.5 µs |
| inserting 100 empty subtitles in the middle | 64.5 µs | 12.3 µs |
| editing a text, through a session | 225 ns | 115 ns |
| removing mentions over 4000 subtitles | 1.43 ms | 182 µs |
| italicising 4000 subtitles | 1.07 ms | 132 µs |
| replacing a frequent word over 4000 subtitles, plain text | 4.7 ms | 243 µs |
| replacing a frequent word over 4000 subtitles, regular expression | 4.82 ms | 165 µs |
| search with no result over 4000 subtitles | 3.12 ms | 198 µs |
| adjusting the durations of 4000 subtitles, four constraints | 1.45 ms | 92.3 µs |
| title case over 4000 subtitles | 21.1 ms | 437 µs |
| pasting 4000 texts | 1.21 ms | 173 µs |
| aligning a 4000-line translation, by position | 991 µs | 64.7 µs |
| aligning a 4000-line translation, by number | 928 µs | 71.9 µs |
| frame rate deduction over 4000 subtitles | 428 µs | 26.9 µs |
| walk of 200 files in a tree of depth 3 | 675 µs | 98.3 µs |
| shift of 200 files of 20 subtitles | 37.9 ms | 359 µs |
| shift of one file of 4000 subtitles | 3.58 ms | 257 µs |

### 0.13.25 — 2026-10-05 — Release — load 1.39 — pace ×0.98

| Measure | Mean | Standard deviation |
| :----- | ------: | ---------: |
| the current line, among 4000 subtitles | 5.69 µs | 246 ns |
| composing a two-line line | 177 ns | 2.8 ns |
| opening a video | 8.98 ms | 340 µs |
| seeking a position | 592 µs | 189 µs |
| building the model over 4000 subtitles | 8.23 µs | 437 ns |
| a window of 40 rows, five columns | 17.6 µs | 535 ns |
| rafraîchir après un shifting 4000 subtitles | 8.18 µs | 144 ns |
| resetting the model after a row is removed | 8.23 µs | 756 ns |
| editing a text cell | 404 ns | 60.7 ns |
| editing a position cell | 9.23 µs | 535 ns |
| splitting 4000 subtitles, in ems, with a length cache | 183 ms | 5.31 ms |
| versionString | 47.3 ns | 0.71 ns |
| parse | 35.9 ns | 2.97 ns |
| format | 35.3 ns | 0.616 ns |
| position to frame | 6.48 ns | 0.0673 ns |
| frame to position | 6.32 ns | 0.0481 ns |
| scaling by an exact rational | 6.73 ns | 0.128 ns |
| splitting 4000 subtitles, without a length cache | 109 ms | 3.23 ms |
| splitting 4000 subtitles, with a length cache | 112 ms | 1.64 ms |
| common French errors over 4000 subtitles | 157 ms | 3.6 ms |
| common English errors over 4000 subtitles | 172 ms | 4.41 ms |
| reading 4000 subtitles | 2.03 ms | 72.8 µs |
| writing 4000 subtitles | 530 µs | 44 µs |
| shifting 4000 subtitles | 6.56 µs | 2.74 µs |
| shifting then undoing | 12.7 µs | 2.87 µs |
| transforming 4000 subtitles | 71 µs | 5.6 µs |
| frame rate conversion over 4000 subtitles | 68.3 µs | 11.9 µs |
| alignment over 4000 subtitles | 123 µs | 6.77 µs |
| sorting 4000 subtitles in reverse | 276 µs | 30.4 µs |
| removing every other subtitle | 160 µs | 21 µs |
| removal then undo | 244 µs | 37.2 µs |
| inserting 100 empty subtitles in the middle | 52.1 µs | 14.6 µs |
| editing a text, through a session | 178 ns | 16.1 ns |
| removing mentions over 4000 subtitles | 1.21 ms | 50.2 µs |
| italicising 4000 subtitles | 906 µs | 43.2 µs |
| replacing a frequent word over 4000 subtitles, plain text | 3.92 ms | 128 µs |
| replacing a frequent word over 4000 subtitles, regular expression | 4.29 ms | 247 µs |
| search with no result over 4000 subtitles | 2.65 ms | 93 µs |
| adjusting the durations of 4000 subtitles, four constraints | 1.25 ms | 59.1 µs |
| title case over 4000 subtitles | 17.8 ms | 235 µs |
| pasting 4000 texts | 978 µs | 41.8 µs |
| aligning a 4000-line translation, by position | 806 µs | 43.8 µs |
| aligning a 4000-line translation, by number | 796 µs | 50.8 µs |
| frame rate deduction over 4000 subtitles | 440 µs | 10.6 µs |
| walk of 200 files in a tree of depth 3 | 550 µs | 12.2 µs |
| shift of 200 files of 20 subtitles | 32.3 ms | 1.8 ms |
| shift of one file of 4000 subtitles | 3.12 ms | 231 µs |

### 0.13.21 — 2026-10-05 — Release — load 1.39 — pace ×1.02

| Measure | Mean | Standard deviation |
| :----- | ------: | ---------: |
| the current line, among 4000 subtitles | 7.36 µs | 2.1 µs |
| composing a two-line line | 169 ns | 2.88 ns |
| opening a video | 9.21 ms | 400 µs |
| seeking a position | 589 µs | 187 µs |
| building the model over 4000 subtitles | 9.89 µs | 4.36 µs |
| a window of 40 rows, five columns | 17.8 µs | 3.22 µs |
| rafraîchir après un shifting 4000 subtitles | 8.37 µs | 2.31 µs |
| resetting the model after a row is removed | 8.49 µs | 2.53 µs |
| editing a text cell | 393 ns | 49.1 ns |
| editing a position cell | 9.2 µs | 651 ns |
| splitting 4000 subtitles, in ems, with a length cache | 189 ms | 10.2 ms |
| versionString | 53.2 ns | 0.851 ns |
| parse | 39.2 ns | 10.1 ns |
| format | 39.8 ns | 0.816 ns |
| position to frame | 8.21 ns | 0.788 ns |
| frame to position | 6.65 ns | 0.0761 ns |
| scaling by an exact rational | 7.04 ns | 1.22 ns |
| splitting 4000 subtitles, without a length cache | 121 ms | 10.6 ms |
| splitting 4000 subtitles, with a length cache | 134 ms | 8.93 ms |
| common French errors over 4000 subtitles | 169 ms | 10.5 ms |
| common English errors over 4000 subtitles | 180 ms | 9.61 ms |
| reading 4000 subtitles | 2.02 ms | 170 µs |
| writing 4000 subtitles | 521 µs | 33.8 µs |
| shifting 4000 subtitles | 7.77 µs | 4.21 µs |
| shifting then undoing | 14.5 µs | 1.61 µs |
| transforming 4000 subtitles | 87.9 µs | 8.03 µs |
| frame rate conversion over 4000 subtitles | 77.7 µs | 11.2 µs |
| alignment over 4000 subtitles | 127 µs | 18.6 µs |
| sorting 4000 subtitles in reverse | 275 µs | 27.9 µs |
| removing every other subtitle | 184 µs | 16.6 µs |
| removal then undo | 283 µs | 25.1 µs |
| inserting 100 empty subtitles in the middle | 53 µs | 16.9 µs |
| editing a text, through a session | 180 ns | 21.5 ns |
| removing mentions over 4000 subtitles | 1.36 ms | 103 µs |
| italicising 4000 subtitles | 987 µs | 95.9 µs |
| replacing a frequent word over 4000 subtitles, plain text | 4.47 ms | 175 µs |
| replacing a frequent word over 4000 subtitles, regular expression | 4.47 ms | 277 µs |
| search with no result over 4000 subtitles | 2.72 ms | 197 µs |
| adjusting the durations of 4000 subtitles, four constraints | 1.35 ms | 189 µs |
| title case over 4000 subtitles | 19.9 ms | 1.2 ms |
| pasting 4000 texts | 991 µs | 68.2 µs |
| aligning a 4000-line translation, by position | 850 µs | 70.9 µs |
| aligning a 4000-line translation, by number | 896 µs | 43.8 µs |
| frame rate deduction over 4000 subtitles | 368 µs | 15.1 µs |
| walk of 200 files in a tree of depth 3 | 567 µs | 41.7 µs |
| shift of 200 files of 20 subtitles | 36.3 ms | 1.89 ms |
| shift of one file of 4000 subtitles | 3.02 ms | 300 µs |

### 0.13.14 — 2026-10-02 — Release — load 1.41 — pace ×0.95

| Measure | Mean | Standard deviation |
| :----- | ------: | ---------: |
| the current line, among 4000 subtitles | 5.49 µs | 1.36 µs |
| composing a two-line line | 167 ns | 3.36 ns |
| opening a video | 9.16 ms | 551 µs |
| seeking a position | 588 µs | 183 µs |
| building the model over 4000 subtitles | 9.39 µs | 6.64 µs |
| a window of 40 rows, five columns | 18.4 µs | 4.5 µs |
| rafraîchir après un shifting 4000 subtitles | 8.57 µs | 3.55 µs |
| resetting the model after a row is removed | 8.73 µs | 3.93 µs |
| editing a text cell | 434 ns | 177 ns |
| editing a position cell | 9.84 µs | 3.39 µs |
| splitting 4000 subtitles, in ems, with a length cache | 186 ms | 6.39 ms |
| versionString | 46.2 ns | 9.4 ns |
| parse | 42.4 ns | 20.4 ns |
| format | 35.3 ns | 0.485 ns |
| position to frame | 6.64 ns | 0.0738 ns |
| frame to position | 6.64 ns | 0.0527 ns |
| scaling by an exact rational | 7.86 ns | 0.0699 ns |
| splitting 4000 subtitles, without a length cache | 115 ms | 4.95 ms |
| splitting 4000 subtitles, with a length cache | 113 ms | 3.61 ms |
| common French errors over 4000 subtitles | 159 ms | 6.03 ms |
| common English errors over 4000 subtitles | 175 ms | 5.3 ms |
| reading 4000 subtitles | 2.19 ms | 164 µs |
| writing 4000 subtitles | 526 µs | 36 µs |
| shifting 4000 subtitles | 7.82 µs | 1.4 µs |
| shifting then undoing | 13.7 µs | 2.67 µs |
| transforming 4000 subtitles | 71.2 µs | 12 µs |
| frame rate conversion over 4000 subtitles | 71.6 µs | 8.09 µs |
| alignment over 4000 subtitles | 127 µs | 15.3 µs |
| sorting 4000 subtitles in reverse | 285 µs | 35.8 µs |
| removing every other subtitle | 173 µs | 34.1 µs |
| removal then undo | 246 µs | 33.1 µs |
| inserting 100 empty subtitles in the middle | 53 µs | 13.3 µs |
| editing a text, through a session | 195 ns | 40.9 ns |
| removing mentions over 4000 subtitles | 1.28 ms | 62.8 µs |
| italicising 4000 subtitles | 930 µs | 128 µs |
| replacing a frequent word over 4000 subtitles, plain text | 4.14 ms | 296 µs |
| replacing a frequent word over 4000 subtitles, regular expression | 4.3 ms | 293 µs |
| search with no result over 4000 subtitles | 2.63 ms | 125 µs |
| adjusting the durations of 4000 subtitles, four constraints | 1.23 ms | 39.3 µs |
| title case over 4000 subtitles | 18.8 ms | 856 µs |
| pasting 4000 texts | 991 µs | 57.1 µs |
| aligning a 4000-line translation, by position | 875 µs | 71.8 µs |
| aligning a 4000-line translation, by number | 815 µs | 59.7 µs |
| frame rate deduction over 4000 subtitles | 374 µs | 25.9 µs |
| walk of 200 files in a tree of depth 3 | 571 µs | 35.9 µs |
| shift of 200 files of 20 subtitles | 34.6 ms | 1.57 ms |
| shift of one file of 4000 subtitles | 2.94 ms | 195 µs |

### 0.13.13 — 2026-10-02 — Release — load 1.39 — pace ×0.92

| Measure | Mean | Standard deviation |
| :----- | ------: | ---------: |
| the current line, among 4000 subtitles | 7.3 µs | 1.73 µs |
| composing a two-line line | 205 ns | 75.5 ns |
| opening a video | 9.24 ms | 461 µs |
| seeking a position | 588 µs | 190 µs |
| building the model over 4000 subtitles | 7.32 µs | 2.7 µs |
| a window of 40 rows, five columns | 18.5 µs | 6.39 µs |
| rafraîchir après un shifting 4000 subtitles | 7.32 µs | 2.75 µs |
| resetting the model after a row is removed | 7.33 µs | 2.97 µs |
| editing a text cell | 393 ns | 58.3 ns |
| editing a position cell | 8.21 µs | 930 ns |
| splitting 4000 subtitles, in ems, with a length cache | 179 ms | 5.69 ms |
| versionString | 44.4 ns | 0.599 ns |
| parse | 41.4 ns | 3.07 ns |
| format | 34.2 ns | 0.447 ns |
| position to frame | 6.32 ns | 0.0681 ns |
| frame to position | 6.47 ns | 0.073 ns |
| scaling by an exact rational | 6.71 ns | 0.0507 ns |
| splitting 4000 subtitles, without a length cache | 113 ms | 4.29 ms |
| splitting 4000 subtitles, with a length cache | 116 ms | 4.24 ms |
| common French errors over 4000 subtitles | 158 ms | 5.18 ms |
| common English errors over 4000 subtitles | 176 ms | 5.21 ms |
| reading 4000 subtitles | 2.15 ms | 185 µs |
| writing 4000 subtitles | 545 µs | 70.7 µs |
| shifting 4000 subtitles | 6.63 µs | 2.57 µs |
| shifting then undoing | 12.7 µs | 2.95 µs |
| transforming 4000 subtitles | 67.5 µs | 5.73 µs |
| frame rate conversion over 4000 subtitles | 66.5 µs | 7.2 µs |
| alignment over 4000 subtitles | 124 µs | 7.61 µs |
| sorting 4000 subtitles in reverse | 273 µs | 26.5 µs |
| removing every other subtitle | 163 µs | 22.7 µs |
| removal then undo | 243 µs | 30.1 µs |
| inserting 100 empty subtitles in the middle | 52.8 µs | 14.7 µs |
| editing a text, through a session | 173 ns | 18.5 ns |
| removing mentions over 4000 subtitles | 1.28 ms | 60.4 µs |
| italicising 4000 subtitles | 952 µs | 93.8 µs |
| replacing a frequent word over 4000 subtitles, plain text | 4.08 ms | 267 µs |
| replacing a frequent word over 4000 subtitles, regular expression | 4.24 ms | 323 µs |
| search with no result over 4000 subtitles | 2.67 ms | 133 µs |
| adjusting the durations of 4000 subtitles, four constraints | 1.25 ms | 54.9 µs |
| title case over 4000 subtitles | 18.6 ms | 938 µs |
| pasting 4000 texts | 1.09 ms | 125 µs |
| aligning a 4000-line translation, by position | 819 µs | 55.7 µs |
| aligning a 4000-line translation, by number | 809 µs | 60.9 µs |
| frame rate deduction over 4000 subtitles | 377 µs | 29.6 µs |
| walk of 200 files in a tree of depth 3 | 593 µs | 45.3 µs |
| shift of 200 files of 20 subtitles | 34.2 ms | 1.66 ms |
| shift of one file of 4000 subtitles | 3.24 ms | 387 µs |

### 0.13.10 — 2026-10-01 — Release — load 1.46 — pace ×1.06

| Measure | Mean | Standard deviation |
| :----- | ------: | ---------: |
| the current line, among 4000 subtitles | 4.91 µs | 1.94 µs |
| composing a two-line line | 166 ns | 3.61 ns |
| opening a video | 9.11 ms | 471 µs |
| seeking a position | 596 µs | 166 µs |
| building the model over 4000 subtitles | 8.45 µs | 2.92 µs |
| a window of 40 rows, five columns | 21.3 µs | 5.45 µs |
| rafraîchir après un shifting 4000 subtitles | 8.39 µs | 2.21 µs |
| resetting the model after a row is removed | 8.39 µs | 2.46 µs |
| editing a text cell | 461 ns | 57 ns |
| editing a position cell | 9.34 µs | 676 ns |
| splitting 4000 subtitles, in ems, with a length cache | 196 ms | 6.56 ms |
| versionString | 44.3 ns | 0.874 ns |
| parse | 41.7 ns | 3.28 ns |
| format | 41.2 ns | 1.28 ns |
| position to frame | 7.62 ns | 0.0821 ns |
| frame to position | 7.62 ns | 0.0795 ns |
| scaling by an exact rational | 7.88 ns | 0.115 ns |
| splitting 4000 subtitles, without a length cache | 120 ms | 6.3 ms |
| splitting 4000 subtitles, with a length cache | 123 ms | 8.3 ms |
| common French errors over 4000 subtitles | 167 ms | 7.1 ms |
| common English errors over 4000 subtitles | 194 ms | 16.3 ms |
| reading 4000 subtitles | 2.52 ms | 188 µs |
| writing 4000 subtitles | 630 µs | 44.8 µs |
| shifting 4000 subtitles | 9.61 µs | 3.89 µs |
| shifting then undoing | 17.9 µs | 2.31 µs |
| transforming 4000 subtitles | 85.3 µs | 7.25 µs |
| frame rate conversion over 4000 subtitles | 79.9 µs | 5.58 µs |
| alignment over 4000 subtitles | 152 µs | 8.42 µs |
| sorting 4000 subtitles in reverse | 339 µs | 52.4 µs |
| removing every other subtitle | 528 µs | 185 µs |
| removal then undo | 1.9 ms | 1.95 ms |
| inserting 100 empty subtitles in the middle | 420 µs | 307 µs |
| editing a text, through a session | 581 ns | 142 ns |
| removing mentions over 4000 subtitles | 3.18 ms | 303 µs |
| italicising 4000 subtitles | 3.36 ms | 1.02 ms |
| replacing a frequent word over 4000 subtitles, plain text | 14 ms | 9.31 ms |
| replacing a frequent word over 4000 subtitles, regular expression | 11.9 ms | 1.36 ms |
| search with no result over 4000 subtitles | 8.01 ms | 461 µs |
| adjusting the durations of 4000 subtitles, four constraints | 4.03 ms | 493 µs |
| title case over 4000 subtitles | 55.6 ms | 4.42 ms |
| pasting 4000 texts | 1.12 ms | 70 µs |
| aligning a 4000-line translation, by position | 954 µs | 172 µs |
| aligning a 4000-line translation, by number | 948 µs | 173 µs |
| frame rate deduction over 4000 subtitles | 449 µs | 69.8 µs |
| walk of 200 files in a tree of depth 3 | 652 µs | 78.6 µs |
| shift of 200 files of 20 subtitles | 38.5 ms | 2.83 ms |
| shift of one file of 4000 subtitles | 3.6 ms | 333 µs |

### 0.13.0 — 2026-10-01 — Release — load 0.85 — pace ×1.02

| Measure | Mean | Standard deviation |
| :----- | ------: | ---------: |
| the current line, among 4000 subtitles | 8.76 µs | 1.9 µs |
| composing a two-line line | 162 ns | 3.92 ns |
| opening a video | 9.19 ms | 690 µs |
| seeking a position | 624 µs | 155 µs |
| building the model over 4000 subtitles | 10.2 µs | 4.78 µs |
| a window of 40 rows, five columns | 20.2 µs | 4.54 µs |
| rafraîchir après un shifting 4000 subtitles | 9.77 µs | 1.89 µs |
| resetting the model after a row is removed | 9.15 µs | 7.72 µs |
| editing a text cell | 416 ns | 139 ns |
| editing a position cell | 11.5 µs | 1.91 µs |
| splitting 4000 subtitles, in ems, with a length cache | 202 ms | 14.4 ms |
| versionString | 44.7 ns | 0.674 ns |
| parse | 36.9 ns | 3.21 ns |
| format | 37.5 ns | 0.4 ns |
| position to frame | 7.31 ns | 0.102 ns |
| frame to position | 7.86 ns | 2.14 ns |
| scaling by an exact rational | 7.43 ns | 2.49 ns |
| splitting 4000 subtitles, without a length cache | 123 ms | 12.4 ms |
| splitting 4000 subtitles, with a length cache | 125 ms | 7.31 ms |
| common French errors over 4000 subtitles | 169 ms | 11.7 ms |
| common English errors over 4000 subtitles | 188 ms | 15.8 ms |
| reading 4000 subtitles | 2.38 ms | 223 µs |
| writing 4000 subtitles | 590 µs | 65.6 µs |
| shifting 4000 subtitles | 7.47 µs | 2.86 µs |
| shifting then undoing | 12.8 µs | 3.43 µs |
| transforming 4000 subtitles | 72.2 µs | 7.72 µs |
| frame rate conversion over 4000 subtitles | 69.2 µs | 11.3 µs |
| alignment over 4000 subtitles | 126 µs | 13.9 µs |
| sorting 4000 subtitles in reverse | 310 µs | 23.5 µs |
| removing every other subtitle | 172 µs | 41.8 µs |
| removal then undo | 272 µs | 53 µs |
| inserting 100 empty subtitles in the middle | 52.4 µs | 15.7 µs |
| editing a text, through a session | 175 ns | 16.6 ns |
| removing mentions over 4000 subtitles | 1.39 ms | 72.6 µs |
| italicising 4000 subtitles | 1.03 ms | 90.4 µs |
| replacing a frequent word over 4000 subtitles, plain text | 5.21 ms | 992 µs |
| replacing a frequent word over 4000 subtitles, regular expression | 4.71 ms | 509 µs |
| search with no result over 4000 subtitles | 3.03 ms | 318 µs |
| adjusting the durations of 4000 subtitles, four constraints | 1.25 ms | 56.1 µs |
| title case over 4000 subtitles | 20.1 ms | 1.09 ms |
| pasting 4000 texts | 1.1 ms | 27.4 µs |
| aligning a 4000-line translation, by position | 811 µs | 44.9 µs |
| aligning a 4000-line translation, by number | 802 µs | 50.1 µs |
| frame rate deduction over 4000 subtitles | 429 µs | 21 µs |

### 0.12.23 — 2026-09-30 — Release — load 1.46 — pace ×0.97

| Measure | Mean | Standard deviation |
| :----- | ------: | ---------: |
| the current line, among 4000 subtitles | 7.23 µs | 174 ns |
| composing a two-line line | 162 ns | 4.02 ns |
| opening a video | 9.29 ms | 643 µs |
| seeking a position | 575 µs | 168 µs |
| building the model over 4000 subtitles | 9.8 µs | 4.99 µs |
| a window of 40 rows, five columns | 21.5 µs | 5.87 µs |
| rafraîchir après un shifting 4000 subtitles | 8.57 µs | 3.75 µs |
| resetting the model after a row is removed | 9.54 µs | 1.48 µs |
| editing a text cell | 426 ns | 166 ns |
| editing a position cell | 10.3 µs | 3.94 µs |
| splitting 4000 subtitles, in ems, with a length cache | 188 ms | 6.09 ms |
| versionString | 43.9 ns | 0.557 ns |
| parse | 36.7 ns | 2.58 ns |
| format | 38.1 ns | 0.751 ns |
| position to frame | 6.32 ns | 0.0477 ns |
| frame to position | 6.47 ns | 0.0669 ns |
| scaling by an exact rational | 6.72 ns | 0.0752 ns |
| splitting 4000 subtitles, without a length cache | 114 ms | 3.35 ms |
| splitting 4000 subtitles, with a length cache | 117 ms | 4.03 ms |
| common French errors over 4000 subtitles | 162 ms | 3.95 ms |
| common English errors over 4000 subtitles | 178 ms | 7.37 ms |
| reading 4000 subtitles | 2.11 ms | 71.3 µs |
| writing 4000 subtitles | 518 µs | 13.6 µs |
| shifting 4000 subtitles | 7.04 µs | 3.63 µs |
| shifting then undoing | 13.6 µs | 3.81 µs |
| transforming 4000 subtitles | 73.7 µs | 5.69 µs |
| frame rate conversion over 4000 subtitles | 77.3 µs | 11 µs |
| alignment over 4000 subtitles | 130 µs | 15.6 µs |
| sorting 4000 subtitles in reverse | 286 µs | 34.7 µs |
| removing every other subtitle | 166 µs | 16.1 µs |
| removal then undo | 249 µs | 25 µs |
| inserting 100 empty subtitles in the middle | 55.1 µs | 19.7 µs |
| editing a text, through a session | 178 ns | 11.8 ns |
| removing mentions over 4000 subtitles | 1.24 ms | 44 µs |
| italicising 4000 subtitles | 945 µs | 44.4 µs |
| replacing a frequent word over 4000 subtitles, plain text | 4.08 ms | 80.7 µs |
| replacing a frequent word over 4000 subtitles, regular expression | 4.26 ms | 91.9 µs |
| search with no result over 4000 subtitles | 2.9 ms | 407 µs |
| adjusting the durations of 4000 subtitles, four constraints | 1.28 ms | 44.2 µs |
| title case over 4000 subtitles | 18.9 ms | 711 µs |
| pasting 4000 texts | 1.01 ms | 56.7 µs |
| aligning a 4000-line translation, by position | 836 µs | 36.4 µs |
| aligning a 4000-line translation, by number | 820 µs | 31.2 µs |
| frame rate deduction over 4000 subtitles | 373 µs | 21.9 µs |

### 0.12.19 — 2026-09-30 — Release — load 1.44 — pace ×1.20

| Measure | Mean | Standard deviation |
| :----- | ------: | ---------: |
| the current line, among 4000 subtitles | 7.01 µs | 1.19 µs |
| composing a two-line line | 203 ns | 4.38 ns |
| opening a video | 9.47 ms | 578 µs |
| seeking a position | 574 µs | 115 µs |
| building the model over 4000 subtitles | 8.71 µs | 1.58 µs |
| a window of 40 rows, five columns | 20.9 µs | 1.06 µs |
| rafraîchir après un shifting 4000 subtitles | 8.85 µs | 3.69 µs |
| resetting the model after a row is removed | 8.74 µs | 2.3 µs |
| editing a text cell | 496 ns | 49.9 ns |
| editing a position cell | 10.5 µs | 2.39 µs |
| splitting 4000 subtitles, in ems, with a length cache | 212 ms | 2.83 ms |
| versionString | 55 ns | 4.58 ns |
| parse | 43.8 ns | 2.76 ns |
| format | 44.3 ns | 0.98 ns |
| position to frame | 8.15 ns | 0.623 ns |
| frame to position | 8.03 ns | 0.221 ns |
| scaling by an exact rational | 8.12 ns | 1.61 ns |
| splitting 4000 subtitles, without a length cache | 132 ms | 4.42 ms |
| splitting 4000 subtitles, with a length cache | 138 ms | 6.54 ms |
| common French errors over 4000 subtitles | 187 ms | 4.35 ms |
| common English errors over 4000 subtitles | 207 ms | 6.74 ms |
| reading 4000 subtitles | 2.47 ms | 243 µs |
| writing 4000 subtitles | 708 µs | 241 µs |
| shifting 4000 subtitles | 9.47 µs | 2.34 µs |
| shifting then undoing | 18.2 µs | 4.32 µs |
| transforming 4000 subtitles | 91 µs | 15.7 µs |
| frame rate conversion over 4000 subtitles | 85.3 µs | 8.91 µs |
| alignment over 4000 subtitles | 159 µs | 23.8 µs |
| sorting 4000 subtitles in reverse | 402 µs | 116 µs |
| removing every other subtitle | 201 µs | 24.3 µs |
| removal then undo | 291 µs | 34.2 µs |
| inserting 100 empty subtitles in the middle | 68.8 µs | 19.1 µs |
| editing a text, through a session | 332 ns | 185 ns |
| removing mentions over 4000 subtitles | 1.46 ms | 88 µs |
| italicising 4000 subtitles | 1.12 ms | 150 µs |
| replacing a frequent word over 4000 subtitles, plain text | 4.78 ms | 220 µs |
| replacing a frequent word over 4000 subtitles, regular expression | 5.55 ms | 1.32 ms |
| search with no result over 4000 subtitles | 3.71 ms | 697 µs |
| adjusting the durations of 4000 subtitles, four constraints | 1.51 ms | 149 µs |
| title case over 4000 subtitles | 21.8 ms | 539 µs |
| pasting 4000 texts | 1.29 ms | 201 µs |
| aligning a 4000-line translation, by position | 1.04 ms | 137 µs |
| aligning a 4000-line translation, by number | 995 µs | 72.1 µs |
| frame rate deduction over 4000 subtitles | 451 µs | 32.1 µs |

### 0.12.12 — 2026-09-29 — Release — load 1.39 — pace ×1.05

| Measure | Mean | Standard deviation |
| :----- | ------: | ---------: |
| the current line, among 4000 subtitles | 6.88 µs | 1.77 µs |
| composing a two-line line | 179 ns | 2.61 ns |
| opening a video | 9.31 ms | 626 µs |
| seeking a position | 610 µs | 184 µs |
| building the model over 4000 subtitles | 8.44 µs | 2.81 µs |
| a window of 40 rows, five columns | 18.1 µs | 3.22 µs |
| rafraîchir après un shifting 4000 subtitles | 7.49 µs | 2.64 µs |
| resetting the model after a row is removed | 7.27 µs | 2.57 µs |
| editing a text cell | 463 ns | 99.3 ns |
| editing a position cell | 8.48 µs | 1.95 µs |
| splitting 4000 subtitles, in ems, with a length cache | 191 ms | 5.09 ms |
| versionString | 44.4 ns | 1.02 ns |
| parse | 37.6 ns | 2.63 ns |
| format | 40.1 ns | 15.6 ns |
| position to frame | 7.14 ns | 2.51 ns |
| frame to position | 6.83 ns | 1.11 ns |
| scaling by an exact rational | 6.88 ns | 0.127 ns |
| splitting 4000 subtitles, without a length cache | 118 ms | 3.67 ms |
| splitting 4000 subtitles, with a length cache | 122 ms | 3.74 ms |
| common French errors over 4000 subtitles | 166 ms | 4.49 ms |
| common English errors over 4000 subtitles | 184 ms | 4.73 ms |
| reading 4000 subtitles | 2.24 ms | 174 µs |
| writing 4000 subtitles | 589 µs | 24.8 µs |
| shifting 4000 subtitles | 6.59 µs | 3.1 µs |
| shifting then undoing | 14.7 µs | 2.91 µs |
| transforming 4000 subtitles | 72.1 µs | 13 µs |
| frame rate conversion over 4000 subtitles | 76.4 µs | 7.69 µs |
| alignment over 4000 subtitles | 136 µs | 25.6 µs |
| sorting 4000 subtitles in reverse | 302 µs | 23.6 µs |
| removing every other subtitle | 189 µs | 24.8 µs |
| removal then undo | 255 µs | 32.3 µs |
| inserting 100 empty subtitles in the middle | 56 µs | 22.1 µs |
| editing a text, through a session | 179 ns | 24.9 ns |
| removing mentions over 4000 subtitles | 1.37 ms | 80 µs |
| italicising 4000 subtitles | 988 µs | 71.6 µs |
| replacing a frequent word over 4000 subtitles, plain text | 4.23 ms | 206 µs |
| replacing a frequent word over 4000 subtitles, regular expression | 4.42 ms | 235 µs |
| search with no result over 4000 subtitles | 3.01 ms | 173 µs |
| adjusting the durations of 4000 subtitles, four constraints | 1.35 ms | 84.2 µs |
| title case over 4000 subtitles | 19.4 ms | 862 µs |
| pasting 4000 texts | 1.04 ms | 125 µs |
| aligning a 4000-line translation, by position | 872 µs | 66.1 µs |
| aligning a 4000-line translation, by number | 832 µs | 51.7 µs |
| frame rate deduction over 4000 subtitles | 374 µs | 20.5 µs |

### 0.12.9 — 2026-09-28 — Release — load 1.41 — pace ×1.01

| Measure | Mean | Standard deviation |
| :----- | ------: | ---------: |
| the current line, among 4000 subtitles | 7.28 µs | 674 ns |
| composing a two-line line | 196 ns | 77.9 ns |
| opening a video | 8.96 ms | 183 µs |
| seeking a position | 592 µs | 171 µs |
| building the model over 4000 subtitles | 8.18 µs | 614 ns |
| a window of 40 rows, five columns | 17.1 µs | 258 ns |
| rafraîchir après un shifting 4000 subtitles | 8.16 µs | 649 ns |
| resetting the model after a row is removed | 8.15 µs | 617 ns |
| editing a text cell | 382 ns | 27.5 ns |
| editing a position cell | 9.58 µs | 2.51 µs |
| splitting 4000 subtitles, in ems, with a length cache | 177 ms | 2.98 ms |
| versionString | 39.3 ns | 0.479 ns |
| parse | 38.3 ns | 2.68 ns |
| format | 36 ns | 0.452 ns |
| position to frame | 6.65 ns | 0.0708 ns |
| frame to position | 6.64 ns | 0.0526 ns |
| scaling by an exact rational | 6.87 ns | 0.0726 ns |
| splitting 4000 subtitles, without a length cache | 110 ms | 1.52 ms |
| splitting 4000 subtitles, with a length cache | 113 ms | 3.36 ms |
| common French errors over 4000 subtitles | 155 ms | 4.2 ms |
| common English errors over 4000 subtitles | 179 ms | 9.96 ms |
| reading 4000 subtitles | 2.04 ms | 117 µs |
| writing 4000 subtitles | 560 µs | 76.5 µs |
| shifting 4000 subtitles | 6.83 µs | 3.07 µs |
| shifting then undoing | 12.8 µs | 3.61 µs |
| transforming 4000 subtitles | 78.7 µs | 14.1 µs |
| frame rate conversion over 4000 subtitles | 76.1 µs | 11.4 µs |
| alignment over 4000 subtitles | 126 µs | 11.6 µs |
| sorting 4000 subtitles in reverse | 270 µs | 23.7 µs |
| removing every other subtitle | 164 µs | 21.3 µs |
| removal then undo | 247 µs | 33.2 µs |
| inserting 100 empty subtitles in the middle | 56.8 µs | 21.3 µs |
| editing a text, through a session | 171 ns | 14.7 ns |
| removing mentions over 4000 subtitles | 1.55 ms | 103 µs |
| italicising 4000 subtitles | 946 µs | 56.8 µs |
| replacing a frequent word over 4000 subtitles, plain text | 4.14 ms | 238 µs |
| replacing a frequent word over 4000 subtitles, regular expression | 4.28 ms | 215 µs |
| search with no result over 4000 subtitles | 2.73 ms | 135 µs |
| adjusting the durations of 4000 subtitles, four constraints | 1.28 ms | 56.7 µs |
| title case over 4000 subtitles | 18.5 ms | 763 µs |
| pasting 4000 texts | 1.01 ms | 60 µs |
| aligning a 4000-line translation, by position | 814 µs | 43.1 µs |
| aligning a 4000-line translation, by number | 899 µs | 51.6 µs |
| frame rate deduction over 4000 subtitles | 382 µs | 36.9 µs |

### 0.12.8 — 2026-09-27 — Release — load 1.18 — pace ×0.98

| Measure | Mean | Standard deviation |
| :----- | ------: | ---------: |
| the current line, among 4000 subtitles | 7.59 µs | 1.4 µs |
| composing a two-line line | 179 ns | 2.2 ns |
| opening a video | 8.95 ms | 177 µs |
| seeking a position | 563 µs | 146 µs |
| building the model over 4000 subtitles | 10.9 µs | 2.24 µs |
| a window of 40 rows, five columns | 17.1 µs | 266 ns |
| rafraîchir après un shifting 4000 subtitles | 10.5 µs | 1.73 µs |
| resetting the model after a row is removed | 10.5 µs | 2.12 µs |
| editing a text cell | 412 ns | 77.5 ns |
| editing a position cell | 11.7 µs | 1.31 µs |
| versionString | 41.2 ns | 8.61 ns |
| parse | 36.8 ns | 2.53 ns |
| format | 36.3 ns | 0.44 ns |
| position to frame | 6.66 ns | 0.195 ns |
| frame to position | 6.65 ns | 0.147 ns |
| scaling by an exact rational | 6.86 ns | 0.0506 ns |
| splitting 4000 subtitles, without a length cache | 109 ms | 3.52 ms |
| splitting 4000 subtitles, with a length cache | 116 ms | 7.2 ms |
| common French errors over 4000 subtitles | 161 ms | 9.4 ms |
| common English errors over 4000 subtitles | 176 ms | 11.5 ms |
| reading 4000 subtitles | 2.04 ms | 55.3 µs |
| writing 4000 subtitles | 525 µs | 26 µs |
| shifting 4000 subtitles | 7.74 µs | 3.1 µs |
| shifting then undoing | 15 µs | 3.94 µs |
| transforming 4000 subtitles | 75.7 µs | 10.2 µs |
| frame rate conversion over 4000 subtitles | 70.9 µs | 8.26 µs |
| alignment over 4000 subtitles | 130 µs | 15.9 µs |
| sorting 4000 subtitles in reverse | 268 µs | 30.8 µs |
| removing every other subtitle | 160 µs | 24.6 µs |
| removal then undo | 239 µs | 31.3 µs |
| inserting 100 empty subtitles in the middle | 55.5 µs | 20.6 µs |
| editing a text, through a session | 181 ns | 45.1 ns |
| removing mentions over 4000 subtitles | 1.25 ms | 47.9 µs |
| italicising 4000 subtitles | 885 µs | 33 µs |
| replacing a frequent word over 4000 subtitles, plain text | 3.9 ms | 81.1 µs |
| replacing a frequent word over 4000 subtitles, regular expression | 4.09 ms | 89.5 µs |
| search with no result over 4000 subtitles | 2.62 ms | 86.6 µs |
| adjusting the durations of 4000 subtitles, four constraints | 1.23 ms | 67.9 µs |
| title case over 4000 subtitles | 18.4 ms | 809 µs |
| pasting 4000 texts | 980 µs | 34.9 µs |
| aligning a 4000-line translation, by position | 828 µs | 39.3 µs |
| aligning a 4000-line translation, by number | 806 µs | 34.3 µs |
| frame rate deduction over 4000 subtitles | 369 µs | 11.1 µs |

### 0.12.7 — 2026-09-27 — Release — load 1.08 — pace ×0.95

| Measure | Mean | Standard deviation |
| :----- | ------: | ---------: |
| the current line, among 4000 subtitles | 7.08 µs | 214 ns |
| composing a two-line line | 162 ns | 2.03 ns |
| opening a video | 9.01 ms | 351 µs |
| seeking a position | 563 µs | 180 µs |
| building the model over 4000 subtitles | 7.13 µs | 494 ns |
| a window of 40 rows, five columns | 16.8 µs | 357 ns |
| rafraîchir après un shifting 4000 subtitles | 7.39 µs | 3.24 µs |
| resetting the model after a row is removed | 7.81 µs | 1.72 µs |
| editing a text cell | 395 ns | 38.6 ns |
| editing a position cell | 8.09 µs | 318 ns |
| versionString | 39.4 ns | 0.58 ns |
| parse | 37 ns | 2.72 ns |
| format | 34 ns | 5.89 ns |
| position to frame | 6.64 ns | 0.0686 ns |
| frame to position | 6.64 ns | 0.0699 ns |
| scaling by an exact rational | 6.86 ns | 0.0536 ns |
| common French errors over 4000 subtitles | 153 ms | 6.72 ms |
| common English errors over 4000 subtitles | 171 ms | 8.26 ms |
| reading 4000 subtitles | 2.24 ms | 157 µs |
| writing 4000 subtitles | 503 µs | 16.8 µs |
| shifting 4000 subtitles | 6.69 µs | 3.01 µs |
| shifting then undoing | 12.9 µs | 3.37 µs |
| transforming 4000 subtitles | 70 µs | 5.86 µs |
| frame rate conversion over 4000 subtitles | 68.4 µs | 9.99 µs |
| alignment over 4000 subtitles | 140 µs | 4.23 µs |
| sorting 4000 subtitles in reverse | 298 µs | 21 µs |
| removing every other subtitle | 183 µs | 19.9 µs |
| removal then undo | 243 µs | 39.7 µs |
| inserting 100 empty subtitles in the middle | 59.1 µs | 12 µs |
| editing a text, through a session | 172 ns | 6.41 ns |
| removing mentions over 4000 subtitles | 1.37 ms | 117 µs |
| italicising 4000 subtitles | 903 µs | 57.9 µs |
| replacing a frequent word over 4000 subtitles, plain text | 4.46 ms | 127 µs |
| replacing a frequent word over 4000 subtitles, regular expression | 4.18 ms | 268 µs |
| search with no result over 4000 subtitles | 2.87 ms | 182 µs |
| adjusting the durations of 4000 subtitles, four constraints | 1.29 ms | 98.6 µs |
| title case over 4000 subtitles | 17.8 ms | 718 µs |
| pasting 4000 texts | 977 µs | 42.1 µs |
| aligning a 4000-line translation, by position | 909 µs | 39.9 µs |
| aligning a 4000-line translation, by number | 782 µs | 36.2 µs |
| frame rate deduction over 4000 subtitles | 426 µs | 25.5 µs |

### 0.12.0 — 2026-09-26 — Release — load 1.29 — pace ×0.99

| Measure | Mean | Standard deviation |
| :----- | ------: | ---------: |
| the current line, among 4000 subtitles | 4.94 µs | 2.91 µs |
| composing a two-line line | 169 ns | 3.44 ns |
| opening a video | 9.24 ms | 665 µs |
| seeking a position | 563 µs | 143 µs |
| building the model over 4000 subtitles | 7.29 µs | 2.53 µs |
| a window of 40 rows, five columns | 20.3 µs | 7.31 µs |
| rafraîchir après un shifting 4000 subtitles | 7.48 µs | 2.29 µs |
| resetting the model after a row is removed | 7.58 µs | 3.41 µs |
| editing a text cell | 380 ns | 49.2 ns |
| editing a position cell | 8.27 µs | 1.37 µs |
| versionString | 37.9 ns | 7.85 ns |
| parse | 39.1 ns | 11.3 ns |
| format | 39.4 ns | 1.76 ns |
| position to frame | 7.81 ns | 2.51 ns |
| frame to position | 6.64 ns | 0.0412 ns |
| scaling by an exact rational | 6.86 ns | 0.0783 ns |
| reading 4000 subtitles | 2.35 ms | 296 µs |
| writing 4000 subtitles | 521 µs | 43.1 µs |
| shifting 4000 subtitles | 7.72 µs | 3.14 µs |
| shifting then undoing | 13.5 µs | 4.8 µs |
| transforming 4000 subtitles | 80.8 µs | 9.58 µs |
| frame rate conversion over 4000 subtitles | 70.1 µs | 12.1 µs |
| alignment over 4000 subtitles | 137 µs | 23.6 µs |
| sorting 4000 subtitles in reverse | 299 µs | 19.3 µs |
| removing every other subtitle | 171 µs | 28.7 µs |
| removal then undo | 244 µs | 44.2 µs |
| inserting 100 empty subtitles in the middle | 50.5 µs | 8.83 µs |
| editing a text, through a session | 299 ns | 986 ns |
| removing mentions over 4000 subtitles | 1.41 ms | 132 µs |
| italicising 4000 subtitles | 926 µs | 92.1 µs |
| replacing a frequent word over 4000 subtitles, plain text | 4.25 ms | 526 µs |
| replacing a frequent word over 4000 subtitles, regular expression | 4.39 ms | 474 µs |
| search with no result over 4000 subtitles | 2.89 ms | 191 µs |
| adjusting the durations of 4000 subtitles, four constraints | 1.32 ms | 79.5 µs |
| title case over 4000 subtitles | 19.2 ms | 1.04 ms |
| pasting 4000 texts | 1.06 ms | 71.3 µs |
| aligning a 4000-line translation, by position | 853 µs | 64.9 µs |
| aligning a 4000-line translation, by number | 871 µs | 58.1 µs |
| frame rate deduction over 4000 subtitles | 377 µs | 24 µs |

### 0.11.30 — 2026-09-25 — Release — load 1.44 — pace ×1.02

| Measure | Mean | Standard deviation |
| :----- | ------: | ---------: |
| the current line, among 4000 subtitles | 7.4 µs | 1.41 µs |
| composing a two-line line | 167 ns | 3.28 ns |
| opening a video | 9.33 ms | 714 µs |
| seeking a position | 575 µs | 136 µs |
| building the model over 4000 subtitles | 8.49 µs | 2.59 µs |
| a window of 40 rows, five columns | 17.4 µs | 3.39 µs |
| rafraîchir après un shifting 4000 subtitles | 8.33 µs | 1.11 µs |
| resetting the model after a row is removed | 8.4 µs | 1.97 µs |
| editing a text cell | 382 ns | 37 ns |
| editing a position cell | 10.7 µs | 415 ns |
| versionString | 43 ns | 9.85 ns |
| parse | 43.3 ns | 15.4 ns |
| format | 37.7 ns | 1.82 ns |
| position to frame | 6.64 ns | 0.0691 ns |
| frame to position | 6.64 ns | 0.073 ns |
| scaling by an exact rational | 7.34 ns | 3.1 ns |
| reading 4000 subtitles | 2.25 ms | 145 µs |
| writing 4000 subtitles | 518 µs | 41.7 µs |
| shifting 4000 subtitles | 6.82 µs | 2.44 µs |
| shifting then undoing | 13.5 µs | 3.81 µs |
| transforming 4000 subtitles | 71 µs | 4.36 µs |
| frame rate conversion over 4000 subtitles | 68.9 µs | 8.02 µs |
| alignment over 4000 subtitles | 129 µs | 17.1 µs |
| sorting 4000 subtitles in reverse | 274 µs | 27.1 µs |
| removing every other subtitle | 159 µs | 17 µs |
| removal then undo | 253 µs | 63.1 µs |
| inserting 100 empty subtitles in the middle | 52.9 µs | 14.2 µs |
| editing a text, through a session | 168 ns | 12.2 ns |
| removing mentions over 4000 subtitles | 1.45 ms | 53.9 µs |
| italicising 4000 subtitles | 913 µs | 48.4 µs |
| replacing a frequent word over 4000 subtitles, plain text | 4.19 ms | 276 µs |
| replacing a frequent word over 4000 subtitles, regular expression | 4.31 ms | 268 µs |
| search with no result over 4000 subtitles | 2.91 ms | 179 µs |
| adjusting the durations of 4000 subtitles, four constraints | 1.33 ms | 72.6 µs |
| title case over 4000 subtitles | 19.4 ms | 972 µs |
| pasting 4000 texts | 1.07 ms | 75.5 µs |
| aligning a 4000-line translation, by position | 856 µs | 56.4 µs |
| aligning a 4000-line translation, by number | 883 µs | 85.8 µs |
| frame rate deduction over 4000 subtitles | 387 µs | 42.2 µs |

### 0.11.25 — 2026-09-25 — Release — load 1.37 — pace ×1.19

| Measure | Mean | Standard deviation |
| :----- | ------: | ---------: |
| the current line, among 4000 subtitles | 14.1 µs | 2.34 µs |
| composing a two-line line | 817 ns | 67.1 ns |
| opening a video | 32.7 ms | 25.8 ms |
| seeking a position | 617 µs | 112 µs |
| building the model over 4000 subtitles | 13.2 µs | 1.44 µs |
| a window of 40 rows, five columns | 20.8 µs | 1.19 µs |
| rafraîchir après un shifting 4000 subtitles | 13.2 µs | 2.92 µs |
| resetting the model after a row is removed | 13.6 µs | 2.98 µs |
| editing a text cell | 502 ns | 199 ns |
| editing a position cell | 11 µs | 727 ns |
| versionString | 50.6 ns | 1 ns |
| parse | 42.3 ns | 2.23 ns |
| format | 40.7 ns | 2.59 ns |
| position to frame | 7.85 ns | 1.35 ns |
| frame to position | 8.28 ns | 2.07 ns |
| scaling by an exact rational | 8.3 ns | 0.221 ns |
| reading 4000 subtitles | 2.64 ms | 200 µs |
| writing 4000 subtitles | 949 µs | 412 µs |
| shifting 4000 subtitles | 8.78 µs | 3.21 µs |
| shifting then undoing | 15.5 µs | 2.65 µs |
| transforming 4000 subtitles | 85.8 µs | 3.07 µs |
| frame rate conversion over 4000 subtitles | 85.9 µs | 12.2 µs |
| alignment over 4000 subtitles | 153 µs | 11.8 µs |
| sorting 4000 subtitles in reverse | 329 µs | 46.1 µs |
| removing every other subtitle | 183 µs | 14.7 µs |
| removal then undo | 286 µs | 24.8 µs |
| inserting 100 empty subtitles in the middle | 70.8 µs | 21.1 µs |
| editing a text, through a session | 219 ns | 64.6 ns |
| removing mentions over 4000 subtitles | 1.57 ms | 53.8 µs |
| italicising 4000 subtitles | 1.09 ms | 99.1 µs |
| replacing a frequent word over 4000 subtitles, plain text | 4.69 ms | 235 µs |
| replacing a frequent word over 4000 subtitles, regular expression | 4.82 ms | 238 µs |
| search with no result over 4000 subtitles | 3.16 ms | 193 µs |
| adjusting the durations of 4000 subtitles, four constraints | 1.43 ms | 76 µs |
| title case over 4000 subtitles | 22 ms | 1.47 ms |
| pasting 4000 texts | 1.19 ms | 160 µs |
| aligning a 4000-line translation, by position | 1.1 ms | 293 µs |
| aligning a 4000-line translation, by number | 973 µs | 60 µs |
| frame rate deduction over 4000 subtitles | 436 µs | 22 µs |

### 0.11.21 — 2026-09-24 — Release — load 1.47 — pace ×0.96

| Measure | Mean | Standard deviation |
| :----- | ------: | ---------: |
| the current line, among 4000 subtitles | 6.01 µs | 1.24 µs |
| composing a two-line line | 187 ns | 3.06 ns |
| opening a video | 8.94 ms | 164 µs |
| seeking a position | 557 µs | 124 µs |
| building the model over 4000 subtitles | 7.25 µs | 1.7 µs |
| a window of 40 rows, five columns | 17.5 µs | 711 ns |
| rafraîchir après un shifting 4000 subtitles | 7.19 µs | 1.15 µs |
| resetting the model after a row is removed | 7.38 µs | 1.92 µs |
| editing a text cell | 398 ns | 48.8 ns |
| editing a position cell | 8.16 µs | 737 ns |
| versionString | 42 ns | 0.435 ns |
| parse | 37.6 ns | 3.28 ns |
| format | 35.6 ns | 0.688 ns |
| position to frame | 6.47 ns | 0.0513 ns |
| frame to position | 6.32 ns | 0.0685 ns |
| scaling by an exact rational | 6.72 ns | 0.0731 ns |
| reading 4000 subtitles | 2.06 ms | 58.3 µs |
| writing 4000 subtitles | 525 µs | 28.8 µs |
| shifting 4000 subtitles | 6.64 µs | 3.18 µs |
| shifting then undoing | 14 µs | 9.38 µs |
| transforming 4000 subtitles | 70.9 µs | 5.75 µs |
| frame rate conversion over 4000 subtitles | 69.3 µs | 20.4 µs |
| alignment over 4000 subtitles | 123 µs | 8.1 µs |
| sorting 4000 subtitles in reverse | 257 µs | 23.2 µs |
| removing every other subtitle | 164 µs | 22.9 µs |
| removal then undo | 241 µs | 35.9 µs |
| inserting 100 empty subtitles in the middle | 51.6 µs | 20.5 µs |
| editing a text, through a session | 180 ns | 11.7 ns |
| removing mentions over 4000 subtitles | 1.23 ms | 71.1 µs |
| italicising 4000 subtitles | 942 µs | 62.6 µs |
| replacing a frequent word over 4000 subtitles, plain text | 4.04 ms | 192 µs |
| replacing a frequent word over 4000 subtitles, regular expression | 4.15 ms | 107 µs |
| search with no result over 4000 subtitles | 2.69 ms | 92.6 µs |
| adjusting the durations of 4000 subtitles, four constraints | 1.23 ms | 49.8 µs |
| title case over 4000 subtitles | 18.2 ms | 684 µs |
| pasting 4000 texts | 1.01 ms | 32 µs |
| aligning a 4000-line translation, by position | 860 µs | 113 µs |
| aligning a 4000-line translation, by number | 855 µs | 61.2 µs |
| frame rate deduction over 4000 subtitles | 368 µs | 14.5 µs |

### 0.11.7 — 2026-09-21 — Release — load 1.41 — pace ×0.98

| Measure | Mean | Standard deviation |
| :----- | ------: | ---------: |
| the current line, among 4000 subtitles | 7.17 µs | 1.85 µs |
| composing a two-line line | 164 ns | 3.11 ns |
| opening a video | 9.11 ms | 591 µs |
| seeking a position | 569 µs | 158 µs |
| building the model over 4000 subtitles | 7.38 µs | 2.89 µs |
| a window of 40 rows, five columns | 17.5 µs | 838 ns |
| rafraîchir après un shifting 4000 subtitles | 8.38 µs | 2.33 µs |
| resetting the model after a row is removed | 8.38 µs | 2.47 µs |
| editing a text cell | 419 ns | 194 ns |
| editing a position cell | 9.33 µs | 667 ns |
| versionString | 41.3 ns | 12.5 ns |
| parse | 36.8 ns | 3.18 ns |
| format | 35.3 ns | 0.58 ns |
| position to frame | 6.48 ns | 0.0669 ns |
| frame to position | 6.32 ns | 0.0763 ns |
| scaling by an exact rational | 7.7 ns | 0.0994 ns |
| reading 4000 subtitles | 2.23 ms | 182 µs |
| writing 4000 subtitles | 560 µs | 60 µs |
| shifting 4000 subtitles | 8.18 µs | 10.8 µs |
| shifting then undoing | 14.9 µs | 3.4 µs |
| transforming 4000 subtitles | 72 µs | 9.68 µs |
| frame rate conversion over 4000 subtitles | 66.2 µs | 8.29 µs |
| alignment over 4000 subtitles | 126 µs | 10.6 µs |
| sorting 4000 subtitles in reverse | 270 µs | 29.2 µs |
| removing every other subtitle | 162 µs | 24.8 µs |
| removal then undo | 235 µs | 32.9 µs |
| inserting 100 empty subtitles in the middle | 51.3 µs | 16.6 µs |
| editing a text, through a session | 200 ns | 23.2 ns |
| removing mentions over 4000 subtitles | 1.27 ms | 90.3 µs |
| italicising 4000 subtitles | 983 µs | 63.5 µs |
| replacing a frequent word over 4000 subtitles, plain text | 4 ms | 260 µs |
| replacing a frequent word over 4000 subtitles, regular expression | 4.18 ms | 288 µs |
| search with no result over 4000 subtitles | 2.67 ms | 165 µs |
| adjusting the durations of 4000 subtitles, four constraints | 1.23 ms | 72 µs |
| title case over 4000 subtitles | 19.9 ms | 1.78 ms |
| pasting 4000 texts | 1.13 ms | 69.5 µs |
| aligning a 4000-line translation, by position | 851 µs | 69.7 µs |
| aligning a 4000-line translation, by number | 824 µs | 64.9 µs |
| frame rate deduction over 4000 subtitles | 370 µs | 19 µs |
