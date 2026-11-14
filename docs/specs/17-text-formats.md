# Phase 17 — Additional text formats

**Scoped (#837).** The initialisation (#836) checked the state of play, drew up the census below and laid the guard a new
format is held to. This document decides the rest — which formats, where this phase ends and phase 18 begins, what each
format reads and writes, the detection, the two surfaces — and splits the phase into issues.

**Three formats: YouTube SBV, SAMI and TTML.** Spruce STL and SCC are evaluated with phase 18.

## What the roadmap promises

The text subtitle formats that neither Gaupol nor phases 1 and 9 cover, the list being drawn up at scoping according to the real use of the
intended audience. Each format moves the three usual measurements of phase 9: a row of the table of promises, the detection score, the
conversion matrix.

## What the initialisation found

**State of play, verified** (2026-11-14):

- **Gaupol's list is matched, all of it.** `reference/gaupol/aeidon/files/` holds nine format modules, and they are the nine of phase 9.
  Every format of this phase is therefore one Gaupol does not read. The sentence of spec 09, "the list is Gaupol's, and it is closed",
  stops being true here, and the comment of `SubtitleFormat` now says so.
- **The three measurements of phase 9 exist, and a tenth format costs each of them little.**
  - The table of promises: a rendering of the scene in `src/test/data/formats/` and its row in `format_corpus_test.cpp`.
  - The detection score: a slug in the `NAMED` table of `score-format-detection.py`.
  - The conversion matrix: a name in the `TARGET_OF` table of `measure-conversion-loss.py`.

  Both scripts **stop** on a name they do not know, rather than measure it against nothing.
- **What the code lists, it derives.** The `--to` values of `convert` and of `transcribe`, the filters of the window's file dialogs, the
  names and the extensions all walk `kSubtitleFormats`. Every `switch` over a format names each one without a `default`, so a forgotten
  case fails to compile.
- **What nothing held is written by hand**, and it was the hole:
  - the formats tables of the manual (`convert.md`, `files.md`, `inspect.md`, the first page);
  - the extensions the detection probe walks;
  - the link between the promise table and the core's list.

  A format added to the core alone would have been offered by the program, missing from its manual, unmeasured, and invisible to the
  probe, all in silence. The guard below closes it. The first thing it found was the `convert` table naming *SubViewer 2.0* where the
  program says *SubViewer 2*. That is corrected, in the 21 trees.
- **The scene stays as it is.** It was cut to the poorest format of phase 9: four lines, on whole seconds, falling on a 25 fps frame. Every
  candidate below can say it:
  - a format that counts in frames at an integer rate falls on it exactly;
  - SCC counts at 29.97 drop-frame, where a whole second is not a frame; that is a precision loss, which the conversion matrix already
    reports, as it does MPL2's tenth;
  - a format without ends is promised `BytesEmptily`, as TMPlayer and LRC are.

  *Corrected at scoping*: SAMI was given here as such a format. It is not. A `SYNC` holding only `&nbsp;` clears the
  line, so a file that clears its lines carries its ends, and the scene, which has gaps, is written that way (D3).

  A format that cannot say the scene at all still gets a rendering; its row then says what it loses.
- **The private corpus says nothing about the candidates**, and this is measured, not assumed. Every file of `src/data/`, whatever its
  extension, was counted:
  - WebVTT, SubRip, one MicroDVD, one Advanced SSA;
  - the five notes `.txt`;
  - two images;
  - an archive holding copies of the same files.

  Not one file is in a format outside the nine. The list of this phase can therefore not be drawn from that corpus: the scoping argues it
  another way, and says which.

## The candidates, a census

What each candidate is, where it comes from and what it would cost. **This is not a measure of use**: the private corpus holds none of
them, and the column *Comes from* is what the formats' own documentation and the common tools say, not a count. It is the material of
the scoping's first question.

| Format | Extension | Text | Specification | Timing | Comes from | `ffmpeg` 6.1 | What it would cost |
| :----- | :-------- | :--- | :------------ | :----- | :--------- | :----------- | :----------------- |
| YouTube SBV | `.sbv` | yes | none published; the platform's help | ms, start and end | YouTube's caption export | does not read it | little: one grammar, close to SubRip |
| SAMI | `.smi`, `.sami` | yes, HTML-like | Microsoft's SAMI 1.0 document | ms, start only (`<SYNC Start=…>`) | Windows Media era; still common in Korea | reads | an HTML-like reader; CSS classes per language and style — the boundary with phase 18 |
| Spruce STL | `.stl` | yes | DVD Studio Pro's documentation | frames (`HH:MM:SS:FF`) | DVD authoring | reads | one grammar; **`.stl` is also EBU STL, a binary broadcast format** — the detection must refuse that one |
| SCC | `.scc` | text of hex byte pairs | CEA-608, a paid standard, widely described | 29.97 drop-frame timecode | US broadcast captioning | reads and writes | decoding CEA-608 control codes; caption modes and rows are positions — phase 18 |
| TTML / DFXP / IMSC | `.ttml`, `.dfxp`, `.xml` | XML | W3C Recommendation | ms, frames or ticks | streaming delivery | writes only | an XML reader; styles and regions — the roadmap places TTML in phase 18 |
| MPsub | `.sub` | yes | MPlayer's documentation | relative (a wait and a duration), seconds or frames | MPlayer | reads | one grammar; a third format behind `.sub` |
| SubViewer 1 | `.sub` | yes | none | `[HH:MM:SS]`, start only | SubViewer | reads | one grammar; a fourth format behind `.sub` |
| RealText | `.rt` | yes, markup | RealNetworks' documentation | `begin` and `end` attributes | RealPlayer, obsolete | reads | a markup reader for a dead player |
| QuickTime Text | `.txt` | yes | Apple's documentation, deprecated | `[HH:MM:SS.ff]` | QuickTime, obsolete | does not read it | a third format behind `.txt` |
| PJS, VPlayer, JACOsub, AQTitle | various | yes | none or a page | various | fan-subbing tools | reads | niche |

**Out of the census by the roadmap's own word** (*text* formats):
- EBU STL, Cheetah CAP and PAC are binary;
- image formats (VobSub, PGS) belong to phases 20 and 21.

## Where the truth of a reading comes from

**There is no Gaupol to hold a new format against**, and that was phase 9's oracle. For each format of this phase:

1. **The format's specification**, where one exists, is the authority. The scoping names it for each kept format, with what it leaves
   open.
2. **The expected values are written by hand against it**: a rendering of the scene, and a file of its own for what the scene does not
   exercise. A test reads them from the fixture, **never from the program's own output** — the rule of the alignment cases of phase 23 and
   of the encodings of phase 8.
3. **`ffmpeg` is a second opinion, by hand only.** Phase 22 already expects it on the user's machine, and its 6.1 reads seven of the
   candidates above. A person may compare a reading with `ffmpeg -i file.smi out.srt`, and say in the issue what they compared and
   what differed. It is never run by the gate, and its output is never a fixture's truth. What it gives is another program's reading,
   which is evidence, not a specification. The rule that keeps `reference/` out of the code applies to any outside program.

## The tools

**Kept, and built here:**

| Tool | Where |
| :--- | :---- |
| the promise table held to the core's list: a format of `kSubtitleFormats` without its rendering of the scene fails | `format_corpus_test.cpp`, *every format the core knows has its rendering of the scene* |
| the manual held to the core's list: the `--to` table of `convert`, the filter table of the window, the list of `inspect` and the first page name each format, with its option name and extension | `format_enumerations_test.cpp` |
| the detection probe held to the core's list: it walks the extension of each format | `format_enumerations_test.cpp` |

**Discarded, with the reason:**

- *A census probe of the private corpus.* It would count nothing: the corpus holds no candidate, and that is measured above. The
  census is the table.
- *A fixture for each candidate, now.* The list is not fixed, and a fixture for a format that is not kept is a file to delete.
  Each format issue writes its rendering of the scene by hand against the specification; the guard above makes a forgotten one fail.
- *One list from which every enumeration is generated.* The code already derives its lists. What remains by hand are tables a reader
  reads, in 21 languages, and generating them would move the problem into the generator. A test is cheaper and says what is missing.
- *A script comparing a reading with `ffmpeg`.* There is nothing to compare before the first reader. If the scoping keeps a format
  `ffmpeg` reads, its issue decides whether the comparison by hand deserves a script, and it never enters the gate.

## D1 — Three formats: YouTube SBV, SAMI and TTML

**The private corpus holds none of the candidates** (measured at the initialisation), so the list is argued from the
use the census reports, for the intended audience of part three: people who watch series and films and want to
automate what they do with their subtitles. Each candidate is kept or set aside on four questions — real use, a
specification, being text, what it costs the detection.

| Format | Decision | Why |
| :----- | :------- | :-- |
| **TTML**, with DFXP and IMSC | kept | what streaming services deliver, and what the audience downloads from them; a W3C Recommendation; text; a declaration (`<tt>` in its namespace) that nothing else claims |
| **SAMI** | kept | the format of a large share of Korean subtitles and of older DVD rips; Microsoft's published document; text; a declaration (`<SAMI>`) |
| **YouTube SBV** | kept | the caption format of YouTube's editor, met by those who subtitle their own videos; text; a grammar close to SubRip's whose stamps no other format writes; the cheapest of the three |
| Spruce STL | evaluated with phase 18 | DVD authoring, rarely met as a file by this audience; `.stl` is also EBU STL, which is binary |
| SCC | evaluated with phase 18 | US broadcast captions, which this audience meets inside a video stream rather than as a file; its rows, columns and caption modes are positions, the subject of phase 18 |
| MPsub, SubViewer 1, RealText, QuickTime Text | set aside | players that are gone; a third or fourth format behind `.sub` and `.txt` for files nobody brought |
| PJS, VPlayer, JACOsub, AQTitle | set aside | niche tools, with no specification to write fixtures against |

**SBV was weighed and kept.** YouTube's editor also exports SubRip and WebVTT, which argued for leaving it out. It is
kept because a file already written in it has no other form, and it costs one grammar.

**A set-aside format comes back with a person who brings a file**, as an issue of its own: the guard of #836 makes the
cost of a format visible, and that cost is what such an issue weighs.

## D2 — The boundary with phase 18: text and times now, styles and positions later

Phase 18 carries named styles, positions and karaoke. TTML and SAMI can carry styles; the question was whether to read
them now and lose their styles, or wait for the style model.

**They are read now, the way phase 9 read ASS**: the raw text of a subtitle goes through a round trip in its own format
intact (ADR 0009), and the markup pivot (ADR 0031) translates what it knows — bold, italic, underline, colour — on
conversion. What it does not know is lost on conversion, and said (spec 09, D5).

- **Why not wait.** Phase 9 opened ASS long before a style model existed, and nothing in phase 18's scope requires
  another order: phase 18 extends the formats that carry styles, and it needs those formats to exist. A TTML file
  with its italics and without its regions is what a person converting a streaming subtitle to SubRip wants.
- **What phase 18 will find.** The head of a TTML file and the header of a SAMI file are kept as the format's own data
  (spec 09, D3): nothing read here is thrown away before phase 18 can model it.
- **No ADR.** The decision applies ADR 0009 and 0031 to two more formats; it reverses nothing and costs nothing to
  extend.

## D3 — What each format reads and writes

| | YouTube SBV | SAMI | TTML |
| :- | :---------- | :--- | :--- |
| Timing | `H:MM:SS.mmm,H:MM:SS.mmm`, start and end | `<SYNC Start=…>` in ms; a line lasts until the next `SYNC`, and a `SYNC` holding only `&nbsp;` clears it | `begin`, `end`, `dur` on `body`, `div`, `p`, `span`, nested; clock and offset times; frames and ticks at the rates the file declares |
| Ends | carried | carried when the file clears its lines; otherwise the next start, and the last line's end deduced (spec 09, D1) | carried, or computed from `dur` |
| Line break | a new line | `<br>` | `<br/>` |
| Markup | none | HTML: `<i>`, `<b>`, `<u>`, `<font>` | `tts:` attributes, inline or by referenced `<style>` |
| Kept from the file (spec 09, D3) | nothing | the header: `<HEAD>`, `<STYLE>`, its comments | the `<head>`: styling, layout, metadata |
| Encoding | UTF-8 | often a legacy code page (CP949, CP1252): phase 8 detects it | UTF-8 |
| Its writer cannot say | italics, any markup | the languages but one (D4) | nothing the pivot knows |
| The scene | `Bytes`, ends carried, no italics | `Bytes`, ends carried, HTML italics | `Bytes`, ends carried, `tts:fontStyle` italics |

**Names, options and extensions:** `YouTube SBV`, `sbv`, `.sbv`; `SAMI`, `sami`, `.smi`; `TTML`, `ttml`, `.ttml`.
DFXP and IMSC are read as TTML: one format, three names a file may be known by. The window's open filter also lists
`.sami`, `.dfxp` and `.xml`, the extensions those formats are found under (#842).

**Writing TTML** produces one `<div>` and one `<p>` per subtitle, in clock times to the millisecond, so a TTML written
by subedit never needs a frame rate. A file read in frames or ticks and saved back is written in clock times: the
times are kept, not their unit, and the save says so.

## D4 — SAMI is read by hand, tolerantly, one language at a time

**An XML parser would refuse most real SAMI files**: `<P>` left unclosed, upper and lower case mixed, attributes
without quotes. The reader is written by hand, as the other grammars are, and tolerates what browsers tolerated.

**A SAMI file can carry several languages**, each `<P>` naming its `Class` (`KRCC`, `ENCC`). The model holds one text
per subtitle, and a translation column (phase 11) that a reading does not fill.

- The reading takes **the first class the `<STYLE>` declares**, or the first met if none is declared, and names the
  others it left.
- Saving such a file writes one language; the loss is announced **before writing**, as a lossy format's is
  (`GUI-FORMAT-03`), and on the command line the account of the save says it.
- Choosing another language, or opening the second as the translation, is open point 1.

## D5 — TTML is read with expat

[ADR 0049](../adr/0049-expat-to-read-xml.md). The core gains its first XML reader, and the command line its third
library. Expat is namespace-aware, small, already loaded by the window through fontconfig, and loads no external entity.

## D6 — Detection keeps its rule, with two more declarations

**"Nothing rather than a guess, the extension plays no part"** (spec 09, D6) holds. Detection is a cascade in which a
declaration is stronger than a line shaped a certain way; two of the three new formats declare themselves.

| Where it goes | What it checks | What it must not be confused with |
| :------------ | :------------- | :-------------------------------- |
| with `WEBVTT` and `ScriptType:`, before any time line | `<SAMI>` as the first element, any case | an HTML page: `<SAMI>` is not HTML |
| the same | a root `tt` in a TTML or DFXP namespace, after the prolog and comments | any other XML, including a `.xml` that is not TTML, which is refused |
| after SubViewer 2 | a line `H:MM:SS.mmm,H:MM:SS.mmm` | SubViewer 2's `HH:MM:SS.cc,HH:MM:SS.cc`: same length, punctuation elsewhere; SubRip, which needs an arrow |

**This is a hypothesis to be measured, as in phase 9.** Each format issue adds its fixtures to `src/test/data/`, and
`make score-format` must show them recognised and **no confusion**, with the nine formats' score unchanged. A comment
`<!-- … -->` in a SAMI or TTML file contains an arrow-like `-->`; SubRip's line check needs a comma stamp on both sides
of it, which the fixtures exercise.

## D7 — Where the truth of a reading comes from, format by format

The rule of the initialisation (above), applied.

| Format | The authority | Second opinion by hand |
| :----- | :------------ | :--------------------- |
| YouTube SBV | none published: the platform's help page; fixtures written by hand against it | none: `ffmpeg` 6.1 does not read it |
| SAMI | Microsoft's *Understanding SAMI 1.0* | `ffmpeg -i file.smi out.srt` |
| TTML | W3C TTML1 (third edition) and TTML2, IMSC 1.1 for the text profile; each fixture names the section it exercises | `ffmpeg` writes TTML but does not read it: nothing for the reader |

**No fixture is copied from a real file.** The shapes of streaming services' files — referenced styles, ticks at
10 000 000 per second, a region per line — are written by hand (#842).

## D8 — The two surfaces

What the guard of #836 already holds, each format issue writes, and the guard fails until it is written:

- `--to sbv`, `--to sami`, `--to ttml`, for `convert` and `transcribe`, derived from `kSubtitleFormats`;
- the window's open filters and `Save As…` formats, derived the same way;
- the format names in `core/wording/formats`; **the names are not translated**, so the catalogues do not move for them;
- the reading's diagnostics (D4's languages left, TTML's declared or default frame rate) and the conversion's account
  of losses: new messages, in the 20 catalogues;
- the manual's tables in 21 trees, and `inspect`'s list of formats.

## D9 — How the phase proves itself

- Each format: its rendering of the scene, its row in the promise table, its own fixtures for what the scene does not
  exercise.
- The three measurements move format by format: the promise table, the detection score with no confusion, the
  conversion matrix (`docs/mesures/conversion.md`) with a row and a column more.
- The requirements below, registered `planned` now, turned `implemented` by the issue that delivers them.

## What the phase does not deliver

- binary formats (EBU STL, Cheetah CAP, PAC);
- image formats and OCR (phases 20 and 21);
- embedded tracks (phase 19);
- the style and position model, TTML's regions and SAMI's classes as styles (phase 18);
- Spruce STL and SCC, evaluated with phase 18 (D1);
- writing TTML in frames or ticks, or with a head built from styles (D3).

## Requirements

Six requirements of phase 9 said "the nine formats": `CLI-FORMAT-01` to `03`, `CLI-CONVERT-04`, `GUI-FORMAT-01` and
`02`. They now say **every format**, which keeps them true as formats arrive; each format issue extends the tests that
cite them. Five are new, registered `planned`.

| Identifier | What it promises |
| :--------- | :--------------- |
| `CLI-FORMAT-05` | a SAMI file opens in its first language, and the reading names the languages it left |
| `CLI-FORMAT-06` | a TTML or DFXP file opens whatever its time expressions, nested times included |
| `CLI-FORMAT-07` | a YouTube SBV file opens, and is rewritten byte for byte |
| `CLI-FRAMES-03` | a TTML file in frames or ticks opens at the rate it declares, or at TTML's default, and the reading names it |
| `GUI-FORMAT-04` | saving a SAMI file read with several languages announces the languages that will be lost before writing |

## Split

**Five slices**, in milestone 17, in English.

| Slice | Issue | What it delivers | Depends on | Size |
| :---- | :---- | :--------------- | :--------- | :--- |
| 1 — SBV | [#839](https://github.com/Guyot-Bertrand/sub-edit/issues/839) | the first format through every surface, `CLI-FORMAT-07` | — | M |
| 2 — SAMI | [#840](https://github.com/Guyot-Bertrand/sub-edit/issues/840) | the tolerant reader, the languages, `CLI-FORMAT-05`, `GUI-FORMAT-04` | 1 | L |
| 3 — TTML | [#841](https://github.com/Guyot-Bertrand/sub-edit/issues/841) | expat, TTML's text and times, `CLI-FORMAT-06`, `CLI-FRAMES-03` | 1 | L |
| 4 — TTML styles | [#842](https://github.com/Guyot-Bertrand/sub-edit/issues/842) | the pivot reads and writes TTML's markup, the losses named, the extra extensions | 3 | M |
| 5 — closure | [#843](https://github.com/Guyot-Bertrand/sub-edit/issues/843) | review of the phase, the measurements compared, the version | all | M |

**SBV comes first because it is the cheapest**: it lays the path a format takes through the code, the manual and the
measurements, with nothing else to learn at the same time. Slices 2 and 3 are independent of each other.

## Open points

1. **Choosing a SAMI language**, or opening the second as the translation column. It needs an option on the command
   line and a question in the window; it waits for a person with a two-language file.
2. **The XML declaration's encoding.** Phase 8 decides the encoding before any format reads, and TTML is UTF-8 in
   practice. A file declaring another encoding is read as phase 8 decoded it; whether the declaration should be
   honoured or only reported waits for such a file.
3. **Writing TTML in its unit of origin.** A file read in ticks is written in clock times (D3); keeping the unit would
   need the document to remember it, which phase 16's work on frame rates may make cheap.
