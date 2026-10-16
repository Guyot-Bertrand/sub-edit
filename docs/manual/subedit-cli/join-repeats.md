# `join-repeats`

```
subedit-cli join-repeats (--output FILE | --output-dir DIR | --in-place)
                         [--dry-run]
                         [--recursive]
                         <file>...
```

Makes **a single subtitle** of the subtitles that show the same thing one after the other: a text cut into pieces
that follow each other, as some segmentation tools leave it. It is `Join Repeated Subtitles` of the window —
[the same engine](../subedit-gui/lines.md#joining-repeated-subtitles).

**A repeat** is a subtitle that has **the same text, the same translation and the same format settings** (a
style, a position, cue settings) as another one, **and that starts at the latest when that one ends**: they
touch or overlap. A run of repeats becomes **the one that starts first**, whose end is
carried to the latest end of the run; the others disappear.

- **The order of the file makes no difference, and is not changed**: the subtitle kept is the one that starts first,
  wherever the others are. A file in order stays in order.
- **Nothing is joined by mistake**: another text, another translation, other settings, a gap between two
  pieces, a subtitle without text or whose end comes before its start — in all these cases they stay apart.
- **To be done before the rest**: sorting and [cutting the overlaps](disjoint.md) read a clean run better.
  [`repair`](repair.md) chains all three, and [`convert`](convert.md#making-the-ranges-disjoint) accepts it as an option (`--join-repeats`, `--repair`).

<!-- example: subedit-cli join-repeats --help -->
```console
$ subedit-cli join-repeats --help
Join the subtitles that repeat each other, one after the other, into one
Usage: subedit-cli join-repeats [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to join the repeats of

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

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nAlpha\n\n2\n00:00:02,000 --> 00:00:04,000\nAlpha\n\n3\n00:00:04,000 --> 00:00:06,000\nAlpha\n\n4\n00:00:08,000 --> 00:00:09,000\nBeta\n\n' > morceaux.srt; subedit-cli join-repeats --output joint.srt morceaux.srt; cat joint.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nAlpha\n\n2\n00:00:02,000 --> 00:00:04,000\nAlpha\n\n3\n00:00:04,000 --> 00:00:06,000\nAlpha\n\n4\n00:00:08,000 --> 00:00:09,000\nBeta\n\n' > morceaux.srt; subedit-cli join-repeats --output joint.srt morceaux.srt; cat joint.srt
morceaux.srt: 2 repeated subtitles joined -> joint.srt
1
00:00:01,000 --> 00:00:06,000
Alpha

2
00:00:08,000 --> 00:00:09,000
Beta
```

The three pieces of `Alpha`, which follow each other from 1 s to 6 s, now make only one. `Beta`, separated by a gap,
stayed. A second pass changes nothing and says so: `no repeated subtitles`.

## Output

**Standard output** — nothing: the result is the file written.

| Level | What is added on the error output |
| :----- | :------------------------------------- |
| 1 | `<path>: N repeated subtitles joined -> <destination>`, or `<path>: no repeated subtitles -> <destination>` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` |
| 3 | `<path>: N bytes read, M written`, then **every reading diagnostic** |

### In JSON

With [`--format json`](invocation.md#output-a-script-can-read), the `counts` of each file: `subtitles` (those of the file
written), `joined` (the subtitles removed because another one covers them) and `chains` (the runs joined).

## Exit codes

Those of the tool: `0` if every file was processed — a file without repeats is —, `2` if none, `3` if
only some, `1` on a usage error.

## Errors

None specific to `join-repeats`: an unreadable file fails as for any subcommand, and the errors of
the destination are common to the subcommands that write: see [Invocation](invocation.md#the-destination).
