# Phase 15 — Internationalisation

Framing of issue [#655](https://github.com/Guyot-Bertrand/sub-edit/issues/655), after the initialisation
[#654](https://github.com/Guyot-Bertrand/sub-edit/issues/654). **This is the first spec written in English**: from this
phase on, issues, specs and ADRs are (decision D7).

## What the roadmap promises, and what is already there

The roadmap asks for the interface in several languages, reusing the 20 locales of Gaupol (GPL); the names and
descriptions of the correction patterns carried over from phase 12; an internationalised help page (the manual that
`Help` opens); and `man` pages for the command line. On 2026-10-09 the maintainer added a second requirement: **the
project documentation is translated into English**, and issues are written in English from now on.

The two requirements meet in the manual, which is at once documentation, the source of the window's help and the source
of the `man` pages.

## Taking stock — what the code says (checked in #654 and while framing)

| Fact | Consequence |
| :--- | :---------- |
| `core` and `cli` include no Qt header (`subedit_cli` links `core` and `platform` only) | `QObject::tr` cannot serve `core/wording/`, which both surfaces share |
| 8 `tr()` calls, all in `grid_analysis_dialog.cpp`; about 350 `QStringLiteral` in `gui`; about 220 literal-carrying lines in `core/wording/` | translation is almost entirely still to do; the exact number of user-visible strings is unknown until the scanner of D10 counts it |
| Neither gettext tools nor `lupdate` are installed here or in `setup-toolchain.sh` | provisioning is part of the work whichever mechanism is chosen |
| Gaupol: plain gettext, **566 messages, no `msgctxt`, no named placeholders, 4 plural messages, 205 mnemonic `_` accelerators**, 20 locales | its catalogues are reusable as they are, with `_` turned into `&` |
| 91 of our 269 window literals (34 %) have an identical msgid in Gaupol's template; **66 % are ours and new** | the seeds help, they do not finish the job |
| Share of Gaupol's template translated per locale: `es`, `fi`, `pt_BR` 99 %; `nl`, `pt` 89 %; `cs`, `pl`, `ru`, `tr`, `zh_CN` 88 %; `fr` 86 %; `de` 71 %; `ca` 52 %; `it_IT` 50 % | counted from the `.po` files, fuzzy entries excluded |
| `Help ▸ Manual` already renders the Markdown installed under `share/subedit/manual` | localised help is a matter of which tree is installed and opened |
| `pandoc` is not installed; the manual's Markdown is a small, constrained subset | the `man` pages need a small converter of our own |
| About **220 000 hand-written words** in 112 files: manuals 72 k, specs 88 k, ADRs 42 k, roadmap 15 k, `CLAUDE.md` 4 k; scripts, `Makefile` and CMake comments are French too (about 17 000 lines of scripts and build files) | translating the repository is the larger half of the phase |

## D1 — One mechanism: gettext catalogues, read by our own code

**Decision.** Messages are kept in GNU `.po` catalogues, compiled to `.mo`, and read at run time by a small reader in
`core/i18n/` (Qt-free), which `core`, `cli` and `gui` all call. Extraction, merging and compilation use the gettext
tools (`xgettext`, `msgmerge`, `msgfmt`) at build time only. [ADR 0042](../adr/0042-gettext-catalogues-read-by-our-own-reader.md).

**Why.** `core/wording/` is shared by the window and the command line and has no Qt, so any scheme built on `QObject::tr`
serves only the window and leaves a second mechanism for the rest. Gaupol's catalogues are `.po` and become seeds with no
conversion beyond the accelerator.

**Why our own reader and not libc `gettext`.** The libc functions translate only if the locale is *generated on the
machine*: on a CI runner or a fresh install without `fr_FR.UTF-8`, a test of the French window cannot run, and a user who
sets `LANGUAGE=fr` gets English silently. The `.mo` format is simple and the `Plural-Forms` expression is a small grammar;
reading them is a few hundred lines that are testable under any locale. This is the one place where the decision costs
code; the first implementation issue ([#658](https://github.com/Guyot-Bertrand/sub-edit/issues/658)) must show the
reader safe on hostile input.

**Dismissed.**

- *Qt Linguist everywhere.* Needs Qt in `core`, which the minimal-executable principle forbids, or a second mechanism.
- *Qt Linguist for the window, something else for the rest.* Two catalogues, two workflows, two sets of tools, to keep in step.
- *Converting Gaupol's `.po` to `.ts`.* Faithful only where strings match (34 %), and it would still leave `core` out.
- *libc `gettext` as it is.* See above.
- *Our own catalogue format.* Throws away the tools translators already use and Gaupol's 20 locales.

## D2 — What a message is

- **The msgid is the English text**, as in Gaupol, so a catalogue is readable without the source and `msgmerge` can match
  a changed sentence by similarity.
- **Arguments are positional** (`%1`..`%9`), the same syntax in `core` and the window, so a translation may reorder them.
  One substitution function serves both; `QString::arg` is not used on translated text.
- **Plurals** use `translatePlural(singular, plural, n)` with the catalogue's `Plural-Forms`, so Russian or Polish get
  their three forms. A sentence with a count never concatenates `"s"`.
- **Accelerators** are written `&` in the source, as Qt wants them; the import turns Gaupol's `_` into `&`.
- **A context** (`translateIn`) is used only to separate two identical English words that translate differently — Gaupol
  has none, so contexts are rare and justified one by one.

## D3 — What is translated, and what never is

| Translated | Never translated |
| :--------- | :--------------- |
| window labels, menus, dialogs, messages | keyboard shortcuts (they stay the Qt key sequences) |
| the command line's `--help`, summaries, narration, errors | `--format json` output: keys, schema, values, exit codes |
| the shared wording of `core/wording/` | file formats' names and identifiers, option names |
| names and descriptions of the correction patterns (D6) | the pattern files and the user's own patterns |
| the manual, the help, the `man` pages | log lines for developers |

**Script output is a contract and does not follow the language**: `LC_ALL=C` yields English everywhere, and `--format
json` is byte-identical whatever the language (the versioned expected files of phase 13 hold it).

CLI11's own diagnostics (a missing required option, an unknown flag) are English. Whether to replace them by our own
messages is decided in [#663](https://github.com/Guyot-Bertrand/sub-edit/issues/663) and written in the manual.

## D4 — Choosing the language

- **Command line and window** read `LANGUAGE`, `LC_ALL`, `LC_MESSAGES`, `LANG`, in the standard order, by our own code
  (D1). An unknown or uninstalled language falls back to English, silently.
- **The window also has a setting**, `interface.language` (`system` by default, or an installed language), in
  `Preferences…`. **It applies at the next launch**: re-translating a live window is not worth its cost, and the dialog says
  so. An unreadable value leaves the default, like every other setting.
- **Every test binary starts in English explicitly**, so no test depends on the machine's environment; tests of another
  language ask for it.

## D5 — Which languages ship

- **English** is the source and always present.
- **French** is the first shipped locale: the seed from Gaupol, then the new strings, translated by Claude and **reviewed
  by the maintainer** (decision of 2026-10-09).
- **The other 19 Gaupol locales are imported as seeds and not installed.** A language ships when it is complete — no
  untranslated and no fuzzy entry — and has been reviewed by someone who reads it. Nobody can review them today; they wait
  in `src/po/` and are reported by the gate of D10 without failing it.
- Qt's own strings (standard buttons, file dialog) come from Qt's catalogues (`qtbase_<lang>.qm`) when installed.

The phase therefore ships **two languages**, with the machinery for twenty.

## D6 — The correction patterns

Pattern files are Gaupol's, read as they are ([ADR 0037](../adr/0037-lire-les-motifs-de-gaupol-tels-quels.md)), and their
titles are English. Their translation lives in our catalogue, keyed by the English title and description and looked up
**when displayed**; the files are never rewritten. A pattern the catalogue does not know — a user's own — shows as
written. [#666](https://github.com/Guyot-Bertrand/sub-edit/issues/666).

## D7 — The language of the repository

**Decision.** Everything is written in English: C++, scripts, build files, documentation, commit messages, pull request
titles, issue titles and bodies, test titles. **French exists as a locale only**: the catalogue `src/po/fr.po` and the
translated manual `docs/i18n/fr/manual/`. [ADR 0043](../adr/0043-english-is-the-language-of-the-repository.md).

This replaces the split of #312 (C++ and test titles in English, everything else in French), which was right while the
product had one language and becomes wrong the moment the repository is a candidate for contributors who do not read it.

**Mechanics.**

- `CLAUDE.md`'s *Language* section is rewritten ([#660](https://github.com/Guyot-Bertrand/sub-edit/issues/660)).
- The comment-language guard becomes repository-wide and refuses French everywhere except the two locale trees, with a
  **descending allowlist** of files not yet translated: a translation issue removes entries, nothing can add one. This is
  the ratchet shape of #312, which worked.
- **Commit messages and PR titles** switch to English with the rule; the grammar of `check-commit-message.sh` is unchanged.
  History is not rewritten.
- **Between the rule and the last translation** the repository is mixed. The allowlist makes it measurable and finite.

**Specified by #660.** The allowlist is `.french-allowlist` (273 files when it was written), held by
`check-comment-language.py` in the `arch` step; its cases are replayed by the `fixtures` step and `verify-gates.sh`
proves on a scratch copy that a French file is refused and that the list cannot grow. Growth is judged against the list
at the merge base with `origin/main`, and skipped with a note when there is none. Besides the two locale trees, three
families are not read, each because its language is not prose of the repository: `CHANGELOG.md`, generated from a
history that is not rewritten; `src/test/data/**`, whose French is the point of the data; and `packaging/patterns/**`,
Gaupol's per-language patterns — the README files of the last two are read. In C++, comments and the messages of Catch2
macros are read, and any other string literal is data.

## D8 — The documentation

**English becomes the source; everything is translated** (decisions of 2026-10-09: all of it, about 220 000 words).

- **Manuals.** The French manual moves, unchanged, to `docs/i18n/fr/manual/`; the English manual is written at
  `docs/manual/`. The French tree then stays in step with the English one, which the structure check of
  [#656](https://github.com/Guyot-Bertrand/sub-edit/issues/656) enforces (headings, tables, code blocks, links).
  Further languages add `docs/i18n/<lang>/manual/` the same way.
- **Everything else** — roadmap, specs, ADRs, `CLAUDE.md`, `README.md`, scripts, `Makefile`, CMake — is translated **in
  place** and not kept in French: there is no second tree to keep in step, and git keeps the French history.
- **Order.** The rule and the guards first (#657, #660, #656); the manuals next, because they feed the help and the `man`
  pages; the top-level documents; then ADRs, specs, scripts and build files, in independent batches that can interleave
  with the code issues.
- **Screenshots.** The captures are taken from the English interface and shared by all manual trees until a language's
  catalogue is installed. One set per language is deferred (open point 3).
- **Translator.** Claude translates; the maintainer reads the English. Identifiers (requirement IDs, ADR numbers, anchors,
  option and key names) never change.

## D9 — Help and `man` pages: one source

The manual is the single source. **The window's help** installs `docs/manual` and each installed language's tree under
`share/subedit/manual/`; `Help ▸ Manual` opens the chosen language, English when the page or the language is missing
([#682](https://github.com/Guyot-Bertrand/sub-edit/issues/682)). **The `man` pages** are generated from the command-line
part of the same Markdown by `src/scripts/md-to-man.py`: `subedit-cli(1)` and one page per sub-command, per language,
installed under `share/man/<lang>/man1` and carried by the `.deb` and `.rpm`
([#683](https://github.com/Guyot-Bertrand/sub-edit/issues/683)). [ADR 0044](../adr/0044-one-manual-feeds-the-help-and-the-man-pages.md).

**Dismissed.** *Two sources* (hand-written `man` pages): they diverge, as two texts always do. *`pandoc`*: one more
dependency for a Markdown subset a hundred-line converter handles, and a converter we can test.

## D10 — A gate that holds

Four checks, each proven by `verify-gates.sh`:

| Check | Issue | What it refuses |
| :---- | :---- | :-------------- |
| scanner | [#661](https://github.com/Guyot-Bertrand/sub-edit/issues/661) | a user-visible literal outside the translation API (a ratchet that reaches zero) |
| stale translations | [#670](https://github.com/Guyot-Bertrand/sub-edit/issues/670) | an installed language with an untranslated or fuzzy entry |
| structure parity | [#656](https://github.com/Guyot-Bertrand/sub-edit/issues/656) | a translated page whose headings, tables, code blocks or links differ from its source |
| language-aware guards | [#657](https://github.com/Guyot-Bertrand/sub-edit/issues/657) | a documentation guard that matches a French word and silently checks nothing on an English page |

Plus a **test harness that opens the window under a second catalogue with no screen** ([#669](https://github.com/Guyot-Bertrand/sub-edit/issues/669)),
which catches an untranslated string and a truncated label the other checks cannot see.

## Settings shared by the window and the command line (#699)

**Issue:** [#699](https://github.com/Guyot-Bertrand/sub-edit/issues/699). Framed first as a standalone spec, then folded
into this phase by the maintainer on 2026-10-12: it is not a phase of its own. Its decisions are numbered S1 to S8 so they
do not collide with D1 to D10. [ADR 0045](../adr/0045-one-settings-file-two-parts.md).

### Objective

Some settings belong to both surfaces: an operation's parameters are the same whether it runs from the window or from the
command line. Before #699 the window remembered the user's values (`Adjust Durations…`, `Correct Texts…`) and the command
line ignored them, so a user who tuned a dialog and then ran `subedit-cli adjust` got other values with nothing to say so.

This work makes the settings file the shared home of the **operation settings**, lets the command line read them, and
gives it the commands to look at and change them. The display preferences stay the window's business.

### Decisions

**S1 — The command line reads the file by default.** `--no-config` switches it off; `--config FILE` reads another file.
The two exclude each other. Chosen by the maintainer over an opt-in flag: a setting made in the window or by `config set`
takes effect at once, and the cost is that one command line can answer differently on two machines. The cost is paid down
by S6.

**S2 — Priority, strongest first: option on the command, then the file, then the core default.** An option always wins.
`--speed off` beats a speed in the file.

**S3 — Two parts in the core, one file, no migration.**

- `Settings` stays one flat struct. `OperationSettings` is a slice of it (`durationAdjustment` and `correction`), read
  through `readOperationSettings`. Splitting the struct would touch every GUI call site and every designated-initialiser
  test for no extra guarantee: the command line is only given the slice. `spellCheck` has no command-line equivalent and
  stays on the display side with geometry, columns, theme, last directory, table share, insert placement, write encoding,
  search options, editor and video. #697 and #698 introduced no new field in `Settings` yet, so the S5 audit finds nothing
  to move for them.
- **The keys do not change.** They are already prefixed by family (`duration-adjust.*`, `correction.*`), so an existing
  file is read as it is.
- On writing, the two parts are two blocks, each under a banner comment. The reader stays tolerant and takes keys in any
  order. No `[section]` headers: they would break the reading of every existing file.
- `readOperationSettings` returns the operation part only. That the command line never reads the display preferences is a
  property of the type.
- ADR 0022's rules hold unchanged: unknown key ignored, unreadable value keeps the default and produces a diagnostic,
  absent option takes the default, an option at its default is rewritten commented, a missing file is not an error.

**S4 — The path is resolved in one place.** `gui::userSettingsPath()` depends on Qt. The resolution moves to
`subedit_platform` (Qt-free: `$XDG_CONFIG_HOME`, else `~/.config`, then `subedit/settings.conf`), and the window and the
command line call the same function. Only the two `main` functions resolve a path and hand it down. Tests receive a path
and never resolve one (`check-config-home.sh`).

**S5 — Which settings are shared.** An audit of every field of `Settings` and every option of every command, written into
the plan before any code moves. A setting is operation-level if a command line option sets the same thing.

**S6 — Reading is announced.** At the first verbosity level a command says which file it read, or that there was none.
With `--format json` the record carries the path read, so a reader can reproduce a result.

**S7 — A catalogue of the operation keys in the core.** One table: key, short description, accepted values, and who uses
it (the command-line commands and the window actions that read the key). The default is not written a second time: it is
derived from what the file writes for a fresh `Settings`, so it cannot drift. The core already defines the keys and the
defaults; the catalogue adds the description and the accepted-values text. A test checks that every key the reader
recognises is in the catalogue and the other way round, so the table cannot drift from the reader. The descriptions are
English, in the core, so that the manual can be checked against them.

**S8 — What the command line honours.** The catalogue carries a `surface` per key: `Both` or `Window only`. `config list`
prints it with the other columns of S7.

| Surface | Keys |
| :------ | :--- |
| Both (13) | the nine `duration-adjust.*` keys (`gap-enabled`, `gap-ms`, `lengthen`, `maximum-enabled`, `maximum-ms`, `minimum-enabled`, `minimum-ms`, `shorten`, `speed`), `correction.human`, `correction.ocr`, `correction.remove-blank`, `correction.line-break.max-lines` |
| Window only | every other `correction.*` key |

`correct` takes `--tasks`, `--code`, `--max-length` and `--language` from its own line on purpose: the length unit
differs and nothing is ticked beforehand. `correction.activations` is not in the catalogue: it is a list the window
manages, and the command line has `--enable` and `--disable`.

For `adjust`, `--speed off` switches the speed off; the reading speed's directions come from the file unless a flag
changes them.

### The `config` commands

`show`, `set` and `unset` act on the user's file, or on `--config FILE`; `list` reads no file. Only operation keys are
accepted. A display key is refused with "display preference, set it in the window"; an unknown key is a usage error.

| Command | Does | Notes |
| :------ | :--- | :---- |
| `config list` | Writes every operation key, whether or not the file defines it, with no value: the key, what it is for, what it accepts (the closed set, or the bounds and the form), its default, and the commands and window actions that use it. | An information command for the user. `--format json` gives one object per key. Reads no file and needs none; works with `--no-config`. |
| `config show` | Writes the path read, whether the file exists, then each operation setting with its value, its source (`file` or `default`) and its default, in every case. | `--format json` gives one object. Display preferences are not listed. |
| `config set KEY VALUE` | Validates `VALUE` with the file's own reader, then rewrites the file atomically. | An unreadable value is a usage error and leaves the file untouched. Display preferences already in the file are kept. |
| `config unset KEY` | Puts `KEY` back to its default, written commented as the format does. | Unsetting a key already at its default succeeds and changes nothing. |

**One catalogue behind all four** (S7). `list` prints it, `show` walks it for the values, `set` and `unset` use it to know
the keys and to validate. A key therefore cannot be settable and undocumented, nor documented and unknown.

`set` and `unset` create the file and its directory when absent. Exit codes are the four of the manual: a refused key or
value is a usage error, an unwritable file is the I/O error.

### Out of scope for #699

- Display preferences on the command line.
- The interface language, which comes from the environment (D4, #658).
- Any setting that has no command line equivalent.
- `correction.activations`, a list managed by the window.

### #699 is done when

- one definition and one default per operation setting, read by the window and by the command line;
- the display preferences are not read by the command line;
- tests cover the priority (option > file > default), `--no-config`, `--config`, a missing or unreadable file, an existing
  file read unchanged, and `config list` (every catalogue key, no value, none missing), `show`, `set` and `unset`
  including their refusals;
- the CLI and GUI manuals say where the file is, what it holds and how a command reads it;
- ADR 0045 records the two parts, the priority and the default reading.

## How the phase proves itself

- every requirement below is cited by a test;
- the window and the command line run under French and English end to end, and `--format json` is identical in both;
- the scanner and the allowlist are at zero at the closure;
- the start-up cost of loading a catalogue is measured and recorded in the benchmark journal ([#684](https://github.com/Guyot-Bertrand/sub-edit/issues/684));
- a value set in the window or by `config set` is the one `adjust` and `correct` use, unless an option overrides it, and
  the end-to-end tests of #699 hold it.

## What the phase does not deliver

- Languages other than English and French **installed**; they are seeded (D5).
- Re-translating a running window (D4).
- Right-to-left layout (Gaupol ships Hebrew; no layout work is planned, and the seed is not installed).
- Per-language screenshots (open point 3).
- Translating the commit history, or the issues already written in French.
- Display preferences on the command line; it reads the operation part of the settings only (S3).

## Gaps with Gaupol

- Gaupol uses gettext through GTK and Python; we read `.mo` ourselves (D1).
- Gaupol's catalogue covers its interface only; its manual page `gaupol.1` is a single file. Ours translates the manual too.
- Gaupol's command line only opens files in the window, so it has no operation settings to share; the shared settings
  file and the `config` commands (#699) are ours.

## Requirements

**Eight**, registered `prévue` in `docs/exigences.md` with this spec; each issue re-reads its own before its code and the
state moves to `implémentée` when a test cites it.

| Identifier | What it promises |
| :--------- | :--------------- |
| `CLI-I18N-01` | the command line's help, narration and errors follow the language chosen by `LANGUAGE`, `LC_ALL`, `LC_MESSAGES`, `LANG`, English when it is unknown |
| `CLI-I18N-02` | `--format json` output is byte-identical whatever the language |
| `CLI-I18N-03` | `LC_ALL=C` yields English |
| `CLI-I18N-04` | `subedit-cli(1)` and one page per sub-command are installed, and list the options `--help` lists |
| `GUI-I18N-01` | the window shows its menus, dialogs and messages in the chosen language, English when it is not installed |
| `GUI-I18N-02` | `Preferences…` offers the interface language; it is kept from one session to the next and applies at the next launch |
| `GUI-I18N-03` | `Help ▸ Manual` opens the manual of the chosen language, English when the page or the language is missing |
| `GUI-I18N-04` | the names and descriptions of Gaupol's correction patterns are shown translated, and a user's own pattern is shown as written |

The shared settings of #699 add no identifier to the registry: what they promise is held by the tests its *done when*
lists, and by the existing `GUI-CONFIG-*` requirements for the reading rules of ADR 0022, which do not change.

## Split

**Seven slices**; the tooling issues of the initialisation ([#656](https://github.com/Guyot-Bertrand/sub-edit/issues/656),
[#657](https://github.com/Guyot-Bertrand/sub-edit/issues/657)) come first and nothing below waits for anything else but
what its column says. Issues #658 to #684 are open in milestone 15, written in English; #699, folded in later, joins them
there.

| Slice | Issue | What it delivers | Depends on | Size |
| :---- | :---- | :--------------- | :--------- | :--- |
| 1 — foundations | [#658](https://github.com/Guyot-Bertrand/sub-edit/issues/658) | the catalogue in `core/i18n`; ADR 0042 | — | L |
| | [#659](https://github.com/Guyot-Bertrand/sub-edit/issues/659) | the gettext toolchain, extraction, install, packages | #658 | M |
| | [#660](https://github.com/Guyot-Bertrand/sub-edit/issues/660) | English as the repository language; guards; ADR 0043 | #657 | M |
| | [#661](https://github.com/Guyot-Bertrand/sub-edit/issues/661) | the scanner and its ratchet | #658 | M |
| 2 — strings | [#662](https://github.com/Guyot-Bertrand/sub-edit/issues/662) | the shared wording of `core` | #658, #661 | L |
| | [#663](https://github.com/Guyot-Bertrand/sub-edit/issues/663) | the command line | #658, #662 | L |
| | [#664](https://github.com/Guyot-Bertrand/sub-edit/issues/664) | the window, part 1: menus, actions, status line, table | #658, #661, #662 | L |
| | [#665](https://github.com/Guyot-Bertrand/sub-edit/issues/665) | the window, part 2: dialogs | #658, #661, #662 | L |
| | [#666](https://github.com/Guyot-Bertrand/sub-edit/issues/666) | the correction patterns | #658, #662 | S |
| 3 — languages | [#667](https://github.com/Guyot-Bertrand/sub-edit/issues/667) | the twenty seeds from Gaupol | #659 | M |
| | [#668](https://github.com/Guyot-Bertrand/sub-edit/issues/668) | the French catalogue, installed | #662–#667 | L |
| | [#669](https://github.com/Guyot-Bertrand/sub-edit/issues/669) | the language setting and the second-locale window test | #664, #665, #668 | M |
| | [#670](https://github.com/Guyot-Bertrand/sub-edit/issues/670) | the stale-translation gate | #659, #667 | M |
| 4 — documentation | [#671](https://github.com/Guyot-Bertrand/sub-edit/issues/671), [#672](https://github.com/Guyot-Bertrand/sub-edit/issues/672) | the command-line and window manuals in English, French kept | #656, #657, #660 | L each |
| | [#673](https://github.com/Guyot-Bertrand/sub-edit/issues/673) | roadmap, README, registry, principles | #660 | M |
| | [#674](https://github.com/Guyot-Bertrand/sub-edit/issues/674), [#675](https://github.com/Guyot-Bertrand/sub-edit/issues/675) | the ADRs | #660 | M each |
| | [#676](https://github.com/Guyot-Bertrand/sub-edit/issues/676), [#677](https://github.com/Guyot-Bertrand/sub-edit/issues/677), [#678](https://github.com/Guyot-Bertrand/sub-edit/issues/678) | the specs, in three batches | #660 | L each |
| | [#679](https://github.com/Guyot-Bertrand/sub-edit/issues/679), [#680](https://github.com/Guyot-Bertrand/sub-edit/issues/680), [#681](https://github.com/Guyot-Bertrand/sub-edit/issues/681) | the scripts, gate steps, `Makefile` and CMake | #660 | M each |
| 5 — help and man | [#682](https://github.com/Guyot-Bertrand/sub-edit/issues/682) | the manual in the interface language | #668, #669, #671, #672 | M |
| | [#683](https://github.com/Guyot-Bertrand/sub-edit/issues/683) | the `man` pages | #671, #672 | L |
| 6 — shared settings | [#699](https://github.com/Guyot-Bertrand/sub-edit/issues/699) | the settings file shared by the window and the command line, the `config` commands; ADR 0045 | — | L |
| 7 — closure | [#684](https://github.com/Guyot-Bertrand/sub-edit/issues/684) | review of the phase, then 0.16.0 | all | M |

The documentation slice does not depend on the code slices and can interleave with them; the batches of ADRs, specs and
scripts are independent of one another.

## Open points

1. **CLI11's own messages** — translate through a custom failure message, or leave in English and say so (#663).
2. **A catalogue per binary or one shared** — `subedit.mo` serves both executables in D1; if the command line's start-up
   cost shows the shared catalogue too large, split it (#684 measures).
3. **Per-language screenshots** — the manual's captures are English; a French set needs the French catalogue installed
   and the capture program run under it. Deferred until a second language is installed.
4. **Reviewing the other 19 locales** — nobody can today; each ships when someone who reads it has reviewed it.
5. **Contributor-facing documentation** (`CONTRIBUTING`, issue templates) does not exist yet; if it is written, it is in English.
