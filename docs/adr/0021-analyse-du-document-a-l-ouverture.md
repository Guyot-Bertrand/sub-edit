# 0021 — A document analysis, computed on opening

**Date:** 2026-08-25
**Status:** accepted

## Context

Phase 16 brings something the core never had: **a judgement on a document**.
Until now, everything it produced was either data read — subtitles, a frame
rate declared by a container — or the result of an operation the user had
requested. Frame-rate deduction is neither: nobody requested it, and it appears
in no file. It is **inferred**.

Two questions arise together, and separating them would give two wrong answers.

**Where does an inference live?** `core/time/` carries the primitives —
`Timestamp`, `Duration`, `Frame`, `FrameRate`, `Ratio` — and a deduction is not
one: it takes a document and returns an opinion. `core/model/` carries what a
document *is*, and an opinion is not part of it. `core/edit/` carries what
*changes* it, and an inference changes nothing.

There is already a precedent nobody managed to name: `core/model/anomaly.hpp`
is an inference too — "this subtitle ends before it starts" is not data from
the file, it is a judgement passed on it. It landed in the model for want of a
better place.

**When is it computed?** On opening, or on demand? The question is not one of
taste: it decides what the surfaces are allowed to show without asking the user
for anything, and it puts a cost on the reading path, which has been measured
since phase 1.

## Decision

**A `core/analysis/` directory, and the deduction computed on opening, under an
explicit budget.**

`core/analysis/` hosts what **observes** a document without changing or
describing it: frame-rate deduction today, anomalies the day they are moved. An
analysis function is pure, knows neither `Project`, nor `Session`, nor the
history, and depends only on the primitives of `core/time/` and on the model's
value types.

**The deduction runs when the file is opened.** Its budget is **one fifth of
the cost of reading** — that is 0.5 ms for 4000 subtitles, reading costing
2.5. The budget is checked by the benchmark, not hoped for.

## Why on opening

Because **the three surfaces that show it show it without being asked**: the
status bar carries the verdict permanently, the conversion dialog pre-fills its
input the moment it opens, and `inspect` reports it in its ordinary output. A
lazy deduction would make each of those three surfaces responsible for
triggering it, hence for deciding when — and three separate decisions on the
same subject end up diverging.

The cost is real and it is small. The method is a pass over the starts, eight
times, and a sum of vectors: it is of the same order as reading, while being
much simpler. If the budget did not hold, the answer would be to make the
deduction lazy behind a cache carried by the document — and it is precisely
because this way out exists that one can start with the simple choice.

## Alternatives dismissed

**Filing the deduction in `core/time/`.** That is where `FrameRate` lives, and
the temptation is great. But `core/time/` is a set of memoryless primitives:
each answers a closed question about a value. A deduction answers an open
question about a collection, and putting it there would make `core/time/` a
place where whatever talks about time is filed — that is, nearly everything.

**Filing the deduction in `core/model/`, with the anomalies.** It would be
consistent with what exists, and that is exactly the problem: what exists is a
defect that would be reproduced. The model describes what a document is; an
anomaly and a deduction say what one thinks of it. Creating `core/analysis/`
now gives that move a destination, without imposing it today — moving
`anomaly.hpp` would touch the table, the CLI and their tests for zero gain as
long as there is only one analysis.

**Computing it on demand.** It would cost nothing on opening, and each surface
would pay the first time. Dismissed for the reason above: three surfaces, three
decisions, a divergence. And the gain amounts to a fraction of a millisecond on
an operation that already costs two and a half.

**Computing it on opening and storing it in the document.** That is the variant
that will come back if the budget does not hold. It is dismissed today because
it brings a derived value into the model, hence an invalidation to maintain:
any edit of a position makes it stale. A pure function that is called again
does not have this problem, and as long as it is fast enough to be called
again, it is the safe choice.

## Consequences

**One more directory in the core**, and a place for what had none.
`check-architecture.sh` has nothing to change for it: the invariant that
matters — `subedit_core` free of any interface dependency — holds for it as for
the others.

**A cost on the reading path**, small and measured. `lecture de 4000
sous-titres` is the record that carries it, and it is the one that will tell
whether the budget holds. The measurement journal makes this check continuous
rather than one-off.

**An invalidation we do not have to maintain**, as long as the deduction stays
a pure function. That is what makes the choice reversible: moving to a cache is
an addition, not a rewrite.

**A move planned and not made.** `core/model/anomaly.hpp` now has a written
destination. Making the move requires a second analysis to be worth it; the day
one exists, the question is already settled.

> **Done since, by #227.** Phase 16 delivered frame-rate deduction and grid
> correction: the second analysis exists, so the condition written above is
> met. `anomaly.hpp` and `anomaly.cpp` live in `core/analysis/`, the callers
> followed, and nothing in their behaviour changed. The decision itself did not
> come back up for discussion — that is the point of a trigger written in
> advance.

The trigger that would reopen this decision: the one-fifth budget not held, or
a second analysis expensive enough that none could run on opening any more.
Neither is in sight.
