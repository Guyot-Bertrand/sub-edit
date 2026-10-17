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
| construction du modèle sur 4000 sous-titres | 59 ns | ~8 µs |
| rafraîchir après un décalage de 4000 sous-titres | 26 ns | ~9 µs |
| édition d'une cellule de position | 579 ns | ~10 µs |

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
| réinitialisation du modèle après une ligne retirée | ~13 µs |
| rafraîchir après un décalage de 4000 sous-titres | ~12 µs |
| construction du modèle sur 4000 sous-titres | ~10 µs |

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
| écriture de 4000 sous-titres | 852 µs | **~610 µs** |

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
| position vers image | 6.32 ns | 0.12.23 — 2026-09-30 | 8.03 ns | 0.10.1 — 2026-09-11 |
| image vers position | 6.32 ns | 0.13.25 — 2026-10-05 | 12.7 ns | 0.10.14 — 2026-09-15 |
| mise à l'échelle par un rationnel exact | 6.71 ns | 0.9.19 — 2026-09-08 | 8.3 ns | 0.10.14 — 2026-09-15 |
| lecture de 4000 sous-titres | 2.01 ms | 0.13.33 — 2026-10-06 | 3.28 ms | 0.5.3 — 2026-08-22 |
| écriture de 4000 sous-titres | 488 µs | 0.3.10 — 2026-08-15 | 825 µs | 0.9.3 — 2026-09-05 |
| décalage de 4000 sous-titres | 6.56 µs | 0.13.25 — 2026-10-05 | 11 µs | 0.9.18 — 2026-09-08 |
| décalage puis annulation | 12.7 µs | 0.13.25 — 2026-10-05 | 20.7 µs | 0.2.6 — 2026-08-13 |
| transformation de 4000 sous-titres | 67.5 µs | 0.13.13 — 2026-10-02 | 94.5 µs | 0.9.18 — 2026-09-08 |
| conversion de fréquence sur 4000 sous-titres | 66.2 µs | 0.11.7 — 2026-09-21 | 100 µs | 0.2.12 — 2026-08-14 |
| tri de 4000 sous-titres à l'envers | 196 µs | 0.3.2 — 2026-08-15 | 352 µs | 0.9.18 — 2026-09-08 |
| suppression d'un sous-titre sur deux | 138 µs | 0.4.14 — 2026-08-21 | 12.5 ms | 0.3.3 — 2026-08-15 |
| insertion de 100 sous-titres vides au milieu | 48.2 µs | 0.7.5 — 2026-08-29 | 97.6 µs | 0.8.13 — 2026-09-02 |
| modification d'un texte, à travers une session | 114 ns | 0.2.14 — 2026-08-14 | 228 ns | 0.8.6 — 2026-08-31 |
| suppression des mentions sur 4000 sous-titres | 853 µs | 0.4.21 — 2026-08-22 | 5.68 ms | 0.4.4 — 2026-08-17 |
| suppression puis annulation | 213 µs | 0.4.14 — 2026-08-21 | 314 µs | 0.10.15 — 2026-09-16 |
| construction du modèle sur 4000 sous-titres | 59.3 ns | 0.4.14 — 2026-08-21 | 16.4 µs | 0.9.13 — 2026-09-07 |
| une fenêtre de 40 lignes, cinq colonnes | 14.7 µs | 0.4.15 — 2026-08-21 | 28.9 µs | 0.6.8 — 2026-08-26 |
| rafraîchir après un décalage de 4000 sous-titres | 26.4 ns | 0.4.15 — 2026-08-21 | 13.2 µs | 0.11.25 — 2026-09-25 |
| édition d'une cellule de texte | 371 ns | 0.5.9 — 2026-08-23 | 496 ns | 0.9.22 — 2026-09-10 |
| édition d'une cellule de position | 8.09 µs | 0.12.7 — 2026-09-27 | 14.8 µs | 0.8.8 — 2026-09-01 |
| réinitialisation du modèle après une ligne retirée | 7.23 µs | 0.9.20 — 2026-09-08 | 13.6 µs | 0.11.25 — 2026-09-25 |
| la réplique en cours, sur 4000 sous-titres | 4.71 µs | 0.8.13 — 2026-09-02 | 14.1 µs | 0.11.25 — 2026-09-25 |
| composer une réplique de deux lignes | 108 ns | 0.15.20 — 2026-10-12 | 284 ns | 0.13.33 — 2026-10-06 |
| ouvrir une vidéo | 8.82 ms | 0.13.33 — 2026-10-06 | 223 ms | 0.15.0 — 2026-10-09 |
| chercher une position | 538 µs | 0.6.13 — 2026-08-27 | 5.64 ms | 0.14.19 — 2026-10-08 |
| déduction de fréquence sur 4000 sous-titres | 366 µs | 0.7.15 — 2026-08-30 | 459 µs | 0.15.7 — 2026-10-09 |
| alignement sur 4000 sous-titres | 123 µs | 0.13.25 — 2026-10-05 | 160 µs | 0.15.18 — 2026-10-12 |
| mise en italique de 4000 sous-titres | 885 µs | 0.12.8 — 2026-09-27 | 1.39 ms | 0.10.10 — 2026-09-13 |
| remplacement d'un mot fréquent sur 4000 sous-titres, texte simple | 3.83 ms | 0.15.15 — 2026-10-12 | 4.78 ms | 0.12.19 — 2026-09-30 |
| remplacement d'un mot fréquent sur 4000 sous-titres, expression régulière | 4.01 ms | 0.15.15 — 2026-10-12 | 5.13 ms | 0.14.5 — 2026-10-06 |
| recherche sans résultat sur 4000 sous-titres | 2.59 ms | 0.15.10 — 2026-10-10 | 8.01 ms | 0.13.10 — 2026-10-01 |
| ajustement des durées de 4000 sous-titres, quatre contraintes | 1.2 ms | 0.10.17 — 2026-09-19 | 1.47 ms | 0.15.18 — 2026-10-12 |
| casse de titre sur 4000 sous-titres | 17.6 ms | 0.15.15 — 2026-10-12 | 22 ms | 0.11.25 — 2026-09-25 |
| collage de 4000 textes | 964 µs | 0.10.17 — 2026-09-19 | 1.26 ms | 0.11.1 — 2026-09-19 |
| alignement d'une traduction de 4000 lignes, par position | 803 µs | 0.13.33 — 2026-10-06 | 1.1 ms | 0.11.25 — 2026-09-25 |
| alignement d'une traduction de 4000 lignes, par numéro | 782 µs | 0.12.7 — 2026-09-27 | 995 µs | 0.12.19 — 2026-09-30 |
| erreurs courantes du français sur 4000 sous-titres | 153 ms | 0.15.37 — 2026-10-17 | 187 ms | 0.12.19 — 2026-09-30 |
| erreurs courantes de l'anglais sur 4000 sous-titres | 169 ms | 0.15.13 — 2026-10-12 | 207 ms | 0.12.19 — 2026-09-30 |
| découpage de 4000 sous-titres, sans cache de longueurs | 107 ms | 0.15.37 — 2026-10-17 | 132 ms | 0.12.19 — 2026-09-30 |
| découpage de 4000 sous-titres, avec cache de longueurs | 111 ms | 0.15.37 — 2026-10-17 | 138 ms | 0.12.19 — 2026-09-30 |
| découpage de 4000 sous-titres, en ems, avec cache de longueurs | 175 ms | 0.15.15 — 2026-10-12 | 213 ms | 0.15.35 — 2026-10-16 |
| walk of 200 files in a tree of depth 3 | 556 µs | 0.15.19 — 2026-10-12 | 659 µs | 0.14.3 — 2026-10-06 |
| shift of 200 files of 20 subtitles | 31.5 ms | 0.15.24 — 2026-10-13 | 38.5 ms | 0.15.18 — 2026-10-12 |
| shift of one file of 4000 subtitles | 2.86 ms | 0.15.19 — 2026-10-12 | 3.6 ms | 0.13.10 — 2026-10-01 |
| rendre une image de 640×360 | 208 µs | 0.14.5 — 2026-10-06 | 378 µs | 0.15.0 — 2026-10-09 |
| rendre une image de 1280×720 | 724 µs | 0.14.7 — 2026-10-07 | 1.35 ms | 0.14.19 — 2026-10-08 |
| rendre une image de 1920×1080 | 1.41 ms | 0.14.7 — 2026-10-07 | 2.66 ms | 0.15.0 — 2026-10-09 |
| chercher l'image 0 (vidéo 720p) | 6.17 ms | 0.15.10 — 2026-10-10 | 7.81 ms | 0.14.19 — 2026-10-08 |
| chercher l'image 100 (vidéo 720p) | 38.6 ms | 0.14.7 — 2026-10-07 | 53.2 ms | 0.14.19 — 2026-10-08 |
| chercher l'image 249 (vidéo 720p) | 82.5 ms | 0.14.5 — 2026-10-06 | 125 ms | 0.14.19 — 2026-10-08 |
| pas avant vers l'image 100 (vidéo 720p) | 40 ms | 0.14.7 — 2026-10-07 | 52.4 ms | 0.14.19 — 2026-10-08 |
| pas avant vers l'image 249 (vidéo 720p) | 83.3 ms | 0.14.5 — 2026-10-06 | 120 ms | 0.15.35 — 2026-10-16 |
| pas arrière vers l'image 99 (vidéo 720p) | 39 ms | 0.14.5 — 2026-10-06 | 49.5 ms | 0.14.19 — 2026-10-08 |
| pas arrière vers l'image 248 (vidéo 720p) | 82.4 ms | 0.14.5 — 2026-10-06 | 115 ms | 0.15.18 — 2026-10-12 |
| traduire une réplique balisée pour l'image | 815 ns | 0.15.5 — 2026-10-09 | 1.17 µs | 0.14.19 — 2026-10-08 |
| centrer une ligne sur 4000 sous-titres | 1.06 µs | 0.15.5 — 2026-10-09 | 1.3 µs | 0.14.19 — 2026-10-08 |
| recherche de conversion sur 4000 sous-titres | 361 µs | 0.15.5 — 2026-10-09 | 448 µs | 0.15.7 — 2026-10-09 |

<!-- versionString min=29.1524 max=55.1 -->
<!-- parse min=29.9 max=67.348 -->
<!-- format min=29.8143 max=68.0218 -->
<!-- position vers image min=6.31843 max=8.0301 -->
<!-- image vers position min=6.31692 max=12.7175 -->
<!-- mise à l'échelle par un rationnel exact min=6.70967 max=8.3022 -->
<!-- lecture de 4000 sous-titres min=2014490.0 max=3280000.0 -->
<!-- écriture de 4000 sous-titres min=488279.0 max=825490.0 -->
<!-- décalage de 4000 sous-titres min=6556.44 max=11029.2 -->
<!-- décalage puis annulation min=12695.6 max=20700.0 -->
<!-- transformation de 4000 sous-titres min=67454.9 max=94496.9 -->
<!-- conversion de fréquence sur 4000 sous-titres min=66238.2 max=100000.0 -->
<!-- tri de 4000 sous-titres à l'envers min=196000.0 max=351534.0 -->
<!-- suppression d'un sous-titre sur deux min=137974.0 max=12500000.0 -->
<!-- insertion de 100 sous-titres vides au milieu min=48185.9 max=97640.4 -->
<!-- modification d'un texte, à travers une session min=114.0 max=227.528 -->
<!-- suppression des mentions sur 4000 sous-titres min=853000.0 max=5676890.0 -->
<!-- suppression puis annulation min=213280.0 max=314434.0 -->
<!-- construction du modèle sur 4000 sous-titres min=59.2521 max=16365.4 -->
<!-- une fenêtre de 40 lignes, cinq colonnes min=14699.7 max=28872.4 -->
<!-- rafraîchir après un décalage de 4000 sous-titres min=26.4 max=13151.9 -->
<!-- édition d'une cellule de texte min=370.791 max=496.387 -->
<!-- édition d'une cellule de position min=8090.8 max=14758.2 -->
<!-- réinitialisation du modèle après une ligne retirée min=7229.3 max=13625.8 -->
<!-- la réplique en cours, sur 4000 sous-titres min=4714.13 max=14131.7 -->
<!-- composer une réplique de deux lignes min=108.361 max=284.466 -->
<!-- ouvrir une vidéo min=8822490.0 max=222615000.0 -->
<!-- chercher une position min=537819.0 max=5643350.0 -->
<!-- déduction de fréquence sur 4000 sous-titres min=366479.0 max=458920.0 -->
<!-- alignement sur 4000 sous-titres min=122556.0 max=160422.0 -->
<!-- mise en italique de 4000 sous-titres min=885492.0 max=1386260.0 -->
<!-- remplacement d'un mot fréquent sur 4000 sous-titres, texte simple min=3828060.0 max=4776230.0 -->
<!-- remplacement d'un mot fréquent sur 4000 sous-titres, expression régulière min=4011170.0 max=5128860.0 -->
<!-- recherche sans résultat sur 4000 sous-titres min=2588120.0 max=8011860.0 -->
<!-- ajustement des durées de 4000 sous-titres, quatre contraintes min=1200330.0 max=1466530.0 -->
<!-- casse de titre sur 4000 sous-titres min=17551600.0 max=21973100.0 -->
<!-- collage de 4000 textes min=964212.0 max=1259920.0 -->
<!-- alignement d'une traduction de 4000 lignes, par position min=802764.0 max=1096980.0 -->
<!-- alignement d'une traduction de 4000 lignes, par numéro min=781546.0 max=995207.0 -->
<!-- erreurs courantes du français sur 4000 sous-titres min=152624000.0 max=186776000.0 -->
<!-- erreurs courantes de l'anglais sur 4000 sous-titres min=168581000.0 max=206664000.0 -->
<!-- découpage de 4000 sous-titres, sans cache de longueurs min=107227000.0 max=131622000.0 -->
<!-- découpage de 4000 sous-titres, avec cache de longueurs min=111172000.0 max=138172000.0 -->
<!-- découpage de 4000 sous-titres, en ems, avec cache de longueurs min=174535000.0 max=212846000.0 -->
<!-- walk of 200 files in a tree of depth 3 min=555900.0 max=658528.0 -->
<!-- shift of 200 files of 20 subtitles min=31494800.0 max=38497100.0 -->
<!-- shift of one file of 4000 subtitles min=2862710.0 max=3596300.0 -->
<!-- rendre une image de 640×360 min=208485.0 max=378356.0 -->
<!-- rendre une image de 1280×720 min=723665.0 max=1349710.0 -->
<!-- rendre une image de 1920×1080 min=1406670.0 max=2658770.0 -->
<!-- chercher l'image 0 (vidéo 720p) min=6173300.0 max=7811380.0 -->
<!-- chercher l'image 100 (vidéo 720p) min=38618500.0 max=53241100.0 -->
<!-- chercher l'image 249 (vidéo 720p) min=82494800.0 max=124818000.0 -->
<!-- pas avant vers l'image 100 (vidéo 720p) min=39987500.0 max=52446900.0 -->
<!-- pas avant vers l'image 249 (vidéo 720p) min=83322300.0 max=119739000.0 -->
<!-- pas arrière vers l'image 99 (vidéo 720p) min=38964900.0 max=49479600.0 -->
<!-- pas arrière vers l'image 248 (vidéo 720p) min=82354100.0 max=114941000.0 -->
<!-- traduire une réplique balisée pour l'image min=815.476 max=1172.95 -->
<!-- centrer une ligne sur 4000 sous-titres min=1064.1 max=1300.8 -->
<!-- recherche de conversion sur 4000 sous-titres min=361110.0 max=447822.0 -->

## Records

One section per version. Records older than a month are pruned; their
extremes survive in the table above.

<!-- records -->

### 0.15.38 — 2026-10-17 — Release — load 1.44 — pace ×1.04

| Measure | Mean | Standard deviation |
| :----- | ------: | ---------: |
| la réplique en cours, sur 4000 sous-titres | 7.38 µs | 1.63 µs |
| traduire une réplique balisée pour l'image | 826 ns | 59.6 ns |
| composer une réplique de deux lignes | 137 ns | 3.45 ns |
| ouvrir une vidéo | 220 ms | 2.22 ms |
| chercher une position | 5.39 ms | 235 µs |
| rendre une image de 640×360 | 307 µs | 20.9 µs |
| rendre une image de 1280×720 | 828 µs | 38.3 µs |
| rendre une image de 1920×1080 | 2.33 ms | 255 µs |
| chercher l'image 0 (vidéo 720p) | 6.54 ms | 623 µs |
| chercher l'image 100 (vidéo 720p) | 51.2 ms | 4.41 ms |
| chercher l'image 249 (vidéo 720p) | 112 ms | 8.38 ms |
| pas avant vers l'image 100 (vidéo 720p) | 48.8 ms | 4.1 ms |
| pas avant vers l'image 249 (vidéo 720p) | 110 ms | 8.18 ms |
| pas arrière vers l'image 99 (vidéo 720p) | 46.7 ms | 4.32 ms |
| pas arrière vers l'image 248 (vidéo 720p) | 112 ms | 14.8 ms |
| construction du modèle sur 4000 sous-titres | 8.46 µs | 5.35 µs |
| une fenêtre de 40 lignes, cinq colonnes | 18 µs | 1.21 µs |
| rafraîchir après un décalage de 4000 sous-titres | 8.92 µs | 3.13 µs |
| réinitialisation du modèle après une ligne retirée | 8.22 µs | 3.02 µs |
| édition d'une cellule de texte | 392 ns | 23.3 ns |
| édition d'une cellule de position | 9.67 µs | 3.56 µs |
| centrer une ligne sur 4000 sous-titres | 1.26 µs | 264 ns |
| découpage de 4000 sous-titres, en ems, avec cache de longueurs | 193 ms | 14.7 ms |
| versionString | 51.5 ns | 0.669 ns |
| parse | 36.9 ns | 4.73 ns |
| format | 38.7 ns | 9.47 ns |
| position vers image | 9.26 ns | 2.8 ns |
| image vers position | 6.64 ns | 1.06 ns |
| mise à l'échelle par un rationnel exact | 7.7 ns | 0.113 ns |
| découpage de 4000 sous-titres, sans cache de longueurs | 113 ms | 8.32 ms |
| découpage de 4000 sous-titres, avec cache de longueurs | 124 ms | 7.87 ms |
| erreurs courantes du français sur 4000 sous-titres | 162 ms | 13.3 ms |
| erreurs courantes de l'anglais sur 4000 sous-titres | 171 ms | 10.1 ms |
| lecture de 4000 sous-titres | 2.17 ms | 285 µs |
| écriture de 4000 sous-titres | 518 µs | 24.9 µs |
| décalage de 4000 sous-titres | 6.96 µs | 3.24 µs |
| décalage puis annulation | 13.5 µs | 3.44 µs |
| transformation de 4000 sous-titres | 71.9 µs | 9.66 µs |
| conversion de fréquence sur 4000 sous-titres | 76.9 µs | 11.3 µs |
| alignement sur 4000 sous-titres | 156 µs | 23.4 µs |
| tri de 4000 sous-titres à l'envers | 270 µs | 16.1 µs |
| suppression d'un sous-titre sur deux | 167 µs | 31.6 µs |
| suppression puis annulation | 295 µs | 64.8 µs |
| insertion de 100 sous-titres vides au milieu | 61.3 µs | 17.7 µs |
| modification d'un texte, à travers une session | 204 ns | 44.1 ns |
| suppression des mentions sur 4000 sous-titres | 1.22 ms | 26.2 µs |
| mise en italique de 4000 sous-titres | 1.13 ms | 136 µs |
| remplacement d'un mot fréquent sur 4000 sous-titres, texte simple | 4.47 ms | 373 µs |
| remplacement d'un mot fréquent sur 4000 sous-titres, expression régulière | 4.45 ms | 385 µs |
| recherche sans résultat sur 4000 sous-titres | 2.62 ms | 100 µs |
| ajustement des durées de 4000 sous-titres, quatre contraintes | 1.34 ms | 96.2 µs |
| casse de titre sur 4000 sous-titres | 19.4 ms | 1.84 ms |
| collage de 4000 textes | 982 µs | 37.9 µs |
| alignement d'une traduction de 4000 lignes, par position | 1.02 ms | 108 µs |
| alignement d'une traduction de 4000 lignes, par numéro | 952 µs | 120 µs |
| déduction de fréquence sur 4000 sous-titres | 368 µs | 14.3 µs |
| recherche de conversion sur 4000 sous-titres | 366 µs | 25.1 µs |
| walk of 200 files in a tree of depth 3 | 628 µs | 95.2 µs |
| shift of 200 files of 20 subtitles | 34.4 ms | 3.16 ms |
| shift of one file of 4000 subtitles | 3.58 ms | 522 µs |

### 0.15.37 — 2026-10-17 — Release — load 1.40 — pace ×0.87

| Measure | Mean | Standard deviation |
| :----- | ------: | ---------: |
| la réplique en cours, sur 4000 sous-titres | 5.62 µs | 408 ns |
| traduire une réplique balisée pour l'image | 831 ns | 32.3 ns |
| composer une réplique de deux lignes | 139 ns | 1.6 ns |
| ouvrir une vidéo | 219 ms | 1.58 ms |
| chercher une position | 5.34 ms | 75.7 µs |
| rendre une image de 640×360 | 316 µs | 24 µs |
| rendre une image de 1280×720 | 913 µs | 42.9 µs |
| rendre une image de 1920×1080 | 2.03 ms | 128 µs |
| chercher l'image 0 (vidéo 720p) | 6.2 ms | 246 µs |
| chercher l'image 100 (vidéo 720p) | 43.5 ms | 2.1 ms |
| chercher l'image 249 (vidéo 720p) | 100 ms | 6.04 ms |
| pas avant vers l'image 100 (vidéo 720p) | 43.5 ms | 2.49 ms |
| pas avant vers l'image 249 (vidéo 720p) | 102 ms | 6.59 ms |
| pas arrière vers l'image 99 (vidéo 720p) | 43.4 ms | 2.12 ms |
| pas arrière vers l'image 248 (vidéo 720p) | 101 ms | 5.85 ms |
| construction du modèle sur 4000 sous-titres | 10.5 µs | 531 ns |
| une fenêtre de 40 lignes, cinq colonnes | 17.9 µs | 244 ns |
| rafraîchir après un décalage de 4000 sous-titres | 10.4 µs | 133 ns |
| réinitialisation du modèle après une ligne retirée | 10.4 µs | 339 ns |
| édition d'une cellule de texte | 400 ns | 29.2 ns |
| édition d'une cellule de position | 11.5 µs | 178 ns |
| centrer une ligne sur 4000 sous-titres | 1.08 µs | 149 ns |
| découpage de 4000 sous-titres, en ems, avec cache de longueurs | 175 ms | 6.8 ms |
| versionString | 45.2 ns | 0.509 ns |
| parse | 35.9 ns | 2.84 ns |
| format | 36.7 ns | 0.451 ns |
| position vers image | 6.32 ns | 0.0674 ns |
| image vers position | 6.47 ns | 0.0679 ns |
| mise à l'échelle par un rationnel exact | 6.71 ns | 0.0524 ns |
| découpage de 4000 sous-titres, sans cache de longueurs | 107 ms | 1.3 ms |
| découpage de 4000 sous-titres, avec cache de longueurs | 111 ms | 5.08 ms |
| erreurs courantes du français sur 4000 sous-titres | 153 ms | 3.32 ms |
| erreurs courantes de l'anglais sur 4000 sous-titres | 171 ms | 6.32 ms |
| lecture de 4000 sous-titres | 2.05 ms | 54.6 µs |
| écriture de 4000 sous-titres | 518 µs | 12 µs |
| décalage de 4000 sous-titres | 8.11 µs | 3.38 µs |
| décalage puis annulation | 15.4 µs | 3.32 µs |
| transformation de 4000 sous-titres | 76 µs | 11.7 µs |
| conversion de fréquence sur 4000 sous-titres | 77.6 µs | 6.49 µs |
| alignement sur 4000 sous-titres | 135 µs | 5.53 µs |
| tri de 4000 sous-titres à l'envers | 282 µs | 27.8 µs |
| suppression d'un sous-titre sur deux | 161 µs | 15.7 µs |
| suppression puis annulation | 246 µs | 26.4 µs |
| insertion de 100 sous-titres vides au milieu | 55.7 µs | 14.8 µs |
| modification d'un texte, à travers une session | 184 ns | 22.9 ns |
| suppression des mentions sur 4000 sous-titres | 1.26 ms | 69.5 µs |
| mise en italique de 4000 sous-titres | 910 µs | 25.2 µs |
| remplacement d'un mot fréquent sur 4000 sous-titres, texte simple | 3.86 ms | 89.5 µs |
| remplacement d'un mot fréquent sur 4000 sous-titres, expression régulière | 4.03 ms | 71.6 µs |
| recherche sans résultat sur 4000 sous-titres | 2.93 ms | 388 µs |
| ajustement des durées de 4000 sous-titres, quatre contraintes | 1.24 ms | 35.6 µs |
| casse de titre sur 4000 sous-titres | 17.8 ms | 252 µs |
| collage de 4000 textes | 1.04 ms | 31.3 µs |
| alignement d'une traduction de 4000 lignes, par position | 829 µs | 34.7 µs |
| alignement d'une traduction de 4000 lignes, par numéro | 807 µs | 34.5 µs |
| déduction de fréquence sur 4000 sous-titres | 367 µs | 1.09 µs |
| recherche de conversion sur 4000 sous-titres | 362 µs | 19.4 µs |
| walk of 200 files in a tree of depth 3 | 563 µs | 17.7 µs |
| shift of 200 files of 20 subtitles | 31.8 ms | 1.25 ms |
| shift of one file of 4000 subtitles | 2.9 ms | 53.8 µs |

