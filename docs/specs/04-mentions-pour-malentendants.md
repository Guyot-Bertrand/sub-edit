# Phase 4 — Removal of hearing impaired annotations

**State:** design approved, ready to be split
**Milestone:** `4 — Mentions pour malentendants`

## Goal

Remove from a set of subtitles the annotations intended for the hearing
impaired — the noises described between brackets or between parentheses —
without leaving anything behind them: no extra space, no empty line, no subtitle
without text.

This is **the first time the core computes a text**. The three previous phases
read, wrote and moved; `SetTextCommand` replaces a text with the one it is given
and decides nothing about its content. Everything that follows stems from this
novelty: where the computation lives, what tests it, and what it costs.

## Scope

**Included** — the two patterns of the roadmap, brackets and parentheses; the
seam rule that `mentions.cas` fixed case by case; an undoable command; the
`hearing-impaired` subcommand; the manual; the requirements; and the first text
measurement of the performance log.

**Excluded, by phase**

| What is not done here | Where |
| :-------------------- | :---- |
| Gaupol's three other patterns — lyrics between hash signs, on a line, speaker name before a colon | phase 12 |
| per-language patterns, pattern files, activation `.conf`, spell checker, recapitalisation | phase 12 |
| the choice of a regular expression engine, PCRE2 or RE2 | phase 12 — see [ADR 0017](../adr/0017-analyseur-de-mentions-ecrit-a-la-main.md) |
| applying the operation to a selection of subtitles | phase 5, which brings a real selection; no subcommand has one today |
| cleaning the translation document from the command line | **phase 13**, per the framing of phase 11 (D9) — the core, for its part, already accepts both documents |

## Applicable decisions

| ADR | Decision |
| :-- | :------- |
| [0009](../adr/0009-texte-en-chaine-brute.md) | the text is a raw string, format tags included |
| [0010](../adr/0010-annulation-par-commandes.md) | every modification goes through a command that carries its inverse |
| [0014](../adr/0014-registre-d-exigences.md) | requirement registry checked against the tests |
| [0017](../adr/0017-analyseur-de-mentions-ecrit-a-la-main.md) | a hand-written parser, without a regular expression engine |

## What Gaupol does, and where we diverge

Its `remove_hearing_impaired` method applies a list of patterns, then **seven
catch-up passes** (`_remove_leftover_hi`): line edges, consecutive spaces, lines
without an alphanumeric character, empty lines, space after the dash, orphan
dash, dash of a single-line subtitle.

Three divergences, deliberate and decided before this framing.

