# `transcribe`

```
subedit-cli transcribe --output FILE
                       [--to FORMAT]
                       [--audio-track N]
                       [--language CODE]
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
  --language CODE             Language of the speech: fr, en; detected by default
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
| `--language` | no | two lower-case letters, an ISO 639-1 code: `en`, `fr` | the setting `transcription.language`; empty: detected |
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

## Output

**Standard output** — nothing: the result is the file written.

| Level | What is added on the error output |
| :---- | :-------------------------------- |
| 1 | `<input>: N subtitles transcribed, language: xx -> <output>` |
| 2 | the progress of the engine, `transcribing: 25%`, as it reports it — in steps of about thirty seconds of audio |

### In JSON

With [`--format json`](invocation.md#output-a-script-can-read) (the global option, written before the
subcommand), the object of the file carries the language used in `language`, `counts.subtitles` (the
subtitles written) and `counts.segments` (the engine's pieces they were made from), and `audio` when
`--keep-audio` kept the audio.

## Exit codes

Those of the tool: `0` if the file was written, `2` if it could not be — a tool or a model missing, a
language the model cannot take, a failed extraction or transcription —, `1` on a usage error, such as an
engine nobody registered.

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
| any other failure | `the transcription failed`, with what the engine said |

The pointer to the guide is `See the manual page "Installing the transcription tools"
(install-transcription).`
