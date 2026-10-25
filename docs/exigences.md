# Requirements registry

What `subedit` promises, line by line, with the state of what proves it.

A requirement is **what the binary shows**: standard output, standard error,
exit code, file produced. The internal guarantees of the core are not
requirements — their unit tests and line coverage play that role. The decision,
and the seven options set aside, are in
[ADR 0014](adr/0014-registre-d-exigences.md).

This registry does not replace the [manual](manual/), which remains the
exhaustive description of what the user sees. It says something else: **which
promise is demonstrated by a test**, which no coverage rate can tell.

## Identifiers

`SURFACE-SUBJECT-NN`, in capitals, two-digit number — `CLI-VERSION-01`. `CLI`
for the command line, `GUI` for the window.

This shape is what makes it possible to recognise an identifier among a test's
tags without maintaining a list: an ordinary tag of the project — `[e2e]`,
`[format]`, `[framerate]` — has neither capitals nor a final numeric segment.

**An identifier is never reused.** A requirement that disappears leaves its
line, struck through, with the reason in the state column. An identifier removed
without a trace leaves a gap nobody knows how to interpret six months later.

## States

| State | What the line asserts | What the check requires |
| :---- | :-------------------- | :---------------------- |
| `planned` | the requirement is decided, the code does not honour it yet | no test cites it |
| `implemented` | the code honours it | at least one test cites it |
| `abandoned` | it will not be honoured; the reason follows in the column | no test cites it |
| `replaced` | another requirement takes its place; its identifier follows | no test cites it |

A test cites a requirement through a **Catch2 tag**:

```cpp
TEST_CASE("invoking with no argument writes the version", "[e2e][CLI-VERSION-01]")
```

`src/scripts/check-requirements.sh` compares these assertions with the tags the
test binary declares, and `make check-local` runs it. An `implemented`
requirement without a test does not pass the gate; nor does a tag that names no
requirement.

**The registry is fed at the start of an issue**, before the code. A
requirement written after the fact describes what was done; written before, it
decides what will be done.

## Registry

**Nothing follows this table.** `src/scripts/verify-gates.sh` proves that the
gate closes by appending a line at the end of the file; text after the table
would make it inject its defect in the wrong place, and the proof would no
longer prove anything.

