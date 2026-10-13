# `correct`

```
subedit-cli correct --tasks LIST --code CODE
                    [--classes human|ocr|human,ocr]
                    [--enable NAME]... [--disable NAME]...
                    [--language CODE]
                    [--max-length N] [--max-lines N]
                    [--skip-length N|off] [--skip-lines N|off]
                    [--keep-blank-subtitles] [--range N-M|N-]
                    [--document main|translation] [-t FILE] [--align-method position|number]
                    (--output FILE | --output-dir DIR | --in-place)
                    [--dry-run]
                    [--recursive]
                    <file>...
```

Corrects the texts with **Gaupol's patterns**, like the window's
[`Correct Texts…`](../subedit-gui/correct-texts.md) assistant — same patterns,
same engine, same counts. Six tasks exist here: the **mentions for the
hearing impaired**, **joining** and **splitting words**, **common errors**,
**capitalization** and **line breaking**.

<!-- exemple: subedit-cli correct --help -->
```console
$ subedit-cli correct --help
Correct the texts with the patterns of the correction assistant
Usage: subedit-cli correct [OPTIONS] files...

Positionals:
  files TEXT ... REQUIRED     Subtitle files to correct

Options:
  -h,--help                   Print this help message and exit
  -r,--recursive              Take directories as inputs, and every subtitle file in them
  --tasks LIST                The tasks to run, in a list: none is chosen beforehand
  --code CODE                 The cascade of patterns the tasks read: Zyyy, Latn, Latn-en, Latn-en-US
  --classes human|ocr|human,ocr
                              The classes of common errors to apply: both by default
  --enable NAME               Switch a pattern on, by its English name or type:name; repeatable
  --disable NAME              Switch a pattern off, by its English name or type:name; repeatable
  --language CODE             The dictionary join-words and split-words read: fr_FR, en_US, en...
  --max-length N              Longest line of line-break, in characters; no default, Gaupol's is in ems
  --max-lines N               Most lines of line-break: 3, as Gaupol's
  --skip-length N|off         Leave alone a subtitle whose longest line is within this, or off: --max-length
  --skip-lines N|off          Leave alone a subtitle whose line count is within this, or off: --max-lines
  --keep-blank-subtitles      Leave the subtitles the correction empties, empty, instead of removing them
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

## What is chosen, and what is not

**Nothing is ticked beforehand.** The window opens its boxes as the user left them; a
command line takes its tasks, its code, its line length and its language from its own line,
never from the settings file: neither the tasks ticked one day in the window, nor the pattern
boxes switched there (`correction.activations`), nor the spell-checker's replacement list are
read, and nothing is written on that side.

**Four settings come from the settings file, by default** — the values `Correct Texts…`
remembers, or that [`config set`](config.md) wrote. **An option on the line always wins**, then
the file, then the built-in default; see [The settings file](invocation.md#the-settings-file)
for where it is, `--config` and `--no-config`.

| Key | What it supplies | Option that overrides it |
| :-- | :--------------- | :----------------------- |
| `correction.human`, `correction.ocr` | the classes of common errors applied | `--classes` |
| `correction.remove-blank` | whether the subtitles the correction empties are removed | `--keep-blank-subtitles` |
| `correction.line-break.max-lines` | the most lines of `line-break` | `--max-lines` |

- **`correction.remove-blank = false` has no option to undo it** on the line: `--no-config`
  gives back the removal.
- **Both classes off in the file is ignored, and said** at level 1 —
  `settings: correction.human and correction.ocr are both false, ignored` —: `--classes` could not write it, and it would
  leave common-errors nothing to do. Both classes are applied instead.
- **A number of lines out of 1 to 1000 is not read from the file at all**: the file's reader
  refuses it — `settings: correction.line-break.max-lines: cannot read 'N', default kept`, at
  level 1 — and the built-in 3 is used.

The examples on this page are run with `--no-config`, so that they say what the built-in
defaults do, whatever the settings of the machine that runs them.

| Option | Required | Values | Default |
| :----- | :----- | :------ | :----- |
| `<file>...` | yes | one or more paths | — |
| `--tasks` | **yes** | a comma-separated list: `mentions`, `join-words`, `split-words`, `common-errors`, `capitalization`, `line-break` | no task |
| `--code` | **yes** with a pattern task, and **not applicable** otherwise | `Script[-language[-COUNTRY]]`: `Zyyy`, `Latn`, `Latn-en`, `Latn-en-US` | — |
| `--language` | **yes** with `join-words` or `split-words`, and **not applicable** otherwise | a language code: `fr`, `fr_FR`, `en_US` | — |
| `--classes` | no | `human`, `ocr` or `human,ocr` | both — or what the settings file says |
| `--enable`, `--disable` | no, repeatable | the English name of a pattern, or `type:name` | the shipped `.conf` files |
| `--max-length` | **yes** with `line-break` | a number of **characters** above zero | **none** — see below |
| `--max-lines` | no | a whole number of lines, above zero | 3 — or what the settings file says |
| `--skip-length`, `--skip-lines` | no | a number, or `off` | the same bounds as `--max-length` and `--max-lines` |
| `--keep-blank-subtitles` | no | a flag | the emptied subtitles are removed — unless the settings file says otherwise |
| `--range` | no | `N-M`, or `N-` up to the end — see [`adjust`](adjust.md#--range) | the whole file |
| `--document` | no | `main` or `translation` — see [A translation](invocation.md#a-translation) | `main` |
| `-t`, `--translation-file` | with `--document translation` | a translation file, for **a single** input | — |
| `--align-method` | no, and only with `-t` | `position` or `number` | `position` |
| `--recursive`, `-r` | no | a flag | off — see [Processing a tree](lots.md) |
| `--output` / `--output-dir` / `--in-place` | **one of the three**, except with `--dry-run` | see [Invocation](invocation.md#the-destination) | — |
| `--dry-run` | no | a flag: computes and says, writes nothing — see [Seeing before writing](invocation.md#seeing-before-writing) | off |

### The tasks

**The order of the list makes no difference**: the tasks apply in
Gaupol's order — the mentions, then joining and splitting words, then the common errors, then capitalization, then line breaking —, each one on the
text the previous one left. A subtitle the mentions emptied plays no
part in what follows.

| Task | What it does |
| :---- | :-------------- |
| `mentions` | removes the sounds between square brackets, and between parentheses when `Sound in parentheses` is switched on (the sweep of [`hearing-impaired`](hearing-impaired.md)), then the lyrics and speaker patterns |
| `join-words` | glues back a word cut in two, from the dictionary of `--language` |
| `split-words` | splits two words stuck together, from the same dictionary |
| `common-errors` | corrects the common errors: spaces, punctuation, ligatures, OCR errors |
| `capitalization` | puts the capital where the language wants it |
| `line-break` | cuts the lines of a subtitle again so that none goes past `--max-length`, weighing the code's break patterns |

### The pattern code

**`--code` names the cascade**: `Zyyy` for any script, then the script, the language
and the country — `Latn-en-US` reads the patterns of `Zyyy`, of `Latn`, of `Latn-en` and of
`Latn-en-US`. **It is required**, because a command line has no ambient language:
a universal default would apply *less* than one believes, without saying so. It holds for
every task of the same run; a task that wants a code of its own is a second
run. A badly written code is a usage error:

<!-- exemple: printf '1\n00:00:01,000 --> 00:00:02,000\nBonjour\n\n' > a.srt; subedit-cli --no-config correct --tasks common-errors --code francais --output b.srt a.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nBonjour\n\n' > a.srt; subedit-cli --no-config correct --tasks common-errors --code francais --output b.srt a.srt; echo "code=$?"
--code: "francais" is not a pattern code: expected Script[-language[-COUNTRY]], like Zyyy, Latn or Latn-en-US
code=1
```

