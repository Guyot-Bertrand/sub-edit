# `transcribe`

```
subedit-cli transcribe --output FILE
                       [--to FORMAT]
                       [--audio-track N]
                       [--language CODE]
                       [--start TIME] [--end TIME]
                       [--model FILE]
                       [--engine NAME]
                       [--keep-audio FILE]
                       [--raw]
                       <input>
```

Writes **the speech of a video, or of an audio file, as a subtitle file**. The audio is prepared,
handed to an engine that finds what is said and when, and the pieces it cuts are merged into
subtitles that respect a duration and a length.

**Nothing it needs is shipped with the program**: the engine (whisper.cpp), a model and `ffmpeg`
are installed by the user, who says where the model is. [Installing the transcription
tools](install-transcription.md) tells where to find each, and which model to choose. **Their absence breaks
nothing**: the command says what is missing, with the pointer to that page, writes nothing and
exits with a non-zero code.

- **The input is a video or an audio file.** From a video, `ffmpeg` extracts one audio track; without
  `ffmpeg`, an audio file the engine reads as it is (`wav`, `mp3`, `flac`, `ogg`) is still
  transcribed, but a video is not, and neither is any file when a track is asked for.
- **Only one file at a time**, and the destination is `--output`: there is no `--output-dir` or
  `--in-place`, since the result is a new file and not a rewriting.
- **The language is detected** unless `--language` forces it, and the language used is said. A
  model made for English alone detects nothing and transcribes nothing else: a language it cannot
  take is refused *before* any work starts, naming the model's limit.
- **A language is typed as two letters — `fr`, `en`, `de` —, or as the three-letter code a file declares, `fra`,
  `eng`, which is read as the two-letter one.** Anything else is refused before any work starts. **Detection
  listens to the first thirty seconds only**: a film that opens with music or silence can be taken for
  another language, and the engine then writes the speech *translated* into that language — a French film taken
  for English comes out in English, or as `[SPEAKING FRENCH]`. When you know the language, give it.
