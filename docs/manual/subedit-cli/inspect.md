# `inspect`

```
subedit-cli inspect [--frame-rate RATE] [--recursive] [--detail]
                   [-t FILE] [--align-method position|number] <file>...
```

Reports what each file is made of. **Changes nothing and writes no
file.**

<!-- example: subedit-cli inspect --help -->
```console
$ subedit-cli inspect --help
Report what a subtitle file is made of
Usage: subedit-cli inspect [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to report on

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --detail                    List every anomaly, subtitle by subtitle, instead of counting them
  --frame-rate RATE           Frame rate of a file counted in frames: 25, 23.976
  -t,--translation-file FILE  Translation file to lay over the subtitle file, for a single input
  --align-method position|number
                              How the lines of the translation find their subtitles
```

## Arguments

| Argument | Required | Value | Default |
| :------- | :----- | :----- | :----- |
| `<file>...` | yes | one or more paths of subtitle files | — |
| `--recursive`, `-r` | no | a flag | off — see [Processing a tree](lots.md) |
| `--detail` | no | a flag: names each anomaly, instead of counting them — see [What `anomalies` reports](#what-anomalies-reports) | off |

No path is a usage error, hence code `1`.

## Options

| Option | Required | Value | Default |
| :----- | :------ | :----- | :----- |
| `--frame-rate` | no | a rate in frames per second: `25`, `23.976`, `29.97` | 23.976 |

**It only serves files counted in frames**, that is MicroDVD, the
only one of the nine. A MicroDVD file carries nothing but frame numbers; the
rate at which they are read decides **every position of the report**,
starting with `span`. Given elsewhere, it changes nothing and is not an
error.

A value that names nothing is a usage error, hence code `1`:

<!-- example: subedit-cli inspect --frame-rate douze film.sub; echo $? -->
```console
$ subedit-cli inspect --frame-rate douze film.sub; echo $?
--frame-rate: "douze" is not a frame rate: expected frames per second, like 25 or 23.976
1
```

It is the same option as on [`convert`](convert.md), word for word, because
it is the same rate. `convert` also uses it to **write**.

## A translation: `-t`

| Option | Required | Value | Default |
| :----- | :------ | :----- | :----- |
| `-t`, `--translation-file` | no | a translation file, for **a single** input | — |
| `--align-method` | no, and only with `-t` | `position` or `number` | `position` |

With `-t`, the translation is laid on the file **as the window does when opening it** — see
[A translation](invocation.md#a-translation) — and the report says **how its lines found their
subtitle**. A script thus knows whether a translation is aligned before correcting it.

<!-- example: printf '1\n00:00:01,000 --> 00:00:02,000\nHello.\n\n2\n00:00:03,000 --> 00:00:04,000\nBye.\n\n' > film.srt; printf '1\n00:00:01,000 --> 00:00:02,000\nBonjour.\n\n' > film.fr.srt; subedit-cli --quiet inspect -t film.fr.srt film.srt | tail -3 -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nHello.\n\n2\n00:00:03,000 --> 00:00:04,000\nBye.\n\n' > film.srt; printf '1\n00:00:01,000 --> 00:00:02,000\nBonjour.\n\n' > film.fr.srt; subedit-cli --quiet inspect -t film.fr.srt film.srt | tail -3
  anomalies: none
  translation file: film.fr.srt, matched by position
  translation: 1 line attached; 1 subtitle left without a translation
```

The first added line names the file and the method; the second is **the sentence the window
says** when it opens a translation, each part only when it is not zero:

| Part | What it counts |
| :----- | :---------------- |
| `N lines attached` | the lines of the translation that found a subtitle of the main file |
| `N subtitles born of a line` | the lines that found none and gave birth to their own, at their own positions |
| `N subtitles left without a translation` | the subtitles of the main file that no line reached |
| `N lines out of order` | the lines that came after a later one in the file |

A translation that carries no line says `translation: the file holds no line`. **The report of the
file itself stays that of the file alone**: a subtitle born of a line is not counted in
`subtitles`.

The errors of `-t` are those of a reading, said with the path of the translation, and this one:
`<translation>: the file is already open as the main document`, when the file given is the main one.
`--align-method` without `-t` is a usage error, and so is `-t` with several inputs — see
[A translation](invocation.md#a-translation).

## Output

The report goes to the **standard output** — it is the result of the command, and
it is written at every level, `--quiet` included. One block per file, preceded
by the path:

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,500\nLe canot dérive.\n\n2\n00:00:04,000 --> 00:00:06,200\nPersonne n a vu la côte.\n' > exemple.srt; subedit-cli --quiet inspect exemple.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,500\nLe canot dérive.\n\n2\n00:00:04,000 --> 00:00:06,200\nPersonne n a vu la côte.\n' > exemple.srt; subedit-cli --quiet inspect exemple.srt
exemple.srt
  format: SubRip
  encoding: UTF-8, detected
  byte order mark: absent
  line endings: LF
  subtitles: 2
  span: 00:00:01.000 -> 00:00:06.200
  frame rate grid: none (too few subtitles to tell)
  anomalies: none
```

| Field | What it says |
| :---- | :----------- |
| `format` | one of the formats `--to` accepts, written in full: `SubRip`, `WebVTT`, `SubViewer 2`, `Sub Station Alpha`, `Advanced SSA`, `MicroDVD`, `MPL2`, `TMPlayer`, `LRC` |
| `encoding` | the encoding read, followed by **where the answer comes from**: `, from its byte order mark` if the file declares it, `, as asked for` if `--encoding` imposed it, `, detected` if it was weighed |
| `byte order mark` | `present` or `absent` |
| `line endings` | `LF`, `CRLF` or `CR`, followed by `, mixed from line N` if the file mixes them |
| `subtitles` | the number of subtitles read |
| `span` | from the earliest start to the latest end, `HH:MM:SS.mmm` |
| `frame rate grid` | the frame rate on which the positions were computed, **deduced** — see below |
| `frame rate` | for a file counted in frames, the rate at which it was read and **where it comes from** — replaces the previous line, see below |
| `anomalies` | `none` on the same line, or what is wrong: **one kind a line with its count**, indented under `anomalies:`; with `--detail`, **one anomaly a line**, subtitle by subtitle |

**`span` is not "from the first to the last"** but from the earliest to the latest:
on a file whose order is broken, the two differ, and only the second tells
the truth about what the file covers.

**`anomalies`, with `--detail`, counts the subtitles from 1**, as the file
numbers them — and not the lines of the file. A subtitle number survives a
change; a line number does not. See below.

## What `frame rate grid` reports

**A subtitle file does not declare its frame rate.** SubRip has no
header, WebVTT's is free text. This line therefore reads nothing:
it **deduces**, by measuring whether the positions fall on a grid of frames.

What is measured are the **starts**. Wherever a grid exists they are on it
without exception, whereas the ends often stray from it — an end is
frequently computed by a reading-speed rule rather than put on
a frame.

The first line always says the same thing: the rate chosen, the verdict,
and the concentration that supports it.

| Verdict | What it means |
| :------ | :----------------- |
| `clean` | the positions are on a grid |
| `partial` | part of them is, the rest is not |
| `none` | no candidate explains anything |

`none` has **two causes, and they are not the same answer**: either no
grid fits — the line then gives the best candidate and its score,
so that one sees by how much it fails — or the file counts too few
subtitles for anything to be said about it. Two starts always look
perfectly aligned; that proves nothing.

**A regular file on a non-standard rate gives `none`, and that is
intended.** The set of candidates is closed — the eight rates the
conversion offers — and reporting the least wrong of them would be giving
a wrong answer where "I do not know" is the right one.

### The lines that follow, when they have something to say

| Line | When it appears |
| :---- | :------------------ |
| `grid offset` | the positions are on the grid **give or take a constant**: the file was shifted, and here is by how much |
| `also fits` | another candidate fits just as well, because it is a whole multiple of the one chosen |
| `too short a span to separate` | the span of the file does not allow the chosen rate to be told apart from those named |
| `off the grid` | how many starts stray from the grid, and in how many **runs** |

**`also fits` is not a hesitation.** A grid at 25 frames per second is
*included* in a grid at 50: a file timed on 25 therefore fits both.
The reverse is false — a real 50 collapses on 25 — so it is the lowest that
is chosen, and the other is named rather than kept quiet. The only
pairs concerned are 25 and 50, 29.97 and 59.94, 30 and 60.

**`too short a span to separate` guards against an overconfident answer.** 23.976
and 24 drift apart by one millisecond per second of film: on a feature film they
separate beyond doubt, over ten seconds they cannot be told apart. The line says
which one remains possible.

**`off the grid` tells apart two stories the percentage alone confuses.**
Many runs of a single start are positions corrected by hand,
one by one. A few long runs are a re-timed section or a file
assembled from two others. The number of runs is what separates them.

### An example

Twelve lines put on a grid at 25 frames per second — so at
positions that are multiples of 40 milliseconds — spread over nearly nine minutes:

<!-- example: k=0; while [ $k -lt 12 ]; do s=$((1000 + k*47000 + (k%3)*40)); e=$((s+2000)); k=$((k+1)); printf '%d\n%02d:%02d:%02d,%03d --> %02d:%02d:%02d,%03d\nRéplique %d.\n\n' $k $((s/3600000)) $((s/60000%60)) $((s/1000%60)) $((s%1000)) $((e/3600000)) $((e/60000%60)) $((e/1000%60)) $((e%1000)) $k; done > grille.srt; subedit-cli --quiet inspect grille.srt | tail -3 -->
```console
$ k=0; while [ $k -lt 12 ]; do s=$((1000 + k*47000 + (k%3)*40)); e=$((s+2000)); k=$((k+1)); printf '%d\n%02d:%02d:%02d,%03d --> %02d:%02d:%02d,%03d\nRéplique %d.\n\n' $k $((s/3600000)) $((s/60000%60)) $((s/1000%60)) $((s%1000)) $((e/3600000)) $((e/60000%60)) $((e/1000%60)) $((e%1000)) $k; done > grille.srt; subedit-cli --quiet inspect grille.srt | tail -3
  frame rate grid: 25 fps, clean (100.0%)
  also fits: 50 fps, of which this rate is a whole divisor
  anomalies: none
```

The example makes its own file rather than reading one from the repository, and that is
deliberate: it can be replayed as it is, and the loop shows what a grid is
better than a sentence — positions that are multiples of a frame duration.

## What `frame rate` reports, and why it is not a grid

**Only one of the nine formats counts in frames**: MicroDVD, each line of which
carries two frame numbers and nothing else. On such a file, `inspect`
does not write `frame rate grid` but `frame rate`, and the two lines are
never shown together.

**The reason fits in a sentence.** The positions of a MicroDVD file were
computed *from* its frames, at a given rate; deducing them from a
grid would give back that very rate, as it has just been supplied. It would be
a figure given, dressed up as a measurement.

What is written instead is **the reading rate and where it comes from** —
the same form as the `encoding` line:

| What the line says | When |
| :------------------ | :---- |
| `frame rate: 25 fps, as asked for` | `--frame-rate` imposed it |
| `frame rate: 24000/1001 fps, assumed` | nobody said it; it is the default |

**Two answers and not three**, where `encoding` has three: a MicroDVD
file declares nothing. That is the whole problem — the rate necessarily comes
from outside, and the report says which of the two outsides.

**The rate decides the whole rest of the report.** `span` goes through it, and two
readings of the same file at two rates give two spans:

<!-- example: printf '{25}{75}Première.\n{100}{150}Deuxième.\n{200}{260}Troisième.\n' > images.sub; subedit-cli --quiet inspect images.sub | tail -3; subedit-cli --quiet inspect --frame-rate 25 images.sub | tail -3 -->
```console
$ printf '{25}{75}Première.\n{100}{150}Deuxième.\n{200}{260}Troisième.\n' > images.sub; subedit-cli --quiet inspect images.sub | tail -3; subedit-cli --quiet inspect --frame-rate 25 images.sub | tail -3
  span: 00:00:01.043 -> 00:00:10.844
  frame rate: 24000/1001 fps, assumed
  anomalies: none
  span: 00:00:01.000 -> 00:00:10.400
  frame rate: 25 fps, as asked for
  anomalies: none
```

## What `anomalies` reports

Three things can be wrong in a document, and each one is repaired differently.
**By default `inspect` counts them**, one line per kind, in the order of the table:
a badly made file carries hundreds of them, and knowing how many of each kind
is what says what to run. **`--detail` names them one by one.**

| What is written | With `--detail` | What it means |
| :--------------- | :-------------- | :------------------ |
| `subtitle ends before it starts: N` | `subtitle N ends before it starts` | the end comes before the start |
| `subtitle starts before the previous one ends: N` | `subtitle N starts before the previous one ends` | it overlaps the one before |
| `subtitle starts before the previous one starts: N` | `subtitle N starts before the previous one starts` | it breaks the order of the file — [`sort`](sort.md) puts it back |

A kind that is absent is not written. In the left-hand column, `N` is the number
of subtitles; in the next one, the number of a subtitle.

**The same subtitle can appear twice**, and that is intended: the one that
starts before the previous one has started also starts before it has
ended. The first point is settled by moving the subtitle, the second by
changing a duration.

**The subtitle named is the one that is misplaced**, not the one it is
misplaced against. On starts at `0`, `4 s`, `2 s`, `3 s`: the third breaks the order, the
fourth does follow the third and is therefore not named — there is nothing to
do about it.

<!-- example: printf '1\n00:00:00,000 --> 00:00:00,500\nA\n\n2\n00:00:04,000 --> 00:00:04,500\nB\n\n3\n00:00:02,000 --> 00:00:02,500\nC\n\n4\n00:00:03,000 --> 00:00:03,500\nD\n' > desordre.srt; subedit-cli --quiet inspect desordre.srt | tail -3 -->
```console
$ printf '1\n00:00:00,000 --> 00:00:00,500\nA\n\n2\n00:00:04,000 --> 00:00:04,500\nB\n\n3\n00:00:02,000 --> 00:00:02,500\nC\n\n4\n00:00:03,000 --> 00:00:03,500\nD\n' > desordre.srt; subedit-cli --quiet inspect desordre.srt | tail -3
  anomalies:
    subtitle starts before the previous one ends: 1
    subtitle starts before the previous one starts: 1
```

And with `--detail`, the subtitle named:

<!-- example: printf '1\n00:00:00,000 --> 00:00:00,500\nA\n\n2\n00:00:04,000 --> 00:00:04,500\nB\n\n3\n00:00:02,000 --> 00:00:02,500\nC\n\n4\n00:00:03,000 --> 00:00:03,500\nD\n' > desordre.srt; subedit-cli --quiet inspect --detail desordre.srt | tail -3 -->
```console
$ printf '1\n00:00:00,000 --> 00:00:00,500\nA\n\n2\n00:00:04,000 --> 00:00:04,500\nB\n\n3\n00:00:02,000 --> 00:00:02,500\nC\n\n4\n00:00:03,000 --> 00:00:03,500\nD\n' > desordre.srt; subedit-cli --quiet inspect --detail desordre.srt | tail -3
  anomalies:
    subtitle 3 starts before the previous one ends
    subtitle 3 starts before the previous one starts
```

**One line per anomaly with `--detail`**, because a hundred findings
stuck on one line can be neither read nor searched. A document without a flaw keeps
its single line, `anomalies: none`, which a script looks for. **The
`--format json` record always lists each anomaly**: a script reads the detail, and
`--detail` only changes the text.

**It is not a reading diagnostic.** The diagnostics say what the
reading came across and point at a **line of the file**; the anomalies say
what the document *is* and point at a **subtitle**. A file whose order
is broken is not malformed for all that: the reading has nothing to say about it.

## In JSON

With [`--format json`](invocation.md#output-a-script-can-read), `inspect` writes **one
object per file** instead of the blocks above; the narration stays on the error output.
The keys describe what the text says, and **none is to be read back out of a sentence**:

| Key | Content |
| :-- | :------ |
| `format` | the identifier of the format, the one `convert --to` accepts: `srt`, `vtt`… |
| `encoding` | `{"charset", "origin", "byte_order_mark"}` — `origin` is `byte-order-mark`, `asked` or `detected` |
| `line_endings` | `{"kind", "mixed_from_line"}` — `lf`, `crlf` or `cr`; the line where a second kind appears, or `null` |
| `subtitles` | the number of subtitles |
| `span_ms` | `{"start", "end"}` — the first start and the last end, in milliseconds |
| `frame_rate` | for a file counted in frames: `{"rate", "origin"}` (`asked` or `assumed`); otherwise `null` |
| `grid` | for the others: the grid deduced, or `null` for a file counted in frames |
| `anomalies` | an array of `{"subtitle", "kind"}` — `end-before-start`, `overlapping-subtitles`, `out-of-order` |
| `translation` | **only with `-t`**: `{"file", "method", "attached", "born", "untranslated", "out_of_order"}` — `method` is `position` or `number`, and the four counts are those of the sentence above |
| `warnings` | what the reading came across, at every level |

`grid` carries `verdict` (`clean`, `partial` or `silent`), `enough_starts`, `rate` (a string,
or `null` when nothing is named), `concentration_permille` (thousandths, `null` without enough
starts), `offset_ms`, `also_fits` (the rate of which this one is a whole divisor, or
`null`), `not_separated` (the rates a span too short does not separate), `strays`
(the starts off the grid) and `starts`.

<!-- example: printf '{25}{50}Un.\n{75}{100}Deux.\n' > f.sub; subedit-cli --format json inspect --frame-rate 25 f.sub 2>/dev/null -->
```console
$ printf '{25}{50}Un.\n{75}{100}Deux.\n' > f.sub; subedit-cli --format json inspect --frame-rate 25 f.sub 2>/dev/null
{"schema":1,"command":"inspect","file":"f.sub","ok":true,"format":"microdvd","encoding":{"charset":"UTF-8","origin":"detected","byte_order_mark":false},"line_endings":{"kind":"lf","mixed_from_line":null},"subtitles":2,"span_ms":{"start":1000,"end":4000},"frame_rate":{"rate":"25","origin":"asked"},"grid":null,"anomalies":[],"warnings":[]}
```

## Narration

On the error output, according to the level asked for:

| Level | What is added |
| :----- | :-------------- |
| 1 | `<path>: N subtitles` |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings` |
| 3 | `<path>: N bytes read`, `<path>: N diagnostic(s) while reading`, then **each of them** — see [Invocation](invocation.md#reading-diagnostics) |

## Exit codes

Those of the tool: `0` if every file was read, `2` if none, `3` if
only some, `1` on a usage error.

## Errors

Those of a reading — of the file, and of the translation when `-t` is given —, and
those of `-t`: see above. `inspect` writes nothing that could be refused.

| What triggers it | Message |
| :------------------ | :------ |
| no path given | `files is required`, followed by a pointer to `--help` |
| missing file | `<path>: does not exist` |
| unreadable file | `<path>: cannot be opened: permission denied` |
| bytes that do not decode | `<path>: cannot be decoded in the chosen encoding` |
| format not recognised | `<path>: is in no format this tool knows` |
| nothing that looks like a subtitle | `<path>: holds nothing recognisable as a subtitle` |

These are the same messages for the seventeen subcommands: the recipe for opening
is written once, in the core.

## An example that fails

An empty file matches no format. It is not reported as empty:
it is refused, and named.

<!-- example: : > vide.srt; subedit-cli inspect vide.srt; echo "code=$?" -->
```console
$ : > vide.srt; subedit-cli inspect vide.srt; echo "code=$?"
vide.srt: is in no format this tool knows
code=2
```
