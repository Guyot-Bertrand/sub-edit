# 0026 — Leave the include graph as it is, and why

**Date:** 2026-08-31
**Status:** accepted

## Context

Issue [#269](https://github.com/Guyot-Bertrand/sub-edit/issues/269) made static
analysis incremental: clang-tidy is hooked onto each source's compilation rule,
and what decides to reanalyse a file is the dependency graph the compiler
wrote. A property invisible until then became measurable, and expensive:
**touching a header costs exactly the number of translation units that reach
it.**

Measured straight away, on twelve cores:

```
nothing changed            0.4 s
one .cpp touched           1 unit
one deep header touched    129 units
```

Hence the question, which arises naturally once the figure is in front of you:
**can more types be forward-declared, to reduce the radius of a header change?
And for what gain?**

It deserves a measured answer, because the trade's default answer — "yes,
always, forward declarations are good practice" — is the one applied without
checking.

### What was measured, and how

The graph comes from `ninja -C build/tidy -t deps`, that is, from the
dependency files the compiler produced while compiling. Not from a `grep` on
`#include` lines: the real transitive closure, the one that governs
reanalyses. 337 translation units, 125 project headers, 3062 edges.

## Decision

**The include graph stays as it is.** Forward declarations are not
generalised, and no rule encouraging them is written.

A single cut is kept as conceivable, and it is not a forward declaration:
moving the two bridge functions between `Frame` and `Timestamp`. It is not made
here — see "Consequences".

## Alternatives dismissed

### Generalising forward declarations — dismissed, structurally impossible

The headers with the largest radius are **all value types**, and that is
exactly what a forward declaration cannot serve.

| Header | Units reached | out of 337 |
| :----- | ------------: | ---------: |
| `core/time/ratio.hpp` | 153 | 45 % |
| `core/time/frame_rate.hpp` | 149 | 44 % |
| `core/time/duration.hpp` | 145 | 43 % |
| `core/time/frame.hpp` | 130 | 39 % |
| `core/time/timestamp.hpp` | 129 | 38 % |
| `core/model/subtitle_format.hpp` | 126 | 37 % |
| `core/model/source_file.hpp` | 123 | 36 % |

`Timestamp` carries `Duration`, `Frame`, `FrameRate` and `Ratio` **by value, in
`constexpr` bodies written inline**: it calls `rate.millisecondsPerFrame()`,
`frame.number()`, `factor.scale(…)`. `Subtitle` carries `Timestamp start` and
`FormatExtras extras` as members. `Project` exposes `subtitleAt()` inline over
a `std::vector<Subtitle>`.

The compiler needs the complete type in each of these cases. It is not
"inconvenient", it is **impossible**: a `constexpr` function is defined in the
header, and its body does not compile on an incomplete type.

**And it is our own design that imposes it.** The
[design principles](../principes-de-conception.md) call for typed models and
values rather than indirections; ADR
[0013](0013-mise-a-l-echelle-exacte-des-positions.md) makes `Ratio` an exact
type handled by value. A dense include graph is the counterpart of that choice,
not a lapse of hygiene.

### Cutting the heaviest edges — dismissed, the ceiling is too low

The gain of each cut was computed exactly, edge by edge, by removing the edge
from the graph and recounting the transitive closure. Regardless of any
feasibility:

| Gain | Edge cut |
| ---: | :------- |
| **6.2 %** | `model/project.hpp` ⇏ `model/subtitle.hpp` |
| 4.0 % | `time/timestamp.hpp` ⇏ `time/frame.hpp` |
| 3.4 % | `model/subtitle.hpp` ⇏ `model/boundary.hpp` |
| 3.2 % | `model/subtitle.hpp` ⇏ `model/format_extras.hpp` |
| 3.0 % | `model/subtitle.hpp` ⇏ `time/timestamp.hpp` |

**The best imaginable cut is worth 6.2 % of the graph, and it is not
feasible:** `Project` exposes `subtitles()`, `setSubtitles()` and
`subtitleAt()` inline over its `std::vector<Subtitle>`. Moving them out of line
would put a function call on `subtitleAt`, called once per visible row on every
repaint of the table. That is a price paid on every frame to save seconds of
analysis once a quarter.

### Hunting dead includes — dismissed, there are none

`misc-include-cleaner` — disabled in `.clang-tidy`, with its justification —
was rerun as a measuring instrument on two core units: **a single useless
include.** There is no seam to mine.

## Consequences

### What the measurement turned up: the radius costs only when the header changes

It is the result the question was not expecting, and it is worth more than the
answer itself. Weighting the radius by the number of commits that have touched
each file since the start of the project:

| Cost | Radius | Commits | Header |
| ---: | -----: | ------: | :----- |
| **783** | 87 | **9** | `core/model/project.hpp` |
| 438 | 146 | 3 | `core/time/frame_rate.hpp` |
| 300 | 150 | **2** | `core/time/ratio.hpp` |
| 300 | **15** | **20** | `gui/main_window.hpp` |
| 284 | 71 | 4 | `core/io/file_system.hpp` |
| — | 130 | **1** | `core/time/frame.hpp` |

**`ratio.hpp` reaches 45 % of the repository and has changed twice in two
years. `frame.hpp` reaches 39 % and has changed once.** They are finished value
types: they have the largest radius because they are fundamental, and they do
not move *because* they are fundamental. Both properties have the same cause,
which is precisely what makes the optimisation pointless.

The real hot spot of cost is elsewhere, and it is twofold: `project.hpp`, whose
radius is medium but which moves often; and `gui/main_window.hpp`, whose radius
is small and whose churn is enormous.

### In seconds, since it is the only unit that matters

Measured on twelve cores, on an up-to-date `build/tidy` tree:

| Header touched | Units | Time |
| :------------- | ----: | ---: |
| `core/model/project.hpp` | 90 | **331 s** |
| `gui/main_window.hpp` | 16 | **115 s** |

An interface unit costs three times a core unit — 7.2 s against 3.7 — because
it drags in Qt's headers. **The radius alone is therefore a poor estimator of
cost:** sixteen interface units are worth thirty core units.

### What remains open, and what does not

**The `Frame` ↔ `Timestamp` bridge is the only clean cut.** `timestamp.hpp`
includes `frame.hpp` for two functions — `fromFrame(Frame, FrameRate)` and
`toFrame(FrameRate)` — which have **twelve real users**: four in the library,
eight in the tests. They impose `frame.hpp` on 130 units.

Moving them into free functions in a conversion header would bring that radius
down from 130 to 12, without moving anything out of line and without touching
the model. It is not a forward declaration but a function move, and that is
what makes it possible: the dependency comes from a convenience placed on
`Timestamp` rather than from a property of the type.

**It is not made, and not because it is hard.** `frame.hpp` has changed once in
two years: the expected gain is a few minutes over the project's lifetime. The
cut will be made the day that file is reopened for another reason, not for its
own sake.

### What would reopen the question

Three things, and none of them is an opinion on forward declarations:

- **`project.hpp` keeps moving.** Nine commits have touched it, each costing
  331 s of analysis. If its interface does not settle, the lever is not the
  include but stability — moving out of its header what moves, not what it
  includes.
- **An interface unit costs three times a core unit.** If `src/` becomes mostly
  Qt, the calculation above changes its conclusion without any figure of the
  graph having moved.
- **A core value type starts changing often.** It would be the sign that
  something has been added to it that does not belong there, and the answer
  would be to take it away — not to forward-declare it.

### What undoing this decision would cost

Nothing: it writes no code. It writes a measurement and a line of reasoning, so
that they are not redone. The cost of not having written it would be to reopen
the question every six months and to answer it each time by the trade's
intuition, which here gives the wrong answer.