| ID | Requirement | Phase | State |
| :- | :---------- | :---- | :---- |
| ~~`CLI-VERSION-01`~~ | invoking with no argument writes `subedit <version>` on standard output | 3 | replaced by `CLI-USAGE-01` |
| ~~`CLI-VERSION-02`~~ | nothing is written on standard error | 3 | replaced by `CLI-VERSION-05` |
| ~~`CLI-VERSION-03`~~ | any argument is ignored, exit code 0 | 3 | replaced by `CLI-USAGE-02` |
| `CLI-VERSION-04` | `--version` writes `subedit <version>` on standard output, alone on the first line, code 0 | 3 | implemented |
| `CLI-VERSION-05` | a successful invocation writes nothing on standard error | 3 | implemented |
| `CLI-USAGE-01` | with no argument, the help is written on standard output, code 0 | 3 | implemented |
| `CLI-USAGE-02` | an unknown option or an invalid value gives code 1 | 3 | implemented |
| `CLI-USAGE-03` | a usage error is detected before any processing | 3 | implemented |
| `CLI-USAGE-04` | `--quiet` and `-v` together give a usage error | 3 | implemented |
| `CLI-OUTPUT-01` | only the result goes to standard output, the narration to standard error | 3 | implemented |
| `CLI-OUTPUT-02` | `--quiet` suppresses all narration and lets errors through | 3 | implemented |
| `CLI-OUTPUT-03` | each verbosity level contains the previous one, line for line | 3 | implemented |
| `CLI-OUTPUT-04` | the default level writes one line per file processed | 3 | implemented |
| `CLI-OUTPUT-05` | the summary only appears from two input files on | 3 | implemented |
| `CLI-OUTPUT-06` | level 3 names each reading diagnostic, on every subcommand | 3 | implemented |
| `CLI-BATCH-01` | several files are processed independently, the failures named | 3 | implemented |
| `CLI-BATCH-02` | all failing gives code 2, a partial failure code 3 | 3 | implemented |
| `CLI-INSPECT-01` | format, encoding, BOM, line endings, count and span are reported | 3 | implemented |
| `CLI-INSPECT-02` | mixed line endings are flagged with their line | 3 | implemented |
| ~~`CLI-INSPECT-03`~~ | `--order-report` gives both readings of the disorder | 3 | abandoned: the corpus did not decide between the two readings, the option goes away and the report names what breaks the order |
| `CLI-CONVERT-01` | `--to` produces the requested format | 3 | implemented |
| `CLI-CONVERT-02` | line endings and BOM are preserved by default, adjustable otherwise | 3 | implemented |
| `CLI-CONVERT-03` | without an explicit destination, nothing is written and the code is 1 | 3 | implemented |
| `CLI-SHIFT-01` | `--by` shifts every position, in both directions | 3 | implemented |
| `CLI-SHIFT-02` | a shift that makes a position negative is refused, naming the subtitle | 3 | implemented |
| `CLI-TRANSFORM-01` | two reference points transform the positions, the others follow | 3 | implemented |
| `CLI-TRANSFORM-02` | two identical indices are refused | 3 | implemented |
| `CLI-FRAMERATE-01` | `--from`/`--to` converts the positions | 3 | implemented |
| `CLI-FRAMERATE-02` | a zero or negative frame rate is refused | 3 | implemented |
| `CLI-HEARING-01` | `hearing-impaired` removes the mentions between brackets, and leaves parentheses by default | 4 | implemented |
| `CLI-HEARING-02` | a subtitle the removal empties is deleted from the written file | 4 | implemented |
| `CLI-HEARING-03` | a purely numeric reference is left as is | 4 | implemented |
| `CLI-HEARING-04` | the report names the number of subtitles changed and deleted | 4 | implemented |
| `CLI-HEARING-05` | a file without any mention is written unchanged, code 0 | 4 | implemented |
| `CLI-HEARING-06` | without an explicit destination, nothing is written and the code is 1 | 4 | implemented |
| `CLI-HEARING-07` | `--parentheses` also removes the mentions between parentheses | 4 | implemented |
| `GUI-VERSION-01` | `subedit-gui --version` writes `subedit <version>` on standard output, alone on the first line, code 0 | 5 | implemented |
| `GUI-OPEN-01` | `subedit-gui <file>` opens the file and displays its subtitles | 5 | implemented |
| `GUI-OPEN-02` | an unreadable file gives a message and leaves the window empty | 5 | implemented |
| `GUI-OPEN-03` | the reading diagnostics are shown, with their line | 5 | implemented |
| `GUI-TABLE-01` | the five columns display number, start, end, duration and text | 5 | implemented |
| `GUI-TABLE-02` | subtitles with anomalies are marked and named | 5 | implemented |
| `GUI-TABLE-03` | the columns, except the text, can be hidden; all can be moved; the table remembers it | 11 | implemented |
| `GUI-EDIT-01` | editing a text cell modifies the subtitle and nothing else | 5 | implemented |
| `GUI-EDIT-02` | editing a start or an end reads a permissive timestamp | 5 | implemented |
| `GUI-EDIT-03` | a commit that changes nothing does not enter the history | 5 | implemented |
| `GUI-EDIT-04` | each line of a text cell and of its editor shows its length, tags not counted, in ems or in characters depending on the setting, and the settings remove it | 12 | implemented |
| `GUI-UNDO-01` | undo restores the previous state, the action names the operation | 5 | implemented |
| `GUI-UNDO-02` | redo does again what was just undone | 5 | implemented |
| `GUI-SAVE-01` | save rewrites the file in its original form | 5 | implemented |
| `GUI-SAVE-02` | save as chooses path and format | 5 | implemented |
| `GUI-SAVE-03` | closing with unsaved changes asks for confirmation | 5 | implemented |
| `GUI-SHIFT-01` | the shift dialog shifts the target | 5 | implemented |
| `GUI-TRANSFORM-01` | the transform dialog corrects through two reference points | 5 | implemented |
| `GUI-SCOPE-01` | an operation dialog offers the choice between the selection and the whole project when only part of it is selected | 5 | implemented |
| `GUI-PREVIEW-01` | an operation dialog shows, before applying, the first subtitles it would change | 5 | implemented |
| `GUI-SORT-01` | `Sort Subtitles` puts the project in order of starts, undoably, and says how many subtitles moved | 5 | implemented |
| `GUI-SORT-02` | a file opened out of order offers to be sorted, and is sorted only on a yes | 5 | implemented |
| `GUI-SORT-03` | the list of diagnostics counts by kind, reading diagnostics as well as subtitle anomalies, follows the corrections, and leads to the first subtitle concerned | 5 | implemented |
| `GUI-SORT-04` | an indeterminate bar in the status bar says a sort is in progress, and goes away with it | 5 | implemented |
| `GUI-DISJOINT-01` | `Make Ranges Disjoint…` cuts the overlapping subtitles across the whole project, undoably, selects the lines written and says what it did; out of order, it is refused with the reason | 5 | implemented |
| `GUI-DISJOINT-02` | a file opened with fixable defects (repeats, out of order, overlaps) offers them in a single box, one checkbox per correction counted where it takes place, and applies only what is checked, in the order join, sort, cut | 5 | implemented |
| `GUI-JOIN-01` | `Join Repeated Subtitles` makes a single subtitle out of consecutive pieces of the same text, without depending on the file's order, undoably, and says how many it joined | 5 | implemented |
| `GUI-JOIN-02` | `Repair Subtitles…` chains joining repeats, sorting and cutting ranges in one go, undoably, and gives a single account of it | 5 | implemented |
| `GUI-FRAMERATE-01` | the conversion dialog re-times the target | 5 | implemented |
| `GUI-HEARING-01` | removing mentions applies to the selection or to the file | 5 | implemented |
| `GUI-HEARING-02` | a removal that changes nothing says so and does not enter the history | 5 | implemented |
| `CLI-INSPECT-04` | the anomalies of a document are reported by subtitle number | 5 | implemented |
| `CLI-INSPECT-08` | `inspect` counts the anomalies by kind, one per line with `--detail`, and keeps `none` on the `anomalies` line when there are none | 5 | implemented |
| `CLI-INSPECT-05` | `inspect` writes the inferred grid, its concentration and the verdict | 16 | implemented |
| `CLI-INSPECT-06` | a file without a grid says so, and names no frame rate | 16 | implemented |
| `CLI-INSPECT-07` | a harmonic ambiguity is named, and the lowest one is retained | 16 | implemented |
| `CLI-SNAP-01` | `snap --rate R` moves each position to the nearest frame of R | 16 | implemented |
| `CLI-SNAP-02` | `snap` writes how many positions moved, and by how much at most | 16 | implemented |
| `CLI-SHIFT-03` | `shift --to-grid` shifts by the measured amount, and writes it | 16 | implemented |
| `CLI-SHIFT-04` | `shift --to-grid` on a file without a grid refuses and says why | 16 | implemented |
| `GUI-GRID-01` | the status bar carries the verdict of the inference | 16 | implemented |
| `GUI-GRID-02` | the analysis dialog shows the eight candidates, the span and the deviations | 16 | implemented |
| `GUI-FRAMERATE-03` | the conversion dialog pre-fills the input with the measurement, and says so | 16 | implemented |
| `GUI-FRAMERATE-04` | a disagreement between the measurement and what the video declares is shown without being arbitrated | 16 | implemented |
| `GUI-SNAP-01` | the window snaps the document to a frame rate, and the operation can be undone | 16 | implemented |
| `GUI-GRID-03` | bringing back onto the grid displays its amount, and is disabled without a grid | 16 | implemented |
| `GUI-VIDEO-01` | choosing a video associates it with the document, and the window names it | 6 | implemented |
| `GUI-VIDEO-02` | opening a subtitle file offers the neighbouring video with the same name | 6 | implemented |
| `GUI-PLAYER-01` | the video opens in the window, and plays | 6 | implemented |
| `GUI-PLAYER-02` | selecting a subtitle puts playback at its start | 6 | implemented |
| `GUI-PLAYER-03` | a video that does not open says so, and leaves the window usable | 6 | implemented |
| `GUI-FRAMERATE-02` | the frame rate read from the video is offered, and its origin is stated | 6 | implemented |
| `GUI-BOUNDS-01` | an operation that goes past the end of the film flags it without preventing it | 6 | implemented |
| `GUI-CONFIG-01` | the window finds its geometry and its columns again from one session to the next | 7 | implemented |
| `GUI-CONFIG-02` | an unreadable value leaves the default in place, and says so | 7 | implemented |
| `GUI-CONFIG-03` | an option left at its default is rewritten commented out, so a changed default takes effect | 7 | implemented |
| `GUI-THEME-01` | the theme is chosen among three values, is remembered, and "system" sets nothing | 7 | implemented |
| `GUI-THEME-02` | "light" and "dark" set an explicit palette | 7 | implemented |
| `GUI-INSERT-01` | inserting places the subtitles after the last selected one, or before by choice | 7 | implemented |
| `GUI-INSERT-02` | inserting into an empty document requires no selection | 7 | implemented |
| `GUI-REMOVE-01` | removing deletes the selection, and the operation can be undone | 7 | implemented |
| `GUI-MANUAL-01` | `Help ▸ Manual` opens the installed manual | 7 | implemented |
| `CLI-ENC-01` | a non-UTF-8 file opens, and the encoding retained is stated | 8 | implemented |
| `CLI-ENC-02` | `--encoding` imposes the reading encoding and overrides detection | 8 | implemented |
| `CLI-ENC-03` | a BOM overrides `--encoding`, and the discrepancy is stated | 8 | implemented |
| `CLI-ENC-04` | `--to-encoding` chooses the encoding produced on writing | 8 | implemented |
| `CLI-ENC-05` | a file rewritten without instructions gives back the same bytes, encoding included | 8 | implemented |
| `CLI-ENC-06` | a file no encoding decodes is refused, and the reason says so | 8 | implemented |
| `CLI-ENC-07` | `inspect` reports the encoding read and whether it was guessed | 8 | implemented |
| `GUI-ENC-01` | the window opens a non-UTF-8 file and displays the encoding retained | 8 | implemented |
| `GUI-ENC-02` | `Save As…` chooses the encoding, the line ending and the BOM | 8 | implemented |
| `GUI-ENC-03` | the settings remember the last writing encoding chosen | 8 | implemented |
| `CLI-FORMAT-01` | each of the nine formats opens, and is rewritten byte for byte | 9 | implemented |
| `CLI-FORMAT-02` | a file none of the nine claims is refused, without guessing | 9 | implemented |
| `CLI-FORMAT-03` | `inspect` names the format read among the nine | 9 | implemented |
| `CLI-FORMAT-04` | a format without ends opens, its ends are inferred, and the reading says so | 9 | implemented |
| `CLI-FRAMES-01` | a frame-based file opens at a declared frame rate, which the reading names | 9 | implemented |
| `CLI-FRAMES-02` | `--frame-rate` imposes the frame rate, on reading as on writing | 9 | implemented |
| `CLI-CONVERT-04` | `--to` accepts the nine formats | 9 | implemented |
| `CLI-CONVERT-05` | converting to a frame-based format without a frame rate takes the inferred grid, and says so | 9 | implemented |
| `CLI-CONVERT-06` | with neither frame rate nor grid, a conversion to a frame-based format is refused | 9 | implemented |
| `CLI-CONVERT-07` | a conversion that loses something says so, item by item | 9 | implemented |
| `GUI-FORMAT-01` | the window opens the nine formats | 9 | implemented |
| `GUI-FORMAT-02` | `Save As…` offers the nine formats | 9 | implemented |
| `GUI-FORMAT-03` | saving to a lossy format announces what will be lost before writing | 9 | implemented |
| `GUI-FRAMES-01` | opening a frame-based file announces the frame rate retained, and lets it be changed | 9 | implemented |
| `GUI-ITALIC-01` | one action puts the selection in italics and takes it out, in the tags of the open format | 10 | implemented |
| `GUI-ITALIC-02` | a format that carries no style disables the action without hiding it | 10 | implemented |
| `GUI-CASE-01` | the four cases apply to the target without touching the tags | 10 | implemented |
| `GUI-DASH-01` | dialogue dashes are added and removed with the same gesture | 10 | implemented |
| `GUI-SPLIT-01` | merge and split, and undo gives back the document as it was | 10 | implemented |
| `GUI-DURATION-01` | the `Duration` column can be edited, and moves the end | 10 | implemented |
| `GUI-ADJUST-01` | adjusting applies the four constraints in the specified order | 10 | implemented |
| `GUI-ADJUST-02` | what no end can satisfy is stated, and counted | 10 | implemented |
| `GUI-SEARCH-01` | search in the visible text, replace in the source, without breaking the tags | 10 | implemented |
| `GUI-SEARCH-02` | the search covers the selection, or the whole document | 10 | implemented |
| `GUI-CLIP-01` | copy, cut, paste texts, and lines are created if there are not enough | 10 | implemented |
| `GUI-CLIP-02` | pasting from a document of another format translates the tags and states the loss | 10 | implemented |
| `GUI-TRANS-01` | opening a translation file aligns it with the main one, by position or by number | 11 | implemented |
| `GUI-TRANS-02` | opening says what happened: lines attached, subtitles created, subtitles without a translation, lines out of order | 11 | implemented |
| `GUI-TRANS-03` | opening a translation is a single history entry, and undo gives back the document as it was | 11 | implemented |
| `GUI-TRANS-04` | the translation column appears with the translation, and can be edited | 11 | implemented |
| `GUI-TRANS-05` | a text operation targets the document of the current column, and the status bar says so | 11 | implemented |
| `GUI-TRANS-06` | the translation is saved separately from the main one, each with its own modified state | 11 | implemented |
| `GUI-CLOSE-01` | closing with several modified documents asks a single question, one checkbox per document | 11 | implemented |
| `GUI-SEARCH-03` | the search covers the targeted document, and replace writes into its source text | 11 | implemented |
| `GUI-PLAYER-04` | the line drawn on the picture is that of the targeted document | 11 | implemented |
| `GUI-APPEND-01` | appending a file shifts it by the end of the last subtitle, states what the conversion loses, and is undone in one go | 11 | implemented |
| `GUI-TABS-01` | several projects open in tabs, each with its history, its selection and its video | 11 | implemented |
| `GUI-TABS-02` | close all, or quit, with a single question for all projects | 11 | implemented |
| `GUI-TABS-03` | a tab's label says its project is modified | 11 | implemented |
| `GUI-TABS-04` | dropping files on the window opens them: one tab per subtitle file, a video to the current tab | 11 | implemented |
| `GUI-SAVE-04` | save all writes each modified document, tab by tab, and stops at a cancellation, saying what was written | 11 | implemented |
| `GUI-PSPLIT-01` | split a project in two, and appending the second part gives back the original project | 11 | implemented |
| `GUI-SEARCH-04` | the search covers, optionally, all open projects | 11 | implemented |
| `GUI-CORRECT-01` | the assistant applies the checked tasks to the chosen target — selection, project, all projects — and to the chosen document | 12 | implemented |
| `GUI-CORRECT-02` | each changed text is shown with its original before being applied, and is accepted, rejected or touched up | 12 | implemented |
| `GUI-CORRECT-03` | applying makes one history entry per project, and the status bar says how many subtitles were modified and deleted | 12 | implemented |
| `GUI-CORRECT-04` | patterns are checked by name, for a script, a language and a country, and the choice is remembered | 12 | implemented |
| `GUI-CORRECT-05` | unchecking Human or OCR removes those patterns from the application | 12 | implemented |
| `GUI-CORRECT-06` | a pattern that does not read, does not translate or does not terminate is named, and the others apply | 12 | implemented |
| `GUI-CORRECT-07` | patterns dropped in by the user are added to the shipped patterns | 12 | implemented |
| `GUI-HEARING-03` | the assistant removes lyrics between hash signs and the speaker's name, in addition to brackets and parentheses | 12 | implemented |
| `GUI-BREAK-01` | line breaking holds the requested length and number of lines, in characters or in ems, and skips the subtitles that already hold them | 12 | implemented |
| `GUI-SPELL-01` | the check walks through the unknown words: ignore, ignore all, add to dictionary, replace, replace all, join with the previous or the next | 12 | implemented |
| `GUI-SPELL-02` | without a dictionary for the chosen language, the spell checker's functions are greyed out and say why | 12 | implemented |
| `GUI-SPELL-03` | the assistant joins and splits words according to the spell checker | 12 | implemented |
| `GUI-SPELL-04` | an unknown word is underlined while it is typed in a text cell's editor, when the setting is true and a dictionary exists; without a dictionary, nothing is underlined and nothing fails | 12 | implemented |
| `CLI-JSON-01` | `--format json` writes one JSON object per line on standard output; `text` is the default and does not change | 13 | implemented |
| `CLI-JSON-02` | each input gives **exactly one** object, failure included, in the order of the inputs | 13 | implemented |
| `CLI-JSON-03` | each object carries `schema`, `command`, `file` and `ok`; a failure carries `error.kind` and `error.message` | 13 | implemented |
| `CLI-JSON-04` | verbosity does not act on standard output in `json`; the narration stays text on standard error | 13 | implemented |
| `CLI-JSON-05` | the reading diagnostics are in `warnings`, at every level | 13 | implemented |
| `CLI-JSON-06` | no floating-point number: positions in whole milliseconds, frame rates as strings | 13 | implemented |
| `CLI-JSON-07` | a usage error writes nothing on standard output, in `json` as in `text` | 13 | implemented |
| `CLI-JSON-08` | `inspect` describes the file: format, encoding, mark, line endings, count, span, grid or frame rate, anomalies | 13 | implemented |
| `CLI-JSON-09` | each subcommand that writes states its destination and its counts | 13 | implemented |
| `CLI-JSON-10` | the same inputs and the same arguments give the bytes of the versioned expected outputs | 13 | implemented |
| `CLI-DRYRUN-01` | `--dry-run` writes no file and creates no directory, and the code is that of a real run | 13 | implemented |
| `CLI-DRYRUN-02` | `--dry-run` requires no destination | 13 | implemented |
| `CLI-DRYRUN-03` | a destination given with `--dry-run` is checked as it would be without it | 13 | implemented |
| `CLI-DRYRUN-04` | on a text subcommand, standard output carries, for each changed subtitle, its number, the text before and the text after | 13 | implemented |
| `CLI-DRYRUN-05` | in `json`, `changes` carries the same changes, `after` being `null` for a deletion | 13 | implemented |
| `CLI-DRYRUN-06` | the narration of a `--dry-run` says that nothing was written | 13 | implemented |
| `CLI-BATCH-03` | two inputs with the same destination are refused before anything is written, code `1`, both named | 13 | implemented |
| `CLI-BATCH-04` | a destination that is an input is refused without `--in-place` | 13 | implemented |
| `CLI-BATCH-05` | the missing output directory is created | 13 | implemented |
| `CLI-BATCH-06` | a write that fails says "written", never "read" | 13 | implemented |
| `CLI-BATCH-07` | an existing destination is overwritten by an atomic write | 13 | implemented |
| `CLI-BATCH-08` | a directory as input without `--recursive` is refused, code `1` | 13 | implemented |
| `CLI-BATCH-09` | `--recursive` walks in name order, without following links, the extensions of the known formats, `.txt` excluded | 13 | implemented |
| `CLI-BATCH-10` | the tree relative to the given directory is preserved under `--output-dir` | 13 | implemented |
| `CLI-BATCH-11` | the output directory, when inside the walked tree, is excluded from the walk | 13 | implemented |
| `CLI-BATCH-12` | a file named on the command line is never filtered by its extension | 13 | implemented |
| `CLI-RANGE-01` | `--range N-M` limits the operation to subtitles N to M, bounds included; `N-` goes to the end | 13 | implemented |
| `CLI-RANGE-02` | an out-of-bounds range is refused, naming the bound, before any processing | 13 | implemented |
| `CLI-ADJUST-01` | `adjust` without an option applies Gaupol's four constraints, in their order | 13 | implemented |
| `CLI-ADJUST-02` | each constraint can be set and turned off (`off`); `--maximum` turns it on | 13 | implemented |
| `CLI-ADJUST-03` | without any active constraint, `adjust` is refused | 13 | implemented |
| `CLI-ADJUST-04` | the report states what was adjusted and what was sacrificed, constraint by constraint | 13 | implemented |
| `CLI-ADJUST-05` | in `json`, `counts` carries `adjusted` and `sacrificed.{speed,minimum,gap}`, and `constraints` the constraints used | 13 | implemented |
| `CLI-ADJUST-06` | the sacrificed counts agree with those the measurement script predicts on the versioned fixtures | 13 | implemented |
| `CLI-CORRECT-01` | `--tasks` is required and names the tasks; none is checked in advance | 13 | implemented |
| `CLI-CORRECT-02` | `--code` is required as soon as a task reads patterns | 13 | implemented |
| `CLI-CORRECT-03` | the report states the texts changed and deleted, never the matches | 13 | implemented |
| `CLI-CORRECT-04` | `--classes` removes the patterns of the unchecked class from the application | 13 | implemented |
| `CLI-CORRECT-05` | `--enable` / `--disable` set a pattern by its name; an unknown or ambiguous name is refused | 13 | implemented |
| `CLI-CORRECT-06` | a pattern task with no active pattern is refused | 13 | implemented |
| `CLI-CORRECT-07` | the user's patterns are added to the shipped patterns | 13 | implemented |
| `CLI-CORRECT-08` | a pattern that does not read, does not translate or does not terminate is named along with the subtitle, and the others apply | 13 | implemented |
| `CLI-CORRECT-09` | `line-break` measures in characters; `--max-length` is required; the break's bounds follow, and can be turned off | 13 | implemented |
| `CLI-CORRECT-10` | `join-words` and `split-words` join and split according to the dictionary of `--language` | 13 | implemented |
| `CLI-CORRECT-11` | without a dictionary for `--language`, `correct` is refused with the window's sentence, before any processing | 13 | implemented |
| `CLI-CORRECT-12` | emptied subtitles are removed, unless `--keep-blank-subtitles` | 13 | implemented |
| `CLI-CORRECT-13` | `correct` reads from the settings file only `correction.human`, `correction.ocr`, `correction.remove-blank` and `correction.line-break.max-lines`; neither the activations, nor its replacement list | 13 | implemented |
| `CLI-REPLACE-01` | `replace` searches in the visible text and replaces in the source text, without breaking a tag | 13 | implemented |
| `CLI-REPLACE-02` | `--regex` reads an expression; an unreadable expression is refused before any processing, with the reason | 13 | implemented |
| `CLI-REPLACE-03` | case is ignored by default, `--case-sensitive` distinguishes it | 13 | implemented |
| `CLI-REPLACE-04` | the report states the texts changed, or that nothing was found | 13 | implemented |
| `CLI-CASE-01` | `case` applies the four cases without touching the tags | 13 | implemented |
| `CLI-ITALIC-01` | `italics --on` / `--off` sets and removes italics in the tags of the file's format | 13 | implemented |
| `CLI-ITALIC-02` | a format that carries no style refuses, file by file, with the reason | 13 | implemented |
| `CLI-DASH-01` | `dialogue-dashes --add` / `--remove` adds and removes dialogue dashes | 13 | implemented |
| `CLI-SORT-01` | `sort` puts the subtitles in order of their start, stably | 13 | implemented |
| `CLI-SORT-02` | `sort` says how many subtitles moved, and writes the file even if it is already in order | 13 | implemented |
| `CLI-SORT-03` | `convert`, `append` and `split-file` accept `--sort`, which puts the subtitles in order of their start before writing and says how many moved | 13 | implemented |
| `CLI-DISJOINT-01` | `disjoint` cuts the overlapping subtitles, merges line by line what is displayed together, says what it did, and a second pass changes nothing | 13 | implemented |
| `CLI-DISJOINT-02` | `disjoint` refuses a file whose starts are not in order, except with `--sort`, which puts it in order first | 13 | implemented |
| `CLI-DISJOINT-03` | `--minimum`, `--max-lines` and `--dashes` set the operation; an absurd value is a usage error naming the option | 13 | implemented |
| `CLI-DISJOINT-04` | `convert`, `append` and `split-file` accept `--disjoint`, which cuts the ranges before writing, after `--sort`, and says what was cut | 13 | implemented |
| `CLI-JOIN-01` | `join-repeats` makes a single subtitle out of consecutive pieces of the same text, says how many it joined, and a second pass changes nothing | 13 | implemented |
| `CLI-JOIN-02` | `repair` chains joining repeats, sorting and cutting ranges, in that order, and states the three counts | 13 | implemented |
| `CLI-JOIN-03` | `convert` accepts `--join-repeats` (before sorting and cutting) and `--repair` (all three in one go), and says how many subtitles were joined | 13 | implemented |
| `CLI-TRANS-01` | `inspect -t` reports lines attached, subtitles created, without a translation and out of order | 13 | implemented |
| `CLI-TRANS-02` | `--document translation` requires `-t`, and `-t` requires `--document translation` on a text subcommand | 13 | implemented |
| `CLI-TRANS-03` | `-t` only makes sense for a single input; with a batch, it is refused | 13 | implemented |
| `CLI-TRANS-04` | only the targeted document is written, at its own path and in its own format | 13 | implemented |
| `CLI-TRANS-05` | `--align-method` chooses between position (default) and number | 13 | implemented |
| `CLI-LISTENC-01` | `--list-encodings` writes the encodings ICU can convert, one per line, and stops | 13 | implemented |
| `CLI-APPEND-01` | `append` adds the files after the first one, shifted by the end of the last subtitle, into a single output | 13 | implemented |
| `CLI-PSPLIT-01` | `split-file --at N` writes the two halves, the second brought back to the origin, or refuses, naming the subtitle | 13 | implemented |
| `CLI-PAIR-01` | `pair` writes the translation re-timed on the positions of the main one, and states the alignment | 13 | implemented |
| `GUI-SURFACE-01` | the video picture is drawn in the window, resizable, its aspect ratio preserved | 14 | implemented |
| `GUI-SURFACE-02` | after a seek, the picture displayed is that of the requested position, the nearest one | 14 | implemented |
| `GUI-SURFACE-03` | the window adopts no native window and does not require X11 | 14 | implemented |
| `GUI-STEP-01` | stepping forward one frame displays the next frame | 14 | implemented |
| `GUI-STEP-02` | stepping back one frame displays the previous frame | 14 | implemented |
| `GUI-STEP-03` | the duration of a frame comes from the video, failing that from the document, failing that from the grid; with none of them, the gesture refuses and says so | 14 | implemented |
| `GUI-STEP-04` | the step is set as a number of frames, at least one; an absurd value does not break opening | 14 | implemented |
| `GUI-NUDGE-01` | moving the start or the end of a subtitle by one frame is undoable in one entry | 14 | implemented |
| `GUI-SEEK-01` | the position slider can be read and moved | 14 | implemented |
| `GUI-SEEK-02` | step back and forward by an adjustable step | 14 | implemented |
| `GUI-SEEK-03` | the previous and the next subtitle put playback at their start | 14 | implemented |
| `GUI-SEEK-04` | the start and the end of the selection place playback, with the configured lead | 14 | implemented |
| `GUI-SEEK-05` | playing the selection stops at its end | 14 | implemented |
| `GUI-VOLUME-01` | the volume can be set and is remembered from one session to the next | 14 | implemented |
| `GUI-TIMECODE-01` | the timecode is overlaid on the picture | 14 | implemented |
| `GUI-AUDIO-01` | the audio track is chosen among those of the video | 14 | implemented |
| `GUI-MARK-01` | set a subtitle's start from the video position, in one history entry | 14 | implemented |
| `GUI-MARK-02` | set its end from the video position | 14 | implemented |
| `GUI-MARK-03` | insert a subtitle at the video position | 14 | implemented |
| `GUI-MARK-04` | select the previous or next subtitle from the video position | 14 | implemented |
| `GUI-FOLLOW-01` | the table centres the current subtitle during playback | 14 | implemented |
| `GUI-FOLLOW-02` | scrolling by hand suspends following | 14 | implemented |
| `GUI-FOLLOW-03` | following resumes on a player gesture or on request | 14 | implemented |
| `GUI-REPLICA-01` | the line drawn on the picture no longer has raw tags | 14 | implemented |
| `GUI-FRAMES-02` | positions are displayed and entered as frame numbers, the frame rate stated | 14 | implemented |
| `GUI-DRIFT-01` | opening a translation says that a constant shift would attach it better, without saying so of a drift | 14 | implemented |
| `GUI-DRIFT-02` | reopening shifted attaches according to the proposed shift, in one history entry | 14 | implemented |
| `GUI-REPAIR-01` | the analysis dialog proposes the frame rate conversion that puts the file back on a grid, or nothing when two are equally good | 14 | implemented |
| `GUI-REPAIR-02` | `Convert Frame Rate…` opens pre-filled with that proposal | 14 | implemented |
| `CLI-I18N-01` | the command line's help, narration and errors follow the language chosen by `LANGUAGE`, `LC_ALL`, `LC_MESSAGES`, `LANG`, English when it is unknown | 15 | implemented |
| `CLI-I18N-02` | `--format json` output is byte-identical whatever the language | 15 | implemented |
| `CLI-I18N-03` | `LC_ALL=C` yields English | 15 | implemented |
| `CLI-I18N-04` | `subedit-cli(1)` and one page per sub-command are installed, and list the options `--help` lists | 15 | implemented |
| `GUI-I18N-01` | the window shows its menus, dialogs and messages in the chosen language, English when it is not installed | 15 | implemented |
| `GUI-I18N-02` | `Preferences…` offers the interface language; it is kept from one session to the next and applies at the next launch | 15 | implemented |
| `GUI-I18N-03` | `Help ▸ Manual` opens the manual of the chosen language, English when the page or the language is missing | 15 | implemented |
| `GUI-I18N-04` | the names and descriptions of Gaupol's correction patterns are shown translated, and a user's own pattern is shown as written | 15 | implemented |
| `CLI-VERSION-06` | after its first line, `--version` names each library the command line runs against — ICU, Enchant, CLI11 — with the version loaded and the version built against, then the installed languages; the words follow the language, the names and versions do not | 15 | implemented |
| `CLI-VERSION-07` | `--version --format json` writes one object with the version, the libraries and the languages, wherever `--format` is written | 15 | implemented |
| `CLI-VERSION-08` | every library the build looks for with `find_package` or `pkg_check_modules` is named by the `--version` of one binary or the other | 15 | implemented |
| `GUI-VERSION-02` | `subedit-gui --version` names Qt, ICU, libmpv and Enchant with both of their versions, then the installed languages | 15 | implemented |
| `GUI-ABOUT-01` | `About subedit` lists the libraries of `--version` in a table — name, version loaded, version built against — and the installed languages, its headings in the chosen language | 15 | implemented |
| `GUI-TRANSFORM-02` | the transform dialog shows each marker's text in three lines of room that scroll past them, keeps its size when a marker changes, and shows its explanation whole at any width and in any language | 15 | implemented |
| `CLI-TRANSCRIBE-01` | `transcribe` writes a subtitle file, in the format of `--output`, from the audio of a video or of an audio file | 22 | implemented |
| `CLI-TRANSCRIBE-02` | `--audio-track` chooses the audio track of a video, and a video with no such track is refused with the tracks it has | 22 | implemented |
| `CLI-TRANSCRIBE-03` | the language is detected unless `--language` forces it, and the language used is reported | 22 | implemented |
| `CLI-TRANSCRIBE-04` | an option writes the extracted audio to the file it names | 22 | implemented |
| `CLI-TRANSCRIBE-05` | `--raw` writes the segments of the engine with no post-processing | 22 | implemented |
| `CLI-TRANSCRIBE-06` | a missing executable, model or extraction tool is reported with a pointer to the installation page, with a non-zero exit code and nothing written | 22 | implemented |
| `CLI-TRANSCRIBE-07` | a language the chosen model cannot transcribe is refused before any work starts | 22 | implemented |
| `CLI-TRANSCRIBE-08` | `--format json` writes one record with the language used, the counts and the files written | 22 | implemented |
| `GUI-TRANSCRIBE-01` | `Transcribe Audio…` opens a dialog offering the audio tracks of the project's video, a language and a model | 22 | implemented |
| `GUI-TRANSCRIBE-02` | the transcription runs in the background with a progress indicator and can be cancelled, a cancelled or failed run leaving the project unchanged | 22 | implemented |
| `GUI-TRANSCRIBE-03` | the transcription replaces the subtitles of the project only once the person has applied it from the preview, as one undoable operation | 22 | implemented |
| `GUI-TRANSCRIBE-04` | without the executable, the model or the extraction tool the command explains what is missing and where to read how to install it, and the rest of the window is unaffected | 22 | implemented |
| `CLI-TRANSCRIBE-09` | `--start` and `--end` restrict the transcription to a part of the film, the subtitles keep the time of the film, and a bound that is no time or a range whose end comes first is refused before any work | 22 | implemented |
| `CLI-TRANSCRIBE-10` | while it runs the command says the step, the percent, the position in the film and the time left, and at the third level the text last obtained | 22 | implemented |
| `CLI-TRANSCRIBE-11` | `--list-languages` lists the languages the engine offers for the model, one per line with its name in the language of the interface, and transcribes nothing | 22 | implemented |
| `CLI-TRANSCRIBE-12` | `--suppress-notes` and `--keep-notes` decide whether the engine writes sounds as notes, over the setting; both together are refused, and an engine that cannot be told refuses the first by name | 22 | implemented |
| `GUI-TRANSCRIBE-05` | the progress box says the step, the position in the film, the time left and the last text obtained, and is busy rather than at 0 % while nothing says how far it is | 22 | implemented |
| `GUI-TRANSCRIBE-06` | the dialog takes a range of the film and transcribes only that part, the result replacing the subtitles of the project like any transcription, after the same question | 22 | implemented |
| `GUI-TRANSCRIBE-07` | the transcription is shown in a preview before it replaces the subtitles of the project — the engine's pieces on the left, the final subtitles on the right, each with its own times, a selected subtitle highlighting its sources — and a cancelled or failed run offers what it had obtained, to apply or to ignore | 22 | implemented |
