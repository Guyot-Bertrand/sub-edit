# `framerate`

> **This is not [`snap`](snap.md).** This command **scales the
> file**: it is for when the timing is wrong and drifts by several
> seconds by the end of the film. `snap` puts the timestamps back on the nearest
> grid without re-timing anything, and is for when the timing is already
> right. Both have the same form, one frame rate or two, and getting them mixed up causes
> no error.

```
subedit-cli framerate --from <fps> --to <fps>
                      (--output FILE | --output-dir DIR | --in-place)
                      [--dry-run]
                      [--recursive]
                      <file>...
```

Re-times a file timed for one frame rate to another. It is
the operation a subtitle file calls for when it was picked up for a copy that
is not the one of the film one has — a PAL DVD against an NTSC transfer, for
example.

The format of the file read is **kept**: changing the format is the work of
[`convert`](convert.md).

<!-- exemple: subedit-cli framerate --help -->
```console
$ subedit-cli framerate --help
Re-time a file mastered at one frame rate for another
Usage: subedit-cli framerate [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to re-time

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --from TEXT REQUIRED        Frame rate the file is timed at: 25, 23.976
  --to TEXT REQUIRED          Frame rate to time it for: 24, 29.97
  --output TEXT               File to write, for a single input
  --output-dir TEXT           Directory to write into
  --in-place                  Write back over the inputs
  --dry-run                   Work out and say what would be written, and write nothing
```

## Arguments and options

| Option | Required | Values | Default |
| :----- | :----- | :------ | :----- |
| `<file>...` | yes | one or more paths | — |
| `--recursive`, `-r` | no | a flag | off — see [Processing a tree](lots.md) |
| `--from` | **yes** | a frame rate, see below | — |
| `--to` | **yes** | a frame rate, see below | — |
| `--output` / `--output-dir` / `--in-place` | **one of the three**, except with `--dry-run` | see [Invocation](invocation.md#the-destination) | — |
| `--dry-run` | no | a flag: computes and says, writes nothing — see [Seeing before writing](invocation.md#seeing-before-writing) | off |

`--from` is the frame rate the file **is** timed for, `--to` the one
it is wanted for. Swapping them shifts the file the
other way — around seven seconds on a feature film between 25 and
23.976 — and nothing flags it before the film is watched.

## Writing a frame rate

In frames per second, decimals included, with the point as separator:

```
25      24      23.976      29.97      60
```

**A zero or negative frame rate is refused**: it is not a frame rate.

### The three NTSC frame rates cannot be written exactly

`23.976`, `29.97` and `59.94` are **labels**. The frame rates they
name are `24000/1001`, `30000/1001` and `60000/1001`, none of which has
a finite decimal writing. The tool recognises them as such:

| What is written | What it names |
| :------------- | :------------------ |
| `23.976` | `24000/1001` — cinema transferred to NTSC video |
| `24` | `24` — cinema |
| `25` | `25` — PAL and SECAM video |
| `29.97` | `30000/1001` — NTSC video |
| `30`, `50`, `60` | themselves |
| `59.94` | `60000/1001` |

The recognition is about **the value and not the letters**: `23.9760`
names the same frame rate as `23.976`.

Any other decimal is taken literally — `23.9` is nobody's
standard, there is nothing to read into it. That is also what makes the table above
necessary: without it, `23.976` would be `23976/1000`, and the command line
would say something other than the window for the same words.

## What the positions become

```
t′ = round( t × (from / to) )
```

A film mastered at 23.976 and played at 25 frames per second runs faster,
so its subtitles arrive earlier; the other way, they arrive
later.

**The factor is an exact rational, and the rounding happens only once.** The
path that looks equivalent — converting the position to a frame number at the
input rate, then back to milliseconds at the output rate — is
not: it quantises each position on the input grid and is wrong by up to
half a frame, 21 ms measured. See
[ADR 0013](../../adr/0013-mise-a-l-echelle-exacte-des-positions.md).

In practice, from 25 to 23.976, the factor is `1001/960`:

| Position read | Written | What going through the frames would give |
| -----------: | -----: | -----------------------------------------: |
| `1 010 ms` | `1 053 ms` | `1 043 ms` |
| `3 600 017 ms` | `3 753 768 ms` | `3 753 750 ms` |

<!-- exemple: printf '1\n00:00:01,010 --> 00:00:02,020\nUn.\n\n' > a.srt; subedit-cli framerate --from 25 --to 23.976 --output b.srt a.srt; cat b.srt -->
```console
$ printf '1\n00:00:01,010 --> 00:00:02,020\nUn.\n\n' > a.srt; subedit-cli framerate --from 25 --to 23.976 --output b.srt a.srt; cat b.srt
a.srt: 1 subtitle retimed from 25 to 24000/1001 fps -> b.srt
1
00:00:01,053 --> 00:00:02,106
Un.
```

## Output

**Standard output** — nothing: the result is the file written. With `--dry-run`, nothing either: there is no file, and the narration line says so — see [Seeing before writing](invocation.md#seeing-before-writing).

| Level | What is added on the error output |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles retimed from 25 to 24000/1001 fps -> <destination>` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — the format, the encoding, the mark and the line endings of the file read, put back as they were |
| 3 | `<path>: N bytes read, M written`, then **every reading diagnostic** — see [Invocation](invocation.md#reading-diagnostics) |

**The narration names the exact frame rate, not the label typed.** Writing
`23.976` on that line would report a conversion that did not happen; it is
the one place where one sees which of the two was used. A frame rate a
decimal writes exactly is shown as a decimal there, the others as a fraction.

### In JSON

With [`--format json`](invocation.md#output-a-script-can-read), the object of each file carries `counts.subtitles` (every subtitle of the file). The full table is [Invocation](invocation.md#what-each-subcommand-puts-in-counts)'s.

## Exit codes

Those of the tool: `0` if every file was written, `2` if none, `3` if
only some, `1` on a usage error.

No refusal depends on the content of a file: two frame rates always define
a factor. It is the only one of the subcommands that write all of whose
refusals come before the first reading.

<!-- exemple: printf '1\n00:00:01,010 --> 00:00:02,020\nUn.\n\n' > a.srt; subedit-cli framerate --from 0 --to 25 --output b.srt a.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:01,010 --> 00:00:02,020\nUn.\n\n' > a.srt; subedit-cli framerate --from 0 --to 25 --output b.srt a.srt; echo "code=$?"
"0" is not a frame rate: a frame rate must be strictly positive; nothing runs at that speed
code=1
```

## Errors

| What triggers it | Message |
| :------------------ | :------ |
| zero or negative frame rate | `"…" is not a frame rate: a frame rate must be strictly positive; nothing runs at that speed` |
| decimal comma | `"…" is not a frame rate: use a decimal point and not a comma, in every language` |
| text that is not a number | `"…" is not a frame rate: expected frames per second, like 25 or 23.976` |
| frame rate out of all proportion | `"…" is not a frame rate: no video runs at that many frames per second` |

**Nothing is written without one of the three destination options**, and their
errors are common to the subcommands that write:
see [Invocation](invocation.md#the-destination).
