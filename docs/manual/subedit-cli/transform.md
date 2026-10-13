# `transform`

```
subedit-cli transform --first <index>=<time> --last <index>=<time>
                      (--output FILE | --output-dir DIR | --in-place)
                      [--dry-run]
                      [--recursive]
                      <file>...
```

Corrects **every** position of the file from two points whose right value
is known. It is the operation a drifting subtitling calls for
— right at the start, wrong at the end — where [`shift`](shift.md), which moves everything by
the same amount, can do nothing.

The format of the file read is **kept**: changing the format is the work of
[`convert`](convert.md).

<!-- exemple: subedit-cli transform --help -->
```console
$ subedit-cli transform --help
Correct every position from two points known to be right
Usage: subedit-cli transform [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to transform

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --first TEXT REQUIRED       Earlier reference, as <index>=<time>: 1=00:00:01.000
  --last TEXT REQUIRED        Later reference, as <index>=<time>: 3=00:00:10.000
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
| `--first` | **yes** | a reference, `<index>=<time>` | — |
| `--last` | **yes** | a reference, `<index>=<time>` | — |
| `--output` / `--output-dir` / `--in-place` | **one of the three**, except with `--dry-run` | see [Invocation](invocation.md#the-destination) | — |
| `--dry-run` | no | a flag: computes and says, writes nothing — see [Seeing before writing](invocation.md#seeing-before-writing) | off |

There is no short form: these two options are only written out in
full.

## Writing a reference

A reference names a subtitle and says where its **start** must now fall:

```
<index>=<time>
```

**The index is counted from 1**, as it is shown and as a SubRip
file numbers it. `0` is refused, and so is an index past the last
subtitle.

**The time is written as that of [`shift`](shift.md)** — seconds or
timestamp, the decimal separator a point, the resolution the millisecond:

| Form | Examples |
| :---- | :------- |
| seconds | `10`, `2.999` |
| timestamp | `00:00:10.000`, `01:39:37.040` |

`--first 3=10` and `--first 3=00:00:10.000` say the same thing.

`--first` and `--last` name two points, not an interval. Their names say
the expected use — one reference at the start of the file, the other at the end — and not a
constraint: swapping them gives the same result, and **everything outside
the two references follows like what is between them**.

## What the other positions become

The two references set a scale factor, and every position follows it:

```
t′ = y₁ + (t − x₁) × r      with r = (y₂ − y₁) / (x₂ − x₁)
```

`x₁`, `x₂` are the current starts of the two subtitles named, `y₁`, `y₂` the
times asked for. The factor is held as an exact rational and **the rounding
happens only once**, to the millisecond; that is what makes the two
references fall **exactly** on the times asked for and not to within a thousandth
(see [ADR 0013](../../adr/0013-mise-a-l-echelle-exacte-des-positions.md)).

Each subtitle stays on screen **in proportion** to how long it stayed there:
the end follows the same scale as the start. A transform is not a
shift, and does not have its property.

<!-- exemple: printf '1\n00:00:01,000 --> 00:00:03,000\nUn.\n\n2\n00:00:05,001 --> 00:00:07,000\nDeux.\n\n3\n00:00:09,000 --> 00:00:11,000\nTrois.\n\n' > a.srt; subedit-cli transform --first 1=00:00:01.000 --last 3=00:00:10.000 --output b.srt a.srt; cat b.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\nUn.\n\n2\n00:00:05,001 --> 00:00:07,000\nDeux.\n\n3\n00:00:09,000 --> 00:00:11,000\nTrois.\n\n' > a.srt; subedit-cli transform --first 1=00:00:01.000 --last 3=00:00:10.000 --output b.srt a.srt; cat b.srt
a.srt: 3 subtitles transformed onto 1=00:00:01.000 and 3=00:00:10.000 -> b.srt
1
00:00:01,000 --> 00:00:03,250
Un.

2
00:00:05,501 --> 00:00:07,750
Deux.

3
00:00:10,000 --> 00:00:12,250
Trois.
```

The first reference does not move, the third goes from 9 s to 10 s: the factor is
`9/8`, and the second subtitle — which nobody named — goes from
`5,001` to `5,501`, that is `1000 + 4001 × 9/8` rounded once.

