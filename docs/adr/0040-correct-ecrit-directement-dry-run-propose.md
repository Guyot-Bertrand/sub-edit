# 0040 — Write directly, and propose through `--dry-run`

**Date:** 2026-10-01
**Status:** accepted

Decided while scoping phase 13, issue
[#542](https://github.com/Guyot-Bertrand/sub-edit/issues/542). Answers the question the
[phase 12 spec](../specs/12-correction.md) (D8) left to this phase: *does a mode that
writes out the proposed changes without applying them replace the confirmation?*

## Context

The window's `Correct Texts…` assistant first computes, without touching any project
(`proposeCorrections`), **then** shows each changed text alongside its original, and
applies only what the user accepted (`applyCorrections`). It is this confirmation page
that makes automatic correction **a proposal rather than a fait accompli** — and it is
the reason we tolerate a common-error pattern being sometimes overzealous.

The command line has no page: it has neither a shared screen nor questions along the
way, and its callers are scripts. Yet it has the same need, **to see before writing** —
and, unlike the window, it has a file that can be left untouched. The subcommands that
rewrite follow a phase 3 rule: **a destination given is a destination written**.

## Decision

**`correct` writes directly**, like every subcommand that rewrites: give a destination,
the file is written. **There is neither confirmation nor prompt.**

**`--dry-run` is the proposal.** It is accepted by **every subcommand that writes** and
means the same thing everywhere: *read, compute, report, write no file*.

- **It does not require a destination** — that is its whole point: one adds `--dry-run`
  to the line one was about to run, or runs it without a destination. **A destination
  given is still checked as usual** (collisions, overwritten inputs, ADR 0039): a command
  line that would be refused without `--dry-run` is refused with it.
- **On the subcommands that change texts** — `correct`, `replace`, `case`, `italics`,
  `dialogue-dashes`, `hearing-impaired` —, **standard output carries the proposed
  changes: for each changed subtitle, its number, the text before and the text after**,
  or "removed" when the subtitle would be. It is the same list the confirmation page
  shows; it comes from the same core function.
- **On the others** (`shift`, `adjust`, `snap`…), it carries what the output of a real
  run would carry — the counts —, with the `dry_run` mark and `destination: null` in
  JSON, and a narration line that does not lie: `… (dry run, nothing written)` in place
  of `-> <destination>`.
- **The exit code is that of a real run**: `0` when everything was computed, `2` or `3`
  for reading failures, `1` for usage. **The existence of changes does not change the
  code**, unlike `diff`: the four codes say whether the tool succeeded, not whether the
  file conforms, and the count is in the output.
- **Under `--format json`**, the file's object carries the changes: a `changes` array of
  `{"subtitle", "document", "before", "after"}` — `after` is `null` for a removal. The
  exact shape is that of ADR 0038, and the manual holds it.

**Applying means rerunning without `--dry-run`.** The computation is a function of the
file and the arguments, **deterministic**: what `--dry-run` showed is what the next run
will write, as long as the file has not changed.

## Alternatives dismissed

- **An interactive write-only-if-confirmed** (`Apply? [y/N]`). It breaks every caller
  that is not a human in front of a terminal, and those are the phase's raison d'être.
- **A changes file that `--apply` replays** (a patch, a `.patch`, an intermediate
  state). That is state between two invocations: the file may have changed, the patch is
  stale, and one has to decide what happens then. Rerunning the computation,
  deterministic and cheap — [ADR 0036](0036-icu-pour-les-motifs-de-correction.md)
  measures the order of magnitude for a whole film: from a few hundredths to a tenth of a
  second —, does not have that problem.
- **A unified diff on standard output.** It does not carry the subtitle number, which
  nothing in a file yields with certainty, it cuts a multiline text wherever it likes,
  and it ties the output to the file's format. The number, the before and the after are
  what a human reads and what a script compares.
- **Inverting: propose by default, write with `--write`.** Consistent with caution, and
  contrary to the phase 3 rule every other subcommand holds: `shift --output x` writes. A
  single subcommand that opts out is the one a script forgets is the exception.
- **`--yes` / `--force`** as a default confirmation: there is no question to answer.
- **`--dry-run` for `correct` alone.** It would deliver what is asked here, and would
  force `replace`, which has the same need — seeing what will be replaced before doing
  it —, to invent it separately, in two grammars.

## Consequences

**A single mechanism, in `rewriteAll`**: the file is read, the operation computes, and
writing is the only step `--dry-run` skips. The subcommands write no line of their own
for it; those that change texts additionally provide the list of changes.

**The list of changes is a result of the operation**, just like its counts (ADR 0038):
text and JSON read it from the same place. It is computed only when `--dry-run` or
`--format json` asks for it — a batch of thousands of subtitles does not build it for
nothing.

**No memory between the two calls**: nothing guarantees that a pattern file dropped by
the user in between has not changed. That is the normal order of things.

**What would justify reopening**: a caller who wants to apply *part* of the proposed
changes (the core can compose a subset: `applyCorrections` already takes one), which
would require a changes format that can be read back, hence a second schema.

## Implementation clarifications

Set by [#557](https://github.com/Guyot-Bertrand/sub-edit/issues/557), which builds the
mechanism; they do not change the decision.

- **The text form of the changes** — open point 4 of the spec — is one block per
  subtitle: `<path>: subtitle N`, then each line of the text before prefixed with `- `,
  each line of the text after prefixed with `+ `. A removal writes it on the first line,
  `(removed)`, and has no `+` lines. The path at the head of the block makes a batch an
  output that can be filtered by file.
- **`destination` is `null` in JSON for every `--dry-run`, even when a destination was
  given**: it was checked, it was not written, and the object says what took place.
- **The dry run produces the bytes and does not hand them to the system**: a character
  the encoding cannot write makes a `--dry-run` fail like a real run, which makes "the
  code is that of a real run" true. Only a refusal by the disk escapes it.
- **`changes` appears in the object of a text subcommand for a real run under
  `--format json` too**, empty when nothing changes; the others do not have the key.
