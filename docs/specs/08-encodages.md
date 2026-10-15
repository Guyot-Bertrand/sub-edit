# Phase 8 — Encodings and line endings

Framing, issue #288. This document says what the phase delivers, what it does
not deliver, and why.

## What the roadmap promises, and what remains to be done

> Automatic encoding detection, full set of encodings, Windows and Mac line
> endings, forcing on save.

**The "line endings" half is already delivered**, and saying so is the first
task of this framing: `Newline` has carried `Lf`, `CrLf` and `Cr` since phase 1,
`--line-endings unix|windows|mac` has existed since phase 3, `scanNewlines`
reports the majority line ending, the mixture and the line where it starts, and
a file that is read is rewritten with its own.

**What remains of the promise is the encoding.** And one thing the roadmap does
not say: **the window offers none of this.** `Save As…` chooses a path and a
format; no encoding, no line ending, no BOM. The command line can do what the
window does not offer, and that gap is within the scope of this phase.

## Preliminary analysis — what Gaupol does

`aeidon/encodings.py`, `aeidon/agents/open.py`, `aeidon/file.py`.

- **Ninety-seven encodings**, each with its Python code, its IANA name and a
  description taken from gedit.
- **Detection in two steps**: the BOM first — UTF-32 BE/LE, UTF-8,
  UTF-16 BE/LE —, `charset-normalizer` next, and `None` if nothing comes out.
- **The BOM wins over the requested encoding**: `open_main` starts the read
  again with the BOM's encoding if it differs from the one it was given.
- **The locale as a last resort**, without telling the user.

This last point is the only one this framing rules out outright: the project's
rule is to say rather than keep quiet — ADR 0008, best-effort reading with
diagnostics.

## D1 — ICU, to convert and to detect

Settled by measurement, and it has its ADR: [0027](../adr/0027-icu-pour-les-encodages.md).

The three routes were run over the same corpus before any was chosen —
ICU 9/9, `uchardet` 9/9, a prototype of hand-written tables 6/9. What decides is
not detection but **conversion**: ICU is the only one of the three that
converts, and iso-functionality calls for ninety-seven encodings, which are not
written by hand.

The price is written in the ADR: `subedit-cli` gains its first external
dependency there.

**Completed afterwards by [ADR 0028](../adr/0028-peser-les-lignes-qui-discriminent.md).**
The bench above carried only monolingual files, and it is while measuring where
detection stops knowing — issue #310 — that a bilingual file turned out to lose
its second alphabet entirely: ICU sees only the mass of ASCII in it. ICU remains
the detector; what changes is what we submit to it.

## D2 — The set of encodings is ICU's, not a list of our own

**No encoding table is written in this repository.** ICU knows more of them
than Gaupol; keeping a reduced list would amount to writing a second truth to
maintain, and to refusing a file we know how to read.

What is written, on the other hand, is **what the interface offers**: a short,
ordered list of the encodings a subtitle file carries in practice, plus
"other…" for the rest. A menu of ninety-seven entries is not a help.

### The fifteenth encoding, and what it costs — issue #316

**This decision and iso-functionality did not say the same thing, and nothing
had confronted them.** The inventory says ninety-seven, which is what
`aeidon/encodings.py` lists; the menu offers fourteen. The end-of-phase review
raised the question, and this is the answer.

**The accessible set was never the difference.** It is ICU's, hence larger than
Gaupol's: two hundred and twenty-one encodings, no file refused. What differed
is the **cost of the fifteenth** — the "Other…" field takes any name, provided
one knows it, where a menu only asks one to recognise it.

**That field therefore completes on what ICU knows.** The list is written
nowhere: `availableEncodings()` asks ICU what it converts and passes each
answer through `Encoding::create`, so that it cannot diverge from the type it
describes. Completion works on what the name **contains** and not on what it
starts with — the canonical name is `windows-1252`, and `cp1252` is the one
people have in mind.

**The menu stays at fourteen**, and both halves of D2 hold: no table is
written, and no menu of two hundred entries is offered. The fifteenth encoding
costs three letters.

**Ruled out: enumerating the converters in the menu.** Two hundred and
thirty-two entries, without the descriptions that gedit gives and Gaupol took
over, and a menu that the first half of D2 refuses for what it is — long.

## D3 — What the model records

`SourceFile` carries today the format, the line ending and `hadUtf8Bom`, **a
boolean specific to UTF-8**. It is no longer enough: a UTF-16 BOM does not say
the same thing, it also carries the byte order.

`hadUtf8Bom` therefore becomes an encoding, and the BOM stops being a separate
property: it is **a variant of the encoding** — `utf-8` and `utf-8-sig` are two
different answers, as Python names them. A strong type rather than a string, as
`FrameRate` already is (ADR 0011): an unknown encoding must be
unrepresentable, not reported on use.

## D4 — A detection is a proposal, never a certainty

**Latin-1 and CP1252 are indistinguishable** on a text that does not use the
`0x80–0x9f` range. The bytes do not carry the answer; a detector that decides
is guessing. The labelled corpus shows it — the three routes answer on
`latin1.srt`, and none of them can prove it.

Consequence, and it follows ADR 0008:

- **a guessed encoding that is not UTF-8 enters the reading diagnostics**,
  those the window shows under the table and that `-vvv` writes. This framing
  wrote "the encoding retained", without reservation, and what was built is
  narrower in two ways: a byte order mark triggers nothing — nothing was
  guessed —, and neither does a file guessed as UTF-8, because that is what
  every file was read as until this phase. Saying it on every opening would put
  a panel under the table of every ordinary document.
  **What the window loses by this is recorded**: it then has nothing to say
  about a file that declares its encoding, where `inspect` writes `UTF-16LE, from its
  byte order mark`. A gap found at the end-of-phase review, settled in favour
  of what was built, and what the window lacks is issue #313;