### 0.15.35 — 2026-10-16 — Release — load 1.47 — pace ×1.48

| Measure | Mean | Standard deviation |
| :----- | ------: | ---------: |
| la réplique en cours, sur 4000 sous-titres | 7.44 µs | 5.11 µs |
| traduire une réplique balisée pour l'image | 1.02 µs | 49.7 ns |
| composer une réplique de deux lignes | 161 ns | 42.8 ns |
| ouvrir une vidéo | 226 ms | 25.3 ms |
| chercher une position | 5.33 ms | 552 µs |
| rendre une image de 640×360 | 395 µs | 106 µs |
| rendre une image de 1280×720 | 1.02 ms | 153 µs |
| rendre une image de 1920×1080 | 2.41 ms | 204 µs |
| chercher l'image 0 (vidéo 720p) | 7.07 ms | 1.78 ms |
| chercher l'image 100 (vidéo 720p) | 52 ms | 3.57 ms |
| chercher l'image 249 (vidéo 720p) | 123 ms | 14.7 ms |
| pas avant vers l'image 100 (vidéo 720p) | 53.5 ms | 6.45 ms |
| pas avant vers l'image 249 (vidéo 720p) | 120 ms | 7.61 ms |
| pas arrière vers l'image 99 (vidéo 720p) | 74.3 ms | 65 ms |
| pas arrière vers l'image 248 (vidéo 720p) | 142 ms | 84.5 ms |
| construction du modèle sur 4000 sous-titres | 13.2 µs | 3.39 µs |
| une fenêtre de 40 lignes, cinq colonnes | 24 µs | 8.92 µs |
| rafraîchir après un décalage de 4000 sous-titres | 12.4 µs | 3.76 µs |
| réinitialisation du modèle après une ligne retirée | 13 µs | 2.9 µs |
| édition d'une cellule de texte | 474 ns | 69.5 ns |
| édition d'une cellule de position | 14.6 µs | 2.32 µs |
| centrer une ligne sur 4000 sous-titres | 1.32 µs | 472 ns |
| découpage de 4000 sous-titres, en ems, avec cache de longueurs | 213 ms | 5.16 ms |
| versionString | 52.4 ns | 4.03 ns |
| parse | 43.3 ns | 2.51 ns |
| format | 45.1 ns | 12.8 ns |
| position vers image | 9.75 ns | 4.81 ns |
| image vers position | 7.64 ns | 0.21 ns |
| mise à l'échelle par un rationnel exact | 8.16 ns | 0.571 ns |
| découpage de 4000 sous-titres, sans cache de longueurs | 131 ms | 10.6 ms |
| découpage de 4000 sous-titres, avec cache de longueurs | 132 ms | 7.36 ms |
| erreurs courantes du français sur 4000 sous-titres | 185 ms | 13.2 ms |
| erreurs courantes de l'anglais sur 4000 sous-titres | 207 ms | 13.9 ms |
| lecture de 4000 sous-titres | 2.61 ms | 450 µs |
| écriture de 4000 sous-titres | 847 µs | 322 µs |
| décalage de 4000 sous-titres | 28.6 µs | 20.8 µs |
| décalage puis annulation | 47.2 µs | 49.9 µs |
| transformation de 4000 sous-titres | 294 µs | 474 µs |
| conversion de fréquence sur 4000 sous-titres | 184 µs | 122 µs |
| alignement sur 4000 sous-titres | 361 µs | 125 µs |
| tri de 4000 sous-titres à l'envers | 1.18 ms | 756 µs |
| suppression d'un sous-titre sur deux | 502 µs | 186 µs |
| suppression puis annulation | 1.17 ms | 944 µs |
| insertion de 100 sous-titres vides au milieu | 314 µs | 416 µs |
| modification d'un texte, à travers une session | 647 ns | 256 ns |
| suppression des mentions sur 4000 sous-titres | 6.51 ms | 3.73 ms |
| mise en italique de 4000 sous-titres | 4.15 ms | 1.62 ms |
| remplacement d'un mot fréquent sur 4000 sous-titres, texte simple | 11.5 ms | 1.29 ms |
| remplacement d'un mot fréquent sur 4000 sous-titres, expression régulière | 12.3 ms | 1.96 ms |
| recherche sans résultat sur 4000 sous-titres | 9.16 ms | 3.91 ms |
| ajustement des durées de 4000 sous-titres, quatre contraintes | 2.23 ms | 849 µs |
| casse de titre sur 4000 sous-titres | 41.9 ms | 18.6 ms |
| collage de 4000 textes | 1.51 ms | 421 µs |
| alignement d'une traduction de 4000 lignes, par position | 1.22 ms | 201 µs |
| alignement d'une traduction de 4000 lignes, par numéro | 1.84 ms | 474 µs |
| déduction de fréquence sur 4000 sous-titres | 743 µs | 167 µs |
| recherche de conversion sur 4000 sous-titres | 564 µs | 57.4 µs |
| walk of 200 files in a tree of depth 3 | 1.05 ms | 330 µs |
| shift of 200 files of 20 subtitles | 53.8 ms | 11.7 ms |
| shift of one file of 4000 subtitles | 3.77 ms | 757 µs |

### 0.15.24 — 2026-10-13 — Release — load 1.43 — pace ×0.99

| Measure | Mean | Standard deviation |
| :----- | ------: | ---------: |
| la réplique en cours, sur 4000 sous-titres | 5.68 µs | 1.56 µs |
| traduire une réplique balisée pour l'image | 830 ns | 27.3 ns |
| composer une réplique de deux lignes | 110 ns | 2.36 ns |
| ouvrir une vidéo | 219 ms | 1.6 ms |
| chercher une position | 5.41 ms | 95.5 µs |
| rendre une image de 640×360 | 311 µs | 23.2 µs |
| rendre une image de 1280×720 | 855 µs | 53.6 µs |
| rendre une image de 1920×1080 | 2.08 ms | 108 µs |
| chercher l'image 0 (vidéo 720p) | 6.34 ms | 370 µs |
| chercher l'image 100 (vidéo 720p) | 45.6 ms | 3.14 ms |
| chercher l'image 249 (vidéo 720p) | 104 ms | 6.41 ms |
| pas avant vers l'image 100 (vidéo 720p) | 45.3 ms | 2.81 ms |
| pas avant vers l'image 249 (vidéo 720p) | 105 ms | 6.38 ms |
| pas arrière vers l'image 99 (vidéo 720p) | 44.1 ms | 2.51 ms |
| pas arrière vers l'image 248 (vidéo 720p) | 105 ms | 6.06 ms |
| construction du modèle sur 4000 sous-titres | 8.15 µs | 1.3 µs |
| une fenêtre de 40 lignes, cinq colonnes | 17.9 µs | 880 ns |
| rafraîchir après un décalage de 4000 sous-titres | 8.03 µs | 406 ns |
| réinitialisation du modèle après une ligne retirée | 8.3 µs | 3.19 µs |
| édition d'une cellule de texte | 386 ns | 13.9 ns |
| édition d'une cellule de position | 9.05 µs | 450 ns |
| centrer une ligne sur 4000 sous-titres | 1.13 µs | 315 ns |
| découpage de 4000 sous-titres, en ems, avec cache de longueurs | 178 ms | 5.42 ms |
| versionString | 45.3 ns | 1.08 ns |
| parse | 43.2 ns | 1.15 ns |
| format | 37 ns | 6.55 ns |
| position vers image | 6.77 ns | 1.57 ns |
| image vers position | 6.32 ns | 0.0643 ns |
| mise à l'échelle par un rationnel exact | 6.72 ns | 0.0719 ns |
| découpage de 4000 sous-titres, sans cache de longueurs | 110 ms | 4.13 ms |
| découpage de 4000 sous-titres, avec cache de longueurs | 113 ms | 4.82 ms |
| erreurs courantes du français sur 4000 sous-titres | 155 ms | 4.12 ms |
| erreurs courantes de l'anglais sur 4000 sous-titres | 171 ms | 4.32 ms |
| lecture de 4000 sous-titres | 2.04 ms | 49.3 µs |
| écriture de 4000 sous-titres | 562 µs | 64.6 µs |
| décalage de 4000 sous-titres | 6.84 µs | 3.55 µs |
| décalage puis annulation | 13.2 µs | 3.54 µs |
| transformation de 4000 sous-titres | 75.5 µs | 10.8 µs |
| conversion de fréquence sur 4000 sous-titres | 75 µs | 7.65 µs |
| alignement sur 4000 sous-titres | 133 µs | 9.7 µs |
| tri de 4000 sous-titres à l'envers | 268 µs | 20.7 µs |
| suppression d'un sous-titre sur deux | 162 µs | 19.5 µs |
| suppression puis annulation | 245 µs | 33.6 µs |
| insertion de 100 sous-titres vides au milieu | 53.2 µs | 14.9 µs |
| modification d'un texte, à travers une session | 184 ns | 83.9 ns |
| suppression des mentions sur 4000 sous-titres | 1.29 ms | 92 µs |
| mise en italique de 4000 sous-titres | 911 µs | 57.6 µs |
| remplacement d'un mot fréquent sur 4000 sous-titres, texte simple | 3.98 ms | 234 µs |
| remplacement d'un mot fréquent sur 4000 sous-titres, expression régulière | 4.15 ms | 228 µs |
| recherche sans résultat sur 4000 sous-titres | 2.73 ms | 330 µs |
| ajustement des durées de 4000 sous-titres, quatre contraintes | 1.24 ms | 55.6 µs |
| casse de titre sur 4000 sous-titres | 17.8 ms | 519 µs |
| collage de 4000 textes | 983 µs | 38.7 µs |
| alignement d'une traduction de 4000 lignes, par position | 817 µs | 48 µs |
| alignement d'une traduction de 4000 lignes, par numéro | 809 µs | 46.2 µs |
| déduction de fréquence sur 4000 sous-titres | 374 µs | 24.5 µs |
| recherche de conversion sur 4000 sous-titres | 367 µs | 14.9 µs |
| walk of 200 files in a tree of depth 3 | 560 µs | 36.9 µs |
| shift of 200 files of 20 subtitles | 31.5 ms | 1.23 ms |
| shift of one file of 4000 subtitles | 3.35 ms | 584 µs |

### 0.15.22 — 2026-10-13 — Release — load 1.39 — pace ×1.12

| Measure | Mean | Standard deviation |
| :----- | ------: | ---------: |
| la réplique en cours, sur 4000 sous-titres | 5.8 µs | 1.64 µs |
| traduire une réplique balisée pour l'image | 826 ns | 31.2 ns |
| composer une réplique de deux lignes | 128 ns | 8.09 ns |
| ouvrir une vidéo | 219 ms | 18.9 ms |
| chercher une position | 5.33 ms | 97.3 µs |
| rendre une image de 640×360 | 352 µs | 58.2 µs |
| rendre une image de 1280×720 | 962 µs | 106 µs |
| rendre une image de 1920×1080 | 2.35 ms | 187 µs |
| chercher l'image 0 (vidéo 720p) | 6.33 ms | 399 µs |
| chercher l'image 100 (vidéo 720p) | 50.6 ms | 4.12 ms |
| chercher l'image 249 (vidéo 720p) | 121 ms | 12.9 ms |
| pas avant vers l'image 100 (vidéo 720p) | 51 ms | 8.39 ms |
| pas avant vers l'image 249 (vidéo 720p) | 135 ms | 27.4 ms |
| pas arrière vers l'image 99 (vidéo 720p) | 51.4 ms | 5.83 ms |
| pas arrière vers l'image 248 (vidéo 720p) | 118 ms | 12.6 ms |
| construction du modèle sur 4000 sous-titres | 11.4 µs | 4.2 µs |
| une fenêtre de 40 lignes, cinq colonnes | 20.1 µs | 986 ns |
| rafraîchir après un décalage de 4000 sous-titres | 13.1 µs | 3.27 µs |
| réinitialisation du modèle après une ligne retirée | 12.7 µs | 3.44 µs |
| édition d'une cellule de texte | 514 ns | 339 ns |
| édition d'une cellule de position | 14.1 µs | 3.16 µs |
| centrer une ligne sur 4000 sous-titres | 1.26 µs | 269 ns |
| découpage de 4000 sous-titres, en ems, avec cache de longueurs | 207 ms | 15.2 ms |
| versionString | 54.9 ns | 15.3 ns |
| parse | 44.2 ns | 11.1 ns |
| format | 39.2 ns | 0.58 ns |
| position vers image | 7.25 ns | 0.0692 ns |
| image vers position | 7.42 ns | 0.132 ns |
| mise à l'échelle par un rationnel exact | 8.8 ns | 3.06 ns |
| découpage de 4000 sous-titres, sans cache de longueurs | 128 ms | 5.05 ms |
| découpage de 4000 sous-titres, avec cache de longueurs | 127 ms | 8.4 ms |
| erreurs courantes du français sur 4000 sous-titres | 178 ms | 10.3 ms |
| erreurs courantes de l'anglais sur 4000 sous-titres | 195 ms | 10.5 ms |
| lecture de 4000 sous-titres | 2.29 ms | 70.5 µs |
| écriture de 4000 sous-titres | 604 µs | 68.4 µs |
| décalage de 4000 sous-titres | 9.47 µs | 3.1 µs |
| décalage puis annulation | 18.1 µs | 2.47 µs |
| transformation de 4000 sous-titres | 76.7 µs | 13.3 µs |
| conversion de fréquence sur 4000 sous-titres | 89.4 µs | 10.7 µs |
| alignement sur 4000 sous-titres | 153 µs | 9.61 µs |
| tri de 4000 sous-titres à l'envers | 344 µs | 42.4 µs |
| suppression d'un sous-titre sur deux | 196 µs | 26.9 µs |
| suppression puis annulation | 280 µs | 18.2 µs |
| insertion de 100 sous-titres vides au milieu | 61.1 µs | 21.8 µs |
| modification d'un texte, à travers une session | 225 ns | 34.4 ns |
| suppression des mentions sur 4000 sous-titres | 1.4 ms | 53.4 µs |
| mise en italique de 4000 sous-titres | 1.02 ms | 68.6 µs |
| remplacement d'un mot fréquent sur 4000 sous-titres, texte simple | 4.48 ms | 212 µs |
| remplacement d'un mot fréquent sur 4000 sous-titres, expression régulière | 4.84 ms | 465 µs |
| recherche sans résultat sur 4000 sous-titres | 2.81 ms | 253 µs |
| ajustement des durées de 4000 sous-titres, quatre contraintes | 1.41 ms | 78.2 µs |
| casse de titre sur 4000 sous-titres | 20.1 ms | 1.34 ms |
| collage de 4000 textes | 1.13 ms | 43.5 µs |
| alignement d'une traduction de 4000 lignes, par position | 946 µs | 37.4 µs |
| alignement d'une traduction de 4000 lignes, par numéro | 954 µs | 46.1 µs |
| déduction de fréquence sur 4000 sous-titres | 424 µs | 50.2 µs |
| recherche de conversion sur 4000 sous-titres | 421 µs | 11.7 µs |
| walk of 200 files in a tree of depth 3 | 677 µs | 97.4 µs |
| shift of 200 files of 20 subtitles | 37.1 ms | 1.82 ms |
| shift of one file of 4000 subtitles | 3.17 ms | 205 µs |

### 0.15.20 — 2026-10-12 — Release — load 1.49 — pace ×1.00

| Measure | Mean | Standard deviation |
| :----- | ------: | ---------: |
| la réplique en cours, sur 4000 sous-titres | 7.26 µs | 175 ns |
| traduire une réplique balisée pour l'image | 825 ns | 22.9 ns |
| composer une réplique de deux lignes | 108 ns | 2.68 ns |
| ouvrir une vidéo | 219 ms | 1.91 ms |
| chercher une position | 5.39 ms | 91 µs |
| rendre une image de 640×360 | 311 µs | 23.7 µs |
| rendre une image de 1280×720 | 841 µs | 58.1 µs |
| rendre une image de 1920×1080 | 2.06 ms | 110 µs |
| chercher l'image 0 (vidéo 720p) | 6.21 ms | 285 µs |
| chercher l'image 100 (vidéo 720p) | 47.3 ms | 2.31 ms |
| chercher l'image 249 (vidéo 720p) | 111 ms | 3.47 ms |
| pas avant vers l'image 100 (vidéo 720p) | 47 ms | 3.09 ms |
| pas avant vers l'image 249 (vidéo 720p) | 111 ms | 1.96 ms |
| pas arrière vers l'image 99 (vidéo 720p) | 47.2 ms | 1.91 ms |
| pas arrière vers l'image 248 (vidéo 720p) | 111 ms | 2.67 ms |
| construction du modèle sur 4000 sous-titres | 8.2 µs | 2.26 µs |
| une fenêtre de 40 lignes, cinq colonnes | 19.8 µs | 7.05 µs |
| rafraîchir après un décalage de 4000 sous-titres | 8.06 µs | 800 ns |
| réinitialisation du modèle après une ligne retirée | 8.03 µs | 643 ns |
| édition d'une cellule de texte | 404 ns | 55.1 ns |
| édition d'une cellule de position | 9.15 µs | 1.64 µs |
| centrer une ligne sur 4000 sous-titres | 1.15 µs | 460 ns |
| découpage de 4000 sous-titres, en ems, avec cache de longueurs | 180 ms | 8.36 ms |
| versionString | 44.9 ns | 0.437 ns |
| parse | 35.8 ns | 3.08 ns |
| format | 32.6 ns | 1.67 ns |
| position vers image | 6.56 ns | 1.24 ns |
| image vers position | 6.46 ns | 0.0486 ns |
| mise à l'échelle par un rationnel exact | 7.37 ns | 2.6 ns |
| découpage de 4000 sous-titres, sans cache de longueurs | 111 ms | 3.84 ms |
| découpage de 4000 sous-titres, avec cache de longueurs | 119 ms | 7.78 ms |
| erreurs courantes du français sur 4000 sous-titres | 160 ms | 10 ms |
| erreurs courantes de l'anglais sur 4000 sous-titres | 174 ms | 7.78 ms |
| lecture de 4000 sous-titres | 2.02 ms | 84.7 µs |
| écriture de 4000 sous-titres | 530 µs | 55.2 µs |
| décalage de 4000 sous-titres | 8.26 µs | 1.94 µs |
| décalage puis annulation | 15.4 µs | 2.74 µs |
| transformation de 4000 sous-titres | 70.9 µs | 5.8 µs |
| conversion de fréquence sur 4000 sous-titres | 75.7 µs | 12.7 µs |
| alignement sur 4000 sous-titres | 134 µs | 17 µs |
| tri de 4000 sous-titres à l'envers | 275 µs | 25.8 µs |
| suppression d'un sous-titre sur deux | 162 µs | 21.4 µs |
| suppression puis annulation | 246 µs | 31.8 µs |
| insertion de 100 sous-titres vides au milieu | 61.4 µs | 17.8 µs |
| modification d'un texte, à travers une session | 183 ns | 17.9 ns |
| suppression des mentions sur 4000 sous-titres | 1.25 ms | 88 µs |
| mise en italique de 4000 sous-titres | 917 µs | 45.7 µs |
| remplacement d'un mot fréquent sur 4000 sous-titres, texte simple | 3.94 ms | 260 µs |
| remplacement d'un mot fréquent sur 4000 sous-titres, expression régulière | 4.16 ms | 328 µs |
| recherche sans résultat sur 4000 sous-titres | 2.72 ms | 169 µs |
| ajustement des durées de 4000 sous-titres, quatre contraintes | 1.23 ms | 46.9 µs |
| casse de titre sur 4000 sous-titres | 17.8 ms | 617 µs |
| collage de 4000 textes | 992 µs | 59.8 µs |
| alignement d'une traduction de 4000 lignes, par position | 814 µs | 44.4 µs |
| alignement d'une traduction de 4000 lignes, par numéro | 799 µs | 45.9 µs |
| déduction de fréquence sur 4000 sous-titres | 423 µs | 6.52 µs |
| recherche de conversion sur 4000 sous-titres | 367 µs | 17.9 µs |
| walk of 200 files in a tree of depth 3 | 563 µs | 28.2 µs |
| shift of 200 files of 20 subtitles | 35.4 ms | 1.15 ms |
| shift of one file of 4000 subtitles | 2.87 ms | 91.3 µs |

### 0.15.19 — 2026-10-12 — Release — load 1.45 — pace ×0.99

