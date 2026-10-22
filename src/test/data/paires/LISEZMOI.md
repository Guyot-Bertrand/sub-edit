# A translation, and what is done with it when it is opened

Eight cases, one same main document — the scene of [`formats/`](../formats), without
the italics — and eight translations of it, each written to **separate the two ways in
which Gaupol attaches a translation file**: by number, and by position.
Next to each translation, what each method must do with it.

The model has carried the two texts of a subtitle since phase 1, and nothing made them enter through a file.
These cases are what phase 11 will be able to check against what it writes, and they were written **before** — for the reason of
[`formats/`](../formats/LISEZMOI.md): a fixture drawn from one's understanding of
an algorithm, read back by the implementation of that same algorithm, proves only their
agreement.

## The scene

Four subtitles, two of them on two lines. The positions are those of
`formats/scene.srt`.

| # | Start | End | Text |
| -: | :--- | :--- | :---- |
| S1 | 00:00:01,000 | 00:00:03,000 | `The harbour wakes before` ⏎ `the town does.` |
| S2 | 00:00:04,000 | 00:00:06,000 | `Nothing moves on the water.` |
| S3 | 00:00:08,000 | 00:00:11,000 | `A gull turns once above the mast` ⏎ `and settles.` |
| S4 | 00:00:12,000 | 00:00:15,000 | `Then the light comes.` |

**The main document is in English and the translations in French**, so that one never
takes one text for the other when reading a result. Each translation line is
named below by the subtitle it translates: `T1` translates S1, and so on.

## The cases

Eight directories, each holding **six files, no more, no fewer**:

| File | What it is |
| :------ | :----------- |
| `principal.srt` | the main document, the same in all eight cases |
| `traduction.srt` | the translation file opened on top |
| `attendu-numero.principal.srt` `attendu-numero.traduction.srt` | the result of the alignment **by number**, written twice: the main texts, then the translations |
| `attendu-position.principal.srt` `attendu-position.traduction.srt` | the result of the alignment **by position**, likewise |

**A result is a single list of subtitles written twice**, once for
each of its texts, at the same positions: this is what the writer already knows how to produce
(`WriteRequest::document`), and an expected file that a human rereads in a diff rather
than a table to learn. A subtitle that nobody translated is written as a block
without text in the second file; a subtitle born of a translation line,
as a block without text in the first. **A block without text is read without anomaly and
comes back byte for byte** — this is verified, see below.

| Case | The translation | By number | By position |
| :-- | :------------ | :--------- | :----------- |
| `temoin` | four lines, at the main document's positions | each line on its subtitle | **the same** |
| `traduction-plus-courte` | three lines: S4's is missing | S1 to S3 translated, S4 without translation | **the same** |
| `traduction-dans-le-desordre` | the four lines, in the file's order 2, 1, 4, 3 | it is sorted first: like the control case | **the same** |
| `une-ligne-de-moins-au-milieu` | three lines: S2's is missing | **slip**: S2 receives `T3`, S3 receives `T4`, S4 stays without translation | S2 without translation, the other three in their place |
| `une-ligne-de-plus-a-la-fin` | five lines, the fifth at 16–18 s | a subtitle is born at **16–18 s** — the translation's positions | **the same** — *Gaupol, for its part, invents 15–18 s by number: see below* |
| `une-ligne-dans-un-intervalle-vide` | five lines, one at 6.5–7.5 s, between S2 and S3 | **slip**: S3 receives that line, S4 receives `T3`, and `T4` gives birth to a subtitle at 12–15 s, its own | a subtitle is born at 6.5–7.5 s, between S2 and S3; the other four in their place |
| `deux-lignes-dans-un-meme-sous-titre` | S3 translated in two lines, at 8–9.5 s and 9.5–11 s | the second slips onto S4, and `T4` gives birth to a subtitle at 12–15 s, its own | the second **gives birth** to a subtitle at 9.5–11 s; S4 keeps `T4` |
| `positions-decalees` | four lines, all two seconds later | the positions are ignored: like the control case | `T1` falls on **S2**, the other three each give birth to a subtitle; S1, S3 and S4 stay without translation — seven subtitles |

**Four cases agree and four do not, and the four are those that matter**: a case where
the two methods return the same thing teaches nothing about which to choose. The control case
is there to show that they can agree; the other three, to say what does not
separate the methods — a shorter translation, a disordered translation, and an
extra line at the end, **which decision D4 moved from one side to the other**: see
below.

## What each case asks of the scoping

These cases pose their questions before anything is implemented, which is their reason for being.
They are carried over to #426 (decision 4).

- **`une-ligne-de-moins-au-milieu`** — which of the two methods is the default. By
  number, a single missing line shifts **all** the following ones, without anything
  saying so; by position, only one stays without translation.
- **`une-ligne-de-plus-a-la-fin`, `une-ligne-dans-un-intervalle-vide`** — which
  positions a subtitle born of an orphan line receives, and whether this is reported. **Settled by
  D4: its own, by both methods.** The opening's count says so.
- **`deux-lignes-dans-un-meme-sous-titre`** — whether a translation that splits a
  subtitle in two must give birth to a new subtitle.
- **`traduction-dans-le-desordre`** — whether the number of lines moved by the sort, which
  Gaupol returns (here **2**), is reported to the user.
