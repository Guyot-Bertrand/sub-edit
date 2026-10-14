# Phase 3 — CLI

**State:** design approved, ready to be split
**Milestone:** `3 — CLI : harnais`

## Goal

Give the core an **invocable surface**, so that what it can do can be exercised
end to end on real files, and measured.

The command line does not appear in the user's needs. It is not an end: it is
the harness that lets us say the core works before a window exists. The
subcommands intended for real use belong to phase 13.

## Scope

**Included** — five subcommands: `inspect`, `convert`, `shift`, `transform`,
`framerate`. Argument parsing, help, exit codes, processing a batch of files and
the behaviour on partial failure.

**Wider than the roadmap**, which announced *inspection, conversion, shifting*.
The reason is written below.

**Excluded, by phase** — removal of hearing impaired annotations (phase 4); any
graphical interface (phase 5); the SubViewer, SSA and ASS formats (phase 9);
encodings other than UTF-8 (phase 9); the subcommands for real use and
machine-readable output (phase 13).

## Applicable decisions

| ADR | Decision |
| :-- | :------- |
| [0006](../adr/0006-positions-en-millisecondes.md) | positions in whole milliseconds, strong types |
| [0008](../adr/0008-lecture-au-mieux-avec-diagnostics.md) | best-effort reading, diagnostics reported |
| [0013](../adr/0013-mise-a-l-echelle-exacte-des-positions.md) | scaling by an exact rational |
| [0014](../adr/0014-registre-d-exigences.md) | requirement registry checked against the tests |
| [0016](../adr/0016-cli11-pour-l-analyse-d-arguments.md) | CLI11 for argument parsing |

## Why five subcommands and not three

The roadmap restricted the phase to inspection, conversion and shifting. Two
findings made while framing it widened that outline.

**The core already knows more.** `transform` and `convert_frame_rate` have been
delivered since phase 2, unit tested and measured by the benchmarks.

**A harness that does not expose an operation does not validate it.** That is
this phase's reason for being: confronting the core with whole files rather
than with constructed cases. Leaving two operations out of the CLI means keeping
code that no end-to-end test goes through until phase 13 — and discovering
then, very late, what a real file does to them.

The extra cost is small: both operations exist, all that remains is to call
them. The cost of their absence would only be paid at the moment it would be
highest.

## The language of the command line

**The CLI speaks English**: subcommand names, option names, help text,
diagnostics, error messages, report headers.

This is not an exception to the project's language rule, it is its application.
The boundary is that of the C++ compiler, and these strings are C++ literals.

The manual stays in French and **quotes the English output verbatim**. Since
`make manual` regenerates the example blocks, these quotations can no longer
lie: they are produced by the binary.

## Common grammar

### The global options

Accepted by every subcommand, and by the bare invocation:

| Option | Effect |
| :----- | :----- |
| `-q`, `--quiet` | level 0 — no narration at all |
| `-v`, `-vv`, `-vvv` | levels 1 to 3, level 1 being the default |
| `-h`, `--help` | the help, that of the subcommand if one is named |
| `--version` | `subedit <version>` |

They go **before or after** the subcommand name, either way:
`subedit-cli -vv shift …` and `subedit-cli shift -vv …` are the same
invocation. Requiring a position would be a trap with nothing in return.

### Time

Two forms accepted wherever a position or a duration is expected:

| Form | Example | Meaning |
| :--- | :------ | :------ |
| seconds | `-7.001`, `2.999`, `30` | optional sign, optional decimals, to the millisecond |
| timestamp | `01:39:37.040`, `-00:00:07.001` | `[±]HH:MM:SS.mmm` |

The decimal separator is the **point**, since the CLI speaks English. The comma
is refused with a message that says so, rather than interpreted by guesswork.

The resolution is the millisecond, as in the model (ADR 0006). A finer value is
refused: rounding it silently would give a result that matches nothing of what
was asked.

### The index of a subtitle

**Counted from 1**, as it is displayed. An out-of-bounds index is refused,
naming the bound.

### The destination

No write takes place without an explicit destination. Three ways to give it,
mutually exclusive:

| Option | Condition |
| :----- | :-------- |
| `--output FILE` | exactly one input file |
| `--output-dir DIR` | whatever the number of inputs; the base name is kept |
| `--in-place` | overwrites the inputs, by atomic write |

