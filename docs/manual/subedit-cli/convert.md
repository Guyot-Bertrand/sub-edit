# `convert`

```
subedit-cli convert --to <format>
                    [--line-endings unix|windows|mac] [--to-encoding NAME]
                    [--bom | --no-bom] [--sort] [--disjoint]
                    [--join-repeats] [--repair]
                    (--output FILE | --output-dir DIR | --in-place)
                    [--dry-run]
                    [--recursive]
                    <file>...
```

Writes each file in the format and the shape asked for. **Never changes
the input**, unless `--in-place` explicitly asks for it.

<!-- example: subedit-cli convert --help -->
```console
$ subedit-cli convert --help
Write a subtitle file out in another format or shape
Usage: subedit-cli convert [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to convert

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --to TEXT:{srt,vtt,subviewer2,ssa,ass,microdvd,mpl2,tmplayer,lrc} REQUIRED
                              Format to write
  --line-endings TEXT:{unix,windows,mac}
                              Line endings to write; the source's by default
  --to-encoding NAME          Encoding to write; the source's by default
  --frame-rate RATE           Frame rate of a file counted in frames: 25, 23.976
  --bom                       Write a byte order mark
  --no-bom                    Write no byte order mark
  --sort                      Put the subtitles in order of their start before writing
  --disjoint                  Cut overlapping subtitles so that no range overlaps another, before writing
  --join-repeats              Join the subtitles that repeat each other into one, before writing
  --repair                    Join the repeats, sort, then make the ranges disjoint, before writing
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
| `--to` | **yes** | a name from the table below, and nothing else | — |
| `--line-endings` | no | `unix`, `windows` or `mac` | those of the file read |
| `--to-encoding` | no | any encoding ICU can write, except those that write their own mark | that of the file read |
| `--bom` / `--no-bom` | no | flags, exclusive of one another | what the file read carried |
| `--sort` | no | a flag: puts the subtitles in the order of their start **before writing**, see [Sorting on writing](#sorting-on-writing) | off |
| `--join-repeats` | no | a flag: makes a single subtitle of the pieces of one text that follow each other, **before anything else**, see [Making the ranges disjoint](#making-the-ranges-disjoint) | off |
| `--repair` | no | a flag: the same as `--join-repeats --sort --disjoint`, in that order | off |
| `--disjoint` | no | a flag: cuts the overlapping subtitles **before writing**, see [Making the ranges disjoint](#making-the-ranges-disjoint) | off |
| `--output` / `--output-dir` / `--in-place` | **one of the three**, except with `--dry-run` | see [Invocation](invocation.md#the-destination) | — |
| `--dry-run` | no | a flag: computes and says, writes nothing — see [Seeing before writing](invocation.md#seeing-before-writing) | off |

`mac` names the carriage return alone (`\r`), the line ending of the classic Mac OS.

## The formats `--to` accepts

| Name | Format | Extension written |
| :-- | :----- | :--------------- |
| `srt` | SubRip | `.srt` |
| `vtt` | WebVTT | `.vtt` |
| `subviewer2` | SubViewer 2.0 | `.sub` |
| `ssa` | Sub Station Alpha | `.ssa` |
| `ass` | Advanced SSA | `.ass` |
| `mpl2` | MPL2 | `.txt` |
| `microdvd` | MicroDVD | `.sub` |
| `tmplayer` | TMPlayer | `.txt` |
| `lrc` | LRC | `.lrc` |

**A name, and not an extension.** Two extensions name two formats
each — `.sub` is also MicroDVD's, `.txt` that of MPL2 and of
TMPlayer — so an option that took extensions would not know which one
is asked for. The names above are unambiguous, and match the
extension wherever the extension is unambiguous too.

**The list is closed.** It is Gaupol's, and all nine are in it.

## The two formats that carry no end

**TMPlayer and LRC write only one position per line**, its start. This
shows on both sides.

**On reading**, the ends are deduced: that of a line is the start of
the next one, and the last one gets five seconds. That is what Gaupol does.
We say so as well, through a diagnostic that `--verbose` shows on the three
subcommands:

```
file.lrc: carries no end times; each one was taken from the next start, settled by the reader
```

**On writing**, the ends are not written, for want of a place to put them.
Converting to one of these two formats therefore loses them, and a round trip does not
find them again — it deduces them again.

**LRC loses one more thing**: it has no way of carrying a line break.
A line of dialogue on two lines is written on one, the two lines joined by
a space. That loss cannot be repaired either.

Neither one has a vocabulary for italics.

## `--frame-rate`, and the one format that counts in frames

**MicroDVD does not count in seconds**: its positions *are* frame
numbers, and no MicroDVD file declares the rate at which they were
counted. `--frame-rate` gives it, and serves on both sides: it is the
rate at which a `.sub` in frames is read, and the one at which a
conversion to MicroDVD counts the frames.

| Option | Required | Values | Default |
| :----- | :----- | :------ | :----- |
| `--frame-rate` | no | a rate in frames per second: `25`, `23.976`, `29.97` | see below |

**On reading, without `--frame-rate`, the file is read at 23.976** — Gaupol's
value — and level `-vvv` says so:
`counts in frames and states no rate; it was read at`. It is the one place in the tool where every position shown
rests on an assumption the file cannot confirm.

**On writing, without `--frame-rate`, three cases and three answers:**

| The starting file | What happens |
| :------------------- | :-------------- |
| is already MicroDVD | the rate that read it writes it back, and the frames come back identical |
| is time-based, and its positions fall on a grid | the grid is taken, and `-vv` says which one |
| is time-based, with no grid | **refusal**, code 2: `writing frames needs a frame rate, and the positions fall on no grid to take one from — give --frame-rate` |

The refusal is deliberate. The only gesture left would be to make up a figure, and
every line of the file would move by however wrong it was.

## What a conversion loses

**Converting to another format is never lossless**, and what does not survive
depends on the pair:

| What does not get through | Why |
| :--------------------- | :------- |
| the header | it only makes sense in its own format: `[INFORMATION]` is not a WebVTT header |
| the data specific to the format | SubRip's coordinates, the identifier and the settings of a WebVTT cue have no equivalent elsewhere |
| the precision | SubViewer 2, the two Sub Station Alphas and LRC write to the hundredth, MPL2 to the tenth, TMPlayer to the second, MicroDVD to the frame; `01:00:00,017` comes back as `01:00:00,020` |
| the end of a line | TMPlayer and LRC have no field for it; the next reading deduces it again |
| the line breaks | LRC alone: its lines fit on one line, and the two are glued back by a space |
| the tags the target format cannot write | WebVTT has no `<font color>`, TMPlayer and LRC have nothing at all |
| the layout | `{\pos(x,y)}`, `{\an8}`, `<v Marie>`, `<ruby>`: they survive a round trip in their own format, and disappear in conversion |

**Writing a file back in its own format loses nothing** — that is the guarantee
that holds up all the rest, and `--to srt` on a `.srt` gives back the same bytes. Nothing
is even decoded: the text stays the raw string that was read.

## Tags are translated, not copied

**Each format writes italics its own way**, and converting without translating
would leave a `{\i1}` in a `.srt`: text the user sees, in a
file that will never know how to interpret it. What gets through are **six
things** — bold, italics, underline, colour, font and size:

| Format | Bold | Italics | Underline | Colour | Font | Size |
| :----- | :--: | :------: | :------: | :-----: | :----: | :----: |
| SubRip | `<b>` | `<i>` | `<u>` | `<font color="#RRGGBB">` | — | — |
| WebVTT | `<b>` | `<i>` | `<u>` | — | — | — |
| SubViewer 2 | `<b>` | `<i>` | `<u>` | `<font color="#RRGGBB">` | — | — |
| Sub Station Alpha | `{\b1}` | `{\i1}` | — | `{\c&HBBGGRR&}` | `{\fnNAME}` | `{\fsN}` |
| Advanced SSA | `{\b1}` | `{\i1}` | `{\u1}` | `{\c&HBBGGRR&}` | `{\fnNAME}` | `{\fsN}` |
| MicroDVD | `{Y:b}` | `{Y:i}` | `{Y:u}` | `{C:$BBGGRR}` | `{F:NAME}` | `{S:N}` |
| MPL2 | `\` | `/` | `_` | `{C:$BBGGRR}` | `{F:NAME}` | `{S:N}` |
| TMPlayer | — | — | — | — | — | — |
| LRC | — | — | — | — | — | — |

**A dash is an announced loss.** Converting a coloured `.srt` to WebVTT removes
the colour and says so; the file produced is WebVTT, not SubRip in disguise.

**MicroDVD and MPL2 cannot style part of a line.** Their tags
have no end: `{y:i}` slants everything that follows to the end of the line, and
`{Y:i}` to the end of the subtitle. A line only half of which was
bold is therefore written with no bold at all, rather than with bold that overflows.

## What the conversion says it lost

**It says so when it loses, and keeps quiet when it loses nothing.** A report
that shows up at every call is a report nobody reads.

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,500\n<i>sur deux</i>\nlignes\n\n' > a.srt; subedit-cli convert --to lrc --output b.lrc a.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,500\n<i>sur deux</i>\nlignes\n\n' > a.srt; subedit-cli convert --to lrc --output b.lrc a.srt
a.srt: 1 subtitle written as LRC -> b.lrc
a.srt: ends are not carried by LRC, line breaks were joined in 1 subtitle, 1 tag dropped
```

