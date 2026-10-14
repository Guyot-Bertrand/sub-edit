# 0015 — Count uncovered lines, and keep a memory of performance

**Date:** 2026-08-11
**Status:** accepted

## Context

Two measurements, one shared defect: they do not survive their execution.

**Coverage.** `COVERAGE_MIN` was 80 when the actual rate was 99.4%: a
nineteen-point regression would pass the gate without a word. Issue #49 raised
it to 99.2, but that number was not chosen — it was **endured**. gcovr rounds
the rate to one decimal *before* comparing it to the threshold, so an integer
threshold of 99 can never be crossed by a rate of 98.95% or more. It had to be
slipped in between the measured 99.4 and the 99.0 displayed by the defect
injected by `verify-gates.sh`. A threshold set to within four tenths is a
threshold nobody can reason about any more: one no longer knows whether it
protects or is about to cry wolf.

**Performance.** "Measure, don't assume" is a project principle, and no
measurement survives its execution. The frame-rate conversion benchmark
measured the wrong operation for an entire phase; nothing could have shown it,
for lack of history.

What is known at the time of deciding:

- `gcovr --json-summary` exposes the **raw counters**, `line_covered` and
  `line_total`, and not only the rounded percentage. A script that does the
  comparison itself therefore escapes the rounding trap entirely;
- Catch2's **XML reporter** outputs benchmarks in full — mean, confidence
  bounds, standard deviation, outliers. Its JSON reporter, on the other hand,
  has a `benchmarkEnded` with an empty body: it produces nothing.

What is not known: the **natural variance** of the measurements. The same sort
varied from 230 to 870 µs between two runs on the development machine.

## Decision

### Coverage is counted in uncovered lines

The ratchet bears on the **number of lines of `src/lib/` that the tests do not
exercise** — 8 today, out of 1353 — and not on a percentage. It lives in
`docs/mesures/couverture.md`, with its instructions.

`--fail-under-line` goes away: the script reads the `--json-summary`, computes
`line_total - line_covered`, and compares with the file.

| Situation | Reaction |
| :-------- | :------- |
| more uncovered lines than recorded | failure, naming the file that gained some |
| as many | passes, silently |
| fewer | passes, and prompts: `make ratchet` records the new value |

**The gate fails on a downturn, it never writes by itself.** A tracked file
moves only if someone asks for it; but nobody has to compute the figure.

### Performance keeps a journal

`docs/mesures/performances.md`, fed by `make bench` from Catch2's XML output.

- **One section per version**, replaced if re-run at the same number. With the
  patch moving once per pull request, the history becomes one measurement per
  shipped version — the granularity that makes two figures comparable.
- **Mean and standard deviation** per measurement. The standard deviation is
  not decorative: it will carry the alert threshold, once the variance is
  known.
- **The build mode** is recorded, although it is always `Release` today.
- **Pruning at one month**, backed by a **permanent table of extremes**: per
  measurement, the minimum and maximum ever reached, with their version and
  date.
- **No alert threshold**, and the file says why.

## Alternatives dismissed

- **A percentage threshold**, even an exact one rather than a rounded one.
  Dismissed for a reason that has nothing to do with rounding: the percentage
  carries a **perverse effect**. Adding well-tested code raises it, so the
  ratchet rises, so the next piece of work is constrained harder without anyone
  having decided it. The count of uncovered lines does not move when covered
  code is added, and reacts only to what one actually wants to watch.

- **The `covered / total` pair.** Richer in information, but one must define
  what constitutes a regression when both numbers move together — one more
  rule to write, justify and defend.

- **Raising the ratchet automatically** as soon as coverage rises. Dismissed: a
  tracked file would change without being asked to, and it would then have to
  be committed — so the gesture comes back, disguised as a surprise rather than
  an instruction.

- **Leaving everything manual**, the gate staying silent on an upturn.
  Dismissed: the ratchet would drift away from reality, and a threshold lagging
  far below the truth is exactly the defect this ticket fixes.

- **Keeping `--fail-under-line` as a coarse net**, under the fine ratchet.
  Dismissed: two thresholds nobody knows how to reconcile, one of which is known
  to be blunted by rounding.

- **Recording every run of `make bench`.** Attractive because the variance
  would show up for free. Dismissed: three tries in a day would make three
  tracked records, two of which nobody chose to keep. The table of extremes
  gives the envelope without the noise.

- **Writing outside the repository and committing on request.** Consistent with
  the choice made for the ratchet, but the ticket expressly asks that
  `make bench` keep its figures, and one more gesture is a gesture that gets
  forgotten.

- **Pinning old sections that hold a record**, rather than a separate table of
  extremes. Dismissed by arithmetic: sixteen measurements each having two
  extremes, up to thirty-two sections would be pinned, and pruning would stop
  meaning anything.

- **Parsing the benchmarks' console output.** Dismissed for the reason that
  already dismissed the `grep` on `TEST_CASE` in
  [0014](0014-registre-d-exigences.md): Catch2 wraps its lines at 70 columns
  and concatenates the spellings of the same tag. We ask the tool what it
  knows, we do not guess its layout.

