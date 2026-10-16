# `hearing-impaired`

```
subedit-cli hearing-impaired
                  (--output FILE | --output-dir DIR | --in-place)
                  [--dry-run]
                  [--recursive] [--parentheses]
                  [--document main|translation] [-t FILE] [--align-method position|number]
                  <file>...
```

Removes the **mentions for the hearing impaired** — the sounds described between square brackets — from the whole
file. **Parentheses stay by default**: they are used as often for a whispered
line as for a described sound, and removing them would erase dialogue. `--parentheses` removes them
too, for a file that describes its sounds in them.

What it does not do: song lyrics between hash signs, the speaker's name
before a colon, capitalizing again, correcting OCR errors. Those
patterns are those of the window's
[`Correct Texts…`](../subedit-gui/correct-texts.md) assistant, which the command
line offers under the name [`correct`](correct.md); and nothing here can be set: the
transformation is decided, not configurable.

<!-- example: subedit-cli hearing-impaired --help -->
```console
$ subedit-cli hearing-impaired --help
Remove the sounds described between square brackets
Usage: subedit-cli hearing-impaired [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to clean

Options:
  -h,--help                   Print this help message and exit
  --parentheses               Also remove the sounds between parentheses, often whispered lines
  -r,--recursive              Take directories as inputs, and every subtitle file in them
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
| `--parentheses` | no | a flag: also removes the mentions between parentheses | off |
| `--recursive`, `-r` | no | a flag | off — see [Processing a tree](lots.md) |
| `--document` | no | `main` or `translation` — see [A translation](invocation.md#a-translation) | `main` |
| `-t`, `--translation-file` | with `--document translation` | a translation file, for **a single** input | — |
| `--align-method` | no, and only with `-t` | `position` or `number` | `position` |
| `--output` / `--output-dir` / `--in-place` | **one of the three**, except with `--dry-run` | see [Invocation](invocation.md#the-destination) | — |
| `--dry-run` | no | a flag: computes and says, writes nothing — see [Seeing before writing](invocation.md#seeing-before-writing) | off |

The format of the file read is **kept**: changing the format is the work of
[`convert`](convert.md).

**The main text by default; the translation with `--document translation -t FILE`.** It is then
the translation that is changed and written, and only it — see [A translation](invocation.md#a-translation).

**A mention that would empty a translation writes it empty, and keeps the subtitle**: the subtitle
stays in the file, without text. The main text, for its part, loses a subtitle emptied entirely, as
below.

## What is removed, and what stays

A mention is a square bracket — or a parenthesis, with `--parentheses` — and what it encloses, **line
break included**: in real files, a mention is often cut by the
end of a line.

| Input | Output |
| :----- | :----- |
| `Bonjour [il tousse] Marie` | `Bonjour Marie` |
| `Bonjour[il tousse]Marie` | `Bonjour Marie` |
| `[Bruit de pas] Bonjour` | `Bonjour` |
| `A [un] [deux] B` | `A B` |

The removal leaves **exactly one space between what surrounded it, and nothing at
the edge of a line**. It is not a clean-up of the file's spaces: two spaces
the text already carried stay two spaces.

**When the mention straddles the line break, it is the break that remains** — the
two lines stay two lines. This case is not a curiosity: across fifteen
real subtitlings, every square bracket that seemed orphaned was a
mention cut that way.

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\nReculez ! [Il\nhurle] Tout de suite !\n\n' > a.srt; subedit-cli --quiet hearing-impaired --output b.srt a.srt; cat b.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\nReculez ! [Il\nhurle] Tout de suite !\n\n' > a.srt; subedit-cli --quiet hearing-impaired --output b.srt a.srt; cat b.srt
1
00:00:01,000 --> 00:00:03,000
Reculez !
Tout de suite !
```

**What is not a mention and does not move:**

| Case | What happens |
| :-- | :-------------- |
| purely numeric content — `[1]`, `(12)`, `[ 156478 ]` | kept as it is, delimiters included: a reference never named a sound |
| a parenthesis and what it encloses, without `--parentheses` | kept as it is: `Venez (chut) ici` does not change |
| a bracket or parenthesis that nothing closes | left as it is |
| a format tag — `<i>`, `</i>` | never removed |

