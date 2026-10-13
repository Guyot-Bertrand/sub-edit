# `repair`

```
subedit-cli repair (--output FILE | --output-dir DIR | --in-place)
                   [--minimum TIME] [--max-lines N] [--dashes]
                   [--dry-run] [--recursive]
                   <file>...
```

Does, in one go, what three commands do one after the other: [`join-repeats`](join-repeats.md), then
[`sort`](sort.md), then [`disjoint`](disjoint.md). **In that order, which is the only one that works**: the cutting reads
the order of the starts, and the repeated pieces of one text are better gone before either. On a file
that is badly segmented, out of order and overlapping, it is the command to run.

The settings are those of `disjoint`, which ends the chain; `--sort` does not exist here, since the sort is always done.

<!-- exemple: subedit-cli repair --help -->
```console
$ subedit-cli repair --help
Join the repeated subtitles, put them in order, then make the ranges disjoint
Usage: subedit-cli repair [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to repair

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --minimum TIME              Shortest interval kept; a shorter one is taken over by a neighbour (default 0.08)
  --max-lines N               Most lines of a merged subtitle; more are shown in windows, 0 for no limit (default 2)
  --dashes                    Put a dash before the text of each merged subtitle
  --output TEXT               File to write, for a single input
  --output-dir TEXT           Directory to write into
  --in-place                  Write back over the inputs
  --dry-run                   Work out and say what would be written, and write nothing
```

## Arguments and options

| Option | Required | Values | Default |
| :----- | :----- | :------ | :----- |
| `<file>...` | yes | one or more paths | — |
| `--minimum` | no | a duration: `0.08`, `1.5`, `00:00:00,080`. Zero keeps every interval | `0.08` (about two frames) |
| `--max-lines` | no | a whole number; `0` lifts the limit | `2` |
| `--dashes` | no | a flag: a dash in front of the text each subtitle brings to a merged subtitle | off |
| `--recursive`, `-r` | no | a flag | off — see [Processing a tree](lots.md) |
| `--output` / `--output-dir` / `--in-place` | **one of the three**, except with `--dry-run` | see [Invocation](invocation.md#the-destination) | — |
| `--dry-run` | no | a flag: computes and says, writes nothing — see [Seeing before writing](invocation.md#seeing-before-writing) | off |

<!-- exemple: printf '1\n00:00:01,000 --> 00:00:02,000\nAlpha\n\n2\n00:00:02,000 --> 00:00:04,000\nAlpha\n\n3\n00:00:01,000 --> 00:00:04,000\nBeta\n\n' > segments.srt; subedit-cli repair --output repare.srt segments.srt; cat repare.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nAlpha\n\n2\n00:00:02,000 --> 00:00:04,000\nAlpha\n\n3\n00:00:01,000 --> 00:00:04,000\nBeta\n\n' > segments.srt; subedit-cli repair --output repare.srt segments.srt; cat repare.srt
segments.srt: 1 repeated subtitle joined; already in order; 0 subtitles split, 1 merged -> repare.srt
1
00:00:01,000 --> 00:00:04,000
Alpha
Beta
```

`Alpha`, cut into two pieces that follow each other, becomes a single subtitle again; it then takes up the same range as `Beta`,
and the two make only one, of two lines, the earliest first.

## Output

**Standard output** — nothing: the result is the file written. On the error output, at the first level, **a sentence
that chains the three counts**, separated by semicolons: what `join-repeats`, `sort` and `disjoint` each say,
in their own words — `N repeated subtitles joined; M subtitles moved; K subtitles split, L merged…`.

### In JSON

With [`--format json`](invocation.md#output-a-script-can-read), the `counts` of each file: `subtitles`, `joined`,
`moved`, then those of [`disjoint`](disjoint.md#in-json) (`split`, `merged`, `windowed`, `absorbed`, `too_short`,
`ending_before_start`, `extras_dropped`).

## Exit codes and errors

Those of the tool. No error is specific to `repair` beyond the settings, which are judged as those of `disjoint`:
`--minimum` that is not a duration or is negative, `--max-lines` that is not a whole number, code `1`.