| Measure | Mean | Standard deviation |
| :----- | ------: | ---------: |
| la réplique en cours, sur 4000 sous-titres | 5.97 µs | 1.81 µs |
| traduire une réplique balisée pour l'image | 835 ns | 112 ns |
| composer une réplique de deux lignes | 110 ns | 3.27 ns |
| ouvrir une vidéo | 219 ms | 1.82 ms |
| chercher une position | 5.42 ms | 92.4 µs |
| rendre une image de 640×360 | 314 µs | 21.7 µs |
| rendre une image de 1280×720 | 841 µs | 44.5 µs |
| rendre une image de 1920×1080 | 2.25 ms | 190 µs |
| chercher l'image 0 (vidéo 720p) | 6.25 ms | 257 µs |
| chercher l'image 100 (vidéo 720p) | 46.2 ms | 2.53 ms |
| chercher l'image 249 (vidéo 720p) | 109 ms | 4.56 ms |
| pas avant vers l'image 100 (vidéo 720p) | 46.4 ms | 2.39 ms |
| pas avant vers l'image 249 (vidéo 720p) | 108 ms | 5.42 ms |
| pas arrière vers l'image 99 (vidéo 720p) | 46.4 ms | 3.93 ms |
| pas arrière vers l'image 248 (vidéo 720p) | 109 ms | 3.55 ms |
| construction du modèle sur 4000 sous-titres | 8.25 µs | 1.36 µs |
| une fenêtre de 40 lignes, cinq colonnes | 17.6 µs | 379 ns |
| rafraîchir après un décalage de 4000 sous-titres | 8.08 µs | 875 ns |
| réinitialisation du modèle après une ligne retirée | 8.03 µs | 514 ns |
| édition d'une cellule de texte | 395 ns | 37.7 ns |
| édition d'une cellule de position | 9.06 µs | 335 ns |
| centrer une ligne sur 4000 sous-titres | 1.08 µs | 78.7 ns |
| découpage de 4000 sous-titres, en ems, avec cache de longueurs | 176 ms | 4.79 ms |
| versionString | 45 ns | 0.572 ns |
| parse | 38.5 ns | 9.67 ns |
| format | 31.4 ns | 0.633 ns |
| position vers image | 6.32 ns | 0.06 ns |
| image vers position | 6.47 ns | 0.0664 ns |
| mise à l'échelle par un rationnel exact | 6.71 ns | 0.0541 ns |
| découpage de 4000 sous-titres, sans cache de longueurs | 109 ms | 3.65 ms |
| découpage de 4000 sous-titres, avec cache de longueurs | 114 ms | 3.45 ms |
| erreurs courantes du français sur 4000 sous-titres | 155 ms | 3.66 ms |
| erreurs courantes de l'anglais sur 4000 sous-titres | 173 ms | 6.62 ms |
| lecture de 4000 sous-titres | 2.25 ms | 449 µs |
| écriture de 4000 sous-titres | 507 µs | 24.6 µs |
| décalage de 4000 sous-titres | 6.96 µs | 2.99 µs |
| décalage puis annulation | 13.4 µs | 2.94 µs |
| transformation de 4000 sous-titres | 72.7 µs | 13.6 µs |
| conversion de fréquence sur 4000 sous-titres | 74.3 µs | 6.44 µs |
| alignement sur 4000 sous-titres | 130 µs | 8.12 µs |
| tri de 4000 sous-titres à l'envers | 272 µs | 25.7 µs |
| suppression d'un sous-titre sur deux | 164 µs | 23.6 µs |
| suppression puis annulation | 244 µs | 24.9 µs |
| insertion de 100 sous-titres vides au milieu | 53.2 µs | 14.8 µs |
| modification d'un texte, à travers une session | 184 ns | 11.3 ns |
| suppression des mentions sur 4000 sous-titres | 1.22 ms | 48.7 µs |
| mise en italique de 4000 sous-titres | 907 µs | 36.1 µs |
| remplacement d'un mot fréquent sur 4000 sous-titres, texte simple | 4.08 ms | 319 µs |
| remplacement d'un mot fréquent sur 4000 sous-titres, expression régulière | 4.07 ms | 96 µs |
| recherche sans résultat sur 4000 sous-titres | 2.59 ms | 56.7 µs |
| ajustement des durées de 4000 sous-titres, quatre contraintes | 1.23 ms | 46.1 µs |
| casse de titre sur 4000 sous-titres | 17.8 ms | 710 µs |
| collage de 4000 textes | 1.03 ms | 46.1 µs |
| alignement d'une traduction de 4000 lignes, par position | 824 µs | 49.8 µs |
| alignement d'une traduction de 4000 lignes, par numéro | 931 µs | 77.4 µs |
| déduction de fréquence sur 4000 sous-titres | 372 µs | 15.3 µs |
| recherche de conversion sur 4000 sous-titres | 382 µs | 25.6 µs |
| walk of 200 files in a tree of depth 3 | 556 µs | 25 µs |
| shift of 200 files of 20 subtitles | 33.2 ms | 1.95 ms |
| shift of one file of 4000 subtitles | 2.86 ms | 93.4 µs |

### 0.15.18 — 2026-10-12 — Release — load 1.46 — pace ×1.18

| Measure | Mean | Standard deviation |
| :----- | ------: | ---------: |
| la réplique en cours, sur 4000 sous-titres | 6.82 µs | 2 µs |
| traduire une réplique balisée pour l'image | 1.14 µs | 630 ns |
| composer une réplique de deux lignes | 128 ns | 10.6 ns |
| ouvrir une vidéo | 221 ms | 1.99 ms |
| chercher une position | 5.35 ms | 112 µs |
| rendre une image de 640×360 | 356 µs | 23.2 µs |
| rendre une image de 1280×720 | 1.01 ms | 142 µs |
| rendre une image de 1920×1080 | 2.35 ms | 201 µs |
| chercher l'image 0 (vidéo 720p) | 6.34 ms | 254 µs |
| chercher l'image 100 (vidéo 720p) | 49.1 ms | 2.18 ms |
| chercher l'image 249 (vidéo 720p) | 115 ms | 3.16 ms |
| pas avant vers l'image 100 (vidéo 720p) | 49 ms | 2.03 ms |
| pas avant vers l'image 249 (vidéo 720p) | 115 ms | 3.52 ms |
| pas arrière vers l'image 99 (vidéo 720p) | 49.4 ms | 2.47 ms |
| pas arrière vers l'image 248 (vidéo 720p) | 115 ms | 3.76 ms |
| construction du modèle sur 4000 sous-titres | 12.6 µs | 4.08 µs |
| une fenêtre de 40 lignes, cinq colonnes | 20.9 µs | 1.62 µs |
| rafraîchir après un décalage de 4000 sous-titres | 11.4 µs | 6.08 µs |
| réinitialisation du modèle après une ligne retirée | 12.3 µs | 1.65 µs |
| édition d'une cellule de texte | 472 ns | 147 ns |
| édition d'une cellule de position | 13.4 µs | 1.5 µs |
| centrer une ligne sur 4000 sous-titres | 1.24 µs | 214 ns |
| découpage de 4000 sous-titres, en ems, avec cache de longueurs | 206 ms | 2.88 ms |
| versionString | 51.7 ns | 0.807 ns |
| parse | 50.4 ns | 22.2 ns |
| format | 38 ns | 0.782 ns |
| position vers image | 8.04 ns | 2.13 ns |
| image vers position | 7.24 ns | 0.067 ns |
| mise à l'échelle par un rationnel exact | 7.92 ns | 1.52 ns |
| découpage de 4000 sous-titres, sans cache de longueurs | 127 ms | 1.97 ms |
| découpage de 4000 sous-titres, avec cache de longueurs | 131 ms | 1.8 ms |
| erreurs courantes du français sur 4000 sous-titres | 179 ms | 2.27 ms |
| erreurs courantes de l'anglais sur 4000 sous-titres | 200 ms | 2.42 ms |
| lecture de 4000 sous-titres | 2.47 ms | 239 µs |
| écriture de 4000 sous-titres | 631 µs | 172 µs |
| décalage de 4000 sous-titres | 9.51 µs | 3.21 µs |
| décalage puis annulation | 18.3 µs | 9.61 µs |
| transformation de 4000 sous-titres | 94 µs | 28.4 µs |
| conversion de fréquence sur 4000 sous-titres | 94.5 µs | 10.2 µs |
| alignement sur 4000 sous-titres | 160 µs | 16.3 µs |
| tri de 4000 sous-titres à l'envers | 328 µs | 24 µs |
| suppression d'un sous-titre sur deux | 200 µs | 21.9 µs |
| suppression puis annulation | 294 µs | 59 µs |
| insertion de 100 sous-titres vides au milieu | 66.9 µs | 8.4 µs |
| modification d'un texte, à travers une session | 213 ns | 27.7 ns |
| suppression des mentions sur 4000 sous-titres | 1.38 ms | 110 µs |
| mise en italique de 4000 sous-titres | 1.07 ms | 68.4 µs |
| remplacement d'un mot fréquent sur 4000 sous-titres, texte simple | 4.56 ms | 211 µs |
| remplacement d'un mot fréquent sur 4000 sous-titres, expression régulière | 4.73 ms | 203 µs |
| recherche sans résultat sur 4000 sous-titres | 3.05 ms | 148 µs |
| ajustement des durées de 4000 sous-titres, quatre contraintes | 1.47 ms | 63.1 µs |
| casse de titre sur 4000 sous-titres | 21.6 ms | 580 µs |
| collage de 4000 textes | 1.16 ms | 79.7 µs |
| alignement d'une traduction de 4000 lignes, par position | 983 µs | 84.4 µs |
| alignement d'une traduction de 4000 lignes, par numéro | 984 µs | 96.7 µs |
| déduction de fréquence sur 4000 sous-titres | 436 µs | 27.6 µs |
| recherche de conversion sur 4000 sous-titres | 451 µs | 58 µs |
| walk of 200 files in a tree of depth 3 | 665 µs | 72.3 µs |
| shift of 200 files of 20 subtitles | 38.5 ms | 705 µs |
| shift of one file of 4000 subtitles | 3.58 ms | 423 µs |

### 0.15.15 — 2026-10-12 — Release — load 1.42 — pace ×0.99

| Measure | Mean | Standard deviation |
| :----- | ------: | ---------: |
| la réplique en cours, sur 4000 sous-titres | 5.72 µs | 251 ns |
| traduire une réplique balisée pour l'image | 828 ns | 41.4 ns |
| composer une réplique de deux lignes | 111 ns | 2.46 ns |
| ouvrir une vidéo | 219 ms | 1.73 ms |
| chercher une position | 5.38 ms | 101 µs |
| rendre une image de 640×360 | 313 µs | 24.9 µs |
| rendre une image de 1280×720 | 849 µs | 39.3 µs |
| rendre une image de 1920×1080 | 2.02 ms | 82.9 µs |
| chercher l'image 0 (vidéo 720p) | 6.21 ms | 254 µs |
| chercher l'image 100 (vidéo 720p) | 44.7 ms | 2.97 ms |
| chercher l'image 249 (vidéo 720p) | 105 ms | 5.69 ms |
| pas avant vers l'image 100 (vidéo 720p) | 45 ms | 2.55 ms |
| pas avant vers l'image 249 (vidéo 720p) | 104 ms | 5.99 ms |
| pas arrière vers l'image 99 (vidéo 720p) | 44.7 ms | 2.61 ms |
| pas arrière vers l'image 248 (vidéo 720p) | 105 ms | 6.46 ms |
| construction du modèle sur 4000 sous-titres | 8.04 µs | 778 ns |
| une fenêtre de 40 lignes, cinq colonnes | 17.8 µs | 1.07 µs |
| rafraîchir après un décalage de 4000 sous-titres | 8.02 µs | 490 ns |
| réinitialisation du modèle après une ligne retirée | 8.23 µs | 2.71 µs |
| édition d'une cellule de texte | 380 ns | 24.3 ns |
| édition d'une cellule de position | 9.01 µs | 440 ns |
| centrer une ligne sur 4000 sous-titres | 1.07 µs | 126 ns |
| découpage de 4000 sous-titres, en ems, avec cache de longueurs | 175 ms | 4.38 ms |
| versionString | 46 ns | 5.79 ns |
| parse | 36.4 ns | 3.23 ns |
| format | 36.9 ns | 0.365 ns |
| position vers image | 6.48 ns | 0.0604 ns |
| image vers position | 6.32 ns | 0.0662 ns |
| mise à l'échelle par un rationnel exact | 6.72 ns | 0.0697 ns |
| découpage de 4000 sous-titres, sans cache de longueurs | 110 ms | 6.82 ms |
| découpage de 4000 sous-titres, avec cache de longueurs | 113 ms | 4.75 ms |
| erreurs courantes du français sur 4000 sous-titres | 154 ms | 4.18 ms |
| erreurs courantes de l'anglais sur 4000 sous-titres | 170 ms | 3.39 ms |
| lecture de 4000 sous-titres | 2.02 ms | 74.1 µs |
| écriture de 4000 sous-titres | 536 µs | 36.5 µs |
| décalage de 4000 sous-titres | 6.84 µs | 2.17 µs |
| décalage puis annulation | 13.4 µs | 3.07 µs |
| transformation de 4000 sous-titres | 77 µs | 16.8 µs |
| conversion de fréquence sur 4000 sous-titres | 98.7 µs | 23.4 µs |
| alignement sur 4000 sous-titres | 134 µs | 11.5 µs |
| tri de 4000 sous-titres à l'envers | 274 µs | 20.2 µs |
| suppression d'un sous-titre sur deux | 161 µs | 15.3 µs |
| suppression puis annulation | 245 µs | 26.5 µs |
| insertion de 100 sous-titres vides au milieu | 55.9 µs | 20.6 µs |
| modification d'un texte, à travers une session | 173 ns | 8.46 ns |
| suppression des mentions sur 4000 sous-titres | 1.2 ms | 28.6 µs |
| mise en italique de 4000 sous-titres | 914 µs | 39.1 µs |
| remplacement d'un mot fréquent sur 4000 sous-titres, texte simple | 3.83 ms | 62 µs |
| remplacement d'un mot fréquent sur 4000 sous-titres, expression régulière | 4.01 ms | 56.3 µs |
| recherche sans résultat sur 4000 sous-titres | 2.59 ms | 45.4 µs |
| ajustement des durées de 4000 sous-titres, quatre contraintes | 1.22 ms | 46.7 µs |
| casse de titre sur 4000 sous-titres | 17.6 ms | 940 µs |
| collage de 4000 textes | 965 µs | 32.2 µs |
| alignement d'une traduction de 4000 lignes, par position | 807 µs | 44.9 µs |
| alignement d'une traduction de 4000 lignes, par numéro | 788 µs | 39.3 µs |
| déduction de fréquence sur 4000 sous-titres | 369 µs | 1.6 µs |
| recherche de conversion sur 4000 sous-titres | 363 µs | 1.3 µs |
| walk of 200 files in a tree of depth 3 | 559 µs | 27 µs |
| shift of 200 files of 20 subtitles | 31.9 ms | 1.5 ms |
| shift of one file of 4000 subtitles | 2.88 ms | 87.1 µs |

### 0.15.13 — 2026-10-12 — Release — load 1.49 — pace ×0.99

| Measure | Mean | Standard deviation |
| :----- | ------: | ---------: |
| la réplique en cours, sur 4000 sous-titres | 7.2 µs | 198 ns |
| traduire une réplique balisée pour l'image | 837 ns | 24.4 ns |
| composer une réplique de deux lignes | 119 ns | 29.9 ns |
| ouvrir une vidéo | 219 ms | 1.95 ms |
| chercher une position | 5.38 ms | 88.4 µs |
| rendre une image de 640×360 | 322 µs | 32.4 µs |
| rendre une image de 1280×720 | 841 µs | 40.2 µs |
| rendre une image de 1920×1080 | 2.34 ms | 110 µs |
| chercher l'image 0 (vidéo 720p) | 6.22 ms | 151 µs |
| chercher l'image 100 (vidéo 720p) | 46.8 ms | 2.08 ms |
| chercher l'image 249 (vidéo 720p) | 111 ms | 3.46 ms |
| pas avant vers l'image 100 (vidéo 720p) | 46.4 ms | 2.24 ms |
| pas avant vers l'image 249 (vidéo 720p) | 112 ms | 9.05 ms |
| pas arrière vers l'image 99 (vidéo 720p) | 46.5 ms | 2.45 ms |
| pas arrière vers l'image 248 (vidéo 720p) | 110 ms | 2.61 ms |
| construction du modèle sur 4000 sous-titres | 8.36 µs | 3.21 µs |
| une fenêtre de 40 lignes, cinq colonnes | 18.3 µs | 3.3 µs |
| rafraîchir après un décalage de 4000 sous-titres | 9.61 µs | 3.22 µs |
| réinitialisation du modèle après une ligne retirée | 8.41 µs | 2.06 µs |
| édition d'une cellule de texte | 376 ns | 23.8 ns |
| édition d'une cellule de position | 9.14 µs | 440 ns |
| centrer une ligne sur 4000 sous-titres | 1.08 µs | 210 ns |
| découpage de 4000 sous-titres, en ems, avec cache de longueurs | 179 ms | 9.41 ms |
| versionString | 46.1 ns | 0.469 ns |
| parse | 36 ns | 2.89 ns |
| format | 37 ns | 0.645 ns |
| position vers image | 6.48 ns | 0.0682 ns |
| image vers position | 6.82 ns | 2.08 ns |
| mise à l'échelle par un rationnel exact | 6.71 ns | 0.0515 ns |
| découpage de 4000 sous-titres, sans cache de longueurs | 108 ms | 2.87 ms |
| découpage de 4000 sous-titres, avec cache de longueurs | 112 ms | 3.75 ms |
| erreurs courantes du français sur 4000 sous-titres | 158 ms | 10.4 ms |
| erreurs courantes de l'anglais sur 4000 sous-titres | 169 ms | 5.02 ms |
| lecture de 4000 sous-titres | 2.03 ms | 86.3 µs |
| écriture de 4000 sous-titres | 535 µs | 30.5 µs |
| décalage de 4000 sous-titres | 7.12 µs | 3.56 µs |
| décalage puis annulation | 13.6 µs | 3.48 µs |
| transformation de 4000 sous-titres | 75.6 µs | 9.29 µs |
| conversion de fréquence sur 4000 sous-titres | 74.8 µs | 14.4 µs |
| alignement sur 4000 sous-titres | 133 µs | 9.89 µs |
| tri de 4000 sous-titres à l'envers | 273 µs | 23.4 µs |
| suppression d'un sous-titre sur deux | 163 µs | 22.9 µs |
| suppression puis annulation | 246 µs | 28.5 µs |
| insertion de 100 sous-titres vides au milieu | 53.5 µs | 21.3 µs |
| modification d'un texte, à travers une session | 176 ns | 9.42 ns |
| suppression des mentions sur 4000 sous-titres | 1.2 ms | 44.7 µs |
| mise en italique de 4000 sous-titres | 928 µs | 50.5 µs |
| remplacement d'un mot fréquent sur 4000 sous-titres, texte simple | 3.86 ms | 79 µs |
| remplacement d'un mot fréquent sur 4000 sous-titres, expression régulière | 4.02 ms | 71.9 µs |
| recherche sans résultat sur 4000 sous-titres | 2.62 ms | 65.5 µs |
| ajustement des durées de 4000 sous-titres, quatre contraintes | 1.22 ms | 45.4 µs |
| casse de titre sur 4000 sous-titres | 18 ms | 581 µs |
| collage de 4000 textes | 977 µs | 54.7 µs |
| alignement d'une traduction de 4000 lignes, par position | 910 µs | 49.3 µs |
| alignement d'une traduction de 4000 lignes, par numéro | 795 µs | 43.1 µs |
| déduction de fréquence sur 4000 sous-titres | 424 µs | 9.87 µs |
| recherche de conversion sur 4000 sous-titres | 366 µs | 13.5 µs |
| walk of 200 files in a tree of depth 3 | 558 µs | 19.1 µs |
| shift of 200 files of 20 subtitles | 34.9 ms | 1.05 ms |
| shift of one file of 4000 subtitles | 3.01 ms | 183 µs |

### 0.15.10 — 2026-10-10 — Release — load 1.36 — pace ×0.99

| Measure | Mean | Standard deviation |
| :----- | ------: | ---------: |
| la réplique en cours, sur 4000 sous-titres | 5.84 µs | 1.85 µs |
| traduire une réplique balisée pour l'image | 829 ns | 28.1 ns |
| composer une réplique de deux lignes | 140 ns | 1.45 ns |
| ouvrir une vidéo | 219 ms | 1.68 ms |
| chercher une position | 5.4 ms | 93.9 µs |
| rendre une image de 640×360 | 313 µs | 27.9 µs |
| rendre une image de 1280×720 | 832 µs | 35.7 µs |
| rendre une image de 1920×1080 | 2.07 ms | 114 µs |
| chercher l'image 0 (vidéo 720p) | 6.17 ms | 258 µs |
| chercher l'image 100 (vidéo 720p) | 45.3 ms | 3.07 ms |
| chercher l'image 249 (vidéo 720p) | 107 ms | 7.42 ms |
| pas avant vers l'image 100 (vidéo 720p) | 45.2 ms | 3.58 ms |
| pas avant vers l'image 249 (vidéo 720p) | 107 ms | 6.55 ms |
| pas arrière vers l'image 99 (vidéo 720p) | 45 ms | 3.45 ms |
| pas arrière vers l'image 248 (vidéo 720p) | 106 ms | 6.78 ms |
| construction du modèle sur 4000 sous-titres | 8.03 µs | 256 ns |
| une fenêtre de 40 lignes, cinq colonnes | 20.1 µs | 974 ns |
| rafraîchir après un décalage de 4000 sous-titres | 8.15 µs | 1.64 µs |
| réinitialisation du modèle après une ligne retirée | 8.44 µs | 4.69 µs |
| édition d'une cellule de texte | 390 ns | 85.6 ns |
| édition d'une cellule de position | 9.02 µs | 373 ns |
| centrer une ligne sur 4000 sous-titres | 1.08 µs | 137 ns |
| découpage de 4000 sous-titres, en ems, avec cache de longueurs | 177 ms | 5.71 ms |
| versionString | 43.9 ns | 0.389 ns |
| parse | 38.8 ns | 3.5 ns |
| format | 36.6 ns | 0.554 ns |
| position vers image | 6.65 ns | 0.0698 ns |
| image vers position | 7.8 ns | 1.48 ns |
| mise à l'échelle par un rationnel exact | 6.87 ns | 0.0763 ns |
| découpage de 4000 sous-titres, sans cache de longueurs | 110 ms | 4.62 ms |
| découpage de 4000 sous-titres, avec cache de longueurs | 114 ms | 4.2 ms |
| erreurs courantes du français sur 4000 sous-titres | 156 ms | 6.72 ms |
| erreurs courantes de l'anglais sur 4000 sous-titres | 172 ms | 5.62 ms |
| lecture de 4000 sous-titres | 2.09 ms | 132 µs |
| écriture de 4000 sous-titres | 570 µs | 39.9 µs |
| décalage de 4000 sous-titres | 6.76 µs | 2.85 µs |
| décalage puis annulation | 15.2 µs | 6.94 µs |
| transformation de 4000 sous-titres | 85.1 µs | 6.12 µs |
| conversion de fréquence sur 4000 sous-titres | 75.9 µs | 6.51 µs |
| alignement sur 4000 sous-titres | 134 µs | 10.8 µs |
| tri de 4000 sous-titres à l'envers | 305 µs | 13.2 µs |
| suppression d'un sous-titre sur deux | 184 µs | 14.2 µs |
| suppression puis annulation | 258 µs | 28.2 µs |
| insertion de 100 sous-titres vides au milieu | 55 µs | 15.3 µs |
| modification d'un texte, à travers une session | 173 ns | 13 ns |
| suppression des mentions sur 4000 sous-titres | 1.21 ms | 23.8 µs |
| mise en italique de 4000 sous-titres | 917 µs | 37.5 µs |
| remplacement d'un mot fréquent sur 4000 sous-titres, texte simple | 3.9 ms | 190 µs |
| remplacement d'un mot fréquent sur 4000 sous-titres, expression régulière | 4.02 ms | 73.1 µs |
| recherche sans résultat sur 4000 sous-titres | 2.59 ms | 48.1 µs |
| ajustement des durées de 4000 sous-titres, quatre contraintes | 1.25 ms | 51.2 µs |
| casse de titre sur 4000 sous-titres | 18.1 ms | 784 µs |
| collage de 4000 textes | 987 µs | 55 µs |
| alignement d'une traduction de 4000 lignes, par position | 1.06 ms | 117 µs |
| alignement d'une traduction de 4000 lignes, par numéro | 800 µs | 30 µs |
| déduction de fréquence sur 4000 sous-titres | 367 µs | 9.23 µs |
| recherche de conversion sur 4000 sous-titres | 365 µs | 15.7 µs |
| walk of 200 files in a tree of depth 3 | 563 µs | 38.6 µs |
| shift of 200 files of 20 subtitles | 32.4 ms | 1.98 ms |
| shift of one file of 4000 subtitles | 2.85 ms | 354 µs |

### 0.15.9 — 2026-10-10 — Release — load 1.46 — pace ×0.96

