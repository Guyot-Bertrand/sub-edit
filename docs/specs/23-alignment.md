# Phase 23 — Automatic alignment

**Status: the tooling only.** The initialisation of the phase (#803) laid the tools the alignment will be held against; the
scoping (#804) writes the rest of this document — the engine, the matching of a line to the speech, the doubtful alignment, the command
and the window — and replaces this paragraph.

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
*A by-hand run documented now*: there is nothing to run before the first adapter; it is written with it, and never reported as passed when
skipped (as for #770).

## Requirements

**None for the tooling**: the registry (`docs/exigences.md`) holds what the command line and the window show, proven by the end-to-end and
interface binaries; these tools are proven by the unit tests that use them (`alignment_engine_test.cpp`: the cases read and their truth,
the measure, the fake engine and the contract). The requirements of the alignment itself come with the scoping (#804).
