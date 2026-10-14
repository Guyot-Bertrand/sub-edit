# 0025 — A single opening recipe, in the core, and the error that comes out of it

**Date:** 2026-08-28
**Status:** accepted

Decided while implementing issue #154.

## Context

Reading a subtitle file means chaining three calls in this order: the bytes are
requested from the system, `readSubtitles` recognises them, and the project
keeps what it needs to write the file back as it arrived — its format
included, since [ADR 0018](0018-vocabulaire-des-formats-dans-le-modele.md).

**That chain was written four times**: in `gui/opening.cpp` and in the three
`cli/` commands that read before acting. Only one of the four sites carried a
comment saying why the order is what it is. That is the definition of a
workaround that repeats itself.

The window's copy had drawn one more consequence from it, and a costly one. It
translated **every** `FileError` into `ReadErrorKind::NoSubtitleFound`:

```cpp
// A file that cannot be opened and a file that cannot be understood are
// both « nothing to show », and the window says so the same way.
```

The reasoning holds for what the window *does* — in both cases, nothing opens.
It does not hold for what it *says*: a missing file, a file refused by the
system, bytes that are not UTF-8 and a Word document all received the same
sentence, and it was true only of the last. The window therefore had **less
information than the terminal** exactly where the user needs it most: at the
moment they do not understand why their file does not open.

## Decision

**The recipe lives in `core/format/`, under the name `openProject`, and the
four sites call it.** Its counterpart `saveProject` goes with it.

**What it returns on failure is a sum of the two errors that can occur**, not
one crushed into the other:

```cpp
using OpenError = std::variant<FileError, ReadError>;
```

## Why `format/` and not `io/`

It is the question the issue left open, because neither answer is obvious: the
function touches the file system, which `readSubtitles` does not, and it knows
the formats, which `io/` does not.

**What settles it is the type it returns.** `io/` returns bytes and system
errors; `format/` returns models. `openProject` returns a `Project` — a model —
so it is on the format's side, which uses `io/` as `writeAtomically` uses
`FileSystem`. The dependent names the directory, not the dependency.

The counterpart already existed and seemed to lean the other way:
`writeAtomically` lives in `io/`. It is not a contradiction, it is the same
rule applied: `writeAtomically` hands bytes to a file and knows nothing of a
subtitle. `saveProject`, which starts from a `Project`, is in `format/` with
`openProject`.

## Why a sum, and not a third enumeration

Both enumerations follow the same rule, written in `read_error.hpp`: **each of
their values is produced by something.** A value nothing emits is a broken
promise — every caller that translates it into a message carries a branch no
test can reach. `ReadErrorKind` actually lost a member for that reason,
`FileNotReadable`, which nothing emitted.

Merging the seven causes into a single list would require maintaining a
mapping in both directions, and adding to it, for each new cause on one side, a
value on the other. The sum produces nothing new: it expresses the choice, and
nothing but the choice.

**The words already existed.** `core/wording/formats.hpp` could say all seven;
only the window could not reach them. A three-line `reasonOf(const
OpenError&)` is enough, and that is all the visible half of the ticket cost.

## Consequences

**`src/lib/subedit/gui/` no longer contains a core recipe.** That is what makes
the boundary readable, and `check-architecture.sh` already holds it in the
other direction by refusing Qt under `core/`.

One exception remains and it is named: `mpv_player` contains not a line of Qt
and stays in `gui/`, for the reason
[ADR 0020](0020-libmpv-pour-le-lecteur-integre.md) gives — a player is an
interface element, it exists only for the window, it receives the window's
own. The rule is therefore **not mechanised**: a check that refused every file
without Qt under `gui/` would flag that one on every run, and a check that
cries wolf ends up disabled.

**The window says what the terminal already said.** The window's manual now
lists the messages it actually writes; it used to say the message was "the
same for all four cases", which was accurate and regrettable.

**Writing has not been unified with the command line's**, and that was not
asked without reservation: the issue said to look, and to do it only if it held
up. It does not. The command line writes to a computed destination, in batch,
with a shape its options can change; the window writes one file to a path the
user has just chosen. What they share is `writeSubtitles` then
`writeAtomically`, which they both already call. `saveProject` moves down to
the core because its place was badly chosen, not because a second caller was
waiting for it.
