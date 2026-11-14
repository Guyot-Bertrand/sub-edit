# The narration of the command line, one fact per line

**Status: scoped (#806).** This document fixes the shape of what `subedit-cli` writes on the standard error, and splits the work
into issues. It changes neither the standard output nor the JSON records.

What the request asks: one information per line, each line prefixed with the program and the subcommand, so that it reads better in
a terminal and chains better in scripts. What this document adds to it: **in a batch, every line says which element it is about**,
without a script having to parse a path.

## What stays as it is

- **The standard output is the result, and only the result** — the report of `inspect`, the diff of `--dry-run`, the
  `--list-*`, `--help`, `--version`, `config list`, and the JSON records of `--format json`. None of it gains a prefix.
- **The JSON records stay the channel of the data** (ADR 0038, `schema` 1). The counts, the units, the identifiers of failure are
  promised there. The text does not compete with them; it has to be filtered and attached to an element, not to carry values.
- **The levels** (`-q`, default, `-vv`, `-vvv`), the rule that each level contains the one below it, and the rule that an error is
  written even under `-q`.
- **The exit codes.**

## D1 — The grammar of a line

```
[subedit] [<command>] [<i>/<n>] <key>: <value>
```

| Field | Content |
| :---- | :------ |
| `subedit` | constant |
| `<command>` | the name of the subcommand as typed (`hearing-impaired`, `split-file`), or `-` before it is known |
| `<i>/<n>` | the element of the batch the line is about: its position, counted from one, and the number of elements; `-` for a line about the run as a whole |
| `<key>` | one of the closed list of D3, lower case, `-` between words |
| `<value>` | **the rest of the line**, after `: ` |

**No space before the colon**, as everywhere else in the program.

**The four first fields are constants, never translated**, like the names of the subcommands and the keys of the JSON. Only a
value that is a sentence is translated. A script may rely on `[subedit] [shift] [2/5] dest:` whatever the language of the machine.

**The value is the rest of the line**, so that a path is written as it is, spaces, colons and brackets included, with no
escaping. The one character it cannot carry is the line end: a path that contains one is written with `\n` in its place, and its
faithfulness is not promised — the same stance as the `path-not-utf8` warning of the records.

**The number of fields never varies.** A line about no element writes `-` where the element would be, and a line written before
the subcommand is known writes `-` for the command too:

```
[subedit] [hearing-impaired] [-] summary: 2 of 3 files cleaned, 1 failed
[subedit] [-] [-] error: The following argument was not expected: --by
```

A script cuts every line the same way, and `[-]` selects what concerns the run.

## D2 — An element of the batch, and how it is identified

**An element is what produces one JSON record**: one input for a subcommand that takes a batch; the whole run for `append`,
`split-file` and `pair`, whose arity is their own and which write one record. `<n>` is therefore the number of records the run
writes; with `--recursive`, the number of files the walk kept.

**The position, and not the path, identifies the element.** Dismissed, with the reason:

- **The path on every line.** A path may contain any character but the line end, so no separator is safe: a line could not be cut
  without ambiguity. It would also make every line long, and repeat the same path twice under `--in-place`.
- **Consecutive lines of one file, the path on the first one only.** It holds while a batch runs in order, and breaks the day one
  runs in parallel; after a `grep`, the lines lose their file.

The position holds in both cases: it is a token with nothing to escape, lines may interleave and still be attached, and it is the
order the records are already promised in (one record per input, in the order of the inputs).

**Finding an element by its name** is then two steps, and the manual says so:

```console
$ subedit-cli hearing-impaired --output-dir o *.srt 2> log
$ grep -F '] source: b.srt' log           # gives [subedit] [hearing-impaired] [1/3] source: b.srt
$ grep -F '[1/3]' log                     # everything about b.srt
```

**Rules that hold for every element:**

1. **Its `source` line comes first.** Every element begins with the line that names its input.
2. **An error never comes without its source.** Under `-q`, where only errors are written, the `source` line of the element is
   written just before its first error, so that `[2/3] error:` is never alone. The reporter does it: the call sites do not.
3. **`dest` is written on every success that writes**, under `--in-place` too, where it is the source. Regularity is worth more
   than the saving of a line.
4. **No `dest` under `--dry-run`**: nothing is written. Its absence says it, and the sentence of `result` says it too.
5. **`[1/1]` is written for a single input.** A script has no case of its own to handle.
6. **The error sentence does not repeat the source.** When it is about another file — the destination that cannot be written, the
   translation that cannot be read — it names that file, as today.

## D3 — The closed list of keys

**About an element**, in the order they are written:

| Key | Level | Value |
| :-- | :---- | :---- |
| `source` | 1 (and 0 before an error) | the input, as given or as the walk composed it |
| `translation` | 1 | the translation file (`-t`, `pair`) |
| `media` | 1 | the audio or video file (`sync`), written when the work on it begins, after what was read of the subtitle file |
| `format` | 2 | the format recognised, by its name (`SubRip`) |
| `encoding` | 2 | the encoding, by its name (`UTF-8`) |
| `bom` | 2 | the byte-order mark, by the same word as the report of `inspect` |
| `line-endings` | 2 | the line endings, by the same word as the report of `inspect` |
| `bytes-read` | 3 | an integer |
| `diagnostic` | 3 | one reading diagnostic, as today (`line 12: …`) |
| `alignment` | 2 (1 for `pair` and `inspect`) | how a translation was laid |
| `progress` | 2 (3 for the last text heard) | one line per update, the current sentence kept whole (`transcribe`, `sync`) |
| `warning` | 1 | what the operation went on without |
| `error` | 0 | why the element came to nothing |
| `result` | 1 | what was done, the counts included: `1 subtitle cleaned, 1 removed` |
| `bytes-written` | 3 | an integer |
| `dest` | 1 | the file written |
| `tail` | 1 | the second file `split-file` writes |

**About the run**, with `[-]`:

| Key | Level | Value |
| :-- | :---- | :---- |
| `settings` | 1 | the settings file read; what was wrong in it is a `warning` |
| `walk` | 1 and 2 | what `--recursive` found and left aside |
| `warning` | 1 | a warning about no element |
| `error` | 0 | a usage error, a refusal of the whole batch (a collision of destinations), a last-resort failure |
| `summary` | 1 | how the batch ended, from two elements on |
| `result` | 1 | what `config set` and `config unset` did |

**The values of `format`, `encoding`, `bom` and `line-endings` are names, not sentences**, and are not translated: the
three facts that share one line today (`SubRip, UTF-8, no BOM, LF line endings kept`) become four lines, one fact each.

**`result` may be written more than once for one element**: a step asked for besides the operation (`--sort`,
`--disjoint`, `--join-repeats`) and each file `append` adds have their own line, before the one that says what the run made.

**The counts stay in the sentence of `result`, translated.** A line `counts: cleaned=1 removed=1` would duplicate the record,
and make the text a second contract on the same data. A script that wants the numbers reads the JSON.

**The progress stays one line per update.** Split into its four facts, it would write four lines every few seconds; the
sentence is kept whole under one key.

**A key is added only with this table**, as a key of the JSON is: the list is promised (D6).

## D4 — Lines that carry several lines

Some sentences carry a line end — the report of an alignment, the pointer to the installation page after a missing tool.
**Every physical line carries the prefix and the key**, repeated:

```
[subedit] [transcribe] [1/1] error: whisper-cli was not found
[subedit] [transcribe] [1/1] error: see https://…/installation.html
```

The reporter splits; no message writes a prefix itself.

## D5 — What speaks on the standard error, everywhere

**Everything that writes on the standard error goes through the reporter**, so that no line escapes the grammar:

| Today | Where | Becomes |
| :---- | :---- | :------ |
| `Reporter::say`, `Reporter::failed` — 72 calls in 15 files | `src/lib/subedit/cli/` | the keys of D3 |
| the usage errors of the argument parser | `src/exe/cli/application.cpp` | `[subedit] [<command> or -] [-] error:` — CLI11's sentence said again in the user's language by `cli::complaintIn` (#830); in English, CLI11's words |
| `refuse()` | `src/exe/cli/commands/command.cpp` | `[subedit] [<command>] [-] error:` |
| the last-resort handlers (`subedit-cli: %1`, `unknown failure`) | `src/exe/cli/main.cpp` | `[subedit] [-] [-] error:` |

`--help` and `--version` stay on the standard output, unprefixed: they are results.

**The calls by file**, for the size of the issues:

| File | Calls | What they say |
| :--- | ----: | :------------ |
| `transcribing.cpp` | 14 | progress, missing tool, model, failures |
| `syncing.cpp` | 8 | failures of the alignment on the speech |
| `conversion.cpp` | 8 | the conversion, what it lost |
| `splitting.cpp`, `appending.cpp`, `configuring.cpp` | 6 each | the runs with one record, and `config` |
| `inspection.cpp` | 5 | the levels 1 to 3 of `inspect` |
| `rewriting.cpp`, `shared_settings.cpp` | 4 each | the path every rewriting subcommand takes; the settings file |
| `batch.cpp` | 3 | the refusals of a batch, the summary |
| `writing.cpp`, `expansion.cpp`, `correcting.cpp` | 2 each | writing, the walk, the codes of `correct` |
| `records.cpp`, `diagnostics.cpp` | 1 each | the failure of an element, the reading diagnostics |

**`Failure::about(prefix)`** today puts the path before the words of a failure, for the narration as for the record. The narration
no longer wants it (rule 6 of D2: the source has its own line); **the record keeps it** — `error.message` is not promised, and
nothing in this issue gives a reason to touch the JSON.

## D6 — What is promised

The manual (*What is promised, and what is not*, `invocation.md`) gains the text:

- **Promised**: the grammar of D1; the meaning of `<i>/<n>` and its link with the order of the records; the keys of D3 and the
  level of each; the rules of D2.
- **Not promised**: the text of a value that is a sentence (`result`, `warning`, `error`, `progress`, `summary`, `walk`), which is
  translated and reworded; the order of lines of different elements, if a batch ever runs in parallel.

**Dismissed: a switch to bring the compact form back.** It would double every sentence of the catalogue, for a text that was never
promised.

**Later, and not here: an `item` key in the JSON records**, giving the same position as `<i>`. It is an addition, which leaves
`schema` at 1; the order of the records already gives the position, which is enough while a batch runs in order.

## D7 — Example, a batch at each level

```console
$ subedit-cli hearing-impaired --output-dir o b.srt bad.srt c.srt
[subedit] [hearing-impaired] [1/3] source: b.srt
[subedit] [hearing-impaired] [1/3] result: 1 subtitle cleaned, 1 removed
[subedit] [hearing-impaired] [1/3] dest: o/b.srt
[subedit] [hearing-impaired] [2/3] source: bad.srt
[subedit] [hearing-impaired] [2/3] error: is in no format this tool knows
[subedit] [hearing-impaired] [3/3] source: c.srt
[subedit] [hearing-impaired] [3/3] result: no mention to remove
[subedit] [hearing-impaired] [3/3] dest: o/c.srt
[subedit] [hearing-impaired] [-] summary: 2 of 3 files cleaned, 1 failed
```

`-vv` adds, after each `source`, the lines `format`, `encoding`, `bom` and `line-endings`; `-vvv` adds `bytes-read`, the
diagnostics, and `bytes-written` before `dest`. `-q` writes two lines:

```console
$ subedit-cli -q hearing-impaired --output-dir o b.srt bad.srt c.srt
[subedit] [hearing-impaired] [2/3] source: bad.srt
[subedit] [hearing-impaired] [2/3] error: is in no format this tool knows
```

## What it costs

- **The tests**: about 66 test files check the text of the narration, and the end-to-end tests. The reporter keeps the grammar in
  one place, so that a test of a subcommand checks keys and values, not a prefix written out.
- **The catalogue**: the patterns `%1: %2 -> %3` and `%1: %2` disappear in favour of the bare sentences; `make po` and the 20
  translations follow each issue (the `translations` step refuses a stale entry). The prefix and the keys are constants of the
  reporter, exempted in `.translation-exemptions` with their reason.
- **The manual**: the examples are regenerated by `make manual`; *Narration levels* and *What is promised* are rewritten; each
  subcommand's page says what it writes. The CHANGELOG announces the change of the text.

## Split

| Issue | Content |
| :---- | :------ |
| 1 | **The reporter**: the grammar, the element (`forItem`), the source before an error, the split of lines, the `[-]` lines; and the four direct writers of D5. The old `say`/`failed` stay, so that nothing else moves. |
| 2 | **The path of the rewriting subcommands**: `rewriting.cpp`, `writing.cpp`, `diagnostics.cpp`, `batch.cpp`, `records.cpp`, `shared_settings.cpp`, `expansion.cpp` — most subcommands at once. |
| 3 | **The other batches and the runs with one record**: `conversion.cpp`, `inspection.cpp`, `correcting.cpp`, `appending.cpp`, `splitting.cpp`, `configuring.cpp`. |
| 4 | **The speech**: `transcribing.cpp`, `syncing.cpp`, the progress. |
| 5 | **The close**: the old API removed, the manual's promise and levels, the CHANGELOG. |

**Between issues 1 and 5 the text is mixed**: some subcommands narrate in the new grammar, others in the old. It is acceptable
because the text is not promised until issue 5, which is the one that promises it; the versions in between say so in the CHANGELOG.

## Open points

1. **The `item` key of the records** (D6): later, once the text is promised.
2. **The words of `bom` and `line-endings`**: those of the report of `inspect` today; issue 2 checks that they are untranslated
   names there, or makes them so.