The patterns the user puts in `$XDG_DATA_HOME/subedit/patterns` (or
`~/.local/share/subedit/patterns`) **are added to the shipped patterns**, as in the
window, and a shipped pattern that cannot be read is said at level 1: `patterns: <file>, line N (<reason>)`.

### Joining and splitting words, and the dictionary

**These two tasks read no pattern**: they query the **system's dictionary, through
Enchant** — a dependency of the packages (`libenchant-2-2` on Debian and Ubuntu, `enchant2` on
Fedora), which `subedit-cli` shares with the window. They therefore have neither a code nor a name to set:
`--code` makes no sense without a pattern task, and `--language` none without one of these two
— one given without the other is a usage error, code `1`.

- **`join-words`** glues a word the dictionary does not know to the one that precedes or
  follows it, **when a single direction gives a known word**: `hel lo` becomes `hello`; two possible
  directions, or none, decide nothing.
- **`split-words`** splits a word the dictionary does not know **when exactly one of its
  suggestions is that word with a space**: `hellothere` becomes `hello there`. A word with an initial
  capital followed by small letters is left alone: it is most often a name.

**Without a dictionary for the language, the run is refused**, code `1`, **before a file is
read** and in the words of the window, which greys the function out and says so — here the command line
refuses and says so, rather than keeping quiet:

