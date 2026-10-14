# 0006 — Represent positions as whole milliseconds, with strong types

**Date:** 2026-08-05
**Status:** accepted

## Context

Gaupol encodes the *kind* of a position in the **Python type of the value**: an
`int` is a frame number, a `float` is seconds, a `str` is a `"HH:MM:SS.mmm"`
time. Every method of its `Calculator` starts with a dispatch chain, and times
are split by character index:

```python
float(time[:2]) * 3600 + float(time[3:5]) * 60 + float(time[6:8]) + float(time[9:]) / 1000
```

A subtitle stores its positions "in native units" — a time string or a frame
integer depending on a `mode` attribute carried by the subtitle itself.

The MVP's outline also requires frame rate conversion, hence round trips between
times and frames that must not drift.

## Decision

Three distinct types, not implicitly convertible to one another nor to an
integer type:

- **`Timestamp`** — absolute position, `std::int64_t` of signed milliseconds;
- **`Duration`** — duration, `std::int64_t` of signed milliseconds;
- **`FrameRate`** — frame rate, an **exact rational** `numerator /
  denominator`, for example `24000/1001` for 23.976.

The arithmetic allowed is the arithmetic that makes sense:

```
Timestamp − Timestamp → Duration
Timestamp ± Duration  → Timestamp
Duration  ± Duration  → Duration
Duration  × scalar    → Duration
Timestamp + Timestamp → does not compile
```

The frame number is **not** an internal representation: it is a view, obtained
by explicit conversion with a `FrameRate`.

**Negative positions are valid.** A shift can legitimately move a subtitle
before the origin, and Gaupol already allows it.

**Rounding: to the nearest integer, halves away from zero.** This choice is
symmetric around zero, so shifting a subtitle before and after the origin by the
same amount gives consistent results. Gaupol inherits Python's round-half-to-even,
which is not.

## Alternatives dismissed

- **Reproduce Gaupol's model** — native units per subtitle, type carried by the
  value. Dismissed without hesitation: it is the counter-model named in
  [`../principes-de-conception.md`](../principes-de-conception.md), and it forces
  every function into a run-time dispatch that the compiler should be doing.
- **Floating-point seconds** — simple, but the accumulation of errors over a
  shift followed by an affine transformation is exactly what a subtitle editor
  must not produce. Two positions equal on display could differ in memory.
- **`std::chrono::milliseconds`** — semantically right and tempting. Dismissed
  because `chrono` does not distinguish an instant from a duration without going
  through `time_point`, whose clock makes no sense here, and because formatting
  to `HH:MM:SS,mmm` and arithmetic with a rational frame rate would need a type
  of our own anyway. Nothing prevents relying on it internally.
- **Frame rate as a `double`** — what Gaupol does. Dismissed: `24/1.001` is not
  exactly representable, and the error spreads with every conversion. An exact
  rational removes the question.

## Consequences

**A frame → milliseconds → frame round trip is exact** as long as the frame rate
does not change. At 60 frames per second, a frame lasts 16.67 ms: the millisecond
grid is finer than the frame grid, so nothing is lost. That is what makes it
safe to unify the internal representation on milliseconds, including for a
natively frame-based format such as MicroDVD in phase 9.

**The reverse round trip is not** — milliseconds → frame → milliseconds loses
sub-frame precision. This is inherent, not a design flaw, and the phase 1 spec
states it as expected and tested behaviour.

Confusing a duration with a position becomes a compile error. It is the classic
mistake in a subtitle editor, and it disappears.

In return, every conversion has to be written. This is intended: an implicit
conversion is a rounding decision nobody took.

Gaupol's saturation at `99:59:59.999` is a **formatting** constraint, not a
representation one: `int64_t` reaches far beyond it. It applies on writing.