The line appears at the default verbosity level, and at every one above it.
The items, in this order:

| Item | What it says |
| :---- | :----------- |
| `ends are not carried by <format>` | the target format writes no end |
| `line breaks were joined in N subtitles` | LRC alone: N subtitles were on several lines |
| `N tags dropped` | N tags could not be written, layout included |
| `the header was dropped` | the header does not cross a format boundary |
| `the <format> fields of N subtitles were dropped` | N subtitles carried data specific to their format |
| `positions moved by up to N ms` | the largest distance by which a position moved |

## Changing the encoding

**`--to-encoding` chooses the encoding written; `--encoding` says the one that is
read.** The two together re-encode a file:

```console
$ subedit-cli convert --to srt --encoding cp1252 --to-encoding utf-8 \
      --output-dir sortie film.srt
```

Without `--to-encoding`, the file is written in **the encoding it was read in**,
as it is with its line endings and its mark: a file converted without an
instruction about encoding gives back the same characters, at the same bytes.

**A character the target encoding cannot write stops the file**, and
nothing is written. An `ł` has no place in Latin-1; replacing it with a `?`
would lose text between reading and writing, without anyone seeing it.

**A byte order mark exists only for the Unicode encodings.**
`--bom` with a `--to-encoding` that carries none is refused rather than ignored
— see the table of errors below.

