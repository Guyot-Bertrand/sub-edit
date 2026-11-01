# 0048 — Lay the file on the speech by default

**Date:** 2026-11-01
**Status:** accepted

Decided in [#820](https://github.com/Guyot-Bertrand/sub-edit/issues/820), the reopening of phase 23; the decision is D9 of [its spec](../specs/23-alignment.md). It amends [0047](0047-align-by-matching-the-text-to-heard-words.md), which stays true for the matching of words.

## Context

The matching of words (0047) needs the text in the language that is spoken. It was proved on one case, a text written by an engine and heard again by another, and the phase was closed. Tried by hand on a film in English with a good subtitle file in French, `sync` found no line and did not say why. What a person wants is the **times** of the file, not its words, and a person who has a film in its original language and subtitles in theirs is the common case, not a corner.

## Decision

1. **`sync` has two methods, and `speech` is the default** (`alignment.method`, `--method`, a choice of the dialog). It asks the sound only where somebody speaks — `whisper-vad-speech-segments`, the voice-activity model of phase 22 run alone — and finds the correction that lays the subtitles on it: a frame rate among the usual ratios, then a shift per stretch of two minutes, through a path that agrees most and changes least. It depends on no language, transcribes nothing and takes seconds.
2. **It corrects the file as a whole, and says so every time.** The command warns on its error output and in its record (`speech-only`); the dialog says it under the choice and the preview repeats it. A file whose agreement with the sound is too low is refused with the reason, not written.
3. **`words` stays**, for a text in the language spoken, where it alone can place one line on its own. The two are not chained; the person chooses.
4. **No second stage on the speech.** Re-placing each line on the speech next to it was measured and discarded: dialogue is continuous, so a line moved by a few seconds almost always lands on other speech.

## Alternatives dismissed

- **Keeping the words as the only method, and a clearer message.** It tells the person why it failed and still serves them nothing.
- **A per-line stage on the speech.** Measured on files with a known fault: it recovered 5 to 22 % of the displaced lines and damaged 20 to 60 % of the good ones.
- **A forced aligner or a cross-language model.** One more thing to install, none measured here; the speech method needs nothing the transcription does not already need.
- **Chaining the speech correction and the matching of words.** Worth doing when a real file in the language spoken asks for it; not before.

## Consequences

- The default of an existing command changes: `sync` needs a voice-activity model and the program `whisper-vad-speech-segments`, and the manual says so; the old behaviour is `--method words`.
- Everything after the placement — the plan, the flags, the report, the preview, the undoable operation — is shared by both methods, because the speech method answers in the shape of the matching.
- The agreement threshold (0.15) and the penalties of the path were set on a few episodes of a series; they are observations, and the cases that would turn them into guarantees are fixtures that lay a known fault on a versioned file.
