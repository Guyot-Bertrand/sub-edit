# Phase 9 — Additional formats and rich markup

Framing, issue #337. This document says what the phase delivers, what it does
not deliver, and why.

## What the roadmap promises, and what is already there

> SubViewer 2, Sub Station Alpha, Advanced SSA — the three formats cited as
> secondary — then MicroDVD, MPL2, TMPlayer and LRC.

**Seven formats on top of two existing ones**, and a deferral from phase 5: the
frame editing mode.

**The tooling is in place before the first reader**, which is new and changes
the way this phase is measured. The three initialisation issues (#336)
delivered:

| Tool | What it holds |
| :--- | :------------ |
| [`src/test/data/formats/`](../../src/test/data/formats/) — #338 | the same scene in the nine formats, and a table of promises, one row per format |
| [`conversion.md`](../mesures/conversion.md) — #339 | what a round trip loses, pair by pair, and the refusal of a loss that gets worse |
| [`detection-de-format.md`](../mesures/detection-de-format.md) — #340 | 12/19, seven refusals, no confusion |

**Each format issue moves three figures**, and that is how the progress of the
phase is read: a row of the table of promises goes from `NotYet` to
`Readable`, the detection score goes up by one rendering, and the conversion
matrix gains a row and a column. None of these three measurements is to be
written by hand.

## Preliminary analysis — what Gaupol does

`aeidon/files/` holds one module per format, `aeidon/markups/` one module per
markup vocabulary, and `aeidon/markupconv.py` the translation from one to the
other.

| Format | Mode | Positions | Ends | Line breaks | Italic | Header |
| :----- | :--- | :-------- | :--- | :---------- | :----- | :----- |
| SubRip | time | millisecond | carried | ⏎ | `<i>…</i>` | none |
| WebVTT | time | millisecond | carried | ⏎ | `<i>…</i>` | free |
| SubViewer 2 | time | hundredth | carried | `[br]` | `<i>…</i>` | `[…]` sections |
| Sub Station Alpha | time | hundredth | carried | `\N` | `{\i1}…{\i0}` | sections, named styles |
| Advanced SSA | time | hundredth | carried | `\N` | `{\i1}…{\i0}` | same, `v4.00+` |
| MicroDVD | **frames** | frame | carried | `\|` | `{Y:i}` | `{DEFAULT}` line |
| MPL2 | time | tenth | carried | `\|` | `/` at the start of a line | none |
| TMPlayer | time | second | **absent** | `\|` | **no vocabulary** | none |
| LRC | time | hundredth | **absent** | **absent** | **no vocabulary** | free |

Three things come out of it, and they are the three costly decisions of the
phase: **two formats carry no end**, **one counts in frames**, and **five
markup vocabularies coexist** where two used to merge.

## D1 — The ends the file does not carry are deduced, and the reading says so

Settled, and it has its ADR: [0029](../adr/0029-fins-deduites-et-annoncees.md).

`lrc.py` and `tmplayer.py` read only one position per line. The end of a line
of dialogue is the start of the next one, and the last one gets five seconds.

**The framing's three routes, and what tells them apart:**

| Route | What it costs |
| :---- | :------------ |
| deduce on reading, as Gaupol does | the model carries a value the file did not carry |
| make `end` optional | twenty-eight places in the library and the executable, and a policy to invent in each of them |
| refuse both formats | iso-functionality |

**What decides is not the count, it is that there is nothing to decide
twenty-eight times.** What does a shift do to a subtitle with no end? Nothing.
What does the Duration column show? Nothing. What does an overlap mean? The
question does not arise. Each of these answers is a void that would have to be
written, tested and maintained, for a state that **two formats out of nine**
can produce and that disappears as soon as the document is saved elsewhere.

**What deduction owes to the project's rule is to say so.** ADR 0008: say
rather than keep quiet. A reading that invents ends emits a diagnostic that
names it, and the round trip stays byte for byte — writing renders only
starts, so nothing that was invented ends up in the file.

This is what the table of promises of #338 calls **the vacuous byte check**:
the comparison would pass whatever the reading had invented, which is why the
four expected ends are written there by hand.

## D2 — A frame rate read with the file, and declared

Settled, and it has its ADR: [0030](../adr/0030-ce-qu-un-document-retient-de-son-fichier.md).

MicroDVD counts in frames; the model has counted in milliseconds since
[ADR 0006](../adr/0006-positions-en-millisecondes.md). **A MicroDVD file states
no frame rate**, and phase 16's deduction can do nothing for it: it looks for a
grid in milliseconds, and frame numbers fall on every grid at once.

The frame rate therefore comes from outside, and it is **carried by the
document** just as its encoding and its line endings are — this is what makes
the round trip exact, since writing reuses the one that served for reading.

**Three sources, in this order:**

1. what the user says — `--frame-rate` on the command line, the dialog in the
   window;
2. what the associated video declares, when there is one — phase 6 already
   knows how to read it;
3. **a declared default, 23.976**, which the reading announces with a
   diagnostic.

The default is Gaupol's — `aeidon.framerates.FPS_23_976`, and
iso-functionality is the only reason to prefer it to 25. **Refusing to open for
lack of a frame rate was ruled out**: a file that has just been double-clicked
must open, and an announced value that can be corrected is better than a
refusal.

## D3 — What a document keeps from its file

Same ADR as D2, and that is the reason for bringing them together: the frame
rate is not a special case, it is the third of a family.

**Three things a file carries and no subtitle carries:**

| What it is | In Gaupol | Without which |
| :--------- | :-------- | :------------ |
| the free header — `[Script Info]`, `[INFORMATION]`, the text after `WEBVTT` | `header` | the round trip loses whole blocks |
| the order of the SSA event fields — the `Format:` line | `event_fields` | writing reorders the columns of the whole file |
| two form settings — TMPlayer's `two_digit_hour`, MicroDVD's frame rate | file attributes | the round trip changes digits |

`header` exists and is a string: **it is enough for what a round trip must give
back**, because nothing needs to understand it. What it does not carry is the
rest, and that rest takes the form of a per-format `variant`, the twin of
`FormatExtras` at file level.

**`FormatExtras` does not become a nine-branch variant**, and counting says
so: five of the nine formats carry nothing per subtitle — SubViewer 2,
MicroDVD, MPL2, TMPlayer and LRC stay on `std::monostate`. SSA and ASS share a
branch. The variant goes from three to four branches, and the compiler guard
its comment claims holds without changing shape.

## D4 — A markup pivot, for conversion only

Settled, and it has its ADR: [0031](../adr/0031-pivot-de-balises-a-la-conversion.md).

[ADR 0009](../adr/0009-texte-en-chaine-brute.md) was written for this very
phase — it names ASS — and it promises a structured model serving as a pivot
**only during a conversion**. That pivot does not exist: nothing had asked for
it as long as the two formats shared `<i>`.

It becomes indispensable, and the demonstration fits in one line: without it,
an SSA converted to SubRip renders `{\i1}Il ne dit rien.{\i0}` — text the user
sees, in a file that will never know how to interpret it. That is worse than a
loss, it is a smear.

**Its reach is Gaupol's: bold, italic, underline, colour, font, size.**
Nothing else is translated. What is not recognised is removed, and ADR 0009
says why that is acceptable: the loss exists only on conversion, never when
opening and saving the same format, where the raw string goes through intact.

**What we add to Gaupol is saying so.** See D5.

## D5 — The degradation policy, and what it says

**A conversion never refuses, and it never keeps quiet.** This is ADR 0008
applied to an operation rather than to a reading, and phase 8 gave the form it
takes: a `reportOutcome` that the three verbosity levels and the window read
alike.

What is said, item by item, and **only when there was a loss**:

| What was lost | How it is said |
| :------------ | :------------- |
| the ends, for a format that carries none | "ends are not carried by LRC" |
| line breaks | "line breaks were joined" |
| markup the target format cannot write | their number |
| the header, the format-specific data | what went, named |
| precision | the largest gap, in milliseconds |

**What we keep quiet about, and why.** A conversion that loses nothing says
nothing: a report that shows up on every call is a report nobody reads. And the
loss of precision **is announced only if it happens** — half of the
conversions between hundredth-based formats carry positions that are already
round.

**The statement of what each format forces one to lose is written**, in
[`conversion.md`](../mesures/conversion.md), and it is on that statement that
this policy is decided rather than on an intuition. What the tool measures is
the gap between it and reality.

## D6 — Detection with nine candidates keeps its rule

**`detectFormat` promises "nothing rather than a guess", and the rule holds.**
The nine signatures separate on content, and the resemblance the framing feared
is not one:

| What looks alike | What separates them |
| :--------------- | :------------------ |
| MicroDVD's `{25}{75}`, MPL2's `[10][30]` | the shape of the brackets, and nothing else has this grammar |
| LRC's `[00:01.00]` | a single bracket, and a timestamp inside it |
| TMPlayer's `00:00:01:` | three fields and a trailing colon, at the start of a line |
| SSA and ASS | `ScriptType: v4.00+` and `[V4+ Styles]` |
| SubViewer 2 | `[INFORMATION]`, and a `HH:MM:SS.cc,HH:MM:SS.cc` line |

**This is not an assertion, it is a measured hypothesis** — that is what #340
is for. Each format delivered raises the score, and **a confusion fails on its
own**, whatever the rate. Should two formats claim the same file, the answer is
today's: nothing, and `inspect` says it could not tell.

The extension does not enter into the decision, and this is written to be
known: the scorer runs every file under a neutral name. Going back on that
would require changing the scorer in the same move, which makes the choice
visible.

## D7 — The two surfaces

**`--to` goes from two values to nine**, and the manual keeps its promise of a
closed, enumerated set.

**`--frame-rate R` comes to `convert`**, and it serves both ways: it imposes
the reading frame rate of a frame-based file, and the writing one of a
conversion to a frame-based format.

**Without it, a conversion to a frame-based format takes the deduced grid** —
this is phase 16 at work, and it is exactly what it was written for: the grid
of a time-based file *is* the frame rate it was timed at. The conversion says
so. **Without a frame rate and without a grid, it refuses**, because the only
remaining move would be to invent a figure that would shift every line.

`inspect` names the format among the nine, and for a frame-based file it names
the frame rate **and where it comes from** — the same form as the encoding line
since phase 8.

On the window side: the open filter lists the nine extensions, `Save As…`
offers the nine formats, and **a lossy format announces what it is about to
lose before writing**, not after.

## What the phase does not deliver

- **The frame editing mode.** Deferred to phase 14, and the reason has
  changed. Phase 5 had deferred it for lack of a frame-based format; one is
  arriving. What remains true is that **MicroDVD carries frame numbers and no
  frame rate**: the toggle would show the file's figures for a MicroDVD, and a
  conversion against a chosen frame rate for the other eight. The question is
  therefore better posed than before, and it remains a question of fine
  editing — the subject of phase 14, which already carries "step forward,
  step back frame by frame".
- **Markup the pivot does not know.** `{\pos(x,y)}`, `{\t(...)}`,
  `<ruby>`, WebVTT's internal timestamps: they go through a round trip in their
  own format intact, and disappear on conversion. That is Gaupol's reach, and
  widening it would require a layout model that nothing calls for.
- **Translating SSA named styles.** A `Style: Default` carries a font, a size,
  four colours and margins. Converting them to SubRip `<font>` would be
  guessing; losing them is what Gaupol does, and the conversion says so.
- **A tenth format.** The list is Gaupol's, and it is closed.

## Requirements

| ID | Requirement |
| :- | :---------- |
| `CLI-FORMAT-01` | each of the nine formats opens, and is rewritten byte for byte |
| `CLI-FORMAT-02` | a file that none of the nine claims is refused, without guessing |
| `CLI-FORMAT-03` | `inspect` names the format read among the nine |
| `CLI-FORMAT-04` | a format without ends opens, its ends are deduced, and the reading says so |
| `CLI-FRAMES-01` | a frame-based file opens at a declared frame rate, which the reading names |
| `CLI-FRAMES-02` | `--frame-rate` imposes the frame rate, on reading as on writing |
| `CLI-CONVERT-04` | `--to` accepts the nine formats |
| `CLI-CONVERT-05` | converting to a frame-based format without a frame rate takes the deduced grid, and says so |
| `CLI-CONVERT-06` | without a frame rate or a grid, a conversion to a frame-based format is refused |
| `CLI-CONVERT-07` | a conversion that loses something says so, item by item |
| `GUI-FORMAT-01` | the window opens the nine formats |
| `GUI-FORMAT-02` | `Save As…` offers the nine formats |
| `GUI-FORMAT-03` | saving in a lossy format announces what will be lost before writing |
| `GUI-FRAMES-01` | opening a frame-based file announces the frame rate retained, and lets it be changed |

## Split into issues

The order follows what each format teaches the model, from the least costly to
the most costly, and **the vocabulary comes before everything else**: nothing
can read a `.ass` as long as `SubtitleFormat` does not know what it is.

| Issue | What it does | What it teaches the model |
| :---- | :----------- | :------------------------ |
| #344 | the nine formats enter the vocabulary | `SubtitleFormat`, `nameOf`, `extensionOf`, what a document keeps from its file |
| #345 | SubViewer 2 | a header of sections, hundredths, `[br]` |
| #346 | Sub Station Alpha and Advanced SSA | event fields, named styles, the extras branch |
| #347 | MPL2 | tenths, `\|`, a tag at the start of a line |
| #348 | MicroDVD | frames, and the frame rate carried by the document |
| #349 | TMPlayer and LRC | deduced ends, and the diagnostic that announces them |
| #350 | the markup pivot, and what a conversion says it loses | — |
| #351 | the command-line surface | — |
| #352 | the window surface | — |

The phase closes on #353, the end-of-phase review — the third of the three
issues that frame it.

**Issues #345 to #349 look alike, and that is intended**: each adds a reader,
a writer, its row in the table of promises of #338, and moves the two
records. What sets them apart is written in the last column — what they have to
teach the model, and that no other teaches it.

**#350 cannot come earlier**: a markup pivot is judged on the vocabularies it
links, and they do not exist before.