**And the encoding has to name its byte order.** `UTF-16` and `UTF-32` are
names ICU knows, whose converter writes **a mark of its own**, whatever
it is asked: `--no-bom` would then be accepted and disobeyed. Both are
therefore refused, and the message names the output:

```console
$ subedit-cli convert --to srt --to-encoding UTF-16 --output copie.srt film.srt
"UTF-16" writes a byte order mark of its own; name the byte order, as UTF-16LE and UTF-16BE do
```

`UTF-16LE`, `UTF-16BE`, `UTF-32LE` and `UTF-32BE` write exactly the same
bytes, mark included — but under the control of `--bom` and `--no-bom`. The
refusal therefore costs nothing but a more precise name, and it holds for
reading as for writing: `--encoding UTF-16` gets the same answer.

## The destination

The three options and their rules are common to the subcommands that
write: see [Invocation](invocation.md#the-destination). Nothing is written
without one of them.

What is specific to `convert`: **`--output-dir` changes the extension with the
format.** `convert --to vtt` on `a.srt` writes `a.vtt`, never an `a.srt` holding
WebVTT — a file whose name lies trips up every other tool.

## Converting in place is refused

`--in-place` with a `--to` that changes the format gives code `1` and writes
nothing: in place, there is no second name to carry the new format, and
the file would stay under an extension its content no longer justifies.

`--in-place` can still be used to change **the shape without the format** —
writing a `.srt` back as a `.srt` with other line endings, for example.

The refusal is decided on **the extension alone**, before any reading: a usage
error must never leave a batch half written.

## Line endings and byte order mark

By default, **the file written takes up what the file read carried**. The model
keeps both on reading; imposing them by default would lose, at every
conversion, a piece of information kept on purpose.

<!-- example: printf '1\n00:00:01,000 --> 00:00:03,000\nBonjour.\n\n' > a.srt; subedit-cli convert --to vtt --output b.vtt a.srt; cat b.vtt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\nBonjour.\n\n' > a.srt; subedit-cli convert --to vtt --output b.vtt a.srt; cat b.vtt
a.srt: 1 subtitle written as WebVTT -> b.vtt
WEBVTT

00:01.000 --> 00:03.000
Bonjour.
```

## Sorting on writing

`--sort` writes the file **in the order of the starts**, whatever the target format. The sort is done **on a copy**: the file read is
not touched, and what is written is what [`sort`](sort.md) would have made of it — stable, two subtitles that start together keep their order.

**Never by default**: the order of a file is its author's as long as nothing else is asked for. With the option, each file says so, at the
first narration level, `<path>: N subtitles moved` or `<path>: already in order`, and the `--format json` record carries a
`moved` count — **absent without the option**, so as not to change the shape of what existing scripts read.

## Making the ranges disjoint

`--disjoint` cuts the overlapping subtitles so that no range overlaps another any more, **on the same copy as the sort**: the file read is not touched.

Never by default. The option does what [`disjoint`](disjoint.md) does, **with its default settings**, before writing, after the sort. It needs a file in order: otherwise it is refused, and `--sort` — which comes first — puts it in order beforehand. The file says so at the first narration level (`<path>: N subtitles split, M merged`), and the `--format json` record carries the `split` and `merged` counts, **absent without the option**.

**`--join-repeats` comes before the sort and the cutting**: a text cut into pieces that follow each other is rebuilt first (see
[`join-repeats`](join-repeats.md)), then the subtitles are sorted, then the ranges made disjoint. **`--repair` is all three at once**,
like [`repair`](repair.md), with the default settings of the cutting. With `--join-repeats`, each file says
`<path>: N repeated subtitles joined` at the first narration level, and the `--format json` record carries `counts.joined` — **absent without the option**.

## Output

**Standard output** — nothing: the result is the file written. With `--dry-run`, nothing either: there is no file, and the narration line says so — see [Seeing before writing](invocation.md#seeing-before-writing).

On the error output, according to the level:

| Level | What is added |
| :----- | :-------------- |
| 1 | `<path>: N subtitles written as WebVTT -> <destination>` |
| 2 | `<path>: SubRip -> WebVTT, UTF-8, no BOM, LF line endings` — the encoding named is the one written |
| 3 | `<path>: N bytes read, M written`, then **every reading diagnostic** — see [Invocation](invocation.md#reading-diagnostics) |

### In JSON

With [`--format json`](invocation.md#output-a-script-can-read), the object of each file carries `counts.subtitles`, then what the conversion lost: `lost_ends` and `lost_header` (`0` or `1`), `joined_lines`, `lost_tags`, `lost_fields` and `furthest_ms` — the items of [the sentence of loss](#what-the-conversion-says-it-lost). The full table is [Invocation](invocation.md#what-each-subcommand-puts-in-counts)'s.

## Exit codes

Those of the tool: `0` if every file was written, `2` if none, `3` if
only some, `1` on a usage error.

## Errors

| What triggers it | Message |
| :------------------ | :------ |
| `--bom` with `--no-bom` | `--bom and --no-bom ask for opposite things; give one or the other` |
| `--in-place` that changes the format | `--in-place cannot change the format: the file would keep a name its content no longer matches` |
| `--bom` on an encoding that has none | `<path>: <encoding> has no byte order mark to write` |
| `--to-encoding` naming an unknown encoding | `no encoding is named "<name>"` |
| `--to-encoding` naming an encoding that writes its own mark | `"<name>" writes a byte order mark of its own; name the byte order, as UTF-16LE and UTF-16BE do` |
| character missing from the encoding written | `<path>: holds a character the chosen encoding cannot write` |

**Refusing `--bom` on an encoding that carries none is not
pedantry.** A byte order mark exists for the Unicode encodings and
for no other: asking for one on a file in Windows-1252 is asking for
something that does not exist. The file would be written without it, and nobody would
know the request was not honoured.

Those of the destination are common to the subcommands that write:
see [Invocation](invocation.md#the-destination).

The reading errors are those of [`inspect`](inspect.md): they concern
the input file and have the same messages.