- **it can be contradicted**: `--encoding` on reading imposes and does not
  propose;
- **the BOM wins over everything**, including `--encoding`, because it is the
  only explicit mark a file carries. A BOM contradicted by an option is
  reported, not arbitrated in silence.

## D5 — Reading no longer refuses, it reports

`ReadErrorKind::InvalidUtf8` renders "is not valid UTF-8". With several
encodings, this message changes meaning: it is no longer "this file is broken"
but "I cannot read it in the encoding I chose". The words live in
`core/wording/formats.hpp`, the only place they are written.

A file that no encoding decodes remains an error; a file decoded under a
guessed encoding is a success with a diagnostic.

## D6 — The two surfaces, and the window catches up

**Command line** — `--encoding` imposes the **reading** encoding, for the seven
subcommands; `--to-encoding` chooses the **writing** one, on `convert` alone, in
the form `--line-endings` already has. On writing, the encoding of the file
that was read is the default, like the BOM and the line endings: a file
rewritten without instructions gives back the same bytes.

> **Two options, where this framing wrote one.** It said "`--encoding` on
> reading **and** on writing", and that sentence could not hold: the same
> invocation does both — `convert --encoding cp1252 --to-encoding
> utf-8` re-encodes a file — so a single name would have had two values to carry
> in a single command line. A gap found at the end-of-phase review, and settled
> in favour of what was built.

**Window** — `Save As…` gains the encoding, the line ending and the BOM. Giving
it the encoding without the other two would leave it half done, and the other
two have existed since phase 3 without the window offering them.

## D7 — The byte round trip is the property that holds everything

A file read and rewritten without instructions gives back **the same bytes**.
That is already true of the BOM and of line endings; the encoding extends it,
and it is not verified on the text — two different encodings carry the same
text.

The harness exists: `corpus_test.cpp` has enumerated `valides/` since #289 and
compares with the original file. The phase adds its encoding fixtures to it as
it learns to read them.

## What the phase does not deliver

- **UTF-32.** Gaupol detects it by BOM and does not list it among its offered
  encodings. No subtitle file carries it, and nothing offers it.

  This framing added "ICU will read it if asked to": **that is wrong, and the
  end-of-phase review measured it.** A UTF-32LE file starts with
  `ff fe 00 00`, whose first two bytes are exactly the UTF-16LE mark; since the
  mark wins over the option, `--encoding UTF-32` never reaches the converter,
  and the file comes back `is in no format this
  tool knows`. `--to-encoding UTF-32` does write one, though, which no reading
  takes back. The gap is issue #308.
- ~~**A batch encoding conversion from the command line.**~~ **The sentence
  became false the day the option existed**, and the end-of-phase review
  records it rather than maintaining it: `convert --to-encoding
  utf-8 --output-dir sortie *.srt` re-encodes a directory, because
  `--to-encoding` is added to a subcommand that already takes several inputs
  and a batch destination. There was nothing to remove to keep the promise, and
  nothing to add to break it.
- **Language detection.** ICU offers it along with the encoding; nothing here
  has a use for it.

## Requirements

| ID | Requirement |
| :- | :---------- |
| `CLI-ENC-01` | a non-UTF-8 file opens, and the encoding retained is stated |
| `CLI-ENC-02` | `--encoding` imposes the reading encoding and wins over detection |
| `CLI-ENC-03` | a BOM wins over `--encoding`, and the gap is stated |
| `CLI-ENC-04` | `--to-encoding` chooses the encoding produced on writing |
| `CLI-ENC-05` | a file rewritten without instructions gives back the same bytes, encoding included |
| `CLI-ENC-06` | a file that no encoding decodes is refused, and the reason says so |
| `CLI-ENC-07` | `inspect` reports the encoding read and whether it was guessed |
| `GUI-ENC-01` | the window opens a non-UTF-8 file and shows the encoding retained |
| `GUI-ENC-02` | `Save As…` chooses the encoding, the line ending and the BOM |
| `GUI-ENC-03` | the settings remember the last writing encoding chosen |

### What `GUI-ENC-03` does not promise — issue #317

**The setting never wins over the encoding of a file that was read**, and this
is a deliberate "no" rather than an omission. An open document offers the
encoding of its file; `file.write-encoding` only serves the document that has
never been on disk — the one obtained by launching `subedit-gui` without an
argument.

The reason is D7: rewriting in another encoding a document that has just been
opened, because a three-week-old setting says so, would lose what reading kept,
and the byte round trip is exactly the property this phase holds.

**The other reading was investigated and ruled out.** Always prefilling would
answer a real use — someone who systematically delivers in UTF-8 and opens
legacy files in CP1252 — at the price of a guarantee that holds for everyone.
What remains for whoever wants that flow is to choose the encoding in the
dialog, where it is offered.

**What did change, on the other hand, is the mark.** It was remembered within
the session and lost between two: `file.write-encoding` carries the name,
`file.write-bom` carries the mark — the same split as `--encoding` and `--bom`,
and the only one possible since `-sig` stopped being a name (#315).

## Split into issues

The order is not negotiable for the first two: the model carries the encoding
before anyone reads it, and reading comes before writing.

| Order | What it does |
| :---- | :----------- |
| 1 | the encoding enters the model, and the BOM becomes its variant |
| 2 | reading in a given encoding, and refusing it cleanly |
| 3 | detecting, and saying what was guessed |
| 4 | writing in a given encoding, and the round trip |
| 5 | the command-line surface |
| 6 | the window surface |