A harness that overwrites its input without being asked to is a harness one
stops using after the second time. `--output-dir` is **mandatory** as soon as
there are several inputs: `--output` would then make no sense, and refusing it
is better than writing the last file over the previous ones.

`inspect` writes no file and therefore accepts none of these options.

### The format of the written file

`shift`, `transform` and `framerate` **keep the format of the file read**.
Changing format is the job of `convert`, and a subcommand that did both would
make the errors of one indistinguishable from those of the other.

## What is written, and where

The exit code says **whether** it worked; it does not say **what happened**. A
harness whose whole output fits in one number forces one to reopen the files to
understand — which is precisely the work it exists to avoid.

### Two outputs, two roles

| Output | What it carries |
| :----- | :-------------- |
| standard | **the result, and only the result** — the report of `inspect`. The subcommands that write files write nothing there: their result *is* the file. |
| error | **everything else** — the narration of the processing, warnings, errors. |

This split is not a decorative convention. It makes
`subedit-cli inspect a.srt | grep …` usable without the narration polluting the
pipe, and it lets `--quiet` silence the narration **without ever silencing the
result** — which a `--quiet` that crushed standard output would make
impossible.

### Four levels

| Level | How | What the error output carries |
| :---- | :-- | :---------------------------- |
| 0 | `--quiet`, `-q` | nothing, **except errors** |
| 1 | by default, or `-v` | one line per processed file, and a summary as soon as there are several |
| 2 | `-vv` | and what was decided for each: format recognised, encoding, BOM, line endings kept or imposed, number of subtitles touched, path written |
| 3 | `-vvv` | and the debugging trace: bytes read and written, and **each reading diagnostic, named** |

**Level 3 was corrected after the fact**, in issue #78. It announced "arguments
parsed, paths resolved, reading diagnostics in detail" and delivered only the
bytes read and a *count* of diagnostics. Two of the three promises are withdrawn
rather than honoured: CLI11 already reports a faulty argument, and level 1
already prints the path written — delivering them would have added noise without
teaching anything. The third one, however, was really missing: diagnostics are
now named one by one, and by all five subcommands, not only by the one that
writes nothing.

Each level **contains the previous one**: going up a notch adds, never replaces.
A user who reruns with `-vv` to understand an incident finds, word for word,
what they had already read, plus the rest.

**Errors are never silenced, `--quiet` included.** A command that fails silently
leaves only its exit code, and turns every incident into an investigation.
"Nothing" refers to what narrates, not to what alerts.

The summary only appears from two files onwards: on a single input, "1 file
processed" repeats the line before it.

`--quiet` and `-v` in the same invocation are a **usage error**. Two opposite
intentions are not arbitrated in favour of the last one written: refusing them
tells the caller they made a mistake, where a silent arbitration would let them
believe they had been understood.

### What it looks like

Illustrative — **the manual carries the real outputs**, generated by
`make manual`, and it is authoritative.

At the default level, a batch in which one file fails:

```
a.vtt: 1356 subtitles shifted by -7.001 s -> out/a.vtt
b.vtt: read failed: line 42: timing line expected
1 of 2 files processed, 1 failed
```

Exit code `3`. With `-vv`, each file also says what was recognised and what was
written:

```
a.vtt: WebVTT, UTF-8, no BOM, LF line endings, 1356 subtitles
a.vtt: 1356 subtitles shifted by -7.001 s -> out/a.vtt
a.vtt: written as WebVTT, UTF-8, no BOM, LF line endings
```

## The subcommands

Each subcommand states below what it carries on **standard output**; its
narration on error output follows the table of levels.

### `inspect`

```
subedit-cli inspect [--order-report breaks|late] <file>...
```

Modifies nothing and writes no file.

**Standard output** — the report, per file: the recognised format, the
encoding, the presence of a BOM, the line endings — and their mixture, with the
line where the second kind appears —, the number of subtitles, the time span,
and the order breaks. This is the result of the command: it is written at every
level, `--quiet` included.

Reading diagnostics (ADR 0008) are reported as they are: a file that opens while
reporting something is not a file that failed. They go to **error output**, with
the rest of the warnings — otherwise a report passed through a pipe would
contain two kinds of lines mixed together.

### `convert`

```
subedit-cli convert --to srt|vtt
                    [--line-endings unix|windows|mac] [--bom | --no-bom]
                    (--output FILE | --output-dir DIR | --in-place) <file>...
```

