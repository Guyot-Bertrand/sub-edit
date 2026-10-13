# `shift`

```
subedit-cli shift (--by <time> | --to-grid)
                  (--output FILE | --output-dir DIR | --in-place)
                  [--dry-run]
                  [--recursive]
                  <file>...
```

Moves **every** position of the file by the same amount, one way or
the other. No subtitle stays on screen any longer or shorter:
that is what makes a shift a shift.

<!-- exemple: subedit-cli shift --help -->
```console
$ subedit-cli shift --help
Move every position of a file by a fixed amount
Usage: subedit-cli shift [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to shift

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --by TEXT                   Amount to move by: 2.999, -7.001, or 00:00:07.001
  --to-grid                   Move by the amount that puts the positions back on their frame grid
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
| `--by` | **one of the two** | a signed duration, see below | — |
| `--to-grid` | **one of the two** | flag: the amount is measured, see below | — |
| `--output` / `--output-dir` / `--in-place` | **one of the three**, except with `--dry-run` | see [Invocation](invocation.md#the-destination) | — |
| `--dry-run` | no | a flag: computes and says, writes nothing — see [Seeing before writing](invocation.md#seeing-before-writing) | off |

The format of the file read is **kept**: changing the format is the work of
[`convert`](convert.md).

## `--to-grid`: the amount measured rather than given

A file whose positions are on a grid of frames **give or take a
constant** was shifted, and that constant can be measured on the positions themselves.
`--to-grid` measures it and applies it, without anyone having to know it.

The amount is **specific to each file**: two files shifted off the same
grid by two different quantities each come back by their own. It is
written in the narration, with the frame rate found.

**The path is the shortest.** Past half a frame, the nearest grid line
is the next one: the correction goes forward rather than moving back by
almost a whole frame.

<!-- exemple: k=0; while [ $k -lt 12 ]; do s=$((1000 + k*47000 + (k%3)*40 + 7)); e=$((s+2000)); k=$((k+1)); printf '%d\n%02d:%02d:%02d,%03d --> %02d:%02d:%02d,%03d\nRéplique %d.\n\n' $k $((s/3600000)) $((s/60000%60)) $((s/1000%60)) $((s%1000)) $((e/3600000)) $((e/60000%60)) $((e/1000%60)) $((e%1000)) $k; done > decale.srt; subedit-cli shift --to-grid --output recale.srt decale.srt -->
```console
$ k=0; while [ $k -lt 12 ]; do s=$((1000 + k*47000 + (k%3)*40 + 7)); e=$((s+2000)); k=$((k+1)); printf '%d\n%02d:%02d:%02d,%03d --> %02d:%02d:%02d,%03d\nRéplique %d.\n\n' $k $((s/3600000)) $((s/60000%60)) $((s/1000%60)) $((s%1000)) $((e/3600000)) $((e/60000%60)) $((e/1000%60)) $((e%1000)) $k; done > decale.srt; subedit-cli shift --to-grid --output recale.srt decale.srt
decale.srt: 12 subtitles shifted by -0.007 s onto their 25 fps grid -> recale.srt
```

**A file on no known grid is refused, and told why.** There
is nothing to rejoin, and a phase measured on noise would move the file
at random. See [`inspect`](inspect.md#what-frame-rate-grid-reports) to
know what a file gives before trying the operation.

`--by` and `--to-grid` both say by how much to shift: giving them together
is a usage error, and so is giving neither.

**`--to-grid` is not [`snap`](snap.md).** It shifts the whole file
by the same quantity and keeps the relative timing **exactly**; `snap`
moves each position independently and absorbs along the way those that had
been corrected by hand. On a clean file the two give the same
result; on a partial file, they do not.

## Writing a duration

Two forms, and only two:

| Form | Examples |
| :---- | :------- |
| seconds | `30`, `2.999`, `-7.001`, `+1.5` |
| timestamp | `00:00:07.001`, `01:39:37.040`, `-00:00:07.001` |

Both say the same thing: `--by 7.001` and `--by 00:00:07.001` produce
the same file.

**The decimal separator is the point**, the command line speaking English.
A comma is refused rather than interpreted: half the world writes `7,001`
for seven thousand and one, and reading it as seven seconds would be a guess —
a wrong one for that half.

**The resolution is the millisecond.** Finer is refused rather than rounded
silently: a tacit rounding would give a result that matches nothing of what
was asked for.

<!-- exemple: printf '1\n00:00:10,000 --> 00:00:12,000\nBonjour.\n\n' > a.srt; subedit-cli shift --by -7.001 --output b.srt a.srt; cat b.srt -->
```console
$ printf '1\n00:00:10,000 --> 00:00:12,000\nBonjour.\n\n' > a.srt; subedit-cli shift --by -7.001 --output b.srt a.srt; cat b.srt
a.srt: 1 subtitle shifted by -7.001 s -> b.srt
1
00:00:02,999 --> 00:00:04,999
Bonjour.
```

## A shift before the origin is refused

Going back past zero gives code `2` and **writes nothing**. The message names
the subtitle that could not take the shift, which says at the same time
how far one can go.

The reason is not a principle: a position before the origin is written
`-00:00:01,000`, a form none of the nine formats defines and no reader
accepts. The core does allow such positions — refusing them would make an
editing operation a special case — but a file is something other than a
model in memory.

<!-- exemple: printf '1\n00:00:01,000 --> 00:00:03,000\nBonjour.\n\n' > a.srt; subedit-cli shift --by -7.001 --output b.srt a.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\nBonjour.\n\n' > a.srt; subedit-cli shift --by -7.001 --output b.srt a.srt; echo "code=$?"
a.srt: subtitle 1 would start before the origin, which no subtitle file can hold
code=2
```

## Output

**Standard output** — nothing: the result is the file written. With `--dry-run`, nothing either: there is no file, and the narration line says so — see [Seeing before writing](invocation.md#seeing-before-writing).

| Level | What is added on the error output |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles shifted by 2.999 s -> <destination>` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — the format, the encoding, the mark and the line endings of the file read, put back as they were |
| 3 | `<path>: N bytes read, M written`, then **every reading diagnostic** — see [Invocation](invocation.md#reading-diagnostics) |

### In JSON

With [`--format json`](invocation.md#output-a-script-can-read), the object of each file carries `counts.subtitles` (every subtitle of the file) and `counts.shifted_by_ms` (the shift, in whole milliseconds, **signed**). The full table is [Invocation](invocation.md#what-each-subcommand-puts-in-counts)'s.

## Exit codes

Those of the tool: `0` if every file was written, `2` if none, `3` if
only some, `1` on a usage error.

## Errors

| What triggers it | Message |
| :------------------ | :------ |
| decimal comma | `"…" is not a time: use a decimal point and not a comma, in every language` |
| finer than the millisecond | `"…" is not a time: positions are held to the millisecond, no finer` |
| text that is not a duration | `"…" is not a time: expected seconds like 2.999, or a timestamp like 00:00:07.001` |
| shift before the origin | `<path>: subtitle N would start before the origin, which no subtitle file can hold` |

**Nothing is written without one of the three destination options**, and their
errors are common to the subcommands that write:
see [Invocation](invocation.md#the-destination).
