# `split-file`

```
subedit-cli split-file --at N --head FILE --tail FILE
                       [--dry-run] [--sort] [--disjoint]
                       <file>
```

Cuts a file **in two**: a head and a tail. It is `Split Project…` of the window — [the same engine](../subedit-gui/operations.md#split-project),
the same counts, the same refusal — and **the exact inverse of [`append`](append.md)**: appending the tail to the
head gives back the starting file.

- **`--at N` is the number of the first subtitle of the tail**, numbered as the window's table
  numbers it: from 1. `--at 3` puts subtitles 1 and 2 in the head, and the third and the rest in the
  tail. It goes **from the second to the last**: a cut needs a subtitle on each side.
- **The tail is brought back to the origin**: it is shifted by minus the end of the last subtitle left in the head,
  its positions keeping their order and their spacing. That is what makes `split-file` the inverse of `append`.
- **Both halves keep the format, the encoding, the byte order mark and the line endings of the file read**, and
  are renumbered from 1 by the writing, like any file written back.
- **Two outputs, so a destination of its own**: `--head` and `--tail`, **both required**. `--output`,
  `--output-dir`, `--in-place` and `--recursive` do not exist for `split-file` and are refused like any unknown
  option, code `1`: there is neither a directory to share two named files into, nor an input to overwrite.

<!-- example: subedit-cli split-file --help -->
```console
$ subedit-cli split-file --help
Cut a subtitle file in two, a head and a tail
Usage: subedit-cli split-file [OPTIONS] file

Positionals:
  file TEXT REQUIRED          Subtitle file to cut

Options:
  -h,--help                   Print this help message and exit
  --at N                      Number of the first subtitle of the tail, as the table numbers them
  --head FILE                 File to write the subtitles before the cut to
  --tail FILE                 File to write the subtitles from the cut on to
  --dry-run                   Work out and say what would be written, and write nothing
  --sort                      Put the subtitles in order of their start before cutting; --at counts in that order
  --disjoint                  Cut overlapping subtitles so that no range overlaps another, before cutting
```

## Arguments and options

| Option | Required | Values | Default |
| :----- | :----- | :------ | :----- |
| `<file>` | yes | **a single** path | — |
| `--at` | yes | a whole number, from 2 to the number of subtitles of the file | — |
| `--head`, `--tail` | **yes, both**, except with `--dry-run` | file paths; their directory is created if it is missing | — |
| `--dry-run` | no | a flag: computes and says, writes nothing — see [Seeing before writing](invocation.md#seeing-before-writing) | off |
| `--sort` | no | a flag: puts the subtitles in the order of their start **before cutting**; `--at` counts in that order | off |
| `--disjoint` | no | a flag: cuts the overlapping subtitles **before cutting**; `--at` counts in what is left, see [Making the ranges disjoint](#making-the-ranges-disjoint) | off |

**With `--dry-run`, no destination is required**: none may be given, but not just one of the two —
`--head` and `--tail` go together. Those that are given are judged as without `--dry-run`.

**The two destinations can be neither the input nor the same file**: writing over what one is
reading, or writing the tail over the head, is refused before anything is read, code `1`.

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nun\n\n2\n00:00:03,000 --> 00:00:04,000\ndeux\n\n3\n00:00:06,000 --> 00:00:07,000\ntrois\n\n4\n00:00:08,000 --> 00:00:09,000\nquatre\n\n' > film.srt; subedit-cli split-file --at 3 --head tete.srt --tail queue.srt film.srt; cat queue.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nun\n\n2\n00:00:03,000 --> 00:00:04,000\ndeux\n\n3\n00:00:06,000 --> 00:00:07,000\ntrois\n\n4\n00:00:08,000 --> 00:00:09,000\nquatre\n\n' > film.srt; subedit-cli split-file --at 3 --head tete.srt --tail queue.srt film.srt; cat queue.srt
film.srt: split at subtitle 3: 2 subtitles in the head, 2 subtitles in the tail -> tete.srt, queue.srt
1
00:00:02,000 --> 00:00:03,000
trois

2
00:00:04,000 --> 00:00:05,000
quatre
```

The second subtitle, left in the head, ends at 4 s: the tail moves back by 4 s, and the third subtitle,
which started at 6 s, starts at 2 s.

## Cutting in time order

`--at` numbers the subtitles **as the file gives them**. On a file whose order is broken, the head is therefore not the half that is earliest
in time. `--sort` puts the subtitles in the order of their starts **before judging the cut** — stable, as [`sort`](sort.md) does —, and
`--at` then counts in that order.

Never by default. With the option, the file says at the first narration level how many subtitles moved (`<path>: N subtitles moved`
or `<path>: already in order`), and the `--format json` record carries a `moved` count, **absent without the option**.

## Making the ranges disjoint

Never by default. The option does what [`disjoint`](disjoint.md) does, **with its default settings**, before judging the cut, after the sort: `--at` counts in what that gives. It needs a file in order: otherwise it is refused, and `--sort` — which comes first — puts it in order beforehand. The file says so at the first narration level (`<path>: N subtitles split, M merged`), and the `--format json` record carries the `split` and `merged` counts, **absent without the option**.

## When the cut is refused

**Two halves that overlap at the cut are not cut.** If a subtitle of the tail started before
the end of the last subtitle left behind, the shift would carry it before the start of the video — a negative position,
which no format writes. `split-file` refuses **naming the subtitle**, in the window's words:

```text
o.srt: Cannot split at subtitle 3: subtitle 3 would fall before the start of the video. Cut somewhere else.
```

The code is `2` and **nothing is written**. Cut somewhere else. Landing exactly on the start is allowed: zero is a
position.

**`--at` outside the file is refused the same way**, once the file is read: `--at 1` — the first subtitle cannot
start a tail, nothing would be left — and `--at` past the last subtitle, which would leave an empty
tail. A file of a single subtitle cannot be cut.

## Output

**Standard output** — nothing: the result is the two files written.

| Level | What is added on the error output |
| :----- | :------------------------------------- |
| 1 | `<path>: split at subtitle N: H subtitles in the head, T subtitles in the tail -> <head>, <tail>`; with `--dry-run`, `(dry run, nothing written)` in place of `-> …` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — the format, the encoding, the mark and the line endings of the file read, put back as they were |
| 3 | `<path>: N bytes read, H and T written`, then **every reading diagnostic** — see [Invocation](invocation.md#reading-diagnostics) |

### In JSON

With [`--format json`](invocation.md#output-a-script-can-read), **a single object**, whose `file` is the file cut.
**`destination` is the head** — the file the input becomes — and **`tail` the tail**, a path, or `null` with
`--dry-run` like `destination`. The `counts`: `subtitles` (those of the file read), `head` and `tail` (those of each
half).

## Exit codes

| Code | When |
| :--- | :---- |
| `0` | both files are written — or computed, with `--dry-run` |
| `1` | usage error: no `--head` and `--tail`, an option `split-file` does not have, a destination that is the input or the other destination, an `--at` that is not a positive whole number |
| `2` | the file could not be read, **the cut is refused** (outside the file, or overlap), or a destination could not be written. There is no code `3`: a single file is processed |

**The head is written first**: a disk that refuses the tail leaves the head behind it, and the code is `2`.

## Errors

| What triggers them | Message |
| :------------------- | :------ |
| neither `--head` nor `--tail` | `no destination given: split-file writes two files, use --head and --tail` |
| only one of the two, with `--dry-run` | `--head and --tail go together: give both, or neither with --dry-run` |
| the two destinations are the same file | `<path>: named as both the head and the tail` |
| a destination is the input | `<path>: would be written over the input <input>` |
| `--at` outside the file | `<file>: --at 9: the file holds 3 subtitles, and the tail can start from 2 to 3` |
| a file of one subtitle | `<file>: a file of 1 subtitle cannot be split: a cut needs a subtitle on each side` |
| overlap | `<file>: Cannot split at subtitle N: subtitle M would fall before the start of the video. Cut somewhere else.` |
| a file missing, unreadable or of an unknown format | `<path>: does not exist`… — the same as everywhere |
