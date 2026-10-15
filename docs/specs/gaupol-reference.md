# Gaupol — feature inventory (reference)

Source: https://github.com/otsaloma/gaupol — local clone in `reference/gaupol`
(not committed), at revision `f433bae` (2026-07-28, post-1.16, 2.0 branch in
progress).

## 1. Original architecture

Two clearly separated Python packages:

| Package | LOC (excluding tests) | Role |
| :------ | --------------------: | :--- |
| `aeidon` | ~9 100 | Pure library: reading/writing/manipulating subtitles. Published on its own on PyPI, no GUI dependency. |
| `gaupol` | ~13 100 | GTK 4 interface (PyGObject), GStreamer video player, dialogs. |
| tests | ~6 600 | pytest suite covering both. |

Translations: 20 locales (`po/`).

This lib/UI split maps directly onto the `src/lib` + `src/exe` split.

## 2. File formats (9)

Each format has a reader/writer (`aeidon/files/`) **and** a markup
converter (`aeidon/markups/`), plus a default header (`aeidon/data/headers/`).

| Format | Ext. | Mode | Header | Detection |
| :----- | :--- | :--- | :----- | :-------- |
| SubRip | `.srt` | time | no | `00:00:00,000 --> …` |
| Advanced Sub Station Alpha | `.ass` | time | yes | `ScriptType: v4.00+` |
| Sub Station Alpha | `.ssa` | time | yes | `ScriptType: v4.00` |
| WebVTT | `.vtt` | time | yes | `WEBVTT` |
| MicroDVD | `.sub` | **frames** | yes | `{123}{456}` |
| SubViewer 2.0 | `.sub` | time | yes | `00:00:00.00,00:00:00.00` |
| MPL2 | `.txt` | time | no | `[123][456]` |
| TMPlayer | `.txt` | time | no | `0:00:00:` |
| LRC | `.lrc` | time | yes | `[00:00.00]` |

Format detection by regex on the content. Conversion between formats (including
time ↔ frames) and markup conversion (italic, bold, colour, font…)
between the syntaxes of each format.

### Encodings and line endings
- Automatic encoding detection (through `charset-normalizer`), the locale's
  encoding, a configurable list of fallback encodings.
- Unix / Windows / classic Mac line endings, which can be forced on saving.

## 3. Data model

- **Subtitle** = start/end position + main text + translation text.
- Positions can be handled in three modes: **time**, **frames**, **seconds**,
  with conversion through the frame rate (8 predefined values from 23.976 to 60.000 +
  custom frame rates).
- **Two documents per project**: `MAIN` and `TRANSLATION` — translation mode
  is a pillar of the tool, not an add-on.
- **Full undo/redo** (`aeidon/agents/register.py`): every action is
  recorded as reversible, with grouping of actions and a readable
  description. Bounded stack.
- Observable model (signals) decoupling lib and UI.

## 4. Core operations (`aeidon` API)

Grouped by "agent" — each agent is a coherent batch of methods.

**Opening / saving** — `open_main`, `open_translation`, `save_main`,
`save_translation`, with alignment of the translation file by subtitle
number or by position.

**Editing** — insertion, removal, merging (`merge_subtitles`), splitting
(`split_subtitle`), bulk replacement of texts and positions, clearing.

**Positions** — shifting (`shift_positions`), affine transformation from
two reference points (`transform_positions`), frame rate conversion
(`convert_framerate`), duration adjustment (`adjust_durations`: minimum/maximum
duration, reading speed in characters/second, minimum gap between
subtitles, lengthen and/or shorten).

**Text** — case (lowercase / UPPERCASE / Sentence / Title Case), italics,
dialogue dashes (add / remove / toggle), automatic line breaking,
correction of common errors, removal of hearing-impaired mentions,
joining/splitting of words through a spell checker.