| Measure | Mean | Standard deviation |
| :----- | ------: | ---------: |
| la réplique en cours, sur 4000 sous-titres | 7.71 µs | 1.83 µs |
| traduire une réplique balisée pour l'image | 856 ns | 174 ns |
| composer une réplique de deux lignes | 127 ns | 50.7 ns |
| ouvrir une vidéo | 219 ms | 1.4 ms |
| chercher une position | 5.39 ms | 105 µs |
| rendre une image de 640×360 | 305 µs | 17.3 µs |
| rendre une image de 1280×720 | 833 µs | 56.1 µs |
| rendre une image de 1920×1080 | 2.05 ms | 151 µs |
| chercher l'image 0 (vidéo 720p) | 6.22 ms | 191 µs |
| chercher l'image 100 (vidéo 720p) | 44.6 ms | 2.78 ms |
| chercher l'image 249 (vidéo 720p) | 107 ms | 5.81 ms |
| pas avant vers l'image 100 (vidéo 720p) | 45.5 ms | 3.14 ms |
| pas avant vers l'image 249 (vidéo 720p) | 106 ms | 6.26 ms |
| pas arrière vers l'image 99 (vidéo 720p) | 44.6 ms | 2.88 ms |
| pas arrière vers l'image 248 (vidéo 720p) | 106 ms | 5.77 ms |
| construction du modèle sur 4000 sous-titres | 8.08 µs | 894 ns |
| une fenêtre de 40 lignes, cinq colonnes | 19.8 µs | 6.97 µs |
| rafraîchir après un décalage de 4000 sous-titres | 8.24 µs | 2.63 µs |
| réinitialisation du modèle après une ligne retirée | 8.18 µs | 2.34 µs |
| édition d'une cellule de texte | 382 ns | 27.8 ns |
| édition d'une cellule de position | 9.01 µs | 294 ns |
| centrer une ligne sur 4000 sous-titres | 1.25 µs | 338 ns |
| découpage de 4000 sous-titres, en ems, avec cache de longueurs | 178 ms | 6.1 ms |
| versionString | 45.3 ns | 18.8 ns |
| parse | 37.2 ns | 4.58 ns |
| format | 36.7 ns | 5.88 ns |
| position vers image | 6.64 ns | 0.0625 ns |
| image vers position | 6.64 ns | 0.0465 ns |
| mise à l'échelle par un rationnel exact | 8.27 ns | 1.92 ns |
| découpage de 4000 sous-titres, sans cache de longueurs | 110 ms | 3.47 ms |
| découpage de 4000 sous-titres, avec cache de longueurs | 114 ms | 4.77 ms |
| erreurs courantes du français sur 4000 sous-titres | 156 ms | 4.63 ms |
| erreurs courantes de l'anglais sur 4000 sous-titres | 174 ms | 6.26 ms |
| lecture de 4000 sous-titres | 2.05 ms | 90.6 µs |
| écriture de 4000 sous-titres | 519 µs | 25.5 µs |
| décalage de 4000 sous-titres | 6.99 µs | 3.41 µs |
| décalage puis annulation | 13.7 µs | 3.79 µs |
| transformation de 4000 sous-titres | 73.7 µs | 5.98 µs |
| conversion de fréquence sur 4000 sous-titres | 88.8 µs | 13.5 µs |
| alignement sur 4000 sous-titres | 131 µs | 5.41 µs |
| tri de 4000 sous-titres à l'envers | 267 µs | 18.6 µs |
| suppression d'un sous-titre sur deux | 164 µs | 26.3 µs |
| suppression puis annulation | 243 µs | 25.1 µs |
| insertion de 100 sous-titres vides au milieu | 53 µs | 14.6 µs |
| modification d'un texte, à travers une session | 227 ns | 82.9 ns |
| suppression des mentions sur 4000 sous-titres | 1.19 ms | 32.6 µs |
| mise en italique de 4000 sous-titres | 1.32 ms | 552 µs |
| remplacement d'un mot fréquent sur 4000 sous-titres, texte simple | 4.36 ms | 382 µs |
| remplacement d'un mot fréquent sur 4000 sous-titres, expression régulière | 4.55 ms | 451 µs |
| recherche sans résultat sur 4000 sous-titres | 2.99 ms | 90.2 µs |
| ajustement des durées de 4000 sous-titres, quatre contraintes | 1.3 ms | 93.8 µs |
| casse de titre sur 4000 sous-titres | 20.4 ms | 1.14 ms |
| collage de 4000 textes | 1.18 ms | 404 µs |
| alignement d'une traduction de 4000 lignes, par position | 837 µs | 93 µs |
| alignement d'une traduction de 4000 lignes, par numéro | 956 µs | 88 µs |
| déduction de fréquence sur 4000 sous-titres | 427 µs | 14.9 µs |
| recherche de conversion sur 4000 sous-titres | 365 µs | 9.54 µs |
| walk of 200 files in a tree of depth 3 | 653 µs | 57.5 µs |
| shift of 200 files of 20 subtitles | 35.7 ms | 1.06 ms |
| shift of one file of 4000 subtitles | 2.93 ms | 206 µs |

### 0.15.7 — 2026-10-09 — Release — load 1.48 — pace ×0.98

| Measure | Mean | Standard deviation |
| :----- | ------: | ---------: |
| la réplique en cours, sur 4000 sous-titres | 5.69 µs | 499 ns |
| traduire une réplique balisée pour l'image | 990 ns | 104 ns |
| composer une réplique de deux lignes | 126 ns | 3.45 ns |
| ouvrir une vidéo | 219 ms | 1.74 ms |
| chercher une position | 5.4 ms | 92.3 µs |
| rendre une image de 640×360 | 310 µs | 26.3 µs |
| rendre une image de 1280×720 | 942 µs | 35.8 µs |
| rendre une image de 1920×1080 | 2.06 ms | 120 µs |
| chercher l'image 0 (vidéo 720p) | 6.19 ms | 153 µs |
| chercher l'image 100 (vidéo 720p) | 47.8 ms | 2.2 ms |
| chercher l'image 249 (vidéo 720p) | 117 ms | 14.2 ms |
| pas avant vers l'image 100 (vidéo 720p) | 47.9 ms | 3.36 ms |
| pas avant vers l'image 249 (vidéo 720p) | 111 ms | 2.67 ms |
| pas arrière vers l'image 99 (vidéo 720p) | 47.3 ms | 3.22 ms |
| pas arrière vers l'image 248 (vidéo 720p) | 112 ms | 6.03 ms |
| construction du modèle sur 4000 sous-titres | 8.06 µs | 685 ns |
| une fenêtre de 40 lignes, cinq colonnes | 20.1 µs | 1.09 µs |
| rafraîchir après un décalage de 4000 sous-titres | 8.09 µs | 1.19 µs |
| réinitialisation du modèle après une ligne retirée | 8.54 µs | 1.86 µs |
| édition d'une cellule de texte | 395 ns | 102 ns |
| édition d'une cellule de position | 9.27 µs | 1.46 µs |
| centrer une ligne sur 4000 sous-titres | 1.11 µs | 481 ns |
| découpage de 4000 sous-titres, en ems, avec cache de longueurs | 185 ms | 9.33 ms |
| versionString | 39.2 ns | 0.482 ns |
| parse | 41.8 ns | 3.68 ns |
| format | 36.1 ns | 0.486 ns |
| position vers image | 6.64 ns | 0.0706 ns |
| image vers position | 8.76 ns | 3.58 ns |
| mise à l'échelle par un rationnel exact | 6.86 ns | 0.0543 ns |
| découpage de 4000 sous-titres, sans cache de longueurs | 114 ms | 6.09 ms |
| découpage de 4000 sous-titres, avec cache de longueurs | 117 ms | 6.71 ms |
| erreurs courantes du français sur 4000 sous-titres | 164 ms | 8.62 ms |
| erreurs courantes de l'anglais sur 4000 sous-titres | 184 ms | 9.75 ms |
| lecture de 4000 sous-titres | 2.05 ms | 125 µs |
| écriture de 4000 sous-titres | 598 µs | 32.6 µs |
| décalage de 4000 sous-titres | 7.96 µs | 2.87 µs |
| décalage puis annulation | 16.2 µs | 5.24 µs |
| transformation de 4000 sous-titres | 84.4 µs | 3.64 µs |
| conversion de fréquence sur 4000 sous-titres | 81.5 µs | 6.63 µs |
| alignement sur 4000 sous-titres | 152 µs | 10.6 µs |
| tri de 4000 sous-titres à l'envers | 270 µs | 19.6 µs |
| suppression d'un sous-titre sur deux | 163 µs | 26.8 µs |
| suppression puis annulation | 282 µs | 34.3 µs |
| insertion de 100 sous-titres vides au milieu | 60.5 µs | 13.6 µs |
| modification d'un texte, à travers une session | 199 ns | 21.5 ns |
| suppression des mentions sur 4000 sous-titres | 1.29 ms | 96.4 µs |
| mise en italique de 4000 sous-titres | 1.09 ms | 90.1 µs |
| remplacement d'un mot fréquent sur 4000 sous-titres, texte simple | 4.36 ms | 287 µs |
| remplacement d'un mot fréquent sur 4000 sous-titres, expression régulière | 5.26 ms | 719 µs |
| recherche sans résultat sur 4000 sous-titres | 2.82 ms | 325 µs |
| ajustement des durées de 4000 sous-titres, quatre contraintes | 1.86 ms | 503 µs |
| casse de titre sur 4000 sous-titres | 22.1 ms | 1.71 ms |
| collage de 4000 textes | 1.2 ms | 62.9 µs |
| alignement d'une traduction de 4000 lignes, par position | 1.02 ms | 98.8 µs |
| alignement d'une traduction de 4000 lignes, par numéro | 1.03 ms | 188 µs |
| déduction de fréquence sur 4000 sous-titres | 459 µs | 15.6 µs |
| recherche de conversion sur 4000 sous-titres | 448 µs | 12.6 µs |
| walk of 200 files in a tree of depth 3 | 705 µs | 62.8 µs |
| shift of 200 files of 20 subtitles | 36.7 ms | 1.32 ms |
| shift of one file of 4000 subtitles | 3.35 ms | 108 µs |

### 0.15.5 — 2026-10-09 — Release — load 1.42 — pace ×0.91

| Measure | Mean | Standard deviation |
| :----- | ------: | ---------: |
| la réplique en cours, sur 4000 sous-titres | 7.25 µs | 1.44 µs |
| traduire une réplique balisée pour l'image | 815 ns | 29 ns |
| composer une réplique de deux lignes | 136 ns | 1.74 ns |
| ouvrir une vidéo | 219 ms | 1.68 ms |
| chercher une position | 5.36 ms | 85.1 µs |
| rendre une image de 640×360 | 343 µs | 14.2 µs |
| rendre une image de 1280×720 | 825 µs | 58.8 µs |
| rendre une image de 1920×1080 | 2.1 ms | 181 µs |
| chercher l'image 0 (vidéo 720p) | 6.2 ms | 321 µs |
| chercher l'image 100 (vidéo 720p) | 45.1 ms | 2.65 ms |
| chercher l'image 249 (vidéo 720p) | 106 ms | 5.48 ms |
| pas avant vers l'image 100 (vidéo 720p) | 45.3 ms | 3.46 ms |
| pas avant vers l'image 249 (vidéo 720p) | 105 ms | 6.19 ms |
| pas arrière vers l'image 99 (vidéo 720p) | 45.4 ms | 2.7 ms |
| pas arrière vers l'image 248 (vidéo 720p) | 105 ms | 5.73 ms |
| construction du modèle sur 4000 sous-titres | 11 µs | 2.93 µs |
| une fenêtre de 40 lignes, cinq colonnes | 17.4 µs | 830 ns |
| rafraîchir après un décalage de 4000 sous-titres | 10.4 µs | 532 ns |
| réinitialisation du modèle après une ligne retirée | 10.4 µs | 1.36 µs |
| édition d'une cellule de texte | 407 ns | 58.4 ns |
| édition d'une cellule de position | 11.5 µs | 388 ns |
| centrer une ligne sur 4000 sous-titres | 1.06 µs | 189 ns |
| découpage de 4000 sous-titres, en ems, avec cache de longueurs | 180 ms | 5.69 ms |
| versionString | 39.2 ns | 0.348 ns |
| parse | 36.9 ns | 3.21 ns |
| format | 44.7 ns | 15.6 ns |
| position vers image | 6.64 ns | 0.0478 ns |
| image vers position | 6.64 ns | 0.0694 ns |
| mise à l'échelle par un rationnel exact | 6.86 ns | 0.0565 ns |
| découpage de 4000 sous-titres, sans cache de longueurs | 111 ms | 4.59 ms |
| découpage de 4000 sous-titres, avec cache de longueurs | 115 ms | 4.8 ms |
| erreurs courantes du français sur 4000 sous-titres | 159 ms | 12.7 ms |
| erreurs courantes de l'anglais sur 4000 sous-titres | 172 ms | 5.98 ms |
| lecture de 4000 sous-titres | 2.17 ms | 88.4 µs |
| écriture de 4000 sous-titres | 505 µs | 27.3 µs |
| décalage de 4000 sous-titres | 7.78 µs | 3.22 µs |
| décalage puis annulation | 14.9 µs | 3.39 µs |
| transformation de 4000 sous-titres | 74.4 µs | 5.52 µs |
| conversion de fréquence sur 4000 sous-titres | 78.7 µs | 8.97 µs |
| alignement sur 4000 sous-titres | 132 µs | 6.71 µs |
| tri de 4000 sous-titres à l'envers | 278 µs | 20.3 µs |
| suppression d'un sous-titre sur deux | 161 µs | 15.4 µs |
| suppression puis annulation | 245 µs | 25.6 µs |
| insertion de 100 sous-titres vides au milieu | 55.5 µs | 13.1 µs |
| modification d'un texte, à travers une session | 184 ns | 8.19 ns |
| suppression des mentions sur 4000 sous-titres | 1.22 ms | 46.8 µs |
| mise en italique de 4000 sous-titres | 922 µs | 36.8 µs |
| remplacement d'un mot fréquent sur 4000 sous-titres, texte simple | 3.93 ms | 174 µs |
| remplacement d'un mot fréquent sur 4000 sous-titres, expression régulière | 4.26 ms | 258 µs |
| recherche sans résultat sur 4000 sous-titres | 2.61 ms | 81.6 µs |
| ajustement des durées de 4000 sous-titres, quatre contraintes | 1.25 ms | 65.6 µs |
| casse de titre sur 4000 sous-titres | 17.9 ms | 697 µs |
| collage de 4000 textes | 994 µs | 48 µs |
| alignement d'une traduction de 4000 lignes, par position | 913 µs | 63.3 µs |
| alignement d'une traduction de 4000 lignes, par numéro | 816 µs | 31.1 µs |
| déduction de fréquence sur 4000 sous-titres | 369 µs | 12.9 µs |
| recherche de conversion sur 4000 sous-titres | 361 µs | 8.67 µs |
| walk of 200 files in a tree of depth 3 | 559 µs | 23.1 µs |
| shift of 200 files of 20 subtitles | 32.9 ms | 1.85 ms |
| shift of one file of 4000 subtitles | 2.98 ms | 262 µs |

### 0.15.0 — 2026-10-09 — Release — load 1.26 — pace ×0.99

| Measure | Mean | Standard deviation |
| :----- | ------: | ---------: |
| la réplique en cours, sur 4000 sous-titres | 8.75 µs | 3.43 µs |
| traduire une réplique balisée pour l'image | 959 ns | 133 ns |
| composer une réplique de deux lignes | 161 ns | 42 ns |
| ouvrir une vidéo | 223 ms | 3.97 ms |
| chercher une position | 5.4 ms | 115 µs |
| rendre une image de 640×360 | 378 µs | 54.2 µs |
| rendre une image de 1280×720 | 1.02 ms | 115 µs |
| rendre une image de 1920×1080 | 2.66 ms | 681 µs |
| chercher l'image 0 (vidéo 720p) | 6.87 ms | 791 µs |
| chercher l'image 100 (vidéo 720p) | 51.2 ms | 6.87 ms |
| chercher l'image 249 (vidéo 720p) | 122 ms | 17.7 ms |
| pas avant vers l'image 100 (vidéo 720p) | 51.9 ms | 11.7 ms |
| pas avant vers l'image 249 (vidéo 720p) | 115 ms | 6.06 ms |
| pas arrière vers l'image 99 (vidéo 720p) | 48.9 ms | 3.2 ms |
| pas arrière vers l'image 248 (vidéo 720p) | 114 ms | 6.37 ms |
| construction du modèle sur 4000 sous-titres | 8.35 µs | 2.04 µs |
| une fenêtre de 40 lignes, cinq colonnes | 20.1 µs | 2.31 µs |
| rafraîchir après un décalage de 4000 sous-titres | 8.37 µs | 3.94 µs |
| réinitialisation du modèle après une ligne retirée | 9.61 µs | 3.78 µs |
| édition d'une cellule de texte | 497 ns | 148 ns |
| édition d'une cellule de position | 9.35 µs | 1.73 µs |
| centrer une ligne sur 4000 sous-titres | 1.25 µs | 218 ns |
| découpage de 4000 sous-titres, en ems, avec cache de longueurs | 197 ms | 8.81 ms |
| versionString | 44.9 ns | 0.551 ns |
| parse | 42.9 ns | 3.37 ns |
| format | 37 ns | 0.818 ns |
| position vers image | 7.97 ns | 2.18 ns |
| image vers position | 6.8 ns | 1.6 ns |
| mise à l'échelle par un rationnel exact | 8.28 ns | 2.53 ns |
| découpage de 4000 sous-titres, sans cache de longueurs | 120 ms | 6.6 ms |
| découpage de 4000 sous-titres, avec cache de longueurs | 128 ms | 6.88 ms |
| erreurs courantes du français sur 4000 sous-titres | 173 ms | 8.14 ms |
| erreurs courantes de l'anglais sur 4000 sous-titres | 190 ms | 6.44 ms |
| lecture de 4000 sous-titres | 2.6 ms | 579 µs |
| écriture de 4000 sous-titres | 572 µs | 37.1 µs |
| décalage de 4000 sous-titres | 7.55 µs | 3.97 µs |
| décalage puis annulation | 15.1 µs | 3.2 µs |
| transformation de 4000 sous-titres | 82.3 µs | 6.14 µs |
| conversion de fréquence sur 4000 sous-titres | 75.5 µs | 8.03 µs |
| alignement sur 4000 sous-titres | 149 µs | 8.19 µs |
| tri de 4000 sous-titres à l'envers | 309 µs | 9.98 µs |
| suppression d'un sous-titre sur deux | 166 µs | 28.7 µs |
| suppression puis annulation | 285 µs | 37.9 µs |
| insertion de 100 sous-titres vides au milieu | 59.3 µs | 28.9 µs |
| modification d'un texte, à travers une session | 208 ns | 19.3 ns |
| suppression des mentions sur 4000 sous-titres | 1.27 ms | 87.1 µs |
| mise en italique de 4000 sous-titres | 959 µs | 80 µs |
| remplacement d'un mot fréquent sur 4000 sous-titres, texte simple | 4.04 ms | 211 µs |
| remplacement d'un mot fréquent sur 4000 sous-titres, expression régulière | 4.36 ms | 271 µs |
| recherche sans résultat sur 4000 sous-titres | 2.71 ms | 163 µs |
| ajustement des durées de 4000 sous-titres, quatre contraintes | 1.4 ms | 29 µs |
| casse de titre sur 4000 sous-titres | 19.4 ms | 962 µs |
| collage de 4000 textes | 1.04 ms | 56.1 µs |
| alignement d'une traduction de 4000 lignes, par position | 936 µs | 34.3 µs |
| alignement d'une traduction de 4000 lignes, par numéro | 861 µs | 72.8 µs |
| déduction de fréquence sur 4000 sous-titres | 415 µs | 17.3 µs |
| recherche de conversion sur 4000 sous-titres | 364 µs | 11.5 µs |
| walk of 200 files in a tree of depth 3 | 573 µs | 39.1 µs |
| shift of 200 files of 20 subtitles | 36.2 ms | 1.82 ms |
| shift of one file of 4000 subtitles | 3.08 ms | 281 µs |

### 0.14.19 — 2026-10-08 — Release — load 1.49 — pace ×1.05

| Measure | Mean | Standard deviation |
| :----- | ------: | ---------: |
| la réplique en cours, sur 4000 sous-titres | 6.78 µs | 1.73 µs |
| traduire une réplique balisée pour l'image | 1.17 µs | 426 ns |
| composer une réplique de deux lignes | 169 ns | 38.2 ns |
| ouvrir une vidéo | 222 ms | 26.9 ms |
| chercher une position | 5.64 ms | 519 µs |
| rendre une image de 640×360 | 373 µs | 17.3 µs |
| rendre une image de 1280×720 | 1.35 ms | 256 µs |
| rendre une image de 1920×1080 | 2.65 ms | 540 µs |
| chercher l'image 0 (vidéo 720p) | 7.81 ms | 679 µs |
| chercher l'image 100 (vidéo 720p) | 53.2 ms | 7.42 ms |
| chercher l'image 249 (vidéo 720p) | 125 ms | 10.6 ms |
| pas avant vers l'image 100 (vidéo 720p) | 52.4 ms | 6.25 ms |
| pas avant vers l'image 249 (vidéo 720p) | 119 ms | 15.9 ms |
| pas arrière vers l'image 99 (vidéo 720p) | 49.5 ms | 2.83 ms |
| pas arrière vers l'image 248 (vidéo 720p) | 114 ms | 6.74 ms |
| construction du modèle sur 4000 sous-titres | 9.57 µs | 2.82 µs |
| une fenêtre de 40 lignes, cinq colonnes | 20 µs | 1.19 µs |
| rafraîchir après un décalage de 4000 sous-titres | 9.49 µs | 2.32 µs |
| réinitialisation du modèle après une ligne retirée | 9.48 µs | 2.55 µs |
| édition d'une cellule de texte | 508 ns | 215 ns |
| édition d'une cellule de position | 10.7 µs | 1.28 µs |
| centrer une ligne sur 4000 sous-titres | 1.3 µs | 431 ns |
| découpage de 4000 sous-titres, en ems, avec cache de longueurs | 204 ms | 3.79 ms |
| versionString | 52.3 ns | 12.4 ns |
| parse | 43 ns | 8.93 ns |
| format | 42.7 ns | 8.54 ns |
| position vers image | 7.25 ns | 0.08 ns |
| image vers position | 7.8 ns | 2.4 ns |
| mise à l'échelle par un rationnel exact | 7.7 ns | 0.0829 ns |
| découpage de 4000 sous-titres, sans cache de longueurs | 126 ms | 3.22 ms |
| découpage de 4000 sous-titres, avec cache de longueurs | 129 ms | 3.43 ms |
| erreurs courantes du français sur 4000 sous-titres | 176 ms | 3.22 ms |
| erreurs courantes de l'anglais sur 4000 sous-titres | 196 ms | 4.82 ms |
| lecture de 4000 sous-titres | 2.44 ms | 108 µs |
| écriture de 4000 sous-titres | 616 µs | 51.8 µs |
| décalage de 4000 sous-titres | 8.3 µs | 2.38 µs |
| décalage puis annulation | 15.5 µs | 3.67 µs |
| transformation de 4000 sous-titres | 87.6 µs | 6.31 µs |
| conversion de fréquence sur 4000 sous-titres | 82 µs | 9.92 µs |
| alignement sur 4000 sous-titres | 150 µs | 8.59 µs |
| tri de 4000 sous-titres à l'envers | 336 µs | 49.1 µs |
| suppression d'un sous-titre sur deux | 289 µs | 79 µs |
| suppression puis annulation | 301 µs | 44.8 µs |
| insertion de 100 sous-titres vides au milieu | 70.7 µs | 19.6 µs |
| modification d'un texte, à travers une session | 221 ns | 23.8 ns |
| suppression des mentions sur 4000 sous-titres | 1.49 ms | 152 µs |
| mise en italique de 4000 sous-titres | 1.07 ms | 51 µs |
| remplacement d'un mot fréquent sur 4000 sous-titres, texte simple | 4.17 ms | 265 µs |
| remplacement d'un mot fréquent sur 4000 sous-titres, expression régulière | 4.44 ms | 377 µs |
| recherche sans résultat sur 4000 sous-titres | 3.09 ms | 218 µs |
| ajustement des durées de 4000 sous-titres, quatre contraintes | 1.42 ms | 55.6 µs |
| casse de titre sur 4000 sous-titres | 21.6 ms | 1.4 ms |
| collage de 4000 textes | 1.15 ms | 53.6 µs |
| alignement d'une traduction de 4000 lignes, par position | 936 µs | 40.9 µs |
| alignement d'une traduction de 4000 lignes, par numéro | 912 µs | 35.8 µs |
| déduction de fréquence sur 4000 sous-titres | 423 µs | 13.2 µs |
| recherche de conversion sur 4000 sous-titres | 414 µs | 9.24 µs |
| walk of 200 files in a tree of depth 3 | 581 µs | 37.2 µs |
| shift of 200 files of 20 subtitles | 35.9 ms | 995 µs |
| shift of one file of 4000 subtitles | 3.34 ms | 82.4 µs |

