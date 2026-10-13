# `snap`

```
subedit-cli snap --rate <rate>
                 (--output FILE | --output-dir DIR | --in-place)
                 [--dry-run]
                 [--recursive]
                 <file>...
```

Puts **every timestamp** — start and end — on the nearest frame of the
frame rate given. Nothing is re-timed: each position moves by half a frame at
most.

## `snap` is not `framerate`, and mixing them up does not show

The two commands have the same form — files, a destination, a frame rate — and do opposite things.

| | [`framerate`](framerate.md) | `snap` |
| :--- | :-------------------------- | :----- |
| what it computes | `t' = t x input rate / output rate` | the nearest frame, on the grid of `--rate` |
| by how much it moves | in proportion to time — **several seconds** on a feature film | half a frame at most, **everywhere** |
| for which file | a file whose **timing is wrong** | a file whose timing is **right** and whose grid is wrong |

**The use case of `snap`**, spelled out: a film at 25 frames per
second, a subtitle file written on a grid at 24, lines already
more or less in their place to within a few milliseconds. There is nothing to re-time
— only to put the timestamps back on the right frames.

**The use case of `framerate`** is the other one: a file timed for a master at
25 that one wants to play on a copy at 23.976, and whose timing drifts by
several seconds by the end of the film.

Taking one for the other **causes no error**: both
accept the file and write a result. The narration line is what
tells them apart — `snap` always says by how much it moved the positions.

<!-- exemple: subedit-cli snap --help -->
```console
$ subedit-cli snap --help
Move every position onto the nearest frame of a frame rate (see framerate)
Usage: subedit-cli snap [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to align

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --rate TEXT REQUIRED        Frame rate to align on: 25, 23.976
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
| `--rate` | **yes** | a frame rate in frames per second, written as for [`framerate`](framerate.md#writing-a-frame-rate) | — |
| `--output` / `--output-dir` / `--in-place` | **one of the three**, except with `--dry-run` | see [Invocation](invocation.md#the-destination) | — |
| `--dry-run` | no | a flag: computes and says, writes nothing — see [Seeing before writing](invocation.md#seeing-before-writing) | off |

`--rate` accepts **any valid frame rate**, and not only the eight
standard ones: it is the user's choice, as for `framerate`. A zero or negative
frame rate is refused.

The format of the file read is **kept**: changing the format is the work of
[`convert`](convert.md).

## Output

**Standard output** — nothing: the result is the file written. With `--dry-run`, nothing either: there is no file, and the narration line says so — see [Seeing before writing](invocation.md#seeing-before-writing).

| Level | What is added on the error output |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles aligned on 25 fps, M positions moved, by at most 20 ms -> <destination>` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — the format, the encoding, the mark and the line endings of the file read, put back as they were |
| 3 | `<path>: N bytes read, M written`, then **every reading diagnostic** — see [Invocation](invocation.md#reading-diagnostics) |

**The two numbers are the point of the line.** Half a frame at 25 frames per
second is twenty milliseconds, and nothing larger can be written there. Whoever
wanted `framerate` and typed `snap` reads "by at most 20 ms" where they expected
seconds, and knows straight away.

## An example

A line at `00:00:01,010`, aligned on 25 frames per second — so on a
multiple of forty milliseconds:

<!-- exemple: printf '1\n00:00:01,010 --> 00:00:03,020\nUn.\n\n' > a.srt; subedit-cli snap --rate 25 --output b.srt a.srt; cat b.srt -->
```console
$ printf '1\n00:00:01,010 --> 00:00:03,020\nUn.\n\n' > a.srt; subedit-cli snap --rate 25 --output b.srt a.srt; cat b.srt
a.srt: 1 subtitle aligned on 25 fps, 2 positions moved, by at most 20 ms -> b.srt
1
00:00:01,000 --> 00:00:03,040
Un.
```

## Two properties

**Aligning twice changes nothing the second time.** A position already on a
frame stays there: it is the same operation applied to the same result.

**The order of the subtitles is kept.** Rounding to the nearest frame is
monotonic, so two starts at least one frame apart stay so. Closer
together, they can merge — which is an anomaly that
[`inspect`](inspect.md#what-anomalies-reports) knows how to name, and not an
inversion.

### In JSON

With [`--format json`](invocation.md#output-a-script-can-read), the object of each file carries `counts.subtitles`, `counts.moved` (the positions moved) and `counts.furthest_ms` (the largest move, in milliseconds). The full table is [Invocation](invocation.md#what-each-subcommand-puts-in-counts)'s.

## Exit codes

Those of the tool: `0` if every file was written, `2` if none, `3` if
only some, `1` on a usage error.

## Errors

Those of a frame rate, word for word those of [`framerate`](framerate.md#errors)
— both read the same value by the same path:

| What triggers it | Message |
| :------------------ | :------ |
| zero or negative frame rate | `"…" is not a frame rate: a frame rate must be strictly positive; nothing runs at that speed` |
| decimal comma | `"…" is not a frame rate: use a decimal point and not a comma, in every language` |
| text that is not a number | `"…" is not a frame rate: expected frames per second, like 25 or 23.976` |
| frame rate out of all proportion | `"…" is not a frame rate: no video runs at that many frames per second` |

**Nothing is written without one of the three destination options**, and their errors
are common to the subcommands that write:
see [Invocation](invocation.md#the-destination).