<!-- exemple: printf '1\n00:00:01,000 --> 00:00:02,000\nhel lo\n\n' > a.srt; subedit-cli --no-config correct --tasks join-words --language qq_QQ --output b.srt a.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\nhel lo\n\n' > a.srt; subedit-cli --no-config correct --tasks join-words --language qq_QQ --output b.srt a.srt; echo "code=$?"
no dictionary for qq_QQ
code=1
```

**The window's replacement list is neither read nor written.** It *fills up* through the
user's choices in the spell check; a command line has no such
choice, and reading the list would make a script depend on what its author once put in it.

### Line breaking

**It is measured in characters, and that is the difference from the window.** The window measures in *ems* —
a width calibrated on the font, at 0.55 em per letter —, and its default 24 is a value
in ems. A command line has no font: it counts characters, those of the
text. **Copying the 24 over as characters would cut almost everything, without
saying so**; there is therefore **no default length**, and `--max-length` is required with
`line-break`:

<!-- exemple: printf '1\n00:00:01,000 --> 00:00:03,000\nUn texte\n\n' > a.srt; subedit-cli --no-config correct --tasks line-break --code Latn-en --output b.srt a.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\nUn texte\n\n' > a.srt; subedit-cli --no-config correct --tasks line-break --code Latn-en --output b.srt a.srt; echo "code=$?"
--max-length is required by the task line-break: Gaupol's 24 is a width in ems, and has no value in characters
code=1
```

**`--max-lines` is 3**, Gaupol's value, which the unit does not change; line breaking takes
one more when three are not enough. **The skip** — a subtitle already within the limits is
left as it is — takes by default **the same bounds** (`--skip-length` = `--max-length`,
`--skip-lines` = `--max-lines`), as Gaupol does; `--skip-length off` and `--skip-lines off`
switch it off, and the text is then read again as one and cut again:

<!-- exemple: printf '1\n00:00:01,000 --> 00:00:03,000\nWhen the rain stopped, we walked home under the dark sky of the old town.\n\n' > a.srt; subedit-cli --no-config correct --tasks line-break --code Latn-en --max-length 24 --output b.srt a.srt; cat b.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:03,000\nWhen the rain stopped, we walked home under the dark sky of the old town.\n\n' > a.srt; subedit-cli --no-config correct --tasks line-break --code Latn-en --max-length 24 --output b.srt a.srt; cat b.srt
a.srt: Edited 1 and removed 0 subtitles -> b.srt
1
00:00:01,000 --> 00:00:03,000
When the rain
stopped, we walked
home under the dark
sky of the old town.
```

The break patterns are the code's: `Latn` does not break between a number and its unit nor
after a dialogue dash, `Latn-en` does not break after an article, a preposition, a title
or a possessive determiner. **Under `Zyyy`, no break pattern exists**: the lines are
only balanced. **This task is never refused for want of an active pattern**: its patterns
weigh where to break, the break happens without them. The four options only make sense with
`line-break`; given without it, they are a usage error.

### The classes, and switching by name

**`--classes` filters the common errors for good**: unticking `ocr` takes its
patterns out of the application — the ligature `ﬁ` then stays `ﬁ`. The other three types of patterns
carry no class.

**`--enable` and `--disable` set a pattern by its English name**, the name being the key
(ADR 0037); a name can designate several records, which then share a single
box. It is necessary, and it is what sets the window's defaults aside: **the
shipped `.conf` files switch the mention patterns off**, so that `--tasks mentions` alone
would apply nothing. The two patterns "Sound in brackets" and "Sound in parentheses"
each command their delimiter — the square brackets, the parentheses — and are set by the same
options: switching one alone on does not remove the other.

Three things are **usage errors**, code `1`, said before a file is read:

| What triggers it | Message |
| :------------------ | :------ |
| a name that designates no pattern of the tasks given, under the code given | `--enable: "<name>" names no pattern of the tasks given, under the code <code>` |
| a name that designates patterns of two types — it is then written `type:name`, the type being `common-error`, `capitalization`, `hearing-impaired` or `line-break` | `--enable: "<name>" names patterns of several types: write type:name, …` |
| a name given to `--enable` and to `--disable` | `"<name>" is given to --enable and to --disable: choose one` |

**A typing error is loud**: the window ignores a stale setting because a
setting can outlive a pattern that disappeared; an invocation has no survivor.

**A task without any active pattern is refused**, code `1` — "nothing to do" is an
answer that is said, not a silent success:

<!-- exemple: printf '1\n00:00:01,000 --> 00:00:02,000\n[Bruit] Bonjour\n\n' > a.srt; subedit-cli --no-config correct --tasks mentions --code Latn --output b.srt a.srt; echo "code=$?" -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\n[Bruit] Bonjour\n\n' > a.srt; subedit-cli --no-config correct --tasks mentions --code Latn --output b.srt a.srt; echo "code=$?"
mentions: no pattern is active under the code Latn; switch one on with --enable
code=1
```

The same file, with the square-bracket sweep switched on:

<!-- exemple: printf '1\n00:00:01,000 --> 00:00:02,000\n[Bruit] Bonjour\n\n' > a.srt; subedit-cli --no-config correct --tasks mentions --code Latn --enable "Sound in brackets" --output b.srt a.srt; cat b.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\n[Bruit] Bonjour\n\n' > a.srt; subedit-cli --no-config correct --tasks mentions --code Latn --enable "Sound in brackets" --output b.srt a.srt; cat b.srt
a.srt: Edited 1 and removed 0 subtitles -> b.srt
1
00:00:01,000 --> 00:00:02,000
Bonjour
```

## What is written, and what is said

The format, the encoding and the line endings of the file read are **kept**. **The
subtitles the correction empties are removed**, like the window's default
box; `--keep-blank-subtitles` leaves them empty. **An emptied translation stays emptied**,
and keeps its subtitle.

**The count is the window's** — "Edited N and removed M subtitles": texts
changed and subtitles removed, **never matches**. A file where nothing
changes is written all the same: a destination given is a destination written.

<!-- exemple: printf '1\n00:00:01,000 --> 00:00:02,000\neﬀet  de ﬁn\n\n2\n00:00:03,000 --> 00:00:04,000\nPropre.\n\n' > a.srt; subedit-cli --no-config correct --tasks common-errors --code Latn --output b.srt a.srt; cat b.srt -->
```console
$ printf '1\n00:00:01,000 --> 00:00:02,000\neﬀet  de ﬁn\n\n2\n00:00:03,000 --> 00:00:04,000\nPropre.\n\n' > a.srt; subedit-cli --no-config correct --tasks common-errors --code Latn --output b.srt a.srt; cat b.srt
a.srt: Edited 1 and removed 0 subtitles -> b.srt
1
00:00:01,000 --> 00:00:02,000
effet de fin

