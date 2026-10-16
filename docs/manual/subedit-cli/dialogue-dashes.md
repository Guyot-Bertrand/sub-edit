# `dialogue-dashes`

```
subedit-cli dialogue-dashes (--add | --remove) [--range N-M|N-]
                            (--output FILE | --output-dir DIR | --in-place)
                            [--dry-run]
                            [--recursive]
                            [--document main|translation] [-t FILE] [--align-method position|number]
                            <file>...
```

Puts a **dialogue dash** at the head of the lines, or takes them off. It is `Dialogue Dashes` of the
[window](../subedit-gui/operations.md).

**Two words, as for [`italics`](italics.md)**: `--add` and `--remove` exclude each other, and one is needed.

- **`--add`** puts a dash in front of each line that has none: `Bonjour` gives `- Bonjour`, and
  `Bonjour\nMarie` gives `- Bonjour\n- Marie`. **A dash already there is not doubled.**
- **Three dashes are recognised** — the keyboard's hyphen and the typographer's two long dashes —
  **and it is the hyphen that is written**: `— Bonjour` becomes `- Bonjour`. Extra spaces
  after the dash disappear.
- The dash goes **in front of the text, not in front of an opening tag**: `<i>Bonjour</i>` gives
  `<i>- Bonjour</i>`. No tag moves.
- **`--remove`** takes the leading dashes off, in all three forms.

<!-- example: subedit-cli dialogue-dashes --help -->
```console
$ subedit-cli dialogue-dashes --help
Put dialogue dashes on the lines, or take them off
Usage: subedit-cli dialogue-dashes [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to change

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --add                       Put a dialogue dash at the head of the lines
  --remove                    Take the dialogue dashes off
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
| `--add` / `--remove` | **one of the two** | flags | — |
| `--range` | no | `N-M`, or `N-` up to the end — see [`adjust`](adjust.md#--range) | the whole file |
| `--document` | no | `main` or `translation` — see [A translation](invocation.md#a-translation) | `main` |
| `-t`, `--translation-file` | with `--document translation` | a translation file, for **a single** input | — |
| `--align-method` | no, and only with `-t` | `position` or `number` | `position` |
| `--output` / `--output-dir` / `--in-place` | **one of the three**, except with `--dry-run` | see [Invocation](invocation.md#the-destination) | — |
| `--dry-run` | no | a flag: computes and says, writes nothing — see [Seeing before writing](invocation.md#seeing-before-writing) | off |

**The main text by default; the translation with `--document translation -t FILE`.** It is then
the translation that is changed and written, and only it — see [A translation](invocation.md#a-translation).

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nBonjour\nMarie\n\n2\n00:00:03,000 --> 00:00:04,000\n<i>Salut</i>\n\n' > a.srt; subedit-cli dialogue-dashes --add --output t.srt a.srt; cat t.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nBonjour\nMarie\n\n2\n00:00:03,000 --> 00:00:04,000\n<i>Salut</i>\n\n' > a.srt; subedit-cli dialogue-dashes --add --output t.srt a.srt; cat t.srt
a.srt: 2 subtitles dashed -> t.srt
1
00:00:01,000 --> 00:00:02,000
- Bonjour
- Marie

2
00:00:03,000 --> 00:00:04,000
<i>- Salut</i>
```

## Output

**Standard output** — nothing: the result is the file written. With `--dry-run`, the list of
changes, described in [Seeing before writing](invocation.md#seeing-before-writing).

| Level | What is added on the error output |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles dashed -> <destination>`, or `N subtitles undashed`, or `nothing to change` |
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
| neither `--add` nor `--remove` | `dialogue-dashes needs --add to put dashes on, or --remove` |
| both | `--add and --remove say opposite things; give one of them` |
| range badly written or outside the file | see [`adjust`](adjust.md#--range) |

The errors of the destination are common to the subcommands that write:
see [Invocation](invocation.md#the-destination).
