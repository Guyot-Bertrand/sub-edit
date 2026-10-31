# `config`

```
subedit-cli [--config FILE | --no-config] [--format text|json] config list
subedit-cli [--config FILE | --no-config] [--format text|json] config show
subedit-cli [--config FILE] config set KEY VALUE
subedit-cli [--config FILE] config unset KEY
```

Lists, shows and changes the **operation settings**: the part of the settings file that the
window and the command line share. `Adjust Durations…` and `Correct Texts…` remember their
values there, and [`adjust`](adjust.md) and [`correct`](correct.md) read them by default — see
[The settings file](invocation.md#the-settings-file).

**What it does not do:** it never reads nor writes a **display preference** (the window's
geometry, columns, theme, search options, spell-check, video…). Those stay the window's
business, set in the window and described in [its manual](../subedit-gui/preferences.md#the-file).
It never touches a subtitle file either.

<!-- example: subedit-cli config --help -->
```console
$ subedit-cli config --help
List, show and change the operation settings
Usage: subedit-cli config [OPTIONS] SUBCOMMAND

Options:
  -h,--help                   Print this help message and exit

Subcommands:
  list                        Write every operation key, what it is for, what it accepts and its default
  show                        Write the settings file used, then every operation key with its value
  set                         Write KEY = VALUE into the settings file, keeping every other setting
  unset                       Put KEY back to its default in the settings file, keeping every other setting
```

## Sub-commands

| Sub-command | What it does | File |
| :---------- | :----------- | :--- |
| `config list` | writes every operation key, with what it is for, what it accepts, its default and who uses it — **no value** | reads none |
| `config show` | writes the file used and whether it exists, then every operation key with its value, where the value comes from, and its default | reads it |
| `config set KEY VALUE` | checks `VALUE`, then rewrites the file with `KEY = VALUE`, keeping every other setting | reads and writes it |
| `config unset KEY` | puts `KEY` back to its default, written commented out as the file always writes a default | reads and writes it |

| Option | Required | Values | Default |
| :----- | :------- | :----- | :------ |
| `KEY` | yes, for `set` and `unset` | an operation key, as `config list` names it | — |
| `VALUE` | yes, for `set` | what `config list` says the key accepts | — |

`config` has no option of its own. The global options written **before** `config` choose the
file and the form of the output:

| Global option | Effect on `config` |
| :------------ | :----------------- |
| `--config FILE` | `show`, `set` and `unset` act on `FILE` instead of the user's file, even when it does not exist yet; `list` ignores it |
| `--no-config` | `list` works as usual; `show` shows the defaults, with `File: none (--no-config)`; `set` and `unset` are refused: there is no file to write to |
| `--format json` | `list` and `show` write one JSON object a line; `set` and `unset` write nothing on standard output |
| `-q`, `--quiet` | `set` and `unset` no longer confirm what they wrote |

## Which file

Without `--config`, the user's file:

```
$XDG_CONFIG_HOME/subedit/settings.conf
```

or `~/.config/subedit/settings.conf` when `XDG_CONFIG_HOME` is unset, empty or not an absolute
path. It is the file the window reads and writes; `config show` writes its path on its first line:

<!-- example: XDG_CONFIG_HOME=/home/vous/.config subedit-cli config show | head -1 -->
```console
$ XDG_CONFIG_HOME=/home/vous/.config subedit-cli config show | head -1
File: /home/vous/.config/subedit/settings.conf (not found)
```

When neither `XDG_CONFIG_HOME` nor `HOME` gives a place, there is no user's file: `show`, `set`
and `unset` are refused, and `--config FILE` names one.

## `config list`

**The catalogue of the operation keys**, in the order of the file: each key, then four indented
lines — what it is for, `Accepts:` (the closed set, or the form and the bounds), `Default:` and
`Used by:` (the `subedit-cli` commands and the window actions that read it). It reads no file and
needs none, and works with `--no-config`.

<!-- example: subedit-cli config list | head -10 -->
```console
$ subedit-cli config list | head -10
duration-adjust.speed
    Reading speed the durations are brought to.
    Accepts: a number of characters per second, above 0, decimals allowed
    Default: 15
    Used by: subedit-cli adjust; Adjust Durations…
duration-adjust.lengthen
    Let the reading speed move an end later.
    Accepts: true or false
    Default: true
    Used by: subedit-cli adjust; Adjust Durations…
```

It lists **45 keys**:

<!-- example: subedit-cli config list | grep -c '^[a-z]' -->
```console
$ subedit-cli config list | grep -c '^[a-z]'
45
```

**Twenty-five of them are read by the command line**; the twenty others are read by the window only,
and their `Used by:` line says so, with the option of `correct` that takes their place:

| Key | Read by |
| :-- | :------ |
| `duration-adjust.speed`, `duration-adjust.lengthen`, `duration-adjust.shorten` | [`adjust`](adjust.md) and `Adjust Durations…` |
| `duration-adjust.minimum-enabled`, `duration-adjust.minimum-ms` | `adjust` and `Adjust Durations…` |
| `duration-adjust.maximum-enabled`, `duration-adjust.maximum-ms` | `adjust` and `Adjust Durations…` |
| `duration-adjust.gap-enabled`, `duration-adjust.gap-ms` | `adjust` and `Adjust Durations…` |
| `correction.human`, `correction.ocr` | [`correct`](correct.md) and `Correct Texts…` |
| `correction.remove-blank` | `correct` and `Correct Texts…` |
| `correction.line-break.max-lines` | `correct` and `Correct Texts…` |
| `transcription.engine`, `transcription.executable`, `transcription.model`, `transcription.vad-model`, `transcription.language`, `transcription.suppress-notes`, `transcription.merge-gap-ms`, `transcription.max-duration-ms`, `transcription.max-characters` | [`transcribe`](transcribe.md) and `Transcribe Audio…` |
| `alignment.engine`, `alignment.min-match`, `alignment.doubt-gap` | [`sync`](sync.md) and `Synchronize with Audio…` |
| every other `correction.*` key | `Correct Texts…` only — `correct` takes `--tasks`, `--code`, `--max-length`, `--skip-length`, `--skip-lines`, `--language`, `--enable` and `--disable` from its own line |

The numbers of lines, `correction.line-break.max-lines` and `correction.line-break.skip-max-lines`,
are whole numbers from 1 to 1000 — the bounds of `correct --max-lines` —, and the line lengths,
`correction.line-break.max-length` and `correction.line-break.skip-max-length`, are numbers above
0: `set` refuses the rest, and a file edited by hand that holds one keeps the default.

`correct` takes the tasks, the code and the line length from its own line on purpose: nothing is
ticked beforehand on a command line, and the window measures lines in ems where the command line
counts characters. `correction.activations`, the list of pattern boxes the window manages, is not
a key of the catalogue: `correct` has `--enable` and `--disable`.

With `--format json`, one object per key — `key`, `description`, `accepts`, `default`, `used_by`,
and `surface`, `"both"` or `"window-only"`:

<!-- example: subedit-cli --format json config list | head -1 -->
```console
$ subedit-cli --format json config list | head -1
{"schema":1,"command":"config","key":"duration-adjust.speed","description":"Reading speed the durations are brought to.","accepts":"a number of characters per second, above 0, decimals allowed","default":"15","used_by":"subedit-cli adjust; Adjust Durations…","surface":"both"}
```

## `config show`

**The file used, then every operation key with its value**: `File: <path> (found)`,
`(not found)` or `(unreadable: <reason>)`, then one line per key —
`[file]` and the default when the file sets it, `[default]` when it does not. A key the file
holds with a value it cannot read shows its default. Display preferences are not listed.

<!-- example: subedit-cli --config mine/settings.conf config set duration-adjust.speed 12; subedit-cli --config mine/settings.conf config show | head -4 -->
```console
$ subedit-cli --config mine/settings.conf config set duration-adjust.speed 12; subedit-cli --config mine/settings.conf config show | head -4
duration-adjust.speed = 12
File: mine/settings.conf (found)
duration-adjust.speed = 12   [file]   (default 15)
duration-adjust.lengthen = true   [default]
duration-adjust.shorten = false   [default]
```

A missing file is not an error: `show` writes the defaults, and the code is `0` — the user's
file as a `FILE` named by `--config`, which [`adjust`](adjust.md) and [`correct`](correct.md), on
the contrary, refuse (see [The settings file](invocation.md#the-settings-file)). With
`--no-config`:

<!-- example: subedit-cli --no-config config show | head -2 -->
```console
$ subedit-cli --no-config config show | head -2
File: none (--no-config)
duration-adjust.speed = 15   [default]
```

With `--format json`, a first object `{"file", "exists"}` — `file` is `null` with `--no-config` —
then one object per key: `key`, `value`, `source` (`"file"` or `"default"`), `default`,
`used_by`, `surface`.

<!-- example: subedit-cli --config mine/settings.conf config set duration-adjust.speed 12; subedit-cli --format json --config mine/settings.conf config show | head -2 -->
```console
$ subedit-cli --config mine/settings.conf config set duration-adjust.speed 12; subedit-cli --format json --config mine/settings.conf config show | head -2
duration-adjust.speed = 12
{"schema":1,"command":"config","file":"mine/settings.conf","exists":true}
{"schema":1,"command":"config","key":"duration-adjust.speed","value":"12","source":"file","default":"15","used_by":"subedit-cli adjust; Adjust Durations…","surface":"both"}
```

## `config set` and `config unset`

**`set` checks the value with the file's own reader** — the reader the window uses — and refuses
it when it cannot be read, before anything is written. **`unset`** writes the default back,
commented out: the key then follows the default even if a later version changes it. Unsetting a
key already at its default succeeds and changes nothing.

Both **rewrite the file whole and atomically**: every display preference already in it is kept,
the operation block comes first and the display block second, each under its banner. **A value
the file could not read, and an unknown key, are not kept** — the rewrite is the window's, which
drops them too. **Each value dropped is said**, on the standard error at the default level, display
preferences included: `<key>: unreadable value '<value>' replaced by its default`. The file and
its directory are created when missing.

The confirmation goes to the standard error, at the default level: `KEY = VALUE` for `set`,
`KEY reset to its default (DEFAULT)` for `unset`.

<!-- example: subedit-cli --config mine/settings.conf config set duration-adjust.gap-ms 100; grep -e '^[a-z]' -e '^# [A-Z][a-z]* [a-z]*:' mine/settings.conf; subedit-cli --config mine/settings.conf config unset duration-adjust.gap-ms; grep gap-ms mine/settings.conf -->
```console
$ subedit-cli --config mine/settings.conf config set duration-adjust.gap-ms 100; grep -e '^[a-z]' -e '^# [A-Z][a-z]* [a-z]*:' mine/settings.conf; subedit-cli --config mine/settings.conf config unset duration-adjust.gap-ms; grep gap-ms mine/settings.conf
duration-adjust.gap-ms = 100
# Operation settings: shared by the window and the command line (subedit-cli reads these).
duration-adjust.gap-ms = 100
# Display preferences: the window's own, the command line never reads them.
duration-adjust.gap-ms reset to its default (0)
#duration-adjust.gap-ms = 0
#transcription.merge-gap-ms = 400
```

## Exit codes

| Code | Meaning |
| :--- | :------ |
| `0` | done — including `show` on a missing file and `unset` on a key at its default |
| `1` | usage error: no sub-command, a missing argument, a refused key or value, `--no-config` with `set` or `unset`, `--config` with an empty name, no user's file and no `--config`; nothing is written |
| `2` | the file cannot be read or written; nothing is written |

## Errors

| What triggers it | What is written, on the standard error |
| :--------------- | :------------------------------------- |
| a display preference | `<key> is a display preference; set it in the window` |
| an unknown key | `unknown setting <key>; subedit-cli config list names them` |
| a value the key does not accept | `cannot read '<value>' for <key>; subedit-cli config list says what it accepts` |
| `correction.activations` | `correction.activations is managed by the window; use --enable/--disable on correct` |
| `set` or `unset` with `--no-config` | `--no-config has nothing to write to` |
| no user's file can be located, and no `--config` | `no settings file could be located; give one with --config FILE` |
| `--config` with `--no-config` | `--config excludes --no-config` |
| `--config` with an empty name | `--config needs a file name` |
| a file that exists and cannot be read, for `set` or `unset` | `<path>: cannot be read; nothing written` |
| a file that cannot be written | `<path>: cannot be written`, or `<path>: cannot be written: permission denied` |

<!-- example: subedit-cli --config mine/settings.conf config set general.theme dark; echo "code=$?"; subedit-cli --config mine/settings.conf config set duration-adjust.speed fast; echo "code=$?"; subedit-cli --no-config config unset duration-adjust.speed; echo "code=$?" -->
```console
$ subedit-cli --config mine/settings.conf config set general.theme dark; echo "code=$?"; subedit-cli --config mine/settings.conf config set duration-adjust.speed fast; echo "code=$?"; subedit-cli --no-config config unset duration-adjust.speed; echo "code=$?"
general.theme is a display preference; set it in the window
code=1
cannot read 'fast' for duration-adjust.speed; subedit-cli config list says what it accepts
code=1
--no-config has nothing to write to
code=1
```
