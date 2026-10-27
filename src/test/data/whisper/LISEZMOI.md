# whisper.cpp samples

What `whisper-cli` actually wrote, captured once from a real run. The adapter of phase 22 is tested on these
files, so that it reads what the tool writes and not what its documentation suggests. **Nothing here is edited by
hand**: [`src/scripts/whisper-fixtures.sh`](../../../scripts/whisper-fixtures.sh) holds the commands that produce
them.

```console
$ ./src/scripts/whisper-fixtures.sh --check --whisper PATH --model PATH --sample PATH     # replay and compare
$ ./src/scripts/whisper-fixtures.sh --capture --whisper PATH --model PATH --sample PATH   # rewrite the samples
```

Neither runs in the gate: the tool and a model are not shipped and weigh too much to carry.

## Provenance

| What | Value |
| :--- | :---- |
| Tool | whisper.cpp `1.9.5-dev`, commit `231638a`, built from source with the default CMake options, CPU only |
| Model | `ggml-tiny.bin`, multilingual (`model.multilingual` is `true`) |
| Input | `samples/jfk.wav` of whisper.cpp: eleven seconds of speech, a public-domain recording, **not versioned here** |
| Long input | the same sample eight times over, made by `ffmpeg` (88 s): one progress line is printed per decoded 30-second window |

Two things the tool writes belong to the machine and are replaced before anything is written: the model path
(`params.model` becomes `ggml-tiny.bin`) and the line listing the CPU features (`systeminfo`). The JSON files are
otherwise the tool's, re-indented with tabs as it writes them.

## Files

| File | Command | What it shows |
| :--- | :------ | :------------ |
| `jfk.json` | `-oj -l auto` | one segment, `result.language`, offsets in milliseconds |
| `jfk.full.json` | `-oj -ojf -l auto` | the tokens of each segment with start, end and probability, and the special tokens (`[_BEG_]`, `[_TT_…]`) an adapter filters |
| `jfk.words.json` | `-oj -ml 1 -sow -l auto` | one "segment" per word: the way timed words are obtained; the first entry is empty, the first word has equal start and end, word text keeps its leading space |
| `segments.stdout.txt` | `-pp -l auto` on the long input | the segment lines the tool writes to standard output as it goes |
| `progress.stderr.txt` | the same run | the detected language and the progress lines of standard error; the probability is dropped |
| `missing-model.txt`, `missing-input.txt`, `not-audio.txt` | a missing model, a missing input, a file that is not audio | the exit code (3, 2, 11) and the `error:` lines |

## What these samples do not show

Another language than English, a model larger than `tiny` (whose word timings are approximate), voice-activity
detection, a GPU build, a start made of silence or music. Each needs its own capture when the adapter relies on it.

## The one sample written by hand

`vad.words.json` is **not a capture**: it was written to reproduce a structure seen once on a real run with a
voice-activity model (issue #808), with made-up words so that no film's dialogue is carried here. The segments are on the
film's time (4.93 s, 9.27 s, 20 s) and **the tokens on the timeline with the silences removed** (0 s, 4.12 s, 8.72 s), which is
what `whisper-cli --vad` writes. It is read by the adapter's tests only, and is not replayed by `whisper-fixtures.sh`.
