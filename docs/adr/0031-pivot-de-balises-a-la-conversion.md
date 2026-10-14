# 0031 — A markup pivot, for conversion only

**Date:** 2026-09-07
**Status:** accepted

Decided while scoping phase 9, issue #337.

## Context

[ADR 0009](0009-texte-en-chaine-brute.md) was written for this very phase.
It names ASS, and it promises:

> A structured model is still used, as in Gaupol, **only as a pivot during a
> conversion between formats**, where the loss is unavoidable and accepted.

**That pivot does not exist.** Nothing had asked for it: SubRip and WebVTT both
write `<i>…</i>`, so a conversion between them never had to translate anything.
The raw string went through as is, and that was right.

Nine formats make five vocabularies:

| Vocabulary | Formats | Italics are written |
| :--------- | :------ | :------------------ |
| HTML | SubRip, WebVTT, SubViewer 2 | `<i>…</i>` |
| SSA | Sub Station Alpha, Advanced SSA | `{\i1}…{\i0}` |
| MicroDVD | MicroDVD | `{Y:i}` up to the end of the subtitle |
| MPL2 | MPL2 | `/` at the start of the line |
| none | TMPlayer, LRC | nowhere |

Without translation, an SSA converted to SubRip gives `{\i1}He says nothing.{\i0}`
— text the user sees, in a file that will never know how to interpret it.
**That is not a loss, it is a smear**: the loss would have been to give
`He says nothing.` without italics, which is readable.

## Decision

**The pivot is built, and it lives only for the duration of a conversion.** Text
that has been read remains a raw string; opening and saving a file in its own
format neither decodes nor touches it. ADR 0009 is neither superseded nor
amended: it is honoured.

**Its scope is Gaupol's** — bold, italic, underline, colour, font, size.
Nothing else is translated.

**What the target format cannot write is removed, and the conversion says so.**
It is the only point where we add something to Gaupol, which removes silently.

## Alternatives dismissed

### Translating nothing, and letting the tags through

That is what a naive conversion would do, and the result is above: `{\i1}`
visible in a `.srt`. The file stays valid and the text becomes wrong. Dismissed
without hesitation.

### Removing all tags on conversion

Simple, predictable, and it loses what could have been kept. Converting an SSA
to SubRip would lose italics although SubRip can write them — a gratuitous loss,
which the measurement of #339 would count as such.

### A permanent structured model

Dismissed in phase 1, for a reason that has not moved: decoding is destructive,
and making it permanent would lose on opening what is only lost on conversion.
ADR 0009 keeps the record of it, and its "opaque fragments" variant remains the
way out it describes — this decision does not close it.

### Translating layout as well

`{\pos(x,y)}`, `{\an8}`, `{\t(...)}`, SSA named styles, `<ruby>` and WebVTT's
inner timestamps. Each would require a layout model that nothing else in the
project calls for, and most have no equivalent anywhere. They go through a round
trip in their own format intact, and disappear on conversion — saying so.

## Consequences

**A conversion is no longer a rewrite, it is a translation**, and its cost in
tests is that of a translation: each pair of vocabularies has its cases. Five
vocabularies make twenty ordered pairs, eight of which go to or from "none" and
are therefore trivial.

**Loss becomes measurable, and it already is** — `docs/mesures/conversion.md`
counts the intact round trips before the pivot exists, and the number will move
when it does. That is what the tool of #339 was written to see.

**The markup-aware parser of ADR 0009 is still not written**, and this pivot does
not write it. They are two different pieces: the pivot translates one vocabulary
into another, the parser protects the tags while an operation transforms the
text around them. Phase 4 did without the latter
([0017](0017-analyseur-de-mentions-ecrit-a-la-main.md)); phase 10 will find it
on its way. The pivot will give it a common vocabulary to recognise, which is a
down payment, not the piece.
