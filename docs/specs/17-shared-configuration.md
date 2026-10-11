# Phase 17 — Settings shared by the window and the command line

**Issue:** [#699](https://github.com/Guyot-Bertrand/sub-edit/issues/699)
**Scheduled after phase 16.** The number identifies the spec and does not say its rank.

## Objective

Some settings belong to both surfaces: an operation's parameters are the same
whether it runs from the window or from the command line. Today the window
remembers the user's values (`Adjust Durations…`, `Correct Texts…`) and the
command line ignores them, so a user who tunes a dialog and then runs
`subedit-cli adjust` gets other values with nothing to say so.

This phase makes the settings file the shared home of the **operation
settings**, lets the command line read them, and gives it the commands to look at
and change them. The display preferences stay the window's business.

## Decisions

**D1 — The command line reads the file by default.** `--no-config` switches it
off; `--config FILE` reads another file. The two exclude each other. Chosen by the
maintainer over an opt-in flag: a setting made in the window or by `config set`
takes effect at once, and the cost is that one command line can answer differently
on two machines. The cost is paid down by D6.

**D2 — Priority, strongest first: option on the command, then the file, then the
core default.** An option always wins. `--speed off` beats a speed in the file.

**D3 — Two parts in the core, one file, no migration.**

- `Settings` stays one flat struct. `OperationSettings` is a slice of it
  (`durationAdjustment` and `correction`), read through `readOperationSettings`.
  Splitting the struct would touch every GUI call site and every
  designated-initialiser test for no extra guarantee: the command line is only
  given the slice. `spellCheck` has no command-line equivalent and stays on the
  display side with geometry, columns, theme, last directory, table share, insert
  placement, write encoding, search options, editor and video. #697 and #698
  introduced no new field in `Settings` yet, so the D5 audit finds nothing to move
  for them.
- **The keys do not change.** They are already prefixed by family
  (`duration-adjust.*`, `correction.*`), so an existing file is read as it is.
- On writing, the two parts are two blocks, each under a banner comment. The
  reader stays tolerant and takes keys in any order. No `[section]` headers: they
  would break the reading of every existing file.
- `readOperationSettings` returns the operation part only. That the command line
  never reads the display preferences is a property of the type.
- ADR 0022's rules hold unchanged: unknown key ignored, unreadable value keeps the
  default and produces a diagnostic, absent option takes the default, an option at
  its default is rewritten commented, a missing file is not an error.

**D4 — The path is resolved in one place.** `gui::userSettingsPath()` depends on
Qt. The resolution moves to `subedit_platform` (Qt-free: `$XDG_CONFIG_HOME`, else
`~/.config`, then `subedit/settings.conf`), and the window and the command line
call the same function. Only the two `main` functions resolve a path and hand it
down. Tests receive a path and never resolve one (`check-config-home.sh`).

**D5 — Which settings are shared.** An audit of every field of `Settings` and
every option of every command, written into the plan before any code moves. A
setting is operation-level if a command line option sets the same thing.

**D6 — Reading is announced.** At the first verbosity level a command says which
file it read, or that there was none. With `--format json` the record carries the
path read, so a reader can reproduce a result.

**D7 — A catalogue of the operation keys in the core.** One table: key, short
description, accepted values, and who uses it (the command-line commands and the
window actions that read the key). The default is not written a second time: it is
derived from what the file writes for a fresh `Settings`, so it cannot drift. The core already defines the keys and the
defaults; the catalogue adds the description and the accepted-values text. A test
checks that every key the reader recognises is in the catalogue and the other way
round, so the table cannot drift from the reader. The descriptions are English, in
the core, so that the manual can be checked against them.

**D8 — What the command line honours.** The catalogue carries a `surface` per
key: `Both` or `Window only`. `config list` prints it with the other columns of D7.

| Surface | Keys |
| :------ | :--- |
| Both (13) | the nine `duration-adjust.*` keys (`gap-enabled`, `gap-ms`, `lengthen`, `maximum-enabled`, `maximum-ms`, `minimum-enabled`, `minimum-ms`, `shorten`, `speed`), `correction.human`, `correction.ocr`, `correction.remove-blank`, `correction.line-break.max-lines` |
| Window only | every other `correction.*` key |

`correct` takes `--tasks`, `--code`, `--max-length` and `--language` from its own
line on purpose: the length unit differs and nothing is ticked beforehand.
`correction.activations` is not in the catalogue: it is a list the window manages,
and the command line has `--enable` and `--disable`.

For `adjust`, `--speed off` switches the speed off; the reading speed's directions
come from the file unless a flag changes them.

## The `config` commands

`show`, `set` and `unset` act on the user's file, or on `--config FILE`; `list`
reads no file. Only operation keys are accepted. A display key is refused with "display preference, set it in the
window"; an unknown key is a usage error.

| Command | Does | Notes |
| :------ | :--- | :---- |
| `config list` | Writes every operation key, whether or not the file defines it, with no value: the key, what it is for, what it accepts (the closed set, or the bounds and the form), its default, and the commands and window actions that use it. | An information command for the user. `--format json` gives one object per key. Reads no file and needs none; works with `--no-config`. |
| `config show` | Writes the path read, whether the file exists, then each operation setting with its value, its source (`file` or `default`) and its default, in every case. | `--format json` gives one object. Display preferences are not listed. |
| `config set KEY VALUE` | Validates `VALUE` with the file's own reader, then rewrites the file atomically. | An unreadable value is a usage error and leaves the file untouched. Display preferences already in the file are kept. |
| `config unset KEY` | Puts `KEY` back to its default, written commented as the format does. | Unsetting a key already at its default succeeds and changes nothing. |

**One catalogue behind all four** (D7). `list` prints it, `show` walks it for the
values, `set` and `unset` use it to know the keys and to validate. A key therefore
cannot be settable and undocumented, nor documented and unknown.

`set` and `unset` create the file and its directory when absent. Exit codes are the
four of the manual: a refused key or value is a usage error, an unwritable file is
the I/O error.

## Out of scope

- Display preferences on the command line.
- The interface language, which comes from the environment (#658).
- Any setting that has no command line equivalent.
- `correction.activations`, a list managed by the window.

## Done when

- one definition and one default per operation setting, read by the window and by
  the command line;
- the display preferences are not read by the command line;
- tests cover the priority (option > file > default), `--no-config`, `--config`, a
  missing or unreadable file, an existing file read unchanged, and `config list`
  (every catalogue key, no value, none missing), `show`, `set` and `unset`
  including their refusals;
- the CLI and GUI manuals say where the file is, what it holds and how a command
  reads it;
- ADR 0045 records the two parts, the priority and the default reading.
