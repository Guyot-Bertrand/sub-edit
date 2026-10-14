# 0037 — Read Gaupol's pattern files as they are

**Date:** 2026-09-26
**Status:** accepted

Decided while scoping phase 12, issue
[#493](https://github.com/Guyot-Bertrand/sub-edit/issues/493).

## Context

Gaupol's correction patterns are twenty-four files: twelve pattern files, named
`<Code>.<type>`, and their twelve activation `.conf` files. The code is
`Script[-language[-COUNTRY]]`, `Zyyy` meaning "any script". A pattern file is a
sequence of GKeyFile-like records — a `[Common Error Pattern]` header, then
`Name`, `Description`, `Classes`, `Pattern`, `Flags`, `Replacement`, `Repeat`,
`Policy`, `SkipIn`, `Capitalize`, `Group`, `Penalty` —, **with repeated
headers** that no standard INI reader accepts. A `.conf` is an XML file with one
line per name: `<pattern name="…" enabled="…"/>`.

The question had been raised once before in phase 4 and dismissed (#88):
"copying would be lawful, both projects are under GPL-3, but it would commit us
to a file format we did not choose". Since then, two facts have changed the
picture:

- **#494 put these files under version control** in
  `src/test/data/motifs/gaupol/`, with their attribution, and the expected
  results Gaupol derives from them: what the engine must reproduce is written
  down, **from these very files**;
- **phase 12 must read pattern files anyway** — the user's. Gaupol reads those
  dropped into its data directory (`~/.local/share/gaupol/patterns`), and
  feature parity calls for the same door.

## Decision

**Read Gaupol's format as it is**, with a reader written in the core, and turn
it at load time into a typed model — a record is a `CorrectionPattern` with
named fields, not a dictionary of strings. The shipped files are **Gaupol's,
untouched**, installed with the program. What is kept from the user — the
activation of each pattern — goes into **our settings**, not into `.conf` files
written the Gaupol way.

## What decides

**A single copy, read by everyone.** The Python oracle of #494 reads these
files; the C++ engine will read the same ones; the package installs the same
ones. A format of our own would need a converter, and the day Gaupol changes a
pattern, one would have to convert before rerunning the oracle — two truths
instead of one.

**Updating from upstream remains a copy operation.** The `LISEZMOI.md` of #494
already describes it: copy the whole set from a more recent revision, rerun the
oracle, read the diff of the expected results. No intermediate format gets in
between.

**A Gaupol user's file is readable here.** Someone who wrote their patterns for
Gaupol drops them into our directory and they apply — subject to the syntax
translation of [ADR 0036](0036-icu-pour-les-motifs-de-correction.md), which
says what it refuses.

**The objections of #88 have an answer.** The format is not ours, but the
reader fits in about a hundred lines, and its rules are already written twice —
in `aeidon/patternman.py` and in `pattern-oracle.py`, which names each step:
whole-line comment, `\0` not followed by a digit removed, keys prefixed with an
underscore accepted. The repeated headers, which no INI reader accepts, are a
problem only for someone looking for an INI reader; we are not looking for one.

## What does not come from Gaupol

**Activation is kept in our settings.** Gaupol writes one XML `.conf` per code
into its configuration directory. Here, the shipped `.conf` files give **the
default values**; what the user ticks or unticks is remembered in the settings
file of [ADR 0022](0022-configuration-au-noyau-et-tolerance-par-option.md), by
type, code and name — the single place of configuration, tolerant option by
option. Writing a second persistence mechanism for a list of check boxes would
be the opposite of 0022.

**The key is the English name.** Gaupol translates `Name` and `Description`
through gettext, but activates and finds a pattern by its untranslated name;
several records share a name and a single box. We keep the key; translation
waits for phase 15.

**The shipped files leave the test directory.** They are currently in
`src/test/data/motifs/gaupol/`, because #494 had only an oracle to feed. The
installed program cannot read a test directory: they move to
`packaging/patterns/`, next to the other files the package installs, and the
oracle and the tests read them there. **One copy, not two.**

## Alternatives dismissed

- **Converting once to a format of our own** — TOML, JSON, or a line-by-line
  format. More readable, and we would choose our fields. But conversion becomes
  one more step at every upstream update, the oracle would have to read one or
  the other, and a Gaupol user's files would no longer be readable without a
  tool.
- **Compiling the patterns into the binary** — C++ tables generated at build
  time. No file to find at run time, but no way left to add a pattern without
  recompiling, and the door to user patterns, which Gaupol opens, would close.
- **Writing Gaupol-style `.conf` files** for activation — compatibility all the
  way. But the compatibility that matters is that of the patterns, not of their
  check boxes, and the price would be a second configuration mechanism next to
  that of ADR 0022.

## Consequences

**A reader to maintain in the core**, with its diagnostics: a record without
`Pattern`, an unknown key, an unreadable file are reported — file, line,
reason — and do not stop the reading of the others, like opening a subtitle
file ([ADR 0008](0008-lecture-au-mieux-avec-diagnostics.md)).

**A data directory at run time.** The program must find the shipped files, and
it does so the way it finds its manual, `installedManualPath()`: **from the
executable**, `../share/subedit/patterns`, never through a path fixed at build
time — the configuration prefix and the installation prefix are not the same.
The user's patterns have their location, `$XDG_DATA_HOME/subedit/patterns`,
resolved by a single function like `userSettingsPath()`; and, as for that one,
**a test never resolves either of them**: it receives its directories. Since the
first location is specific to the executable, and the command line has to
correct in phase 13, its resolution lives outside `gui`.

**Where, since #566**: `subedit::platform` (`src/lib/subedit/platform/`), a
Qt-free library that the window and the command line both link. The core keeps
its two pure functions and still reads no variable; the library is what gives
them their arguments — the executable's directory (`/proc/self/exe` on Linux,
the door left open elsewhere, ADR 0003), `XDG_DATA_HOME` and `HOME`. The build
tree reproduces the installation, through a link CMake sets up
(`build/<preset>/share/subedit/patterns`), so that a binary launched from it
finds its patterns like an installed binary, without a debugging option.

**What would justify reopening**: Gaupol changing its format, or a need its
format cannot carry — a pattern specific to `subedit`, for instance. That day, a
file of our own would be added next to its files rather than replacing them.

## Addendum — the names and descriptions in the user's language ([#666](https://github.com/Guyot-Bertrand/sub-edit/issues/666))

**The files stay Gaupol's, and English.** A pattern's `Name` and `Description` are translated the
way Gaupol translates them: in the program's catalogue, never in the file. The catalogue is keyed
by the English text — the msgid is the name or the description as the file writes it — and is
looked up **when the pattern is shown**, by `core::patternName` and `core::patternDescription`
(`core/wording/correction.hpp`): the tick boxes of `Correct Texts…`, with the description as their
tool tip, the patterns its confirmation page says were not applied, and the same sentence on the
command line's error stream. Nothing is written back: the files are read as they are and stay
byte for byte what they were ([spec 15](../specs/15-internationalisation.md), D6).

**A pattern the catalogue does not know shows as written**, which is what a user's own pattern is.
**The English name stays the identifier**: a setting records it, `--enable` and `--disable` take
it, and `--format json` writes it, whatever the language (D3).

**How the template gets the msgids.** They are not C++ literals, so the extraction of the code
cannot find them. `src/scripts/update-pot.sh` reads the shipped pattern files of
`packaging/patterns/` as well, as the desktop-entry files they resemble, with `Name` and
`Description` for keywords, and joins them to the extraction of the code; `make po` merges them
into each language like any other message, and `pot-check` refuses a template that misses one.
