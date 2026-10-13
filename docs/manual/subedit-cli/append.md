# `append`

```
subedit-cli append --output FILE
                   [--dry-run] [--sort] [--disjoint]
                   <base> <file>...
```

Puts files **after the first one**, into **a single** file: the film in two parts, turned into a
single set of subtitles. It is `Append File…` of the window — [the same engine](../subedit-gui/operations.md#append-file),
the same counts — applied as many times as there are files to append.

- **The first file is the base.** It gives the result **its format, its encoding, its byte
  order mark and its line endings**.
- **Nothing is inserted between two files**: each one is shifted by **the end of the last subtitle of what
  precedes it**, its positions keeping their order and their spacing. The third follows the second, not the
  base: the files follow each other in the order they are given.
- **Each appended file goes through the conversion** to the base's format: its tags are translated,
  and what that format cannot write is said, file by file, in the window's words.
- **The appended files have no translation**: a file opened alone has none to give.
- **A file can be given several times**, or appended to itself: nothing forbids it.

**`append` is not a batch.** N inputs, **one** output: the loop of the other subcommands — one result
per file, and a failure that lets the others go on — makes no sense here. Three things follow from it:

- **the destination is `--output`, and nothing else.** `--output-dir` and `--in-place` do not exist for
  `append` and are refused like any unknown option, code `1`; `--recursive` likewise: the order of the
  files is the one given, and a walked tree has none;
- **all or nothing**: the first file that cannot be read stops the run, and **nothing is written**. A
  film in three parts one of which is missing is not a film in two;
- **a single line per run** for the result, and, in JSON, **a single object**.

<!-- exemple: subedit-cli append --help -->
```console
$ subedit-cli append --help
Put subtitle files one after another into a single file
Usage: subedit-cli append [OPTIONS] files...

Positionals:
  files BASE FILE...          The base file, then the files to put after it, in order

Options:
  -h,--help                   Print this help message and exit
  --output FILE               File to write
  --dry-run                   Work out and say what would be written, and write nothing
  --sort                      Put the subtitles in order of their start before writing
  --disjoint                  Cut overlapping subtitles so that no range overlaps another, before writing
```

## Arguments and options

| Option | Required | Values | Default |
| :----- | :----- | :------ | :----- |
| `<base> <file>...` | yes, **at least two** | paths: the base, then what is put after it, in order | — |
| `--output` | **yes**, except with `--dry-run` | a file path; its directory is created if it is missing | — |
| `--dry-run` | no | a flag: computes and says, writes nothing — see [Seeing before writing](invocation.md#seeing-before-writing) | off |
| `--sort` | no | a flag: puts the whole in the order of its starts **once everything is appended**, see [Sorting the result](#sorting-the-result) | off |
| `--disjoint` | no | a flag: cuts the overlapping subtitles **once everything is appended and sorted**, see [Making the ranges disjoint](#making-the-ranges-disjoint) | off |

**The destination cannot be any of the inputs**, the base included: writing over a file one is
reading is refused before anything is read, code `1`. An existing `--output` distinct from the inputs is
overwritten, as everywhere.

<!-- exemple: printf '1\n00:00:01,000 --> 00:00:02,000\nun\n\n2\n00:00:03,000 --> 00:00:04,000\ndeux\n\n' > partie1.srt; printf '1\n00:00:00,500 --> 00:00:01,500\ntrois\n\n' > partie2.srt; subedit-cli append --output film.srt partie1.srt partie2.srt; cat film.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nun\n\n2\n00:00:03,000 --> 00:00:04,000\ndeux\n\n' > partie1.srt; printf '1\n00:00:00,500 --> 00:00:01,500\ntrois\n\n' > partie2.srt; subedit-cli append --output film.srt partie1.srt partie2.srt; cat film.srt
partie2.srt: appended 1 subtitle
partie1.srt: 3 subtitles from 2 files -> film.srt
1
00:00:01,000 --> 00:00:02,000
un

2
00:00:03,000 --> 00:00:04,000
deux

3
00:00:04,500 --> 00:00:05,500
trois
```

The last subtitle of `partie1.srt` ends at 4 s; the one of `partie2.srt`, which started at 0.5 s, starts at
4.5 s.

## Sorting the result

Each file is shifted **from the end of what precedes it**: the order that can break is therefore that of the files themselves. `--sort`
puts the result in the order of the starts, **in one go, after the last append**, as [`sort`](sort.md) would — stable, and without changing anything
in the content.

Never by default. With the option, the base says so at the first narration level, `<base>: N subtitles moved` or `<base>: already in order`, and
the `--format json` record carries a `moved` count, **absent without the option**.

## Making the ranges disjoint

Never by default. The option does what [`disjoint`](disjoint.md) does, **with its default settings**, once everything is appended, on the whole. It needs a file in order: otherwise it is refused, and `--sort` — which comes first — puts it in order beforehand. The file says so at the first narration level (`<path>: N subtitles split, M merged`), and the `--format json` record carries the `split` and `merged` counts, **absent without the option**.

## When the formats differ

The result is in **the base's format**, whatever the others'. What the crossing costs is
said file by file, in the window's words — the same sentence as `Append File…` and as
[pasting from another format](../subedit-gui/presse-papiers.md#les-balises-dun-autre-format-sont-traduites):

```text
d.ass: appended 1 subtitle; Advanced SSA into SubRip: 1 tag dropped, the header was dropped
```

What is lost depends only on the format of **each** appended file: a base in Advanced SSA receives a
SubRip file without losing anything, not the reverse. To choose the format of the result, put first a file
that is already in that format, or convert afterwards with [`convert`](convert.md).

## Output

**Standard output** — nothing: the result is the file written.

| Level | What is added on the error output |
| :----- | :------------------------------------- |
| 1 | one line per appended file — `<path>: appended N subtitles`, followed, when the format cost something, by `; <source format> into <base format>: <what is lost>` — then `<base>: N subtitles from M files -> <destination>` (with `--dry-run`, `(dry run, nothing written)` in place of `-> <destination>`) |
| 2 | `<base>: SubRip, UTF-8, no BOM, LF line endings kept` — the format, the encoding, the mark and the line endings of the base, put back as they were |
| 3 | `<base>: N bytes read, M written` (all the files together), then **every reading diagnostic** — see [Invocation](invocation.md#reading-diagnostics) |

### In JSON

With [`--format json`](invocation.md#output-a-script-can-read), **a single object** for the whole run,
whose `file` is **the base**. Its `counts`:

| Count | What it says |
| :----- | :----------- |
| `files` | the files of the run, the base included |
| `subtitles` | the subtitles of the result |
| `appended` | those that come from the appended files |
| `lost_ends`, `lost_header` | `0` or `1`: one of the appended files lost it |
| `joined_lines`, `lost_tags`, `lost_fields` | the sum of what the appended files lost |
| `furthest_ms` | the largest move of a position, all files together |

**An `inputs` array** goes with the `counts`, outside them: one element per appended file, in order,
`{"file", "counts"}` — `counts` carrying `appended`, then the same losses, **for that file alone**. It is what
says, in a run of ten files, which one cost what. The reading warnings of all the files
are gathered in `warnings`.

A failure is an object too, **a single one**: that of the first file that could not be read.

## Exit codes

| Code | When |
| :--- | :---- |
| `0` | the result is written — or computed, with `--dry-run` |
| `1` | usage error: fewer than two files, no `--output`, an option `append` does not have, a destination that is one of the inputs |
| `2` | **a file could not be read**, or the destination could not be written: nothing is written. There is no code `3`: a run that writes only one file succeeds or does not |

## Errors

| What triggers them | Message |
| :------------------- | :------ |
| a single file given | `append needs a base file and at least one file to put after it` |
| neither `--output` nor `--dry-run` | `no destination given: append writes one file, use --output` |
| `--output` names an input | `<destination>: would be written over the input <input>` |
| a file missing, unreadable or of an unknown format | `<path>: does not exist`, `<path>: is in no format this tool knows`… — the same as everywhere |

**An empty file is not appended**: no format reads without at least one subtitle, and the file is
refused like any file that cannot be read — `<path>: holds nothing recognisable as a subtitle` —, which
stops the run and writes nothing.