- **A part of the film can be transcribed alone**, with `--start` and `--end` — see
  [Transcribing a part](#transcribing-a-part). It is how to try a model on two minutes before running it on
  two hours.
- **Sounds are not written as notes**: the engine is told not to (its own option is `--suppress-nst`), so
  that `(Sonnerie de téléphone)` or `[MUSIC PLAYING]` do not come up in the subtitles, in parentheses as in brackets.
- **The segments are merged and cut** to the limits of the settings (`transcription.merge-gap-ms`,
  `transcription.max-duration-ms`, `transcription.max-characters`): two pieces of one sentence that are
  close are one subtitle, and a piece that is too long or too wide is cut at a word. **`--raw`
  writes the engine's segments as they came**, one per subtitle, which is also how to see what the
  merging does.

<!-- example: subedit-cli transcribe --help -->
```console
$ subedit-cli transcribe --help
Write the speech of a video or an audio file as subtitles
Usage: subedit-cli transcribe [OPTIONS] input

Positionals:
  input TEXT REQUIRED         Video or audio file to transcribe

Options:
  -h,--help                   Print this help message and exit
  --output FILE               Subtitle file to write
  --to TEXT:{srt,vtt,subviewer2,ssa,ass,microdvd,mpl2,tmplayer,lrc}
                              Format to write; the one the extension of --output says by default
  --audio-track N             Audio track of a video, counted from 0; the first by default
  --start TIME                Where to begin, as 2:00, 1:02:03 or seconds; the beginning by default
  --end TIME                  Where to stop, as 2:00, 1:02:03 or seconds; the end by default
  --language CODE             Language of the speech, two letters: fr, en, de; detected from the first 30 seconds by default
  --model FILE                Model file; the transcription.model setting by default
  --engine NAME               Engine; the transcription.engine setting by default
  --keep-audio FILE           Keep the extracted audio in this file
  --raw                       Write the segments as the engine cut them
```

## Arguments and options

| Option | Required | Values | Default |
| :----- | :------- | :----- | :------ |
| `<input>` | yes | a video or an audio file | — |
| `--output` | yes | the subtitle file to write; its directory is created when missing | — |
| `--to` | no | the short name of a format: `srt`, `vtt`, `subviewer2`, `ssa`, `ass`, `microdvd`, `mpl2`, `tmplayer`, `lrc` | the format the extension of `--output` says, else `srt` |
| `--audio-track` | no | a whole number from 0: the position among the audio tracks of the video | `0`, the first |
| `--language` | no | two letters, an ISO 639-1 code: `en`, `fr`; a three-letter code such as `fra` is read as `fr` | the setting `transcription.language`; empty: detected |
| `--start` | no | a time: `2:00`, `1:02:03`, `2:00.5`, or a number of seconds, `90` | the beginning of the audio |
| `--end` | no | a time, written as for `--start`; it must come after `--start` | the end of the audio |
| `--model` | no | the model file | the setting `transcription.model` |
| `--engine` | no | the name of an engine: `whisper.cpp` | the setting `transcription.engine` |
| `--keep-audio` | no | a file to keep the extracted audio in, as 16 kHz mono 16-bit PCM in a WAV file; its directory is created when missing | the audio is kept nowhere |
| `--raw` | no | a flag: write the engine's segments with no merging and no cutting | off |

The **settings** are read from the settings file like those of [`adjust`](adjust.md) — see
[the settings file](invocation.md#the-settings-file) — and each has its option here:

| Key | Meaning | Default |
| :-- | :------ | :------ |
| `transcription.engine` | the engine | `whisper.cpp` |
| `transcription.executable` | the engine's program, an absolute path; empty: searched for on `PATH` | empty |
| `transcription.model` | the model file, an absolute path | empty |
| `transcription.language` | the language of the audio, two letters; empty: detected | empty |
| `transcription.merge-gap-ms` | pieces of one sentence closer than this many milliseconds are joined | `400` |
| `transcription.max-duration-ms` | the longest a subtitle stays on screen, in milliseconds | `7000` |
| `transcription.max-characters` | the most characters a subtitle holds, line breaks not counted | `84` |

## Choosing the audio

A video can carry several audio tracks — the original, a commentary, another language. `--audio-track`
takes the **position among the audio tracks** of the file, from 0, in the order the file lists them. A
track the video does not have is refused, **with the tracks it does have**, so that the next call is
the right one:

```bash
subedit-cli transcribe --audio-track 4 --output film.srt --model small.bin film.mkv
```

```text
film.mkv: the video has no audio track 4
audio track 0: fra, Original
audio track 1: eng, Commentary
```

## Transcribing a part

`--start` and `--end` restrict the work to the part of the film between them. Either can be given alone: a
`--start` alone goes to the end, an `--end` alone begins at the start.

```bash
subedit-cli transcribe --start 2:00 --end 3:30 --output test.srt --model large-v3.bin film.mkv
```

- **A time is typed as `M:SS`, `H:MM:SS`, either with a decimal part after a point (`2:00.5`), or as a number of
  seconds (`90`, `90.5`)**. A comma is refused, as everywhere in the tool.
- **The subtitles keep the time of the film**: the line said at 2:10 of the film is written at `00:02:10`, not
  at `00:00:10`. The file can be laid over the whole film as it is.
- **The edges are approximate**: the engine works on windows of thirty seconds, so what it returns is cut
  to the range afterwards; a sentence that straddles an edge is cut there.
- **Nothing starts if the range cannot be used**: a bound that is not a time, or an `--end` that is not after
  `--start`, is a usage error that names the problem, writes nothing and runs neither `ffmpeg` nor the
  engine.

The audio of the whole film is still extracted: it is the transcription, much longer, that the range saves.

## Output

**Standard output** — nothing: the result is the file written.

| Level | What is added on the error output |
| :---- | :-------------------------------- |
| 1 | `<input>: N subtitles transcribed, language: xx -> <output>` |
| 2 | the step and the progress, one line for each percent: `extracting the audio…`, then `transcribing…`, then `transcribing: 25%, 0:12:30 of 0:50:00, about 38 min left, elapsed: 0:05:12` — the percent, the position reached in the audio, the time left, and the time spent; **the line is said again every ten seconds when nothing new has come**, so that a long wait is not taken for a freeze |
| 3 | and each new text the engine obtains, `last text: …` |

**The percent is the engine's position in the audio, not a count of its own windows**: the engine writes what it
has transcribed as it goes, and the command reads where that has got to, so that the figure moves from the first
minute. (The engine's own progress comes in steps of 5 % of the audio, after a window of thirty seconds is done:
alone, it can stay at 0 % for a quarter of an hour on a long film with a large model on a processor.) **The
time left is silent at first**: it is given once enough is done to extrapolate from, and it follows the speed of the
last minute, not the average of the whole run.

### In JSON

With [`--format json`](invocation.md#output-a-script-can-read) (the global option, written before the
subcommand), the object of the file carries the language used in `language`, `counts.subtitles` (the
subtitles written) and `counts.segments` (the engine's pieces they were made from), and `audio` when
`--keep-audio` kept the audio.

## Notices

Two notices are said at the first level, because they change what to expect:

- **Before the work, with a model for English alone**: `This model is for English only: whatever language the film
  speaks, the result will be English, or notes in brackets. Choose a multilingual model in the preferences.` A model
  whose file name ends in `.en` is one.
- **After the work, when the engine wrote mostly notes in brackets** — `[MUSIC PLAYING]`, `[SPEAKING FRENCH]` —,
  at least two and a third of its pieces: `N of the M pieces are only notes in brackets, such as [MUSIC PLAYING]: the
  language used (xx) is probably not the one spoken, and the engine may have translated the speech. Run it again with
  the language typed.` The subtitles are written all the same; the notice says what to suspect.

## Exit codes

Those of the tool: `0` if the file was written, `2` if it could not be — a tool or a model missing, a
language the model cannot take, a failed extraction or transcription —, `1` on a usage error, such as an
engine nobody registered or a range that cannot be used.

## Errors

| Cause | Message |
| :---- | :------ |
| no engine program found, or the configured one is not there | `the transcription engine was not found`, then the pointer to the guide |
| no model given, or it is not there | `no model was given, or the model file is not there`, then the pointer to the guide |
| `ffmpeg` not found for a video | `ffmpeg was not found, so the audio of a video cannot be extracted`, then the pointer |
| a track the video does not have | `the video has no audio track N`, then the tracks it has |
| a language the model cannot transcribe | `the model cannot transcribe the language xx` |
| no language, and a model that cannot detect one | `the model cannot detect the language: give one` |
| an engine nobody registered | `no transcription engine is called NAME` |
| a language that is not a code | `"TEXT" is not a language code: write two letters, such as fr, en, de or es` |
| a bound of the range that is not a time | `"TEXT" is not a time: write it as 2:00, 1:02:03 or 2:00.5, or in seconds` |
| `--end` not after `--start` | `the end of the range must come after its start` |
| any other failure | `the transcription failed`, with what the engine said |

The pointer to the guide is `See the manual page "Installing the transcription tools"
(install-transcription).`
