# Phase 10 — Additional operations

Framing of issue [#370](https://github.com/Guyot-Bertrand/sub-edit/issues/370),
after the initialisation [#369](https://github.com/Guyot-Bertrand/sub-edit/issues/369).

## What the roadmap promises, and what is already there

Duration adjustment, case, italics, dialogue dashes, merging, splitting,
search and replace, clipboard.

**This is the first phase whose subject is not a property of the file.**
Phases 8 and 9 answered questions that have a right answer: what encoding is
this file in, does this format carry an end. Here, most operations have no
single right answer — they have a result a user finds acceptable or not. The
framing must therefore say **who decides**, and not only what to compute.

Four things are already done when this framing is written, and they change its
content:

| Issue | What it delivered |
| :---- | :---------------- |
| [#365](https://github.com/Guyot-Bertrand/sub-edit/issues/365) | italics — **one entry, not three** |
| [#371](https://github.com/Guyot-Bertrand/sub-edit/issues/371) | the census of duration contradictions |
| [#372](https://github.com/Guyot-Bertrand/sub-edit/issues/372) | the cases of markup-aware search, and the rule they set |
| [#373](https://github.com/Guyot-Bertrand/sub-edit/issues/373) | the two grid fixtures that phase 16 had left |

## Preliminary analysis — what Gaupol does

Read in `aeidon/agents/position.py`, `format.py`, `edit.py`, `clipboard.py`,
`parser.py` and `finder.py`, and **executed** where reading was not enough.
The inventory carries the detail; three findings govern what follows.

**A markup-aware parser is at the centre.** `aeidon/parser.py` removes the
tags, remembers their positions, lets the bare text be transformed, then puts
them back. Case and dashes go through it. This
is not the pivot of ADR 0031, which translates one vocabulary into another and
removes what it cannot carry; the ADR says so itself — "they are two different
pieces".

**`adjust_durations` has no priorities, it has an order of lines.** Reading
speed, minimum, maximum, gap; only the end moves; the last constraint applied
wins and undoes the previous one, without saying so.

**The clipboard carries only texts.** `aeidon/clipboard.py` is a list of
strings, one per subtitle. No positions, no format, no whole subtitle — which
is what the framing issue believed, and what reading disproved.

## D1 — The markup-aware parser, and what it decides

**The piece that ADR 0009 has announced since phase 1 is written here.** Three
operations are waiting for it, and none can do without it: applying an
expression to the source text breaks the tags, and the ADR gives the example —
`\[.*?\]` swallowing an opening tag and leaving its closing tag orphaned.

**Its rule is written case by case, and the corpus is the specification.**
`src/test/data/textes/recherche.cas` carries thirty-one of them in the HTML
vocabulary, `recherche-accolades.cas` eight in Sub Station Alpha, and the rule
is at the head of the first, in six points. The two that decide:

1. **A tag boundary that cuts a word is pushed to the edge of the word**,
   outwards, before anything is searched. `<i>bon</i>jour` is worth
   `<i>bonjour</i>`.
2. **Any style that touches the match covers the whole replacement.**

> **Settled in #402: these two rules are those of replacement, and of
> replacement alone.** The parser applied them from its construction, hence to
> every operation that went through it: case moved a tag that cut a word, a
> replacement rewrote tags it did not touch, two `<font>` of two colours merged
> on their name, and a non-breaking space or a quotation mark counted as
> letters. From now on:
>
> | Operation | What becomes of the tags |
> | :-------- | :----------------------- |
> | replacement | those the match reaches are pushed to the edge of the word, then cover the replacement; the others do not move |
> | transformation — case, dashes | **none moves**: `<i>Bon</i>jour` in capitals gives `<i>BON</i>JOUR` |
>
> Two ranges merge only if **the whole tag** is the same, attributes
> included, and a word is made of **letters and digits**, read code point by
> code point.

> **A single markup reader, since #403.** The pivot, the parser, italics and
> the hearing-impaired mentions analyser each read their own, and contradicted
> one another. `core/text/markup_reader` now decides for all of them:
>
> | Rule | What it settles |
> | :--- | :-------------- |
> | a tag closes on its own line, before any second opening bracket | a lone `<` is text, and does not swallow the tag that follows |
> | an MPL2 marker is only one at the start of a line | `{y:b}/Bonjour` carries a visible `/` |
> | the name of an HTML tag stops at a space or a tab | `<i >` and `<b class="x">` are italic and bold for everyone |
> | a self-closing tag names nothing | `<i/>` opens no italic |
> | the mentions analyser reads the format's tags | `{\i1}[SOUPIR]{\i0}` is emptied; the `(320,50)` of a `\pos` is not a mention |

> **Written at the end-of-phase review.** The corpus moved once after the
> framing: #378 corrected an expected answer of `recherche.cas` — two
> neighbouring styles on the same word, `<i>Bon</i><b>jour</b>` — and added at
> the head of the file an ordering rule that the six points did not carry. The
> answer written before the code contradicted itself; it is the corpus that was
> corrected, and it remains the specification.

**What this rules out, and why.** Gaupol has no rule on the first point, only
an offset arithmetic, and the boundary falls back to a place unrelated to the
new text — `<i>Bon</i>jour` replaced by `Bonjour tout le monde` gives it
`<i>Bonjour tout le m</i>onde`. Measured by running its parser. This is the
only place where this phase deliberately departs from it, and the inventory
records it.

> **Corrected at the end-of-phase review: the gap is wider.** Gaupol's search
> **does not go through its parser**: `SearchAgent` searches the stored text,
> tags included, with a bare `Finder`. In Gaupol `<i>` can be found, and
> `Bonjour` does not find `<i>Bon</i>jour`. The example above describes what
> its parser would do, not what its dialog does. Searching the visible text is
> therefore a choice of this phase, not a corrected takeover — it is the choice
> of the hard point the roadmap raised. And the list of deliberate gaps has
> other lines, which D4, D5 and D7 now carry.

The rule "the replacement takes the style of its first character" was ruled
out for a reason that can be seen: it is not symmetric. It answers
`<i>Salut Sophie</i>` or `Salut Sophie` for the same shape, depending on which
of the two halves carried the italic.

**Implementation consequence, and it is firm:** the parser cannot be built on
`decodeAs`/`encodeAs`. A tag left open must stay open — a decoding followed by
a re-encoding would close it and change bytes nobody asked to change. And a
layout tag, `{\pos(x,y)}`, must go through intact, where the pivot counts it as
lost.

## D2 — The order of the four duration constraints, and what is sacrificed

**The order is Gaupol's** — reading speed, minimum duration, maximum duration,
gap to the next — and **only the end moves**. The start is never touched;
moving it would be moving the subtitle, which is the job of
`Shift Positions…`.

**The gap wins, and the other three are wishes.** It is applied last and can
bring the duration back under the minimum that had just been set. This is not
an inherited defect: it is the only defensible hierarchy. Two subtitles on
screen at the same time are a defect the viewer sees; a slightly short
subtitle is not.

**What is sacrificed is stated**, in the spirit of [ADR 0008](../adr/0008-lecture-au-mieux-avec-diagnostics.md) —
report rather than keep quiet.
This is the only point where we add something to Gaupol, which violates in
silence, and it is the same move as ADR 0031 for conversion.

**The contradiction is not an edge case, and that is measured.** On the private
corpus, with Gaupol's defaults — minimum 1.5 s, maximum 6 s, gap 0 s,
speed 15 char/s:

| What no end can satisfy together | Share of subtitles |
| :------------------------------- | -----------------: |
| minimum against gap | 4.5 % |
| speed against gap | 9.2 % |
| speed against maximum | 0.0 % |
| **at least one of the three** | **12.9 %** |

Replayable with `./src/scripts/measure-duration-constraints.py`, issue #371.
One subtitle in eight: refusing the operation on those cases would make it
useless, and violating them in silence would make its result incomprehensible.

**An inactive constraint is absent, not zero.** In Gaupol, a minimum of zero
disables the minimum by accident — `minimum and …` is false for zero, and the
gap, tested by `gap is not None`, does not have this defect. Three parameters,
two conventions. Here they are expressed as `std::optional`, and zero means
zero.

**The last subtitle has no next one, and has no bound for all that.** Gaupol
gives it 360 000 s, that is a hundred hours; a hard-coded number that means
"no constraint" is written as such.

**The length of the text is counted without the tags**, like
`get_text_length`: reading speed is about what the viewer reads.

## D3 — An entered duration moves the end

**Deferral from the phase 5 framing.** The `Duration` column is displayed and
cannot be edited, for lack of a command in the core; that command is born here.

> **Corrected at the end-of-phase review.** The framing added "which D2 needs
> anyway": that is wrong, the adjustment sets the end through the position
> command that already existed, and `SetDurationCommand` has only the cell as
> its caller.

**The end, and never the start.** It is the bound the adjustment moves, the
`Start` column can already be edited, and moving the start would move the
subtitle.

**An overlap created by the gesture is allowed, and it is already reported.**
`scanAnomalies` has flagged `OverlappingSubtitles` since phase 1 and the table
tints it. The framing wrote "the diagnostics panel shows it": that panel speaks
only of reading the file and is fed only on opening —
corrected at the end-of-phase review. Refusing an entry whose consequence is
visible would be refusing what the user may want — this is decision D4 of
phase 5, a warning and never a refusal.

## D4 — Case and dashes go through the parser, and case goes through ICU

**Through the parser, as in Gaupol.** Putting a subtitle in lower case must not
turn `<I>` into `<i>`, and a dialogue dash goes in front of the text, not in
front of an opening tag. **And no tag changes place** — settled in #402: the
word-edge rule, in D1, is that of replacement.

**ICU carries the four cases.** `std::toupper` works byte by byte and would cut
an accented letter in two; ICU has been a project dependency since
[ADR 0027](../adr/0027-icu-pour-les-encodages.md) and can do all four, title
case included. No new dependency, and the only serious alternative — a
hand-written table — would be wrong for every language that had not been
foreseen.

> **Clarified in #401: in the root locale, and following what ICU rewrites.**
> Called without a locale, ICU cased according to the process's — `istanbul` in
> capitals gave `İSTANBUL` under `LANG=tr_TR`. And a case change alters the
> length of a letter — `ﬁ` becomes `FI`, `İ` two code points — so that putting
> the tags back at their byte offset wrote one in the middle of a character, or
> lost one. The tags now follow the edits ICU records. Casing according to the
> document's language is a question for phase 15.

**Case does not touch what precedes the first letter.** Gaupol looks for the
first alphanumeric character and applies the transformation only from there:
a dialogue dash, an opening quotation mark, a parenthesis survive.

**The dash is a toggle that looks at the whole target**: if a single line has
none, all of them gain one; otherwise all of them lose it. This is the rule
that the italics of #365 already set — one entry, two directions, and pressing
twice gives the target back as it was.

> **Clarified at the end-of-phase review: for a homogeneous target.** The
> gesture normalises what it touches. A mixed target leaves as a block and does
> not come back mixed, and an em dash `—` comes back as a hyphen. That is the
> price of a single button; the history, for its part, restores the exact state.

> **Two gaps from Gaupol, recorded at the end-of-phase review.** Its dash rule
> recognises only the hyphen, and an empty line counts there as a line without
> a dash — so it puts dashes on empty lines. Here the three dashes are
> recognised and empty lines ignored: a target entirely in em dashes gains
> dashes in Gaupol and loses them here. And its title case is `str.title`, which
> gives `L'Été`; ICU's gives `L'été`.

## D5 — Merging joins, splitting cuts in the middle of the duration

**Merging** takes the start of the first, the end of the last, and joins the
non-empty texts with a line break.

**Splitting** cuts in the middle of the duration: the first keeps all the text,
the second is born empty. This is `split_subtitle`, kept as is — **with one
difference, found at the end-of-phase review**: Gaupol resets the
format-specific fields of both halves to their defaults, and here the first
keeps them, like the merged line.

> **Cutting the text at its line break was ruled out.** It is tempting — a
> two-line subtitle would make two one-line ones — and it is wrong as soon as it
> has one or three. A rule that works on the frequent case and makes things up
> on the others is worse than a predictable rule.

**One command per gesture, each undoable in one go.** Phase 2 laid down that
the history is a faithful account of what was done: merging five subtitles is
one entry, not five.

**Three choices made by #380, recorded at the end-of-phase review:**

- **merging takes only a block of neighbouring lines** — the core receives an
  interval, not a `Selection`: merging 1 and 3 without 2 would cover line 2;
- **the style, the layer and the coordinates of the first are kept**, where
  Gaupol loses them;
- **no `M` and `S` shortcuts**: a single letter would be taken by the table
  before reaching a cell being edited.

## D6 — The clipboard carries texts, and translates what it drops

**Texts, and nothing else.** That is what Gaupol carries, and there is no
reason to carry more: pasting positions would overwrite a timing nobody asked
to touch.

**Pasting moves no position and creates lines if some are missing.** This is
`paste_texts`, which inserts empty subtitles at the end to receive what
overflows, and says how many.

**An empty text, when copied, is a hole** — settled by #417. The plain text the
system clipboard receives does not distinguish a hole from an empty text: both
are written there as an empty chunk between two empty lines. A copy that kept
them distinct was therefore pasted in two ways depending on the route — from
the same window, the empty text overwrote the target line; read back from the
system, it left it as it was. Gaupol knows only one route: `_paste_texts`
always reads the system clipboard first, and `set_string` reads back any `""`
there as `None`, that is, a hole. `subedit` does the same, once, on copying:
a selected line without text is recorded as a line left out of the
selection, and the target line keeps its text.

**A single gap from Gaupol, and it is intended**: a copy that carries only an
empty text. In Gaupol, `if text:` is false for `""`, the internal copy takes
over and overwrites the target; here, it pastes nothing. That gap comes down to
this condition, which nothing presents as a rule — Gaupol's paste tests only
pin down reversibility and the number of lines, never what an empty text
overwrites — and keeping it would have made the result depend on the number of
lines copied.

**A text pasted from a document in another format has its tags translated,
and the loss is stated.** Gaupol does not do this and drops `{\i1}` into a
SubRip — the smear that ADR 0031 was written to end and that #364 closed on
writing. The clipboard is the door next to it.

In concrete terms: the internal clipboard remembers which format its texts come
from, a paste into another goes through `convertMarkup`, and the loss is
announced in the words `Save As…` already uses. **A text from outside
`subedit` has no format**, and is pasted as is: there is nothing to translate
from nowhere.

**The loss is stated after pasting, not asked before**, unlike
`Save As…` — a question opened by #370, settled by #382: a paste is undone
with `Ctrl+Z`, an overwritten file is not. **And the three entries require a
selection**: cutting without a selection would empty every text of the
document.

## D7 — Search works on the target, with two options

**Two options, Gaupol's**: regular expression or plain text, case-sensitive or
not. Its defaults are kept — plain text, case-insensitive.

**The scope is the usual target**: the selection, or the whole document if
nothing is selected. This has been the rule of the `Tools` menu since phase 5,
and it replaces Gaupol's scopes.

> **Corrected at the end-of-phase review.** The framing wrote "Gaupol's three
> targets". Its dialog has **two scopes** — the current project or all open
> projects — and **two fields** — main text and translation — and it **never
> looks at the selection**, which only gives it the starting line.
> Restricting to the selection is therefore an addition of this phase.

**`Replace All` is one history entry, not a hundred.**

**A gap from Gaupol, recorded at the end-of-phase review: the replacement
syntax.** Expressions are read by ICU, and a group is written `$1`; Gaupol
reads its own in Python, where it is written `\1`. Typing `\1` here inserts a
`1`. The manual says so.

## D8 — The two surfaces, and only one is delivered

**The phase delivers the core and the window.** The command line is phase 13,
"Complete CLI", and none of the eight operations appears there here.

This is not an oversight but a consequence: these operations are interactive
editing gestures — one italicises what one is looking at, one searches for
what one has just read. The four operations the command line already carries —
shift, transform, convert, remove hearing-impaired mentions — have in common
that they apply to a whole file without anyone looking at it, which is exactly
what batch processing calls for.

**The core commands are written for both surfaces nonetheless**, as
`removeHearingImpaired` was: a `Selection` received rather than deduced, and a
count read from the command rather than recounted. Phase 13 will only have
wiring to do.

> **Held in spirit, not to the letter — end-of-phase review.** No command
> deduces its target from the state of the window. But merging receives an
> interval, splitting and pasting an index, because a discontinuous
> `Selection` makes no sense there; and the clipboard, the adjustment and
> `Replace All` return their count **with** the command, in a structure
> computed at its construction, rather than having it read from it.

## What the phase does not deliver

Each with a phase or an issue, because a deferral without a recipient ends up
designating a phase that has already passed.

- **The spell checker**, and the joining or splitting of words it allows —
  phase 12.
- **Correcting common errors by patterns** — phase 12, whose heart it is.
- **Multiple projects**, hence the "all open projects" scope of a search and
  the translation text as a searchable field — phase 11.
- **Finding the pair of a conversion made at the wrong frame rate** —
  [#386](https://github.com/Guyot-Bertrand/sub-edit/issues/386), phase 14.
  Nothing this phase brings makes that mechanism safer: it is a search over
  rationals, and neither how far to search nor how to state its confidence has
  an answer today.
- **Preview in an external player** — **ruled out, and not deferred.** Gaupol
  writes a temporary file and launches MPlayer or VLC because it had no
  built-in player for fifteen years. `subedit` has had one since phase 6, and
  phase 14 adds control to it. Rebuilding the detour would be shipping a
  regression.

## Requirements

| ID | Requirement |
| :- | :---------- |
| `GUI-ITALIC-01` | one entry puts the selection in italics and takes it out, in the tags of the open format |
| `GUI-ITALIC-02` | a format that carries no style disables the entry without hiding it |
| `GUI-CASE-01` | the four cases apply to the target without touching the tags |
| `GUI-DASH-01` | dialogue dashes are added and removed with the same gesture |
| `GUI-SPLIT-01` | merging and splitting, and undo gives the document back as it was |
| `GUI-DURATION-01` | the `Duration` column can be edited, and moves the end |
| `GUI-ADJUST-01` | the adjustment applies the four constraints in the specified order |
| `GUI-ADJUST-02` | what no end can satisfy is stated, and counted |
| `GUI-SEARCH-01` | searching in the visible text, replacing in the source, without breaking the tags |
| `GUI-SEARCH-02` | search works on the selection, or on the whole document |
| `GUI-CLIP-01` | copying, cutting, pasting texts, and lines are born if some are missing |
| `GUI-CLIP-02` | pasting from a document in another format translates the tags and states the loss |

**The parser of D1 has no requirement, and that is the registry's rule**: a
requirement is what the binary shows. An internal guarantee of the core is held
by its unit tests and by coverage — and here, by thirty-two cases written before
it.

## Split into issues

The order goes from the foundational to the costly, and **the parser comes
before everything else**: three of the seven issues are waiting for it.

| Issue | What it does | What it depends on |
| :---- | :----------- | :----------------- |
| [#378](https://github.com/Guyot-Bertrand/sub-edit/issues/378) | the markup-aware parser | — |
| [#379](https://github.com/Guyot-Bertrand/sub-edit/issues/379) | case and dialogue dashes | #378, ICU |
| [#380](https://github.com/Guyot-Bertrand/sub-edit/issues/380) | merging and splitting | — |
| [#381](https://github.com/Guyot-Bertrand/sub-edit/issues/381) | the duration can be edited | — |
| [#382](https://github.com/Guyot-Bertrand/sub-edit/issues/382) | the clipboard | `convertMarkup` |
| [#383](https://github.com/Guyot-Bertrand/sub-edit/issues/383) | adjusting durations | #378 — the length is counted without the tags |
| [#384](https://github.com/Guyot-Bertrand/sub-edit/issues/384) | search and replace | #378 |

The phase closes on [#385](https://github.com/Guyot-Bertrand/sub-edit/issues/385),
the end-of-phase review — the third of the three issues that frame it.

**#380 and #381 can be done at any time**, and they are placed early because
they are small: once the parser is done, two short issues test the tooling
before tackling the two big ones. That is the order that was followed:
#378, #379, #380, #381, then the rest.

> **Corrected at the end-of-phase review.** The framing gave #381 as a
> dependency of #383; the adjustment does not use it, and it is on the parser
> that it depends.

**#383 and #384 are the two big ones**, for opposite reasons. The first carries
a dialog of four constraints and a report of what it sacrificed; the second
carries four gestures, two options remembered from one session to the next,
and a table that moves by itself.
