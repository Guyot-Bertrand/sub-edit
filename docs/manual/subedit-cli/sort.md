# `sort`

```
subedit-cli sort (--output FILE | --output-dir DIR | --in-place)
                 [--dry-run]
                 [--recursive]
                 <file>...
```

Puts the subtitles back **in the order of their start**. Nothing else changes: neither the texts, nor the
positions, nor the format of the file. The subtitles are **renumbered** from 1 by the writing,
as for any file written back.

**The core never sorts on its own**: reading a file leaves its order as it was, and
[`inspect`](inspect.md#what-anomalies-reports) reports a break in order with
`subtitle N starts before the previous one starts`. `sort` is **the gesture made on purpose**, for a file
or a batch whose order is broken — and it can be undone, in the window, like any operation.

- **The sort is stable**: two subtitles that start at the same moment **keep the order the
  file gave them**. Neither comes before the other; moving them would be a decision nobody
  asked for.
- **`sort` takes neither a range nor a document**: there is nothing to choose, the order is that of the starts.

<!-- example: subedit-cli sort --help -->
```console
$ subedit-cli sort --help
Put the subtitles in the order of their start, keeping ties as they are
Usage: subedit-cli sort [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to sort

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
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
| `--output` / `--output-dir` / `--in-place` | **one of the three**, except with `--dry-run` | see [Invocation](invocation.md#the-destination) | — |
| `--dry-run` | no | a flag: computes and says, writes nothing — see [Seeing before writing](invocation.md#seeing-before-writing) | off |

<!-- example: printf '1\n00:00:05,000 --> 00:00:06,000\ncinq\n\n2\n00:00:01,000 --> 00:00:02,000\nun\n\n3\n00:00:03,000 --> 00:00:04,000\ntrois\n\n' > desordre.srt; subedit-cli sort --output ordre.srt desordre.srt; cat ordre.srt -->
```console
$ printf '1\n00:00:05,000 --> 00:00:06,000\ncinq\n\n2\n00:00:01,000 --> 00:00:02,000\nun\n\n3\n00:00:03,000 --> 00:00:04,000\ntrois\n\n' > desordre.srt; subedit-cli sort --output ordre.srt desordre.srt; cat ordre.srt
desordre.srt: 3 subtitles moved -> ordre.srt
1
00:00:01,000 --> 00:00:02,000
un

2
00:00:03,000 --> 00:00:04,000
trois

3
00:00:05,000 --> 00:00:06,000
cinq
```

## What is counted

**The count is that of the places that changed subtitle**, as the core reports it — what
the window's history would call moved. A single misplaced subtitle shifts several: in
the example above, none of the three places keeps its subtitle, and the sentence says `3 subtitles moved`.

**A file already in order is written all the same**, and says so: a destination given is a
destination written, and making `sort` the exception would force a script to know which
subcommands sometimes produce a file and sometimes nothing.

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nun\n\n2\n00:00:03,000 --> 00:00:04,000\ntrois\n\n' > ordre.srt; subedit-cli sort --output copie.srt ordre.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nun\n\n2\n00:00:03,000 --> 00:00:04,000\ntrois\n\n' > ordre.srt; subedit-cli sort --output copie.srt ordre.srt
ordre.srt: already in order -> copie.srt
```

## Output

**Standard output** — nothing: the result is the file written. With `--dry-run`, nothing either: there is
no file, and the narration line says so — see [Seeing before writing](invocation.md#seeing-before-writing).

| Level | What is added on the error output |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles moved -> <destination>`, or `<path>: already in order -> <destination>` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — the format, the encoding, the mark and the line endings of the file read, put back as they were |
| 3 | `<path>: N bytes read, M written`, then **every reading diagnostic** — see [Invocation](invocation.md#reading-diagnostics) |

### In JSON

With [`--format json`](invocation.md#output-a-script-can-read), the object of each file carries
`counts.subtitles` (every subtitle of the file) and `counts.moved` (the places that changed;
`0` for a file already in order).

## Exit codes

Those of the tool: `0` if every file was processed — a file already in order is —, `2` if
none, `3` if only some, `1` on a usage error.

## Errors

None specific to `sort`: an unreadable file fails as for any subcommand, and the
errors of the destination are common to the subcommands that write:
see [Invocation](invocation.md#the-destination).
