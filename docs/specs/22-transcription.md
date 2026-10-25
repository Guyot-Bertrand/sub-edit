# Phase 22 — Audio transcription

**Milestone:** `22 — Transcription audio`
**Scoped in** [#768](https://github.com/Guyot-Bertrand/sub-edit/issues/768), after the tooling of
[#767](https://github.com/Guyot-Bertrand/sub-edit/issues/767).

## What the roadmap promises, and what is already there

The roadmap says: produce subtitles, with their positions, from a video's audio; the expected engine is whisper.cpp,
run as an executable; the models are not shipped; the output is timed sentences or timed words, which phase 23 reuses;
the language of the audio is detected. It adds a rule for every external tool: **its absence breaks nothing** — the
feature disables itself with a clear message, or the user provides the data.

What the code already offers:

| Need | What exists |
| :--- | :---------- |
| Starting a program | `core/process/start_process.hpp`: `startProcess` (returns at once, output to a file) and `runAndCapture` (waits, standard output through a pipe, **standard error discarded**) |
| Finding a tool | `findExecutable` (`core/io/`), the search path being a parameter so that the missing-tool branch is testable |
| A tolerant reading of an external tool | `readDeclaredFrameRate`: nothing is an ordinary answer, never an error |
| A fake external program | `src/test/tools/fake_player.cpp`: knobs travel through argv, never the environment |
| Audio tracks | `AudioTrack` (identifier, language, title) in `core/video/video_player.hpp`, **filled by libmpv in the window only**; the command line cannot list the tracks of a container |
| Background work with progress | the correction progress page of the window (`QtConcurrent`, cancellable) |
| Shared settings | `OperationSettings` and the `config` commands, ADR 0045 |
| Words both surfaces say | `core/wording/`, one family per header |
| Test videos | `audio-1.mkv`, `audio-2.mkv` (`fra` "Original", `eng` "Commentary"), `sound-only.mkv`, verified by `video-fixtures.sh --check` |
| Machine-readable output | JSON Lines, versioned, ADR 0038 |

**Gaupol has no equivalent**: there is nothing to read in `reference/gaupol`, and no oracle. The phase is a new design,
and goes beyond iso-functionality.

## What a real run of whisper.cpp showed

Checked on 2026-10-22 with whisper.cpp 1.9.5-dev (commit `231638a`), built from source with the default options, the
`tiny` multilingual model, on a CPU. The findings are recorded in
[#768](https://github.com/Guyot-Bertrand/sub-edit/issues/768); the ones the design rests on:

- **The executable is `whisper-cli`**, and it writes its results to **files** (`-of`), not to standard output.
- **JSON** (`-oj`) carries `result.language` and, per segment, the text and the offsets in milliseconds. `-ojf` adds the
  tokens with start, end and probability. **Words come from a splitting option** (`-ml 1 -sow`), not from a dedicated
  field: the first word can have equal start and end, special tokens (`[_BEG_]`, `[_TT_…]`) must be filtered, word text
  keeps its leading space, and the timings are approximate.
- **Progress** (`-pp`) is a `progress = N%` line on **standard error**, live but coarse: one step per decoded 30-second
  window. Segment lines also arrive live on standard output.
- **Exit codes:** 0; 2 for a missing input; 3 for a missing model; 11 for an input that is not audio; 143 after a
  `SIGTERM`, which leaves no file behind.
- **VAD options exist** (`--vad`, `--vad-model`, thresholds), contrary to the README read at first. Untested.
- **No speaker information is to be counted on**: diarization needs stereo input (`-di`) or a tinydiarize model.
- **The model decides what the engine can do.** Whisper does not change model once the language is detected. A
  multilingual model detects and transcribes about a hundred languages; an English-only model (`*.en`) cannot. The JSON
  tells which one it is (`model.multilingual`).
- **Speed:** the `tiny` model on four threads, CPU only, transcribed 88 s of audio in about 4.3 s. Larger models were not
  measured.

Not tested, and to be settled with the real adapter: other languages than English, a larger model, VAD, GPU builds, the
behaviour on a start made of silence or music, and the quality of word timings with a model of real size.

## D1 — Two ports, the first one written now

**The transcription is behind a port in the core**, so that another engine can replace whisper.cpp without touching the
rest. Two ports exist as headers; only the first has an adapter:

1. **Transcription** — audio in, segments out. Phase 22.
2. **Alignment** — audio and a known text in, timings out. **Phase 23.** Only its contract is written here, because
   aligning a text one has is not the same problem as finding what was said, and an engine can offer one without the
   other. A single adapter can still offer both, as whisper.cpp will.

A port **knows nothing** of whisper, of a command line, of JSON, of Qt. It is a narrow interface in the style of
`video_player.hpp`: it speaks the project's types and nothing else. `core/transcription/` holds it.

An engine is an **adapter**. Adding one is writing an adapter and passing the contract suite (D3). The engine is chosen
by a setting through a registry; whisper.cpp is the default and the only one shipped.

## D2 — What a transcription is

**In:** an audio file (the normalised file of D5), an optional language, an optional model, a way to report progress and
a way to be cancelled.

**Out:** the segments of the engine, in order. A segment carries its text, its start and its end, and, **when the engine
gives them**, its timed words (each with start, end and, optionally, a confidence), a confidence for the segment and a
speaker. The detected language comes with the result. Nothing in a segment names an engine.

Timed words are what phase 23 reuses. They are optional because an engine may not provide them, and because the first
adapter obtains them from a splitting option (see the findings above).

## D3 — Capabilities, per engine and per model

Each engine **declares what it can do**, and the surfaces adapt to the declaration instead of assuming:

- timed words;
- detection of the language;
- the languages it can transcribe;
- speakers;
- whether it needs a model, and which.

**The declaration is made for a pair, engine and model, never for the engine alone.** An English-only whisper model has
no detection and one language; a multilingual one has both. The adapter reads this from the model (the JSON of the
engine says `multilingual`), not from a file name.

A language the chosen pair cannot transcribe, or a detection it cannot do, is **refused before any work starts**, with a
message that says why, rather than producing a transcription in the wrong language.

**The contract suite.** One suite that every adapter must pass, run against a deterministic **fake engine** and against
the whisper.cpp adapter. It lives with the port and is born with it (#772). The engine registry and the setting
`transcription.engine` choose among the adapters. **ADR 0046** records the port and the declaration of capabilities.

## D4 — A subprocess that streams and can be cancelled

`startProcess` returns at once and writes to a file; `runAndCapture` waits and discards standard error, where whisper
prints its progress. **Neither fits.** The phase adds a third primitive to `core/process/`: it runs a program, delivers
standard output and standard error **line by line while it runs**, and can be **cancelled** — terminated, then
collected, with no zombie left. It is tested with the fake program of #769, never with a real engine. It also serves the
extraction of D5, which is long on a film.

## D5 — Preparing the audio

An **audio source** is prepared before the engine sees it, outside the ports and shared by phases 22, 23 and 21:

- an audio file is used as it is, or converted to the normalised format;
- a video is probed for its audio tracks (language, title), then the chosen one is extracted by an external tool.

The tool is `ffmpeg`, and `ffprobe` lists the tracks, on the model of `readDeclaredFrameRate`. It follows the rule of the
roadmap: **if it is absent, the extraction disables itself with a clear message**, and an audio file given directly still
works. The command line gets its own track listing; the window can use the list the player already has.

The extraction command is written once. #771 checks it for real against the existing fixtures, as `video-fixtures.sh
--check` does for the rest.

## D6 — The whisper.cpp adapter

The first adapter runs `whisper-cli` as a subprocess — **an executable and not a library**, for the reason phase 19
gives: it leaves the choice of the hardware variant to the user. It:

- works in a temporary directory, since the tool writes files;
- builds the command line from the model, the language (`auto` by default) and the splitting options;
- reads the JSON, derives the timed words, filters the special tokens;
- reads the progress lines of standard error and maps the exit codes and messages to the port's errors;
- reads `model.multilingual` and declares the capabilities of the pair (D3);
- never selects or refuses the GPU: the user installs the build they want, and `-ng` is offered as a setting if needed.

**Nothing is shipped in the packages.** The executable, the model and `ffmpeg` are installed by the user. The roadmap's
"CPU-only build in the packages" point therefore falls: it was a question about what to ship, and nothing is shipped.

## D7 — Post-processing

An engine cuts where it likes. A step in the core, **independent of the engine**, turns its segments into subtitle
entries:

- it **merges** very close segments that belong to the same sentence — judged by the gap between them, the punctuation
  that closes the first, and the speaker when the engine gives one;
- it **cuts** those that exceed the duration and character limits, at a word boundary (the engine's timed words, or the letters in proportion). Cutting a subtitle into *lines* stays the job of the existing line-breaking task of phase 12, which needs the pattern engine and which the user already runs on any subtitle;
- its thresholds are settings with defaults. The first defaults are starting points, **tuned on the real output of #770**
  and said so in the pull request of #776.

It works with no speaker, and with no timed words: it then falls back on segment boundaries.

**The raw output is a feature.** The command line can write the engine's segments with no post-processing, one segment
per entry, to judge what the step does and to feed other tools.

## D8 — Settings and wording

The transcription's operation settings join the catalogue of ADR 0045 and so appear in `config list`, `show`, `set` and
`unset` on both surfaces: `transcription.engine`, `transcription.executable`, `transcription.model`,
`transcription.language`, and the post-processing thresholds. The priority is the usual one: an option on the command,
the file, the core default. **Paths are settings, never constants.**

What both surfaces say — progress, the detected language, a missing tool or model with the pointer to the installation
page, the refusal of an unsupported language — lives in `core/wording/transcription`, written once.

**The cost of translation is paid in each slice.** The `translations` step refuses an untranslated, fuzzy or stale
message in any of the twenty languages, so every pull request that adds a message runs `make po` and translates it in the
same diff. Fewer, shorter messages cost less; the wording family is therefore written first (#777) and the surfaces reuse
it.

## D9 — The command line

`subedit-cli transcribe INPUT`, where `INPUT` is an audio file or a video:

- writes a subtitle file: `--output`, `--format`, as the other commands do;
- `--audio-track` chooses the track of a video;
- the language is detected unless `--language` forces it, and **the language used is reported**;
- an option writes the extracted audio to a file;
- an option writes the output with no post-processing;
- `--format json` writes one record (ADR 0038), with the language, the counts and the files written;
- a missing executable, model or extraction tool is reported with a pointer to the installation page, with a non-zero
  exit code and nothing written; a language the model cannot transcribe is refused before any work starts;
- the batch and exit-code behaviour follows the other commands.

The name `align` is already taken by the command that snaps a file to a frame grid. Phase 23 will have to name its own
command with that in mind (open point 1).

## D10 — The window

A command transcribes the audio of **the project's video**. The window never takes an audio file.

- A dialog offers the audio tracks of the video (language and title, from the player), the language and the model. The
  declared language of the chosen track is the default before the detection.
- The work runs **in the background, with a progress indicator and a way to cancel**, as the correction pages do. A
  cancelled or failed run leaves the project unchanged.
- The result **replaces the subtitle section of the open project**. When that section is not empty, the window **asks
  for confirmation first**; when it is empty, it does not. The replacement is a **single undoable operation**.
- A missing tool or model is explained in the window, with the pointer to the installation page, and nothing else in the
  window is affected.
- The shortcuts table, the screenshots and the manual section exist in every language of `src/po/LINGUAS`.

## D11 — Installation is the user's, and the manual says how

The user installs the `whisper-cli` executable, a model and `ffmpeg`. **subedit downloads nothing**: it takes a path.
A dedicated manual page tells where to find each, with links to the official documentation; **the URLs live only in the
documentation**, never in the program, so that a broken link is a documentation fix. The error messages and the help of
the command point to that page.

The page includes the table of **which model for which language**: multilingual or English-only, and size, since the
small models are noticeably worse outside English. A download command can be an issue of its own later.

## How the phase proves itself

- every requirement below is cited by a test;
- **the whole chain runs on a deterministic fake engine and a small versioned audio file**, and the same chain is run
  against the fixtures of `src/test/data/videos/`;
- **the whisper.cpp adapter is tested on versioned samples of the tool's real output** (#770), and on a fake program for
  the failures (#769), never by launching the tool inside the gate;
- **a real-engine test exists outside the gate**, run by hand and documented; it never reports itself passed when it is
  skipped;
- the extraction command is checked for real against the fixtures (#771);
- the post-processing is judged on versioned fixtures compared with the expected-file comparator of #543;
- no file of the private corpus is read, and no figure is attributed to one.

## What the phase does not deliver

- Whisper's own translation. Translating is phase 11's job.
- Live transcription, or transcription during playback. The file is processed whole.
- A speaker-identification adapter. The port carries the capability; no adapter provides it.
- Training or fine-tuning a model, or managing the GPU: the user installs the build they want.
- Aligning an existing text: phase 23. Only the contract of its port is written.
- Styles and positions: phase 18.
- Downloading models.

## Gaps with Gaupol

- Gaupol has no transcription. It reaches video only through its player, with no audio processing of its own.
- The abstraction over an engine, the post-processing and the tolerated absence of every external tool are ours.

## Requirements

**Twelve**, registered `planned` in `docs/exigences.md` with this spec; each issue re-reads its own before its code and
the state moves to `implemented` when a test cites it. Four more were added by #792, once the first real runs showed
what was missing: `CLI-TRANSCRIBE-09`, `CLI-TRANSCRIBE-10`, `GUI-TRANSCRIBE-05` and `GUI-TRANSCRIBE-06`.

| Identifier | What it promises |
| :--------- | :--------------- |
| `CLI-TRANSCRIBE-01` | `transcribe` writes a subtitle file, in the format of `--output`, from the audio of a video or of an audio file |
| `CLI-TRANSCRIBE-02` | `--audio-track` chooses the audio track of a video, and a video with no such track is refused with the tracks it has |
| `CLI-TRANSCRIBE-03` | the language is detected unless `--language` forces it, and the language used is reported |
| `CLI-TRANSCRIBE-04` | an option writes the extracted audio to the file it names |
| `CLI-TRANSCRIBE-05` | `--raw` writes the segments of the engine with no post-processing |
| `CLI-TRANSCRIBE-06` | a missing executable, model or extraction tool is reported with a pointer to the installation page, with a non-zero exit code and nothing written |
| `CLI-TRANSCRIBE-07` | a language the chosen model cannot transcribe is refused before any work starts |
| `CLI-TRANSCRIBE-08` | `--format json` writes one record with the language used, the counts and the files written |
| `GUI-TRANSCRIBE-01` | `Transcribe Audio…` opens a dialog offering the audio tracks of the project's video, a language and a model |
| `GUI-TRANSCRIBE-02` | the transcription runs in the background with a progress indicator and can be cancelled, a cancelled or failed run leaving the project unchanged |
| `GUI-TRANSCRIBE-03` | the transcription replaces the subtitles of the project only once the person has applied it from the preview, as one undoable operation |
| `GUI-TRANSCRIBE-04` | without the executable, the model or the extraction tool the command explains what is missing and where to read how to install it, and the rest of the window is unaffected |
| `CLI-TRANSCRIBE-09` | `--start` and `--end` restrict the transcription to a part of the film, the subtitles keep the time of the film, and a bound that is no time or a range whose end comes first is refused before any work |
| `CLI-TRANSCRIBE-10` | while it runs the command says the step, the percent, the position in the film and the time left, and at the third level the text last obtained |
| `CLI-TRANSCRIBE-11` | `--list-languages` lists the languages the engine offers for the model, one per line with its name in the language of the interface, and transcribes nothing |
| `CLI-TRANSCRIBE-12` | `--suppress-notes` and `--keep-notes` decide whether the engine writes sounds as notes, over the setting; both together are refused, and an engine that cannot be told refuses the first by name |
| `GUI-TRANSCRIBE-05` | the progress box says the step, the position in the film, the time left and the last text obtained, and is busy rather than at 0 % while nothing says how far it is |
| `GUI-TRANSCRIBE-06` | the dialog takes a range of the film and transcribes only that part, the result replacing the subtitles of the project like any transcription, after the same question |
| `GUI-TRANSCRIBE-07` | the transcription is shown in a preview before it replaces the subtitles of the project — the engine's pieces on the left, the final subtitles on the right, each with its own times, a selected subtitle highlighting its sources — and a cancelled or failed run offers what it had obtained, to apply or to ignore |

## Split

**Eleven slices.** The tooling issues of the initialisation come first, and nothing waits for anything but what its column
says. All are in milestone 22, written in English.

| Slice | Issue | What it delivers | Depends on | Size |
| :---- | :---- | :--------------- | :--------- | :--- |
| 1 — tooling | [#769](https://github.com/Guyot-Bertrand/sub-edit/issues/769) | the fake transcriber program | — | S |
| | [#770](https://github.com/Guyot-Bertrand/sub-edit/issues/770) | real whisper.cpp output as versioned samples, and the by-hand test | — | M |
| | [#771](https://github.com/Guyot-Bertrand/sub-edit/issues/771) | the extraction check against the video fixtures | — | S |
| 2 — core | [#772](https://github.com/Guyot-Bertrand/sub-edit/issues/772) | the port, the types, the registry, the fake engine, the contract suite; ADR 0046 | — | L |
| | [#773](https://github.com/Guyot-Bertrand/sub-edit/issues/773) | the streaming subprocess with cancellation | #769 | M |
| | [#774](https://github.com/Guyot-Bertrand/sub-edit/issues/774) | audio tracks of a container and extraction | #773, #769, #771 | M |
| | [#775](https://github.com/Guyot-Bertrand/sub-edit/issues/775) | the whisper.cpp adapter | #772, #773, #769, #770 | L |
| | [#776](https://github.com/Guyot-Bertrand/sub-edit/issues/776) | the post-processing | #772 | M |
| | [#777](https://github.com/Guyot-Bertrand/sub-edit/issues/777) | the settings and the wording | #772 | M |
| 3 — surfaces | [#778](https://github.com/Guyot-Bertrand/sub-edit/issues/778) | the `transcribe` command | #774–#777 | L |
| | [#779](https://github.com/Guyot-Bertrand/sub-edit/issues/779) | transcription in the window | #774–#777 | L |
| 4 — documentation | [#780](https://github.com/Guyot-Bertrand/sub-edit/issues/780) | the installation guide | #775 | M |
| 5 — closure | [#781](https://github.com/Guyot-Bertrand/sub-edit/issues/781) | review of the phase, then the version the maintainer decides | all | M |

## Delivered by #777 to #780

One pull request carried the four: the settings (`transcription.*`, seven keys of the shared catalogue), the
`core/wording/transcription` family, the `transcribe` command, `Transcribe Audio…` in the `Video` menu, and the
installation guide. The pipeline both surfaces run is `transcribeMedia` (`core/transcription/`), so they cannot
differ; the window replaces the project's subtitles with `replaceAllSubtitles`, one entry of the history
(`CommandKind::Transcribe`), after a confirmation that is asked **before** the work (replaced by the preview, see below). The command's subtitle format
is chosen with `--to`, since `--format` is the global option that selects JSON output.

## Delivered by #792 to #794

Run on real films, the first version showed what the spec had not foreseen; three issues followed.

**#792 — progress, time left, a range.** whisper.cpp reports its own progress in steps of 5 % of the audio and only after a
window of thirty seconds, so a long film on a large model stayed at 0 % for a long while. The adapter reads the pieces the
engine prints as it decodes them, takes the length of the audio from its banner, and folds both into one fraction that only
goes up (`TranscriptionProgress`, `TranscriptionObserver::advance`). `RemainingTime` works the time left out of the speed
since the start, up to the last minute, and is silent until it can. `--start` and `--end` restrict the work to a part of the
film; the engine's whole windows are cut to the range afterwards. A model for English alone is said so before the work.

**#793 — the window.** The box is shown at once, the time fields ask for `HH:MM:SS`, a language is read as two letters or as
the three-letter code of a file.

**#794 — options, languages, preview.**

- **What an engine can do is a capability.** `EngineCapabilities` gains `suppressNotes` and `sampleRate`, and `languages` is now
  the closed set the engine offers for the model (about a hundred for a multilingual whisper.cpp model, English alone for the
  other) — empty still means any, for an engine that cannot list. The surfaces adapt: the window greys what the engine cannot
  do, the command line refuses by name an option the engine cannot honour. **What the engine can do with the model is asked
  before any extraction**, so a request it cannot take — a language, a missing model — is refused before the long work.
- **Sounds as notes** are an intent, `transcription.suppress-notes` (on by default), with `--suppress-notes` and
  `--keep-notes`, a checkbox in the dialog and in the preferences; the adapter turns it into `-sns`. Forbidden the symbols
  notes are made of, whisper writes `...` for a passage with no speech: **the post-processing drops the pieces with no letter
  and no digit**, and says where each subtitle comes from (`assemble` returns an `Assembly`: the subtitles and, for each, the
  indices of its source pieces). `--raw` keeps the engine's pieces as they came. A transcript with many of them is not suspect:
  a film has music and effects almost throughout.
- **The language** is chosen in the engine's list, named by ICU in the language of the window (`languageDisplayName`;
  `interfaceLanguageName` is the name of the catalogue installed), with a first entry that detects. `--list-languages` prints
  the same list, `code<TAB>name`, for the engine and model asked. The front-end speaks in neutral values (an ISO 639-1 code);
  the adapter turns them into the engine's.
- **The sampling rate** is the engine's (16 kHz for whisper.cpp); the extraction asks for it. The name of what `--keep-audio`
  writes takes a `{rate}` wildcard, filled in from the real value. The number of channels, and the speaker options of the
  engine, are left for later, on purpose.
- **The preview.** The transcription no longer replaces the subtitles of the project on its own: a dialog shows the pieces of
  the engine on the left and the final subtitles on the right, each with its own times, and the person applies or ignores
  (and loses) them. **The question that used to be asked before the work is gone**: "Apply" is the confirmation, and the
  replacement stays one undoable operation. A subtitle selected on the right highlights its sources on the left, which scrolls
  to them; one at a time, no editing. **A cancelled or failed run offers what it had obtained** (`TranscriptionError::partial`,
  carried by `MediaTranscriptionError`), titled as partial, so that a long run stopped late does not lose its work unasked.
  The notice of #793 about a transcript "mostly notes in brackets" is removed with it. On the command line, an interrupted run
  writes nothing, as before.

## Open points

1. **The name of phase 23's command.** `align` already snaps a file to a frame grid. The scoping of phase 23 names the new
   command with that in mind.
2. **VAD.** whisper.cpp offers voice-activity detection, which could cut long silences before the engine. Untested; the
   adapter (#775) decides whether it is worth exposing.
3. **Other engines.** sherpa-onnx is the only serious C++ alternative surveyed; Phonon-2 is to be reassessed if its
   languages and a command-line entry are documented. Neither is part of the phase: the port exists so that adding one is
   an adapter.
4. **Downloading models.** Left out on purpose (D11). A download command, with a source and an integrity check, can be a
   later issue.
5. **Timings with a real model.** Word timings were seen with `tiny` only. The thresholds of D7 are tuned on a larger
   model's output before the defaults are fixed.
6. **Other languages than English.** The run covered one English sample; French and a wrong detection are to be tried
   with the real adapter.
