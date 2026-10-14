# Invocation

```
subedit-gui [--version] [<file>]
```

| Argument | Required | Value | Default |
| :------- | :------- | :---- | :------ |
| `<file>` | no | the path of a subtitle file to open | none — the window opens empty |
| `--version` | no | — | — |

**A single file.** The following arguments are ignored: the window holds
only one project at a time.

## `--version`

Writes `subedit <version>` on standard output and returns at once, without
opening a window. Exit code `0`.

```console
$ subedit-gui --version
subedit <version>
```

The number is not copied here: this manual has no generated block that would
keep it up to date, and a frozen number would be wrong from the next increment.

## Opening a file

```console
$ subedit-gui film.srt
```

The formats read are **the nine** that the open filter lists — see
[Opening and saving](files.md#opening). The format is recognised by the content and
not by the extension, and **so is the encoding**: a BOM
if the file carries one, otherwise the encoding its bytes suggest.

## When opening fails

A missing file, an unreadable one, one whose bytes decode in no
encoding, or one whose content no format recognises: **the window opens
all the same, empty**, and the failure is written on the error stream.

**The message names the cause**, in the same words as the command line:

| Message | What happened |
| :------ | :------------ |
| `does not exist` | the path designates no file |
| `cannot be opened: permission denied` | the system refuses to open it |
| `cannot be read` | the system refused for another reason |
| `cannot be decoded in the chosen encoding` | the bytes do not read in the encoding retained |
| `is in no format this tool knows` | no format recognises the content |
| `holds nothing recognisable as a subtitle` | the format is recognised, but nothing in it is a subtitle |

```console
$ subedit-gui notes.txt
subedit-gui: notes.txt: is in no format this tool knows
```

Opening empty rather than refusing to start: the tool is still good for
something, and the failure of an argument is no reason not to launch.

**A badly formed but readable file opens**, and what reading caught up
along the way can be read in a button of the status bar — see
[the diagnostics of a read](files.md#the-diagnostics-of-a-read).

## Exit codes

| Code | When |
| :--- | :--- |
| `0` | the window opened then was closed normally, or `--version` answered |
| `2` | an unexpected error before opening — Qt missing, display unavailable |

The failure to open a file **does not change the exit code**: the
window opened, and that is what the code reports.

## The manual page

An installation lays down `subedit-gui(1)` under `share/man/man1`:

```console
$ man subedit-gui
```

It gives the calling form, `--version`, the opening failure messages, the
exit codes, the two environment variables and the settings file.
**It does not describe the window** — this manual does, and the page
refers here.

## The Qt platform

`QT_QPA_PLATFORM` chooses what the window runs on. **If the variable is
set, `subedit-gui` respects it without argument.**

If it is not, it lets Qt choose: the built-in player draws the picture in
the window itself, and asks nothing of the platform — see
[The player](player.md).

`QT_QPA_PLATFORM=offscreen` runs the window where there is no
graphical server — which is what continuous integration and the tests do.

## Language

The window speaks the language of its environment, **chosen once, when it starts**: changing the
variables of a running window changes nothing until the next launch. The variables and the rules
are those of the command line — `LANGUAGE`, `LC_ALL`, `LC_MESSAGES`, `LANG`, `LC_ALL=C` for
English, English **without a word** when the language asked for is not installed — described in
[the command line's manual](../subedit-cli/invocation.md#language). The catalogues are the same
files, installed under `<prefix>/share/subedit/locale`.

**A language chosen in `Edit ▸ Preferences…` comes first**, and the environment is read only
when it is `System default`, which it is until one is chosen: an installed language chosen there
wins over every variable, `LC_ALL=C` included. The choice, too, applies at the next launch. See
[The interface language](preferences.md#the-interface-language).

| What | Follows the language |
| :--- | :------------------- |
| the menus and their entries, with their underlined letters; the toolbar; the tool tips | yes |
| the columns of the table and of `View ▸ Columns`, the tabs, the status line, the play bar | yes |
| the standard buttons Qt draws itself (`OK`, `Cancel`) | yes, when Qt's own catalogue of that language is installed (`qtbase_<language>.qm`); English otherwise |
| the dialogs, the pages of `Correct Texts…`, the file dialogs' titles and filters, the message boxes and what they say | yes |
| the lines the window writes on its error stream, `subedit-gui: …` | yes, the program's name excepted |
| the names of the correction patterns in `Correct Texts…`, and their descriptions in the tool tips | yes, for the patterns shipped with the program; a pattern of your own is shown as its file writes it |
| the names of encodings, line endings and spelling languages (`UTF-8`, `CRLF`, `en_US`) | never: they are names, not words |
| the keyboard shortcuts | never: `Ctrl+S` is `Ctrl+S` in every language |
| a message the catalogue does not translate | shown in English |

This version installs `fr`, French, translated in full: on a French desktop the menu bar reads
`&Fichier`, `É&dition`, `&Affichage`, `&Vidéo`, `&Outils`, `&Projets`, `Aid&e`. The captures of
this manual are taken in English.
