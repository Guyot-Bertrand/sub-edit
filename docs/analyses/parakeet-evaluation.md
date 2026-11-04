# Parakeet and Nemotron as transcription engines — an evaluation

**Date:** 2026-11-04
**Status:** analysis; **no phase is opened for Parakeet**. Revisit if the need shows up (see
[When to come back](#when-to-come-back)).

This page keeps the work done to answer one question: are NVIDIA's Parakeet and Nemotron speech models
candidates to sit beside whisper.cpp behind the transcription port of
[ADR 0046](../adr/0046-a-port-for-transcription-engines.md)? It records what was read, what was measured, how
to replay it, what was not tested, and why the decision is to wait.

## Conclusion

- **Parakeet TDT 0.6B v3 is technically a candidate.** It is 15 to 20 times faster than Whisper
  large-v3-turbo on a processor, and about 2 points of word error behind medium and large-v3-turbo on clean
  French and English speech.
- **It does not fit the port as it stands.** It is built for short segments and gives no sentence cutting of its
  own, so an adapter would have to pre-cut the audio with a voice-activity detector and transcribe the pieces. The
  maintainer does not want that machinery today.
- **Nemotron is not a candidate.** The streaming model targets real-time voice agents, where latency
  matters and a whole file is never available at once; the English one is English alone.
- **Decision:** do not integrate Parakeet now. The work of the phase that was about to open goes to named
  transcription configurations instead (see [What was decided next](#what-was-decided-next)).

## What the models are

Everything in this section comes from model cards and search results read on 2026-11-03; none of it was checked
against the vendor's own documents unless said so.

| | Parakeet TDT 0.6B v3 | Nemotron speech streaming (English) |
| :-- | :-- | :-- |
| Family | FastConformer encoder, token-and-duration transducer | cache-aware FastConformer encoder, RNN-T decoder |
| Size | 0.6 billion parameters | 0.6 billion parameters |
| Languages | 25 European languages, French and English among them | English only; a separate multilingual variant is listed by one provider for 40 language-locale pairs, **unconfirmed** |
| Made for | transcribing audio segments | streaming recognition with a latency chosen at run time (80 ms to 1.12 s) |
| Licence | CC-BY-4.0, attribution to NVIDIA required (stated by the ONNX re-exports; the original card was not read) | not read |
| How to run it without Python | sherpa-onnx, `nemo_transducer` model type, int8 files (encoder, decoder, joiner, tokens) | NeMo, a vendor container; no lightweight runtime found |

Whether Parakeet detects the language of the audio, and which of the other languages the project translates its manual
into (Catalan, Czech, German, Spanish, Finnish) it covers, **was not checked**.

## What was measured

### Method

- **Data:** the test split of the public FLEURS corpus (`google/fleurs`), French (`fr_fr`) and English (`en_us`):
  read sentences, one speaker each, clean. One recording per distinct sentence, shuffled with the seed 7, the first
  30 kept per language: 312.2 s of French and 306.7 s of English.
- **Engines:** Parakeet TDT 0.6B v3 int8 through the sherpa-onnx Python package (2.0.1), and whisper.cpp with
  the `small`, `medium` and `large-v3-turbo` multilingual models, each given the language.
- **Machine:** a 12-core processor, no graphics card, 8 threads for each engine, nothing else running.
- **Score:** the word error rate and the character error rate of `jiwer`, over the whole sample, after the same
  normalisation of the reference and of the hypothesis: Unicode NFC, lower case, every character that is neither a
  letter, a digit nor a space replaced by a space, spaces collapsed.
- **Speed:** wall-clock time divided by the duration of the audio. For Parakeet, the model is loaded once; for
  whisper.cpp, the executable is started once per file and **reloads its model each time**, which penalises it, most
  for the largest model.

### Results

| Engine | Word errors, French | Word errors, English | Time / audio, French | Time / audio, English |
| :----- | ------------------: | -------------------: | -------------------: | --------------------: |
| Parakeet TDT v3 int8 | 5.2 % | 6.2 % | 0.095 | 0.086 |
| Whisper small | 11.0 % | 5.3 % | 0.47 | 0.40 |
| Whisper medium | 3.3 % | 4.3 % | 1.51 | 1.27 |
| Whisper large-v3-turbo | 3.0 % | 3.2 % | 1.92 | 1.85 |

Character errors point the same way: in French, 1.7 % for Parakeet, 4.4 % for small, 1.2 % for medium and 0.8 % for
large-v3-turbo; in English, 2.7 %, 2.1 %, 1.8 % and 1.5 %.

Parakeet also returned token timestamps, non-decreasing, for all 60 sentences. **Their accuracy was not measured.**

### How far to trust it

- **30 sentences per language is little.** Differences under about a point are noise; the speed gap is not.
- **The speech is read, clean and short.** Films have music, noise, overlapping voices and files of two hours.
  Nothing here says how either engine behaves on them.
- **The sherpa-onnx executables were not run**, only the Python package. The command-line programs, their
  output and their handling of long audio are unverified.
- **The figures are an observation, not a guarantee.** If one ever has to become a guarantee, it needs a fixture
  in `src/test/data/`; FLEURS is public and its sentences could supply one.

## Why Parakeet does not fit the port as it stands

The port ([ADR 0046](../adr/0046-a-port-for-transcription-engines.md)) wants an engine that takes an audio file and
returns segments with a start, an end and optional timed words. whisper.cpp does that: it windows the audio and
cuts sentences itself. Parakeet takes a short utterance and returns its text with token timings. For a film an adapter
would have to:

1. cut the audio into speech segments with a voice-activity detector (the Silero model of whisper's `--vad`
   would do);
2. transcribe each piece, and offset its timings by the piece's start;
3. decide what a segment is, since the engine gives none: one piece per segment, or a cut on the punctuation
   Parakeet writes;
4. declare honest capabilities: timed words yes (from the tokens), language detection unknown, languages the 25.

That is a real adapter plus a pre-cutting stage of its own, and the quality of the result then depends on the
detector's cuts as much as on the model. The maintainer judged it more machinery than the gain in speed is worth
today.

## When to come back

- A user needs a transcription much faster than a large Whisper model gives on a processor, and accepts about 2
  points more word errors.
- The pre-cutting stage becomes useful for another reason (for instance if the speech detector of phase 23 is
  shared with the transcription).
- sherpa-onnx or NVIDIA ships a command-line program that cuts long audio itself and writes segments, which would
  remove the stage above. That program should be tried before anything is written.
- A model of the same family handles long audio natively.

Before any decision then, three things to measure that this page does not: accuracy of the timings against
whisper's, behaviour on a film's audio rather than read sentences, and the languages the project translates into.

## What was decided next

The same conversation settled the design of **named transcription configurations**, the work that replaces the
Parakeet phase. Recorded here so that it is not lost; its own scoping will be written with the phase.

- A configuration is named, and the user can add, edit and delete them in the preferences of the window; they are
  not fixed.
- It holds **the tool**: the engine, its program, the model, the voice-activity model.
- It does **not** hold the language, chosen at each transcription since it follows the film and not the tool, nor
  the post-processing (merging, cutting limits, notes), which stays common to all configurations.
- The transcription is run by naming a configuration, in the window and on the command line.
- The existing `transcription.*` settings become a first configuration, so that nothing is lost.
- The settings file stays flat ([ADR 0045](../adr/0045-one-settings-file-two-parts.md)),
  so the name has to travel in the keys.

## Replaying the measurement

Needs Python 3 with `sherpa-onnx`, `jiwer` and `soundfile`; `whisper-cli` and the three Whisper models; the
Parakeet archive `sherpa-onnx-nemo-parakeet-tdt-0.6b-v3-int8` of the sherpa-onnx `asr-models` release; and, for each
language, the FLEURS `test.tsv` and `audio/test.tar.gz` unpacked under `audio/<fr|en>/test/`.

```python
import csv, json, random, re, subprocess, sys, time, unicodedata, tempfile, os
import soundfile as sf
import jiwer
import sherpa_onnx

ROOT = os.path.dirname(os.path.abspath(__file__))
N = int(sys.argv[1]) if len(sys.argv) > 1 else 30
THREADS = 8
LANGS = {"fr": "fr_fr", "en": "en_us"}
WHISPER = {
    "whisper-small": "ggml-small.bin",
    "whisper-medium": "ggml-medium.bin",
    "whisper-large-v3-turbo": "ggml-large-v3-turbo.bin",
}  # give the path of each model


def norm(t):
    t = unicodedata.normalize("NFC", t).lower().replace("’", "'")
    t = re.sub(r"[^\w\s]", " ", t)
    return re.sub(r"\s+", " ", t).strip()


def sample(lang):
    rows = {}
    with open(f"{ROOT}/fleurs/{LANGS[lang]}/test.tsv", newline="", encoding="utf-8") as f:
        for r in csv.reader(f, delimiter="\t"):
            rows.setdefault(r[2], r)  # one recording per sentence
    keys = sorted(rows)
    random.Random(7).shuffle(keys)
    out = []
    for k in keys:
        r = rows[k]
        p = f"{ROOT}/audio/{lang}/test/{r[1]}"
        if os.path.exists(p):
            out.append((p, norm(r[2])))
        if len(out) == N:
            break
    return out


def whisper(model, path, lang):
    with tempfile.TemporaryDirectory() as d:
        t = time.time()
        subprocess.run(["whisper-cli", "-m", model, "-f", path, "-l", lang, "-t", str(THREADS),
                        "-nt", "-np", "-otxt", "-of", f"{d}/o"], check=True,
                       stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        return open(f"{d}/o.txt", encoding="utf-8").read(), time.time() - t


m = f"{ROOT}/models/sherpa-onnx-nemo-parakeet-tdt-0.6b-v3-int8"
rec = sherpa_onnx.OfflineRecognizer.from_transducer(
    encoder=f"{m}/encoder.int8.onnx", decoder=f"{m}/decoder.int8.onnx",
    joiner=f"{m}/joiner.int8.onnx", tokens=f"{m}/tokens.txt",
    num_threads=THREADS, model_type="nemo_transducer")
results = {}
for lang in LANGS:
    data = sample(lang)
    audio_s = sum(sf.info(p).duration for p, _ in data)
    refs = [r for _, r in data]
    hyps, t0, ts_ok = [], time.time(), 0
    for p, _ in data:
        x, sr = sf.read(p, dtype="float32")
        s = rec.create_stream()
        s.accept_waveform(sr, x)
        rec.decode_stream(s)
        hyps.append(norm(s.result.text))
        ts = list(s.result.timestamps)
        ts_ok += bool(ts) and ts == sorted(ts)
    dt = time.time() - t0
    results[f"{lang}/parakeet"] = dict(wer=jiwer.wer(refs, hyps), cer=jiwer.cer(refs, hyps),
                                       rtf=dt / audio_s, token_timestamps_ok=f"{ts_ok}/{len(data)}")
    for name, model in WHISPER.items():
        hyps, wall = [], 0.0
        for p, _ in data:
            h, dt = whisper(model, p, lang)
            hyps.append(norm(h))
            wall += dt
        results[f"{lang}/{name}"] = dict(wer=jiwer.wer(refs, hyps), cer=jiwer.cer(refs, hyps),
                                         rtf=wall / audio_s)
print(json.dumps(results, indent=1))
```

## Sources read

- [sherpa-onnx, NeMo transducer models](https://k2-fsa.github.io/sherpa/onnx/pretrained_models/offline-transducer/nemo-transducer-models.html)
- [Parakeet TDT v3, ONNX int8](https://huggingface.co/soniqo/Parakeet-TDT-v3-ONNX)
- [Parakeet TDT 0.6B v3, split ONNX](https://huggingface.co/talatapp/parakeet-tdt-0.6b-v3-onnx-split)
- [Nemotron speech streaming, English, model card](https://huggingface.co/nvidia/nemotron-speech-streaming-en-0.6b)
- [Nemotron 3 ASR streaming 0.6B, Together AI listing](https://www.together.ai/models/nemotron-3-asr-streaming-0-6b)
- [FLEURS corpus](https://huggingface.co/datasets/google/fleurs)