**Search** — find and replace, plain text or regex, case-sensitive or not,
targeting the main text, the translation or both, on the current project
or on all open projects. *(Corrected at the end-of-phase review of
phase 10 — this inventory added "the selection": search has none,
see [below](#the-search--finderpy-agentssearchpy).)*

`aeidon/parser.py` is the piece that [ADR 0009](../adr/0009-texte-en-chaine-brute.md)
calls the markup-aware parser: it removes the tags, remembers their
positions, replaces in the bare text, then puts them back. **It has no rule for
a match that crosses a tag boundary, only offset
arithmetic**, and the boundary falls back to a place that means nothing
in the new text:

| Text | Searched → put in its place | What Gaupol writes |
| :--- | :-------------------------- | :----------------- |
| `<i>Bon</i>jour` | `Bonjour` → `Salut` | `<i>S</i>alut` |
| `<i>Bon</i>jour` | `Bonjour` → `Bonjour tout le monde` | `<i>Bonjour tout le m</i>onde` |
| `Bon<i>jour</i>` | `Bonjour` → `Salut` | `S<i>alut</i>` |

Measured by running the parser, not deduced from reading it. It is the only point where
phase 10 deliberately departs from it: `src/test/data/textes/recherche.cas`
carries the rule retained — a boundary that cuts a word encloses it — and the cases
that pin it down.

**Clipboard** — copy / cut / paste **of texts only**.
`aeidon/clipboard.py` is a list of strings, one per subtitle, that
`get_string` joins back with an empty line: no positions, no format, no whole
subtitle. Pasting adds rows if some are missing to receive the texts.
*(Corrected after reading, while initialising phase 10 — this inventory said
"whole subtitles".)*

**Preview** — generating a temporary file and launching an
external player (MPlayer, mpv, VLC, or a custom command) positioned at the
current subtitle. **Set aside, and not deferred**: the built-in player of
phase 6 takes its place — spec 10,
["What the phase does not deliver"](10-operations.md#what-the-phase-does-not-deliver).

### `adjust_durations`, in order

Read in `aeidon/agents/position.py`, because the order of resolution is what
phase 10 must specify and it is written nowhere in Gaupol: it is
the order of the lines.

**A single pass, and a single bound moved — the end.** The start is
never touched. The four constraints apply one after the other:

| Rank | Constraint | What it sets |
| ---: | :--------- | :----------- |
| 1 | reading speed | `end = start + length / speed`, if `lengthen` and too short, or if `shorten` and too long |
| 2 | minimum duration | `end = start + minimum` |
| 3 | maximum duration | `end = start + maximum` |
| 4 | gap to the next | `end = max(start, next.start − gap)` |

**The last one applied wins, and it undoes the previous one.** The gap can
bring the duration back under the minimum just set, and down to zero. Gaupol
does not report it.

**The length of the text is counted without markup** — `get_text_length` removes what
the format's markup expression recognises, then takes `len`, line break
included.

Three traps, not to be reproduced inadvertently:

- **a minimum of zero is not a minimum of zero, it is no minimum.**
  `domin = minimum and …`: in Python, `0` is false. The maximum has the same
  defect; the gap, tested by `gap is not None`, does not;
- **the last subtitle has no next one**, and receives a hard-coded bound at
  360 000 s, that is a hundred hours;
- the default values of `gaupol/config.py` are minimum 1.5 s **active**,
  maximum 6 s **inactive**, gap 0 s active, speed 15 chars/s, lengthen only.

**The four contradict each other on a real corpus**, and that can be measured without
writing an adjustment: `src/scripts/measure-duration-constraints.py` counts the
subtitles for which no end satisfies two of them. Issue #371.

**Gaupol persists its settings, `subedit` does not — a deviation observed, not decided.**
`gaupol/dialogs/duration_adjust.py` reads its ten values from
`gaupol.conf.duration_adjust` and writes them back there — the four constraints, their
checkboxes, lengthen, shorten, and the target `SELECTED`, `CURRENT` or `ALL` —, hence
from one session to the next. `subedit` keeps them in
`MainWindow::m_durationConstraints`, for as long as the window stays open. The
manual says so; spec 10 does not settle it.

### `format.py` — case, dashes, italics

Each gesture rewrites the texts through `replace_texts`, a history action.
Case and dashes go through the parser (`get_parser`); italics does not,
it works on the stored text with the format's `italic_tag` expression.

| Point | Gaupol | `subedit` |
| :---- | :----- | :-------- |
| case, where it starts | `_change_case_first`: first `\w` of the bare text, the prefix left as is | kept, through `u_isalnum` — D4 |
| case, engine | `str.title`, `str.capitalize`, `str.upper`, `str.lower` | ICU — D4. `title` capitalises after an apostrophe (`L'Été`); ICU's break iterator does not (`L'été`). `capitalize` lowercases the rest: kept |
| case, leading `ß` | `str.capitalize` returns `Ss` — checked by running the method alone | `sentenced` leaves `ß` — a letter that lengthens in uppercase keeps its form. **Deviation**, and the comment that attributed it to Gaupol is corrected (#401) |
| dashes, adding | `add_dialogue_dashes` removes `^[-–—]\s*` then puts `- ` at `^`, under `MULTILINE`: every line, **empty line included** | kept, except the empty line, which gains none — D4 |
| dashes, direction | `_should_add_dialogue_dashes` adds if a line, tags removed, does not start with `-`: **the hyphen alone**, and an empty line counts | all three dashes count, the empty line does not (`wantsDashes`). **Deviation recorded in D4 at the end-of-phase review**: a target made entirely of em dashes gains dashes in Gaupol, and loses them here |
| italics | `toggle_italicization`: italicises if a text does not open, past its other leading tags, with an italic tag; `italicize` removes every italic tag then wraps the whole text, empty included | one entry, two directions — #365, with no decision in spec 10; an empty line gains no tags |

### `edit.py` — merging and splitting

`merge_subtitles` sorts the indices, builds a subtitle through `new_subtitle` —
start of the first, end of the last, main texts then non-empty translations
joined with `\n` —, removes the indices and inserts it at the first. `split_subtitle`
cuts at `calc.get_middle`: the first half receives text and translation, the
second is born empty. Both group removal and insertion into one action.

**What they lose, read and not assumed.** `new_subtitle` returns a bare
`Subtitle`; its format containers are born lazily (`Subtitle.__getattr__`)
with the class values of `aeidon/containers.py` — `Default` style, layer 0,
empty name, margins and effect in Sub Station Alpha, coordinates `x1`…`y2` at zero in
SubRip, empty identifier and cue settings in WebVTT. Only `Subtitle.copy`
copies them, and neither of the two calls it: **the loss applies to merging and
to both halves of a split.**

| Point | Gaupol | `subedit` |
| :---- | :----- | :-------- |
| rows merged | arbitrary indices: 1 and 3 merge without 2, which the result covers | a neighbouring block, received as an interval — D5 |
| texts, translation | non-empty, joined with `\n` | kept — D5 |
| format fields, merging | reset to the defaults | those of the first — D5 |
| splitting, positions and texts | middle; everything to the first, nothing to the second | kept — D5 |
| format fields, splitting | reset to the defaults, on both halves | kept on the first (`first = whole`), the second is born bare. **Deviation recorded in D5 at the end-of-phase review**, which wrote "kept as is" |
| shortcuts | `M` and `S` (`gaupol/actions/edit.py`) | none — D5 |

### The search — `finder.py`, `agents/search.py`

**It does not go through the parser.** `SearchAgent` holds a bare `aeidon.Finder`
and gives it `get_text(doc)`, the stored text: `<i>` is found, and `Bonjour` does not
find `<i>Bon</i>jour`. The deviations of the `parser.py` table, above,
are those of the corrections of `agents/text.py`, which use it — not those of the
search dialog.

| Point | Gaupol | `subedit` |
| :---- | :----- | :-------- |
| options | text or expression, case ignored or not; defaults `regex` false, `ignore_case` true | kept, defaults included — D7 |
| flags | `set_regex`: `DOTALL \| MULTILINE`, plus `IGNORECASE` | the same in ICU — D7 |
| plain text | `str.index` after `lower()` on both sides | ICU pattern `UREGEX_LITERAL`, case folded by ICU |
| text searched | the stored text, tags included | the visible text — D1 |
| replacement | literal for a text; for an expression, `match.expand`: `\1`, `\g<1>`, `\g<name>` | `$0` to `$9`, `\n`, `\$`, `\\`; `\1` puts `1`, no named group — deviation recorded in D7 |
| scope | fields `MAIN_TEXT`, `TRAN_TEXT` and target `CURRENT` or `ALL`; **never the selection**: `_update_search_targets` passes `None` for the indices, and the selection only gives the starting row | the usual target — D7; translation and all open projects, phase 11 |
| wrapping | `wrap` true for `CURRENT`: starts again from the beginning; for `ALL`, moves to the next project and loops over the whole | starts again from the beginning, in both directions (`findNext`, `findPrevious`) |
| `Replace All` | one `replace_texts` per document, grouped if both change | one entry — D7 |
| persistence | `regex`, `ignore_case`, `target`, `fields` in `gaupol.conf.search`; the last ten patterns and replacements in `search/*.history` | the two options, as `search.regex` and `search.ignore-case`. **No history**: a deviation observed, not decided |

## 5. Text correction engine

This is the most "domain-specific" part of the tool, driven by declarative
pattern files (`aeidon/data/patterns/*.conf`), classified by script/language/country
(ISO 15924 / 639 / 3166): `Latn`, `Latn-en`, `Latn-fr`, `Latn-fi`, `Latn-en-US`,
`Zyyy`.

Four families of patterns:
- **common-error** — common errors, classified `Human` / `OCR` (spaces around
  punctuation, quotation marks, OCR confusions `l`/`I`, etc.).
- **capitalization** — capitalisation at the start of a sentence.
- **hearing-impaired** — removal of `[NOISE]`, speaker names, etc.
- **line-break** — break penalties for the line-breaking algorithm.

Pattern format: name, description, classes, regex + flags, replacement,
policy, repetition — translatable.

The **line-breaking algorithm** (`aeidon/liner.py`) is a variant of
Knuth–Plass (boxes / penalties / demerits) adapted to subtitling: maximum
line length, maximum number of lines, length unit (characters or **em**),
non-intervention thresholds.

The whole is exposed in the UI through a multi-page assistant (`gaupol/assistants.py`)
that chains the chosen corrections with a before/after confirmation
screen.

## 6. Graphical interface

**Multiple projects**: tabs, grouped saving, grouped closing, splitting a
project in two, appending a file after another.

**Table view**: columns no., start, end, duration, main text, translation —
can be shown/hidden and reordered. Specialised renderers per column type
(time, integer, float, multiline). Display of line lengths,
zebra stripes, customisable font, colouring of differences (insertion /
removal / modification) during corrections.

**Built-in video player** (GStreamer, `gtk4paintablesink`): play/pause,
seek forward/back, jump to the previous/next subtitle, playing only the
selection, overlay of the subtitles and of the timecode (font, colour,
alpha, position configurable), audio track selection, volume.

**Video ↔ subtitle synchronisation**: set the start / the end from the video
position, insert a subtitle at the video position, select the
next/previous subtitle from the video position, move the start or the
end forward/back by small increments. This is the heart of the *timing* work.

**Spell checker** (libspelling): dedicated editable dialog,
on-the-fly underlining, personal dictionary, remembered list of
replacements.

**Dialogs**: opening/saving, appending a file, shifting positions,
transforming positions, frame rate conversion, duration adjustment,
insertion, splitting, find/replace, spell checking,
preferences, encoding, language, debugging.

**Persisted preferences** (~30 sections): encodings, editor, video player,
preview, search, correction, light/dark theme.

## 7. Command line

`gaupol [FILES…]` with `-e/--encoding`, `-t/--translation-file`,
`-a/--align-method`, `-v/--video-file`, `--list-encodings`, `--version`.
No batch mode without an interface — scripted manipulation goes through the
`aeidon` library in Python.

## 8. Performance pressure points

Observations to be confirmed by measurement, not conclusions:

- Text corrections apply dozens of regexes to each
  subtitle, in Python, with undo re-recording per action.
- The line-breaking algorithm is combinatorial and runs over the whole file.
- The GTK table view materialises the data through PyGObject at each
  cell render.
- Opening/encoding detection reads and decodes the file several times.
- Global find/replace: a Python walk over all open projects.

## 9. Licence

GPL-3.0-or-later. An independent rewrite does not take over the code; on the
other hand, the **pattern files** and the **header files** are
data under the same licence — reusing them as they are would impose the
GPL on the project. To be settled.
