# `disjoint`

```
subedit-cli disjoint (--output FILE | --output-dir DIR | --in-place)
                     [--minimum TIME] [--max-lines N] [--dashes] [--sort]
                     [--dry-run] [--recursive]
                     <file>...
```

Cuts the overlapping subtitles so that **no `[start, end]` range overlaps
another any more**. It is `Make Ranges Disjoint…` of the window — [the same engine](../subedit-gui/lines.md#making-the-ranges-disjoint).

**Nothing is shortened nor lengthened**: at every moment, the file shows the same texts as before. What
changes is the way the timeline is cut:

- it is cut at **every start and every end** of the subtitles of an overlapping group;
- an interval covered by **a single** subtitle stays that subtitle;
- an interval covered by **several** becomes **one** subtitle whose lines are theirs, **in
  the order of their starts** — the earliest on line 1, the next on line 2;
- a subtitle that overlaps nothing is not touched, and the order of the rest is kept.

<!-- exemple: subedit-cli disjoint --help -->
```console
$ subedit-cli disjoint --help
Cut overlapping subtitles so that no range overlaps another, merging what overlaps
Usage: subedit-cli disjoint [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to make disjoint

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --minimum TIME              Shortest interval kept; a shorter one is taken over by a neighbour (default 0.08)
  --max-lines N               Most lines of a merged subtitle; more are shown in windows, 0 for no limit (default 2)
  --dashes                    Put a dash before the text of each merged subtitle
  --sort                      Put the subtitles in order of their start first; without it, a file out of order is refused
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
| `--max-lines` | no | a whole number: the most lines of a merged subtitle; `0` lifts the limit | `2` |
| `--dashes` | no | a flag: a dash in front of the text each subtitle brings to a merged subtitle | off |
| `--sort` | no | a flag: puts the subtitles in the order of their start **first** — see [A file in order is needed](#a-file-in-order-is-needed) | off |
| `--recursive`, `-r` | no | a flag | off — see [Processing a tree](lots.md) |
| `--output` / `--output-dir` / `--in-place` | **one of the three**, except with `--dry-run` | see [Invocation](invocation.md#the-destination) | — |
| `--dry-run` | no | a flag: computes and says, writes nothing — see [Seeing before writing](invocation.md#seeing-before-writing) | off |

<!-- exemple: printf '1\n00:00:00,000 --> 00:00:05,000\nA\n\n2\n00:00:03,000 --> 00:00:08,000\nB\n\n' > chevauche.srt; subedit-cli disjoint --output disjoint.srt chevauche.srt; cat disjoint.srt -->
```console
$ printf '1\n00:00:00,000 --> 00:00:05,000\nA\n\n2\n00:00:03,000 --> 00:00:08,000\nB\n\n' > chevauche.srt; subedit-cli disjoint --output disjoint.srt chevauche.srt; cat disjoint.srt
chevauche.srt: 2 subtitles split, 1 merged -> disjoint.srt
1
00:00:00,000 --> 00:00:03,000
A

2
00:00:03,000 --> 00:00:05,000
A
B

3
00:00:05,000 --> 00:00:08,000
B
```

`A [0, 5]` and `B [3, 8]` give three disjoint subtitles: `A` alone, `A` and `B` together, `B` alone. No
moment is lost nor added.

## What the operation decides

**A line is never duplicated.** Two subtitles with the same text give a single line where they
overlap, and two neighbouring pieces that show the same thing make only one subtitle. The text of a
subtitle cut into several pieces is otherwise **repeated in each one**: it is what is on screen at that
moment.

**A very short interval is taken over by a neighbour.** Under `--minimum`, an interval disappears: the neighbour
**extends its range to cover it**, so that no moment is left empty and nothing moves by more than that
threshold. The neighbour chosen is the one whose lines **already contain** those of the interval, otherwise the
longest. The text of the interval is lost only if no neighbour carries it, and then for less than the threshold.
**A whole subtitle shorter than the threshold is never taken over**: it is left, and counted.

**More lines than the limit: windows.** A merged subtitle that would have more than `--max-lines` lines
is shown **in successive windows** of that size, in equal shares of the duration. Three lines over three
seconds:

<!-- exemple: printf '1\n00:00:00,000 --> 00:00:03,000\nA\n\n2\n00:00:00,000 --> 00:00:03,000\nB\n\n3\n00:00:00,000 --> 00:00:03,000\nC\n\n' > trois.srt; subedit-cli disjoint --output fenetres.srt trois.srt; cat fenetres.srt -->
```console
$ printf '1\n00:00:00,000 --> 00:00:03,000\nA\n\n2\n00:00:00,000 --> 00:00:03,000\nB\n\n3\n00:00:00,000 --> 00:00:03,000\nC\n\n' > trois.srt; subedit-cli disjoint --output fenetres.srt trois.srt; cat fenetres.srt
trois.srt: 0 subtitles split, 1 merged; 1 shown in windows for holding too many lines -> fenetres.srt
1
00:00:00,000 --> 00:00:01,500
A
B

2
00:00:01,500 --> 00:00:03,000
B
C
```

Four lines give three windows, and so on: the window slides one line at a time. Nothing is
truncated nor cut again.

**The translation follows the same cuts**, since a subtitle carries both texts. **What the format carries
on top** — a style, a position — is that of the first subtitle that covers; the loss is counted when
the merged subtitles differ.

**A subtitle whose end comes before its start** is not guessed at: it is left where it is, and counted.

## A file in order is needed

The groups are runs of neighbours: out of order, the neighbours are not the ones that overlap. A file
whose starts are not in order is therefore **refused**, and the message says so:

<!-- exemple: printf '1\n00:00:03,000 --> 00:00:08,000\nB\n\n2\n00:00:00,000 --> 00:00:05,000\nA\n\n' > desordre.srt; subedit-cli disjoint --output x.srt desordre.srt; echo "code $?" -->
```console
$ printf '1\n00:00:03,000 --> 00:00:08,000\nB\n\n2\n00:00:00,000 --> 00:00:05,000\nA\n\n' > desordre.srt; subedit-cli disjoint --output x.srt desordre.srt; echo "code $?"
desordre.srt: The subtitles are not in the order of their start: sort them first. Use --sort to do that here.
code 2
```

`--sort` does the sort first — the same as [`sort`](sort.md) —, and says so. The core never sorts on its own.

A second pass changes nothing: once the ranges are disjoint, there is nothing left to cut, and the
sentence says `already disjoint`.

## Output

**Standard output** — nothing: the result is the file written.

| Level | What is added on the error output |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles split, M merged -> <destination>`, followed after a `;` by what could not be done in full: `K shown in windows for holding too many lines`, `K very short taken over by a neighbour`, `K subtitles shorter than the limit left whole`, `K merged with styles or positions that differ, the first kept`, `K subtitles ending before the start left alone` — or `<path>: already disjoint -> <destination>` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` |
| 3 | `<path>: N bytes read, M written`, then **every reading diagnostic** |

`N subtitles split` counts the original subtitles cut into several pieces, `M merged` the pieces that
show several subtitles together.

### In JSON

With [`--format json`](invocation.md#output-a-script-can-read), the `counts` of each file:

| Count | What it says |
| :----- | :----------- |
| `subtitles` | the subtitles of the file written |
| `split`, `merged` | as in the sentence |
| `windowed` | the pieces shown in windows |
| `absorbed` | the intervals taken over by a neighbour |
| `too_short` | the subtitles shorter than the threshold, left whole |
| `ending_before_start` | the subtitles whose end comes before their start, left alone |
| `extras_dropped` | the merged pieces whose style or position differ |
| `moved` | with `--sort` only: the places that changed |

## Exit codes

Those of the tool: `0` if every file was processed — a file without overlaps is —, `2` if
none, `3` if only some, `1` on a usage error.

## Errors

| What triggers them | Message |
| :------------------- | :------ |
| a file out of order without `--sort` | `<path>: The subtitles are not in the order of their start: sort them first. Use --sort to do that here.` |
| `--minimum` is not a duration, or is negative | `--minimum: …` |
| `--max-lines` is not a whole number | `--max-lines: "…" is not a number of lines: expected a whole number, 0 for no limit` |

An unreadable file fails as for any subcommand; the errors of the destination are common to the
subcommands that write: see [Invocation](invocation.md#the-destination).

## In the other commands

[`convert`](convert.md#making-the-ranges-disjoint), [`append`](append.md#making-the-ranges-disjoint) and
[`split-file`](split-file.md#making-the-ranges-disjoint) accept `--disjoint`, which does the same thing with
the default settings, before writing.

On a badly segmented file — a text cut into pieces that follow each other, given out of order —, sorting and cutting
alone give lines that flip from one piece to the next: [`join-repeats`](join-repeats.md) first, or
[`repair`](repair.md), which chains all three.
