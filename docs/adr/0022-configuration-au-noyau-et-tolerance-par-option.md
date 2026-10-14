# 0022 — A configuration in the core, tolerant option by option

**Date:** 2026-08-28
**Status:** accepted

Decided when phase 7 was scoped (#237).

## Context

Phase 7 brings persisted preferences: window geometry, column widths, theme,
default frame rate. Two questions arise together, and they are better settled
together because the second depends on the first: **where the configuration
lives**, and **what happens when the file does not say what is expected**.

The second is not theoretical. A configuration file outlives the versions of
the software that writes it: it is written by today's version and read back by
the one two years from now, which has added options, removed some, and changed
the defaults.

### What Gaupol does, read rather than assumed

`gaupol/config.py` was read when phase 5 was scoped and read again here.
**There is no migration, and it is not an oversight: it is a complete
mechanism.**

- The file is INI with nested sections — `[a::b]` — whose values are JSON, plus
  a codec for enumerations.
- An unknown key is **accepted** on reading, then **erased on writing** if it is
  no longer among the defaults.
- An unreadable value prints to standard error and **leaves the default in
  place**. A null value is discarded likewise.
- Every option left at its default value is written back **commented out**, so
  that a change of default takes effect for whoever never overrode it.
- `general.version` is written and **never read back**: a trace, not a trigger.
- Reading and writing are wrapped: a failure prints and stops nothing.

Twenty years of use without versioned migration. It is not proof, but it is a
data point.

## Decision

**The configuration is a core type, which reads and writes through
`core::FileSystem`, and which receives its location rather than looking for
it.**

**It is tolerant option by option, with neither version nor migration** —
Gaupol's mechanism, including writing default values back commented out.

Concretely:

| Case | What happens |
| :--- | :----------- |
| unknown key | ignored on reading, absent on rewriting |
| unreadable value | the default is kept, a diagnostic is produced |
| missing option | the default, without it being a special case |
| option at its default | written back **commented out** |
| missing file | all the defaults, and it is not an error |
| unreadable file | all the defaults, and a diagnostic |

## Why in the core rather than in `QSettings`

`QSettings` would provide the location, the parsing and atomic writing without
our writing a line. Three reasons not to take it, and the third is the decisive
one.

**Our values are typed.** A default frame rate is not a float: it is a
`FrameRate`, whose invariant is that it is one of the eight. `QSettings` returns
`QVariant`s, so the boundary between the file and the strong type would exist
anyway — and it is precisely the place where the behaviour above must be
written. `QSettings` does not remove the work, it moves it.

**The core is free of Qt, and `check-architecture.sh` checks it.** A
configuration in `gui/` would cut the command line off from any preference,
for good. We want none today, but closing the door through a choice of library
rather than through a decision would be closing it without having decided to.

**The seam already exists and it is proven.** `core::FileSystem` is the
abstraction through which formats are tested without a disk, and
`InMemoryFileSystem` is its test implementation. A configuration that goes
through it can be tested without writing anywhere — which is exactly what issue
#238 is after, and it gets it without a global Qt setting or a test mode.

**What `QStandardPaths` then becomes:** a line of wiring in
`src/exe/gui/main.cpp`, which hands the location to the type. That is the
definition of wiring, and it is the only path the tests will not cover — it is
named here so that it is known.

> **Corrected while laying it down, #238.** That path is covered, and making it
> so was not expensive: `gui::userSettingsPath()` is a library function —
> `main.cpp` will call it in one line when #240 gives it a type to serve — and
> a test reads what it returns without any process having to start. What the
> ADR anticipated — one line out of reach — was the price of a
> `QStandardPaths` called in place; it was enough not to call it there. The
> sentence that matters remains true: it is the **only** place where a
> standard location is resolved.

## Why tolerance rather than a versioned migration

**They do not cost at the same moment.** Tolerance costs on every read, a
little; migration costs on every change of format, a lot, and the cost falls on
whoever changes the format — that is, at the worst moment, when one's mind is
on something else.

**A migration can only be tested against files one has made up.** It catches
only the cases one has imagined, and its failure mode is to refuse a legitimate
file. Tolerance has the opposite failure mode: it accepts too much, and what it
lets through falls back on a default, which is written down.

**Writing defaults back commented out is what makes tolerance sufficient**, and
it is the detail of Gaupol worth keeping: without it, an option the user never
touched would be frozen at the value of the day it was written. An improved
default would never reach anyone. With it, changing a default changes the
behaviour of everyone who has not overridden it — which is exactly what a
default should do.

## Alternatives dismissed

**`QSettings`.** See above. Dismissed on the argument of the Qt-free core, not
on convenience — it offers plenty.

**A versioned migration, triggered by a number written in the file.**
Dismissed for its cost and its failure mode. Note that Gaupol *writes* that
number and never reads it back: the trace is useful when reading a file by
hand, the trigger is not. We write no number at all — a field that is never
read back is a field nobody will know to be right.

**Refusing a file we do not understand.** It is the strictest behaviour, and the
worst: it turns a lost preference into software that does not start. A
configuration is a convenience; its failure should cost the convenience and
nothing else.

## Consequences

**The diagnostic needs an output, and the window has none.** Gaupol prints to
standard error, including from its graphical interface. It is the cheapest
route and it is acceptable for a fault that prevents nothing; the
implementation issue settles it, and a modal dialog at startup for an
unreadable preference would be a bad trade.

> **Settled while laying it down, #240: standard error.** One line per
> unreadable option, plus one for the whole file when it exists and refuses to
> be read; each names the file, the option, and quotes the offending value —
> without it, "cannot be read" leaves the user searching their file for what
> was wrong. The formatting lives in `gui::readUserSettings`, not in
> `main.cpp`: an entry point wires, it does not compose messages.

**The file stays readable and editable by hand**, which is a property and not a
side effect: it is what makes a preference fixable when the interface that sets
it is precisely the one that is misbehaving.

**A changed default changes the behaviour of existing users.** That is
intended, and it is the flip side of writing defaults back commented out: a
change of default is not trivial and is decided like a change of behaviour.
