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

The steps run **in this order, each over the whole set before the next one starts** (#795) — interleaving them piece by piece
is what produced a two-word subtitle left after a cut:

1. **drop the notes**: the pieces with no letter and no digit (`...`, `…`, `♪`);
2. **merge** very close segments that belong to the same sentence — judged by the gap between them, the punctuation that closes the
   first, and the speaker when the engine gives one — as long as the result respects the limits;
3. **cut** those that exceed the duration and character limits. **Punctuation first**: the candidates are the ends of words, ranked
   by the mark they end with — an end of sentence (`. ! ? …`, the ideographic full stop and its full-width forms; a closing quote or
   bracket after it does not hide it) before a clause (`, ; :` and their ideographic forms) before nothing at all, which is the
   middle of a sentence and is only taken when no mark is there. Among candidates of the same rank, **the one that makes the two
   halves the most alike in characters**; the halves are then cut the same way until each fits. A cut that would leave a half **too
   small** (fewer than 12 characters, or less than 700 ms — constants of the step, not settings) is only taken when every other cut
   is as bad, so that a one-word subtitle is not left behind. Cutting a subtitle into *lines* stays the job of the existing
   line-breaking task of phase 12, which needs the pattern engine and which the user already runs on any subtitle;
- **times are the engine's, never the words'**: a subtitle that begins a piece keeps the start of the piece, and one that ends it
  keeps its end; the timed words (the engine's, or the letters in proportion) only place the cuts *inside* a piece, where the
  following subtitle starts at the word it begins with. A piece taken whole, or several merged, keeps the times of its pieces;
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
what was missing: `CLI-TRANSCRIBE-09`, `CLI-TRANSCRIBE-10`, `GUI-TRANSCRIBE-05` and `GUI-TRANSCRIBE-06`. Six more came with
#794, #795 and #798: `CLI-TRANSCRIBE-11` to `CLI-TRANSCRIBE-13` and `GUI-TRANSCRIBE-07` to `GUI-TRANSCRIBE-09`. All twenty-two are
`implemented`.

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
| `GUI-TRANSCRIBE-03` | when the subtitle section is not empty the window asks for confirmation before the work, and the transcription replaces it only once the person has applied it from the preview, as one undoable operation |
| `GUI-TRANSCRIBE-04` | without the executable, the model or the extraction tool the command explains what is missing and where to read how to install it, and the rest of the window is unaffected |
| `CLI-TRANSCRIBE-09` | `--start` and `--end` restrict the transcription to a part of the film, the subtitles keep the time of the film, and a bound that is no time or a range whose end comes first is refused before any work |
| `CLI-TRANSCRIBE-10` | while it runs the command says the step, the percent, the position in the film and the time left, and at the third level the text last obtained |
| `CLI-TRANSCRIBE-11` | `--list-languages` lists the languages the engine offers for the model, one per line with its name in the language of the interface, and transcribes nothing |
| `CLI-TRANSCRIBE-12` | `--suppress-notes` and `--keep-notes` decide whether the engine writes sounds as notes, over the setting; both together are refused, and an engine that cannot be told refuses the first by name |
| `GUI-TRANSCRIBE-05` | the progress box says the step, the position in the film, the time left and the last text obtained, and is busy rather than at 0 % while nothing says how far it is |
| `GUI-TRANSCRIBE-06` | the dialog takes a range of the film and transcribes only that part, the result replacing the subtitles of the project like any transcription, after the same question |
| `GUI-TRANSCRIBE-07` | the transcription is shown in a preview before it replaces the subtitles of the project — the engine's pieces on the left, the final subtitles on the right, each with its own times, a selected subtitle highlighting its sources — and a cancelled or failed run offers what it had obtained, to apply or to ignore |
| `GUI-TRANSCRIBE-08` | the post-processing is tuned in the preview — the merge gap, the longest subtitle and the most characters — the right-hand side made again at once with the selection kept; the values hold for that transcript only, and one button hands them to the settings of the application |
| `CLI-TRANSCRIBE-13` | `--vad-model` (over the `transcription.vad-model` setting) gives the engine a voice-activity model that skips the silences, so that its segments follow the speech; an engine that cannot refuses the option by name and drops the setting without a word, and a file that is not there is refused before any work |
| `GUI-TRANSCRIBE-09` | the Preferences name the voice-activity model with a field and a Browse button, saved as `transcription.vad-model`, and the transcription uses it |

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
  (and loses) them. **The question asked before the work stays**, when the project holds subtitles: an hour of computing is not started
  by mistake. "Apply" is the second choice, made once the result is seen, and the replacement stays one undoable operation. A subtitle selected on the right highlights its sources on the left, which scrolls
  to them; one at a time, no editing. **A cancelled or failed run offers what it had obtained** (`TranscriptionError::partial`,
  carried by `MediaTranscriptionError`), titled as partial, so that a long run stopped late does not lose its work unasked.
  The notice of #793 about a transcript "mostly notes in brackets" is removed with it. On the command line, an interrupted run
  writes nothing, as before.

## Delivered by #795

- **The post-processing is reworked** as D7 now says: drop, merge, cut, each over the whole set; the cut on punctuation first,
  balanced, avoiding a half too small; the times of the pieces kept (the first subtitle of a piece no longer starts at its first
  word, which whisper's tokens date approximately). Tested on the two examples of the issue (a comma after "comes", a "the" alone).
- **The preview is the place to tune it** (`GUI-TRANSCRIBE-08`): the merge gap, the longest subtitle and the most characters are
  three fields above the tables, bounded as the settings are (`assembly_limits.hpp`, one definition); changing one makes the
  right-hand side again with `assemble`, a pure function of the pieces and the limits, and the selection follows the piece it was
  made from. **The values hold for that transcript only**; "Keep these values as settings" hands them to the application's settings
  (saved with the others) whether the result is then applied or ignored, and says so. Closing the window never does it.
- **The first subtitle that starts too early is whisper's, not ours** (point 4 of the issue). Measured on a real film, its first
  two minutes extracted at 16 kHz mono, `whisper-cli` (`small` model, `-sns -osrt`) writes its first segment from 0:00:00,000, the
  same start as the raw column of the preview: the reading of `offsets.from` is faithful, and whisper starts its first segment at 0
  after an intro. Voice-activity detection (`--vad`, open point 2) is the lead left; it needs a Silero model, which is not
  installed on the machine of the measure (only the test model of whisper.cpp), so no with/without comparison was made. No other
  setting is added: none was found.

## Delivered by #798

A voice-activity model, as an intent and a capability like the sound notes: `transcription.vad-model` (a path, empty for none),
`EngineCapabilities::voiceActivity`, `TranscriptionRequest::vadModel`, the `--vad-model` option and a field in the Preferences with
its Browse button. The whisper.cpp adapter turns it into `--vad --vad-model FILE`; a file that is not there is a missing model, said before
the tool starts. On the command line, **an option names a file and is refused by name when the engine cannot use it, or when the file is
missing; a setting is dropped without a word** for an engine that cannot, since a setting is written for all the engines. The model is the
user's to provide (`models/download-vad-model.sh` of whisper.cpp), as the speech model is; nothing is shipped.

**A defect found afterwards (#808).** With a voice-activity model, whisper.cpp writes the times of the *segments* on the film's time and
the times of the *tokens* — the timed words — **on the timeline with the silences removed** (a segment at 118 s whose tokens are at 67 s). The
post-processing cuts a piece inside at the words' times, so with a model set the cuts came out of order (34 subtitles of 91 with a small
character limit). The adapter now **puts the words of each segment back on the film's time**: shifted so that the first begins where
the segment does, and kept inside it; the words stay right relative to one another. Same film, same limits: no subtitle out of order.
A sample written by hand (`vad.words.json`, made-up words, the structure of the real output) is the one exception to "nothing here is
edited by hand" in `src/test/data/whisper/`, and says so.

Measured by hand on the first two minutes of a real film (`small` model): without it the first segment begins at 0:00:00,000 and times are
rounded to whole windows (9.000, 14.000); with it the first begins at 4.930 and the next ones at 9.270 and 14.430.

## Closure

**Phase 22 closes in 1.1.0, on 2026-10-26.** Twenty issues, #767 to #799, and nothing open once this one is merged.
What it delivers: the engine port and its capabilities, the whisper.cpp adapter, the audio preparation, the post-processing, the
`transcribe` command, `Transcribe Audio…` with its preview, the settings, and the installation guide.

**What the review of the whole phase found**, and fixed in the course of it rather than at the end: the first real run showed what
the spec had not foreseen (#792, #794); the post-processing cut in the middle of sentences and moved the first subtitle (#795);
two windows took seconds to open because the whole model was read for its first eight bytes (#799). The spec's two deviations are
written where they happened: D7 cuts by duration and characters, with no reuse of the line-breaking task of phase 12 (which is
for lines, not for subtitles), and the preview came to stand between the engine and the project.

**Measured on a real film** (a film of 41 minutes, its first two minutes, `small` model, `whisper-cli` and `ffmpeg` installed, VAD with
the Silero v6.2.0 model; a machine with four cores working at 380 %): the whole chain, from the video to the file, took 1 min 8 s for
120 s of film, and gave 42 subtitles, the first beginning at 4.93 s where the speech begins. Without the voice-activity model the first
begins at 0 and the times fall on whole windows. The post-processing alone, on three thousand pieces (a long film), takes about
20 ms (`assembling the subtitles of a full-length transcript`, in the bench), which is why the preview makes its right-hand side again at
every key typed.

**What is left, and is not a defect**: speakers (`-di`, `-tdrz`) and the number of channels of the extraction were left out on purpose
(#794) for lack of an asset to prove them; no model larger than `small` and no language but English was measured (open points 5 and 6);
nothing is shipped, and subedit downloads nothing (D11). Phase 23 reuses the engine and its timed words, which are the tokens of the
engine and therefore approximate.

## Open points

1. **The name of phase 23's command.** `align` already snaps a file to a frame grid. The scoping of phase 23 names the new
   command with that in mind.
2. ~~**VAD.**~~ **Settled by #798**: exposed as `transcription.vad-model`, measured on a real film (see *Closure*).
3. **Other engines.** sherpa-onnx is the only serious C++ alternative surveyed; Phonon-2 is to be reassessed if its
   languages and a command-line entry are documented. Neither is part of the phase: the port exists so that adding one is
   an adapter.
4. **Downloading models.** Left out on purpose (D11). A download command, with a source and an integrity check, can be a
   later issue.
5. **Timings with a real model.** Seen with `small` on a real film at the closure (see *Closure*); the defaults of D7 (400 ms,
   7000 ms, 84 characters) held and are kept. `medium` and `large` were not measured.
6. **Other languages than English.** The run covered one English sample; French and a wrong detection are to be tried
   with the real adapter.
