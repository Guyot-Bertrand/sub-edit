# 0018 — Separate the vocabulary of formats from their operations

**Date:** 2026-08-17
**Status:** accepted

## Context

The phase 5 window opens a file, edits it, and saves it. To rewrite it, it must
remember its format. Yet `SourceFile` — whose comment announces "what the file
a project comes from looked like" — keeps the path, the line endings, the BOM
and the header, **and nothing else**. The format lives in `ReadResult`, which
the CLI passes from function to function and which a `Project` does not keep.

Adding `SubtitleFormat` to `SourceFile` creates the first `model/ → format/`
edge. The core's graph is acyclic today — `time/` and `text/` depend on
nothing, `model/ → time/`, `command/ → model/`, `edit/ → command, model, time,
text`, `format/ → model, time` — and this edge would be its first cycle between
directories.

**Except that the cycle already exists, and the file names hide it.**
`model/format_extras.hpp` defines `SubRipExtras` and `WebVttExtras`;
`model/subtitle.hpp` includes them, so a `Subtitle` carries data named after
two formats. `model/source_file.hpp` carries a `header` field documented as
"the WebVTT header, empty for formats that have none". The model already knows
the formats: it merely avoids naming their enumeration. The boundary is clean
because the format-specific types were filed on the `model/` side, not because
the dependency is absent.

Symmetrically, `format/` hosts things that do not even know the word
"subtitle": a file-system abstraction and its atomic write, a splitting of text
into lines, a UTF-8 validation, a detection of BOM and line endings.

The deciding criterion is therefore not "is it related to reading?" but
**"does it know a subtitle format?"**

## Decision

`format/` contains only the **operations** that translate bytes into the model
and back — readers, writers, detection — and the types those operations
produce.

The **vocabulary** that describes what a document *is* lives in `model/`: its
format, its format-specific data, its file shape.

What knows no subtitle leaves both: `io/` for disk access, `text/` for
operations on strings.

| What moves | To | Because |
| :--------- | :- | :------ |
| `SubtitleFormat`, the enumeration alone | `model/` | a document *has* a format as it has line endings |
| `detectFormat` | stays in `format/` | it is an operation, not a property |
| `lines` — `splitLines`, `trimmedBlanks`, `isBlank` | `text/` | splitting text is not reading a subtitle |
| `encoding` — `isValidUtf8`, `hasUtf8Bom`, `scanNewlines` | `text/` | nor is validating bytes |
| `file_system`, `real_file_system`, `in_memory_file_system`, `atomic_write` | `io/` | opening and writing a file, whatever it is |

`SourceFile` then gains its format without any new edge appearing.

### The hard case: diagnostics

`DiagnosticKind` mixes ten categories, three of which — `EndBeforeStart`,
`OverlappingSubtitles`, `OutOfOrder` — do not describe what a reading
encountered but **what a document is**. `Project::outOfOrder()` in fact
recomputes the third.

They separate, and the locator says why:

| | Locator | Lives as long as |
| :--- | :--- | :--- |
| `format/diagnostic.hpp` — what a reading encountered | a **line number** | the file has not been edited |
| `analysis/anomaly.hpp` — what a document is | a **`SubtitleIndex`** | the document exists |

A line exists only at reading time; an index survives editing. So the
following stay on the reading side: `IgnoredLine`, `MalformedTimestamp`,
`MissingNumbering`, `InconsistentNumbering`, `TextBeforeAnyTimestamp`,
`UnknownBlock`, `MixedNewlines`.

## Alternatives dismissed

- **Leaving `SubtitleFormat` in `format/` and having the window carry the
  format.** It would keep beside its `Session` a field that `SourceFile` is
  precisely made for, and the question would come back with the second
  consumer — the phase 6 preview, phase 11 multi-project. A workaround that
  repeats is a decision that was not taken.

- **Moving `FormatExtras` to `format/`,** which the rule "classes that
  characterise formats go in `format/`" suggests at first sight. Impossible
  without reversing the graph: `model/subtitle.hpp` includes `FormatExtras`, so
  `model/ → format/`. The cycle reappears from the other end. One of the two
  directions has to give, and it is this one: **a reader needs the model, the
  model never needs a reader.**

- **Splitting `Diagnostic` while keeping the line number on both sides.** An
  edited document no longer has lines; the value would be right at reading time
  and wrong afterwards, which is worse than absent.

- **Not reorganising anything.** Disk access, UTF-8 validation and line
  splitting would stay under a name that promises subtitle formats, and
  `readSubtitles` would be looked for in a directory that also contains an
  atomic write.

## Consequences

**Made easy.** `SourceFile` carries its format, so saving is a complete
operation with no state kept on the side. Upcoming phases find directories that
bear their name: the phase 8 encodings arrive in `text/`, the phase 9
additional formats in `format/`. A document's anomalies become computable
**after an edit**, which Gaupol cannot do — and that is exactly what a table
that highlights rows needs.

**Made hard.** Nothing, structurally. The move touches 38 files and made
`tidy-scope.sh` fall back on the full analysis: a full-price gate, paid once.
(That script no longer exists since #269; the analysis now follows the real
dependencies.)

**What it costs to undo.** A rename, mechanical. The diagnostics split, on the
other hand, does not undo for free: it changes what `inspect` prints — a
subtitle number where it gave a line number — and therefore the manual, the
requirements and the end-to-end tests. It is a lower-quality locator for
whoever opens the file in a text editor, accepted because the index is the only
one that survives an edit.

**Trigger for a review.** A format whose *reading* would require a model type
nothing else uses — ASS named styles, in phase 9. If the model has to host a
type whose only client is a reader, the boundary drawn here is in the wrong
place.
