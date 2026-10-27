# 0047 — Align by matching the text to the words an engine hears

**Date:** 2026-10-27
**Status:** accepted

Decided at the scoping of [#804](https://github.com/Guyot-Bertrand/sub-edit/issues/804), part of phase 23; the decisions are D1 to D3 of [its spec](../specs/23-alignment.md).

## Context

Phase 23 re-times an existing subtitle file on the audio. The port for it exists since [0046](0046-a-port-for-transcription-engines.md) (`AlignmentEngine`: audio and lines in, one optional start and end per line out), with a contract and no adapter. whisper.cpp, the only engine installed, transcribes; it does not align a known text. Nothing is shipped and subedit downloads nothing, so whatever aligns must be something the person already has or installs on purpose. A real film showed that the words whisper times are approximate, and good enough to follow the speech once a voice-activity model keeps the silences out.

## Decision

1. **The first adapter transcribes, then matches**: it runs a transcription engine of the registry and aligns the normalised words of the text to the normalised heard words, in order, by an edit-distance alignment over words. A line takes the start of its first matched word and the end of its last.
2. **A line is found when at least half of its words are matched** (a setting). A line not found **gets no position**; the operation keeps its original time and flags it.
3. **Doubt is classified, not hidden**: not found, partial (under 80 % of the words), out of line (its shift departs from its neighbours' by more than a setting). The report carries the median shift and the drift.
4. **The matching is a pure function** in the core, tested on versioned cases with a known truth, with no engine.

## Alternatives dismissed

- **A forced aligner as an external executable.** More exact when the text is the transcript, and one more tool, models and languages to install; none is installed here, so it could not be measured. The port makes it an adapter later.
- **Interpolating the lines not found from their neighbours.** It looks tidy and it is a guess the audio did not give; the contract of `AlignedLine` says never to invent a position.
- **Matching on the engine's segments.** The coarsest timing the engine gives.
- **Assuming a model of the fault** (an offset, a linear drift) and fitting it. It would fix the two cases the roadmap names and fail on the third; matching each line on its own makes them one case, and the drift is measured afterwards.

## Consequences

- Aligning costs a transcription of the audio; the surfaces run it in the background and report its progress.
- The text must be in the spoken language; a translation is out of scope.
- A forced-alignment adapter is a later, separate decision, taken on the by-hand run of the phase and not before.
