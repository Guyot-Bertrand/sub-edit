# Gaupol's correction patterns, and what it does with them

Gaupol's correction patterns, versioned as they are, texts written by hand to
test them, and **what Gaupol does with them**, written by an oracle. This is what
the phase 12 correction engine will have to reproduce, whichever regular-expression
engine is retained. Issue
[#494](https://github.com/Guyot-Bertrand/sub-edit/issues/494), a tool retained by
the initialisation of the phase ([#492](https://github.com/Guyot-Bertrand/sub-edit/issues/492)).

These expected files were written **before** the engine, for the reason given in
[`formats/`](../formats/LISEZMOI.md): a fixture drawn from one's understanding of
an algorithm, read back by the implementation of that same algorithm, proves only
their agreement.

## What is here

| Directory | What it is | Written by |
| :--------- | :----------- | :-------- |
| [`packaging/patterns/`](../../../../packaging/patterns/) | Gaupol's twelve pattern files and their twelve activation `.conf` files | copied, never touched up |
| `entrees/` | the texts to test, one file per pattern type | by hand |
| `attendus/` | what Gaupol does with each text, in the `.cas` format, and **the cascade of each code** (`cascades.txt`) | `src/scripts/pattern-oracle.py` |
| `utilisateur/` | pattern files **that the user drops** into `$XDG_DATA_HOME/subedit/patterns`: two that fail (`Latn-xt`, a pattern that does not terminate; `Latn-xu`, an untranslatable pattern), `Latn-xg` — two untranslatable patterns, one bearing a shipped pattern's English name and one the user's own, for the names shown in a language (#666) —, two that share a name between two types (`Latn-xa`), one that is added to the shipped patterns (`Latn-xb`), and the three penalties of the Gaupol line-breaker test (`Zzzz-xl`, what the `essai` cases of `line-break.cas` ask for) | by hand, for the end-to-end cases of `correct` (`src/test/e2e/cli/correct_test.cpp`), which drops them into the directory the harness has moved |

## `packaging/patterns/` — the patterns, as Gaupol ships them

They no longer live here: the program installs and reads them, the oracle and the
tests with it, so there is only one copy. Their provenance, their attribution
and the update rule are in
[`packaging/patterns/LISEZMOI.md`](../../../../packaging/patterns/LISEZMOI.md).
Issue [#498](https://github.com/Guyot-Bertrand/sub-edit/issues/498).

## `entrees/` — the texts to test

One file per type: `common-error`, `capitalization`, `hearing-impaired`,
`line-break`. One line per case:

```
target | expectation | label | "text"
```

| Field | What it says |
| :---- | :----------- |
| target | a single record — `Latn-en:3`, the third of `Latn-en.<type>`, applied whether it is active or not —, or a cascade — `cascade Latn-en`, as Gaupol activates it by default, `cascade Latn-en tous` (all), everything ticked |
| expectation | `corrige` (fixed) or `intact` (untouched): what the case **announces** |
| label | what the case shows, as prose |
| text | the text, between quotation marks; several texts separated by `;` are consecutive subtitles |

The escapes are those of the `.cas` files: `\"`, `\\`, `\n`, `\t`. The alignment
spaces of a target do not count.

**The oracle refuses a case that does not do what it announces** — a `corrige` that
changes nothing, an `intact` that changes —, **a record that does not have a case of
each kind**, and a shipped language without its cascade. This is what keeps these
inputs honest: an `intact` case is placed **at the edge** of the pattern, where an engine
that translated a lookbehind badly would correct too much.

**Line breaking has its own target and its own expectation** — issue
[#495](https://github.com/Guyot-Bertrand/sub-edit/issues/495):

- the target ends with the settings, `<length>/<lines>` — `Latn-en:2 16/2`,
  `cascade Latn-en 42/2`; two more selections, `essai` for the three
  penalties of `aeidon/test/test_liner.py` and `aucune` (none);
- the expectation is `coupe` (breaks) or `intact`;
- **each pattern decides at least one case**: its breaking differs from that of
  no penalty at the same settings. The oracle refuses a pattern that decides
  nothing, otherwise an engine that ignored it would pass;
- **in characters only**: measurement in *ems* depends on the font, and it is
  for the scoping to decide it (#493).

**Most texts carry no tag.** Gaupol's parser strips the tags and puts them back
around the replacements. Where a correction touches a SubRip tag, `entrees/` carries
a few more cases — issue [#501](https://github.com/Guyot-Bertrand/sub-edit/issues/501) —,
and the oracle carries Gaupol's parser (`aeidon/parser.py`) to produce them; this is what
the tag parser of phase 10, tested elsewhere, must reproduce.

## `attendus/` — what Gaupol does with them

One `.cas` per type, which `textCasesOf` can read: `label | "input" | "expected"`,
`=` for a text the patterns leave alone, `supprimé` (removed) for a subtitle that
mention removal empties. The label begins with the target and the expectation; a
subtitle of a sequence carries its rank, `[2/3]`.

**Generated, never written by hand**:

```console
$ ./src/scripts/pattern-oracle.py --write   # after changing an input
$ ./src/scripts/pattern-oracle.py --check   # what make fixtures runs
```

## What the oracle reproduces, and why not `re.sub`

The oracle reproduces Gaupol's loading (comment lines, `\0` removed,
activation through the `.conf` files), its cascade of codes (`SkipIn`, `Policy=Replace`), its
search, its three operations on the text and its line breaker (`Liner`),
**line by line**. Each is named in the
script next to what it carries.

**Gaupol does not replace with `re.sub`.** It searches for each match in the
text **already modified**, from the end of the last replacement, with its own
rule for empty matches. The difference is narrow, and measured: tens of thousands of
randomly drawn texts show none for common errors and capitalisation, where `Repeat`
catches everything. It appears in the cleanup that follows mention removal, and the two
cases that show it are commented in `entrees/hearing-impaired.entrees`.

## What was verified, and what cannot be replayed

**The oracle was checked against Gaupol's own code** before being versioned:
each case, passed through Gaupol's functions on a copy of its `aeidon` package
made outside the repository, once without a file and once in a SubRip project — to
exercise the tag parser. **No disagreement.** The same check,
redone with an oracle reduced to `re.sub`, finds the two cases of the
previous section: it knows how to see a difference.

**The line breaker was checked likewise**, against Gaupol's `Liner` at the same settings
and the same penalties: no disagreement, and the four breakings of
`test_liner.py` are exactly those that this test asserts. Three
falsified oracles — without the "pyramid" term, without the variance of lengths, without the
rule that waits for `max_lines` lines before breaking — each find one.

Two behaviours of the breaker are worth knowing before writing one:

- **it exceeds `max_lines` when the length requires it** — "It is 40 cm longer than
  mine" comes out on three lines at width 11 with two lines wanted, so as not to
  separate "40" from "cm";
- **it breaks nothing as long as the number of lines tried does not reach
  `max_lines`**: two words too long for a line stay on a single line
  when three are allowed, and break when two are allowed.

This is **an observation, not a check**: it requires Gaupol, and the repository does not
contain it. What can be replayed anywhere is the oracle on the versioned copies,
and it is the one the gate runs.

## `cascades.txt` — the patterns each code activates

One line per type and per code: `<type> <code> | <code>:<rank>± …`, the
records in the order in which Gaupol applies them, `+` if active by
default and `-` otherwise. This is what `get_patterns` returns, and **what the core's
reader returns in turn** — the test `pattern_catalogue_test.cpp` checks them against each other, without
launching Python. Issue [#498](https://github.com/Guyot-Bertrand/sub-edit/issues/498).
