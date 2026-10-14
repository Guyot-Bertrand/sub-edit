# 0039 — A safe batch: collisions refused, directory created, tree preserved

**Date:** 2026-10-01
**Status:** accepted

Decided while scoping phase 13, issue
[#542](https://github.com/Guyot-Bertrand/sub-edit/issues/542), on the findings of
[#544](https://github.com/Guyot-Bertrand/sub-edit/issues/544).

## Context

The phase 3 batch is a sequential loop over given paths: each one is read, processed and
written independently, failures are named, the exit code is global (`0`, `2`, `3`).
**That was enough as long as the batch was a harness**; it is no longer enough for what a
script does to thousands of files. The tests of #544 wrote down three behaviours that had
never been decided, and that the manual (`docs/manual/subedit-cli/invocation.md`) admits
under "three things the tool does not do":

- **two inputs with the same base name** — `a/film.srt`, `b/film.srt` — **write to the
  same place** with `--output-dir`: code `0`, each input announces its destination, the
  second overwrites the first, without a word;
- **the `--output-dir` directory is not created**: if it is missing, every input fails,
  and the label says "`cannot be read`" for a **write** — `reasonOf(FileErrorKind::Io)` is
  the reading sentence, which writing borrows — whereas the manual announces "`cannot be
  opened: permission denied`";
- **a destination that exists is overwritten**, without question or option, by atomic
  write.

A fourth, which #544 did not see and the scoping analysis found: `--output-dir .` with
inputs that are in `.` **overwrites the inputs themselves**, without `--in-place`. The
rule "nothing is written without an explicit destination, and overwriting one's input is
an act one names" (spec 03) does not hold there.

And the phase asks for what the loop cannot do: take **directories** as input.

## Decision

**Four safety rules, and one extension.**

1. **Two inputs that end up at the same destination are refused before anything is
   written.** Usage error, code `1`, **no file written** — the rule of `CLI-USAGE-03`: a
   usage error never leaves a half-written batch. The message names the destination
   **and both inputs**. The comparison is on the **final** destination — extension
   included, `convert` changes it — and is done on normalized paths.
2. **A destination that is one of the inputs is refused without `--in-place`**, same
   rule, same code, same reasons; `std::filesystem::equivalent` decides when the file
   exists.
3. **The output directory is created** if it is missing, parents included, **after** the
   validation of the command line and **before** the first file processed; never under
   `--dry-run`. The directory `--output FILE` writes into follows the same rule. If the
   creation fails, **it is said once**, no file is read, and the code is `2`.
4. **A destination that exists is overwritten**, by atomic write — the previous content
   is lost, the file is never half-written. **This is not a defect we keep: it is the
   rule, and the manual states it** in the destination section, at the top and no longer
   under a heading of caveats.
5. **Directories as input, with `--recursive` (`-r`).**

   - A directory given **without** `--recursive` is a usage error (code `1`):
     `<directory>: is a directory: use --recursive`. Refusing it is better than
     processing nothing of it and returning `0`.
   - With `--recursive`, the walk is **recursive, deterministic and unsurprising**: the
     entries of each directory are sorted by name (byte comparison), depth first;
     **symbolic links are not followed** (no loop); **hidden files are ignored**.
   - **Only files whose extension is that of a known format are kept**: `.srt`, `.vtt`,
     `.ssa`, `.ass`, `.lrc`, `.sub`. **`.txt` is excluded from the walk**: it denotes MPL2
     and TMPlayer, but above all every `README.txt` in the world, and a batch that failed
     on each of them would be unusable. **A file named on the command line is never
     filtered**: naming it means wanting it. What was left aside is counted, and said at
     level 2.
   - **The tree is preserved under `--output-dir`**: the path of a file **relative to the
     given directory** is reproduced under the output directory (`films/a/x.srt`, given
     `films/`, is written `out/a/x.srt`). The name of the given directory is not part of
     it — the semantics of `rsync` with a trailing slash, not those of `cp -r` —: the user
     names the destination, and adds the level they want. **With two directories, rule 1
     decides** what overlaps.
   - **The output directory, if it is inside the walked tree, is excluded from the
     walk**: a second run does not re-read what the first one wrote.
   - The list of inputs is **established in full before the first write** — which is
     what makes rules 1 and 2 checkable before anything, and the walk independent of what
     writing changes.
   - `--in-place` with `--recursive` rewrites each kept file in place.
6. **The label of a write that fails says "written"**, never "read" nor "opened":
   `<destination>: cannot be written: permission denied`, or `: cannot be written` for
   the rest. Reading and writing no longer share `reasonOf(FileErrorKind)`.

**What does not change**: each file is processed independently, the failure of one does
not stop the others, failures are named, the exit codes remain the four of phase 3. The
walk produces **a list of paths**, which the rest of the batch consumes as before:
`--format json` (ADR 0038) finds its "one object per input" there.

## Alternatives dismissed

- **Renaming the second input** (`film-2.srt`). The name of a written file becomes a
  function of the order of inputs: a script that reruns the same batch in another order
  writes something else, and whoever looks for `film.srt` does not know which one they
  found. A refusal costs a line; a silent renaming costs an investigation.
- **Letting the first through, failing the second** (code `3`). A half-written batch is
  the worst case, and the phase 3 rule exists to prevent it. The collision is known
  *before* writing: saying so then is possible, hence required.
- **Keeping "the last one wins"** and writing it in the manual. That is today's state; it
  loses a file without anything saying so.
- **Refusing a missing output directory.** More cautious against a typo, and it was the
  leaning of the first finding of #544. Dismissed: the user **names** this directory, it
  is the product of the command, and asking for a prior `mkdir` is the step every script
  adds. The typo creates one more directory, visible and harmless, where overwriting a
  file is not.
- **A `--no-clobber` option.** Legitimate, and not requested: the collision rule and
  atomicity cover the two kinds of damage we have seen. A candidate if a caller asks for
  it.
- **Relying on the shell** (`**/*.srt`, `find … -exec`). `globstar` is disabled by default
  in bash, `**` does not exist everywhere, the list sometimes exceeds the length of a
  command line, and above all **the shell does not preserve the tree under an output
  directory**: that is half the need.
- **A name pattern (`--include '*.fr.srt'`)**. Useful, and more grammar: the known
  extensions settle the common case. A candidate.
- **Following symbolic links.** A tree loop is a defect one finds in production; not
  following them is one you read about in the manual.
- **A parallel batch.** The loop is sequential, its cost is the sum of its files, and the
  output stays deterministic without arbitrating anything. The measurement that would
  tell its worth is born with the walk (batch issue, bench of #541); parallelism is
  retained only if it shows it — and it would require reopening the order of objects of
  ADR 0038.
- **Including the given directory's name under `--output-dir`** (`cp -r`). More familiar,
  but then one cannot write "my `films/` tree into `out/` without an extra level"; one can
  always write `--output-dir out/films`.

## Consequences

**The behaviour changes for anyone who relied on overwriting or on directories not being
created**: two inputs with the same base name, which overwrote each other, are now
refused; a missing output directory, which caused failure, is now created. The tests of
#544 that **pin the previous state** — `batch_test.cpp`, `generated_batch_test.cpp` —
become visible diffs, which is the point of having written them first; the paragraph of
`invocation.md` gets rewritten.

**The list of inputs is an object** (`std::vector` of `{given path, computed
destination, relative}`) that validation, the walk, writing and JSON share; the
destination is computed **once**, at validation, and writing reads it back.

**The batch bench**: the walk makes the batch measurable — N files generated in a
`Scratch` against the same work in a single one —, and the measurement is born with it,
in the log.

**What would justify reopening**: a batch large enough for the sequential loop to show;
a caller who wants a name pattern or `--no-clobber`; a case-insensitive file system that
would make two names collide that byte comparison tells apart (rule 1 relies on
`equivalent` when the file exists, and not on the string, for that reason).

## Implementation of the first half (#554)

Reviewed at implementation; **no decision is changed**, three clarifications the ADR did
not state:

- **A write that finds its directory missing says "cannot be written"**, like the other
  write refusals other than permission: the output directory is created before anything
  is written, so this case is no longer the ordinary one. A creation that fails has its
  own sentence, `cannot be created` (`reasonOfCreating`).
- **The separation of reading and writing also holds for the window**:
  `reasonOf(SaveError)` and the saving of the corrector's replacements say "written" as
  well. Both surfaces kept the same wrong sentence; they now keep the same right one.
- **The identity of two paths** is judged by the normalized absolute spelling, then by
  `FileSystem::equivalent` (new, true when both exist) — with a sieve on the file name,
  case-insensitive, so that the comparison stays linear in the size of the batch.

## Implementation of the second half (#555)

Reviewed at implementation; **no decision is changed**, four clarifications:

- **The walk goes through `FileSystem`** — `entriesIn` (typed entries, links not
  followed, sorted by bytes) and `isDirectory` — and not through `std::filesystem`: it is
  tested without a disk, and the order is the contract's, not the system's.
- **"Left aside" counts only files with an unknown extension** (including `.txt`): hidden
  entries and links are ignored silently, because counting them would make a number
  nobody asked for.
- **A directory with nothing to walk is said at level 1 and is not an error**: the batch
  of zero files succeeds, like any empty batch.
- **The tree goes through `Destination::withRoots`** rather than through a list-of-inputs
  type: the batch functions keep their signatures, and an input found under a root keeps
  its relative path while a named input keeps only its name.
