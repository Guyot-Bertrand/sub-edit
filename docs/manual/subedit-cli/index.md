# subedit-cli — manual

The command-line tool of subedit: working on subtitle files without a graphical
interface, in batches or from a script. For the other program and to know
where to begin, see
[the manual](../index.md).

> **Current state.** The seventeen subcommands of `subedit-cli` exist: inspect,
> convert, shift, transform, re-time the frame rate, snap onto a frame grid, remove
> the mentions for the hearing impaired, adjust the durations, replace a text, change the case,
> the italics and the dialogue dashes, put the subtitles back in order, correct the
> texts with Gaupol's patterns, put files one after another, cut a
> file in two, and write a translation at the positions of its main file. This manual
> describes what exists, never what is planned; what comes next is in
> the [roadmap](../../feuille-de-route.md).

## Sections

| Section | Content |
| :------ | :------ |
| [Installation](installation.md) | building and installing the tool |
| [Invocation](invocation.md) | global options, outputs, exit codes |
| [The settings file](invocation.md#the-settings-file) | what `adjust` and `correct` read from the window's settings file, `--config`, `--no-config` |
| [Processing a tree](lots.md) | several files, a whole directory, `--recursive` |
| [`inspect`](inspect.md) | report what a file is made of |
| [`convert`](convert.md) | write a file in another format or another shape |
| [`shift`](shift.md) | move every position by the same amount |
| [`transform`](transform.md) | correct every position from two points known to be right |
| [`framerate`](framerate.md) | re-time a file from one frame rate to another |
| [`snap`](snap.md) | put the timestamps back on the frames of a frame rate |
| [`hearing-impaired`](hearing-impaired.md) | remove the mentions for the hearing impaired |
| [`adjust`](adjust.md) | adjust the durations to a reading speed and to limits |
| [`replace`](replace.md) | replace a text without breaking a tag |
| [`case`](case.md) | put the texts in title, sentence, upper or lower case |
| [`italics`](italics.md) | put the texts in italics, or take their italics out |
| [`dialogue-dashes`](dialogue-dashes.md) | put dialogue dashes on, or take them off |
| [`sort`](sort.md) | put the subtitles back in the order of their start |
| [`join-repeats`](join-repeats.md) | make a single subtitle of the pieces of one text that follow each other |
| [`disjoint`](disjoint.md) | cut the overlapping subtitles so that no range overlaps another any more |
| [`repair`](repair.md) | chain `join-repeats`, `sort` and `disjoint` on a file that is badly segmented, out of order and overlapping |
| [`correct`](correct.md) | correct the texts with Gaupol's patterns: mentions, joining and splitting words, common errors, capitalization, line breaking |
| [`append`](append.md) | put files after the first one, into a single file |
| [`split-file`](split-file.md) | cut a file in two, the tail brought back to the origin |
| [`pair`](pair.md) | write a translation at the positions of its main file, and say how its lines lined up |
| [`config`](config.md) | list, show and change the operation settings shared with the window |
