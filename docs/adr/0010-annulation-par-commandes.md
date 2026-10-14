# 0010 — Undo through commands that carry their own inverse

**Date:** 2026-08-05
**Status:** accepted

## Context

Undo does not appear among the user's priorities, but editing in a table implies
it, and it is typically what cannot be added after the fact: every operation
written without it would have to be reworked.

Gaupol handles it this way: each revertible action carries an **inverse
function** and its arguments (`RevertableAction.revert_function`,
`revert_args`). Undoing consists of calling that function with `register=UNDO`,
which makes it push itself onto the redo stack. Undo and redo therefore use the
same machinery. The previous state is not captured wholesale but **in the
arguments of the inverse operation**, and so reduced to the bare minimum:
removing subtitles records "insert these subtitles at these indices" as its
inverse.

## Decision

A **command** is an object carrying `apply` and `revert`, along with what it
needs to undo itself — captured at construction, and limited to what is
necessary. The history holds two stacks of commands.

Three mechanisms taken from Gaupol, because they meet real needs:

- **Grouping.** A composite command contains a sequence of commands and counts
  as a single entry in the history. An operation that chains others must not
  force the user to undo seven times.
- **A modification counter per document.** An integer moved by ±1 on each
  action; zero means "identical to the file on disk". The state therefore
  returns to "unmodified" when one undoes back to the save point, which a plain
  boolean cannot do.
- **Indices of the elements touched.** Each command reports the indices it
  affects, and in what respect — positions, main text, translation text,
  insertion, removal. This is what will let the phase 5 interface refresh only
  what changes instead of rebuilding everything.

## Alternatives dismissed

- **Capture the complete state before and after** — robust, and impossible to
  get wrong. Dismissed on cost: a global replacement on a file of several
  thousand subtitles would duplicate the whole set on every operation, and the
  history would keep it. That is precisely the kind of waste that performance as
  a cross-cutting goal rules out.
- **A log of differences** — economical, but requires defining a difference
  format for each kind of data, and maintaining it with every change to the
  model. Complexity with no return at this scale.
- **Qt's `QUndoStack`** — already exists and does the job. Dismissed without
  hesitation: it would put the history in Qt code, whereas the phase 3 command
  line needs it and the core does not know Qt. The interface will plug into the
  core's history; it will not keep a second one.
- **An inverse function and arguments, exactly as in Gaupol** — it is the same
  idea, expressed in Python. In C++, a command object carries it more clearly
  than a function pointer accompanied by a bag of arguments, and the compiler
  checks that the inverse receives what it expects.

## Consequences

Every core operation is written as a command, from phase 2 onwards. A function
that modifies a project without going through this is a defect, and the rule
must be laid down before a single operation exists — that is why this decision
is taken in phase 1.

Each command must be able to undo itself **exactly**. A non-invertible
operation — should one appear — will have to clear the stack rather than lie
about its reversibility.

The size of the history is bounded. Gaupol sets a limit of 100,000, which in
practice means none; we will measure the real footprint before choosing a
number rather than copying Gaupol's.

The history belongs to the project and lives in the core, with no dependency on
the interface. It is therefore testable without a window, and usable by the
command line.