### 0.14.7 — 2026-10-07 — Release — load 1.46 — pace ×0.95

| Measure | Mean | Standard deviation |
| :----- | ------: | ---------: |
| la réplique en cours, sur 4000 sous-titres | 5.31 µs | 902 ns |
| composer une réplique de deux lignes | 185 ns | 32.4 ns |
| ouvrir une vidéo | 218 ms | 1.49 ms |
| chercher une position | 5.39 ms | 103 µs |
| rendre une image de 640×360 | 216 µs | 44.6 µs |
| rendre une image de 1280×720 | 724 µs | 88.3 µs |
| rendre une image de 1920×1080 | 1.41 ms | 187 µs |
| chercher l'image 0 (vidéo 720p) | 6.65 ms | 500 µs |
| chercher l'image 100 (vidéo 720p) | 38.6 ms | 1.82 ms |
| chercher l'image 249 (vidéo 720p) | 85.7 ms | 7.89 ms |
| pas avant vers l'image 100 (vidéo 720p) | 40 ms | 5.36 ms |
| pas avant vers l'image 249 (vidéo 720p) | 87.6 ms | 10.6 ms |
| pas arrière vers l'image 99 (vidéo 720p) | 40.7 ms | 5.32 ms |
| pas arrière vers l'image 248 (vidéo 720p) | 86.5 ms | 9.59 ms |
| construction du modèle sur 4000 sous-titres | 8.91 µs | 4.17 µs |
| une fenêtre de 40 lignes, cinq colonnes | 17.9 µs | 2.26 µs |
| rafraîchir après un décalage de 4000 sous-titres | 8.24 µs | 2.6 µs |
| réinitialisation du modèle après une ligne retirée | 8.27 µs | 2.91 µs |
| édition d'une cellule de texte | 454 ns | 199 ns |
| édition d'une cellule de position | 9.62 µs | 2.21 µs |
| découpage de 4000 sous-titres, en ems, avec cache de longueurs | 192 ms | 11.1 ms |
| versionString | 44.7 ns | 0.614 ns |
| parse | 36.6 ns | 2.61 ns |
| format | 42.9 ns | 6.71 ns |
| position vers image | 6.65 ns | 0.0731 ns |
| image vers position | 6.64 ns | 0.046 ns |
| mise à l'échelle par un rationnel exact | 6.86 ns | 0.0636 ns |
| découpage de 4000 sous-titres, sans cache de longueurs | 116 ms | 5.85 ms |
| découpage de 4000 sous-titres, avec cache de longueurs | 123 ms | 7.92 ms |
| erreurs courantes du français sur 4000 sous-titres | 164 ms | 10 ms |
| erreurs courantes de l'anglais sur 4000 sous-titres | 183 ms | 11.1 ms |
| lecture de 4000 sous-titres | 2.05 ms | 96 µs |
| écriture de 4000 sous-titres | 555 µs | 42.3 µs |
| décalage de 4000 sous-titres | 7.56 µs | 3.2 µs |
| décalage puis annulation | 15.4 µs | 4.75 µs |
| transformation de 4000 sous-titres | 72.6 µs | 11.3 µs |
| conversion de fréquence sur 4000 sous-titres | 67.2 µs | 9.06 µs |
| alignement sur 4000 sous-titres | 127 µs | 12.5 µs |
| tri de 4000 sous-titres à l'envers | 321 µs | 32.5 µs |
| suppression d'un sous-titre sur deux | 199 µs | 35.3 µs |
| suppression puis annulation | 242 µs | 24.6 µs |
| insertion de 100 sous-titres vides au milieu | 50.9 µs | 7.35 µs |
| modification d'un texte, à travers une session | 174 ns | 24.6 ns |
| suppression des mentions sur 4000 sous-titres | 1.32 ms | 143 µs |
| mise en italique de 4000 sous-titres | 947 µs | 130 µs |
| remplacement d'un mot fréquent sur 4000 sous-titres, texte simple | 4.65 ms | 1.04 ms |
| remplacement d'un mot fréquent sur 4000 sous-titres, expression régulière | 4.44 ms | 545 µs |
| recherche sans résultat sur 4000 sous-titres | 2.67 ms | 168 µs |
| ajustement des durées de 4000 sous-titres, quatre contraintes | 1.24 ms | 59.6 µs |
| casse de titre sur 4000 sous-titres | 20.1 ms | 2.02 ms |
| collage de 4000 textes | 1 ms | 55.5 µs |
| alignement d'une traduction de 4000 lignes, par position | 922 µs | 55.8 µs |
| alignement d'une traduction de 4000 lignes, par numéro | 1.02 ms | 193 µs |
| déduction de fréquence sur 4000 sous-titres | 380 µs | 13.9 µs |
| walk of 200 files in a tree of depth 3 | 639 µs | 43.6 µs |
| shift of 200 files of 20 subtitles | 35.6 ms | 3.08 ms |
| shift of one file of 4000 subtitles | 3.45 ms | 263 µs |

### 0.14.5 — 2026-10-06 — Release — load 1.49 — pace ×1.06

| Measure | Mean | Standard deviation |
| :----- | ------: | ---------: |
| la réplique en cours, sur 4000 sous-titres | 4.94 µs | 1.83 µs |
| composer une réplique de deux lignes | 180 ns | 3.42 ns |
| ouvrir une vidéo | 218 ms | 1.39 ms |
| chercher une position | 5.42 ms | 72.9 µs |
| rendre une image de 640×360 | 208 µs | 30.3 µs |
| rendre une image de 1280×720 | 738 µs | 131 µs |
| rendre une image de 1920×1080 | 1.5 ms | 339 µs |
| chercher l'image 0 (vidéo 720p) | 7.02 ms | 1.17 ms |
| chercher l'image 100 (vidéo 720p) | 39.1 ms | 2.79 ms |
| chercher l'image 249 (vidéo 720p) | 82.5 ms | 2.99 ms |
| pas avant vers l'image 100 (vidéo 720p) | 40.8 ms | 6.03 ms |
| pas avant vers l'image 249 (vidéo 720p) | 83.3 ms | 5.22 ms |
| pas arrière vers l'image 99 (vidéo 720p) | 39 ms | 4.47 ms |
| pas arrière vers l'image 248 (vidéo 720p) | 82.4 ms | 3.16 ms |
| construction du modèle sur 4000 sous-titres | 11.3 µs | 6.42 µs |
| une fenêtre de 40 lignes, cinq colonnes | 20.8 µs | 9.41 µs |
| rafraîchir après un décalage de 4000 sous-titres | 11 µs | 4.04 µs |
| réinitialisation du modèle après une ligne retirée | 11 µs | 3.88 µs |
| édition d'une cellule de texte | 412 ns | 145 ns |
| édition d'une cellule de position | 13.8 µs | 883 ns |
| découpage de 4000 sous-titres, en ems, avec cache de longueurs | 197 ms | 11.6 ms |
| versionString | 47.9 ns | 2.36 ns |
| parse | 38.7 ns | 0.889 ns |
| format | 38.5 ns | 13 ns |
| position vers image | 6.64 ns | 0.0523 ns |
| image vers position | 7.75 ns | 0.84 ns |
| mise à l'échelle par un rationnel exact | 7.91 ns | 0.388 ns |
| découpage de 4000 sous-titres, sans cache de longueurs | 124 ms | 5.21 ms |
| découpage de 4000 sous-titres, avec cache de longueurs | 126 ms | 2.77 ms |
| erreurs courantes du français sur 4000 sous-titres | 174 ms | 8.33 ms |
| erreurs courantes de l'anglais sur 4000 sous-titres | 198 ms | 14 ms |
| lecture de 4000 sous-titres | 2.37 ms | 275 µs |
| écriture de 4000 sous-titres | 653 µs | 41.7 µs |
| décalage de 4000 sous-titres | 13.3 µs | 13.5 µs |
| décalage puis annulation | 20.6 µs | 10.2 µs |
| transformation de 4000 sous-titres | 90.8 µs | 14.5 µs |
| conversion de fréquence sur 4000 sous-titres | 84.5 µs | 6.04 µs |
| alignement sur 4000 sous-titres | 156 µs | 11.3 µs |
| tri de 4000 sous-titres à l'envers | 336 µs | 31 µs |
| suppression d'un sous-titre sur deux | 409 µs | 127 µs |
| suppression puis annulation | 1.24 ms | 862 µs |
| insertion de 100 sous-titres vides au milieu | 189 µs | 116 µs |
| modification d'un texte, à travers une session | 202 ns | 23.7 ns |
| suppression des mentions sur 4000 sous-titres | 1.46 ms | 50.2 µs |
| mise en italique de 4000 sous-titres | 1.34 ms | 747 µs |
| remplacement d'un mot fréquent sur 4000 sous-titres, texte simple | 5.1 ms | 818 µs |
| remplacement d'un mot fréquent sur 4000 sous-titres, expression régulière | 5.13 ms | 321 µs |
| recherche sans résultat sur 4000 sous-titres | 4.11 ms | 520 µs |
| ajustement des durées de 4000 sous-titres, quatre contraintes | 1.49 ms | 145 µs |
| casse de titre sur 4000 sous-titres | 22.1 ms | 1.36 ms |
| collage de 4000 textes | 1.1 ms | 134 µs |
| alignement d'une traduction de 4000 lignes, par position | 887 µs | 62.7 µs |
| alignement d'une traduction de 4000 lignes, par numéro | 936 µs | 136 µs |
| déduction de fréquence sur 4000 sous-titres | 400 µs | 28.3 µs |
| walk of 200 files in a tree of depth 3 | 568 µs | 38.4 µs |
| shift of 200 files of 20 subtitles | 37.1 ms | 1.9 ms |
| shift of one file of 4000 subtitles | 3.7 ms | 614 µs |

### 0.14.3 — 2026-10-06 — Release — load 1.48 — pace ×1.10

| Measure | Mean | Standard deviation |
| :----- | ------: | ---------: |
| la réplique en cours, sur 4000 sous-titres | 7.94 µs | 2.74 µs |
| composer une réplique de deux lignes | 168 ns | 2.88 ns |
| ouvrir une vidéo | 10.1 ms | 721 µs |
| chercher une position | 644 µs | 103 µs |
| construction du modèle sur 4000 sous-titres | 9.99 µs | 6.36 µs |
| une fenêtre de 40 lignes, cinq colonnes | 21.4 µs | 1.96 µs |
| rafraîchir après un décalage de 4000 sous-titres | 13.7 µs | 6.33 µs |
| réinitialisation du modèle après une ligne retirée | 9.78 µs | 4.8 µs |
| édition d'une cellule de texte | 437 ns | 162 ns |
| édition d'une cellule de position | 10.6 µs | 1.56 µs |
| découpage de 4000 sous-titres, en ems, avec cache de longueurs | 191 ms | 10.7 ms |
| versionString | 39.1 ns | 0.695 ns |
| parse | 40.7 ns | 1.93 ns |
| format | 33.7 ns | 2.23 ns |
| position vers image | 6.48 ns | 0.0716 ns |
| image vers position | 7.25 ns | 0.131 ns |
| mise à l'échelle par un rationnel exact | 7.7 ns | 0.0825 ns |
| découpage de 4000 sous-titres, sans cache de longueurs | 120 ms | 7.79 ms |
| découpage de 4000 sous-titres, avec cache de longueurs | 124 ms | 7.16 ms |
| erreurs courantes du français sur 4000 sous-titres | 167 ms | 8.52 ms |
| erreurs courantes de l'anglais sur 4000 sous-titres | 187 ms | 9.93 ms |
| lecture de 4000 sous-titres | 2.24 ms | 149 µs |
| écriture de 4000 sous-titres | 652 µs | 65.7 µs |
| décalage de 4000 sous-titres | 10.2 µs | 3.56 µs |
| décalage puis annulation | 19.9 µs | 4.71 µs |
| transformation de 4000 sous-titres | 90.1 µs | 13.1 µs |
| conversion de fréquence sur 4000 sous-titres | 68.5 µs | 11.2 µs |
| alignement sur 4000 sous-titres | 153 µs | 21.1 µs |
| tri de 4000 sous-titres à l'envers | 310 µs | 47.6 µs |
| suppression d'un sous-titre sur deux | 680 µs | 764 µs |
| suppression puis annulation | 250 µs | 52 µs |
| insertion de 100 sous-titres vides au milieu | 59.9 µs | 7.53 µs |
| modification d'un texte, à travers une session | 176 ns | 18 ns |
| suppression des mentions sur 4000 sous-titres | 1.47 ms | 111 µs |
| mise en italique de 4000 sous-titres | 1 ms | 77.3 µs |
| remplacement d'un mot fréquent sur 4000 sous-titres, texte simple | 4.16 ms | 326 µs |
| remplacement d'un mot fréquent sur 4000 sous-titres, expression régulière | 4.43 ms | 489 µs |
| recherche sans résultat sur 4000 sous-titres | 3.18 ms | 421 µs |
| ajustement des durées de 4000 sous-titres, quatre contraintes | 1.49 ms | 261 µs |
| casse de titre sur 4000 sous-titres | 20.4 ms | 1.83 ms |
| collage de 4000 textes | 1.18 ms | 112 µs |
| alignement d'une traduction de 4000 lignes, par position | 925 µs | 45.6 µs |
| alignement d'une traduction de 4000 lignes, par numéro | 842 µs | 68.3 µs |
| déduction de fréquence sur 4000 sous-titres | 425 µs | 17.9 µs |
| walk of 200 files in a tree of depth 3 | 659 µs | 40.8 µs |
| shift of 200 files of 20 subtitles | 36.8 ms | 2.15 ms |
| shift of one file of 4000 subtitles | 3.27 ms | 213 µs |

### 0.13.33 — 2026-10-06 — Release — load 1.48 — pace ×0.95

| Measure | Mean | Standard deviation |
| :----- | ------: | ---------: |
| la réplique en cours, sur 4000 sous-titres | 5.93 µs | 317 ns |
| composer une réplique de deux lignes | 284 ns | 5.57 ns |
| ouvrir une vidéo | 8.82 ms | 190 µs |
| chercher une position | 587 µs | 183 µs |
| construction du modèle sur 4000 sous-titres | 8.36 µs | 1.83 µs |
| une fenêtre de 40 lignes, cinq colonnes | 19.5 µs | 6.89 µs |
| rafraîchir après un décalage de 4000 sous-titres | 8.64 µs | 2.89 µs |
| réinitialisation du modèle après une ligne retirée | 9.16 µs | 3.71 µs |
| édition d'une cellule de texte | 395 ns | 23.7 ns |
| édition d'une cellule de position | 10.6 µs | 2.13 µs |
| découpage de 4000 sous-titres, en ems, avec cache de longueurs | 184 ms | 4.66 ms |
| versionString | 44.6 ns | 0.597 ns |
| parse | 37.6 ns | 8.08 ns |
| format | 38.6 ns | 12.2 ns |
| position vers image | 6.32 ns | 0.0485 ns |
| image vers position | 6.47 ns | 0.069 ns |
| mise à l'échelle par un rationnel exact | 7.1 ns | 2.14 ns |
| découpage de 4000 sous-titres, sans cache de longueurs | 112 ms | 4.49 ms |
| découpage de 4000 sous-titres, avec cache de longueurs | 111 ms | 3.9 ms |
| erreurs courantes du français sur 4000 sous-titres | 155 ms | 4.84 ms |
| erreurs courantes de l'anglais sur 4000 sous-titres | 171 ms | 5.66 ms |
| lecture de 4000 sous-titres | 2.01 ms | 59.6 µs |
| écriture de 4000 sous-titres | 527 µs | 24.2 µs |
| décalage de 4000 sous-titres | 6.78 µs | 2.78 µs |
| décalage puis annulation | 13.5 µs | 4.03 µs |
| transformation de 4000 sous-titres | 73.3 µs | 7.75 µs |
| conversion de fréquence sur 4000 sous-titres | 73.6 µs | 16.3 µs |
| alignement sur 4000 sous-titres | 126 µs | 10.3 µs |
| tri de 4000 sous-titres à l'envers | 271 µs | 19.9 µs |
| suppression d'un sous-titre sur deux | 165 µs | 25 µs |
| suppression puis annulation | 245 µs | 30.4 µs |
| insertion de 100 sous-titres vides au milieu | 53.7 µs | 14.8 µs |
| modification d'un texte, à travers une session | 177 ns | 13.6 ns |
| suppression des mentions sur 4000 sous-titres | 1.25 ms | 28.8 µs |
| mise en italique de 4000 sous-titres | 905 µs | 32.1 µs |
| remplacement d'un mot fréquent sur 4000 sous-titres, texte simple | 4 ms | 235 µs |
| remplacement d'un mot fréquent sur 4000 sous-titres, expression régulière | 4.04 ms | 113 µs |
| recherche sans résultat sur 4000 sous-titres | 2.62 ms | 116 µs |
| ajustement des durées de 4000 sous-titres, quatre contraintes | 1.21 ms | 58.8 µs |
| casse de titre sur 4000 sous-titres | 18.3 ms | 685 µs |
| collage de 4000 textes | 1.14 ms | 133 µs |
| alignement d'une traduction de 4000 lignes, par position | 803 µs | 47.7 µs |
| alignement d'une traduction de 4000 lignes, par numéro | 828 µs | 66.1 µs |
| déduction de fréquence sur 4000 sous-titres | 371 µs | 24 µs |
| walk of 200 files in a tree of depth 3 | 574 µs | 42.6 µs |
| shift of 200 files of 20 subtitles | 33.2 ms | 1.84 ms |
| shift of one file of 4000 subtitles | 2.89 ms | 114 µs |

### 0.13.27 — 2026-10-05 — Release — load 1.39 — pace ×1.16

| Measure | Mean | Standard deviation |
| :----- | ------: | ---------: |
| la réplique en cours, sur 4000 sous-titres | 7.72 µs | 5.33 µs |
| composer une réplique de deux lignes | 213 ns | 4.01 ns |
| ouvrir une vidéo | 9.26 ms | 501 µs |
| chercher une position | 574 µs | 83.3 µs |
| construction du modèle sur 4000 sous-titres | 10.6 µs | 3.89 µs |
| une fenêtre de 40 lignes, cinq colonnes | 20.1 µs | 962 ns |
| rafraîchir après un décalage de 4000 sous-titres | 9.89 µs | 3.56 µs |
| réinitialisation du modèle après une ligne retirée | 10.4 µs | 7.39 µs |
| édition d'une cellule de texte | 518 ns | 284 ns |
| édition d'une cellule de position | 11.4 µs | 798 ns |
| découpage de 4000 sous-titres, en ems, avec cache de longueurs | 209 ms | 2.83 ms |
| versionString | 51.7 ns | 0.625 ns |
| parse | 43.6 ns | 2.97 ns |
| format | 40.3 ns | 0.724 ns |
| position vers image | 7.81 ns | 0.108 ns |
| image vers position | 7.63 ns | 0.171 ns |
| mise à l'échelle par un rationnel exact | 8.22 ns | 1.28 ns |
| découpage de 4000 sous-titres, sans cache de longueurs | 128 ms | 1.44 ms |
| découpage de 4000 sous-titres, avec cache de longueurs | 133 ms | 1.83 ms |
| erreurs courantes du français sur 4000 sous-titres | 179 ms | 2.45 ms |
| erreurs courantes de l'anglais sur 4000 sous-titres | 200 ms | 2.2 ms |
| lecture de 4000 sous-titres | 2.44 ms | 149 µs |
| écriture de 4000 sous-titres | 628 µs | 49.4 µs |
| décalage de 4000 sous-titres | 8.2 µs | 2.54 µs |
| décalage puis annulation | 15.9 µs | 5.06 µs |
| transformation de 4000 sous-titres | 87.3 µs | 16.8 µs |
| conversion de fréquence sur 4000 sous-titres | 81.2 µs | 14.1 µs |
| alignement sur 4000 sous-titres | 147 µs | 10.5 µs |
| tri de 4000 sous-titres à l'envers | 321 µs | 43.2 µs |
| suppression d'un sous-titre sur deux | 191 µs | 24.3 µs |
| suppression puis annulation | 294 µs | 43.5 µs |
| insertion de 100 sous-titres vides au milieu | 64.5 µs | 12.3 µs |
| modification d'un texte, à travers une session | 225 ns | 115 ns |
| suppression des mentions sur 4000 sous-titres | 1.43 ms | 182 µs |
| mise en italique de 4000 sous-titres | 1.07 ms | 132 µs |
| remplacement d'un mot fréquent sur 4000 sous-titres, texte simple | 4.7 ms | 243 µs |
| remplacement d'un mot fréquent sur 4000 sous-titres, expression régulière | 4.82 ms | 165 µs |
| recherche sans résultat sur 4000 sous-titres | 3.12 ms | 198 µs |
| ajustement des durées de 4000 sous-titres, quatre contraintes | 1.45 ms | 92.3 µs |
| casse de titre sur 4000 sous-titres | 21.1 ms | 437 µs |
| collage de 4000 textes | 1.21 ms | 173 µs |
| alignement d'une traduction de 4000 lignes, par position | 991 µs | 64.7 µs |
| alignement d'une traduction de 4000 lignes, par numéro | 928 µs | 71.9 µs |
| déduction de fréquence sur 4000 sous-titres | 428 µs | 26.9 µs |
| walk of 200 files in a tree of depth 3 | 675 µs | 98.3 µs |
| shift of 200 files of 20 subtitles | 37.9 ms | 359 µs |
| shift of one file of 4000 subtitles | 3.58 ms | 257 µs |

### 0.13.25 — 2026-10-05 — Release — load 1.39 — pace ×0.98

