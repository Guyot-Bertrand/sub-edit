# 0011 — Type the frame number rather than leave it an integer

**Date:** 2026-08-06
**Status:** accepted

## Context

[0006](0006-positions-en-millisecondes.md) sets three strong types —
`Timestamp`, `Duration`, `FrameRate` — and states that the frame number is not
an internal representation but "a view, obtained by explicit conversion with a
`FrameRate`". It does not say what type that view has.

The question arises while writing issue #2, when the time ↔ frame conversions
need a signature. Two answers present themselves: a bare `std::int64_t`, or a
fourth type.

Yet the [design principles](../principes-de-conception.md) forbid "a bare
`int` to represent a duration, a position, a subtitle number or a track index".
A frame number is a position. Making the view an integer would reintroduce, at
the conversion boundary, exactly the confusion the three other types eliminate:
nothing would prevent adding a frame number to a count of milliseconds.

## Decision

The frame number is a strong type, **`Frame`**, not implicitly convertible to
an integer nor to the other position types.

The conversions are two explicit member functions of `Timestamp`:
`Timestamp::fromFrame(Frame, FrameRate)` and `Timestamp::toFrame(FrameRate)`.

Unlike time, a frame has no sub-unit: the same type serves as position and
count, and `Frame + Frame` has a meaning. That is the only difference in shape
from the `Timestamp` / `Duration` pair.

## Alternatives dismissed

- **Bare `std::int64_t`** — faithful to the letter of
  [0006](0006-positions-en-millisecondes.md), which names only three types, and
  a smaller API surface. Dismissed: the saving is some thirty lines, the cost is
  a hole in the only guarantee these types exist to give.
- **Two types, `Frame` and `FrameCount`**, modelled on `Timestamp` and
  `Duration`. Dismissed: the distinction has no physical counterpart — a frame
  lasts one frame — and the arithmetic would become heavier than what it
  protects.
- **Conversions as free functions** rather than members. Dismissed without much
  conviction: `position.toFrame(rate)` reads better than `toFrame(position,
  rate)`, and `time/` stays a single module whose types know each other.

## Consequences

Confusing a frame with a millisecond becomes a compilation error, just like
confusing a duration with a position.

Phase 9 welcomes natively frame-based formats — MicroDVD — without having to
choose a type at that point.

In return, a fourth type to maintain, and a conversion to write wherever a
frame number is displayed or read. That is the price already accepted in
[0006](0006-positions-en-millisecondes.md).

Undoing this decision would touch every conversion site. The trigger would be
a use showing that `Frame` hinders more than it protects — for instance frame
arithmetic rich enough for the named factories to become noise.
