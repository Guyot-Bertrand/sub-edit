# Phase 23 — Automatic alignment

**Status: shipped (#804 to #820), closed in 1.2.0 by [#816](https://github.com/Guyot-Bertrand/sub-edit/issues/816).** The first closure was withdrawn the same day, and the phase delivered a second method ([#820](https://github.com/Guyot-Bertrand/sub-edit/issues/820)): the matching of words (D1 to D8) needs the text in the language spoken; D9 does not.

**Scoped (#804).** The tooling came first (#803); this document decides the rest — the engine, the matching of a line to the
speech, the doubtful alignment, the command and the window — and splits the phase into issues.

What the roadmap promises: re-time an existing subtitle file on the audio. From the text and the voice, find where each sentence is
actually spoken. The case it exists for is the `.srt` fetched online, shifted at the start and at the end, which two-point correction
does not catch.

## What the initialisation found

**State of play, verified** (2026-10-27):

- **The port exists, the adapter does not.** `AlignmentEngine` (`core/transcription/alignment_engine.hpp`) takes an audio file and the
  lines of a text and answers one `AlignedLine` per line, with **no position** for a line not found. It is a header and its contract: nothing
  implements it, the registry holds transcription engines only, and the engine capabilities are shared. Unlike `TranscriptionEngine`, it has
  no template method: **each adapter does the checks before its work itself**, which is what the contract suite below holds it to.
- **whisper.cpp transcribes; it does not align a known text.** What the adapter gives is the tokens' times, and they are the
  engine's own and approximate. **Measured on a real film** (first two minutes, `small` model): without a voice-activity model 15 % of
  the tokens (50 of 335) have no duration and the first word of the film is at 1.0 s though the speech begins at 4.9 s; with the
  Silero model of #798, 2 of 307 have none, the segments follow the speech — **and the tokens are on the timeline with the silences
  removed**, which is the defect #808 fixed (the words are put back on the film's time). Whatever the alignment is built on, it is built on
  those words, and on a voice-activity model.
- **No forced aligner is installed** on the maintainer's machine, and none is shipped (D11 of phase 22: subedit downloads and ships
  nothing).
- **Names already taken**: `align` (the frame grid) and the alignment of a translation (`--align-method`).
- **What `assemble` offers**: it goes the other way (it keeps the times and cuts the text); what is reusable is the rule that the times of a
  piece are kept and the word times are used only inside it.

## The tools, kept

| Tool | Where |
| :--- | :---- |
| the alignment cases: a file at a known fault, what an engine would have heard, the truth | `src/test/data/alignment/`, written by `src/scripts/alignment-cases.py`, replayed by the `fixtures` step |
| the measure of an answer: distance of each found line, lines missed, lines **invented** | `src/test/unit/core/transcription/alignment_cases.hpp` |
| a fake alignment engine and the contract suite of the port | `fake_alignment_engine.hpp`, `alignment_engine_contract.hpp` |

**Discarded, with the reason.** *A synthetic voice for the audio of the cases*: no speech synthesiser is available on every machine, and a
recorded voice is a file nobody can regenerate; the cases are held at the level of the transcript, and the audio is the by-hand run.

## D1 — Which engine aligns: the words whisper hears, matched to the text

**The first adapter transcribes, then matches.** `TranscriptMatchingAligner` implements `AlignmentEngine` by running a
`TranscriptionEngine` of the registry (whisper.cpp, with the voice-activity model when the setting names one) on the audio, and matching
the lines of the text to the timed words it gives. Nothing new is installed: whoever can transcribe can align.

Dismissed for now, with the reason:

- **A forced aligner as an external executable** (a CTC aligner, a Montreal-style tool). It is more exact on a text that is the transcript
  and is also one more thing to install, with its own models and languages; none is installed here, so nothing could be measured. The port is
  the answer to "later": such a tool is an adapter and passes the same contract suite. Open point 1.
- **Matching on the segments only.** Segment times are the coarsest thing the engine gives; the words are what whisper times, and they
  are approximate but finer.

The cost is the transcription: a feature film takes the time of a transcription (phase 22 measured two minutes of film in about one).
The window runs it in the background and the command line says its progress as `transcribe` does. A range (`--start`, `--end`) limits it.

## D2 — How a line is matched to speech

The matching is **a pure function of the text and the heard words**, in the core, unit-tested against the cases of #803 with no engine.

1. **Normalise** both sides to comparable words: case folded, accents folded, punctuation and the markup of a line (styles, speaker
   dashes, sound notes in brackets) removed, digits kept as written. The heard words get the same treatment.
2. **Align the two sequences** in order, by an edit-distance alignment over words, with a cost that counts a word that is nearly the same
   (one or two characters apart on a word of five or more) as a match. The result is monotonic by construction: the lines come out in the
   order they are in the file.
3. **A line takes** the start of its first matched word and the end of its last. It is **found** when at least half of its words are
   matched (setting `alignment.min-match`, default 50 %), and at least one; a line of a single word is found only if that word is.
4. **A line spoken nowhere** (music, a foreign language, a credit that is not said) has no match and **gets no position**: the contract
   of `AlignedLine`, held by the suite. The engine never invents one.

A translation is out of scope: the text must be in the language that is spoken. The language is the one the transcription detects, or
`--language`.

## D3 — What a doubtful alignment is, and what is done about it

The core classifies each line, after the matching. Three kinds, in this order:

| Kind | Meaning | Threshold |
| :--- | :------ | :-------- |
| **not found** | the matching gave no position | below `alignment.min-match` |
| **partial** | found, but few of its words were heard | matched fraction below `0.8` |
| **out of line** | found, but its shift from the original time departs from the shift of its neighbours | by more than `alignment.doubt-gap` (default 2000 ms) from the median shift of the five lines around it |

Every median here is the **lower** of the two middle values when the count is even, so that a single outlier among few neighbours cannot pull the median onto itself. The classification is the first kind that holds: a partial line is not also called out of line.

The result also carries **the shift of every found line** and two figures that tell the story of the file: the median shift, and the
**drift** — the difference between the median shift of the last tenth of the found lines and of the first tenth. A constant offset reads as a
large median and no drift; a progressive drift reads as both.

What is done, **never inventing a position**:

- a **found** line takes its new start and end, a **partial** or **out of line** line too but is **flagged**;
- a line **not found** keeps its original time and is flagged. The operation does not interpolate: a guess is a position the audio did not
  give. Open point 2 asks whether an option should.
- the **order is kept**: a new time that would overlap a neighbour is trimmed to it and flagged; the rule is written with the operation (slice 3), not here.

The report is the same on both surfaces: counts of each kind, the median shift, the drift, and the numbers of the flagged lines.

## D4 — What it changes

**Starts and ends only**, never the text, the style, the number of entries or their order. In the window, as one undoable operation.

- **The whole file** by default; `--start` and `--end` (the grammar of `transcribe`) restrict the audio to a range, and only the lines
  that begin in it are touched.
- **A selection** in the window aligns the selected lines on the audio from the start of the first minus a margin to the end of the last
  plus the margin (60 s), so that a shifted selection is still inside what is heard. The lines outside are untouched.
- **A constant offset and a progressive drift** are the same case for the matching: every line is matched on its own, so no model of the
  fault is assumed. The two figures of D3 describe it afterwards.

## D5 — The command: `sync`

`subedit-cli sync` — *synchronise the subtitles with the audio*. `align` is the frame grid and `adjust` is the manual shift; `sync` collides
with nothing in the command line or the window.

```console
$ subedit-cli sync film.fr.srt --video film.mkv --output film.fr.synced.srt
```

- the subtitle file is the positional argument; `--video` (or `--audio`) gives the sound, with `--audio-track`, `--language`,
  `--model`, `--vad-model`, `--start`, `--end` as for `transcribe`;
- `--output` as for the other commands (in place only when asked, nothing written on failure);
- `--min-match` (percent) and `--doubt-gap` (milliseconds) over the settings;
- `--report` writes the flagged lines to the file it names; `--format json` writes one record (ADR 0038) with the counts, the median shift,
  the drift and the flagged numbers;
- a flagged line is **not a failure**: exit code 0, the report says it. A run that finds no line at all (a wrong audio, a wrong language) is
  an error with a non-zero code and nothing written;
- a missing executable, model or extraction tool is reported as `transcribe` reports it.

## D6 — The window

`Synchronize with Audio…` in the Video menu, next to `Transcribe Audio…`; the project's video is the audio, as for the transcription.

1. A dialog takes the language, the model, the scope (the whole file, or the selection) and the two thresholds.
2. The work runs in the background with the progress box of phase 22, and can be cancelled, a cancelled or failed run leaving the project
   unchanged.
3. **The preview of phase 22 is reused**: the current times on the left, the new ones on the right, a selected line showing its
   shift; the flagged lines are marked and can be filtered to. The person applies the result as one undoable operation, or ignores it.
4. The table of the window does not get a column for the flags in this phase (open point 3). The dialog of the transcription serves, with the three choices of the alignment added and the question about notes taken out; the undo menu names the operation (`CommandKind::Synchronize`).

## D7 — Settings and wording

| Key | Option | Default | Meaning |
| :-- | :----- | :------ | :------ |
| `alignment.engine` | — | `transcript-matching` | the adapter chosen in the registry |
| `alignment.min-match` | `--min-match` | `50` | the share of a line's words that must be heard for it to be found, in percent |
| `alignment.doubt-gap` | `--doubt-gap` | `2000` ms | how far a shift may depart from its neighbours' |

**Delivered with the command (#813).** The settings are read from and written to the settings file, and the account of the operation and the reasons for failure are worded in `core/wording/alignment`, where a person first sees them.

The model, the voice-activity model and the engine of the **transcription** are those of phase 22; there is no second set. The account of
the operation and the reasons for failure live in a new family of `core/wording/` (`alignment`), shared by the two surfaces.

## D8 — How the phase proves itself

- the cases and the measure of #803, now with the real matching under them: **the figure that must hold** is that every case that has a
  truth ends at a worst distance below the tolerance of the words (500 ms), with **no line invented** and the lines the cases leave unheard
  reported as missed, not placed;
- the contract suite of the port, passed by the adapter, with a fake transcription engine under it;
- a **by-hand run** on a real film outside the gate, with a shifted file, written with the adapter and never reported as passed when
  skipped (as for #770);
- the requirements below, registered `planned` in `docs/exigences.md`, moved to `implemented` by the issue whose test cites them.

## D9 — Laying the file on the speech, whatever the language (#820)

**The matching of words needs the text in the language that is spoken** (D2), and the case it was proved on — a text written by an engine, heard
again by another — was the only one. A person who has a film in its original language and a subtitle file in theirs, nearly right, gets nothing from
it: every line is "not found". The goal is the **times** of the file, not its words, and the times can be found without the words.

**The method.** Ask the sound only *where somebody speaks* (the voice-activity model of phase 22, run alone by `whisper-vad-speech-segments`), and
find the correction that makes the subtitles agree with it:

1. **The frame rate** is chosen among the ratios between any two of 23.976, 24, 25, 29.97 and 30, and 1; a ratio other than 1 is taken only when it
   correlates 1.15 times better than 1.
2. **The shift** is found for each stretch of two minutes, every thirty seconds, as the shift that best correlates the two curves (speech, subtitle
   shown) within ±150 s; the **path through those shifts** is the one that agrees most and changes least (a Viterbi path, with a cost for changing the
   shift and a small one for being far from 0). A constant offset, a drift and a second half that is late are all paths.
3. **The correction** is the ratio and the shifts, interpolated between the stretches; each start and each end goes through it.
4. **Agreement** — the median, along the path, of the correlation — is measured; under 0.15 the file is refused as having too little in common with
   the sound.

**What it measured** (observations on three episodes of a TV series with English sound and French subtitles, not guarantees): with the file's own
times as the truth and a known fault laid on it — late, early, drifting, half late, another frame rate —, every start lands within 500 ms in two
episodes and 91 % in the third (the teaser, with music), in about thirteen seconds for forty minutes of film; a file of another episode gives an
agreement under 0.1, against 0.3 for the right one.

**What it does not do — and is told, every time.** It corrects the file as a whole. It cannot find a line that is out of place alone in a file that is
otherwise right: a line moved by a few seconds almost always falls on other speech. **A second stage that re-placed each line on the speech next to it
was tried and discarded**: it recovered 5 to 22 % of the displaced lines and damaged 20 to 60 % of the good ones; flagging lines with little speech under
them caught a fraction of the displaced lines with many false alarms. A title card shown over a silence (a chapter card, a caption translating
something on screen) has no speech to lean on: it moves with the file and may be wrong.

**Which method runs.** `speech` is the default (`alignment.method`, `--method`, the choice of the dialog); `words` stays for a text in the language
spoken, where it alone can place one line on its own. The two are not chained: the choice is the person's. Chaining — correcting the file by the speech,
then refining each line by its words — is worth an issue when a real file in the language spoken asks for it.

**Said, not hidden.** Both surfaces carry the warning of what `speech` does not do — the command on the error output and as a `speech-only` warning
of the record; the dialog under the choice of the method, and the preview under its summary — and a file that does not fit the sound is refused with
the reason (`noAgreementWithTheSpeech`), as the failure of the words is followed by the pointer to the method that ignores the language.

**Requirements:** `CLI-SYNC-07`, `CLI-SYNC-08` and `GUI-SYNC-05`.

**Not delivered, noted.** Chaining the two methods; a placement of the cards over a silence (it would need a model of what a card is); a route for the
single misplaced line in a file whose language is not the one spoken (align a file in the language spoken by its words, then carry its times over by
overlap, as the alignment of a translation of phase 13 does).

## What the phase does not deliver

Transcription itself (phase 22), speaker identification, styles and positions (phase 18), live alignment, model training, the alignment
of a translation (the text must be in the spoken language), a forced-alignment adapter (open point 1), and a column of flags in the table
(open point 3).

## Requirements

**Thirteen**: ten registered with this spec, and three of #820 (`CLI-SYNC-07`, `CLI-SYNC-08`, `GUI-SYNC-05`).

| Identifier | What it promises |
| :--------- | :--------------- |
| `CLI-SYNC-01` | `sync` re-times a subtitle file on the audio of a video or an audio file and writes the result in the format of `--output`, changing the starts and ends only |
| `CLI-SYNC-02` | a line not found keeps its original time, is reported, and the command succeeds; a run that finds no line is refused with a non-zero code and nothing written |
| `CLI-SYNC-03` | `--start` and `--end` restrict the audio, and only the lines that begin in the range are touched |
| `CLI-SYNC-04` | the report gives the counts of lines found, partial, out of line and not found, the median shift and the drift, and `--format json` writes them in one record |
| `CLI-SYNC-05` | `--min-match` and `--doubt-gap` override the settings, and a value out of range is refused before any work |
| `CLI-SYNC-06` | a missing executable, model or extraction tool is reported with a pointer to the installation page, with a non-zero exit code and nothing written |
| `GUI-SYNC-01` | `Synchronize with Audio…` opens a dialog offering the language, the model, the scope (the whole file or the selection) and the thresholds |
| `GUI-SYNC-02` | the work runs in the background with a progress indicator and can be cancelled, a cancelled or failed run leaving the project unchanged |
| `GUI-SYNC-03` | the result is shown in a preview — current times and new times side by side, the flagged lines marked — and applies as one undoable operation |
| `GUI-SYNC-04` | without the executable, the model or the extraction tool the command explains what is missing and where to read how to install it, and the rest of the window is unaffected |
| `CLI-SYNC-07` | `sync --method speech`, the default, lays the file on when there is speech without transcribing, whatever the language of the subtitles, and refuses, with nothing written, a missing voice-activity model or program, and a file that has too little in common with the sound |
| `CLI-SYNC-08` | the warning that only the file as a whole was put in line is said every time `speech` runs, in the output and as a `speech-only` warning of the record |
| `GUI-SYNC-05` | the dialog offers the method, speech first, says what speech does not do, and the preview repeats it; a file that does not fit the sound is refused with the reason |

## Split

**Seven slices.** All in milestone 23, written in English.

| Slice | Issue | What it delivers | Depends on | Size |
| :---- | :---- | :--------------- | :--------- | :--- |
| 1 — matching | [#810](https://github.com/Guyot-Bertrand/sub-edit/issues/810) | the normalisation and the word alignment as a pure function, tested on the cases of #803 | — | L |
| 2 — doubt | [#811](https://github.com/Guyot-Bertrand/sub-edit/issues/811) | the classification of D3, the shifts, the median and the drift | 1 | M |
| 3 — adapter | [#812](https://github.com/Guyot-Bertrand/sub-edit/issues/812) | `TranscriptMatchingAligner` over a transcription engine, registered, the contract suite passed; settings and the `alignment` wording | 1, 2 | L |
| 4 — command | [#813](https://github.com/Guyot-Bertrand/sub-edit/issues/813) | `sync`, the report, the manual and `CLI-SYNC-01` to `06` | 3 | L |
| 5 — window | [#814](https://github.com/Guyot-Bertrand/sub-edit/issues/814) | the dialog, the background run, the preview, the undoable operation, `GUI-SYNC-01` to `04` | 3 | L |
| 6 — by hand | [#815](https://github.com/Guyot-Bertrand/sub-edit/issues/815) | the real-film run with a shifted file, documented, never reported as passed when skipped | 4 | M |
| 7 — closure | [#816](https://github.com/Guyot-Bertrand/sub-edit/issues/816) | review of the phase, then the version the maintainer decides | all | M |

## The by-hand run (#815)

`src/scripts/alignment-by-hand.sh` runs `sync` on a real film, outside the gate, and prints a table. It transcribes the first minutes of the film
with one model (the *truth*), makes from that file four files at a known fault — **late** by 2.4 s, a **drift** that grows from nothing to 3 s,
the **second half** late by 2 s, and a line that nobody says (**extra**) —, runs `sync` on each with another model, and measures how far each
start lands from the truth. `--control` adds a run with the model that wrote the truth. A run that lacks a tool or a model says
`SKIPPED`, names it and exits 77: **a skipped run is never a passed one.**

**What it proves, and what it does not.** The truth comes from an engine, not from a person: the table says whether the matching finds its way
back from a known fault with the words a second model heard, not whether the engine's times are those of the speech. The engine's own text is
also cleaner than a subtitle written by a person; a real subtitle file is a harder case.

**Measured on 2026-10-31**, on the first five minutes of a real film (103 subtitles), truth by `medium` and `sync` with `small` and the Silero
model, on twelve cores:

| Case | Median | 95th percentile | Worst | Within 500 ms | Not found | Flagged | Time |
| :--- | -----: | --------------: | ----: | ------------: | --------: | ------: | ---: |
| late | 260 ms | 2440 ms | 16300 ms | 71 of 103 | 1 | 22 | 146 s |
| drift | 260 ms | 2440 ms | 16300 ms | 71 of 103 | 1 | 22 | 142 s |
| second half late | 260 ms | 2440 ms | 16300 ms | 71 of 103 | 1 | 23 | 140 s |
| extra line | 230 ms | 2440 ms | 16300 ms | 72 of 103 | 2 | 24 | 145 s |
| control (`medium` listens) | 0 ms | 0 ms | 0 ms | 103 of 103 | 0 | 0 | 445 s |

What it says:

- **The matching is exact when the same words are heard.** With the model that wrote the truth, every start lands on the truth, the median shift is
  the 2.4 s that was laid down, and nothing is flagged.
- **The fault does not matter.** The four faults give the same figures: each line is matched on its own, so a constant offset, a drift and a
  half-late file are one case, as D4 has it. The median shift and the drift of the report are what tell them apart.
- **What remains with another model is the models' disagreement**, and a few mishearings. Two models do not put the start of a word at the same
  moment: 71 of 103 starts are within 500 ms, the others mostly within a second, the median at a quarter of a second. Eight starts are more than two
  seconds out, one by sixteen: the small model misheard the line or matched a repeated phrase elsewhere.
- **The doubt catches them.** Seven of the eight starts more than two seconds out are flagged — "only partly heard" or "moved differently from its
  neighbours" —; the eighth is not. Twenty-two lines are flagged in all, which is what a person is asked to look at.
- **A line nobody says keeps its time**, and is reported.
- **Time.** Five minutes of film take about two and a half minutes with `small` and the voice model, and seven and a half with `medium`: the cost
  is the transcription (D1), and grows with the model and the length of the part listened to.

**Decision on a forced aligner (open point 1): not yet.** The matching is exact on the words it is given, and the errors that remain are the
listening model's — its word times, and its mishearings. A forced aligner would know the true words and would remove the mishearings; it would not
change the other half, and it is one more tool to install. The evidence does not call for it; a larger model (`medium` gave no error at all as the
reference) and the flagged lines are the answer for now. It is reconsidered if a real subtitle file, with its own text, shows many lines not found.

## Open points

1. **A forced-alignment adapter.** Judged on the real-film run (see above) and not called for: the matching is exact on the words it is given. Worth an
   issue only if a real subtitle file, with its own text, shows many lines not found, or if a language the transcription handles badly matters. It
   is an adapter and a contract suite, nothing else.
2. **Interpolating a line not found.** Between two corrected lines, an option could place it at the neighbours' shift. It is a guess; it
   waits for a person who asks for it.
3. **A flag column in the table.** It needs a place in the project model that is not the undo stack; it waits for the first use of the
   preview to show whether the flags are read there.
4. **Other languages than English** are untested, as for phase 22.
