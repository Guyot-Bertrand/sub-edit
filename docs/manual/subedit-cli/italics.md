# `italics`

```
subedit-cli italics (--on | --off) [--range N-M|N-]
                    (--output FILE | --output-dir DIR | --in-place)
                    [--dry-run]
                    [--recursive]
                    [--document main|translation] [-t FILE] [--align-method position|number]
                    <file>...
```

Puts the texts **in italics**, or **takes their italics out**, in the tags of the file's format.
It is `Italic` of the [window](../subedit-gui/operations.md).

**Two words and not a toggle**: the window's button asks the core which way it
plays; here one says it. `--on` and `--off` exclude each other, and one is needed — leaving it out, or giving both,
is a usage error (code `1`).

- **`--on`** wraps each text in a single pair of tags, **after removing those that
  were in it**: a text already in italics is not wrapped twice, and italics cut in two
  are redone in one piece. Over several lines, one pair covers the whole.
- **`--off`** removes the italic tags; **the rest of a tag is kept** (`{\b1\i1}` gives
  `{\b1}`).
- The text is written **as the format of the file writes italics**: `<i>…</i>` for SubRip, WebVTT and
  SubViewer 2, `{\i1}…{\i0}` for the two Sub Station Alphas, `{Y:i}` for MicroDVD.

<!-- example: subedit-cli italics --help -->
```console
$ subedit-cli italics --help
Put the texts in italics, or take their italics out
Usage: subedit-cli italics [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to change

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --on                        Put the texts in italics
  --off                       Take the italics out
  --range N-M|N-              Act only on subtitles N to M, or N to the end
  --document main|translation The document to change: the main one, or the translation given by -t
  -t,--translation-file FILE  Translation file to lay over the subtitle file, for a single input
  --align-method position|number
                              How the lines of the translation find their subtitles
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
| `--on` / `--off` | **one of the two** | flags | — |
| `--range` | no | `N-M`, or `N-` up to the end — see [`adjust`](adjust.md#--range) | the whole file |
| `--document` | no | `main` or `translation` — see [A translation](invocation.md#a-translation) | `main` |
| `-t`, `--translation-file` | with `--document translation` | a translation file, for **a single** input | — |
| `--align-method` | no, and only with `-t` | `position` or `number` | `position` |
| `--output` / `--output-dir` / `--in-place` | **one of the three**, except with `--dry-run` | see [Invocation](invocation.md#the-destination) | — |
| `--dry-run` | no | a flag: computes and says, writes nothing — see [Seeing before writing](invocation.md#seeing-before-writing) | off |

**The main text by default; the translation with `--document translation -t FILE`.** It is then
the translation that is changed and written, and only it — see [A translation](invocation.md#a-translation).

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\n<i>bon</i>jour marie\n\n2\n00:00:03,000 --> 00:00:04,000\nBonjour\nMarie\n\n' > a.srt; subedit-cli italics --on --output i.srt a.srt; cat i.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\n<i>bon</i>jour marie\n\n2\n00:00:03,000 --> 00:00:04,000\nBonjour\nMarie\n\n' > a.srt; subedit-cli italics --on --output i.srt a.srt; cat i.srt
a.srt: 2 subtitles put in italics -> i.srt
1
00:00:01,000 --> 00:00:02,000
<i>bonjour marie</i>

2
00:00:03,000 --> 00:00:04,000
<i>Bonjour
Marie</i>
```

## A format without style refuses

**TMPlayer and LRC write no style**: there are no italics to put on nor italics to take out. The
window greys the entry out rather than hiding it; the command line **refuses, file by
file, with the reason**, and the other files of the batch go on. What counts is what the
format can carry, not the way it writes it: SubRip and WebVTT write italics the same
way, and do not say the same thing about colour.

<!-- example: printf '00:00:01:Hello there\n00:00:03:Bye\n' > a.txt; subedit-cli italics --on --output-dir sortie a.txt; echo "code=$?" -->
```console
$ printf '00:00:01:Hello there\n00:00:03:Bye\n' > a.txt; subedit-cli italics --on --output-dir sortie a.txt; echo "code=$?"
a.txt: TMPlayer writes no style: there are no italics to put on
code=2
```

In JSON, the failure carries the identifier **`no-style`**.

## Output

**Standard output** — nothing: the result is the file written. With `--dry-run`, the list of
changes, described in [Seeing before writing](invocation.md#seeing-before-writing).

| Level | What is added on the error output |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles put in italics -> <destination>`, or `N subtitles taken out of italics`, or `nothing to change` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — the format, the encoding, the mark and the line endings of the file read, put back as they were |
| 3 | `<path>: N bytes read, M written`, then **every reading diagnostic** — see [Invocation](invocation.md#reading-diagnostics) |

`N` is the number of subtitles **whose text changed**, read from the operation itself.

### In JSON

With [`--format json`](invocation.md#output-a-script-can-read), the object of each file carries
`counts.changed` — the `N` of the sentence — and `changes`, the list of changed subtitles
(`{"subtitle", "document", "before", "after"}`, **empty** when nothing changed).

## Exit codes

Those of the tool: `0` if every file was processed — a file where nothing changes is —,
`2` if none, `3` if only some, `1` on a usage error.

## Errors

| What triggers it | Message |
| :------------------ | :------ |
| neither `--on` nor `--off` | `italics needs --on to put the texts in italics, or --off` |
| both | `--on and --off say opposite things; give one of them` |
| a format without style | `<path>: TMPlayer writes no style: there are no italics to put on` (or `take out`) |
| range badly written or outside the file | see [`adjust`](adjust.md#--range) |

The errors of the destination are common to the subcommands that write:
see [Invocation](invocation.md#the-destination).
