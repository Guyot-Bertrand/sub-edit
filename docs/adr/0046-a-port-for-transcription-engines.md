# 0046 — A port for transcription engines, with capabilities per engine and model

**Date:** 2026-10-23
**Status:** accepted

Decided at the scoping of [#768](https://github.com/Guyot-Bertrand/sub-edit/issues/768), part of phase 22; the decisions are D1 to D3 of [its spec](../specs/22-transcription.md). Written with the code it describes, in [#772](https://github.com/Guyot-Bertrand/sub-edit/issues/772).

## Context

Phase 22 transcribes the audio of a video with whisper.cpp, run as an executable. The maintainer wants to be able to replace it the day a better engine exists, without touching the command line, the window or the post-processing. A real run of the tool showed two things that shape the interface: its output is files and a stream of progress lines rather than a value, and **what it can do depends on the model** — an English-only model cannot detect a language or transcribe another one, a multilingual one does both.

## Decision

1. **A port in the core**, `core/transcription/`, that knows nothing of whisper, of a command line, of JSON or of Qt: `TranscriptionEngine` takes a `TranscriptionRequest` (an audio file, an optional language, an optional model) and a `TranscriptionObserver` (progress from 0 to 1, polled cancellation), and answers a `Transcription` — segments with text, start and end, optional timed words, optional confidence and speaker, and the language — or a `TranscriptionError` with a kind the caller acts on.
2. **Capabilities are declared for the pair, engine and model** (`capabilitiesFor(model)`): timed words, language detection, the languages it can transcribe (empty meaning any), speakers, whether it needs a model. The surfaces read the declaration instead of assuming.
3. **The port refuses before it works.** `transcribe` is not virtual: it asks for the capabilities, applies `checkRequest` — a missing model, a language the pair cannot transcribe, no language when the pair cannot detect one — and only then calls the adapter's `run`. A transcription in the wrong language therefore never comes back looking like a success, and an adapter cannot forget to refuse.
4. **A registry** (`EngineRegistry`) holds the engines of a build and finds them by name; the setting `transcription.engine` (#777) chooses among them.
5. **A contract suite**, `transcription_engine_contract.hpp` in the unit tests, that every adapter passes: well-formed segments, words inside their segment, honest capabilities, progress in [0, 1] ending at 1, the refusals made before any progress, cancellation, a missing audio file. A deterministic fake engine passes it and serves the tests of everything around the port.
6. **A second port for alignment** (`AlignmentEngine`: audio and a known text in, one optional start and end per line out) exists as a header, with its contract only. Its first adapter and command are phase 23's. It is a port of its own because aligning a known text is not finding what was said, and an engine can offer one without the other.

## Alternatives dismissed

- **One concrete class for whisper.cpp, extracted later if a second engine came.** Cheaper today, but the interface would then be shaped by whatever whisper does — files, progress lines, a splitting option for words — and the second engine would pay for it. The maintainer asked for the abstraction now.
- **Capabilities per engine.** Simpler, and wrong: it would let an English-only model be offered a French transcription, which the tool accepts and answers in English.
- **A single port for transcription and alignment.** An engine that can only transcribe would have to refuse half its interface, and phase 23 would shape the first port.
- **A virtual `transcribe` with the checks left to each adapter.** One forgotten check per adapter is one wrong-language transcription that looks like a success. The non-virtual wrapper costs one indirection.
- **A library API for whisper.cpp instead of the executable.** It removes the subprocess, and it gives up the user's choice of hardware variant, which phase 19 settled for the other external tools.

## Consequences

- Adding an engine is writing an adapter and passing the contract suite; nothing else changes.
- The post-processing (#776) and the two surfaces (#778, #779) are written against `Transcription` and `EngineCapabilities` and are tested with the fake engine, with no engine installed.
- The words timings are optional and approximate; phase 23 must not trust them more than the engine does.
- Speaker identification is a capability with no adapter yet; the post-processing must work without it.
- The port is synchronous: a run blocks the thread that calls it. The window runs it in the background, as the correction pages do.
