# `pair`

```
subedit-cli pair (--output FILE | --output-dir DIR | --in-place)
                 -t FILE [--align-method position|number]
                 [--dry-run]
                 <main>
```

Writes **the translation re-timed on the positions of the main file**, and says how its lines found their
subtitle. It is `Open Translation…` of the window — [the same pairing](../subedit-gui/files.md#opening-a-translation),
the same sentence — followed by writing the result, which the window does through `Save Translation`.

**It is the one thing pairing does that a file processed alone does not.** The other subcommands
that take `-t` change the translation they read — [`case`](case.md), [`replace`](replace.md)… —; `pair` changes
nothing in the text: it **lays** the translation on the main file and writes what the laying produced. The main file is
read to receive the lines, **never written**.

- **What is written is the translation**, at **its own path and in its own format**: the format, the encoding and
  the line endings of the translation file. Its subtitles are **at the positions of the main file**: a line that
  fell beside the mark gets back the bounds of the subtitle it translates.
- **`-t` is required.** `pair` without a translation has nothing to lay: it is a usage error, code `1`.
- **A single main file**, and no `--recursive`: a pair is two files, and a tree has no
  way of saying which translation goes with which main file. For a batch, one invocation per pair.
- **`--output` names the file of the translation written, `--output-dir` puts its base name there, `--in-place`
  rewrites it**, as for any subcommand that takes `-t` — see [A translation](invocation.md#a-translation).
- **`--document` does not exist**: there is only one document aimed at, the translation.

<!-- example: subedit-cli pair --help -->
```console
$ subedit-cli pair --help
Write a translation at the positions of its main file, and say how it lined up
Usage: subedit-cli pair [OPTIONS] main

Positionals:
  main TEXT REQUIRED          The main subtitle file

Options:
  -h,--help                   Print this help message and exit
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
| `<main>` | yes | **a single** path | — |
| `-t`, `--translation-file` | **yes** | the translation file | — |
| `--align-method` | no | `position` or `number` — see [A translation](invocation.md#a-translation) | `position` |
| `--output` / `--output-dir` / `--in-place` | **one of the three**, except with `--dry-run` | see [Invocation](invocation.md#the-destination) | — |
| `--dry-run` | no | a flag: computes and says, writes nothing — see [Seeing before writing](invocation.md#seeing-before-writing) | off |

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nHello.\n\n2\n00:00:03,000 --> 00:00:04,000\nBye.\n\n' > film.srt; printf '1\n00:00:01,300 --> 00:00:02,300\nBonjour.\n\n2\n00:00:03,300 --> 00:00:04,300\nAu revoir.\n\n' > film.fr.srt; subedit-cli pair film.srt -t film.fr.srt --output recale.fr.srt; cat recale.fr.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nHello.\n\n2\n00:00:03,000 --> 00:00:04,000\nBye.\n\n' > film.srt; printf '1\n00:00:01,300 --> 00:00:02,300\nBonjour.\n\n2\n00:00:03,300 --> 00:00:04,300\nAu revoir.\n\n' > film.fr.srt; subedit-cli pair film.srt -t film.fr.srt --output recale.fr.srt; cat recale.fr.srt
film.fr.srt: translation: 2 lines attached -> recale.fr.srt
1
00:00:01,000 --> 00:00:02,000
Bonjour.

2
00:00:03,000 --> 00:00:04,000
Au revoir.
```

The translation was 0.3 s late: each line found the subtitle its middle overlaps, and the
file written carries the positions of the main file.

## What the alignment says

The sentence is **the one the window says** when it opens a translation, each part only when it is not
zero — see [`inspect -t`](inspect.md#a-translation--t) for the table of its parts. **It is written
from level 1 on**, and not at level 2 as for the other subcommands: here the alignment is not a
detail of the processing, it is the result.

**What the alignment does to the file written**:

- **a subtitle nobody translated is written as a block without text**, which reads with no anomaly;
- **a subtitle born of a line** — a line that nothing reached — is written with **its own positions**, whatever
  the method: the file written can therefore count more subtitles than the main file;
- **whatever the document carried as a translation is cleared** before the laying, which does not apply on the command line:
  the main file is read alone.

**The two methods** give the same file when the lines each fall on their subtitle, and do not
when a line is missing or the translation is shifted — that is why the method is a choice, and why
position is the default. The eight cases of `src/test/data/paires/` list them, with the file expected for
each method.

## Output

**Standard output** — nothing: the result is the file written.

| Level | What is added on the error output |
| :----- | :------------------------------------- |
| 1 | `<translation>: translation: N lines attached; … -> <destination>` — **the path is that of the translation**, which is the file written; with `--dry-run`, `(dry run, nothing written)` in place of `-> <destination>` |
| 2 | `<translation>: SubRip, UTF-8, no BOM, LF line endings kept` — the format, the encoding, the mark and the line endings **of the translation**, put back as they were |
| 3 | `<translation>: N bytes read, M written`, then **every reading diagnostic** of the main file and of the translation — see [Invocation](invocation.md#reading-diagnostics) |

### In JSON

With [`--format json`](invocation.md#output-a-script-can-read), **a single object**, whose `file` is the translation and
`destination` the file written. `counts.subtitles` counts **the subtitles of the file written**, those a line
brought to life included. The `alignment` key — `{"file", "method", "attached", "born", "untranslated", "out_of_order"}` —
carries the same counts as the sentence, like the `translation` key of [`inspect`](inspect.md#in-json).

## Exit codes

Those of the tool: `0` if the translation is written — or computed, with `--dry-run` —, `2` if one of the two files could not
be read or if the destination could not be written, `1` on a usage error. **A badly aligned translation is not
a failure**: it is written, and the sentence says what did not find its place.

## Errors

| What triggers them | Message |
| :------------------- | :------ |
| no `-t` | `pair needs the translation file to lay over the main one: use -t` |
| `--document` | `The following argument was not expected: --document` — it does not exist for `pair` |
| more than one main file | `The following argument was not expected: <path>` |
| the translation is the main file | `<translation>: the file is already open as the main document`, code `2` |
| one of the files missing, unreadable or of an unknown format | the same as everywhere, with the path of the file at fault |