## Two references that are not

| Situation | When it is seen | Code |
| :-------- | :------------- | :--- |
| `--first` and `--last` name the same index | before any reading | `1` |
| an index is past the last subtitle | when the file is opened | `2` or `3` |
| the two subtitles named start at the same moment | when the file is opened | `2` or `3` |

All three are refused for the same reason: `x₂ − x₁` is zero, and two
coinciding points define no line. Returning a division by zero dressed up as a
result would be worse than refusing.

The first case is a **usage error** — the two options are at fault whatever
the files, so a single message is due, not one per file. The
other two depend on the file opened, and therefore count as a processing
failure.

<!-- exemple: printf '1\n00:00:01,000 --> 00:00:03,000\nUn.\n\n' > a.srt; subedit-cli transform --first 2=1.000 --last 2=4.000 --output b.srt a.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\nUn.\n\n' > a.srt; subedit-cli transform --first 2=1.000 --last 2=4.000 --output b.srt a.srt; echo "code=$?"
--first and --last both name subtitle 2, and two references on one subtitle define no transform
code=1
```

An index out of bounds is refused **naming the bound**, which says at the
same time what could have been written:

<!-- exemple: printf '1\n00:00:01,000 --> 00:00:03,000\nUn.\n\n2\n00:00:05,000 --> 00:00:07,000\nDeux.\n\n' > a.srt; subedit-cli transform --first 1=1.000 --last 9=10.000 --output b.srt a.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\nUn.\n\n2\n00:00:05,000 --> 00:00:07,000\nDeux.\n\n' > a.srt; subedit-cli transform --first 1=1.000 --last 9=10.000 --output b.srt a.srt; echo "code=$?"
a.srt: subtitle 9 is past the end: the file holds 2 subtitles
code=2
```

## A position before the origin is refused

As for [`shift`](shift.md), and for the same reason: a negative position
is written `-00:00:01,000`, a form none of the nine formats defines and no
reader accepts. The case arises here when a subtitle lies **before
the first reference** and the scale pushes it past zero. Nothing is
written, the code is `2`, and the message names the subtitle at fault.

## Output

**Standard output** — nothing: the result is the file written. With `--dry-run`, nothing either: there is no file, and the narration line says so — see [Seeing before writing](invocation.md#seeing-before-writing).

| Level | What is added on the error output |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles transformed onto 1=00:00:01.000 and 3=00:00:10.000 -> <destination>` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — the format, the encoding, the mark and the line endings of the file read, put back as they were |
| 3 | `<path>: N bytes read, M written`, then **every reading diagnostic** — see [Invocation](invocation.md#reading-diagnostics) |

The references are written back with the decimal point, however they were typed.

### In JSON

With [`--format json`](invocation.md#output-a-script-can-read), the object of each file carries `counts.subtitles` (every subtitle of the file). The full table is [Invocation](invocation.md#what-each-subcommand-puts-in-counts)'s.

## Exit codes

Those of the tool: `0` if every file was written, `2` if none, `3` if
only some, `1` on a usage error.

## Errors

| What triggers it | Message |
| :------------------ | :------ |
| reference without `=` | `"…" is not a reference: write it <index>=<time>, as in 3=00:00:10.000` |
| reference missing one half | `"…" is missing one of its two halves: write it <index>=<time>, as in 3=00:00:10.000` |
| index `0` | `"0" is not a subtitle number: subtitles are counted from 1, as the file shows them` |
| index that is not a whole number | `"…" is not a subtitle number: expected a whole number, counted from 1 as the file shows them` |
| invalid time | those of [`shift`](shift.md) |
| `--first` and `--last` on the same index | `--first and --last both name subtitle N, and two references on one subtitle define no transform` |
| index out of bounds | `<path>: subtitle N is past the end: the file holds M subtitles` |
| two subtitles starting together | `<path>: subtitles N and M start at the same moment, so they define no transform` |
| position before the origin | `<path>: subtitle N would land before the origin, which no subtitle file can hold` |

**Nothing is written without one of the three destination options**, and their
errors are common to the subcommands that write:
see [Invocation](invocation.md#the-destination).