| Measure | Mean | Standard deviation |
| :----- | ------: | ---------: |
| la réplique en cours, sur 4000 sous-titres | 5.69 µs | 246 ns |
| composer une réplique de deux lignes | 177 ns | 2.8 ns |
| ouvrir une vidéo | 8.98 ms | 340 µs |
| chercher une position | 592 µs | 189 µs |
| construction du modèle sur 4000 sous-titres | 8.23 µs | 437 ns |
| une fenêtre de 40 lignes, cinq colonnes | 17.6 µs | 535 ns |
| rafraîchir après un décalage de 4000 sous-titres | 8.18 µs | 144 ns |
| réinitialisation du modèle après une ligne retirée | 8.23 µs | 756 ns |
| édition d'une cellule de texte | 404 ns | 60.7 ns |
| édition d'une cellule de position | 9.23 µs | 535 ns |
| découpage de 4000 sous-titres, en ems, avec cache de longueurs | 183 ms | 5.31 ms |
| versionString | 47.3 ns | 0.71 ns |
| parse | 35.9 ns | 2.97 ns |
| format | 35.3 ns | 0.616 ns |
| position vers image | 6.48 ns | 0.0673 ns |
| image vers position | 6.32 ns | 0.0481 ns |
| mise à l'échelle par un rationnel exact | 6.73 ns | 0.128 ns |
| découpage de 4000 sous-titres, sans cache de longueurs | 109 ms | 3.23 ms |
| découpage de 4000 sous-titres, avec cache de longueurs | 112 ms | 1.64 ms |
| erreurs courantes du français sur 4000 sous-titres | 157 ms | 3.6 ms |
| erreurs courantes de l'anglais sur 4000 sous-titres | 172 ms | 4.41 ms |
| lecture de 4000 sous-titres | 2.03 ms | 72.8 µs |
| écriture de 4000 sous-titres | 530 µs | 44 µs |
| décalage de 4000 sous-titres | 6.56 µs | 2.74 µs |
| décalage puis annulation | 12.7 µs | 2.87 µs |
| transformation de 4000 sous-titres | 71 µs | 5.6 µs |
| conversion de fréquence sur 4000 sous-titres | 68.3 µs | 11.9 µs |
| alignement sur 4000 sous-titres | 123 µs | 6.77 µs |
| tri de 4000 sous-titres à l'envers | 276 µs | 30.4 µs |
| suppression d'un sous-titre sur deux | 160 µs | 21 µs |
| suppression puis annulation | 244 µs | 37.2 µs |
| insertion de 100 sous-titres vides au milieu | 52.1 µs | 14.6 µs |
| modification d'un texte, à travers une session | 178 ns | 16.1 ns |
| suppression des mentions sur 4000 sous-titres | 1.21 ms | 50.2 µs |
| mise en italique de 4000 sous-titres | 906 µs | 43.2 µs |
| remplacement d'un mot fréquent sur 4000 sous-titres, texte simple | 3.92 ms | 128 µs |
| remplacement d'un mot fréquent sur 4000 sous-titres, expression régulière | 4.29 ms | 247 µs |
| recherche sans résultat sur 4000 sous-titres | 2.65 ms | 93 µs |
| ajustement des durées de 4000 sous-titres, quatre contraintes | 1.25 ms | 59.1 µs |
| casse de titre sur 4000 sous-titres | 17.8 ms | 235 µs |
| collage de 4000 textes | 978 µs | 41.8 µs |
| alignement d'une traduction de 4000 lignes, par position | 806 µs | 43.8 µs |
| alignement d'une traduction de 4000 lignes, par numéro | 796 µs | 50.8 µs |
| déduction de fréquence sur 4000 sous-titres | 440 µs | 10.6 µs |
| walk of 200 files in a tree of depth 3 | 550 µs | 12.2 µs |
| shift of 200 files of 20 subtitles | 32.3 ms | 1.8 ms |
| shift of one file of 4000 subtitles | 3.12 ms | 231 µs |

### 0.13.21 — 2026-10-05 — Release — load 1.39 — pace ×1.02

| Measure | Mean | Standard deviation |
| :----- | ------: | ---------: |
| la réplique en cours, sur 4000 sous-titres | 7.36 µs | 2.1 µs |
| composer une réplique de deux lignes | 169 ns | 2.88 ns |
| ouvrir une vidéo | 9.21 ms | 400 µs |
| chercher une position | 589 µs | 187 µs |
| construction du modèle sur 4000 sous-titres | 9.89 µs | 4.36 µs |
| une fenêtre de 40 lignes, cinq colonnes | 17.8 µs | 3.22 µs |
| rafraîchir après un décalage de 4000 sous-titres | 8.37 µs | 2.31 µs |
| réinitialisation du modèle après une ligne retirée | 8.49 µs | 2.53 µs |
| édition d'une cellule de texte | 393 ns | 49.1 ns |
| édition d'une cellule de position | 9.2 µs | 651 ns |
| découpage de 4000 sous-titres, en ems, avec cache de longueurs | 189 ms | 10.2 ms |
| versionString | 53.2 ns | 0.851 ns |
| parse | 39.2 ns | 10.1 ns |
| format | 39.8 ns | 0.816 ns |
| position vers image | 8.21 ns | 0.788 ns |
| image vers position | 6.65 ns | 0.0761 ns |
| mise à l'échelle par un rationnel exact | 7.04 ns | 1.22 ns |
| découpage de 4000 sous-titres, sans cache de longueurs | 121 ms | 10.6 ms |
| découpage de 4000 sous-titres, avec cache de longueurs | 134 ms | 8.93 ms |
| erreurs courantes du français sur 4000 sous-titres | 169 ms | 10.5 ms |
| erreurs courantes de l'anglais sur 4000 sous-titres | 180 ms | 9.61 ms |
| lecture de 4000 sous-titres | 2.02 ms | 170 µs |
| écriture de 4000 sous-titres | 521 µs | 33.8 µs |
| décalage de 4000 sous-titres | 7.77 µs | 4.21 µs |
| décalage puis annulation | 14.5 µs | 1.61 µs |
| transformation de 4000 sous-titres | 87.9 µs | 8.03 µs |
| conversion de fréquence sur 4000 sous-titres | 77.7 µs | 11.2 µs |
| alignement sur 4000 sous-titres | 127 µs | 18.6 µs |
| tri de 4000 sous-titres à l'envers | 275 µs | 27.9 µs |
| suppression d'un sous-titre sur deux | 184 µs | 16.6 µs |
| suppression puis annulation | 283 µs | 25.1 µs |
| insertion de 100 sous-titres vides au milieu | 53 µs | 16.9 µs |
| modification d'un texte, à travers une session | 180 ns | 21.5 ns |
| suppression des mentions sur 4000 sous-titres | 1.36 ms | 103 µs |
| mise en italique de 4000 sous-titres | 987 µs | 95.9 µs |
| remplacement d'un mot fréquent sur 4000 sous-titres, texte simple | 4.47 ms | 175 µs |
| remplacement d'un mot fréquent sur 4000 sous-titres, expression régulière | 4.47 ms | 277 µs |
| recherche sans résultat sur 4000 sous-titres | 2.72 ms | 197 µs |
| ajustement des durées de 4000 sous-titres, quatre contraintes | 1.35 ms | 189 µs |
| casse de titre sur 4000 sous-titres | 19.9 ms | 1.2 ms |
| collage de 4000 textes | 991 µs | 68.2 µs |
| alignement d'une traduction de 4000 lignes, par position | 850 µs | 70.9 µs |
| alignement d'une traduction de 4000 lignes, par numéro | 896 µs | 43.8 µs |
| déduction de fréquence sur 4000 sous-titres | 368 µs | 15.1 µs |
| walk of 200 files in a tree of depth 3 | 567 µs | 41.7 µs |
| shift of 200 files of 20 subtitles | 36.3 ms | 1.89 ms |
| shift of one file of 4000 subtitles | 3.02 ms | 300 µs |

### 0.13.14 — 2026-10-02 — Release — load 1.41 — pace ×0.95

| Measure | Mean | Standard deviation |
| :----- | ------: | ---------: |
| la réplique en cours, sur 4000 sous-titres | 5.49 µs | 1.36 µs |
| composer une réplique de deux lignes | 167 ns | 3.36 ns |
| ouvrir une vidéo | 9.16 ms | 551 µs |
| chercher une position | 588 µs | 183 µs |
| construction du modèle sur 4000 sous-titres | 9.39 µs | 6.64 µs |
| une fenêtre de 40 lignes, cinq colonnes | 18.4 µs | 4.5 µs |
| rafraîchir après un décalage de 4000 sous-titres | 8.57 µs | 3.55 µs |
| réinitialisation du modèle après une ligne retirée | 8.73 µs | 3.93 µs |
| édition d'une cellule de texte | 434 ns | 177 ns |
| édition d'une cellule de position | 9.84 µs | 3.39 µs |
| découpage de 4000 sous-titres, en ems, avec cache de longueurs | 186 ms | 6.39 ms |
| versionString | 46.2 ns | 9.4 ns |
| parse | 42.4 ns | 20.4 ns |
| format | 35.3 ns | 0.485 ns |
| position vers image | 6.64 ns | 0.0738 ns |
| image vers position | 6.64 ns | 0.0527 ns |
| mise à l'échelle par un rationnel exact | 7.86 ns | 0.0699 ns |
| découpage de 4000 sous-titres, sans cache de longueurs | 115 ms | 4.95 ms |
| découpage de 4000 sous-titres, avec cache de longueurs | 113 ms | 3.61 ms |
| erreurs courantes du français sur 4000 sous-titres | 159 ms | 6.03 ms |
| erreurs courantes de l'anglais sur 4000 sous-titres | 175 ms | 5.3 ms |
| lecture de 4000 sous-titres | 2.19 ms | 164 µs |
| écriture de 4000 sous-titres | 526 µs | 36 µs |
| décalage de 4000 sous-titres | 7.82 µs | 1.4 µs |
| décalage puis annulation | 13.7 µs | 2.67 µs |
| transformation de 4000 sous-titres | 71.2 µs | 12 µs |
| conversion de fréquence sur 4000 sous-titres | 71.6 µs | 8.09 µs |
| alignement sur 4000 sous-titres | 127 µs | 15.3 µs |
| tri de 4000 sous-titres à l'envers | 285 µs | 35.8 µs |
| suppression d'un sous-titre sur deux | 173 µs | 34.1 µs |
| suppression puis annulation | 246 µs | 33.1 µs |
| insertion de 100 sous-titres vides au milieu | 53 µs | 13.3 µs |
| modification d'un texte, à travers une session | 195 ns | 40.9 ns |
| suppression des mentions sur 4000 sous-titres | 1.28 ms | 62.8 µs |
| mise en italique de 4000 sous-titres | 930 µs | 128 µs |
| remplacement d'un mot fréquent sur 4000 sous-titres, texte simple | 4.14 ms | 296 µs |
| remplacement d'un mot fréquent sur 4000 sous-titres, expression régulière | 4.3 ms | 293 µs |
| recherche sans résultat sur 4000 sous-titres | 2.63 ms | 125 µs |
| ajustement des durées de 4000 sous-titres, quatre contraintes | 1.23 ms | 39.3 µs |
| casse de titre sur 4000 sous-titres | 18.8 ms | 856 µs |
| collage de 4000 textes | 991 µs | 57.1 µs |
| alignement d'une traduction de 4000 lignes, par position | 875 µs | 71.8 µs |
| alignement d'une traduction de 4000 lignes, par numéro | 815 µs | 59.7 µs |
| déduction de fréquence sur 4000 sous-titres | 374 µs | 25.9 µs |
| walk of 200 files in a tree of depth 3 | 571 µs | 35.9 µs |
| shift of 200 files of 20 subtitles | 34.6 ms | 1.57 ms |
| shift of one file of 4000 subtitles | 2.94 ms | 195 µs |

### 0.13.13 — 2026-10-02 — Release — load 1.39 — pace ×0.92

| Measure | Mean | Standard deviation |
| :----- | ------: | ---------: |
| la réplique en cours, sur 4000 sous-titres | 7.3 µs | 1.73 µs |
| composer une réplique de deux lignes | 205 ns | 75.5 ns |
| ouvrir une vidéo | 9.24 ms | 461 µs |
| chercher une position | 588 µs | 190 µs |
| construction du modèle sur 4000 sous-titres | 7.32 µs | 2.7 µs |
| une fenêtre de 40 lignes, cinq colonnes | 18.5 µs | 6.39 µs |
| rafraîchir après un décalage de 4000 sous-titres | 7.32 µs | 2.75 µs |
| réinitialisation du modèle après une ligne retirée | 7.33 µs | 2.97 µs |
| édition d'une cellule de texte | 393 ns | 58.3 ns |
| édition d'une cellule de position | 8.21 µs | 930 ns |
| découpage de 4000 sous-titres, en ems, avec cache de longueurs | 179 ms | 5.69 ms |
| versionString | 44.4 ns | 0.599 ns |
| parse | 41.4 ns | 3.07 ns |
| format | 34.2 ns | 0.447 ns |
| position vers image | 6.32 ns | 0.0681 ns |
| image vers position | 6.47 ns | 0.073 ns |
| mise à l'échelle par un rationnel exact | 6.71 ns | 0.0507 ns |
| découpage de 4000 sous-titres, sans cache de longueurs | 113 ms | 4.29 ms |
| découpage de 4000 sous-titres, avec cache de longueurs | 116 ms | 4.24 ms |
| erreurs courantes du français sur 4000 sous-titres | 158 ms | 5.18 ms |
| erreurs courantes de l'anglais sur 4000 sous-titres | 176 ms | 5.21 ms |
| lecture de 4000 sous-titres | 2.15 ms | 185 µs |
| écriture de 4000 sous-titres | 545 µs | 70.7 µs |
| décalage de 4000 sous-titres | 6.63 µs | 2.57 µs |
| décalage puis annulation | 12.7 µs | 2.95 µs |
| transformation de 4000 sous-titres | 67.5 µs | 5.73 µs |
| conversion de fréquence sur 4000 sous-titres | 66.5 µs | 7.2 µs |
| alignement sur 4000 sous-titres | 124 µs | 7.61 µs |
| tri de 4000 sous-titres à l'envers | 273 µs | 26.5 µs |
| suppression d'un sous-titre sur deux | 163 µs | 22.7 µs |
| suppression puis annulation | 243 µs | 30.1 µs |
| insertion de 100 sous-titres vides au milieu | 52.8 µs | 14.7 µs |
| modification d'un texte, à travers une session | 173 ns | 18.5 ns |
| suppression des mentions sur 4000 sous-titres | 1.28 ms | 60.4 µs |
| mise en italique de 4000 sous-titres | 952 µs | 93.8 µs |
| remplacement d'un mot fréquent sur 4000 sous-titres, texte simple | 4.08 ms | 267 µs |
| remplacement d'un mot fréquent sur 4000 sous-titres, expression régulière | 4.24 ms | 323 µs |
| recherche sans résultat sur 4000 sous-titres | 2.67 ms | 133 µs |
| ajustement des durées de 4000 sous-titres, quatre contraintes | 1.25 ms | 54.9 µs |
| casse de titre sur 4000 sous-titres | 18.6 ms | 938 µs |
| collage de 4000 textes | 1.09 ms | 125 µs |
| alignement d'une traduction de 4000 lignes, par position | 819 µs | 55.7 µs |
| alignement d'une traduction de 4000 lignes, par numéro | 809 µs | 60.9 µs |
| déduction de fréquence sur 4000 sous-titres | 377 µs | 29.6 µs |
| walk of 200 files in a tree of depth 3 | 593 µs | 45.3 µs |
| shift of 200 files of 20 subtitles | 34.2 ms | 1.66 ms |
| shift of one file of 4000 subtitles | 3.24 ms | 387 µs |

### 0.13.10 — 2026-10-01 — Release — load 1.46 — pace ×1.06

| Measure | Mean | Standard deviation |
| :----- | ------: | ---------: |
| la réplique en cours, sur 4000 sous-titres | 4.91 µs | 1.94 µs |
| composer une réplique de deux lignes | 166 ns | 3.61 ns |
| ouvrir une vidéo | 9.11 ms | 471 µs |
| chercher une position | 596 µs | 166 µs |
| construction du modèle sur 4000 sous-titres | 8.45 µs | 2.92 µs |
| une fenêtre de 40 lignes, cinq colonnes | 21.3 µs | 5.45 µs |
| rafraîchir après un décalage de 4000 sous-titres | 8.39 µs | 2.21 µs |
| réinitialisation du modèle après une ligne retirée | 8.39 µs | 2.46 µs |
| édition d'une cellule de texte | 461 ns | 57 ns |
| édition d'une cellule de position | 9.34 µs | 676 ns |
| découpage de 4000 sous-titres, en ems, avec cache de longueurs | 196 ms | 6.56 ms |
| versionString | 44.3 ns | 0.874 ns |
| parse | 41.7 ns | 3.28 ns |
| format | 41.2 ns | 1.28 ns |
| position vers image | 7.62 ns | 0.0821 ns |
| image vers position | 7.62 ns | 0.0795 ns |
| mise à l'échelle par un rationnel exact | 7.88 ns | 0.115 ns |
| découpage de 4000 sous-titres, sans cache de longueurs | 120 ms | 6.3 ms |
| découpage de 4000 sous-titres, avec cache de longueurs | 123 ms | 8.3 ms |
| erreurs courantes du français sur 4000 sous-titres | 167 ms | 7.1 ms |
| erreurs courantes de l'anglais sur 4000 sous-titres | 194 ms | 16.3 ms |
| lecture de 4000 sous-titres | 2.52 ms | 188 µs |
| écriture de 4000 sous-titres | 630 µs | 44.8 µs |
| décalage de 4000 sous-titres | 9.61 µs | 3.89 µs |
| décalage puis annulation | 17.9 µs | 2.31 µs |
| transformation de 4000 sous-titres | 85.3 µs | 7.25 µs |
| conversion de fréquence sur 4000 sous-titres | 79.9 µs | 5.58 µs |
| alignement sur 4000 sous-titres | 152 µs | 8.42 µs |
| tri de 4000 sous-titres à l'envers | 339 µs | 52.4 µs |
| suppression d'un sous-titre sur deux | 528 µs | 185 µs |
| suppression puis annulation | 1.9 ms | 1.95 ms |
| insertion de 100 sous-titres vides au milieu | 420 µs | 307 µs |
| modification d'un texte, à travers une session | 581 ns | 142 ns |
| suppression des mentions sur 4000 sous-titres | 3.18 ms | 303 µs |
| mise en italique de 4000 sous-titres | 3.36 ms | 1.02 ms |
| remplacement d'un mot fréquent sur 4000 sous-titres, texte simple | 14 ms | 9.31 ms |
| remplacement d'un mot fréquent sur 4000 sous-titres, expression régulière | 11.9 ms | 1.36 ms |
| recherche sans résultat sur 4000 sous-titres | 8.01 ms | 461 µs |
| ajustement des durées de 4000 sous-titres, quatre contraintes | 4.03 ms | 493 µs |
| casse de titre sur 4000 sous-titres | 55.6 ms | 4.42 ms |
| collage de 4000 textes | 1.12 ms | 70 µs |
| alignement d'une traduction de 4000 lignes, par position | 954 µs | 172 µs |
| alignement d'une traduction de 4000 lignes, par numéro | 948 µs | 173 µs |
| déduction de fréquence sur 4000 sous-titres | 449 µs | 69.8 µs |
| walk of 200 files in a tree of depth 3 | 652 µs | 78.6 µs |
| shift of 200 files of 20 subtitles | 38.5 ms | 2.83 ms |
| shift of one file of 4000 subtitles | 3.6 ms | 333 µs |

### 0.13.0 — 2026-10-01 — Release — load 0.85 — pace ×1.02

| Measure | Mean | Standard deviation |
| :----- | ------: | ---------: |
| la réplique en cours, sur 4000 sous-titres | 8.76 µs | 1.9 µs |
| composer une réplique de deux lignes | 162 ns | 3.92 ns |
| ouvrir une vidéo | 9.19 ms | 690 µs |
| chercher une position | 624 µs | 155 µs |
| construction du modèle sur 4000 sous-titres | 10.2 µs | 4.78 µs |
| une fenêtre de 40 lignes, cinq colonnes | 20.2 µs | 4.54 µs |
| rafraîchir après un décalage de 4000 sous-titres | 9.77 µs | 1.89 µs |
| réinitialisation du modèle après une ligne retirée | 9.15 µs | 7.72 µs |
| édition d'une cellule de texte | 416 ns | 139 ns |
| édition d'une cellule de position | 11.5 µs | 1.91 µs |
| découpage de 4000 sous-titres, en ems, avec cache de longueurs | 202 ms | 14.4 ms |
| versionString | 44.7 ns | 0.674 ns |
| parse | 36.9 ns | 3.21 ns |
| format | 37.5 ns | 0.4 ns |
| position vers image | 7.31 ns | 0.102 ns |
| image vers position | 7.86 ns | 2.14 ns |
| mise à l'échelle par un rationnel exact | 7.43 ns | 2.49 ns |
| découpage de 4000 sous-titres, sans cache de longueurs | 123 ms | 12.4 ms |
| découpage de 4000 sous-titres, avec cache de longueurs | 125 ms | 7.31 ms |
| erreurs courantes du français sur 4000 sous-titres | 169 ms | 11.7 ms |
| erreurs courantes de l'anglais sur 4000 sous-titres | 188 ms | 15.8 ms |
| lecture de 4000 sous-titres | 2.38 ms | 223 µs |
| écriture de 4000 sous-titres | 590 µs | 65.6 µs |
| décalage de 4000 sous-titres | 7.47 µs | 2.86 µs |
| décalage puis annulation | 12.8 µs | 3.43 µs |
| transformation de 4000 sous-titres | 72.2 µs | 7.72 µs |
| conversion de fréquence sur 4000 sous-titres | 69.2 µs | 11.3 µs |
| alignement sur 4000 sous-titres | 126 µs | 13.9 µs |
| tri de 4000 sous-titres à l'envers | 310 µs | 23.5 µs |
| suppression d'un sous-titre sur deux | 172 µs | 41.8 µs |
| suppression puis annulation | 272 µs | 53 µs |
| insertion de 100 sous-titres vides au milieu | 52.4 µs | 15.7 µs |
| modification d'un texte, à travers une session | 175 ns | 16.6 ns |
| suppression des mentions sur 4000 sous-titres | 1.39 ms | 72.6 µs |
| mise en italique de 4000 sous-titres | 1.03 ms | 90.4 µs |
| remplacement d'un mot fréquent sur 4000 sous-titres, texte simple | 5.21 ms | 992 µs |
| remplacement d'un mot fréquent sur 4000 sous-titres, expression régulière | 4.71 ms | 509 µs |
| recherche sans résultat sur 4000 sous-titres | 3.03 ms | 318 µs |
| ajustement des durées de 4000 sous-titres, quatre contraintes | 1.25 ms | 56.1 µs |
| casse de titre sur 4000 sous-titres | 20.1 ms | 1.09 ms |
| collage de 4000 textes | 1.1 ms | 27.4 µs |
| alignement d'une traduction de 4000 lignes, par position | 811 µs | 44.9 µs |
| alignement d'une traduction de 4000 lignes, par numéro | 802 µs | 50.1 µs |
| déduction de fréquence sur 4000 sous-titres | 429 µs | 21 µs |

### 0.12.23 — 2026-09-30 — Release — load 1.46 — pace ×0.97