A blank **between** the digits, a letter or a sign — `[1 2]`, `[1a]`,
`[-1]` — make it a mention. So does empty content, `[]` or `( )`: no
digit, so not a reference; nothing to say, so nothing to keep.

## What disappears entirely

**A line the removal emptied disappears**, and **a subtitle emptied
entirely is removed from the file**. The subtitles that follow are renumbered.

A line counts as empty when all that is left in it is blanks, format
tags, or a lone dialogue dash — `<i>[Musique]</i>` does not leave an
`<i></i>` on screen.

**The tags are those of the file's format.** `{\i1}[Musique]{\i0}` in an
Advanced SSA, `{Y:i}[Musique]` in a MicroDVD and `/[Musique]` in an MPL2 leave
nothing either. Conversely, an `<i>` in a format that writes no
tag — TMPlayer, LRC — is text the viewer reads, and the line that
carries it is not empty.

**A dialogue reduced to a single voice is no longer one**, and the dash that is left
goes away. As long as two voices remain, the dashes stay.

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\n[Bruit de pas]\n\n2\n00:00:04,000 --> 00:00:06,000\nAttends [il tousse] Marie.\n\n3\n00:00:07,000 --> 00:00:09,000\nVoir [1] la note.\n\n' > a.srt; subedit-cli hearing-impaired --output b.srt a.srt; cat b.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\n[Bruit de pas]\n\n2\n00:00:04,000 --> 00:00:06,000\nAttends [il tousse] Marie.\n\n3\n00:00:07,000 --> 00:00:09,000\nVoir [1] la note.\n\n' > a.srt; subedit-cli hearing-impaired --output b.srt a.srt; cat b.srt
a.srt: 1 subtitle cleaned, 1 removed -> b.srt
1
00:00:04,000 --> 00:00:06,000
Attends Marie.

2
00:00:07,000 --> 00:00:09,000
Voir [1] la note.
```

## A file without any mention is written all the same

It is copied identically, the exit code is `0`, and the report says there
was nothing to remove.

It is the rule of the subcommands that write: **a destination given is
a destination written**. Making an exception here would force a script to know
which of the subcommands sometimes produces a file and sometimes nothing.

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\nRien à signaler.\n\n' > a.srt; subedit-cli hearing-impaired --output b.srt a.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\nRien à signaler.\n\n' > a.srt; subedit-cli hearing-impaired --output b.srt a.srt
a.srt: no mention to remove -> b.srt
```

## Output

**Standard output** — nothing: the result is the file written. **With `--dry-run`, it is the proposed changes**: for each subtitle whose text would change, its number, the text before and the text after — see [Seeing before writing](invocation.md#seeing-before-writing).

| Level | What is added on the error output |
| :----- | :------------------------------------- |
| 1 | `<path>: N subtitles cleaned, M removed -> <destination>`, or `<path>: no mention to remove -> <destination>` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` — the format, the encoding, the mark and the line endings of the file read, put back as they were |
| 3 | `<path>: N bytes read, M written`, then **every reading diagnostic** — see [Invocation](invocation.md#reading-diagnostics) |

`N` counts the subtitles whose text changed, `M` those the removal
emptied entirely and that therefore left the file.

### In JSON

With [`--format json`](invocation.md#output-a-script-can-read), the object of each file carries `counts.cleaned` (the texts rewritten) and `counts.removed` (the subtitles removed); **with `--dry-run`**, the `changes` key also lists the changed subtitles, like the standard output of the text form. The full table is [Invocation](invocation.md#what-each-subcommand-puts-in-counts)'s.

## Exit codes

Those of the tool: `0` if every file was written, `2` if none, `3` if
only some, `1` on a usage error.

## Errors

This subcommand has no error of its own: it takes no
value to parse, and no text can make it refuse a file. What
can fail is the reading or the writing, as for the others.

**Nothing is written without one of the three destination options**, and their errors
are common to the subcommands that write:
see [Invocation](invocation.md#the-destination).