`--to` is required. `--line-endings` and the BOM default to **what the file
read carried** — the phase 1 model already retains both, and imposing them
would make every conversion lose information the core had taken care to keep.

`mac` denotes the carriage return alone (`\r`), the line ending of classic
Mac OS.

**Standard output** — nothing: the result is the written file.

### `shift`

```
subedit-cli shift --by <time> (--output … ) <file>...
```

Shifts every position by `<time>`, in either direction. A shift that would make
a position negative is refused, naming the subtitle at fault.

**Standard output** — nothing: the result is the written file.

### `transform`

```
subedit-cli transform --first <index>=<time> --last <index>=<time>
                      (--output … ) <file>...
```

Two reference points: subtitle `<index>` must now start at `<time>`. All the
other positions follow, by the exact scaling of ADR 0013.

The two indices must differ — two coinciding reference points define no
transformation, and refusing it is better than returning a division by zero
disguised as a result.

**Standard output** — nothing: the result is the written file.

### `framerate`

```
subedit-cli framerate --from <fps> --to <fps> (--output … ) <file>...
```

Frame rates are written in frames per second, decimals included: `25`, `24`,
`23.976`. A zero or negative frame rate is refused.

**Standard output** — nothing: the result is the written file.

## Exit codes

| Code | Meaning |
| :--- | :------ |
| `0` | everything succeeded |
| `1` | usage error — unknown option, invalid value, forbidden combination |
| `2` | no file could be processed |
| `3` | some files were processed, others were not |

Distinguishing `2` from `3` is what lets a script act without rereading the
output: "nothing worked" and "one is missing" do not call for the same reaction.

A usage error is detected **before any processing**: it never leaves a batch
half written.

## A batch, and what is done with it

Every subcommand accepts several files. Each one is processed independently; the
failure of one does not interrupt the others, and failures are reported naming
the file and the reason.

Stopping at the first failure would force one to rerun as many times as there
are faulty files to learn their list. The harness exists to give that list in
one go.

## Disorder: two readings, and an option to decide between them

`Project::outOfOrder()` currently compares each subtitle with its **immediate
predecessor**. The other reading compares with the **greatest start seen so
far**. On the starts `0, 4000, 2000, 3000`, the first returns `{2}`, the second
`{2, 3}`.

Both always agree on whether there is disorder and differ only on the list. Up to
now, no caller consumed that list: the question could stay open.

`inspect` is the first that has to settle it. **It does not settle it: it
exposes both.**

| Value | What it returns |
| :---- | :-------------- |
| `breaks` (default) | the lines that break the order relative to the previous one |
| `late` | every line that starts before something already seen |

This is a deliberate use of what this phase is. A harness serves to discover,
and comparing the two outputs on real files will settle the matter better than
reasoning conducted without data. **Phase 5 inherits this choice**, and the
option will then disappear in favour of the reading retained — it is written in
the open points so that the disappearance has a trigger, and not an intention.

The second reading requires an addition to the phase 2 core: the rule becomes a
named parameter rather than a fixed behaviour.

## What the bare invocation becomes

Three phase 1 requirements describe a binary that parses nothing: the invocation
with no argument writes the version, and **every argument is ignored**. This
phase removes their purpose.

| Before | After |
| :----- | :---- |
| no argument → the version | no argument → the help, code `0` |
| every argument ignored, code `0` | unknown argument → usage error, code `1` |
| — | `--version` → `subedit <version>` |

**All three will move to the `replaced` state**, each with the identifier that
takes its place — `CLI-VERSION-02` included. Its wording, "nothing is written
on error output", described a binary that has no error to report; it becomes
false as soon as it has one. Keeping it by rereading it as "nothing on error
output *when all goes well*" would be rewriting a requirement without saying so,
which ADR 0014 precisely forbids.

**Not in the framing: in the skeleton issue**, together with the tests that cite
them. The `replaced` state requires that no test designate the identifier;
switching them before rewriting those tests would make `check-requirements.sh`
fail on a contradiction that the registry exists precisely to prevent. They
therefore stay `implemented` in the diff of this framing, which is accurate: the
binary still honours them.

The "Arguments" section of the manual, which currently explains that nothing is
parsed, is rewritten accordingly.

## What is set aside, with its reason