| Measure | Mean | Standard deviation |
| :----- | ------: | ---------: |
| la réplique en cours, sur 4000 sous-titres | 7.23 µs | 174 ns |
| composer une réplique de deux lignes | 162 ns | 4.02 ns |
| ouvrir une vidéo | 9.29 ms | 643 µs |
| chercher une position | 575 µs | 168 µs |
| construction du modèle sur 4000 sous-titres | 9.8 µs | 4.99 µs |
| une fenêtre de 40 lignes, cinq colonnes | 21.5 µs | 5.87 µs |
| rafraîchir après un décalage de 4000 sous-titres | 8.57 µs | 3.75 µs |
| réinitialisation du modèle après une ligne retirée | 9.54 µs | 1.48 µs |
| édition d'une cellule de texte | 426 ns | 166 ns |
| édition d'une cellule de position | 10.3 µs | 3.94 µs |
| découpage de 4000 sous-titres, en ems, avec cache de longueurs | 188 ms | 6.09 ms |
| versionString | 43.9 ns | 0.557 ns |
| parse | 36.7 ns | 2.58 ns |
| format | 38.1 ns | 0.751 ns |
| position vers image | 6.32 ns | 0.0477 ns |
| image vers position | 6.47 ns | 0.0669 ns |
| mise à l'échelle par un rationnel exact | 6.72 ns | 0.0752 ns |
| découpage de 4000 sous-titres, sans cache de longueurs | 114 ms | 3.35 ms |
| découpage de 4000 sous-titres, avec cache de longueurs | 117 ms | 4.03 ms |
| erreurs courantes du français sur 4000 sous-titres | 162 ms | 3.95 ms |
| erreurs courantes de l'anglais sur 4000 sous-titres | 178 ms | 7.37 ms |
| lecture de 4000 sous-titres | 2.11 ms | 71.3 µs |
| écriture de 4000 sous-titres | 518 µs | 13.6 µs |
| décalage de 4000 sous-titres | 7.04 µs | 3.63 µs |
| décalage puis annulation | 13.6 µs | 3.81 µs |
| transformation de 4000 sous-titres | 73.7 µs | 5.69 µs |
| conversion de fréquence sur 4000 sous-titres | 77.3 µs | 11 µs |
| alignement sur 4000 sous-titres | 130 µs | 15.6 µs |
| tri de 4000 sous-titres à l'envers | 286 µs | 34.7 µs |
| suppression d'un sous-titre sur deux | 166 µs | 16.1 µs |
| suppression puis annulation | 249 µs | 25 µs |
| insertion de 100 sous-titres vides au milieu | 55.1 µs | 19.7 µs |
| modification d'un texte, à travers une session | 178 ns | 11.8 ns |
| suppression des mentions sur 4000 sous-titres | 1.24 ms | 44 µs |
| mise en italique de 4000 sous-titres | 945 µs | 44.4 µs |
| remplacement d'un mot fréquent sur 4000 sous-titres, texte simple | 4.08 ms | 80.7 µs |
| remplacement d'un mot fréquent sur 4000 sous-titres, expression régulière | 4.26 ms | 91.9 µs |
| recherche sans résultat sur 4000 sous-titres | 2.9 ms | 407 µs |
| ajustement des durées de 4000 sous-titres, quatre contraintes | 1.28 ms | 44.2 µs |
| casse de titre sur 4000 sous-titres | 18.9 ms | 711 µs |
| collage de 4000 textes | 1.01 ms | 56.7 µs |
| alignement d'une traduction de 4000 lignes, par position | 836 µs | 36.4 µs |
| alignement d'une traduction de 4000 lignes, par numéro | 820 µs | 31.2 µs |
| déduction de fréquence sur 4000 sous-titres | 373 µs | 21.9 µs |

### 0.12.19 — 2026-09-30 — Release — load 1.44 — pace ×1.20

| Measure | Mean | Standard deviation |
| :----- | ------: | ---------: |
| la réplique en cours, sur 4000 sous-titres | 7.01 µs | 1.19 µs |
| composer une réplique de deux lignes | 203 ns | 4.38 ns |
| ouvrir une vidéo | 9.47 ms | 578 µs |
| chercher une position | 574 µs | 115 µs |
| construction du modèle sur 4000 sous-titres | 8.71 µs | 1.58 µs |
| une fenêtre de 40 lignes, cinq colonnes | 20.9 µs | 1.06 µs |
| rafraîchir après un décalage de 4000 sous-titres | 8.85 µs | 3.69 µs |
| réinitialisation du modèle après une ligne retirée | 8.74 µs | 2.3 µs |
| édition d'une cellule de texte | 496 ns | 49.9 ns |
| édition d'une cellule de position | 10.5 µs | 2.39 µs |
| découpage de 4000 sous-titres, en ems, avec cache de longueurs | 212 ms | 2.83 ms |
| versionString | 55 ns | 4.58 ns |
| parse | 43.8 ns | 2.76 ns |
| format | 44.3 ns | 0.98 ns |
| position vers image | 8.15 ns | 0.623 ns |
| image vers position | 8.03 ns | 0.221 ns |
| mise à l'échelle par un rationnel exact | 8.12 ns | 1.61 ns |
| découpage de 4000 sous-titres, sans cache de longueurs | 132 ms | 4.42 ms |
| découpage de 4000 sous-titres, avec cache de longueurs | 138 ms | 6.54 ms |
| erreurs courantes du français sur 4000 sous-titres | 187 ms | 4.35 ms |
| erreurs courantes de l'anglais sur 4000 sous-titres | 207 ms | 6.74 ms |
| lecture de 4000 sous-titres | 2.47 ms | 243 µs |
| écriture de 4000 sous-titres | 708 µs | 241 µs |
| décalage de 4000 sous-titres | 9.47 µs | 2.34 µs |
| décalage puis annulation | 18.2 µs | 4.32 µs |
| transformation de 4000 sous-titres | 91 µs | 15.7 µs |
| conversion de fréquence sur 4000 sous-titres | 85.3 µs | 8.91 µs |
| alignement sur 4000 sous-titres | 159 µs | 23.8 µs |
| tri de 4000 sous-titres à l'envers | 402 µs | 116 µs |
| suppression d'un sous-titre sur deux | 201 µs | 24.3 µs |
| suppression puis annulation | 291 µs | 34.2 µs |
| insertion de 100 sous-titres vides au milieu | 68.8 µs | 19.1 µs |
| modification d'un texte, à travers une session | 332 ns | 185 ns |
| suppression des mentions sur 4000 sous-titres | 1.46 ms | 88 µs |
| mise en italique de 4000 sous-titres | 1.12 ms | 150 µs |
| remplacement d'un mot fréquent sur 4000 sous-titres, texte simple | 4.78 ms | 220 µs |
| remplacement d'un mot fréquent sur 4000 sous-titres, expression régulière | 5.55 ms | 1.32 ms |
| recherche sans résultat sur 4000 sous-titres | 3.71 ms | 697 µs |
| ajustement des durées de 4000 sous-titres, quatre contraintes | 1.51 ms | 149 µs |
| casse de titre sur 4000 sous-titres | 21.8 ms | 539 µs |
| collage de 4000 textes | 1.29 ms | 201 µs |
| alignement d'une traduction de 4000 lignes, par position | 1.04 ms | 137 µs |
| alignement d'une traduction de 4000 lignes, par numéro | 995 µs | 72.1 µs |
| déduction de fréquence sur 4000 sous-titres | 451 µs | 32.1 µs |

### 0.12.12 — 2026-09-29 — Release — load 1.39 — pace ×1.05

| Measure | Mean | Standard deviation |
| :----- | ------: | ---------: |
| la réplique en cours, sur 4000 sous-titres | 6.88 µs | 1.77 µs |
| composer une réplique de deux lignes | 179 ns | 2.61 ns |
| ouvrir une vidéo | 9.31 ms | 626 µs |
| chercher une position | 610 µs | 184 µs |
| construction du modèle sur 4000 sous-titres | 8.44 µs | 2.81 µs |
| une fenêtre de 40 lignes, cinq colonnes | 18.1 µs | 3.22 µs |
| rafraîchir après un décalage de 4000 sous-titres | 7.49 µs | 2.64 µs |
| réinitialisation du modèle après une ligne retirée | 7.27 µs | 2.57 µs |
| édition d'une cellule de texte | 463 ns | 99.3 ns |
| édition d'une cellule de position | 8.48 µs | 1.95 µs |
| découpage de 4000 sous-titres, en ems, avec cache de longueurs | 191 ms | 5.09 ms |
| versionString | 44.4 ns | 1.02 ns |
| parse | 37.6 ns | 2.63 ns |
| format | 40.1 ns | 15.6 ns |
| position vers image | 7.14 ns | 2.51 ns |
| image vers position | 6.83 ns | 1.11 ns |
| mise à l'échelle par un rationnel exact | 6.88 ns | 0.127 ns |
| découpage de 4000 sous-titres, sans cache de longueurs | 118 ms | 3.67 ms |
| découpage de 4000 sous-titres, avec cache de longueurs | 122 ms | 3.74 ms |
| erreurs courantes du français sur 4000 sous-titres | 166 ms | 4.49 ms |
| erreurs courantes de l'anglais sur 4000 sous-titres | 184 ms | 4.73 ms |
| lecture de 4000 sous-titres | 2.24 ms | 174 µs |
| écriture de 4000 sous-titres | 589 µs | 24.8 µs |
| décalage de 4000 sous-titres | 6.59 µs | 3.1 µs |
| décalage puis annulation | 14.7 µs | 2.91 µs |
| transformation de 4000 sous-titres | 72.1 µs | 13 µs |
| conversion de fréquence sur 4000 sous-titres | 76.4 µs | 7.69 µs |
| alignement sur 4000 sous-titres | 136 µs | 25.6 µs |
| tri de 4000 sous-titres à l'envers | 302 µs | 23.6 µs |
| suppression d'un sous-titre sur deux | 189 µs | 24.8 µs |
| suppression puis annulation | 255 µs | 32.3 µs |
| insertion de 100 sous-titres vides au milieu | 56 µs | 22.1 µs |
| modification d'un texte, à travers une session | 179 ns | 24.9 ns |
| suppression des mentions sur 4000 sous-titres | 1.37 ms | 80 µs |
| mise en italique de 4000 sous-titres | 988 µs | 71.6 µs |
| remplacement d'un mot fréquent sur 4000 sous-titres, texte simple | 4.23 ms | 206 µs |
| remplacement d'un mot fréquent sur 4000 sous-titres, expression régulière | 4.42 ms | 235 µs |
| recherche sans résultat sur 4000 sous-titres | 3.01 ms | 173 µs |
| ajustement des durées de 4000 sous-titres, quatre contraintes | 1.35 ms | 84.2 µs |
| casse de titre sur 4000 sous-titres | 19.4 ms | 862 µs |
| collage de 4000 textes | 1.04 ms | 125 µs |
| alignement d'une traduction de 4000 lignes, par position | 872 µs | 66.1 µs |
| alignement d'une traduction de 4000 lignes, par numéro | 832 µs | 51.7 µs |
| déduction de fréquence sur 4000 sous-titres | 374 µs | 20.5 µs |

### 0.12.9 — 2026-09-28 — Release — load 1.41 — pace ×1.01

| Measure | Mean | Standard deviation |
| :----- | ------: | ---------: |
| la réplique en cours, sur 4000 sous-titres | 7.28 µs | 674 ns |
| composer une réplique de deux lignes | 196 ns | 77.9 ns |
| ouvrir une vidéo | 8.96 ms | 183 µs |
| chercher une position | 592 µs | 171 µs |
| construction du modèle sur 4000 sous-titres | 8.18 µs | 614 ns |
| une fenêtre de 40 lignes, cinq colonnes | 17.1 µs | 258 ns |
| rafraîchir après un décalage de 4000 sous-titres | 8.16 µs | 649 ns |
| réinitialisation du modèle après une ligne retirée | 8.15 µs | 617 ns |
| édition d'une cellule de texte | 382 ns | 27.5 ns |
| édition d'une cellule de position | 9.58 µs | 2.51 µs |
| découpage de 4000 sous-titres, en ems, avec cache de longueurs | 177 ms | 2.98 ms |
| versionString | 39.3 ns | 0.479 ns |
| parse | 38.3 ns | 2.68 ns |
| format | 36 ns | 0.452 ns |
| position vers image | 6.65 ns | 0.0708 ns |
| image vers position | 6.64 ns | 0.0526 ns |
| mise à l'échelle par un rationnel exact | 6.87 ns | 0.0726 ns |
| découpage de 4000 sous-titres, sans cache de longueurs | 110 ms | 1.52 ms |
| découpage de 4000 sous-titres, avec cache de longueurs | 113 ms | 3.36 ms |
| erreurs courantes du français sur 4000 sous-titres | 155 ms | 4.2 ms |
| erreurs courantes de l'anglais sur 4000 sous-titres | 179 ms | 9.96 ms |
| lecture de 4000 sous-titres | 2.04 ms | 117 µs |
| écriture de 4000 sous-titres | 560 µs | 76.5 µs |
| décalage de 4000 sous-titres | 6.83 µs | 3.07 µs |
| décalage puis annulation | 12.8 µs | 3.61 µs |
| transformation de 4000 sous-titres | 78.7 µs | 14.1 µs |
| conversion de fréquence sur 4000 sous-titres | 76.1 µs | 11.4 µs |
| alignement sur 4000 sous-titres | 126 µs | 11.6 µs |
| tri de 4000 sous-titres à l'envers | 270 µs | 23.7 µs |
| suppression d'un sous-titre sur deux | 164 µs | 21.3 µs |
| suppression puis annulation | 247 µs | 33.2 µs |
| insertion de 100 sous-titres vides au milieu | 56.8 µs | 21.3 µs |
| modification d'un texte, à travers une session | 171 ns | 14.7 ns |
| suppression des mentions sur 4000 sous-titres | 1.55 ms | 103 µs |
| mise en italique de 4000 sous-titres | 946 µs | 56.8 µs |
| remplacement d'un mot fréquent sur 4000 sous-titres, texte simple | 4.14 ms | 238 µs |
| remplacement d'un mot fréquent sur 4000 sous-titres, expression régulière | 4.28 ms | 215 µs |
| recherche sans résultat sur 4000 sous-titres | 2.73 ms | 135 µs |
| ajustement des durées de 4000 sous-titres, quatre contraintes | 1.28 ms | 56.7 µs |
| casse de titre sur 4000 sous-titres | 18.5 ms | 763 µs |
| collage de 4000 textes | 1.01 ms | 60 µs |
| alignement d'une traduction de 4000 lignes, par position | 814 µs | 43.1 µs |
| alignement d'une traduction de 4000 lignes, par numéro | 899 µs | 51.6 µs |
| déduction de fréquence sur 4000 sous-titres | 382 µs | 36.9 µs |

### 0.12.8 — 2026-09-27 — Release — load 1.18 — pace ×0.98

| Measure | Mean | Standard deviation |
| :----- | ------: | ---------: |
| la réplique en cours, sur 4000 sous-titres | 7.59 µs | 1.4 µs |
| composer une réplique de deux lignes | 179 ns | 2.2 ns |
| ouvrir une vidéo | 8.95 ms | 177 µs |
| chercher une position | 563 µs | 146 µs |
| construction du modèle sur 4000 sous-titres | 10.9 µs | 2.24 µs |
| une fenêtre de 40 lignes, cinq colonnes | 17.1 µs | 266 ns |
| rafraîchir après un décalage de 4000 sous-titres | 10.5 µs | 1.73 µs |
| réinitialisation du modèle après une ligne retirée | 10.5 µs | 2.12 µs |
| édition d'une cellule de texte | 412 ns | 77.5 ns |
| édition d'une cellule de position | 11.7 µs | 1.31 µs |
| versionString | 41.2 ns | 8.61 ns |
| parse | 36.8 ns | 2.53 ns |
| format | 36.3 ns | 0.44 ns |
| position vers image | 6.66 ns | 0.195 ns |
| image vers position | 6.65 ns | 0.147 ns |
| mise à l'échelle par un rationnel exact | 6.86 ns | 0.0506 ns |
| découpage de 4000 sous-titres, sans cache de longueurs | 109 ms | 3.52 ms |
| découpage de 4000 sous-titres, avec cache de longueurs | 116 ms | 7.2 ms |
| erreurs courantes du français sur 4000 sous-titres | 161 ms | 9.4 ms |
| erreurs courantes de l'anglais sur 4000 sous-titres | 176 ms | 11.5 ms |
| lecture de 4000 sous-titres | 2.04 ms | 55.3 µs |
| écriture de 4000 sous-titres | 525 µs | 26 µs |
| décalage de 4000 sous-titres | 7.74 µs | 3.1 µs |
| décalage puis annulation | 15 µs | 3.94 µs |
| transformation de 4000 sous-titres | 75.7 µs | 10.2 µs |
| conversion de fréquence sur 4000 sous-titres | 70.9 µs | 8.26 µs |
| alignement sur 4000 sous-titres | 130 µs | 15.9 µs |
| tri de 4000 sous-titres à l'envers | 268 µs | 30.8 µs |
| suppression d'un sous-titre sur deux | 160 µs | 24.6 µs |
| suppression puis annulation | 239 µs | 31.3 µs |
| insertion de 100 sous-titres vides au milieu | 55.5 µs | 20.6 µs |
| modification d'un texte, à travers une session | 181 ns | 45.1 ns |
| suppression des mentions sur 4000 sous-titres | 1.25 ms | 47.9 µs |
| mise en italique de 4000 sous-titres | 885 µs | 33 µs |
| remplacement d'un mot fréquent sur 4000 sous-titres, texte simple | 3.9 ms | 81.1 µs |
| remplacement d'un mot fréquent sur 4000 sous-titres, expression régulière | 4.09 ms | 89.5 µs |
| recherche sans résultat sur 4000 sous-titres | 2.62 ms | 86.6 µs |
| ajustement des durées de 4000 sous-titres, quatre contraintes | 1.23 ms | 67.9 µs |
| casse de titre sur 4000 sous-titres | 18.4 ms | 809 µs |
| collage de 4000 textes | 980 µs | 34.9 µs |
| alignement d'une traduction de 4000 lignes, par position | 828 µs | 39.3 µs |
| alignement d'une traduction de 4000 lignes, par numéro | 806 µs | 34.3 µs |
| déduction de fréquence sur 4000 sous-titres | 369 µs | 11.1 µs |

### 0.12.7 — 2026-09-27 — Release — load 1.08 — pace ×0.95

| Measure | Mean | Standard deviation |
| :----- | ------: | ---------: |
| la réplique en cours, sur 4000 sous-titres | 7.08 µs | 214 ns |
| composer une réplique de deux lignes | 162 ns | 2.03 ns |
| ouvrir une vidéo | 9.01 ms | 351 µs |
| chercher une position | 563 µs | 180 µs |
| construction du modèle sur 4000 sous-titres | 7.13 µs | 494 ns |
| une fenêtre de 40 lignes, cinq colonnes | 16.8 µs | 357 ns |
| rafraîchir après un décalage de 4000 sous-titres | 7.39 µs | 3.24 µs |
| réinitialisation du modèle après une ligne retirée | 7.81 µs | 1.72 µs |
| édition d'une cellule de texte | 395 ns | 38.6 ns |
| édition d'une cellule de position | 8.09 µs | 318 ns |
| versionString | 39.4 ns | 0.58 ns |
| parse | 37 ns | 2.72 ns |
| format | 34 ns | 5.89 ns |
| position vers image | 6.64 ns | 0.0686 ns |
| image vers position | 6.64 ns | 0.0699 ns |
| mise à l'échelle par un rationnel exact | 6.86 ns | 0.0536 ns |
| erreurs courantes du français sur 4000 sous-titres | 153 ms | 6.72 ms |
| erreurs courantes de l'anglais sur 4000 sous-titres | 171 ms | 8.26 ms |
| lecture de 4000 sous-titres | 2.24 ms | 157 µs |
| écriture de 4000 sous-titres | 503 µs | 16.8 µs |
| décalage de 4000 sous-titres | 6.69 µs | 3.01 µs |
| décalage puis annulation | 12.9 µs | 3.37 µs |
| transformation de 4000 sous-titres | 70 µs | 5.86 µs |
| conversion de fréquence sur 4000 sous-titres | 68.4 µs | 9.99 µs |
| alignement sur 4000 sous-titres | 140 µs | 4.23 µs |
| tri de 4000 sous-titres à l'envers | 298 µs | 21 µs |
| suppression d'un sous-titre sur deux | 183 µs | 19.9 µs |
| suppression puis annulation | 243 µs | 39.7 µs |
| insertion de 100 sous-titres vides au milieu | 59.1 µs | 12 µs |
| modification d'un texte, à travers une session | 172 ns | 6.41 ns |
| suppression des mentions sur 4000 sous-titres | 1.37 ms | 117 µs |
| mise en italique de 4000 sous-titres | 903 µs | 57.9 µs |
| remplacement d'un mot fréquent sur 4000 sous-titres, texte simple | 4.46 ms | 127 µs |
| remplacement d'un mot fréquent sur 4000 sous-titres, expression régulière | 4.18 ms | 268 µs |
| recherche sans résultat sur 4000 sous-titres | 2.87 ms | 182 µs |
| ajustement des durées de 4000 sous-titres, quatre contraintes | 1.29 ms | 98.6 µs |
| casse de titre sur 4000 sous-titres | 17.8 ms | 718 µs |
| collage de 4000 textes | 977 µs | 42.1 µs |
| alignement d'une traduction de 4000 lignes, par position | 909 µs | 39.9 µs |
| alignement d'une traduction de 4000 lignes, par numéro | 782 µs | 36.2 µs |
| déduction de fréquence sur 4000 sous-titres | 426 µs | 25.5 µs |

### 0.12.0 — 2026-09-26 — Release — load 1.29 — pace ×0.99

| Measure | Mean | Standard deviation |
| :----- | ------: | ---------: |
| la réplique en cours, sur 4000 sous-titres | 4.94 µs | 2.91 µs |
| composer une réplique de deux lignes | 169 ns | 3.44 ns |
| ouvrir une vidéo | 9.24 ms | 665 µs |
| chercher une position | 563 µs | 143 µs |
| construction du modèle sur 4000 sous-titres | 7.29 µs | 2.53 µs |
| une fenêtre de 40 lignes, cinq colonnes | 20.3 µs | 7.31 µs |
| rafraîchir après un décalage de 4000 sous-titres | 7.48 µs | 2.29 µs |
| réinitialisation du modèle après une ligne retirée | 7.58 µs | 3.41 µs |
| édition d'une cellule de texte | 380 ns | 49.2 ns |
| édition d'une cellule de position | 8.27 µs | 1.37 µs |
| versionString | 37.9 ns | 7.85 ns |
| parse | 39.1 ns | 11.3 ns |
| format | 39.4 ns | 1.76 ns |
| position vers image | 7.81 ns | 2.51 ns |
| image vers position | 6.64 ns | 0.0412 ns |
| mise à l'échelle par un rationnel exact | 6.86 ns | 0.0783 ns |
| lecture de 4000 sous-titres | 2.35 ms | 296 µs |
| écriture de 4000 sous-titres | 521 µs | 43.1 µs |
| décalage de 4000 sous-titres | 7.72 µs | 3.14 µs |
| décalage puis annulation | 13.5 µs | 4.8 µs |
| transformation de 4000 sous-titres | 80.8 µs | 9.58 µs |
| conversion de fréquence sur 4000 sous-titres | 70.1 µs | 12.1 µs |
| alignement sur 4000 sous-titres | 137 µs | 23.6 µs |
| tri de 4000 sous-titres à l'envers | 299 µs | 19.3 µs |
| suppression d'un sous-titre sur deux | 171 µs | 28.7 µs |
| suppression puis annulation | 244 µs | 44.2 µs |
| insertion de 100 sous-titres vides au milieu | 50.5 µs | 8.83 µs |
| modification d'un texte, à travers une session | 299 ns | 986 ns |
| suppression des mentions sur 4000 sous-titres | 1.41 ms | 132 µs |
| mise en italique de 4000 sous-titres | 926 µs | 92.1 µs |
| remplacement d'un mot fréquent sur 4000 sous-titres, texte simple | 4.25 ms | 526 µs |
| remplacement d'un mot fréquent sur 4000 sous-titres, expression régulière | 4.39 ms | 474 µs |
| recherche sans résultat sur 4000 sous-titres | 2.89 ms | 191 µs |
| ajustement des durées de 4000 sous-titres, quatre contraintes | 1.32 ms | 79.5 µs |
| casse de titre sur 4000 sous-titres | 19.2 ms | 1.04 ms |
| collage de 4000 textes | 1.06 ms | 71.3 µs |
| alignement d'une traduction de 4000 lignes, par position | 853 µs | 64.9 µs |
| alignement d'une traduction de 4000 lignes, par numéro | 871 µs | 58.1 µs |
| déduction de fréquence sur 4000 sous-titres | 377 µs | 24 µs |

### 0.11.30 — 2026-09-25 — Release — load 1.44 — pace ×1.02