2
00:00:03,000 --> 00:00:04,000
Propre.
```

### A pattern that cannot apply

**A pattern that cannot be read, cannot be translated or does not finish is named, with the
subtitle where it gave up**, and this is where the command line says more than the
window: the confirmation page names the pattern only once, without the text.

| Reason | What it means |
| :----- | :------------------- |
| `cannot be translated` | its expression uses what the translation to ICU refuses |
| `will not compile` | the engine refused its expression |
| `has an invalid replacement` | its replacement does not read as Python reads it |
| `timed out` | the engine gave up on that text |
| `never settled` | `Repeat` was still changing the text after a hundred passes |
| `grew the text too long` | the text went past what a subtitle can hold |

At level 1, on the error output: 
`<path>: pattern "<name>" (<reason>) was not applied to subtitle N` — without the end when the pattern is set aside for every text. **The number is the
file's**, whatever tasks removed subtitles before. In JSON, it is
a `pattern-failed` warning of the file, whose `detail` carries the sentence. **A pattern
given up is not a failed file**: the file is written, and the exit code is `0`.

## Output

**Standard output** — nothing, the result being the file written. **With `--dry-run`, it is the
proposed changes**, one block per subtitle: its number, the text before, the text after —
and `(removed)` with no `+` line for a subtitle the correction would remove; with
`--keep-blank-subtitles`, it stays, empty. See [Seeing before writing](invocation.md#seeing-before-writing).

| Level | What is added on the error output |
| :----- | :------------------------------------- |
| 1 | `<path>: Edited N and removed M subtitles -> <destination>`, the patterns that could not apply, the pattern files that cannot be read — and, first, the settings file read — `settings: read <path>` — and what of it was ignored |
| 2 | `<path>: SubRip, UTF-8, no BOM, LF line endings kept` |
| 3 | `<path>: N bytes read, M written`, then every reading diagnostic |

In JSON, `counts` carries `corrected` (the texts changed) and `removed` (the subtitles
removed), and `changes` the subtitles touched. With `-t`, the `alignment` key says how the
translation was laid.
`settings` names the settings file that was read, or is `null` when none was.

## Exit codes

Those of the tool: `0` if every file was written, `2` if none, `3` if only
some, `1` on a usage error.

## Errors

| What triggers it | What is written, on the error output |
| :------------------ | :--------------------------------------- |
| a file named by `--config` that does not exist — the user's own file, missing, gives the defaults | `settings file not found: <FILE>` |
| `--tasks` missing | `--tasks is required`, followed by a pointer to `--help` |
| an unknown task | `--tasks: "<name>" is not a task: expected mentions, join-words, split-words, common-errors, capitalization or line-break` |
| `--code` missing | `--code is required by the tasks that read patterns: <tasks>` |
| `--code` without a pattern task | `--code is for the tasks that read patterns, and none was asked for` |
| `--language` missing with `join-words` or `split-words` | `--language is required by the tasks that check words: <tasks>` |
| `--language` that is not a language code | `--language: "<value>" is not a language: expected a locale code, like fr, en_US or sr@Latn` |
| `--language` without one of those two tasks | `--language is for the tasks join-words and split-words, which were not asked for` |
| a language without a dictionary | `no dictionary for <language>` |
| a badly written code | `--code: "<code>" is not a pattern code: expected Script[-language[-COUNTRY]], like Zyyy, Latn or Latn-en-US` |
| `--max-length` missing with `line-break` | `--max-length is required by the task line-break: Gaupol's 24 is a width in ems, and has no value in characters` |
| `--max-length` that is not a number above zero | `--max-length: "<value>" is not a length: expected a number of characters greater than zero` |
| `--max-lines`, `--skip-lines` that are not a whole number of lines | `--max-lines: "<value>" is not a number of lines: expected a whole number greater than zero` |
| `--skip-length` or `--skip-lines` unreadable | `--skip-length: "<value>" is not a bound: expected a number greater than zero, or off` |
| one of the four options without `line-break` | `--max-length is for the task line-break, which was not asked for` |
| an unknown class | `--classes: "<name>" is not a class: expected human, ocr or human,ocr` |
| a faulty pattern name | see the table above |
| a task without an active pattern | `<task>: no pattern is active under the code <code>; switch one on with --enable` |
| `--range`, `-t`, `--document`, the destination | those of [Invocation](invocation.md#errors) and of [A translation](invocation.md#a-translation) |
