# `replace`

```
subedit-cli replace [--regex] [--case-sensitive] [--range N-M|N-]
                    (--output FILE | --output-dir DIR | --in-place)
                    [--dry-run]
                    [--recursive]
                    [--document main|translation] [-t FILE] [--align-method position|number]
                    <pattern> <replacement> <file>...
```

Replaces a text in the subtitles **without breaking a tag**. It is the
`Replace All` operation of the window's [search](../subedit-gui/recherche.md), with its rules:

- **the search is in the visible text** — what the viewer reads. Tags are not part
  of it: looking for `<i>` finds nothing, and looking for `Bonjour` finds a "Bonjour" half of which
  is in italics;
- **the replacement is in the source text**, the file's, keeping the tags where they
  were. A tag that cuts the word looked for encloses the whole replacement; a style that
  touches the match covers the whole replacement; what the match does not touch
  does not move. The rules, case by case, are those of the window's search.

**There is no `find` subcommand**: to see where a pattern is without changing anything,
it is [`--dry-run`](invocation.md#seeing-before-writing) that lists each subtitle concerned.

<!-- exemple: subedit-cli replace --help -->
```console
$ subedit-cli replace --help
Replace a text in the subtitles, without breaking a tag
Usage: subedit-cli replace [OPTIONS] pattern replacement files...

Positionals:
  pattern TEXT REQUIRED       Text to look for in what is shown
  replacement TEXT REQUIRED   What replaces it, tags allowed
  files TEXT ... REQUIRED     Subtitle files to change

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --regex                     Read the text to look for as a regular expression
  --case-sensitive            Tell capitals from small letters
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
| `<pattern>` | yes | the text to look for in what is shown; **not empty** | — |
| `<replacement>` | yes | what replaces it, tags allowed; can be empty | — |
| `<file>...` | yes | one or more paths | — |
| `--recursive`, `-r` | no | a flag | off — see [Processing a tree](lots.md) |
| `--regex` | no | a flag: the pattern is an ICU regular expression | plain text |
| `--case-sensitive` | no | a flag: tells capitals from small letters | they are ignored |
| `--range` | no | `N-M`, or `N-` up to the end — see [`adjust`](adjust.md#--range) | the whole file |
| `--document` | no | `main` or `translation` — see [A translation](invocation.md#a-translation) | `main` |
| `-t`, `--translation-file` | with `--document translation` | a translation file, for **a single** input | — |
| `--align-method` | no, and only with `-t` | `position` or `number` | `position` |
| `--output` / `--output-dir` / `--in-place` | **one of the three**, except with `--dry-run` | see [Invocation](invocation.md#the-destination) | — |
| `--dry-run` | no | a flag: computes and says, writes nothing — see [Seeing before writing](invocation.md#seeing-before-writing) | off |

**A pattern that begins with a dash** is written after `--`: `subedit-cli replace -- -- "—" film.srt`.

**The main text by default; the translation with `--document translation -t FILE`.** It is then
the translation that is changed and written, and only it — see [A translation](invocation.md#a-translation).

### Plain text, or expression

By default the pattern is **read as it is**: `a.b` looks for an `a`, a dot, a `b`. Case
is ignored with the same finesse as in the window — `É` and `é` are a single letter.

With `--regex`, the pattern is an **ICU** regular expression, read as Gaupol reads it: `.` crosses
a line break, `^` and `$` hold at each line of a subtitle. In the replacement, and
only then:

| Write | To say |
| :----- | :-------- |
| `$1` to `$9` | the groups of the match |
| `$0` | the whole match |
| `\n` | a line break |
| `\$`, `\\` | a `$`, a backslash |

Mind the shell's quotes: `'$1 EUR'`, and not `"$1 EUR"`, which would let the shell read `$1`.

<!-- exemple: printf '1\n00:00:01,000 --> 00:00:02,000\nPrix: 12 euros, et 7 euros.\n\n' > prix.srt; subedit-cli replace --regex '([0-9]+) euros' '$1 EUR' --output euros.srt prix.srt; cat euros.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nPrix: 12 euros, et 7 euros.\n\n' > prix.srt; subedit-cli replace --regex '([0-9]+) euros' '$1 EUR' --output euros.srt prix.srt; cat euros.srt
prix.srt: replaced 2 matches -> euros.srt
1
00:00:01,000 --> 00:00:02,000
Prix: 12 EUR, et 7 EUR.
```

## Tags

A text where a tag cuts the word looked for:

<!-- exemple: printf '1\n00:00:01,000 --> 00:00:02,000\n<i>Bon</i>jour tout le monde\n\n2\n00:00:03,000 --> 00:00:04,000\nBonjour <i>Marie</i>, ca va\n\n' > balises.srt; subedit-cli replace Bonjour Salut --output salut.srt balises.srt; cat salut.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\n<i>Bon</i>jour tout le monde\n\n2\n00:00:03,000 --> 00:00:04,000\nBonjour <i>Marie</i>, ca va\n\n' > balises.srt; subedit-cli replace Bonjour Salut --output salut.srt balises.srt; cat salut.srt
balises.srt: replaced 2 matches -> salut.srt
1
00:00:01,000 --> 00:00:02,000
<i>Salut</i> tout le monde

2
00:00:03,000 --> 00:00:04,000
Salut <i>Marie</i>, ca va
```

In the first subtitle, the tag cut "Bonjour": it now encloses "Salut".
In the second, it is not touched and stays where it is. **A tag left open
stays open**: repairing shaky markup would be another operation.

The replacement is read in **the tags of the file's format**: `<i>…</i>` for SubRip,
`{\i1}…` for Advanced SSA. It is the only way to format what one writes.

## Nothing found, nothing to change

Two answers that are not failures — the exit code is `0` and **the file is written
all the same**, a destination given being a destination written:

| Situation | Sentence |
| :-------- | :----- |
| the pattern is nowhere | `"zzz" not found` |
| it is there, but each match is replaced by itself | `nothing to change` |

<!-- exemple: printf '1\n00:00:01,000 --> 00:00:02,000\nBonjour\n\n' > a.srt; subedit-cli replace zzz x --output b.srt a.srt; subedit-cli replace Bonjour Bonjour --output c.srt a.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nBonjour\n\n' > a.srt; subedit-cli replace zzz x --output b.srt a.srt; subedit-cli replace Bonjour Bonjour --output c.srt a.srt
a.srt: "zzz" not found -> b.srt
a.srt: nothing to change -> c.srt
```

## `--dry-run`

The standard output lists **each subtitle whose text would change**, before and after, as
they are in the file — tags included:

<!-- exemple: printf '1\n00:00:01,000 --> 00:00:02,000\n<i>Bon</i>jour tout le monde\n\n2\n00:00:03,000 --> 00:00:04,000\nBonjour <i>Marie</i>, ca va\n\n' > balises.srt; subedit-cli replace --dry-run Bonjour Salut balises.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\n<i>Bon</i>jour tout le monde\n\n2\n00:00:03,000 --> 00:00:04,000\nBonjour <i>Marie</i>, ca va\n\n' > balises.srt; subedit-cli replace --dry-run Bonjour Salut balises.srt
balises.srt: replaced 2 matches (dry run, nothing written)
balises.srt: subtitle 1
- <i>Bon</i>jour tout le monde
+ <i>Salut</i> tout le monde
balises.srt: subtitle 2
- Bonjour <i>Marie</i>, ca va
+ Salut <i>Marie</i>, ca va
```

The form of the list is described in [Seeing before writing](invocation.md#seeing-before-writing);
in JSON, it is `changes`.

## Output

**Standard output** — nothing: the result is the file written. With `--dry-run`, the list of
changes above.

| Level | What is added on the error output |
| :----- | :------------------------------------- |
| 1 | `<path>: replaced N matches -> <destination>`, or `"<pattern>" not found`, or `nothing to change` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — the format, the encoding, the mark and the line endings of the file read, put back as they were |
| 3 | `<path>: N bytes read, M written`, then **every reading diagnostic** — see [Invocation](invocation.md#reading-diagnostics) |

`N` counts **the matches replaced in the subtitles whose text changed** — it is the
number the window says too.

### In JSON

With [`--format json`](invocation.md#output-a-script-can-read), the object of each file carries:

| Key | Content |
| :-- | :------ |
| `counts.replaced` | the matches replaced, in the subtitles whose text changed: the `N` of the sentence |
| `counts.matched` | the matches found, **whether their replacement changed the text or not** — that is what tells "nothing found" (`0`) from "nothing to change" (`replaced` at `0`, `matched` positive) |
| `changes` | the changed subtitles: `{"subtitle", "document", "before", "after"}`, **empty** when nothing changed |

## Exit codes

Those of the tool: `0` if every file was processed — a file where nothing was found is
—, `2` if none, `3` if only some, `1` on a usage error.

## Errors

| What triggers it | Message |
| :------------------ | :------ |
| empty pattern | `pattern: nothing to look for` |
| unreadable expression | `pattern: not a regular expression (<ICU's reason>)` |
| range badly written or outside the file | see [`adjust`](adjust.md#--range) |

**The pattern is read before the first file**: an unreadable expression is a usage error
(code `1`), nothing is read nor written.

The errors of the destination are common to the subcommands that write:
see [Invocation](invocation.md#the-destination).
