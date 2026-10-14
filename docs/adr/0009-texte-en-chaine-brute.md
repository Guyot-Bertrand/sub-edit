# 0009 — Store text as a raw string carrying the format's markup

**Date:** 2026-08-05
**Status:** accepted

## Context

A subtitle's text carries formatting markup, specific to each format: `<i>`,
`<b>`, `<u>`, `<font color="#RRGGBB">` for SubRip; the same without `<font>` for
WebVTT, plus `<c.class>`, `<v Speaker>`, `<ruby>` and inline timestamps;
arbitrary overrides `{\an8}`, `{\pos(x,y)}`, `{\t(...)}` for ASS in phase 9.

Two representations were conceivable: keep the string as it is, or decode it
into a structured model — text plus style ranges.

The analysis of Gaupol settled the question in practice. Its internal pivot
format exists **only for the duration of a conversion between formats**;
nothing is stored in that form. And that decoding is destructive:
`markups/webvtt.py` states that tags such as `<c>` "cannot be supported", and
its `_post_decode` removes **every unrecognised tag**:

```python
text = self._substitute(text, r"</?[^biu]>", "")
text = self._substitute(text, r"</?[^/>]{2,}>", "")
```

So Gaupol only loses `<v Marie>` or `<c.loud>` when it converts. When a `.vtt`
is opened, edited and saved, nothing is decoded and nothing is lost. **Its
fidelity comes precisely from storing a raw string.**

## Decision

Text is stored **as it appears in the file**, markup included. No decoding takes
place on opening or on saving.

Operations that transform the text go through a **markup-aware parser**: it
removes the tags, remembers their positions, applies the transformation to the
bare text, then reinserts them.

A structured model is still used, as in Gaupol, **only as a pivot during a
conversion between formats**, where the loss is unavoidable and accepted.

## Alternatives dismissed

- **A permanent structured model — text and style ranges.** Semantically much
  cleaner, and text operations become trivial in it. Dismissed because it
  imposes the destructive decoding **all the time**: opening a `.vtt` containing
  `<v Marie>` and saving it would lose those tags. On one of the two priority
  formats, from the MVP onwards. The loss would not be deferred to phase 9; it
  would start right away.
- **A structured model with opaque fragments** — tags that are not understood
  would be kept verbatim in fragments treated as atomic. This variant preserves
  fidelity and keeps clean semantics. Dismissed for a deliberate reason of
  simplicity: it requires designing, from phase 1, a fragment model whose
  soundness only phase 9 will test, whereas the raw string is faithful by
  construction and with no prior design. It remains the best way out should the
  markup-aware parser prove untenable.

## Consequences

**Round-trip fidelity is obtained by construction**, for every format, including
for tags we do not know.

In return, **no text operation is trivial**. Each must go through the parser, or
it will break the markup: applying `\[.*?\]` directly to the text to remove a
hearing-impaired annotation would erase an opening tag located inside the
brackets, leaving its closing tag orphaned. It is a real and recurring cost, paid
on every operation.

The parser is therefore a first-rank piece of infrastructure. It is written
once, in phase 4 where it becomes necessary, and reused by phases 10 and 12. Its
design deserves the same care as the data model, and its edge cases — tags
overlapping a match, tags enclosing every line, insertion at a tag boundary —
call for dedicated tests.

The model cannot guarantee that markup is well formed: an unclosed `<i>` stays
as it is. This is consistent with
[0008](0008-lecture-au-mieux-avec-diagnostics.md) — report it with a diagnostic,
do not repair it unasked.
