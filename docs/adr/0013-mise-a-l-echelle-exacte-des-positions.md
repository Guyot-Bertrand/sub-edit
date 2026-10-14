# 0013 — Scale positions by an exact rational, rounded once

**Date:** 2026-08-08
**Status:** accepted

## Context

Two of the MVP's three position operations multiply a position by a factor:
the affine transformation through two anchor points, and frame-rate
conversion. Neither can be expressed with the phase 1 types, which only offer
adding a duration and converting to a frame.

Frame-rate conversion nevertheless seems to reduce to what exists:
`fromFrame(toFrame(t, input), output)`. That is wrong, and the phase 1 spec
already says so without the consequence being drawn — `ms → frame → ms` is
**not** the identity. That path quantises every position onto the input frame
grid. Measured from 25 to 24000/1001:

| position | via frames | exact | gap |
| -------: | ---------: | ----: | --: |
| 1010 ms | 1043 | 1053 | 10 ms |
| 1020 ms | 1084 | 1064 | 20 ms |
| 3600017 ms | 3753750 | 3753768 | 18 ms |

Largest gap found over 200,000 positions: **21 ms**, that is half a frame.
Gaupol does not make this mistake: it computes in seconds and rounds only
once. The conversion benchmark delivered by issue #9 does make it, and so
measures an operation that will not be implemented.

## Decision

Positions are scaled by an **exact rational**, `Ratio`, with **a single
rounding**, the phase 1 one: to the nearest integer, halves away from zero.

```cpp
Timestamp Timestamp::scaledBy(Ratio) const;
Duration  Duration::scaledBy(Ratio) const;
```

A named method and not an operator: scaling a position is a rounding decision,
and [0006](0006-positions-en-millisecondes.md) wants none of them to be taken
implicitly.

Frame-rate conversion is written `t × (input / output)`, without going through
frames.

The transformation is written **`(t − x₁) × r + y₁`** and not
`r × t + constant`, which is Gaupol's form. Besides the single rounding, this
form makes the two anchor points land **exactly** on the requested positions:
`(x₂ − x₁) × r` equals `y₂ − y₁` exactly, by construction of `r`. Gaupol's
form does not guarantee it.

## Alternatives dismissed

- **The round trip through frames.** Attractive because it reuses what exists,
  and wrong for the reason measured above. This is the alternative that had to
  be named: it would have slipped into the code unnoticed.
- **A floating-point factor**, which is what Gaupol does. Dismissed for the
  reason that already dismissed a `double` frame rate in
  [0006](0006-positions-en-millisecondes.md): `25 / (24000/1001)` is not
  exactly representable, and the error propagates to every position. A
  rational removes the question.
- **Rounding the coefficient and the constant separately**, the most direct
  form of the transformation. Dismissed: two roundings where one suffices, and
  the anchor points no longer land on their target.

## Consequences

Every computed position is within a millisecond of the exact result, not
within half a frame.

Intermediate computations fit in a `std::int64_t`: a plausible position does
not exceed `3.6 × 10⁸` ms, and a reduced rational keeps a numerator of the same
order — the product stays far from the bounds.

`Ratio` joins the types of the `time` module. It is a fifth type where
[0006](0006-positions-en-millisecondes.md) named three and
[0011](0011-numero-d-image-en-type-fort.md) a fourth; it does not represent a
quantity but a **factor**, which justifies it being neither a position nor a
duration.

Operations that round cannot be inverted by a second computation:
transformation and conversion must keep the previous positions, whereas
shifting makes do with a `Duration`. That is what
[0010](0010-annulation-par-commandes.md) calls capturing the bare minimum —
which is not the same thing for every operation.

Undoing this decision would silently change every computed position: no test
would see the difference without being rewritten, and files already saved
would be inconsistent with new ones. The trigger for a review would be a
measurement showing that the cost of the rational weighs on a real file — the
phase 2 benchmarks are there to tell.
