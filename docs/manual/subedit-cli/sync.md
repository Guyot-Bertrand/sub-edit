# `sync`

```
subedit-cli sync (--video FILE | --audio FILE)
                 [--audio-track N]
                 [--language CODE]
                 [--start TIME] [--end TIME]
                 [--model FILE]
                 [--vad-model FILE]
                 [--engine NAME]
                 [--keep-audio FILE]
                 [--method METHOD]
                 [--min-match PERCENT]
                 [--doubt-gap MS]
                 [--report FILE]
                 [-r]
                 [--output FILE | --output-dir DIR | --in-place] [--dry-run]
                 <files>...
```

**Re-times a subtitle file on the audio of a film.** The case it exists for is the `.srt` fetched online, shifted at
the start and at the end, which a single shift does not bring back. Two methods do it, and `--method` (or the setting
`alignment.method`) chooses between them:

- **`speech`, the default, lays the file on when somebody speaks.** It asks the sound only where there is speech, and
  finds the shift that makes the subtitles agree with it, stretch of film by stretch of film: a constant offset, a drift,
  a second half that is late, and a file timed for another frame rate (25 for 23.976, and the like) are all put right.
  **It does not depend on the language** — the subtitles may be in French and the film in English —, it needs no
  transcription, and it takes seconds. It needs `ffmpeg`, the program `whisper-vad-speech-segments` that comes with
  whisper.cpp, and a voice-activity model (`--vad-model`, the setting `transcription.vad-model`).
- **`words` matches the words of each subtitle to the words an engine heard.** It listens with a transcription engine —
  the same one as [`transcribe`](transcribe.md) — and each subtitle takes the place where its words are said. Nothing
  is translated: the subtitles must be in the language that is spoken. It can place a single subtitle that is out of
  place in a file that is otherwise right, and it takes the time of a transcription.

**What `speech` does not do is said every time it runs**, on the error output and in the record: it corrects the file
**as a whole**, never one subtitle at a time. A subtitle that is out of place by itself in a file that is otherwise
right, or a title shown over a silence (a chapter card, a caption translating something on screen), is not looked for
and may still be wrong, and the first minutes may stay off when the film opens with music or titles. Check the result,
or use `--method words` when the text is in the language spoken.

**Nothing it needs is shipped with the program**: the programs, the model and `ffmpeg` are the ones of
[Installing the transcription tools](install-transcription.md), and their absence breaks nothing — the command says
what is missing, with the pointer to that page, writes nothing and exits with a non-zero code.

- **Only starts and ends change.** Never the text, the style, the number of subtitles or their order.
- **A subtitle the audio does not give keeps its times** — a credit nobody says, a song, a line in another language.
  **No position is ever invented, and none is guessed from the neighbours**: the line is left where the file had it,
  and the command says so. It is *not* a failure, and the exit code is `0`.
- **With `words`, a subtitle is found when at least half of its words are heard** (`--min-match`,
  `alignment.min-match`), and at least one. Case, accents, punctuation and markup do not matter; a word one letter off
  a long word counts as heard. `speech` places every subtitle, and takes none of `--language`, `--model`, `--engine`
  or `--min-match`.
- **The end is where the last word ends.** The engine times words approximately, a few hundredths of a second before
  the subtitle's own end, so a subtitle may come out slightly shorter than it was.
- **A subtitle that would run into the next is cut back to where the next one begins.** One that still overlaps the
  next, which cutting cannot cure, is left and said.
- **A file that does not fit the sound is refused and not written**: with `words`, none of the subtitles was found,
  which is almost always the wrong audio or the wrong language — `speech` does not depend on the language; with
  `speech`, the subtitles and the speech have too little in common, which is almost always the wrong film.