| Measure | Mean | Standard deviation |
| :----- | ------: | ---------: |
| la réplique en cours, sur 4000 sous-titres | 7.4 µs | 1.41 µs |
| composer une réplique de deux lignes | 167 ns | 3.28 ns |
| ouvrir une vidéo | 9.33 ms | 714 µs |
| chercher une position | 575 µs | 136 µs |
| construction du modèle sur 4000 sous-titres | 8.49 µs | 2.59 µs |
| une fenêtre de 40 lignes, cinq colonnes | 17.4 µs | 3.39 µs |
| rafraîchir après un décalage de 4000 sous-titres | 8.33 µs | 1.11 µs |
| réinitialisation du modèle après une ligne retirée | 8.4 µs | 1.97 µs |
| édition d'une cellule de texte | 382 ns | 37 ns |
| édition d'une cellule de position | 10.7 µs | 415 ns |
| versionString | 43 ns | 9.85 ns |
| parse | 43.3 ns | 15.4 ns |
| format | 37.7 ns | 1.82 ns |
| position vers image | 6.64 ns | 0.0691 ns |
| image vers position | 6.64 ns | 0.073 ns |
| mise à l'échelle par un rationnel exact | 7.34 ns | 3.1 ns |
| lecture de 4000 sous-titres | 2.25 ms | 145 µs |
| écriture de 4000 sous-titres | 518 µs | 41.7 µs |
| décalage de 4000 sous-titres | 6.82 µs | 2.44 µs |
| décalage puis annulation | 13.5 µs | 3.81 µs |
| transformation de 4000 sous-titres | 71 µs | 4.36 µs |
| conversion de fréquence sur 4000 sous-titres | 68.9 µs | 8.02 µs |
| alignement sur 4000 sous-titres | 129 µs | 17.1 µs |
| tri de 4000 sous-titres à l'envers | 274 µs | 27.1 µs |
| suppression d'un sous-titre sur deux | 159 µs | 17 µs |
| suppression puis annulation | 253 µs | 63.1 µs |
| insertion de 100 sous-titres vides au milieu | 52.9 µs | 14.2 µs |
| modification d'un texte, à travers une session | 168 ns | 12.2 ns |
| suppression des mentions sur 4000 sous-titres | 1.45 ms | 53.9 µs |
| mise en italique de 4000 sous-titres | 913 µs | 48.4 µs |
| remplacement d'un mot fréquent sur 4000 sous-titres, texte simple | 4.19 ms | 276 µs |
| remplacement d'un mot fréquent sur 4000 sous-titres, expression régulière | 4.31 ms | 268 µs |
| recherche sans résultat sur 4000 sous-titres | 2.91 ms | 179 µs |
| ajustement des durées de 4000 sous-titres, quatre contraintes | 1.33 ms | 72.6 µs |
| casse de titre sur 4000 sous-titres | 19.4 ms | 972 µs |
| collage de 4000 textes | 1.07 ms | 75.5 µs |
| alignement d'une traduction de 4000 lignes, par position | 856 µs | 56.4 µs |
| alignement d'une traduction de 4000 lignes, par numéro | 883 µs | 85.8 µs |
| déduction de fréquence sur 4000 sous-titres | 387 µs | 42.2 µs |

### 0.11.25 — 2026-09-25 — Release — load 1.37 — pace ×1.19

| Measure | Mean | Standard deviation |
| :----- | ------: | ---------: |
| la réplique en cours, sur 4000 sous-titres | 14.1 µs | 2.34 µs |
| composer une réplique de deux lignes | 817 ns | 67.1 ns |
| ouvrir une vidéo | 32.7 ms | 25.8 ms |
| chercher une position | 617 µs | 112 µs |
| construction du modèle sur 4000 sous-titres | 13.2 µs | 1.44 µs |
| une fenêtre de 40 lignes, cinq colonnes | 20.8 µs | 1.19 µs |
| rafraîchir après un décalage de 4000 sous-titres | 13.2 µs | 2.92 µs |
| réinitialisation du modèle après une ligne retirée | 13.6 µs | 2.98 µs |
| édition d'une cellule de texte | 502 ns | 199 ns |
| édition d'une cellule de position | 11 µs | 727 ns |
| versionString | 50.6 ns | 1 ns |
| parse | 42.3 ns | 2.23 ns |
| format | 40.7 ns | 2.59 ns |
| position vers image | 7.85 ns | 1.35 ns |
| image vers position | 8.28 ns | 2.07 ns |
| mise à l'échelle par un rationnel exact | 8.3 ns | 0.221 ns |
| lecture de 4000 sous-titres | 2.64 ms | 200 µs |
| écriture de 4000 sous-titres | 949 µs | 412 µs |
| décalage de 4000 sous-titres | 8.78 µs | 3.21 µs |
| décalage puis annulation | 15.5 µs | 2.65 µs |
| transformation de 4000 sous-titres | 85.8 µs | 3.07 µs |
| conversion de fréquence sur 4000 sous-titres | 85.9 µs | 12.2 µs |
| alignement sur 4000 sous-titres | 153 µs | 11.8 µs |
| tri de 4000 sous-titres à l'envers | 329 µs | 46.1 µs |
| suppression d'un sous-titre sur deux | 183 µs | 14.7 µs |
| suppression puis annulation | 286 µs | 24.8 µs |
| insertion de 100 sous-titres vides au milieu | 70.8 µs | 21.1 µs |
| modification d'un texte, à travers une session | 219 ns | 64.6 ns |
| suppression des mentions sur 4000 sous-titres | 1.57 ms | 53.8 µs |
| mise en italique de 4000 sous-titres | 1.09 ms | 99.1 µs |
| remplacement d'un mot fréquent sur 4000 sous-titres, texte simple | 4.69 ms | 235 µs |
| remplacement d'un mot fréquent sur 4000 sous-titres, expression régulière | 4.82 ms | 238 µs |
| recherche sans résultat sur 4000 sous-titres | 3.16 ms | 193 µs |
| ajustement des durées de 4000 sous-titres, quatre contraintes | 1.43 ms | 76 µs |
| casse de titre sur 4000 sous-titres | 22 ms | 1.47 ms |
| collage de 4000 textes | 1.19 ms | 160 µs |
| alignement d'une traduction de 4000 lignes, par position | 1.1 ms | 293 µs |
| alignement d'une traduction de 4000 lignes, par numéro | 973 µs | 60 µs |
| déduction de fréquence sur 4000 sous-titres | 436 µs | 22 µs |

### 0.11.21 — 2026-09-24 — Release — load 1.47 — pace ×0.96

| Measure | Mean | Standard deviation |
| :----- | ------: | ---------: |
| la réplique en cours, sur 4000 sous-titres | 6.01 µs | 1.24 µs |
| composer une réplique de deux lignes | 187 ns | 3.06 ns |
| ouvrir une vidéo | 8.94 ms | 164 µs |
| chercher une position | 557 µs | 124 µs |
| construction du modèle sur 4000 sous-titres | 7.25 µs | 1.7 µs |
| une fenêtre de 40 lignes, cinq colonnes | 17.5 µs | 711 ns |
| rafraîchir après un décalage de 4000 sous-titres | 7.19 µs | 1.15 µs |
| réinitialisation du modèle après une ligne retirée | 7.38 µs | 1.92 µs |
| édition d'une cellule de texte | 398 ns | 48.8 ns |
| édition d'une cellule de position | 8.16 µs | 737 ns |
| versionString | 42 ns | 0.435 ns |
| parse | 37.6 ns | 3.28 ns |
| format | 35.6 ns | 0.688 ns |
| position vers image | 6.47 ns | 0.0513 ns |
| image vers position | 6.32 ns | 0.0685 ns |
| mise à l'échelle par un rationnel exact | 6.72 ns | 0.0731 ns |
| lecture de 4000 sous-titres | 2.06 ms | 58.3 µs |
| écriture de 4000 sous-titres | 525 µs | 28.8 µs |
| décalage de 4000 sous-titres | 6.64 µs | 3.18 µs |
| décalage puis annulation | 14 µs | 9.38 µs |
| transformation de 4000 sous-titres | 70.9 µs | 5.75 µs |
| conversion de fréquence sur 4000 sous-titres | 69.3 µs | 20.4 µs |
| alignement sur 4000 sous-titres | 123 µs | 8.1 µs |
| tri de 4000 sous-titres à l'envers | 257 µs | 23.2 µs |
| suppression d'un sous-titre sur deux | 164 µs | 22.9 µs |
| suppression puis annulation | 241 µs | 35.9 µs |
| insertion de 100 sous-titres vides au milieu | 51.6 µs | 20.5 µs |
| modification d'un texte, à travers une session | 180 ns | 11.7 ns |
| suppression des mentions sur 4000 sous-titres | 1.23 ms | 71.1 µs |
| mise en italique de 4000 sous-titres | 942 µs | 62.6 µs |
| remplacement d'un mot fréquent sur 4000 sous-titres, texte simple | 4.04 ms | 192 µs |
| remplacement d'un mot fréquent sur 4000 sous-titres, expression régulière | 4.15 ms | 107 µs |
| recherche sans résultat sur 4000 sous-titres | 2.69 ms | 92.6 µs |
| ajustement des durées de 4000 sous-titres, quatre contraintes | 1.23 ms | 49.8 µs |
| casse de titre sur 4000 sous-titres | 18.2 ms | 684 µs |
| collage de 4000 textes | 1.01 ms | 32 µs |
| alignement d'une traduction de 4000 lignes, par position | 860 µs | 113 µs |
| alignement d'une traduction de 4000 lignes, par numéro | 855 µs | 61.2 µs |
| déduction de fréquence sur 4000 sous-titres | 368 µs | 14.5 µs |

### 0.11.7 — 2026-09-21 — Release — load 1.41 — pace ×0.98

| Measure | Mean | Standard deviation |
| :----- | ------: | ---------: |
| la réplique en cours, sur 4000 sous-titres | 7.17 µs | 1.85 µs |
| composer une réplique de deux lignes | 164 ns | 3.11 ns |
| ouvrir une vidéo | 9.11 ms | 591 µs |
| chercher une position | 569 µs | 158 µs |
| construction du modèle sur 4000 sous-titres | 7.38 µs | 2.89 µs |
| une fenêtre de 40 lignes, cinq colonnes | 17.5 µs | 838 ns |
| rafraîchir après un décalage de 4000 sous-titres | 8.38 µs | 2.33 µs |
| réinitialisation du modèle après une ligne retirée | 8.38 µs | 2.47 µs |
| édition d'une cellule de texte | 419 ns | 194 ns |
| édition d'une cellule de position | 9.33 µs | 667 ns |
| versionString | 41.3 ns | 12.5 ns |
| parse | 36.8 ns | 3.18 ns |
| format | 35.3 ns | 0.58 ns |
| position vers image | 6.48 ns | 0.0669 ns |
| image vers position | 6.32 ns | 0.0763 ns |
| mise à l'échelle par un rationnel exact | 7.7 ns | 0.0994 ns |
| lecture de 4000 sous-titres | 2.23 ms | 182 µs |
| écriture de 4000 sous-titres | 560 µs | 60 µs |
| décalage de 4000 sous-titres | 8.18 µs | 10.8 µs |
| décalage puis annulation | 14.9 µs | 3.4 µs |
| transformation de 4000 sous-titres | 72 µs | 9.68 µs |
| conversion de fréquence sur 4000 sous-titres | 66.2 µs | 8.29 µs |
| alignement sur 4000 sous-titres | 126 µs | 10.6 µs |
| tri de 4000 sous-titres à l'envers | 270 µs | 29.2 µs |
| suppression d'un sous-titre sur deux | 162 µs | 24.8 µs |
| suppression puis annulation | 235 µs | 32.9 µs |
| insertion de 100 sous-titres vides au milieu | 51.3 µs | 16.6 µs |
| modification d'un texte, à travers une session | 200 ns | 23.2 ns |
| suppression des mentions sur 4000 sous-titres | 1.27 ms | 90.3 µs |
| mise en italique de 4000 sous-titres | 983 µs | 63.5 µs |
| remplacement d'un mot fréquent sur 4000 sous-titres, texte simple | 4 ms | 260 µs |
| remplacement d'un mot fréquent sur 4000 sous-titres, expression régulière | 4.18 ms | 288 µs |
| recherche sans résultat sur 4000 sous-titres | 2.67 ms | 165 µs |
| ajustement des durées de 4000 sous-titres, quatre contraintes | 1.23 ms | 72 µs |
| casse de titre sur 4000 sous-titres | 19.9 ms | 1.78 ms |
| collage de 4000 textes | 1.13 ms | 69.5 µs |
| alignement d'une traduction de 4000 lignes, par position | 851 µs | 69.7 µs |
| alignement d'une traduction de 4000 lignes, par numéro | 824 µs | 64.9 µs |
| déduction de fréquence sur 4000 sous-titres | 370 µs | 19 µs |

### 0.11.1 — 2026-09-19 — Release — load 1.39 — pace ×1.11

| Measure | Mean | Standard deviation |
| :----- | ------: | ---------: |
| la réplique en cours, sur 4000 sous-titres | 5.83 µs | 993 ns |
| composer une réplique de deux lignes | 206 ns | 18.6 ns |
| ouvrir une vidéo | 9.31 ms | 642 µs |
| chercher une position | 584 µs | 182 µs |
| construction du modèle sur 4000 sous-titres | 9.69 µs | 2.15 µs |
| une fenêtre de 40 lignes, cinq colonnes | 17.5 µs | 460 ns |
| rafraîchir après un décalage de 4000 sous-titres | 8.38 µs | 2.2 µs |
| réinitialisation du modèle après une ligne retirée | 8.91 µs | 2.52 µs |
| édition d'une cellule de texte | 460 ns | 73.6 ns |
| édition d'une cellule de position | 9.51 µs | 1.27 µs |
| versionString | 42.3 ns | 0.852 ns |
| parse | 47.7 ns | 9.13 ns |
| format | 41.9 ns | 2.6 ns |
| position vers image | 7.62 ns | 0.0625 ns |
| image vers position | 8.06 ns | 0.438 ns |
| mise à l'échelle par un rationnel exact | 7.87 ns | 0.0843 ns |
| lecture de 4000 sous-titres | 2.58 ms | 152 µs |
| écriture de 4000 sous-titres | 626 µs | 93.3 µs |
| décalage de 4000 sous-titres | 8.32 µs | 5.12 µs |
| décalage puis annulation | 14.9 µs | 3.26 µs |
| transformation de 4000 sous-titres | 85.5 µs | 8.04 µs |
| conversion de fréquence sur 4000 sous-titres | 84.6 µs | 18.3 µs |
| alignement sur 4000 sous-titres | 142 µs | 9.16 µs |
| tri de 4000 sous-titres à l'envers | 304 µs | 35 µs |
| suppression d'un sous-titre sur deux | 188 µs | 23.2 µs |
| suppression puis annulation | 287 µs | 73.1 µs |
| insertion de 100 sous-titres vides au milieu | 58.6 µs | 19.2 µs |
| modification d'un texte, à travers une session | 209 ns | 19.2 ns |
| suppression des mentions sur 4000 sous-titres | 1.41 ms | 35 µs |
| mise en italique de 4000 sous-titres | 1.1 ms | 155 µs |
| remplacement d'un mot fréquent sur 4000 sous-titres, texte simple | 4.65 ms | 307 µs |
| remplacement d'un mot fréquent sur 4000 sous-titres, expression régulière | 4.74 ms | 206 µs |
| recherche sans résultat sur 4000 sous-titres | 3.11 ms | 262 µs |
| ajustement des durées de 4000 sous-titres, quatre contraintes | 1.38 ms | 102 µs |
| casse de titre sur 4000 sous-titres | 21.2 ms | 1.87 ms |
| collage de 4000 textes | 1.26 ms | 189 µs |
| déduction de fréquence sur 4000 sous-titres | 447 µs | 14.4 µs |

### 0.10.18 — 2026-09-19 — Release — load 0.85 — pace ×0.95

| Measure | Mean | Standard deviation |
| :----- | ------: | ---------: |
| la réplique en cours, sur 4000 sous-titres | 4.74 µs | 1.28 µs |
| composer une réplique de deux lignes | 188 ns | 56 ns |
| ouvrir une vidéo | 9.65 ms | 1.41 ms |
| chercher une position | 568 µs | 173 µs |
| construction du modèle sur 4000 sous-titres | 8.8 µs | 2.69 µs |
| une fenêtre de 40 lignes, cinq colonnes | 18 µs | 6.1 µs |
| rafraîchir après un décalage de 4000 sous-titres | 9.48 µs | 7.42 µs |
| réinitialisation du modèle après une ligne retirée | 8.38 µs | 1.42 µs |
| édition d'une cellule de texte | 404 ns | 120 ns |
| édition d'une cellule de position | 10.7 µs | 502 ns |
| versionString | 42 ns | 0.467 ns |
| parse | 37 ns | 4.98 ns |
| format | 37.1 ns | 5.35 ns |
| position vers image | 7.96 ns | 2.09 ns |
| image vers position | 6.64 ns | 0.0771 ns |
| mise à l'échelle par un rationnel exact | 7.87 ns | 0.101 ns |
| lecture de 4000 sous-titres | 2.49 ms | 283 µs |
| écriture de 4000 sous-titres | 594 µs | 80.5 µs |
| décalage de 4000 sous-titres | 7.03 µs | 3.43 µs |
| décalage puis annulation | 13.8 µs | 4.13 µs |
| transformation de 4000 sous-titres | 77.3 µs | 10.8 µs |
| conversion de fréquence sur 4000 sous-titres | 70.6 µs | 17.3 µs |
| alignement sur 4000 sous-titres | 128 µs | 20.5 µs |
| tri de 4000 sous-titres à l'envers | 261 µs | 18.5 µs |
| suppression d'un sous-titre sur deux | 156 µs | 21.9 µs |
| suppression puis annulation | 236 µs | 33.8 µs |
| insertion de 100 sous-titres vides au milieu | 50.7 µs | 12.4 µs |
| modification d'un texte, à travers une session | 191 ns | 20.6 ns |
| suppression des mentions sur 4000 sous-titres | 1.24 ms | 94.2 µs |
| mise en italique de 4000 sous-titres | 952 µs | 80.3 µs |
| remplacement d'un mot fréquent sur 4000 sous-titres, texte simple | 4.02 ms | 306 µs |
| remplacement d'un mot fréquent sur 4000 sous-titres, expression régulière | 4.18 ms | 247 µs |
| recherche sans résultat sur 4000 sous-titres | 2.66 ms | 187 µs |
| ajustement des durées de 4000 sous-titres, quatre contraintes | 1.23 ms | 93 µs |
| casse de titre sur 4000 sous-titres | 19 ms | 1.52 ms |
| collage de 4000 textes | 1.04 ms | 79 µs |
| déduction de fréquence sur 4000 sous-titres | 427 µs | 17.6 µs |

### 0.10.17 — 2026-09-19 — Release — load 0.91 — pace ×0.86

| Measure | Mean | Standard deviation |
| :----- | ------: | ---------: |
| la réplique en cours, sur 4000 sous-titres | 6.01 µs | 3.47 µs |
| composer une réplique de deux lignes | 176 ns | 2.87 ns |
| ouvrir une vidéo | 8.98 ms | 543 µs |
| chercher une position | 596 µs | 180 µs |
| construction du modèle sur 4000 sous-titres | 8.58 µs | 4.41 µs |
| une fenêtre de 40 lignes, cinq colonnes | 15.6 µs | 2.06 µs |
| rafraîchir après un décalage de 4000 sous-titres | 8.52 µs | 3.96 µs |
| réinitialisation du modèle après une ligne retirée | 9.86 µs | 4.43 µs |
| édition d'une cellule de texte | 404 ns | 162 ns |
| édition d'une cellule de position | 9.54 µs | 1.87 µs |
| versionString | 41.8 ns | 0.484 ns |
| parse | 36.2 ns | 2.89 ns |
| format | 36.7 ns | 0.672 ns |
| position vers image | 6.75 ns | 0.252 ns |
| image vers position | 6.64 ns | 0.0761 ns |
| mise à l'échelle par un rationnel exact | 7.86 ns | 0.06 ns |
| lecture de 4000 sous-titres | 2.2 ms | 153 µs |
| écriture de 4000 sous-titres | 558 µs | 67.3 µs |
| décalage de 4000 sous-titres | 7.17 µs | 3.94 µs |
| décalage puis annulation | 13.3 µs | 2.72 µs |
| transformation de 4000 sous-titres | 78.7 µs | 7.82 µs |
| conversion de fréquence sur 4000 sous-titres | 68.3 µs | 10.2 µs |
| alignement sur 4000 sous-titres | 124 µs | 9.24 µs |
| tri de 4000 sous-titres à l'envers | 261 µs | 23.4 µs |
| suppression d'un sous-titre sur deux | 184 µs | 13.8 µs |
| suppression puis annulation | 232 µs | 26 µs |
| insertion de 100 sous-titres vides au milieu | 52.7 µs | 16.2 µs |
| modification d'un texte, à travers une session | 178 ns | 36.4 ns |
| suppression des mentions sur 4000 sous-titres | 1.23 ms | 71.5 µs |
| mise en italique de 4000 sous-titres | 906 µs | 60.8 µs |
| remplacement d'un mot fréquent sur 4000 sous-titres, texte simple | 3.85 ms | 122 µs |
| remplacement d'un mot fréquent sur 4000 sous-titres, expression régulière | 4.12 ms | 221 µs |
| recherche sans résultat sur 4000 sous-titres | 2.68 ms | 166 µs |
| ajustement des durées de 4000 sous-titres, quatre contraintes | 1.2 ms | 63.7 µs |
| casse de titre sur 4000 sous-titres | 17.9 ms | 585 µs |
| collage de 4000 textes | 964 µs | 32.9 µs |
| déduction de fréquence sur 4000 sous-titres | 367 µs | 9.61 µs |

### 0.10.15 — 2026-09-16 — Release — load 0.75 — pace ×1.13

| Measure | Mean | Standard deviation |
| :----- | ------: | ---------: |
| la réplique en cours, sur 4000 sous-titres | 5.63 µs | 2.19 µs |
| composer une réplique de deux lignes | 194 ns | 3.52 ns |
| ouvrir une vidéo | 10.4 ms | 2.34 ms |
| chercher une position | 584 µs | 130 µs |
| construction du modèle sur 4000 sous-titres | 8.43 µs | 2.82 µs |
| une fenêtre de 40 lignes, cinq colonnes | 18.5 µs | 4.06 µs |
| rafraîchir après un décalage de 4000 sous-titres | 10.7 µs | 5.81 µs |
| réinitialisation du modèle après une ligne retirée | 10.3 µs | 8.17 µs |
| édition d'une cellule de texte | 477 ns | 92.7 ns |
| édition d'une cellule de position | 14.5 µs | 1.92 µs |
| versionString | 50.2 ns | 1.2 ns |
| parse | 52.6 ns | 24.8 ns |
| format | 44.5 ns | 7.24 ns |
| position vers image | 7.95 ns | 1.82 ns |
| image vers position | 7.42 ns | 0.142 ns |
| mise à l'échelle par un rationnel exact | 7.7 ns | 0.1 ns |
| lecture de 4000 sous-titres | 2.6 ms | 153 µs |
| écriture de 4000 sous-titres | 627 µs | 85.3 µs |
| décalage de 4000 sous-titres | 9.65 µs | 5.76 µs |
| décalage puis annulation | 21.2 µs | 23.1 µs |
| transformation de 4000 sous-titres | 85.1 µs | 23 µs |
| conversion de fréquence sur 4000 sous-titres | 77.6 µs | 9.25 µs |
| alignement sur 4000 sous-titres | 150 µs | 20.8 µs |
| tri de 4000 sous-titres à l'envers | 320 µs | 21.1 µs |
| suppression d'un sous-titre sur deux | 196 µs | 37.4 µs |
| suppression puis annulation | 314 µs | 44.6 µs |
| insertion de 100 sous-titres vides au milieu | 71.6 µs | 32.4 µs |
| modification d'un texte, à travers une session | 236 ns | 65.2 ns |
| suppression des mentions sur 4000 sous-titres | 2.3 ms | 288 µs |
| mise en italique de 4000 sous-titres | 1.87 ms | 555 µs |
| déduction de fréquence sur 4000 sous-titres | 432 µs | 25 µs |
