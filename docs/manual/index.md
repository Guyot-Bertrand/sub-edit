# subedit — manual

subedit reads, inspects and re-times subtitle files. It comes as two
programs that share the same core, and so the same rules and the same words.

| Program | What for | Its manual |
| :-------- | :-------------- | :--------- |
| `subedit-gui` | open a file, see it in a table, edit it, watch it against the film | [the window's manual](subedit-gui/index.md) |
| `subedit-cli` | process files in batches, from a script, without an interface | [the command-line manual](subedit-cli/index.md) |

Both read and write **the nine formats** — SubRip, WebVTT, SubViewer 2,
Sub Station Alpha, Advanced SSA, MicroDVD, MPL2, TMPlayer, LRC — and
recognise the format by its content rather than by its extension. **The encoding too
is recognised by the content**: a file in Latin-1, in CP1252 or in UTF-16
opens, what it was read as is said, and it is written back in its own — a
file opened then saved without a change does not move by a byte.

## Where to begin

| What you want | Where to go |
| :----------------- | :------- |
| install the tool | [Installation](subedit-cli/installation.md) — it holds for both programs |
| open a file and look at what it holds | [The table](subedit-gui/table.md) |
| re-time subtitles shifted by a constant amount | [The operations](subedit-gui/operations.md#shift-positions), or [`shift`](subedit-cli/shift.md) |
| re-time a file written for another frame rate | [`framerate`](subedit-cli/framerate.md), and [`snap`](subedit-cli/snap.md) if the timing is already right |
| know which frame rate a file was written against | [The frame grid](subedit-gui/grille.md), or [`inspect`](subedit-cli/inspect.md) |
| remove the mentions for the hearing impaired | [The operations](subedit-gui/operations.md#remove-hearing-impaired-mentions), or [`hearing-impaired`](subedit-cli/hearing-impaired.md) |
| correct mentions, common errors, case and line breaking in a single pass | [`Correct Texts…`](subedit-gui/correct-texts.md), or [`correct`](subedit-cli/correct.md) |
| check the spelling word by word, with the system's dictionaries | [`Check Spelling…`](subedit-gui/verifier-orthographe.md) |
| lengthen subtitles too short to be read | [`Adjust Durations…`](subedit-gui/operations.md#adjust-durations), or [`adjust`](subedit-cli/adjust.md) |
| put back in order a file whose subtitles are not | [`sort`](subedit-cli/sort.md) |
| make a single subtitle of the pieces of one text that follow each other | [`Join Repeated Subtitles`](subedit-gui/lignes.md#fusionner-les-sous-titres-répétés), or [`join-repeats`](subedit-cli/join-repeats.md) |
| straighten out, in one go, a file that is badly segmented, out of order and overlapping | [`repair`](subedit-cli/repair.md) |
| remove the overlaps between subtitles | [`Make Ranges Disjoint…`](subedit-gui/lignes.md#rendre-les-plages-disjointes), or [`disjoint`](subedit-cli/disjoint.md) |
| put the two parts of a film end to end | [`Append File…`](subedit-gui/operations.md#append-file), or [`append`](subedit-cli/append.md) |
| cut a film in two parts | [`Split Project…`](subedit-gui/operations.md#split-project), or [`split-file`](subedit-cli/split-file.md) |
| change the case, the italics or the dialogue dashes of a whole file | [The operations](subedit-gui/operations.md#italic), or [`case`](subedit-cli/case.md), [`italics`](subedit-cli/italics.md), [`dialogue-dashes`](subedit-cli/dialogue-dashes.md) |
| look for a word and replace it, without breaking the tags | [Find and replace](subedit-gui/recherche.md), or [`replace`](subedit-cli/replace.md) |
| translate alongside the original text | [Opening a translation](subedit-gui/fichiers.md#ouvrir-une-traduction), or [`pair`](subedit-cli/pair.md) to re-time it on the main file |
| process a hundred files at once | [Processing a tree](subedit-cli/lots.md), then [Invocation](subedit-cli/invocation.md) |

**Between `framerate` and `snap`, a mistake goes unnoticed.** If
you hesitate, [`snap`](subedit-cli/snap.md#snap-is-not-framerate-and-mixing-them-up-does-not-show)
opens on the comparison of the two.

## What this manual promises

**It describes what exists, never what is planned.** What will come, and in
what order, is in the [roadmap](../feuille-de-route.md).

**This manual is written in English, and translated.** Its translations follow it
page for page, under the repository's `docs/i18n/` directory, one tree per language. The
programs speak the language of their environment; the names they show — subcommands,
options, menu labels — are quoted here as the program writes them in English.

**The examples and the pictures are generated, the prose is not.** The
`console` blocks are produced by actually running the command; the
screenshots, by building the real window and photographing it. Neither
of the two can therefore describe a tool that no longer exists. What remains without a net
is the text around them, whose correctness rests on proofreading.

**The screenshots show both palettes**, `Light` and `Dark` — those
[the preferences](subedit-gui/preferences.md#le-thème) offer.

## The two surfaces say the same words

One reason to read either manual when using the other: the
error messages, the format names, the grid verdicts and the undo
labels are written **once**, in the core, and both programs
quote them. A refused file gives the same sentence on both sides.

What differs is what each surface can do:

| | `subedit-gui` | `subedit-cli` |
| :--- | :------------ | :------------ |
| several files at once | yes — one tab per project | yes |
| a translation alongside the main file | yes | yes — `-t` on the text subcommands, and [`pair`](subedit-cli/pair.md) to re-time it |
| append a file, split a project | yes | yes — [`append`](subedit-cli/append.md), [`split-file`](subedit-cli/split-file.md) |
| undo | yes, a thousand entries | not applicable — nothing is changed in place without asking |
| edit a text or a position by hand | yes | no |
| insert, delete, merge and split lines | yes | no |
| find and replace | yes | yes — [`replace`](subedit-cli/replace.md) |
| cut, copy and paste texts | yes | no |
| adjust the durations, the italics, the case, the dashes | yes | yes — [`adjust`](subedit-cli/adjust.md), [`italics`](subedit-cli/italics.md), [`case`](subedit-cli/case.md), [`dialogue-dashes`](subedit-cli/dialogue-dashes.md) |
| correct the texts by patterns | yes — `Correct Texts…` | yes — [`correct`](subedit-cli/correct.md) |
| check the spelling word by word | yes — `Check Spelling…` | no |
| watch the film while timing | yes | no |
| write over the input | `Save` | `--in-place` |
