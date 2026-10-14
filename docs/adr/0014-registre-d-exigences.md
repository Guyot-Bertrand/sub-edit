# 0014 — Name requirements in a flat register, cite them by a test tag

**Date:** 2026-08-10
**Status:** accepted

## Context

Phase 3 opens the command line. It is the first time the project will have
something a user invokes, and a command is a whole: arguments parsed, file
read, operation applied, file written, exit code returned. Unit tests say that
each piece works; none says that the assembly works, and the assembly is what
the user invokes.

Line coverage does not fill this gap. It says **which code was executed**,
never **which advertised feature is demonstrated**. The two questions look
alike and do not have the same answer: a file covered at 100% may promise
nothing, and a promise of the manual may be checked nowhere without any figure
dropping.

The manual already describes exhaustively what the user sees — that is a
project rule. What it does not do, and cannot do, is bind itself to what proves
it: a sentence of the manual and the test that demonstrates it have, today, no
common ground a machine can read.

What is known at the time of deciding: the project has only one executable,
which prints its version, and the manual specifies it exhaustively. What is
not known: how many subcommands phase 3 will produce, and whether the graphical
interface phases will lend themselves to the same mechanism.

## Decision

Requirements live in **a flat, citable register**, `docs/exigences.md`: one
table, one row per requirement, four columns — identifier, statement, phase,
state.

A requirement is **what the binary shows**: standard output, standard error,
exit code, file produced. The core's internal guarantees are not requirements;
their unit tests and line coverage play that role.

An identifier has the form `SURFACE-SUBJECT-NN`, in capitals —
`CLI-VERSION-01`. The form is not decorative: it is what makes it possible to
recognise an identifier among a test's tags without keeping a list. An ordinary
project tag — `[e2e]`, `[format]`, `[framerate]` — has neither capitals nor a
final numeric segment, and so cannot be mistaken for an identifier. The
converse is what matters: a tag that has this form **must** designate a
requirement, or the script fails.

**An identifier is never reused.** A replaced row stays, struck through, with
its reason. An identifier that disappears without a trace leaves a gap nobody
knows how to interpret.

Four states, and each constrains the presence of a test in both directions:

| State | What the register asserts |
| :---- | :------------------------ |
| `implemented` | at least one test cites it |
| `planned` | no test cites it |
| `abandoned` | no test cites it |
| `replaced` | no test cites it |

The end-to-end tests of `src/test/e2e/` cite a requirement by a **Catch2
tag**, `[CLI-VERSION-01]`, and not by a title prefix.
`src/scripts/check-requirements.sh` queries the test binary — `--list-tags`
outputs the exact set of tags — and compares the two lists. It is part of
`make check-local`, not `make check`: CI only runs `make check`, so this check
does not gate every push. `make check-local` is the target dedicated to checks
one wants to see pass before opening a pull request without having them gate
CI.

## Alternatives dismissed

- **Finding the tags by parsing the C++.** A `grep` on `TEST_CASE` would have
  avoided building the binary. Dismissed because the answer would be
  *approximate*: it would miss a tag produced by a macro or by conditional
  compilation, and invent those of a commented-out file. Catch2 knows its tags
  exactly; asking it makes the check exact instead of plausible.

- **Labelling by a prefix in the test title**, `TEST_CASE("[CLI-VERSION-01]
  the invocation…")`. Dismissed for the opposite, symmetrical reason: the
  binary could no longer list the identifiers itself, and we would fall back on
  the text parsing just dismissed.

- **Two registers, one for requirements, the other for features.** Dismissed:
  the second one's role is already held by the manual, which the project rule
  obliges to be exhaustive about everything the user sees. Three places saying
  the same thing would make two opportunities to diverge.

- **Giving an identifier to the core's guarantees too** — exact undo, format
  round trip, ordering policy. Dismissed: the boundary of "what deserves an
  identifier" would again become a question to settle for every test, and the
  register would duplicate the unit suite without asserting anything more. The
  scope "what the binary shows" has the advantage of being decidable without
  discussion.

- **Allowing a unit test to cover a requirement.** Dismissed: an end-to-end
  requirement "covered" by a test that never invokes the binary is exactly the
  confusion the harness exists to dispel.

- **Reporting rather than failing**, or failing only on `implemented` while
  leaving the other states as warnings. Dismissed: a warning drowned in ten
  minutes of output is a warning nobody reads. A constraint that does not shut
  the gate is not one.

- **Constraining only the `implemented` state**, the other three remaining
  documentary. Narrowly dismissed — it is the literal reading of the need. But
  a test that cites a `planned` requirement proves it is implemented and that
  the state was not flipped; a test that cites an `abandoned` requirement is a
  test left behind. These two drifts are mechanically decidable, so there is no
  reason to leave them to review.

## Consequences

What the decision makes easy: reading in a single place what the tool promises
and what proves it. An implemented requirement without a test no longer passes
`make check-local`, and neither does a tag that designates nothing.

What it makes demanding: **every issue that adds a visible behaviour feeds the
register at the start of the work**, before the code. That is the point, and it
is a discipline that holds only because a script checks it — the same rule
written in `CLAUDE.md` would have drifted, as the `Closes #N` rule drifted
there.

The real cost of placing it in `check-local` rather than `check`: the script
does not gate CI. A pull request whose register and tags diverge is stopped
only if someone runs `make check-local` before opening it — nothing forces it
mechanically, unlike `make check`, which CI imposes on everyone. That is
acceptable for what the check protects: a register that lies about what is
proven is a defect visible at the next reading of the file, not a bug shipped
to the user — the nature of the risk tolerates a looser net than the one that
protects against a memory leak or a coverage regression. But it is a real
weakness, not something to embellish: without local discipline, a pull request
can merge with the register drifting, and nothing flags it until a human
review or a late `make check-local` notices.

The harness launches a real process, which has an effect to watch on the
coverage measurement: under the `coverage` preset, `subedit-cli` is also
instrumented and each invocation would inflate the coverage of `src/lib`
without any unit test having been written for that code. The end-to-end tests
are therefore **registered in CTest only under `asan` and `release`** — the
cache option `SUBEDIT_REGISTER_E2E`, `ON` in those two presets and nowhere
else, declares it rather than leaving it to be deduced from the compilation
flags.

Under `asan`, a leak in the real binary shows. `release` is the shipped mode,
hence the only one whose behaviour commits the user. `dev` gained nothing from
it: it is a Debug build like `asan`, which is strictly more informative.
Consequence to know, also written in the foundations spec: **`make test` does
not exercise the harness**; `make asan` is the shortest path to doing so.

The register says nothing about the core, and that is deliberate. If an
internal guarantee one day deserves a name, it will be through a separate
decision, not through a drift in scope.

Undoing the decision would cost little: removing the script from `check-local`
suffices, the tags become inert tags again and the register a document. The
trigger for a review is identified — the day a requirement can only be
demonstrated through the graphical interface, the harness will have to cover a
second surface, and "one binary, one `--list-tags`" will have to become
"several".