- **Catch2's JSON reporter**, which would be the most convenient medium. It
  emits nothing for benchmarks: `benchmarkEnded` has an empty body. That is not
  a choice, it is a fact checked in its sources.

## Consequences

**The proof of the coverage gate becomes robust.** Today it only closes because
99.2 was set four tenths from the actual rate; with a line count, the uncovered
function injected by `verify-gates.sh` makes the gate fail whatever the size of
the project.

**`make check` changes**, and that must be said: its coverage step no longer
uses `--fail-under-line`. It is not an extra step — the step has existed since
phase 0, only the way it decides changes. CI runs nothing new.

**`make check-local` already chains `bench`**, so running it before opening a
pull request checks and records in a single gesture. The modified file is
committed with the rest, which matches the project rule: benchmarks are re-run
for every issue.

**One condition makes "one measurement per shipped version" true rather than
assumed:** that `make check-local` must follow the patch bump, not precede it.
`CMakeLists.txt` is what `record-bench.sh` reads to name the section it writes;
re-run before the bump, it names its section after the *previous* version, and
the next section — the real measurement of this version — is never written:
the current version stays without a record while an already shipped version
gains a second, false one.

**A second condition adds to it, and it took two phases to find:** that
`make check-local` must come **before** `make check`, not after. That is the
decision of the end-of-phase-8 review, and the measurement behind it is the
rate of lost records:

| Phase | Records committed | Refused for load |
| :---- | ----------------: | ---------------: |
| 7 | 7 of 13 | 6 |
| 8 | 3 of 6 | 3 — loads 1.59 · 2.08 · 10.64 |

**Almost one in two, two phases in a row, and #270 had already named the
cause:** what loads the machine is not what `check-local` does before the
bench — `e2e` and `install-check` peak at 1.78 and drop back below the
threshold within twenty seconds — but the quarter of an hour of `make check`
that the procedure prescribed just before. It had also measured the
counter-example: `check-local` alone, on an idle machine, reads 1.38 and gets
recorded.

**Three options were on the table, and they shared a false premise:** that the
bench's place in the procedure was a given. Waiting longer can do nothing — the
tail of a quarter of an hour of compilation lasts minutes, which #270 measured;
taking the bench out of the procedure leaves to chance a measurement that the
definition of "done" requires; accepting one record in two means accepting a
known structural cause.

**First decision: swapping the two targets**, `make check-local` then
`make check`. The bench then finds a machine nothing has warmed up, and the
procedure gains elsewhere too — the diagnostic run of `make bench` before the
gate disappears, since the record that stays now precedes it. **The principle
already required it**: `gate.sh` orders its steps from cheapest to most
expensive, `check` lasts a quarter of an hour and `check-local` a few minutes;
the procedure prescribed the reverse of its own principle.

**It is not enough, and its first run said so.** On a machine starting from
0.63, in the new order, the bench read **2.21** and recorded nothing. The cause
was therefore not — not only — the tail of `make check`.

**Second decision, and it is the one that carries the subject: the load is
read before the build.** `bench.sh` compiled the Release tree, then asked
`await-quiet.sh` whether the machine was free. Two runs a minute apart, on the
same day and on the same machine, isolate what that costs:

| Run | Load read | Outcome |
| :-- | --------: | :------ |
| full `check-local`, with Release rebuild | 2.21 | refused |
| `make bench` alone, Release tree already up to date | **1.32** | **recorded** |

**The second started from a more loaded machine than the first, and was
recorded.** What separates them is the build the step launches itself.

**The guard was badly instrumented, and that is the real subject.**
`/proc/loadavg` gives a **one-minute** average — that is, the past. Read just
after a Release compilation, it answers about that compilation, not about the
machine. The threshold was never wrong, nor the delay: the question was put to
it at the only moment it could not answer.

The reading therefore comes before the build. What the guard now answers is
"was the machine ours when we started", and that is the right question: what
must be ruled out is a competing load — a browser, a development environment,
the desktop session at 10.64 — and that one is there before as well as during.
What it stops seeing is a load that would start during the build; the
dispersion criterion sees it afterwards, and that is precisely why it exists.

**Neither the threshold nor the delay moves.** No measurement calls them into
question.

**The table of extremes is what the future alert threshold will read.** That is
the second step the ticket announces without handling it, and it needs to know
the variance before it can set a figure.

*First figure, in August 2026, over thirty-eight records:* one measurement in
eight exceeds one and a half times the median of its neighbours in time, and
the load threshold rules out nine in ten. That is what
[`analyse-bench-journal.py`](../../src/scripts/analyse-bench-journal.py)
computes from the journal, and the preface of
[`performances.md`](../mesures/performances.md) draws the consequence: the
admission criterion was called into question, measured, and kept.

**The cost, written rather than kept quiet:** a line count does not say
*where*. The script must name the file that gained uncovered lines, otherwise
the failure message sends one looking for a needle in thirteen hundred lines.

The trigger for a review is identifiable: the day a legitimately unreachable
line — a defensive branch, an impossible `default:` — has to be tolerated
durably, the ratchet will force raising it, which silently allows one more
uncovered line elsewhere. If the case repeats, a per-file ratchet or exclusion
markers will become the subject.

Undoing this decision costs little: restore `--fail-under-line` and delete the
two files.
