# `adjust`

```
subedit-cli adjust [--speed CPS|off] [--shorten] [--no-lengthen]
                   [--minimum TIME|off] [--maximum TIME|off] [--gap TIME|off]
                   [--range N-M|N-]
                   (--output FILE | --output-dir DIR | --in-place)
                   [--dry-run]
                   [--recursive]
                   <file>...
```

Adjusts the **duration** of each subtitle to a reading speed and to limits: a
subtitle too short to be read is lengthened, a subtitle that leaves the screen
too late can be shortened, and two subtitles do not overlap. **Only the
end moves**: moving the start would move the subtitle, which is the work of
[`shift`](shift.md). It is the `Adjust Durations…` operation of the
[window](../subedit-gui/operations.md#adjust-durations), with the same defaults.

## The four constraints, and their order

They apply **in this order**, each one setting the end, the last one winning:

1. **the reading speed** — the duration the text needs, at so many characters per
   second (tags do not count, line breaks do);
2. **the minimum** — a duration below it is brought up to the minimum;
3. **the maximum** — a duration above it is brought down to the maximum;
4. **the gap** — the time left before the next subtitle. It comes last and
   **it wins**: two subtitles shown together are a fault the viewer sees,
   a subtitle a little short is not.

The last subtitle of the file has no next one, and so no gap to keep. The next one
of a subtitle is the one in the file, **whether chosen or not** by `--range`.

| Constraint | Option | Default | Switching it off |
| :--------- | :----- | :----- | :------- |
| speed | `--speed CPS` | **15** characters per second | `--speed off` |
| direction of the speed | `--shorten`, `--no-lengthen` | it only **lengthens** | — |
| minimum | `--minimum TIME` | **1.5 s** | `--minimum off` |
| maximum | `--maximum TIME` | **none** | `--maximum off` |
| gap | `--gap TIME` | **0** | `--gap off` |

These are the settings of Gaupol and of the window's first opening. **`off` switches a
constraint off: it is no longer checked.** That is not setting it to zero — a minimum of `0`
is a minimum of zero, a gap of `0` forbids any overlap.

## What the settings file supplies

**The defaults above are the built-in ones.** By default `adjust` reads the operation settings of
the settings file — the values `Adjust Durations…` remembers in the window, or that
[`config set`](config.md) wrote — and they take the place of the built-in defaults. **An option
given on the line always wins**, then the file, then the built-in default. See
[The settings file](invocation.md#the-settings-file) for where the file is, `--config` and
`--no-config`.

| Key | What it supplies | Option that overrides it |
| :-- | :--------------- | :----------------------- |
| `duration-adjust.speed` | the reading speed, in characters per second | `--speed CPS` |
| `duration-adjust.lengthen`, `duration-adjust.shorten` | which way the speed may move an end; both `false` switch the speed off | `--no-lengthen`, `--shorten`, `--speed off`; giving `--speed CPS` switches a speed the file turned off back on, lengthening |
| `duration-adjust.minimum-enabled`, `duration-adjust.minimum-ms` | the shortest duration, or none | `--minimum TIME`, `--minimum off` |
| `duration-adjust.maximum-enabled`, `duration-adjust.maximum-ms` | the longest duration, or none | `--maximum TIME`, `--maximum off` |
| `duration-adjust.gap-enabled`, `duration-adjust.gap-ms` | the least gap, or none | `--gap TIME`, `--gap off` |

The flags only move the speed's directions **from what the file says**: `--shorten` adds
shortening, `--no-lengthen` takes lengthening away. `--maximum off` drops a maximum the file
switches on. Shortening the file switches on has no flag to undo it on the line: `--speed off`
or `--no-config` drops it. With `--format json`, `constraints` says the settings
really used, wherever they came from, and `settings` the file that was read:

<!-- example: printf '1\n00:00:01,000 --> 00:00:01,200\nUn.\n\n' > a.srt; subedit-cli --config mine/settings.conf config set duration-adjust.speed 12 2>/dev/null; subedit-cli --config mine/settings.conf --format json adjust --dry-run a.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:01,200\nUn.\n\n' > a.srt; subedit-cli --config mine/settings.conf config set duration-adjust.speed 12 2>/dev/null; subedit-cli --config mine/settings.conf --format json adjust --dry-run a.srt
settings: read mine/settings.conf
a.srt: adjusted the durations of 1 subtitle (dry run, nothing written)
{"schema":1,"command":"adjust","file":"a.srt","ok":true,"dry_run":true,"destination":null,"counts":{"subtitles":1,"adjusted":1,"sacrificed":{"speed":0,"minimum":0,"gap":0}},"constraints":{"speed":{"cps":"12","lengthen":true,"shorten":false},"minimum_ms":1500,"maximum_ms":null,"gap_ms":0},"settings":"mine/settings.conf","warnings":[]}
```

The other examples on this page are run with `--no-config`, so that they say what the built-in
defaults do, whatever the settings of the machine that runs them.

<!-- example: subedit-cli adjust --help -->
```console
$ subedit-cli adjust --help
Bring the duration of every subtitle within a reading speed and limits
Usage: subedit-cli adjust [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to adjust

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --speed CPS|off             Reading speed in characters per second, or off (default 15)
  --shorten                   Let the reading speed bring an end earlier
  --no-lengthen               Do not let the reading speed move an end later
  --minimum TIME|off          Shortest duration, or off (default 1.5)
  --maximum TIME|off          Longest duration, or off (default: none)
  --gap TIME|off              Least time left before the next subtitle, or off (default 0)
  --range N-M|N-              Act only on subtitles N to M, or N to the end
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
| `--speed` | no | a number of characters per second, **above zero** (`15`, `12.5`), or `off` | `15` |
| `--shorten` | no | a flag: the speed may also **bring** an end **earlier** | off |
| `--no-lengthen` | no | a flag: the speed no longer **moves** an end **later** | off |
| `--minimum` | no | a duration of zero or more, or `off` | `1.5` |
| `--maximum` | no | a duration **above zero**, or `off` | none |
| `--gap` | no | a duration of zero or more, or `off` | `0` |
| `--range` | no | `N-M`, or `N-` up to the end — see below | the whole file |
| `--output` / `--output-dir` / `--in-place` | **one of the three**, except with `--dry-run` | see [Invocation](invocation.md#the-destination) | — |
| `--dry-run` | no | a flag: computes and says, writes nothing — see [Seeing before writing](invocation.md#seeing-before-writing) | off |

A duration is written as for [`shift`](shift.md#writing-a-duration): `2.5`, `00:00:02.500`.

**Without any active constraint, `adjust` is refused** (code `1`, nothing written): there would
be nothing to adjust to, and a command that does nothing while saying it succeeded is the worst
answer. That is the case of `--speed off --minimum off --gap off` without a maximum, and of
`--no-lengthen` alone with the minimum and the gap switched off: a speed that moves no
end constrains nothing.

`--shorten` and `--no-lengthen` say which way the speed moves an end; they only make
sense with a speed, and **`--speed off` refuses them**.

### `--range`

`--range 3-7` limits the adjustment to subtitles 3 to 7, **counted from 1, bounds
included**; `--range 3-` goes up to the last one. A single subtitle is written `4-4`.

- A range **that ends before it starts**, a `0`, a sign or a lone number are
  refused as a usage error (code `1`).
- **It holds for every file of the batch**, which only makes sense if the files
  look alike. It is judged **against each file**: a range that goes past that file is
  refused **before it is touched**, naming the bound —
  `range 3-9 ends after the last subtitle: the file has 4` —, that file fails (code `2` or `3`, identifier
  `range-out-of-bounds`) and the others go on.
- The counts of the report only concern the subtitles chosen.

## What is sacrificed

Two constraints can contradict each other on the same subtitle: a minimum of 1.5 s does not fit
in the 0.8 s the next subtitle leaves. **The order decides, and the report says
what gave way** — which is what Gaupol does not do: it breaks them silently.

Each constraint is named only if **at least one subtitle did not get it**, once
the others are applied. The maximum has no count: it is applied before the gap, which can
only shorten, so it always holds. A minimum above the maximum gives way to the maximum and is
counted as a minimum.

<!-- example: printf '1\n00:00:00,000 --> 00:00:00,400\nabcdefghijklmnopqrstuvwxyz1234\n\n2\n00:00:10,000 --> 00:00:14,000\nabcdefghijklmno\n\n3\n00:00:14,200 --> 00:00:14,300\nabc\n\n4\n00:00:15,000 --> 00:00:15,100\nabcdef\n\n' > courts.srt; subedit-cli --no-config adjust --output ajuste.srt courts.srt; grep -e '-->' ajuste.srt -->
```console
$ printf '1\n00:00:00,000 --> 00:00:00,400\nabcdefghijklmnopqrstuvwxyz1234\n\n2\n00:00:10,000 --> 00:00:14,000\nabcdefghijklmno\n\n3\n00:00:14,200 --> 00:00:14,300\nabc\n\n4\n00:00:15,000 --> 00:00:15,100\nabcdef\n\n' > courts.srt; subedit-cli --no-config adjust --output ajuste.srt courts.srt; grep -e '-->' ajuste.srt
courts.srt: adjusted the durations of 3 subtitles; could not satisfy the minimum duration in 1 subtitle -> ajuste.srt
00:00:00,000 --> 00:00:02,000
00:00:10,000 --> 00:00:14,000
00:00:14,200 --> 00:00:15,000
00:00:15,000 --> 00:00:16,500
```

The third subtitle would like to end at 15.7 s to last 1.5 s, but the fourth starts at
15.0 s: **the gap wins**, it ends at 15.0 s, and the report names the minimum.

Switching the gap off lets the minimum win — and the two subtitles overlap:

<!-- example: printf '1\n00:00:00,000 --> 00:00:00,400\nabcdefghijklmnopqrstuvwxyz1234\n\n2\n00:00:10,000 --> 00:00:14,000\nabcdefghijklmno\n\n3\n00:00:14,200 --> 00:00:14,300\nabc\n\n4\n00:00:15,000 --> 00:00:15,100\nabcdef\n\n' > courts.srt; subedit-cli --no-config adjust --gap off --output libre.srt courts.srt; grep -e '-->' libre.srt -->
```console
$ printf '1\n00:00:00,000 --> 00:00:00,400\nabcdefghijklmnopqrstuvwxyz1234\n\n2\n00:00:10,000 --> 00:00:14,000\nabcdefghijklmno\n\n3\n00:00:14,200 --> 00:00:14,300\nabc\n\n4\n00:00:15,000 --> 00:00:15,100\nabcdef\n\n' > courts.srt; subedit-cli --no-config adjust --gap off --output libre.srt courts.srt; grep -e '-->' libre.srt
courts.srt: adjusted the durations of 3 subtitles -> libre.srt
00:00:00,000 --> 00:00:02,000
00:00:10,000 --> 00:00:14,000
00:00:14,200 --> 00:00:15,700
00:00:15,000 --> 00:00:16,500
```

And nothing is sacrificed nor adjusted when the file already fits within its limits:

<!-- example: printf '1\n00:00:00,000 --> 00:00:00,400\nabcdefghijklmnopqrstuvwxyz1234\n\n2\n00:00:10,000 --> 00:00:14,000\nabcdefghijklmno\n\n3\n00:00:14,200 --> 00:00:14,300\nabc\n\n4\n00:00:15,000 --> 00:00:15,100\nabcdef\n\n' > courts.srt; subedit-cli --no-config adjust --speed off --minimum off --output intact.srt courts.srt -->
```console
$ printf '1\n00:00:00,000 --> 00:00:00,400\nabcdefghijklmnopqrstuvwxyz1234\n\n2\n00:00:10,000 --> 00:00:14,000\nabcdefghijklmno\n\n3\n00:00:14,200 --> 00:00:14,300\nabc\n\n4\n00:00:15,000 --> 00:00:15,100\nabcdef\n\n' > courts.srt; subedit-cli --no-config adjust --speed off --minimum off --output intact.srt courts.srt
courts.srt: no duration to adjust -> intact.srt
```

The file is written as it is: **a destination given is a destination written**.

## Output

**Standard output** — nothing: the result is the file written.

| Level | What is added on the error output |
| :----- | :------------------------------------- |
| 1 | `<path>: adjusted the durations of N subtitles; could not satisfy …  -> <destination>`, or `<path>: no duration to adjust -> <destination>` — and, first, the settings file read — `settings: read <path>` — see [The settings file](invocation.md#the-settings-file) |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — the format, the encoding, the mark and the line endings of the file read, put back as they were |
| 3 | `<path>: N bytes read, M written`, then **every reading diagnostic** — see [Invocation](invocation.md#reading-diagnostics) |

The sentence is the window's: `N` is the number of subtitles whose end moved, and the
rest, absent when nothing gave way, lists what was not kept — `the reading speed`,
`the minimum duration`, `the gap` — with the number of subtitles concerned.

### In JSON

With [`--format json`](invocation.md#output-a-script-can-read), the object of each file
carries, besides the envelope:

| Key | Content |
| :-- | :------ |
| `counts.subtitles` | the subtitles **aimed at**: the whole file, or the range |
| `counts.adjusted` | those whose end moved |
| `counts.sacrificed.speed`, `.minimum`, `.gap` | the subtitles that still do not keep this constraint, once the others are applied, **including those whose end did not move** |
| `constraints.speed` | `{"cps", "lengthen", "shorten"}` — the characters per second **as a string** (`"15"`, `"12.5"`) — or `null` if switched off |
| `constraints.minimum_ms`, `.maximum_ms`, `.gap_ms` | the durations used, in **whole milliseconds**, or `null` if switched off |
| `settings` | the settings file that was read, or `null` when none was |

`constraints` says **the settings really used**, defaults included: two computations
about the same subtitles then compare without reading the command line.

<!-- example: printf '1\n00:00:00,000 --> 00:00:00,400\nabcdefghijklmnopqrstuvwxyz1234\n\n2\n00:00:10,000 --> 00:00:14,000\nabcdefghijklmno\n\n3\n00:00:14,200 --> 00:00:14,300\nabc\n\n4\n00:00:15,000 --> 00:00:15,100\nabcdef\n\n' > courts.srt; subedit-cli --no-config --format json adjust --dry-run --maximum 1.8 courts.srt -->
```console
$ printf '1\n00:00:00,000 --> 00:00:00,400\nabcdefghijklmnopqrstuvwxyz1234\n\n2\n00:00:10,000 --> 00:00:14,000\nabcdefghijklmno\n\n3\n00:00:14,200 --> 00:00:14,300\nabc\n\n4\n00:00:15,000 --> 00:00:15,100\nabcdef\n\n' > courts.srt; subedit-cli --no-config --format json adjust --dry-run --maximum 1.8 courts.srt
courts.srt: adjusted the durations of 4 subtitles; could not satisfy the reading speed in 1 subtitle, the minimum duration in 1 subtitle (dry run, nothing written)
{"schema":1,"command":"adjust","file":"courts.srt","ok":true,"dry_run":true,"destination":null,"counts":{"subtitles":4,"adjusted":4,"sacrificed":{"speed":1,"minimum":1,"gap":0}},"constraints":{"speed":{"cps":"15","lengthen":true,"shorten":false},"minimum_ms":1500,"maximum_ms":1800,"gap_ms":0},"settings":null,"warnings":[]}
```

## Exit codes

Those of the tool: `0` if every file was processed, `2` if none, `3` if only
some, `1` on a usage error. A file that already fits within its limits is processed.

## Errors

| What triggers it | Message |
| :------------------ | :------ |
| a file named by `--config` that does not exist — the user's own file, missing, gives the defaults | `settings file not found: <FILE>` |
| no active constraint | `adjust has nothing to adjust to: every constraint is off, or the reading speed is told to move no end` |
| a speed that is not one | `--speed: "…" is not a reading speed: expected a number of characters per second above zero, like 15 or 12.5, or off` |
| `--shorten`/`--no-lengthen` with `--speed off` | `--shorten and --no-lengthen say which way the reading speed may move an end, and --speed off switches the reading speed off` |
| a duration that is not one | `--minimum: "…" is not a time: …` (likewise `--maximum`, `--gap`) |
| negative duration | `--minimum: "-1" is not a duration of zero or more` (likewise `--gap`) |
| zero or negative maximum | `--maximum: "0" is not a duration above zero` |
| badly written range | `--range: "…" is not a range: write it N-M, or N- to go to the end, counted from 1 and inclusive` |
| range that ends before it starts | `--range: "5-3" is not a range: it ends before it starts` |
| range outside the file | `<path>: range N-M starts after the last subtitle: the file has K`, or `… ends after …` |

The errors of the destination are common to the subcommands that write:
see [Invocation](invocation.md#the-destination).