- **A part of the film can be listened to alone**, with `--start` and `--end`, as for
  [`transcribe`](transcribe.md#transcribing-a-part): only the subtitles that **begin** in the range are looked for and
  touched, the others are left as they are and are not counted.
- **The audio is listened to once for each file given.** To re-time several files on the same film, expect to wait for
  each.

<!-- example: subedit-cli sync --help -->
```console
$ subedit-cli sync --help
Re-time the subtitles of a file on the audio of a film
Usage: subedit-cli sync [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to re-time

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --video FILE                Film to listen to
  --audio FILE                Audio file to listen to, instead of a film
  --audio-track N             Audio track of a film, counted from 0; the first by default
  --start TIME                Where to begin, as 2:00, 1:02:03 or seconds; the beginning by default
  --end TIME                  Where to stop, as 2:00, 1:02:03 or seconds; the end by default
  --language CODE             Language of the speech, two letters: fr, en, de; detected from the first 30 seconds by default
  --model FILE                Model file; the transcription.model setting by default
  --vad-model FILE            Voice-activity model that skips silences; the transcription.vad-model setting by default
  --engine NAME               Engine that listens; the transcription.engine setting by default
  --keep-audio FILE           Keep the extracted audio in this file
  --method METHOD             How the subtitles are put in line: speech (from when there is speech, whatever the language) or words (from the words heard); the alignment.method setting by default
  --min-match PERCENT         Share of a subtitle's words that must be heard, in percent; the alignment.min-match setting by default
  --doubt-gap MS              How far a subtitle may move from its neighbours' shift before it is flagged, in milliseconds; the alignment.doubt-gap setting by default
  --report FILE               Write the subtitles to check to this file
  --output TEXT               File to write, for a single input
  --output-dir TEXT           Directory to write into
  --in-place                  Write back over the inputs
  --dry-run                   Work out and say what would be written, and write nothing
```

## Arguments and options

| Option | Required | Values | Default |
| :----- | :------- | :----- | :------ |
| `<files>` | yes | the subtitle files to re-time; with `-r`, directories | — |
| `--recursive`, `-r` | no | a flag | off — see [Processing a tree](lots.md) |
| `--video` | one of `--video` and `--audio` | the film to listen to | — |
| `--audio` | one of `--video` and `--audio` | an audio file to listen to, instead of a film | — |
| `--audio-track` | no | a whole number from 0: the position among the audio tracks of the film | `0`, the first |
| `--language` | no | two letters, an ISO 639-1 code: `en`, `fr` | the setting `transcription.language`; empty: detected |
| `--start`, `--end` | no | a time: `2:00`, `1:02:03`, `2:00.5`, or a number of seconds; `--end` must come after `--start` | the whole audio |
| `--model` | no | the model file | the setting `transcription.model` |
| `--vad-model` | no | a voice-activity model, which makes the engine skip the silences; with it the words follow the speech more closely | the setting `transcription.vad-model` |
| `--engine` | no | the name of the engine that listens: `whisper.cpp` | the setting `transcription.engine` |
| `--keep-audio` | no | a file to keep the extracted audio in; its directory is created when missing | the audio is kept nowhere |
| `--method` | no | `speech`: lay the file on when there is speech, in any language; `words`: match the words to what was heard, in the language spoken | the setting `alignment.method`, `speech` |
| `--min-match` | no | a whole number from 1 to 100: the share of a subtitle's words that must be heard, in percent | the setting `alignment.min-match`, `50` |
| `--doubt-gap` | no | a whole number of milliseconds, 0 or more: how far the shift of a subtitle may differ from its neighbours' before it is flagged | the setting `alignment.doubt-gap`, `2000` |
| `--report` | no | a file to write the subtitles to check to, one per line: the number of the subtitle, a tab, and why; one subtitle file at a time | nothing is written |
| `--output` / `--output-dir` / `--in-place` | **one of the three**, except with `--dry-run` | see [Invocation](invocation.md#the-destination) | — |
| `--dry-run` | no | a flag: computes and says, writes nothing — see [Seeing before writing](invocation.md#seeing-before-writing) | off |

The destination — `--output`, `--output-dir`, `--in-place`, `--dry-run` — and `-r` are those of the other commands that
rewrite a file: see [the destination](invocation.md#the-destination). The file keeps its format, its encoding and its
line endings.

The **settings** are read from the settings file — see [the settings file](invocation.md#the-settings-file) — and each
has its option here. The engine, the model and the voice-activity model are those of `transcribe`.

| Key | Meaning | Default |
| :-- | :------ | :------ |
| `alignment.engine` | the alignment engine of the `words` method; there is one, `transcript-matching`, which listens and matches | `transcript-matching` |
| `alignment.method` | how the subtitles are put in line: `speech` or `words` | `speech` |
| `alignment.min-match` | the share of a subtitle's words that must be heard for it to be found, in percent | `50` |
| `alignment.doubt-gap` | how far, in milliseconds, a subtitle's shift may depart from its neighbours' before it is flagged | `2000` |

## What is flagged

The command says which subtitles to check, and why. A subtitle is flagged for the **first** of these that holds:

| Why | Meaning |
| :-- | :------ |
| not found in the audio, left as it was | fewer than `--min-match` of its words were heard: it keeps its times |
| only partly heard | found, but fewer than 80 % of its words were heard |
| moved differently from its neighbours | found, but its shift departs from the middle shift of the two subtitles on each side by more than `--doubt-gap` |

With `speech` every subtitle is placed, and only the last of these can apply: where the file changes its shift by more
than `--doubt-gap` from one subtitle to the next (a second half that is late), the subtitles on each side of the step are
flagged.

A subtitle can also be **cut back to the start of the next subtitle**, or **still overlap the next subtitle**.

Two figures tell the story of the file as a whole. **The median shift** is the middle of how far the subtitles moved: a
file that was simply late reads here, and nowhere else. **The drift** is the median shift of the last tenth of the
subtitles minus that of the first tenth: it is near zero for a constant offset, and large for a file that was late by
more and more along the film.

## Output

**Standard output** — nothing: the result is the file written.

| Level | What is said on the error output |
| :---- | :------------------------------- |
| 1 | `source: <file>`, `media: <audio>`, `result: N of M subtitles placed on the audio, K to check; median shift X s, drift Y s`, `dest: <destination>` — and, when a frame rate was put right, the ratio; with `speech`, **the warning of what it does not do**, on a line of its own |
| 2 | the step and the progress, as for [`transcribe`](transcribe.md#output) |

### In JSON

With [`--format json`](invocation.md#output-a-script-can-read), the object of the file carries `counts`: `subtitles`
(looked for), `placed` (given a place), `moved` (whose times changed), `partial`, `out_of_line`, `not_found`,
`median_shift_ms` and `drift_ms`; and `flagged`, a list of `{"subtitle": N, "kind": "…"}` where the number is the
subtitle's place in the file and `kind` is one of `not-found`, `partial`, `out-of-line`, `cut-back` and `overlaps`.
With `speech`, `warnings` carries one entry of `kind` `speech-only`. The ids are promised; the sentences are not.

## Example

```bash
subedit-cli sync --video film.mkv --vad-model silero.bin \
    --report to-check.txt --output film.synced.srt film.srt
```

To place each subtitle by its words, for a text in the language spoken:

```bash
subedit-cli sync --method words --video film.mkv --model small.bin --vad-model silero.bin \
    --output film.synced.srt film.srt
```

## Exit codes

Those of the tool: `0` if the file was written, flagged subtitles or not, `2` if it could not be — a tool or a model
missing, a failed extraction or listening, a file that does not fit the sound (none of its subtitles found, or too
little in common with the speech) —, `1` on a usage error, such as neither `--video` nor `--audio` (or both), an option
out of its range, or a range that cannot be used.

## Errors

| Cause | Message |
| :---- | :------ |
| neither `--video` nor `--audio`, or both | `sync needs the audio to listen to: give --video or --audio, and not both` |
| no engine program found, or the configured one is not there | `the transcription engine was not found`, then the pointer to the guide |
| no model given, or it is not there | `no model was given, or the model file is not there`, then the pointer to the guide |
| `ffmpeg` not found for a film | `ffmpeg was not found, so the audio of a video cannot be extracted`, then the pointer |
| a track the film does not have | `the video has no audio track N`, then the tracks it has |
| a language the model cannot transcribe | `the model cannot transcribe the language xx` |
| an engine nobody registered | `no transcription engine is called NAME`, or `there is no alignment engine called NAME` |
| none of the subtitles found (`words`) | `none of the N subtitles was found in the audio: is it the right audio, and the right language?`, then `The method based on speech does not depend on the language.` |
| the voice-activity model not named, or the program that finds speech not found (`speech`) | `Finding where there is speech needs a voice-activity model: name one in the transcription.vad-model setting.` or `Finding where there is speech needs the program whisper-vad-speech-segments, which comes with whisper.cpp, and it was not found.`, then the pointer to the guide |
| the subtitles and the speech have too little in common (`speech`) | `The sound and the subtitles have too little in common to be put in line: is it the right film?` |
| `--min-match` outside 1 to 100, `--doubt-gap` negative | the usage error of the option |
| `--report` with more than one subtitle file | `--report names one file: give one subtitle file when you use it` |
