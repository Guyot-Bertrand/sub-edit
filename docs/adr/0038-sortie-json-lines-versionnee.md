# 0038 — Offer output a script can read: JSON Lines, versioned

**Date:** 2026-10-01
**Status:** accepted

Decided while scoping phase 13, issue
[#542](https://github.com/Guyot-Bertrand/sub-edit/issues/542). Settles the open point the
[phase 3 spec](../specs/03-cli.md) had written down: "shape of a machine-readable output —
phase 13, when a real caller needs it".

## Context

The command line writes two things to two outputs: the **result** to standard output —
today, only the `inspect` report, in blocks of text —, and **everything else** to the
error stream, at the four narration levels. The subcommands that rewrite write nothing to
standard output: their result *is* the file.

**No structured output exists**, and phase 3 had dismissed it with a reason worth
keeping: "nothing would consume it today; its shape must be dictated by a real caller".
The caller has come, and it is precise: a script that processes hundreds of files and
must know, **per file**, whether it succeeded, what it counted (subtitles changed,
constraints sacrificed, patterns abandoned), what reading had to decide, and where the
result went. The exit code only says `0`, `2` or `3` for the whole batch; re-reading an
English sentence to extract a number from it is exactly what a "usable" output must
avoid, and what every wording fix would silently break.

The repository already has the tool that makes the promise **provable**: `MatchesFile`
([#543](https://github.com/Guyot-Bertrand/sub-edit/issues/543)), a byte comparator
against a versioned expected file, which shows the first diverging line.

## Clarification: `--list-encodings` is an exception to the envelope

Written by [#564](https://github.com/Guyot-Bertrand/sub-edit/issues/564). `--list-encodings` has no
input file, and its result is a set, not the account of a file. **With `--format json`, it writes
one object per encoding** — `{"schema":1,"command":"list-encodings","name":"UTF-8"}` —, which keeps
the property that matters: one object per entry, one per line, a stream that can be cut and
concatenated like the others. **It carries neither `file` nor `ok`**: the envelope of file objects
(`schema`, `command`, `file`, `ok`) makes no sense for an encoding name, and inventing one for it
would have meant writing `"file": null` so that the shape looked uniform. `schema` and `command` are
there, because each line must remain readable on its own.

## Decision

**`--format json`, a global option, opt-in.** `text` remains the default and does not
change. With `json`, standard output carries **one JSON object per input file, as JSON
Lines** (one compact object per line, UTF-8 without a byte order mark, `\n` line
endings), in input order. **The subcommands that rewrite write there too** — which is
what changes their phase 3 contract —; their file remains their result, and the object
is its account. Narration (`-q`, `-v`, `-vv`, `-vvv`) stays on the error stream, **as
text, unchanged**.

### The shape

One object per input, **never zero, never two**, whether the input succeeded or not:

```json
{"schema":1,"command":"shift","file":"a.vtt","ok":true,"dry_run":false,"destination":"out/a.vtt","counts":{"subtitles":1356,"shifted_by_ms":-7001},"warnings":[]}
{"schema":1,"command":"shift","file":"b.vtt","ok":false,"error":{"kind":"unknown-format","message":"b.vtt: is in no format this tool knows"}}
```

| Key | Content | Present |
| :-- | :------ | :------ |
| `schema` | integer, the version of this shape | always |
| `command` | the subcommand's name | always |
| `file` | the path **as given** on the command line — or as the walk of a directory composed it —, never normalized | always |
| `ok` | boolean | always |
| `error` | `{"kind", "message"}` | if `ok` is false |
| `destination` | the path written, or `null` under `--dry-run` | if `ok` is true, on a subcommand that writes |
| `dry_run` | boolean | if `ok` is true, on a subcommand that writes |
| `counts` | an object of **integers**, specific to the subcommand | if `ok` is true |
| `warnings` | an array of objects `{"kind", "line"?, "detail"?, "settled"?}` | if `ok` is true |

**The keys specific to each subcommand** — those of `counts`, and for `inspect` the
description of the file — are **written in the manual**, as one table per subcommand
and an example replayed by `manual-check`. The spec does not copy the list: a second
list is a second thing to maintain. The three choices that commit us, however, are
here:

- **No floating-point numbers.** A position or a duration is an **integer number of
  milliseconds** (`…_ms`), like the model (ADR 0006); a count is an integer; a frame rate
  is a **string** (`"23.976"`) because it is exact, a rational (ADR 0013) that floating
  point cannot hold; a grid concentration is an integer in **thousandths**. Comparing two
  outputs byte for byte then depends on no rounding.
- **Reading diagnostics always go into `warnings`**, at every narration level. Text puts
  them at level 3 because it is meant to be read; a script always wants them, and an
  object whose content changed with `-q` would not be usable. **Verbosity has no effect
  on standard output under `json`.**
- **A usage error writes no object**: it is detected before any processing
  (`CLI-USAGE-03`), so there is no input to attach it to. It goes to the error stream, as
  text, with code `1`, and standard output stays empty.

### Versioning

`schema` is `1`. It is **on every object**: a stream gets cut, concatenated, filtered,
and each line must remain readable on its own.

**What is promised stable as long as `schema` is `1`**:

- the names of the keys, their type and their unit;
- the meaning of `ok`, `file`, `destination`;
- **one object per input, in input order**; the order of the elements of an array when
  it has a meaning (warnings in reading order, anomalies by subtitle number);
- the existing `kind` **identifiers** — of errors and warnings;
- determinism: same inputs, same arguments, same bytes.

**What is not promised**:

- **the text of messages** (`message`, `detail`): it is English for a human, it lives in
  `core/wording/` and gets corrected. A script that compares it is a script broken in
  advance;
- the **order of keys** in an object — it is fixed, because determinism requires it, but
  JSON does not say so and a reader must not rely on it;
- the presence of **keys not yet added**.

**A compatible change does not change `schema`**: adding a key, adding a `kind`
identifier. **A reader must therefore ignore an unknown key and treat an unknown `kind`
as "other"**; this is written in the manual, at the head of the section. **An
incompatible change increments `schema`**: removing or renaming a key, changing a type
or a unit, removing an identifier, changing what a value means. The increment is a
break, announced in the CHANGELOG and the manual; `--format json` always writes the
current version, without a selector (see "What would justify reopening").

### The proof

**The expected results are versioned and written by hand** —
`src/test/data/attendus/json/`, one `.jsonl` file per case —, **read by an end-to-end
test that compares them through `MatchesFile`**. Never the program's output read back by
the program: that is the lesson of #338, and it is the raison d'être of #543. A format
change then shows up as a **fixture diff**, never as a test that adapts on its own.

**A Python script** (standard library, `json`) checks that **every line of every
expected file is valid JSON, carries the envelope, and contains no floating-point
number**; `check-architecture.sh` calls it. The C++ has no JSON reader — it does not need
one, it only writes — and the script is the second pair of eyes, independent of the
writer.

## Alternatives dismissed

- **A single JSON array** — `[ {…}, {…} ]`. It can only be read at the end: a process
  interrupted at the thousandth file leaves nothing readable, and the whole batch
  accumulates in memory before the first byte. JSON Lines can be read, cut and filtered
  (`grep`, `head`, `jq -c`) while the batch runs.
- **A final summary object** (`{"processed":…,"failed":…}`). A line of another shape in
  the middle of a homogeneous stream forces every reader to tell it apart, and the
  summary already exists: the exit code (`0`, `2`, `3`), and the level 1 text on the
  error stream. **"One object per input, never zero, never two"** is a property a test
  can state; a summary would have made it false.
- **TSV or CSV.** Warnings and per-constraint counts are nested, and the texts of
  `correct --dry-run` carry line breaks, tabs and quotes that would have to be escaped
  anyway, in a format every tool escapes differently.
- **Stabilizing the text lines** (`key: value`) — which is what `inspect` already writes.
  That would commit every sentence never to change again, and would forbid precisely
  what `wording/` needs to do: fix a label. Text remains meant for a human, and nothing
  is promised to it.
- **`--json`, a flag.** `--format` reads better (`text`, `json`) and leaves room for
  another format without a new option. **No other is promised.**
- **Writing a JSON Schema file.** It would be a second description to maintain, next to
  the manual's tables and the expected files; it would be justified the day a tooled
  consumer asks for it.
- **A JSON library** (nlohmann, RapidJSON) — one more dependency
  ([ADR 0004](0004-gestion-des-dependances.md), paid for in the `.deb`, the `.rpm` and CI)
  to write about a hundred lines. A minimal writer, conforming to RFC 8259, is enough
  since there is **nothing to read**.
- **A schema version in the option's value** (`--format json:1`). Premature: there is no
  second version. The selector gets added the day an increment requires it, without
  breaking anything.

## Consequences

**A structured result replaces the sentence.** `cli::Operation` currently returns a
string ("4000 subtitles shifted by 2.999 s"), and a count cannot be recovered from a
sentence. Each operation now returns **its counts**, and the level 1 sentence is written
**from those same counts**, through `core/wording/` when the window says the same thing:
the text and the JSON can no longer diverge, because they come from the same object.
That is the discipline #541 had set down for every new subcommand, held here by
construction.

**A JSON writer in `subedit_cli`**, without dependency, with its own tests: escaping of
quotes, backslashes and control characters; other characters — accents, characters
outside the basic plane — come out as they are, in UTF-8.

**Paths that are not UTF-8** — possible on Linux — cannot be represented in JSON. They
are written with U+FFFD in place of the invalid bytes, and the object carries a
`path-not-utf8` warning: fidelity is not promised for them, and that is stated.

**Cost of undoing**: high, and that is why this ADR exists. A script written against
`schema` 1 is in the hands of someone the repository does not see. Adding keys costs
nothing; removing one costs an increment and the patience of whoever had read it.

**What would justify reopening**: a consumer that needs to read the old shape after an
increment (the version selector); one more format a real caller would ask for; or proof
that the order of objects is not enough for a parallel batch, if the batch becomes one
(ADR 0039).

## Implementation (#556)

Reviewed at implementation; **no decision is changed**, six clarifications the ADR did
not state — the shape gains keys, it loses none:

- **The `kind` identifiers of errors** are those the manual enumerates: `not-found`,
  `permission-denied`, `io` (the system); `unknown-format`, `no-subtitle-found`,
  `undecodable` (reading); `unencodable` (writing); `no-byte-order-mark`,
  `no-frame-rate` (`convert`); `before-the-origin`, `beyond-the-end`, `no-transform`,
  `no-grid` (the operations); `refused` for any other refusal.
- **`settled` is written only when it is true** — reading decided something —, just as
  `line` is written only for a diagnostic that has a line and `detail` when it has
  something to say.
- **`concentration_permille` ranges from 0 to 1000**: the core's concentration ranges
  from 0 to 100, and the thousandth is that of the whole — a perfect grid is 1000.
- **A failure carries `warnings` in one case only**: the path is not UTF-8. The
  decision's table reserves `warnings` for successes; the `path-not-utf8` warning is the
  only extra thing a failure has to say, and keeping it quiet would have written an
  object that does not describe itself.
- **`inspect` describes the file** with keys the manual enumerates: `format` (the
  identifier of `convert --to`), `encoding`, `line_endings`, `subtitles`, `span_ms`,
  `frame_rate` or `grid` (one of the two is `null`), `anomalies`.
- **The text and the JSON come from the same object**: `cli::OperationResult` carries the
  counts and the sentence, which the operation builds from the same values. The sentence
  has not changed by a word — that is the criterion — but it is no longer written by
  `core/wording/`: the window's and the command line's do not overlap.