| Set aside | Reason |
| :-------- | :----- |
| machine-readable output (`--format json`) | nothing would consume it today; its shape must be dictated by a real caller, hence by phase 13 |
| reading SubViewer, SSA, ASS | the core does not read them; that is phase 9 |
| encodings other than UTF-8 | the core refuses everything else, deliberately (phase 9) |
| a subcommand that converts *and* transforms | would make the errors of one indistinguishable from those of the other |
| undo from the CLI | the history exists, but a process that terminates has nothing to undo |
| colour in the output | it would take `--color`, terminal detection, and an output that stops being stable byte for byte — yet the end-to-end tests compare it, and `make manual` copies it into the manual |
| a progress bar | same reason, and files are processed in a few tens of milliseconds |

## Tests

**End to end, through the real binary**, on the phase 3 harness: every
subcommand, every exit code, every usage refusal.

**The two outputs are checked separately.** The harness already separates them —
`CliRun::output` and `CliRun::errors` — so nothing needs building; what is
required here is that the tests use them. Conflating them would make invisible
the most likely fault of this phase: a narration line slipping onto standard
output, undetectable as long as one reads a terminal where both arrive mixed.

**The nesting of the levels is proven by comparison**, and not by rewriting the
expected output at each notch: **every line of level *n* is found, identical, in
level *n+1***, the added lines being allowed to interleave. Writing the same
expected output three times by hand would let the three diverge without anything
noticing.

**The fixtures are fabricated**, and deliberately cover what breaks: one file
per line ending, with and without a BOM, a file with mixed line endings, an
empty file, a file whose order is broken in each of the two ways, shifts to the
millisecond, a `25 → 23.976` conversion whose factor is not exact in decimal.

**No test compares two implementations.** What is expected is written in the
test, not produced by another program.

## Requirements

The phase's requirements, as the registry carries them today. They enter in the
`planned` state and only leave it once cited by a test.

**Four rows do not date from the framing**, and the phase 7 review brought them
back here because nothing did: the three original `CLI-VERSION` ones, written
for the skeleton before the phase was framed and replaced since, and
`CLI-OUTPUT-06`, born during the phase with the third verbosity level.
`CLI-INSPECT-03` is struck out in the same move — it was in the registry and not
here.

| ID | Requirement |
| :- | :---------- |
| ~~`CLI-VERSION-01`~~ | the invocation with no argument writes `subedit <version>` on standard output — replaced by `CLI-USAGE-01` |
| ~~`CLI-VERSION-02`~~ | nothing is written on error output — replaced by `CLI-VERSION-05` |
| ~~`CLI-VERSION-03`~~ | every argument is ignored, exit code `0` — replaced by `CLI-USAGE-02` |
| `CLI-VERSION-04` | `--version` writes `subedit <version>` on standard output, code `0` |
| `CLI-VERSION-05` | an invocation that succeeds writes nothing on error output |
| `CLI-USAGE-01` | with no argument, the help is written on standard output, code `0` |
| `CLI-USAGE-02` | an unknown option or an invalid value gives code `1`, without writing anything on standard output |
| `CLI-USAGE-03` | a usage error is detected before any processing |
| `CLI-USAGE-04` | `--quiet` and `-v` together give a usage error |
| `CLI-OUTPUT-01` | only the result goes on standard output, the narration on error output |
| `CLI-OUTPUT-02` | `--quiet` suppresses all narration, and lets errors through |
| `CLI-OUTPUT-03` | each level contains the previous one, word for word |
| `CLI-OUTPUT-04` | the default level writes one line per processed file |
| `CLI-OUTPUT-05` | the summary only appears from two input files onwards |
| `CLI-OUTPUT-06` | level 3 names each reading diagnostic, on every subcommand |
| `CLI-BATCH-01` | several files are processed independently, failures named |
| `CLI-BATCH-02` | all failing gives code `2`, a partial failure code `3` |
| `CLI-INSPECT-01` | format, encoding, BOM, line endings, count and span are reported |
| `CLI-INSPECT-02` | mixed line endings are reported with their line |
| ~~`CLI-INSPECT-03`~~ | `--order-report` returns both readings of disorder — abandoned: the corpus did not decide between the two readings |
| `CLI-CONVERT-01` | `--to` produces the requested format |
| `CLI-CONVERT-02` | line endings and BOM are kept by default, adjustable otherwise |
| `CLI-CONVERT-03` | without an explicit destination, nothing is written and the code is `1` |
| `CLI-SHIFT-01` | `--by` shifts every position, in both directions |
| `CLI-SHIFT-02` | a shift making a position negative is refused, naming the subtitle |
| `CLI-TRANSFORM-01` | two reference points transform the positions, the others follow |
| `CLI-TRANSFORM-02` | two coinciding indices are refused |
| `CLI-FRAMERATE-01` | `--from`/`--to` converts the positions |
| `CLI-FRAMERATE-02` | a zero or negative frame rate is refused |

