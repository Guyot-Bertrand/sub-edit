# Installing the transcription tools

Transcribing audio into subtitles — [`transcribe`](transcribe.md) on the command line,
[`Transcribe Audio…`](../subedit-gui/video.md#transcribing-the-audio) in the window — needs three
things that **subedit does not ship and does not download**: an engine, a model for that engine, and
`ffmpeg`. You install them, and you tell subedit where the model is.

**Nothing breaks without them.** The rest of the program works as before; only the transcription
disables itself, with a message that points here.

## The engine: whisper.cpp

[whisper.cpp](https://github.com/ggml-org/whisper.cpp) is the engine subedit runs. It is a program,
`whisper-cli`, that runs on an ordinary processor — a graphics card only makes it faster. Build it
as its own documentation says (a C++ compiler and CMake are enough):

```bash
git clone https://github.com/ggml-org/whisper.cpp
cd whisper.cpp
cmake -B build -DBUILD_SHARED_LIBS=OFF
cmake --build build -j --config Release
```

The program is then `build/bin/whisper-cli`. Built with `-DBUILD_SHARED_LIBS=OFF`, as above, it is a
single file that needs no library beside it, so you can copy it anywhere. Put it where your `PATH` finds it, or
**give its path** to subedit with the setting `transcription.executable` — in the window, the
[preferences](../subedit-gui/preferences.md#transcription) have a field for it. A build for a graphics card adds
one option to the first `cmake` line: `-DGGML_CUDA=1` for an NVIDIA card, `-DGGML_VULKAN=1` for most others;
subedit runs whichever you installed and chooses nothing for you.

To keep the program and nothing else, **install** it and delete the clone:

```bash
cmake --install build --prefix ~/.local
```

`--prefix` says where. `~/.local` needs no administrator rights, and its `bin` directory must be on your `PATH`;
without the option the default is `/usr/local`, which asks for `sudo`. `whisper-cli` lands in the `bin` directory
of that prefix, beside tools subedit does not use; once it is there, the clone and its `build` directory can go.

## A model

The engine reads a **model**, a file of several hundred megabytes to a few gigabytes that you
download once. The official models are listed at
[huggingface.co/ggerganov/whisper.cpp](https://huggingface.co/ggerganov/whisper.cpp), and the
repository of the engine has a script that fetches one:

```bash
sh ./models/download-ggml-model.sh small
```

Then give its path with `--model` on the command line, or with the setting `transcription.model`,
or in the [preferences](../subedit-gui/preferences.md#transcription) of the window, which remember it.

### A voice-activity model, to begin where the speech does

Optionally, a **voice-activity model** (Silero) lets the engine skip the silences before it decodes. Without one, whisper starts its first
subtitle at 0 whatever comes first in the film, music included, and rounds its times to whole windows; with one, the first subtitle begins where
the speech does and the others follow it. The repository of the engine has a script that fetches one:

```bash
sh ./models/download-vad-model.sh silero-v6.2.0
```

Give its path with `--vad-model`, with the setting `transcription.vad-model`, or in the
[preferences](../subedit-gui/preferences.md#transcription) of the window.

### Which model for which language

Whisper does not change model once it has found the language: **the model you give decides what it
can do.**

| Model | Languages | Detects the language | Size |
| :---- | :-------- | :------------------- | :--- |
| `tiny`, `base`, `small`, `medium`, `large-v3`, `large-v3-turbo` | about a hundred | yes | from about 75 MB to about 3 GB |
| `tiny.en`, `base.en`, `small.en`, `medium.en` | English only | no | as the same size above |

- **For anything but English, take a multilingual model** — one whose name has no `.en`. An English-only
  model refuses another language: the command says so before doing any work.
- **The smallest models are noticeably worse outside English.** For French, German, Spanish and the
  like, `small` is a minimum, and `medium` or `large-v3-turbo` give much better results at the price
  of more memory and more time.
- **Language detection is only as good as the model.** A small model may be wrong on a beginning made of
  music or silence; say the language with `--language` when you know it.

## ffmpeg

[`ffmpeg`](https://ffmpeg.org/download.html) extracts the audio track of a video, and `ffprobe`, which comes
with it, lists the tracks. Install it with your distribution's package manager. Without it, an audio
file the engine reads as it is (`wav`, `mp3`, `flac`, `ogg`) can still be transcribed, but a video
cannot.

## Checking it works

```bash
subedit-cli transcribe --model /path/to/ggml-small.bin --output film.srt film.mkv
```

If something is missing, the message says what, and points to this page.
