# `case`

```
subedit-cli case --to title|sentence|upper|lower [--range N-M|N-]
                 (--output FILE | --output-dir DIR | --in-place)
                 [--dry-run]
                 [--recursive]
                 [--document main|translation] [-t FILE] [--align-method position|number]
                 <file>...
```

Puts the texts in **title case, sentence case, capitals or small letters**, without touching a
tag. It is `Tools ▸ Case` of the [window](../subedit-gui/operations.md), with its rules.

| `--to` | Effect | `bonjour marie` becomes |
| :----- | :---- | :---------------------- |
| `title` | each word takes a capital, the rest comes down | `Bonjour Marie` |
| `sentence` | the first letter of the text, and only it; the rest comes down | `Bonjour marie` |
| `upper` | all in capitals | `BONJOUR MARIE` |
| `lower` | all in small letters | `bonjour marie` |

- **What comes before the first letter or the first digit is not touched**: a dialogue
  dash, an opening quotation mark or a parenthesis survive (`- bonjour` gives `- Bonjour`).
- **Tags are not text**: they do not change case — `<I>` stays `<I>` — and do not
  change place: a tag that cuts a word still cuts it afterwards.
- **ICU decides where a word begins**: `l'été sera chaud` gives `L'été Sera Chaud`, and the accents
  hold (`éléonore` gives `Éléonore`).

<!-- exemple: subedit-cli case --help -->
```console
$ subedit-cli case --help
Put the texts in title, sentence, upper or lower case, tags intact
Usage: subedit-cli case [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to change

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --to title|sentence|upper|lower
                              The case to put the texts in
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
| `--to` | **yes** | `title`, `sentence`, `upper`, `lower` | — |
| `--range` | no | `N-M`, or `N-` up to the end — see [`adjust`](adjust.md#--range) | the whole file |
| `--document` | no | `main` or `translation` — see [A translation](invocation.md#a-translation) | `main` |
| `-t`, `--translation-file` | with `--document translation` | a translation file, for **a single** input | — |
| `--align-method` | no, and only with `-t` | `position` or `number` | `position` |
| `--output` / `--output-dir` / `--in-place` | **one of the three**, except with `--dry-run` | see [Invocation](invocation.md#the-destination) | — |
| `--dry-run` | no | a flag: computes and says, writes nothing — see [Seeing before writing](invocation.md#seeing-before-writing) | off |

**The main text by default; the translation with `--document translation -t FILE`.** It is then
the translation that is changed and written, and only it — see [A translation](invocation.md#a-translation).

<!-- exemple: printf '1\n00:00:01,000 --> 00:00:02,000\n<i>bon</i>jour marie\n\n2\n00:00:03,000 --> 00:00:04,000\n- BONJOUR MARIE\n\n' > a.srt; subedit-cli case --to title --output titre.srt a.srt; cat titre.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\n<i>bon</i>jour marie\n\n2\n00:00:03,000 --> 00:00:04,000\n- BONJOUR MARIE\n\n' > a.srt; subedit-cli case --to title --output titre.srt a.srt; cat titre.srt
a.srt: 2 subtitles recased -> titre.srt
1
00:00:01,000 --> 00:00:02,000
<i>Bon</i>jour Marie

2
00:00:03,000 --> 00:00:04,000
- Bonjour Marie
```

A file where **no text changes** — everything is already in the case asked for — is written as it is and
says so:

<!-- exemple: printf '1\n00:00:01,000 --> 00:00:02,000\nBONJOUR\n\n' > a.srt; subedit-cli case --to upper --output b.srt a.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nBONJOUR\n\n' > a.srt; subedit-cli case --to upper --output b.srt a.srt
a.srt: nothing to change -> b.srt
```

## Output

**Standard output** — nothing: the result is the file written. With `--dry-run`, the list of
changes, described in [Seeing before writing](invocation.md#seeing-before-writing).

| Level | What is added on the error output |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles recased -> <destination>`, or `nothing to change` |
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
| `--to` missing | `--to is required` |
| a value that is not one of the four | `--to: … not in {title,sentence,upper,lower}` |
| range badly written or outside the file | see [`adjust`](adjust.md#--range) |

The errors of the destination are common to the subcommands that write:
see [Invocation](invocation.md#the-destination).