## Split into issues

ADR 0016 does not appear here: it is written **at framing time**, with this
spec. A dependency decision taken at the same time as the code that consumes it
is no longer a decision, it is a justification after the fact.

| Issue | Subject | Depends on |
| :---- | :------ | :--------- |
| Issue | Subject | Depends on |
| :---- | :------ | :--------- |
| [#61](https://github.com/Guyot-Bertrand/sub-edit/issues/61) | skeleton — dispatch, `--version`, `--help`, exit codes, batch, verbosity, split of the two outputs — **and `inspect`** | — |
| [#62](https://github.com/Guyot-Bertrand/sub-edit/issues/62) | the second reading of disorder in the core | — |
| [#63](https://github.com/Guyot-Bertrand/sub-edit/issues/63) | `--order-report` on `inspect` | #61, #62 |
| [#64](https://github.com/Guyot-Bertrand/sub-edit/issues/64) | `convert`, line endings and BOM, **and the destination** | #61 |
| [#65](https://github.com/Guyot-Bertrand/sub-edit/issues/65) | `shift`, **and the time grammar** | #61 |
| [#66](https://github.com/Guyot-Bertrand/sub-edit/issues/66) | `transform`, **and the index grammar** | #61 |
| [#67](https://github.com/Guyot-Bertrand/sub-edit/issues/67) | `framerate` | #61 |
| [#68](https://github.com/Guyot-Bertrand/sub-edit/issues/68) | end-of-phase review | all |

**`inspect` is in the skeleton, and the common grammar is not.** That was not
the initial split; it was corrected when tackling it, for a reason worth writing
down.

A skeleton without a subcommand has **nothing reachable**: no file list, hence
no batch; no processing, hence nothing to narrate at levels 1 to 3; and codes
`2` and `3` cannot occur. The first issue would have promised four exit codes
and four levels while proving only two and one. `inspect` is the simplest of the
exercisers — it writes no file — and it makes the rest reachable.

Symmetrically, the grammar of time, indices and destination goes down into the
issues that use it. Writing it in the skeleton would leave it without a caller,
hence without proof — the same defect, the other way round.

`#62` does not touch the CLI and can be done in parallel with `#61`. The four
subcommands that write — `#64` to `#67` — are independent of one another once
the skeleton is in place.

## Open points

| Point | Trigger |
| :---- | :------ |
| which of the two readings of disorder to keep | phase 5, which will display the same set; the `--order-report` option then disappears |
| shape of a machine-readable output | phase 13, when a real caller needs it |
| validating operations against the video's duration | phase 6, which associates a video with the project — the detail is written there, in the roadmap |

On this last point, what this phase can and cannot do deserves to be said here,
since it was while writing it that we saw it: **only one bound can be checked,
that of zero.** A negative shift that is too large makes a position negative,
and refusing it is possible. The upper bound — not pushing subtitles past the
end of the film — does not exist for the core, which does not know where the
film ends.

**Corrected at review:** this sentence only spoke of `shift`. `transform` is
concerned in the same way — a subtitle placed before the first reference point
can be pushed below zero — and carries the same refusal, with the same message.
`framerate` is not: the factor is strictly positive, so a positive position
stays positive. The two other bounds, for their part, remain out of reach.

## Completion criteria

- [ ] The five subcommands exist and are documented in the manual
- [ ] Every requirement of the table above is `implemented` and cited by a test
- [ ] The four exit codes are proven
- [ ] The four verbosity levels are proven, nesting included
- [ ] No narration slips onto standard output
- [ ] The replaced requirements carry their new identifier
- [ ] The CLI11 ADR is written
- [ ] The benchmarks are rerun and the measurements recorded
- [ ] The end-of-phase review is done
