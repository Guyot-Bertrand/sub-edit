# Phase 12 — Complete correction engine

Framing of issue [#493](https://github.com/Guyot-Bertrand/sub-edit/issues/493),
after the initialisation [#492](https://github.com/Guyot-Bertrand/sub-edit/issues/492).

## What the roadmap promises, and what is already there

Declarative patterns per script, language and country — common errors classed
Human and OCR, capitalization, remaining hearing-impaired mentions —, line
breaking, and a spell checker with the joining and splitting of words it makes
possible. Plus four deferrals: the three mention patterns and the choice of
engine, from phase 4; PCRE2 or RE2 and hunspell, from phase 0; joining,
splitting and common errors, from phase 10; and the markup-aware parser, which
phase 10 wrote so that it could be borrowed here.

**This is the first phase whose framing finds its expected results already
written.** The initialisation produced two tools, and they change the nature of
the decisions: an engine is no longer chosen on its reputation, it is confronted
with what Gaupol does.

| What is already there | Where |
| :-------------------- | :---- |
| the **24 pattern files of Gaupol**, versioned as they are, with their attribution | [#494](https://github.com/Guyot-Bertrand/sub-edit/issues/494), `src/test/data/motifs/gaupol/` |
| **184 cases** written by hand at the edges of the 72 correction records, and **what Gaupol makes of them**, written by a Python oracle that reproduces its `Finder` | #494, `entrees/`, `attendus/`, `src/scripts/pattern-oracle.py` |
| **26 expected line breaks**, Gaupol's `Liner` ported into the same oracle, in character units | [#495](https://github.com/Guyot-Bertrand/sub-edit/issues/495), `line-break.cas` |
| a **regular expression engine in service**, ICU, the one of search | phase 10, `core/edit/search.cpp` |
| the **markup-aware parser**, `replace` and `transform` | phase 10, `core/text/markup_parser` |
| the **removal of mentions** between brackets and parentheses, by a hand-written scan | phase 4, [ADR 0017](../adr/0017-analyseur-de-mentions-ecrit-a-la-main.md) |
| a **count of changed texts**, not of matches | `rewrittenCount`, phase 10 |
| **multiple projects**, which gives its meaning to the "all open projects" target | phase 11 |

**What is not there.** No reader of pattern files, no translation of Python's
syntax, no expansion of replacements the way Python does it, no length measure
other than `size()`, no spell checker, no assistant.

## Preliminary analysis — what Gaupol does

Read in `aeidon/patternman.py`, `pattern.py`, `agents/text.py`, `parser.py`,
`finder.py`, `liner.py`, `spell.py`, `gaupol/ruler.py`, `gaupol/assistants.py`,
`gaupol/dialogs/spell_check.py` and `gaupol/config.py`; **executed** by #494 and
#495, whose oracles were confronted with the real `aeidon` (360 cases then 26,
without disagreement). The body of #493 gives the table; what follows is what
governs a decision.

**Loading.** `Zyyy`, then the script, the language, the country, concatenated in
that order; a repeated name is placed after the previous one, except with
`Policy=Replace`, which replaces it; `SkipIn` excludes a record from a code. The
shipped files are read, then those of the user (`~/.local/share/gaupol/patterns`);
the shipped `.conf` files, then those of the user. **The `.conf` of `Latn` for
mentions names two patterns that do not exist** — "Song lyrics between
asterisks" and its one-line variant: activating an unknown name is ignored
without a word.

**Application.** The patterns in order, each through a `Finder` that searches
the already modified text. `Repeat=True` loops again **as long as a pass found
something**, not as long as it changed something: a pattern whose replacement
still matches itself never stops. No shipped pattern does so; a user pattern
can.

**The Human/OCR filter is only a display filter.** `CommonErrorPage._filter_patterns`
(`gaupol/assistants.py`, lines 445-450) hides the patterns of the unchecked
class; `correct_texts` (line 430) applies **all** of them, hidden ones included.
Checked by #492 by reading, and no longer on the strength of a delegated
reading.

**Capitalization has a state that crosses subtitles**: a `.` at the end of a
subtitle capitalizes the start of the next one, and the first subtitle of the
document always is. The crossing stops at the gaps in the selection.

**The removal of mentions is followed by seven global clean-ups**
(`_remove_leftover_hi`), and by a removal of the emptied subtitles.

**Line breaking** measures each candidate through `length_func`, and **the
patterns are applied only once per text**, to place their penalties. The hot
spot is the measure: in *ems*, it is the natural width of a `Gtk.Label` in the
default font, relative to an em calibrated so that the lowercase alphabet is
worth 0.55 em per letter.

**The spell checker** goes through libspelling, hence through Enchant, which
chooses the dictionary and keeps the personal word list. On top of it: a list of
`word|replacement` replacements per language, in Gaupol's configuration;
heuristics **for English only** (`-in'` for `-ing`, suffixes
`'s 'll 're 've 'd`, ordinals); two suggestions added at the head for OCR
(`I` → `l`, a digit stuck to a unit). A word splitting that admits it is
"unlikely to work well for all languages".

**The assistant** chains its pages — tasks and target, mentions, joining and
splitting (absent without a spell checker), common errors, capitalization, line
breaking and its settings, progress, **confirmation** —, computes everything on a
copy of each project, presents each changed text with its original, is checked,
unchecked, edited, and applies only what is accepted: **one history entry per
project**, then "Edited N and removed M subtitles".

## D1 — The engine: ICU

**Decided by [ADR 0036](../adr/0036-icu-pour-les-motifs-de-correction.md)**, on
a throwaway measurement whose figures it carries: PCRE2 compiled on the fly is
2.6 times faster, for a difference of 0.13 s to 0.05 s per film that no user
will see; ICU is already in the core, already the search engine, and does not
change behaviour from one distribution to another.

**Fidelity does not settle it either, and that is the discovery of the
framing.** Neither ICU nor PCRE2 reads `\w` as Python does as soon as a letter is
written in decomposed form — ICU never, PCRE2 from its version 10.43 on.
Rewriting `\w`, `\W`, `\b` and `\B` at load time brings ICU down to zero
deviation from Python, on the four pattern types and the 18,563 subtitles of the
private corpus.

**The engine hides behind a core interface**, `PatternEngine` or its
equivalent, with a single implementation. This is the exception the design
principles allow: the variation is identified — the ADR says in which case PCRE2
would come back —, and the interface is what would keep that return local.

**A pattern never runs without a bound.** `RegexMatcher::setTimeLimit` stops a
catastrophic backtracking; the correction of that text by that pattern is
abandoned, and said.

## D2 — The format: Gaupol's, read as it is

**Decided by [ADR 0037](../adr/0037-lire-les-motifs-de-gaupol-tels-quels.md).**
The shipped files are Gaupol's, untouched, and move from
`src/test/data/motifs/gaupol/` to **`packaging/patterns/`**: a single copy, which
the package installs, the oracle reads and the tests read. They are installed in
`share/subedit/patterns`, found **from the executable** like the manual. The
user's patterns are read from `$XDG_DATA_HOME/subedit/patterns`, after the
shipped ones, as in Gaupol.

**Activation is kept in our settings**, by type, code and name; the shipped
`.conf` files are its default values. A setting that names a pattern that has
disappeared is ignored, as Gaupol ignores an outdated `.conf` — and as ADR 0022
does option by option.

**The model is typed.** A record is a `CorrectionPattern`: an enumerated type
(`CommonError`, `Capitalization`, `HearingImpaired`, `LineBreak`), enumerated
classes, enumerated flags, and the fields specific to each type — `Repeat` for
common errors, `Capitalize` for capitalization, `Group` and `Penalty` for line
breaking. No dictionary of strings: that is the counter-example the design
principles cite, and it is precisely Gaupol's.

## D3 — Replacements: converted at load time, applied the way Gaupol does

**Python's template is converted once**, at load time, into a sequence of
pieces — literal text or group number. Everything Python reads in it is
translated: `\1` to `\99`, `\g<n>`, `\g<name>`, octals (`\040` is a space,
`\060` a zero), the escapes `\n` and `\t`; a lone `\0` has already disappeared at
reading, as in Gaupol. The converted template depends on no engine: it is neither
ICU's `$1` nor Python's `\1`, it is a structure.

**Application is that of the `Finder`, not a "replace all".** #494 measured it:
the difference shows only in the clean-up that follows the removal of mentions,
and two cases show it — but an engine that ignored it would fail them. The loop
is written in the core, once, on the interface of D1: search from the end of the
last replacement in the modified text, move forward one character after a
repeated empty match.

**`Repeat` stops when the text no longer changes**, not when a pass no longer
finds anything. **Wherever Gaupol terminates, the result is identical**: a pass
that finds without changing anything would make the next one identical, and
Gaupol would not terminate. Where Gaupol loops forever, `subedit` stops. A bound
also covers the case of a pattern that oscillates between two texts: **a hundred
passes**, beyond which the pattern is abandoned for that text, and said.

> **Clarified by [#499](https://github.com/Guyot-Bertrand/sub-edit/issues/499).** Two bounds
> are added to the hundred passes, because a pattern whose replacement doubles what it finds, under
> `Repeat`, would fill memory before the hundredth: **a text does not exceed 16,384 bytes**
> while a pattern works on it, and **each search is bounded by the engine's time**
> (`kSearchLimit`, a few tenths of a second on a catastrophic pattern). A pattern that
> exceeds any of the three leaves the text **as it was before it** — a half-done replacement
> is worth less than no replacement —, and says so, naming the text. The engine also
> reads `\n` alone as the end of a line (`UREGEX_UNIX_LINES`), which is what `.`, `^` and `$` know
> of the line in `re`.

> **Clarified at the end-of-phase review.** "And says so, naming the text" holds **in the core
> only**: `PatternFailure::text` carries the index of the abandoned text. The confirmation page
> names the pattern and the reason **only once**, however many texts it failed on
> (`correction_confirmation_page.cpp`) — the manual says so. The faulty text is therefore read nowhere
> on screen; the command line of phase 13, which does not have this page, will be able to name it.

## D4 — Human and OCR: a real filter

**Unchecking a class removes its patterns from the application**, and not only
from the list. Gaupol's behaviour — hidden patterns that are applied all the
same — cannot be defended by any reading of its interface: an unchecked "OCR"
box says "do not correct OCR errors". This is an intended deviation, recorded
below.

A record that carries both classes is applied if either of them is checked —
Gaupol's rule for display, which becomes the rule for application.

## D5 — The measure: injected, and cached

**A core interface, `LineMeasure`, two implementations.**

| Implementation | Where | For whom |
| :------------- | :---- | :------- |
| in characters: code points, markup removed, line break counted as a space | core | the tests, the command line of phase 13, and the "characters" unit of the window |
| in *ems*: the width in the application's font, through `QFontMetricsF`, relative to an em calibrated as Gaupol does — the lowercase alphabet is worth 0.55 em per letter | `gui` | the "ems" unit of the window |

**Gaupol's calibration is taken over as it is**, with its oddity: the em is not
the typographic one, it is a unit where an average letter is worth 0.55.
Gaupol's defaults — **24 long, 3 lines, in ems** — only make sense in that unit,
and keeping them requires keeping the unit.

> **Clarified by [#502](https://github.com/Guyot-Bertrand/sub-edit/issues/502).**
> `LineMeasure` (the core) and `CharacterLineMeasure` (`get_char_length`:
> code points, `\n` counted like any other character since a count of code
> points does not see the difference). `CachedLineMeasure` wraps any
> `LineMeasure` in a string → length table — **injected**: the caller decides
> whether to wrap it or not, as it chooses the pattern engine. The line breaker
> (Gaupol's `Liner`, ported line by line from the oracle already confronted with
> the real Gaupol by #495) knows only the interface: the 26 cases of
> `line-break.cas` pass without deviation, `essai` included. **The cache does
> not show on the benchmark for the character measure** — counting code points
> is already too cheap for a hash table to pay back its own lookup; both figures
> are in the PR, and `line_breaking_bench.cpp` explains why they look alike. It
> is the *ems* measure of #503, behind `QFontMetricsF`, that has a real reason to
> be cached — the same interface will carry it without changing anything in the
> line breaker.

**What is cached: the length of a string, for the time of a line breaking.**
The line breaker measures each line of each candidate, and the same lines come
back from one candidate to the next. A string → length table, living for the
time of one operation on a document, is enough; it does not survive the
operation, so that a font change never makes a length outdated. Gaupol memoizes
by object identity — #495 learned that the hard way —, which a table by value
does not have to reproduce.

**The tests of the ems measure** are authoritative only under the settings of
[ADR 0024](../adr/0024-captures-engendrees-et-ou-elles-font-foi.md): headless
platform, `DejaVu Sans`. Elsewhere, they exercise the line breaker with a test
measure of fixed widths, which says what the computation does with a length
without depending on a font.

> **Clarified by [#503](https://github.com/Guyot-Bertrand/sub-edit/issues/503).**
> `EmsLineMeasure`, in `gui`, receives its font in its constructor — never
> `QApplication::font()` on its own, the same rule as `CachedLineMeasure`: the
> caller passes the application's font when that is what it means, a test passes
> `DejaVu Sans`. The calibration measures the lowercase alphabet once,
> `QFontMetricsF::horizontalAdvance`, relative to 0.55 × 26. The tests refuse to
> conclude under a fallback font, as `subedit_screenshots` refuses to take
> pictures — `iiii` and `MMMM` differ in ems, a `breakLines` on both shows it.
>
> **The benchmark required a `QApplication` that `subedit_bench` did not have**:
> `QFontMetricsF` does not answer without a font database. The binary now carries
> its own `main`, under `offscreen`, exactly the pattern already written for
> `subedit_gui_test` — a single binary, as `record-bench.sh` wants, and the two
> `gui/` benchmarks already there, which did without, gain from it without losing
> anything. The cache shows this time: that is what #502 had announced.

> **Written at the end-of-phase review.** **The cache is not wired into the window.**
> `CachedLineMeasure` exists in the core, exercised by its tests and by both benchmarks, but the assistant
> builds a bare `EmsLineMeasure` (`correction_controller.cpp`): what D5 called "a string → length table,
> living for the time of one operation" is never placed in front of `QFontMetricsF` in the shipped
> program. The benchmark measures the line breaking of 4,000 subtitles in *ems* **with** a cache only — the
> version without has no record —, so the gain to be expected is not quantified. Axis submitted to the
> decision of the review, see the end of this document.

> **Clarified by [#526](https://github.com/Guyot-Bertrand/sub-edit/issues/526).**
> **The length of lines in the table** — the ruler of `gaupol/ruler.py` and the margin of
> `gaupol/view.py`, `GUI-EDIT-04`. **What Gaupol does, and what #526 takes over identically:** three settings
> (`editor.length_unit`, `editor.show_lengths_cell`, `editor.show_lengths_edit`); in a **text cell**,
> each line followed by its length, `[N]`, in smaller type; in the cell's **editor**, a strip on the
> right that shows the length of each line, aligned with it and as wide as the largest number. The
> length is that of the line **without its markup**, **rounded down**, in *ems* (default) or in
> characters; in *ems*, it is the measure of #503, unchanged. **An empty line has no length in the
> cell** (Gaupol does not write `[0]` after nothing), and **an empty text has none in the strip**.
>
> **What #526 decides.** The settings are `editor.length-unit` (`em` or `characters`, default `em`),
> `editor.show-lengths-cell` and `editor.show-lengths-edit` (true by default), in `EditorSettings`, read and
> written like their neighbours (commented default, unreadable value: default and diagnostic). They are set
> in `Edit ▸ Preferences…`, where Gaupol puts them too; **the unit is greyed out when neither box is
> checked**, and the window applies the choice live to the table. **This is not the assistant's limit**:
> `editor.length-unit` and `correction.line-break.in-ems` are two settings, like `editor.length_unit` and
> `text_assistant.length_unit` in Gaupol.
>
> **Four pieces.** In the core, `lineLengths(measure, text, vocabulary)` (`line_lengths.hpp`, without Qt) returns
> the length of each line: markup is removed through `decodeAs` then `plainTextOf`, **the reader the whole
> application shares** and not one more pattern, in the vocabulary of the format of the document whose column
> is the text; rounding down happens here. In `gui`, `LengthMeasures` builds the measure and its cache,
> **bounded by the lifetime of the font and the unit** — changing either builds a new one, and a measure
> already handed to a delegate stays valid as long as it holds it —: this is the cache of D5 at last placed in
> front of `QFontMetricsF` in the shipped program for the table, the cells being repainted constantly.
> `TextDelegate` receives two sources (`LineLengthSource`, like `SpellCheckerSource`), one for the cells
> and one for the editor, **and resolves nothing itself**: the window answers "nothing" when the setting is
> false, and the delegate then paints as before. Finally `SubtitleEditor`, moved out of `cell_delegates.cpp` so
> that a test can see it, carries the strip (`showLengths`), redrawn on typing and on scrolling.

> **Clarified by [#527](https://github.com/Guyot-Bertrand/sub-edit/issues/527).** **The cache is also placed
> in front of the assistant.** `assistantLineMeasure(inEms, font)` builds the measure for line breaking: in *ems*, a
> `CachedEmsLineMeasure`, which owns both the `EmsLineMeasure` and its table; in characters, the bare
> `CharacterLineMeasure`, since counting code points costs less than a table (the benchmark of #502).
> **One per computation, never shared**: the table of a `CachedLineMeasure` is not queried from two threads, and
> that of the assistant's computation is queried only by the thread that carries it — the window's table
> (`LengthMeasures`) is another object, on the interface thread. `cachedCount()` says how many texts the
> table holds, and that is what the test reads to tell this measure from a bare `EmsLineMeasure`. The
> benchmark of line breaking in *ems* now builds the measure through this same function: **it measures the path
> the program takes**. The version without a cache still has no record, and will have none: it no longer
> exists in the program.
>
> **The cell is painted by the delegate** when lengths are shown: the style paints the background, the
> anomaly tint, the selection and the focus, then the delegate writes each line and its length, in a smaller
> (0.8) and dimmed font on the same baseline. A line too long for the column is elided
> **before** the number, which always stays visible. The `sizeHint` widens by the room the numbers take and never
> grows in height.
>
> **Deviations:** the issue's criterion said "a line that is too long is marked in the cell". **Gaupol marks
> nothing**: it shows the number, and it is up to the eye to compare it with the limit; #526 follows Gaupol, and
> no marking is added. **The font of the *ems* measure is that of the table**, the
> same as that of the cells and the editor; Gaupol measures with that of a separate `Gtk.Label`.
> **The cache of a measure** grows by one entry per distinct line until the next change of font or
> unit — which also applies to each keystroke in an editor; it is not bounded in size.

## D6 — The spell checker: Enchant, and a feature that switches off without a dictionary

**Enchant 2**, the library Gaupol uses through libspelling. Three reasons, the
first of which settles it:

- **the personal word list is Gaupol's.** Enchant keeps it in its own
  directory, shared by all the applications that use it: the words a user has
  added in Gaupol are recognised by `subedit` from day one, without importing.
  This is iso-functionality in the most literal sense — and it is a Gaupol user
  who asked for this program. **Read in the code, not yet observed**:
  libspelling adds a word through Enchant's dictionary, and the spell checker
  issue checks it on a machine where Gaupol has added a word, before the
  decision is built on it;
- **the dictionaries are the system's**, whatever their engine — hunspell,
  nuspell, aspell, voikko for Finnish —, without our writing their discovery;
- a stable C API, packaged everywhere, Windows included.

**hunspell directly**, which phase 0 named, is ruled out: we would have to write
the dictionary lookup, keep a personal list of our own — which Gaupol's words
would not reach — and give up the languages it serves poorly. **nuspell**: same
dictionaries, same gaps, rarer packaging. **A system service**: there is no
portable one.

**Behind a core interface, `SpellChecker`**, and a double for the tests, whose
form #492 had deferred to here: a dictionary written in the test, a list of
words and their suggestions. No test depends on the dictionaries installed on
the machine that runs it, by the rule that already forbids reading
`reference/`.

**Without a dictionary, the program works and the feature switches off**, as was
done without `ffprobe`: the `Check Spelling…` entry and the joining and splitting
page stay visible, **greyed out**, and say why — "no dictionary for fr" —, where
Gaupol removes the page without a word. Enchant missing at startup does not
happen: it is a dependency of the package.

**What we take over from Gaupol's spell checker, identically**: the English
heuristics, the two OCR suggestions, the replacement list per language — in our
configuration directory, `spell-check/<language>.repl`, without reading Gaupol's
—, and its word splitting. The latter admits it is imperfect; it remains
Gaupol's as long as no case catches it out, because joining and splitting depend
on it directly.

**The dependency is the only one of the phase**, and it is paid three times:
`find_package` in CMake, `Depends` of the `.deb` and `Requires` of the `.rpm`,
and the installation in the CI and in `fedora.yml`. `subedit-cli` pulls it in,
since the spell checker is in the core and phase 13 will join and split words on
the command line.

> **Clarified by [#507](https://github.com/Guyot-Bertrand/sub-edit/issues/507).**
> **Observed, no longer only read**: on this machine, Gaupol's `SpellChecker`
> (1.11, through Gspell) writes an added word into `fr_FR.dic`, in Enchant's
> configuration, and `enchant-2` immediately holds it correct — not a single
> Gaupol directory, no importing. The upstream repository goes through
> libspelling, which ends at `enchant_dict_add` in the same way; both of Gaupol's
> engines reach the same list. The observation took place in a temporary
> configuration directory, never in the user's.
>
> **Two interfaces, and `SpellChecker` is not one of them.** `SpellDictionary`
> (check, suggest, add to the personal list) and `SpellProvider`
> (the languages, opening a dictionary) are abstract; `SpellChecker` is a
> class, what Gaupol puts on top of a dictionary — the English heuristics, the
> two OCR suggestions, the replacement list, the words ignored for the session.
> That upper layer is identical for any dictionary and it is what the tests
> target: the double (`WordListSpellProvider`, in the core like
> `InMemoryFileSystem`) is a dictionary written in the test, and
> `EnchantSpellProvider` the real implementation, the only one that includes
> `enchant.h`.
>
> **The "no dictionary" answer is a `std::unexpected`** —
> `NoDictionary{language}` — and its sentence, `noDictionaryFor`, lives in
> `core/wording/counts.hpp`: "no dictionary for fr", which the window will show.
>
> **Deviations from Gaupol, and they are stated**: a line of a `.repl` without a
> vertical bar is ignored, where Gaupol reads it as a one-element pair that
> fails at the first suggestion; the whitespace of a line is stripped as ASCII,
> not as Unicode. **Kept as it is, defect included**: the class `[0,4-9]` of the
> English ordinals contains a comma that Gaupol probably did not intend — the
> word splitting never yields one, it only counts for a direct call. The word
> splitting follows Python's `\w` through `u_isalnum` and ICU's underscore; the
> positions returned are UTF-8 bytes, not code points.
>
> **The configuration directory is an argument**, never resolved in the core:
> `spellReplacementFile(directory, language)` returns
> `spell-check/<language>.repl`, and it is the caller — the window, one issue
> later — that knows where it is. The only test that touches the real Enchant
> moves `ENCHANT_CONFIG_DIR` into a directory of its own, and
> `check-config-home.sh` now watches `~/.config/enchant` along with
> `~/.config/subedit`.
>
> **The dependency**: `pkg_check_modules(enchant)` in CMake, `libenchant-2-2`
> in the `.deb`, `enchant2` in the `.rpm`, `libenchant-2-dev` in the list of
> `setup-toolchain.sh` — which the CI and `fedora.yml` read, without a second
> list — and `check-installation.sh` checks that each of the two packages names
> it.

> **Clarified by [#509](https://github.com/Guyot-Bertrand/sub-edit/issues/509).**
> **Two windows, as in Gaupol**: `Tools ▸ Check Spelling…`, the walk through the
> unknown words, and `Tools ▸ Spell-Check Settings…`, the language, the target
> and the document — Gaupol's `LanguageDialog`. **The second is always enabled**
> as soon as a project is open: it is the one that gets out of a language without
> a dictionary, without which the first, greyed out, would be a dead end. Without
> a dictionary, `Check Spelling…` is switched off and its tooltip and its status
> line say `no dictionary for` followed by the code (GUI-SPELL-02, cited a second
> time here). The three settings are kept under `spell-check.language`,
> `spell-check.target` and `spell-check.document` (Gaupol: `language`,
> `target`, `field`); an unknown value gives the default and a diagnostic, as
> for `correction.*`. The size of the window is not kept.
>
> **Closing midway applies what has been done — and that is a deviation from
> Gaupol.** Kept are the texts already left **and** the gestures (replace,
> replace all, join) already made on the current text; Gaupol, for its part,
> loses the current text. What does not count: edits typed in the text area and
> not validated by `Save and Resume`, and the rest of the walk, never reached.
> The walk (`SpellCheckWalk`, without Qt) touches no project: it returns
> `ProposedCorrection` values, and the controller passes them through
> `applyCorrections`, hence **one history entry per project**, without a new
> command. The replacement list is written on closing, under the directory
> received from the window (ADR 0022); a write failure is stated, it does not
> cancel the corrections. Each unknown word brings back the project's tab and
> selects the row.
>
> **Corrected at the end-of-phase review.** "**Always enabled** as soon as a project is open" was
> broader than the code: `Spell-Check Settings…` switches off on a document **without subtitles**, like
> `Correct Texts…` and `Check Spelling…` (`main_window.cpp`, `refreshTabActions`). What is specific to it is
> that it **does not depend on the dictionary** — it is what allows leaving a language that has none —, and
> not that it stays enabled on an empty project, where there is nothing to check.

> **The label `Chec&k Spelling…`**: `C` is already the accelerator of `&Correct
> Texts…`, and two entries of the same menu cannot share it; `k` is free in
> `Tools`. **GUI-SPELL-01 is implemented**: ignore, ignore all, add, replace,
> replace all and join are held on the `WordListSpellProvider` double, never on
> the machine's dictionaries.

> **Clarified by [#525](https://github.com/Guyot-Bertrand/sub-edit/issues/525).**
> **Spelling as you type** — Gaupol's `spell_check.inline`. **The setting** is
> `spell-check.inline` (`SpellCheckSettings::inlineCheck`), **false by default,
> as in Gaupol**, read and written like its three neighbours; an unreadable value
> gives the default and a diagnostic. **The box** `Check spelling while
> editing` is in `Spell-Check Settings…` — Gaupol puts it in its preferences, but
> the language it uses is that of this window, and `subedit` has no preferences
> window (see the table of deviations). **When it is checked and a dictionary
> exists** for the configured language, an unknown word is **underlined with a red
> wave while typing**, in the editor of both text columns, `Text` and
> `Translation`; **without a dictionary, or without a provider, the editor stays
> silent**, without failure.
>
> **Three pieces.** In the core, `misspelledSpellRanges` (`spell_ranges.hpp`,
> without Qt) returns the ranges, in UTF-8 bytes, of the words that
> `SpellChecker::check` rejects given their previous and next character: **the
> rule of `SpellCheckNavigator::next`**, so that the underlining and the walk of
> `Check Spelling…` agree — a test checks it by running both on the same text. In
> the interface, `SpellHighlighter` (a `QSyntaxHighlighter`) converts these bytes
> into UTF-16 units and applies `SpellCheckUnderline`; it redoes each block that
> changes, so the underlining follows the typing. **The delegate resolves neither
> path nor language** (ADR 0022): `TextDelegate` receives from the window a
> spell checker source (`SpellCheckerSource`), which it queries for each editor
> opened. **The window** opens the spell checker of the configured language
> (`resolvedLanguage`, `openSpellChecker`, the replacement list read only),
> renews it when the provider or the settings change, and hands it out shared: an
> editor already open keeps its own until it closes.
>
> **What the box does not do**: it applies only to editors opened after it, and
> the editor's context menu offers no suggestions. **GUI-SPELL-04 is
> implemented**: core, settings, settings window, editor and window end to end,
> on the `WordListSpellProvider` double.

> **Clarified by [#530](https://github.com/Guyot-Bertrand/sub-edit/issues/530).** **Four findings of the review
> of #509, two fixed and two dismissed.** *Fixed*: **the replacement list no longer grows on each silent
> application** of a "Replace All" — `addReplacement` moves an already known pair to the end of the list
> instead of repeating it, which the file already did on writing; **an empty configuration directory no
> longer gives a relative path** — `spellReplacementFile` then returns an empty path, which is neither read nor
> written, where `spell-check/<language>.repl` would have landed in the current directory. *Dismissed*: **`Save and
> Resume` rereads from the start of the text**, and a word simply ignored is reported again — that is what
> Gaupol does, whose `reset(text)` sets the position back to zero and whose `ignore` is not kept (only
> `ignore_all` is, and it survives); the manual already says so ("restarts the walk from its beginning").
> *Dismissed*: **the walk's `Project*` pointers that nothing protects** — the dialog is modal, no user gesture
> can close a project while it runs, and the program closes none on its own; one more guard would protect a
> state no path reaches, and closing **after** the dialog is already covered (`pageOwning`, and its test).

## D7 — The mentions of phase 4: the scan stays, the engine takes the rest

**The hand-written scan keeps brackets and parentheses.** ADR 0017 had set its
trigger, and it fired; but its reason still holds: the seam is **local to the
removal site**, and an expression pass would make it global. The thirty-nine
cases of `mentions.cas` describe a better rule than Gaupol's seven clean-ups,
and the program has kept it since phase 4.

**The four other records go through the engine** — lyrics between hash signs,
over several lines and over one, speaker's name before a colon, in capitals or
not —, **followed by Gaupol's clean-ups**, because that is what the expected
results of #494 describe and those patterns have no rule of our own.

**In the assistant, the mentions page shows Gaupol's six names.**
"Sound in brackets" and "Sound in parentheses" control the scan there; the
other four, the engine. A mentions pattern dropped in by the user goes through
the engine. All six are **unchecked by default**, as in Gaupol.

**The `Remove Hearing-Impaired Mentions…` entry of phase 4 does not change**,
nor does the `hearing-impaired` subcommand: they remove brackets and
parentheses, as before. The assistant is the place for the six patterns; the
direct entry is the short gesture it used to be.

**What phase 4 already said and remains true**: the rule lives behind a free
function, `withoutHearingImpaired`, which none of these decisions touches.

> **Clarified by [#500](https://github.com/Guyot-Bertrand/sub-edit/issues/500).**
> The order kept in the core, `correctHearingImpaired`: the scan first, then the
> four engine patterns in the order of the cascade, each only once
> — `HearingImpairedFields` carries no `Repeat` —, then Gaupol's seven
> clean-ups **if and only if an engine pattern changed the text**; the scan does
> not need them, its own seam being already local.
> **Two cases of `hearing-impaired.cas` remain out of agreement, knowingly**: a
> line reduced to a lone quotation mark, next to a mention removed by the scan.
> Gaupol's seven clean-ups would empty it (`^\W*$` then the empty line); the
> scan, true to its local seam, leaves it. This is the deviation ADR 0017 had
> announced, measured here for the first time — `mentions.cas` did not show it,
> for lack of a case where the affected line carried only punctuation already
> unrelated to the mention.

> **Clarified by [#528](https://github.com/Guyot-Bertrand/sub-edit/issues/528).** **The two boxes without effect
> no longer exist.** The end-of-phase review had read it in the code, a red test confirmed it: the
> list of `Latn` patterns on the `Mentions` page offered, among the others, a `Sound in brackets` box and
> a `Sound in parentheses` box — the two records of Gaupol's file, which the engine sets aside by their
> name since the scan plays them alone. They could be checked without changing anything, next to the two real
> boxes of the page. `isScanOnlyPattern` (core, `hearing_impaired_correction.hpp`) is now **the only
> definition** of these two records, read by the engine and by the list, which no longer offers them. A
> pattern of another kind that carried the same name remains an ordinary box. The distinction between brackets
> and parentheses, two boxes for a single scan, is not taken over: no issue asks for it.

> **Clarified by [#529](https://github.com/Guyot-Bertrand/sub-edit/issues/529).** **A pattern name with a comma
> no longer makes the other settings get lost.** Gaupol's pattern format allows a comma in a name, and
> `correction.activations` is a comma-separated list, read all or nothing: a single such name
> made everything rejected, with a diagnostic, and all the overrides went away. The comma and the
> backslash are now written preceded by a backslash in an entry; a backslash
> followed by anything else stays a backslash, so that a file written before is not read differently. **And the
> all-or-nothing rule falls for this option alone**: the entries are independent overrides, each
> naming its pattern, and an unreadable entry costs only its own — the diagnostic carries its text. It
> remains the rule for column orders, which are a permutation and not a list of choices.

## D8 — The assistant, and what the command line expects of it

**An assistant like Gaupol's**, `Tools ▸ Correct Texts…`, with the same order of
pages, and first of all **the confirmation page**: it is what makes automatic
correction a proposal rather than a fait accompli. Each changed text is shown
there with its original, the difference marked; it is accepted, rejected or
edited; `Mark All` and `Unmark All` check everything or nothing; "Remove all
blank subtitles" is a box, checked by default. `Preview`, which in Gaupol
launches an external player at the chosen subtitle, here places the built-in
player on that subtitle when the project has a video — the answer phase 11
already gave for the toolbar.

**Computation and application are in the core**, the window only shows: one
function takes the tasks, their settings, the projects and the target, and
returns the list of changes — project, subtitle, original text, proposed text —
**without touching any project**; another applies an accepted list, **one
composite command per project**, hence one history entry per project. This is
what makes the assistant testable without a window, and phase 13 writable
without it. Gaupol copies each project to do its computations on it; we do not
have to, since corrections are functions of texts.

**The target is Gaupol's** — the selection, the current project, all open
projects — **and the document is chosen** on the first page, text or
translation, the translation being offered only if a project carries one. The
choice of phase 11 — the document of the current column — does not apply here:
a multi-page assistant is not launched "from a column".

**The report counts changed texts**, never matches: #492 showed it,
`'[Il][Il]\b` → `'ll` matches a correct text and returns it as it is. The
sentence lives in `core/wording/counts.hpp`, since phase 13 will say the same.

**What the assistant keeps**: the checked tasks, the target, the document, the
script, the language and the country of each type, the classes, the line
breaking settings, the blank subtitles box — in the settings of ADR 0022.

**The command line waits for phase 13**, as translation waited for it in
phase 11. Nothing the phase writes is specific to the window, except the ems
measure and the pages themselves: phase 13 will find the computation, the
patterns, the spell checker and the sentences in the core, and will only have a
grammar to write — `correct`, with its tasks, its language, its line breaking
settings in characters. Confirmation, for its part, has no command-line
equivalent; phase 13 will say whether a mode that writes the proposed changes
without applying them replaces it.

> **Clarified by [#504](https://github.com/Guyot-Bertrand/sub-edit/issues/504).**
> Four tasks, not five: `CorrectionTask` — mentions, common errors,
> capitalization, line breaking — without joining or splitting, absent from the
> core as long as no spell checker exists to feed them (D6, #508). The order is
> Gaupol's, and a subtitle that a mention empties plays no part in the
> following tasks — the following tasks never see it.
>
> **`proposeCorrections` does not resolve the target.** It takes a list of
> `CorrectionTarget{project, selection, document}`, already decided — the
> selection, the current project or all open projects turned into a concrete
> list is the work of whoever calls, a window later (#505). A second function,
> `applyCorrections`, takes an accepted subset and returns a `CompositeCommand`
> per project, applied but not yet recorded in a history — recording the entry
> is left to the same caller, which holds the histories.
>
> **The rule of #431 on translations reaches all the way here**: a mention that
> would empty a translation writes it empty rather than removing the subtitle —
> `removeHearingImpaired` (phase 4) already did so for its direct gesture,
> `correctHearingImpaired` (the assistant) did not know it yet, since it knows
> nothing of `Document`. It is `proposeCorrections` that carries this rule from
> now on, once for both commands.
>
> **D4, measured and not assumed**: a common-error record is applied if one of
> its classes is checked; none of the three other types carries any. The two
> bracket and parenthesis patterns (D7) are controlled by a single box in the
> core — either **one** or the other being checked is enough to launch the scan —
> rather than by two, `withoutHearingImpaired` only knowing how to handle both
> styles together; really distinguishing them would remain to be done if an issue
> asked for it.
>
> **The settings** (`CorrectionSettings`, in the settings of ADR 0022, the
> sixteenth block after the one for duration splitting): a checked task and its
> code per type, the two classes, the two scan boxes, an open list of activation
> overrides — `<type>:<code>:<name>:<0|1>`, in a single list key — the two line
> breaking settings and the blank subtitles box. An override that no longer
> names anything is never read, which is enough to ignore it without dedicated
> code — the pattern table knows only itself.
>
> **`CommandKind::CorrectTexts`** joins the enumeration; `noticeOfCorrection`
> joins `core/wording/counts.hpp`, Gaupol's very template — "Edited N and
> removed M subtitles" — and counts texts, never matches, by construction: a
> match that returns an unchanged text never gets past `proposeCorrections`.

> **Clarified by [#505](https://github.com/Guyot-Bertrand/sub-edit/issues/505).**
> The three menus of a task page — script, language, country — are populated
> with what the pattern catalogue actually carries for that type, never from an
> external language registry: `PatternCodeSelector` cascades on `catalogue`,
> not on an ISO list.
>
> **The changed part of a diff is marked in bold, never in colour** —
> `correctionDiffHtml` puts `<b>` around each `DiffSpan` marked as changed. A
> tint would have behaved badly under both palettes, and in any case
> `QStyledItemDelegate` paints a cell's `Qt::DisplayRole`, not its font colours
> per range.
>
> **`applyCorrections` composes without applying**, rather than adding to
> `Session`/`History` a method that would record an already applied command: it
> returns a `CompositeCommand` per project, and it is the caller — the window's
> controller — that passes it through `Session::apply`, like any other
> operation. The reason fits in one rule: `Session::project()` returns a `const`
> reference, and a command applied outside `Session::apply` would have bypassed
> it.
>
> **"Abandoning" is the wizard's `Cancel` button, without an interruption
> mechanism of its own**: a computation already launched on its background
> thread is never cut, only never read — **but the progress page waits for it to
> finish before being destroyed**. What the computation references — the engine,
> the catalogue, the projects of its targets — therefore only has to outlive the
> page, never the thread: cancelling blocks, at most for the time the engine
> grants a search. `Back` during the computation lets it finish without waiting
> for it, and its result is discarded; `Next` launches a new one.
>
> **The target and the document are not kept** from one session to the next —
> D8 said they would be, the scope narrowed it: the first page always reopens on
> `Current Project` and `Text`. **The line breaking unit was not kept either,
> and #506 added it** — see below. Only what `CorrectionSettings` carries is
> kept — the checked tasks and their codes, the classes, the activation
> overrides, the line breaking settings, the blank subtitles box.
>
> **An activation override disappears when its box returns to the default**,
> instead of remaining written: for each pattern a page shows, the stored
> override is erased before those that still differ from the default are
> written again (`foldActivations`). Changing code on a page first folds in the
> state of the boxes of the code being left.
>
> **GUI-CORRECT-06 covers the three cases**: a pattern that does not translate or compile, or
> does not terminate (`PatternFailure`), and a file line that cannot be read
> (`PatternCatalogue::diagnostics()`), named under the same table.
>
> **A task page carries no "enabled" box of its own**: it is the one on the first
> page, per task, that alone decides whether the task runs. A second box here
> could have contradicted it.

> **Clarified by [#506](https://github.com/Guyot-Bertrand/sub-edit/issues/506).**
> **Gaupol's skip has its own thresholds**, and #502 had confused them with the
> target limits: `max_skip_length` and `max_skip_lines` are two settings, each
> with its box (`use_skip_max_length`, `use_skip_max_lines`), both at `24` and
> `3` by default — hence the confusion, invisible as long as nobody changes
> them. `breakLines` now takes `std::optional<SkipLimits>`: absent, no skip;
> present, an unchecked box is an infinite threshold, like Gaupol's `32768`.
> `CorrectionSettings` carries the four skip settings, and
> **`proposeCorrections` wires them in** — a subtitle under `24` left as it is
> is now the assistant's default behaviour, which it was not before this issue.
>
> **The unit is kept**, contrary to what #505 had narrowed: `lineBreakInEms`,
> `true` by default, Gaupol's unit — the defaults `24` and `3` only make sense
> in ems (D5). The core carries only the choice, never the measure: the window
> builds `EmsLineMeasure` or `CharacterLineMeasure` at each computation, so that
> no length from a previous measure survives a change of unit. The length skip
> threshold follows the unit, as in Gaupol (`skip_unit_combo` and `unit_combo`
> read the same value). The settings go from sixteen to twenty-one.

> **Clarified by [#508](https://github.com/Guyot-Bertrand/sub-edit/issues/508).**
> **`joinWords` and `splitWords` are in the core**, two functions of texts like
> the others — one text out for each text in, returned as it is when nothing
> applies —, on a `SpellChecker` and Gaupol's navigator
> (`SpellCheckNavigator`), ported line by line. Texts are taken **raw, markup
> included**, as in Gaupol: the `font` of a tag is a word for the splitting.
>
> **One deviation, and it is intended.** Gaupol compares the final text with the
> one the navigator had **at the last misspelled word reached** (`text =
> navigator.text` in the loop): a join followed by a misspelled word left as it
> is is never recorded. Ours compares with the starting text, spaces collapsed;
> the case is a test. For the rest, Python's `istitle` is reproduced by ICU's case
> properties (`Don't` is not a title, nor is `McDonald`), and the `\W+` of the
> neighbour splitting by the same `u_isalnum` and underscore as the word
> splitting.
>
> **The order is Gaupol's, plus one**: mentions, **joining and splitting**,
> common errors, capitalization, line breaking — joining before splitting, each
> on the text the other left. `CorrectionTask::JoinSplitWords` joins the
> enumeration; `proposeCorrections` takes a `const SpellChecker*` that defaults
> to `nullptr`, and **the task does nothing without it** — a missing dictionary
> is not a pattern failure, the window says why. The spell checker is opened by
> the caller, on its thread, with the replacement list of the language (read,
> never written: `replace` in Gaupol adds to it, with no effect on a word already
> processed), and the computation becomes its owner.
>
> **The settings** go from twenty-one to twenty-five: the checked task
> (`correction.join-split.enabled`), joining (`join`, checked), splitting
> (`split`, unchecked) and the language (`language`, a locale code, empty until
> one is chosen). Empty, the language is the system's — exact code, then
> language alone among those Enchant offers, otherwise the system's code
> itself, which the page will then say has no dictionary.
>
> **The page stays and is greyed out without a dictionary** (GUI-SPELL-02): the
> two boxes switched off, the `noDictionaryFor` sentence beneath, the language
> still editable since a dictionary for another one may be there. The
> dictionary provider and the configuration directory of the replacement lists
> are **received by the window**, like the patterns — `setSpellChecking` — and
> never resolved by the controller; `configureFromEnvironment` builds the real
> `EnchantSpellProvider`, a test gives a double or nothing.
> **The `Check Spelling…` entry is left to #509**: GUI-SPELL-02 will be cited there again.

## D9 — Markup: the parser of phase 10, through `transform`

The question of the framing — does the parser hold the offsets the patterns
demand? — has a two-part answer.

**Yes for the mechanism.** A correction goes through `MarkupParser::transform`,
and not through `replace`: `replace` is the half made for what a user types,
which takes the style of everything it touches; a correction is a
transformation, which neither creates nor extends any style. `transform` already
knows how to replace one piece with another of a different length and shift what
follows.

**No for fidelity, as long as a case has not shown it.** Gaupol's parser places
a tag **inside** a match differently: it brings it back to the start of the
removed piece, and guesses whether a tag at the edge is opening or closing from
the space that follows it. Ours keeps it "as far as the new text goes", and
deletes a style that the correction has emptied, where Gaupol leaves the empty
tags. None of the 184 cases of #494 carries a tag: **the deviation is not
measured**.

**Therefore: an issue of the first slice brings the oracle to markup** —
Gaupol's parser ported into `pattern-oracle.py` for SubRip tags, marked-up cases
written by hand where a correction touches a tag — and **decides case by case**:
a disagreement is fixed in `transform`, or recorded as a deviation with its
reason. The only deviation already intended is that of an emptied tag: an
`<i></i>` left in a file is progress for nobody.

> **Clarified by [#501](https://github.com/Guyot-Bertrand/sub-edit/issues/501).**
> Six marked-up cases, in `common-error.entrees`, `capitalization.entrees` and
> `hearing-impaired.entrees`: a tag right after what a correction removes, a tag
> inside a match, a correction that empties it, a tag around each line of a
> multi-line text — a margin, in the sense of Gaupol's parser —, and a
> capitalized word the tag touches. Three disagreements, none fixed in
> `transform`:
>
> - **the tag inside a match** confirms the deviation this paragraph already
>   announced: Gaupol brings it back to the start of the removed piece, then its
>   tag clean-up removes the pair that has become empty; `transform` keeps it
>   stretched over the whole text that replaces it, the rule phase 10 gave it and
>   that `recherche.cas` holds. Going back on it would redo a rule proven
>   elsewhere for a single caller;
> - **the marked-up speaker's name**, in the mentions, is the same deviation:
>   the `<b>` tag that surrounded the removed name survives, stretched over the
>   dialogue dash that replaces it, where Gaupol leaves nothing;
> - **the marked-up capitalized word** is a different deviation, not the one
>   announced above: `SubRip.clean` moves a space stuck to a tag before rendering
>   it, a cosmetic clean-up that nothing in `MarkupParser` carries. A
>   capitalization never moves a tag — it rewrites only one code point, never
>   inside a tag —, so the deviation comes only from this missing clean-up,
>   deliberately: touching up the spacing around tags is the work of none of the
>   three corrections of this slice.
>
> All three are named in the tests that play them
> (`common_errors_test.cpp`, `capitalization_test.cpp`,
> `hearing_impaired_correction_test.cpp`), not only here. The margin
> (`<i>` around each line) and the two other cases show no disagreement: the
> per-tag mechanics of `transform` are enough to reproduce what Gaupol puts in a
> special case.

## D10 — No gate

The issue asked for a gate, **like #435 for multiple projects**, if part of the
phase turned out to be too heavy. None is heavy enough to call for one. #435
carried a reservation the roadmap had written — "the need is not confirmed";
here, each piece is a Gaupol feature that the original user uses, with no
written reservation.

**What could have justified one, and why not**: the spell checker, for its
dependency and its dialog. The dependency is a package present on every
distribution that carries Gaupol; the dialog is the smallest of the phase. It
comes **last**, which is enough: if the phase had to stop earlier, what would be
delivered would stand on its own.

## What the phase does not deliver

Each with a phase or an issue, because a deferral without a recipient ends up
designating a phase already past.

- **The command line for correction** — `correct`, joining and splitting on the
  command line: phase 13, D8.
- **The translation of the names and descriptions of the patterns** — gettext in
  Gaupol: phase 15, with the rest of the interface.
- **Spell checking as you type** — Gaupol offers it in its editing cell
  (`spell_check.inline`). It requires highlighting in the table's delegate,
  which nothing had: **deferred to the end-of-phase review**, which retained
  it — **delivered by
  [#525](https://github.com/Guyot-Bertrand/sub-edit/issues/525)**, see D6.
- **A pattern editor** — Gaupol has none; a file is dropped in.
- **The length of lines displayed in the cells** — Gaupol's ruler
  (`gaupol/ruler.py`, its margin); it would borrow the measure of D5, but it is
  an editing feature, not a correction one: **deferred to the review**, like the
  previous one.

> **Written at the end-of-phase review.** **These two deferrals still have no recipient.** The
> roadmap places them in no phase: `spell_check.inline` does not appear in it, and `ruler.py` is
> cited there only for the measure of D5, not for the display in the cells. The Gaupol inventory
> ([`gaupol-reference.md`](gaupol-reference.md), §6) counts them nonetheless, one as "underlining on
> the fly", the other as "display of line lengths". The review submits them to decision — an
> issue with its phase, or a written deviation with its reason — and the answer is recorded here.
>
> **Two more deferrals had remained without a name**, both in the boxes above:
> **distinguishing brackets from parentheses** in the assistant (#504: "would remain to be done if an issue
> asked for it", whereas the two boxes of the `Mentions` page really are two boxes for Gaupol), and
> **wiring the length cache** into the window (D5, above).

## Deviations from Gaupol

> **Completed at the end-of-phase review.** **The table carried eleven, and all eleven hold**: each one is
> found in the code and in a test (the engine rewrites `\w`, `Repeat` stops at a hundred passes, the
> unchecked class is not applied, the joining page is greyed out, tags are placed as #501 decided…).
> **Ten others had slipped in**, each stated in the box of its issue but none here — the body of the
> issue counted only two more. **Twenty-one deviations, then**, and only one reads as a gap
> rather than as a choice: the size of the checking window, not kept for no written reason. The
> ten are added below the line.

| What Gaupol does | What `subedit` does | Why |
| :--------------- | :------------------ | :-- |
| `\w` and `\b` the way Python reads them | **identically**, by rewriting | without it, neither ICU nor a recent PCRE2 returns the same texts — D1 |
| `Repeat` loops as long as a pass finds something, **possibly forever** | as long as the text changes, **a hundred passes at most**, and says so | identical wherever Gaupol terminates — D3 |
| a pattern that backtracks endlessly **freezes the program** | **stopped**, abandoned for that text, and said | D1 |
| an unchecked class **is applied all the same** | unchecked, it **is not applied** | the box says what it says — D4 |
| activation is written in XML `.conf` files | in **our settings** | ADR 0022, [0037](../adr/0037-lire-les-motifs-de-gaupol-tels-quels.md) |
| brackets and parentheses removed by pattern, then **seven global clean-ups** | by the **scan of phase 4**, with a local seam | ADR 0017, maintained — D7 |
| the joining and splitting page **disappears** without a spell checker | **greyed out**, and says why | saying rather than keeping silent, ADR 0008 — D6 |
| the replacement list in **Gaupol's** configuration | in **ours** | a program does not write into another's — D6 |
| a tag emptied by a correction **stays** | **removed** | D9; the other placements are decided case by case |
| a tag inside a match **is brought back to the start of the removed piece** | **stays stretched over the whole text that replaces it** | D9, decided by #501: the rule `transform` has held since phase 10, for which `recherche.cas` is authoritative |
| `SubRip.clean` **touches up the spacing around tags** afterwards | **nothing does** | D9, decided by #501: none of the three corrections moves a tag, this cosmetic clean-up is the work of none of them |
| *added at the end-of-phase review* | | |
| no bound: a pattern that doubles what it finds, under `Repeat`, **fills memory** | **16,384 bytes** per text and a **time limit** per search; the text stays **as it was before the pattern**, and the pattern is named | D3, #499: a half-done replacement is worth less than no replacement |
| a line reduced to a lone quotation mark, next to a removed mention, **is emptied** by the seven clean-ups | **left** by the scan | D7, #500: the local seam of ADR 0017; two cases of `hearing-impaired.cas` remain out of agreement, knowingly |
| `Sound in brackets` and `Sound in parentheses` are **two patterns checked separately** | **a single scan**: either box launches it, and it removes both | D8, #504: `withoutHearingImpaired` only knows how to handle both styles together |
| `Preview` launches **an external player** at the chosen subtitle | the **built-in player**, when the project has a video; nothing otherwise | D8: the answer phase 11 had already given for the toolbar |
| the assistant keeps **the target and the field** (`text_assistant.target`, `.field`) | it **always reopens** on `Current Project` and `Text` | D8 planned it ("what the assistant keeps: … the target, the document"); #505 narrowed the scope, with no other written reason |
| a `.repl` line **without a vertical bar** is read as a one-element pair, which fails at the first suggestion | **ignored** | #507: Gaupol's reading fails at the first suggestion |
| the whitespace of a `.repl` line is stripped **as Unicode** | **as ASCII** | #507 |
| joining and splitting compare the final text with that of the **last misspelled word reached**: a join followed by a word left as it is is **never recorded** | they compare with the **starting text**, spaces collapsed; the case is a test | D8, #508: intended deviation |
| closing `Check Spelling…` midway **loses the current text** | **applies** the texts left **and** the gestures already made on the current text | D6, #509 |
| the size of the checking window is **kept** (`spell_check.size`) | **it is not** | #509 says so, without a reason; recorded here for not having written it |
| *added by #525* | | |
| the box for underlining while typing is in the **preferences** | in **`Spell-Check Settings…`**, next to the language | `subedit` has no preferences window; the language of the underlining is that of this window |
| underlining goes through Enchant's checker **alone**: neither the colloquial English cases nor the replacement list | it goes through **`SpellChecker::check`**, the English cases included | the underlining and `Check Spelling…` agree on the same words — D6, #525 |
| the editor's context menu **offers suggestions** | **it offers none** | a gap, not a choice: #525 delivers the underlining, not its gestures |
| *added by #526* | | |
| a line that is too long is **marked** in the cell (the issue's criterion) | **nothing is marked**: the number is displayed, as in Gaupol | Gaupol shows `[N]` without comparing it with a limit; the issue's criterion lent Gaupol a marking it does not have |
| the `editor.*` settings are in the preferences, under `Editor` | in `Edit ▸ Preferences…`, **under the theme**, without a tab | `subedit` has a one-page preferences window; the unit is greyed out there if no box is checked |

## Requirements

**Fourteen** — twelve at the framing, all `planned`, `GUI-SPELL-04`, born of [#525](https://github.com/Guyot-Bertrand/sub-edit/issues/525), and `GUI-EDIT-04`, born of [#526](https://github.com/Guyot-Bertrand/sub-edit/issues/526); the registry is fed at the start of each issue.

> **Corrected at the end-of-phase review.** **Twelve, all `implemented`, and each cited by at least one
> test** — a Catch2 tag, which `check-requirements.sh` confronts with the registry: `GUI-CORRECT-01` by two cases,
> `-02`, `-03`, `-04`, `-05` and `-07` by one each, `-06` by two, `GUI-HEARING-03` by one, `GUI-BREAK-01`
> by four, `GUI-SPELL-01` by thirteen, `-02` by eight, `-03` by three. None was born along the way,
> none was abandoned: the table below is that of the framing, apart from the state.

| ID | Requirement |
| :- | :---------- |
| `GUI-CORRECT-01` | the assistant applies the checked tasks to the chosen target — selection, project, all projects — and to the chosen document |
| `GUI-CORRECT-02` | each changed text is shown with its original before being applied, and is accepted, rejected or edited |
| `GUI-CORRECT-03` | applying makes one history entry per project, and the status bar says how many subtitles were modified and removed |
| `GUI-CORRECT-04` | patterns are checked by name, for a script, a language and a country, and the choice is kept |
| `GUI-CORRECT-05` | unchecking Human or OCR removes those patterns from the application |
| `GUI-CORRECT-06` | a pattern that cannot be read, cannot be translated or does not terminate is named, and the others are applied |
| `GUI-CORRECT-07` | the patterns dropped in by the user are added to the shipped patterns |
| `GUI-HEARING-03` | the assistant removes lyrics between hash signs and the speaker's name, in addition to brackets and parentheses |
| `GUI-BREAK-01` | line breaking holds the requested length and number of lines, in characters or in ems, and skips the subtitles that already hold them |
| `GUI-SPELL-01` | checking walks through the unknown words: ignore, ignore all, add to the dictionary, replace, replace all, join to the previous or the next |
| `GUI-SPELL-02` | without a dictionary for the chosen language, the spell checker features are greyed out and say why |
| `GUI-SPELL-03` | the assistant joins and splits words according to the spell checker |
| `GUI-SPELL-04` | an unknown word is underlined while it is typed in the editor of a text cell, when the setting is true and a dictionary exists; without a dictionary, nothing is underlined and nothing fails |
| `GUI-EDIT-04` | each line of a text cell and of its editor shows its length, markup not counted, in ems or in characters according to the setting, and the settings remove it |

> **Added by #525.** `GUI-SPELL-04` brings the total to **thirteen**; it is `implemented`, cited by
> ten cases: the core (three), the settings window (one), the cell editor (four) and the window (five).
>
> **Added by #526.** `GUI-EDIT-04` brings the total to **fourteen**; it is `implemented`, cited by
> fifteen cases: the delegate and the measure (six), the cell editor (three), the preferences (two) and the
> window (four). The core (`lineLengths` and the settings) has no requirement of its own: its tests hold it.

**The core has no requirement**, by the registry's rule: the cases of #494 and
#495 hold it, and they are what its tests will read.

## Split into issues

**Four slices**, from the core to the window, and the spell checker last —
D10.

| Slice | Issue | What it does | What it depends on |
| :---- | :---- | :----------- | :----------------- |
| 1 — the engine | [#498](https://github.com/Guyot-Bertrand/sub-edit/issues/498) | the shipped patterns move to `packaging/patterns/` and are installed; their reader, the cascade of codes, default activation, the user's patterns | — |
| | [#499](https://github.com/Guyot-Bertrand/sub-edit/issues/499) | the syntax translation, the template, the `Finder` loop, common errors; **the 184 cases pass**; the permanent benchmark | [#498](https://github.com/Guyot-Bertrand/sub-edit/issues/498) |
| | [#500](https://github.com/Guyot-Bertrand/sub-edit/issues/500) | capitalization that crosses subtitles, the four mention patterns and their clean-ups | [#499](https://github.com/Guyot-Bertrand/sub-edit/issues/499) |
| | [#501](https://github.com/Guyot-Bertrand/sub-edit/issues/501) | marked-up texts: the oracle carries Gaupol's parser, marked-up cases, and `transform` decides case by case | [#499](https://github.com/Guyot-Bertrand/sub-edit/issues/499) |
| 2 — line breaking | [#502](https://github.com/Guyot-Bertrand/sub-edit/issues/502) | the injected measure, in characters; Gaupol's line breaker ported; **the 26 line breaks pass** | [#498](https://github.com/Guyot-Bertrand/sub-edit/issues/498) |
| | [#503](https://github.com/Guyot-Bertrand/sub-edit/issues/503) | the ems measure, through the application's font, and its cache | [#502](https://github.com/Guyot-Bertrand/sub-edit/issues/502) |
| 3 — the assistant | [#504](https://github.com/Guyot-Bertrand/sub-edit/issues/504) | in the core: computing the changes of several tasks on several projects without touching them, applying an accepted list, the report; the settings | [#500](https://github.com/Guyot-Bertrand/sub-edit/issues/500), [#502](https://github.com/Guyot-Bertrand/sub-edit/issues/502) |
| | [#505](https://github.com/Guyot-Bertrand/sub-edit/issues/505) | the window: `Correct Texts…`, its pages for tasks, mentions, common errors, capitalization, and the confirmation | [#504](https://github.com/Guyot-Bertrand/sub-edit/issues/504), [#501](https://github.com/Guyot-Bertrand/sub-edit/issues/501) |
| | [#506](https://github.com/Guyot-Bertrand/sub-edit/issues/506) | the pages for line breaking and its settings | [#505](https://github.com/Guyot-Bertrand/sub-edit/issues/505), [#503](https://github.com/Guyot-Bertrand/sub-edit/issues/503) |
| 4 — the spell checker | [#507](https://github.com/Guyot-Bertrand/sub-edit/issues/507) | Enchant in the core, behind `SpellChecker`, and its double; the heuristics and the replacement list; the absence of a dictionary; the packaged dependency | — |
| | [#508](https://github.com/Guyot-Bertrand/sub-edit/issues/508) | joining and splitting words, in the core and in the assistant | [#507](https://github.com/Guyot-Bertrand/sub-edit/issues/507), [#505](https://github.com/Guyot-Bertrand/sub-edit/issues/505) |
| | [#509](https://github.com/Guyot-Bertrand/sub-edit/issues/509) | `Check Spelling…`, the dialog | [#507](https://github.com/Guyot-Bertrand/sub-edit/issues/507) |

The phase closes on the end-of-phase review, opened after the last issue.

**[#499](https://github.com/Guyot-Bertrand/sub-edit/issues/499) is the cornerstone**, and the biggest of the phase: it is the one that
confronts the engine with the expected results, and a disagreement it found would
call ADR 0036 into question before anything is built on it. **[#507](https://github.com/Guyot-Bertrand/sub-edit/issues/507) depends on
nothing** and can be done at any time; it is placed at the head of its slice
because the dependency it adds touches packaging, which is better tested early
than late. **[#505](https://github.com/Guyot-Bertrand/sub-edit/issues/505) is the biggest of the window**: a multi-page
assistant, whose confirmation is an editable table with two texts.

> **Written at the end-of-phase review.** Fifteen versions, from `0.12.1` to `0.12.15`. The twelve issues of
> the table were delivered in the order in which it lists them, #498 to #509, one pull request each; the two
> tooling issues, [#494](https://github.com/Guyot-Bertrand/sub-edit/issues/494) and
> [#495](https://github.com/Guyot-Bertrand/sub-edit/issues/495), preceded it, and the framing
> ([#493](https://github.com/Guyot-Bertrand/sub-edit/issues/493)) came after them.

## The issues opened by the review

End-of-phase review, [#523](https://github.com/Guyot-Bertrand/sub-edit/issues/523), on 2026-09-30.
**Two kinds of axes**: those that call for a decision — the deferrals —, and those that looking at
the code as a whole found. None is fixed by the review; each becomes an issue of the milestone
if it is retained, and the issue is recorded in the right-hand column.

| Axis | What the review saw | Issue |
| :--- | :------------------ | :---- |
| checking as you type | a deferral of the spec without a recipient: it requires highlighting in the table's delegate, which nothing has | [#525](https://github.com/Guyot-Bertrand/sub-edit/issues/525), **delivered** (D6) |
| the length of lines in the cells | a deferral of the spec without a recipient: an editing feature that would borrow the measure of D5 | [#526](https://github.com/Guyot-Bertrand/sub-edit/issues/526), **delivered** (D5): `GUI-EDIT-04`, three `editor.*` settings, the length after each line of a cell and a strip in the editor; the measure goes through `CachedLineMeasure`, bounded by the font |
| brackets and parentheses, two boxes for one scan | the assistant presents them as two Gaupol patterns; in the core, either one launches the scan that removes both. **And the `Mentions` page also lists, with the other `Latn` patterns, two boxes of the same name** — `hearing_impaired_correction.cpp` (`isScanOnly`) sets them aside by their name, so that they are checked without effect (read, then **reproduced** by a red test) | [#528](https://github.com/Guyot-Bertrand/sub-edit/issues/528), **delivered** (D7): the list no longer offers them; the bracket / parenthesis distinction stays set aside: no issue asks for it |
| the length cache | `CachedLineMeasure` is wired nowhere in the shipped program (`correction_controller.cpp`); the benchmark does not have the version without a cache in *ems* | [#527](https://github.com/Guyot-Bertrand/sub-edit/issues/527), **delivered** (D5): `assistantLineMeasure`, one table per computation, and the benchmark that measures it |
| the minor findings of #509 | the walk's `Project*` pointers, which nothing protects if a project closed during the dialog; `Save and Resume`, which rereads from the start of the text (`SpellCheckWalk::resumeWithText`); the in-memory replacement list, which grows on each application (`spell_checker.cpp`); an empty configuration directory, which would give a relative path (`spellReplacementFile`) | [#530](https://github.com/Guyot-Bertrand/sub-edit/issues/530), **delivered**: two fixed, two dismissed with their reason, see the box of D6 |
| a pattern name with a comma | `correction.activations` is read all or nothing, the comma being its separator: a single such name makes all the overrides get lost (`settings.cpp`) | [#529](https://github.com/Guyot-Bertrand/sub-edit/issues/529), **delivered**: the comma and the backslash are escaped, an unreadable entry costs only its own setting |
| `Closes #N` without a link | PR #522 carried the line and `closingIssuesReferences` stayed empty; cause unknown, then gone: other PRs, of the same form, linked their issue afterwards — a transient GitHub defect | [#531](https://github.com/Guyot-Bertrand/sub-edit/issues/531), closed without an added check |

**Set aside, with their reason.**

- **`MainWindow`**: 1,478 lines at the close of phase 11, **1,578** today (the header, from 692 to 747).
  `CorrectionSide`, which answers both controllers, weighs sixty-four of them: the logic is in
  `CorrectionController` and `SpellCheckController`, the window has kept only the wiring. One more
  extraction would only move those sixty-four lines.
- **The language ratchet** held: **not one line of French comment left in the C++**, and
  [#325](https://github.com/Guyot-Bertrand/sub-edit/issues/325) has been closed since 2026-09-06. The figures
  of 1,161 lines that `CLAUDE.md` and the roadmap still cite are those of the phase 8 review.
- **The Enchant dependency** is pulled in by the real packages of `0.12.15`: `libenchant-2-2` in the `.deb`,
  `enchant2` and `libenchant-2.so.2` in the `.rpm`. That `dnf` resolves it is tested by `fedora.yml`, every week.
- **The benchmark**: four records for fifteen versions — `0.12.7`, `0.12.8`, `0.12.9` and `0.12.12` —, plus that
  of `0.12.0` which closes phase 11, the machine having been found loaded the rest of the time; the record of `0.13.0` is due at the close
  ([#270](https://github.com/Guyot-Bertrand/sub-edit/issues/270)).