**The numeric reference is kept.** `\[.*?\]` eats `[1]`; `mentions.cas` (#87)
decided that purely numeric content is not a noise and stays as it is, brackets
included.

**The seam is local to the removal.** A `" {2,}" → " "` pass is global: it also
rewrites the double spaces the file already had, and that nobody asked to touch.
Our rule only speaks of what bordered the annotation. That is the argument that
decides the engine, developed in ADR 0017.

**Nothing is configurable.** Gaupol ships a list of patterns that can be
enabled; phase 4 ships a decided transformation. #88 ruled out reusing its
pattern data — the copy would be lawful, both are GPL-3, but it would commit us
to a file format we did not choose.

## The rule

`src/test/data/textes/mentions.cas` **is** the specification, written case by
case and before any code. What follows states what those cases bring out; if
the two disagree, the case file is authoritative — it is what the code must
pass.

### What an annotation is

A `[` or a `(`, and **the first closing delimiter of the same kind that
follows**. The pattern crosses the line break: in real files, an annotation is
often cut by it, and across fifteen subtitle files the twelve brackets that
seemed orphaned were all of that form.

### What is not one

| Case | What happens | Why |
| :--- | :----------- | :-- |
| **purely numeric** content — `[1]`, `[ 156478 ]`, `(12)` | kept as is, delimiters included | a reference has never denoted a noise |
| a blank **between** the digits, a sign, a letter — `[1 2]`, `[-1]`, `[1a]` | it is an annotation | the criterion is narrow on purpose |
| delimiter **without a closing one** | left as is | no real file produces one; this is not a decision but what a scan does when it does not find its end |

Empty content, or content made only of blanks — `[]`, `[ \t ]`, `()` — **is an
annotation**: no digit, hence not a reference; nothing to say, hence nothing to
keep.

**Both delimiters are symmetrical.** `mentions.cas` only wrote the numeric
reference between brackets; the framing extends the rule to parentheses. A
digit is a digit, and two rules to remember where one suffices are paid for at
every review. The twin cases are added to the corpus.

### What the removal leaves

**Exactly one space between what surrounded it, and nothing at a line edge.**

This is not "we glue back the stray spaces": `Bonjour[il tousse]Marie` had none
and gains one. What counts is what remains on either side, not what was written
around it.

| Input | Output |
| :---- | :----- |
| `Bonjour [il tousse] Marie` | `Bonjour Marie` |
| `Bonjour[il tousse]Marie` | `Bonjour Marie` |
| `A [un] [deux] B` | `A B` — two neighbouring annotations make one seam |
| `[Bruit de pas] Bonjour` | `Bonjour` |
| `Bonjour [il tousse]` | `Bonjour` |

**A tag does not border an annotation.** Fifty-three subtitles of the corpus are
written `<i>(Guild navigator) We have`: by the letter of the rule, the tag is
what surrounded the annotation, and the removal would leave `<i> The Bene` — a
space at the start of the line on screen, which "nothing at a line edge"
precisely forbids. The tag therefore does not count, neither to judge an edge
nor to judge emptiness. It is the same rule on both sides, and it is written in
a single place.

*Decided during implementation (#101), on the evidence.* The spec had only seen
emptiness; the edge appeared while looking for the files that carry the former.

**When the annotation spans the line break, it is the break that remains**, not
the space: the two lines stay two lines.

| Input | Output |
| :---- | :----- |
| `fall back. [ Hicks\n] Fall back, now!` | `fall back.\nFall back, now!` |
| `Go! [\nMurmurs Of Agreement ]` | `Go!` — the second line is emptied, so it disappears |

### What the removal makes disappear

**A line that the removal has emptied disappears.** Emptied means: only blanks,
format tags, or a lone dialogue dash remain in it.

Tags are in the rule because the corpus put them there: nine real subtitles are
written `<i>[PEOPLE SCREAMING]</i>`, and without this clarification `<i></i>`
would remain — an empty subtitle on screen, which no rule would have removed.
ADR 0009 keeps the text raw, tags included; it is therefore up to the emptiness
test to ignore them, not up to the text to lose them.

**The test is local**, like the seam: a line already empty in the file is not
touched. The case cannot actually occur — an empty line separates two blocks in
both MVP formats — but the rule is written this way so that it stays true the
day a format produces one.

**A text that is entirely emptied takes its subtitle with it.** This is the
costly case, and the most frequent one: 58.9% of the affected subtitles of the
corpus are in this case.

### Dialogue

**A dialogue reduced to a single voice is no longer one**, and its dash goes.
As long as two remain, the dashes stay.

| Input | Output |
| :---- | :----- |
| `- [ Hisses ]\n- Uhh-` | `Uhh-` |
| `- A\n- [rires]\n- B` | `- A\n- B` |

This is a rule about dashes, not about annotations: it would also apply to a
line deleted by hand. It is here because this is where it is encountered, and
phase 12 inherits it.

**The dash is only the hyphen-minus `-` (U+002D).** Gaupol also recognises the
en dash and the em dash; of the 97,059 lines of the corpus, 4,874 start with a
dash, and **all** of them are U+002D — none is `–` or `—`. Widening without a
file that calls for it means writing code that no test can motivate. Trigger
for coming back to it: a file that shows one.

The space after the dash is tolerated when absent — eight lines of the corpus
omit it.

## The architecture

### The pure function

```cpp
// src/lib/subedit/core/text/hearing_impaired.hpp
/// Returns the cleaned text, or nothing if the subtitle is taken away.
[[nodiscard]] std::optional<std::string> withoutHearingImpaired(std::string_view text);
```

No state, no `Project`, no command, no file. It is **the exact shape of the
cases of `mentions.cas`** — an input, an output, or `supprimé` — so those cases
test it directly, through the `checkTextCases` that #87 already wrote.

`std::nullopt` says "the subtitle does not survive". An empty string would not
say so: it would be a text, and a subtitle with empty text is not the same thing
as an absent subtitle. The case corpus already makes that distinction, and it
made it in order to carry it.

### The command

```cpp
// src/lib/subedit/core/edit/hearing_impaired_removal.hpp
/// Returns the command that cleans `document`, or nothing if no text matches.
[[nodiscard]] std::unique_ptr<Command> removeHearingImpaired(const Project& project,
                                                             Document document);
```

It returns a `CompositeCommand` that groups **one `SetTextCommand` per changed
text and a single `RemoveCommand`** for the emptied subtitles. A single history
entry, hence a single undo — this is precisely what `CompositeCommand` exists
for.

**No new command class.** `CompositeCommand` *is* the command; writing one that
delegates to it would duplicate it. What is needed, on the other hand, is a
`CommandKind::RemoveHearingImpaired`, failing which the group would have to pass
itself off as something else — `Remove` would lie, since the operation rewrites
far more than it removes.

*Corrected at review (#110).* This section announced that "the CLI report tells
it apart". **That is false of the code as written**: the report reads
`describe()`, which carries the figures, and never the `kind`. Its only
production reader is `mayBreakOrder`, which answers "no, no position moves".
The reader that will name the operation in an "undo" menu arrives with the
phase 5 window — the enumerator is therefore, for now, justified by the honesty
of the name, not by a display.

**Nothing at all when nothing matches.** An empty composite would apply without
doing anything and push a history entry the user would not understand. The
caller receives a null pointer and reports that there was nothing to do.

**The order inside the group**: the texts first, the removal afterwards. The
indices do not move during the rewrites, so those the removal targets remain
valid. Undo replays the group in reverse — `CompositeCommand` already does so,
and for this reason.

### What the core does not gain

No pattern file, no format to read, no text rule interface. An interface with a
single implementation cannot be validated, and the fields that would bring it to
life — name, activation, order, language, replacement — all come from a format
that phase 12 will choose. A free function will be wrapped in that interface the
day it exists; the reverse costs more.

## The command-line surface

```
subedit-cli hearing-impaired (--output FILE | --output-dir DIR | --in-place)
                             <file>...
```

It reuses **the destination grammar, the reporter, the verbosity levels and the
exit codes of phase 3**, without inventing anything.

*One exception, which appeared while writing (#103):* the rewriting framework
could only say a fixed phrase, decided once for the whole batch — "by
2.999 s". A removal has figures **per file**, which no phrase decided in advance
can carry. The operation therefore now formulates its phrase itself, since it
alone knows what it did; the three earlier subcommands say word for word what
they used to say.

**The whole file, the main text.** No subcommand takes a selection, and
`Document::Translation` appears nowhere in `src/lib/subedit/cli` — a
`--document` option would be dead grammar. The core, for its part, accepts both
documents: it is the phase 5 window, then phase 11, that will use them.

**What the report counts**: the changed subtitles and the removed subtitles,
which `describe()` already knows how to say. Not the annotations: that figure
would require new plumbing through the pure function, the command and the
report, for a number nobody has asked for.

**A file where nothing matches is written anyway**, as with `shift`, and the
report says so. The project's rule is that a subcommand that receives a
destination writes to it; making the exception here would force a script to know
which of the subcommands sometimes produces a file and sometimes nothing.

A subcommand **and not a verb with flags**. `strip --hearing-impaired` would make
room for the four clean-ups of phase 12; but it is phase 12 that will know
whether they belong together, by knowing them. Today's CLI is a harness — the
phase 3 spec says so — and its names can still move.

## Tests

**The cases of `mentions.cas` go through the pure function**, in one line of
test: `checkTextCases` has existed since #87 and expects exactly this signature.
The corpus went from twenty-four cases to thirty-nine while writing the phase —
the parenthesis twins, the tag ones, those that prove nothing else is touched,
and the two delimiters that nothing closes.

**Undo is proven on the project, not on the command**: after `apply` then
`revert`, the project is the one from before, exactly — texts and removed
subtitles put back in their place, including when the removal is
discontinuous.

**End to end, through the real binary**, on a file that carries the cases that
matter: a lone annotation, an annotation straddling a line break, a one-voice
dialogue, a numeric reference left alone.

*The straddling annotation was missing from the fixture, and the review (#110)
caught it.* It was the most useful of the four cases: the only one that comes
from real files rather than from reasoning, and the only one that a
line-by-line reading would break without anything else saying so.

**No test compares two implementations.** What is expected is written in the
test or in the case corpus, never produced by Gaupol.

## Measurements

The phase brings **the first text measurement of the log**. It is taken on the
fixture of #89, which was its reason for being: four thousand subtitles, one in
five of which carries an annotation, in the proportions of real files, and 11.6%
of which are taken away by the removal — the costly case.

| Measurement | What it says |
| :---------- | :----------- |
| `suppression des mentions sur 4000 sous-titres` | what the whole operation costs, command included |

A single one, and through the command: that is what a user triggers. The pure
function alone would be more stable to measure, but would measure less.

## Requirements

New entries in the registry, in the `planned` state until a test cites them:

| ID | Requirement |
| :- | :---------- |
| `CLI-HEARING-01` | `hearing-impaired` removes the annotations between brackets, and leaves parentheses by default |
| `CLI-HEARING-02` | a subtitle that the removal empties is removed from the written file |
| `CLI-HEARING-03` | a purely numeric reference is left as is |
| `CLI-HEARING-04` | the report names the number of changed and removed subtitles |
| `CLI-HEARING-05` | a file without any annotation is written unchanged, code `0` |
| `CLI-HEARING-06` | without an explicit destination, nothing is written and the code is `1` |
| `CLI-HEARING-07` | `--parentheses` also removes the annotations between parentheses |

## Split into issues

ADR 0017 does not appear here: it is written **at framing time**, with this
spec, for the reason ADR 0016 already served — a decision taken at the same
time as the code that consumes it is no longer a decision, it is a justification
after the fact.

| Issue | Subject | Depends on |
| :---- | :------ | :--------- |
| [#101](https://github.com/Guyot-Bertrand/sub-edit/issues/101) | the core: the text rule, and the enlarged case corpus | — |
| [#102](https://github.com/Guyot-Bertrand/sub-edit/issues/102) | the core: the composite command, its undo, and the measurement in the log | #101 |
| [#103](https://github.com/Guyot-Bertrand/sub-edit/issues/103) | the CLI: the subcommand, the report, the manual, the requirements | #102 |
| — | end-of-phase review, then publication of `v0.4.0` | all |

**The rule before the command, and on its own.** That is where all the
difficulty lies — the seam, the emptied lines, the dashes, the tags — and it can
be tested without building anything around it. Mixing them would make a single
issue carry the case corpus and a composition of commands, and the failure of
one would hide the other.

## Open points

| Point | Trigger |
| :---- | :------ |
| widening the dialogue dash to `–` and `—` | a real file that shows one; the corpus has none among 4,874 dashes |
| Gaupol's three other patterns, and configurable patterns | phase 12, which brings the engine and the file format |
| cleaning the translation document | phase 13 — see [`specs/11-traduction.md`](11-traduction.md), D9 |
| applying the operation to a selection | phase 5, which brings an on-screen selection |

## Completion criteria

- [ ] The pure function passes every case of `mentions.cas`, parentheses and
      tags included
- [ ] Undo restores the project from before, exactly
- [ ] The subcommand exists and is documented in the manual
- [ ] Every requirement of the table above is `implemented` and cited by a test
- [ ] The text measurement appears in the performance log
- [ ] ADR 0017 is written
- [ ] The end-of-phase review is done