- **`positions-decalees`** — Gaupol has no offset detection: a translation
  timed two seconds too late is not recognised for what it is. To be named, not
  fixed.

## They are written by hand, and there will be no factory

For the reason given in `formats/`: what would verify these files would be the code that
reads and aligns them, and a file generated from our understanding of the alignment would
prove only their agreement.

**What stands in for a third party is a reading of `aeidon/agents/open.py`**, of
`_align_translations_by_number` and `_align_translations_by_position`, and of
`_insert_blank_subtitles` — the function that invents the three seconds. Each expected file
was computed by hand from it: the pointer that never advances, the midpoint of
each line compared with the subtitles' bounds, the 3 s window following the last one.

### And they were observed, once

The sixteen results — eight cases, two methods — were then checked against what
Gaupol actually does: the `aeidon` package copied **outside** the clone, imported from
the copy, `Project.open_main` then `open_translation(path, encoding, method)` on each
case. **All sixteen agreed**, and the number of lines moved in the disordered case is
indeed 2 — *at the time of writing #427*: see the next section for what has changed since.

**This check was done by hand and cannot be replayed.** No test reads the
Gaupol clone: it is absent from CI and from a fresh machine. It can be redone with
a small script — copy `aeidon/` elsewhere, import it, compare — and it is not
in the repository, because a script that depends on the clone would be a test that nobody else
can run.

### What decision D4 changed, and where the expected files are no longer Gaupol's

**Three of the sixteen expected files by number deviate from Gaupol, on purpose.** By number, Gaupol
gives birth to a subtitle for each line that exceeds the main document, **at positions it
invents** — equal durations in the window preceding the next one, three seconds
following the last. The scoping of phase 11 (D4, [`specs/11-traduction.md`](../../../../docs/specs/11-traduction.md))
rules this out: the translation file **states** its line's positions, and inventing them
is a loss. The subtitle born keeps them, by both methods.

The three cases concerned, and what Gaupol does in them:

| Case | What Gaupol invents, by number | What the files now say |
| :-- | :-------------------------------- | :------------------------------------ |
| `une-ligne-de-plus-a-la-fin` | 15–18 s | 16–18 s, the line's own |
| `une-ligne-dans-un-intervalle-vide` | 15–18 s | 12–15 s, those of `T4` |
| `deux-lignes-dans-un-meme-sous-titre` | 15–18 s | 12–15 s, those of `T4` |

**The `attendu-numero.*` files of these three cases were rewritten by #430**, the day the
implementation existed and the harness could check them: they said what Gaupol
does, and they had to say what `subedit` does. The thirteen other results did not
change, and still agree with Gaupol.

**Consequence, and it was noticed by the test of #427 before any reader**:
`une-ligne-de-plus-a-la-fin` no longer separates the methods. This case used to show that Gaupol
does not give the same positions for them; with D4, it shows that they agree. It stays in the
directory for that, and because it is the only one to give birth to a subtitle **at the end**
without a slip.

### What the observation corrected

**Two claims of the issue that asked for these files turned out to be beside the point.**

- It wrote of `positions-decalees` that *"each line gives birth to a subtitle"*.
  **No**: the first line, shifted by two seconds, has its midpoint at 4 s, and 4 s
  falls **within** S2 — it sticks to it. Three lines give birth to a subtitle, not
  four, and S2 receives a translation that is not its own with nothing to tell it
  apart from a correct attachment.
- It marked `deux-lignes-dans-un-meme-sous-titre` as *"read, not observed"*. It is
  now observed, and it is indeed what reading the loop led one to expect:
  the second line gives birth to a subtitle, the pointer having already advanced.

## What these fixtures do not carry

- **Opening a translation on an already translated project erases the old one** — Gaupol
  first empties all translations. The case had been proposed and it is **ruled out**: it
  is not a property of a pair of files but of a project's state, which a
  file cannot carry. It will be tested by a test on an in-memory `Project`,
  in the issue that writes the opening.
- **The number of lines moved by the sort** is a returned value, not a file: it
  is written in the table above, and the test cannot read it here.
- **None of these cases is sampled.** They are written, not drawn from a real file.

## What the harness asks of them

`pairs_corpus_test.cpp`, under `src/test/unit/core/format/`, attaches nothing: it
tests **the data**, and these are the safeguards that no alignment can break.

- the directory and the test's table name the same cases, **in both directions**;
- each case holds exactly its six files;
- each file is SubRip that reads without anomaly and **comes back byte for byte** — a
  block without text included;
- the eight cases start from the same main document;
- the two files of a result have the same positions;
- a subtitle born of a translation line has no main text, and the four
  texts of the scene survive, in their order;
- **each translation line lands exactly once**, whatever the
  method: nothing lost, nothing invented, nothing counted twice — which a hand-written
  expected file can get wrong without a reader seeing it;
- the two methods differ **exactly** where the table says so;
- only the disordered case has a translation whose starts are not sorted.

The first implementation issue will add what is missing: open each translation
on its main document, and compare the result to the expected file byte for byte.

## Why not in `valides/`

Since #289, `validFiles()` (`core/format/corpus_test.cpp`) keeps **every regular
file** of `valides/` and requires that it open: that directory *is* the list. A
`LISEZMOI.md` there would make the test fail, and a translation without its main document would be
read there as a full document. The pairs have their directory, and their own.
