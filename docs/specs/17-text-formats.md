# Phase 17 — Additional text formats

**Status: the tooling only.** The initialisation of the phase (#836) checked the state of play, drew up the census below and laid the
guard a new format is held to. The scoping (#837) writes the rest of this document and replaces this paragraph:
- which formats are kept;
- where this phase ends and phase 18 begins;
- the detection;
- the two surfaces.

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
  - a format without ends (SAMI clears a line with the next `SYNC`) is promised `BytesEmptily`, as TMPlayer and LRC are.

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
