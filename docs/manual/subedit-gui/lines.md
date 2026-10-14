# Inserting, removing, merging, splitting, sorting and making lines disjoint

Eight entries of the `Edit` menu, under a separator that sets them apart from
`Find and Replace…`. Four have in common that they change the number of lines, where
cut, copy and paste, above, move texts; `Sort Subtitles` changes
only **the order** of the lines, `Join Repeated Subtitles` makes a single
subtitle again of the consecutive pieces of one same text, and `Make Ranges Disjoint…`
re-cuts them where they overlap.

| Entry | Shortcut | What it does |
| :---- | :------- | :----------- |
| Insert Subtitles… | `Ins` | opens a dialog, then adds blank lines |
| Remove Subtitles | `Del` | removes the selection, without asking anything |
| Merge Subtitles | none | makes a single line of a block of neighbouring lines |
| Split Subtitle | none | cuts a line in two in the middle of its duration |
| Join Repeated Subtitles | none | makes a single subtitle of the consecutive pieces of one same text |
| Sort Subtitles | none | puts the lines back in the order of their start, over the whole project |
| Make Ranges Disjoint… | none | cuts the overlapping lines so that no range overlaps another any more |
| Repair Subtitles… | none | chains the three preceding entries — joining the repeats, sorting, cutting the ranges — in one go |

All eight enter the history: `Ctrl+Z` undoes them like the rest.

## Inserting

![The insertion dialog, light palette: the number of lines, and the side of
the selection where they will go.](captures/insertion.png)

![The same dialog under the dark palette.](captures/insertion-sombre.png)

| Field | What it holds | Default |
| :---- | :------------ | :------ |
| `How many` | a whole number from 1 to 99,999 | `1` |
| `Where` | `Above the selection` or `Below the selection` | the last choice, `Below` on first launch |

`OK` adds the lines, `Cancel` adds nothing.

### Where they go

**At the last one selected**, not the first. Selecting lines 1 and 3
then inserting below puts the new line in position 4 — not in
position 2. That is what Gaupol has done for twenty years, and what the hand
expects after sweeping from top to bottom.

`Above the selection` puts it just before that same line.

### How long they last

A blank line has no text, but it has positions. They are deduced
from the space available:

| Situation | Start | Duration |
| :-------- | :---- | :------- |
| between two subtitles | the end of the one before | the space until the next one, shared in equal parts |
| after the last one | the end of the last one | three seconds each |
| at the head of a file | the origin | the space until the first one, shared |
| in an empty file | the origin | three seconds each |

When the file overlaps at the chosen place, there is **no space to
share**: the lines receive a zero duration, which the table shows, rather
than an invented duration that would go over the neighbour.

### The empty document

**No selection is required**, and it is the only way to start a
new file: the lines go to index zero. The choice of side is then disabled
in the dialog — there is no selection to place them against.

As soon as the document carries a line, the `Insert Subtitles…` entry **is disabled as long
as nothing is selected**: without a selection, the index would be guessed.

### Afterwards

The lines added are **selected**, and the table goes to them. Pressing
`Ins` a second time therefore inserts after them, without having to click between the
two.

### The side is remembered

`Above` or `Below` is remembered **from one insertion to the next, and from one session to
the next**: it is the `edit.insert-placement` option of the
[preferences file](preferences.md#the-file). You do not insert once,
you insert ten lines in a row, always on the same side.

## Removing

`Remove Subtitles`, or `Del`, removes the selected lines — including a
discontinuous selection, each line going back to its place if you undo.

**No confirmation is asked**, and it is not an oversight:
the operation is undone with `Ctrl+Z`. A modal before an undoable gesture
would cost a click every time to spare a `Ctrl+Z` now and then.

**The entry is disabled when nothing is selected.** Elsewhere in the window,
“nothing selected” means “the whole file” — that is what makes a
shift usable without selecting four thousand lines. Here, that reading
would empty a document with an unlucky `Del`, so it is not offered.

After the removal, **the line that took the place of the first one removed is
selected** — or the last remaining one, if the removal took away the end of the
file. Pressing `Del` several times in a row therefore removes the lines one
by one.

A file emptied entirely leaves the window usable: `Insert Subtitles…`
becomes enabled again, since an empty document is inserted into without a selection.

## Merging

`Merge Subtitles` makes **a single line** of the selected lines:

| What the merged line receives | Where it comes from |
| :---------------------------- | :------------------ |
| its start | the start of the first |
| its end | the end of the last |
| its text | the texts of all, in order, a line break between two |
| its translation | the translations, joined the same way |
| what the format carries besides — a style, a layer, coordinates | the first |

**An empty text leaves no empty line**: merging `Un.`, a line with no
text and `Trois.` gives two lines of text, not three.

The merged lines disappear, and the line that replaces them takes the place
of the first.

**The entry is disabled** as long as the selection is not **a block of at least
two neighbouring lines**. A single line has nothing to merge with; lines
1 and 3 without 2 neither, since the result would go over 2 and
cover it.

After the merge, **the merged line is selected**: `Split Subtitle` is
ready to cut it again.

## Splitting

`Split Subtitle` cuts the selected line **in the middle of its duration**, to the
millisecond:

| Half | Start | End | Text |
| :--- | :---- | :-- | :--- |
| the first | the original start | the middle | **all** the text, and the translation |
| the second | the middle | the original end | empty |

**The text is not cut at its line break.** That would be right for a
two-line subtitle and wrong for a subtitle of one line or of three;
all the text stays in the first half, and you move yourself what must
go to the second.

The first half keeps what the format carried besides the text; the second
is born like an inserted line, with nothing.

**The entry is disabled** as long as the selection does not count **exactly one
line**. Nothing selected does not mean “the whole file” here.

After the split, **both halves are selected** — it is what Gaupol
does after any insertion, and `Merge Subtitles` is then ready to undo the
gesture.

## Sorting

`Sort Subtitles` puts **the whole project** back in the order of the starts. Never the
selection: an order is a property of the document, and sorting half of a
file means nothing. The sort is **stable**: two subtitles that
start together keep the order the file gave them.

**A bar runs in the status bar during the sort, and the cursor becomes an
hourglass**: on a long file, the window stays unresponsive for a while — the
table rebuilds line by line —, and without that you do not know whether something
is in progress. The bar shows no fraction, the sort does not report one,
and it goes away with it.

The status bar says how many lines changed place — `3 subtitles moved`
— or, when everything was in order, `already in order`. **A project already in order
does not enter the history**: an operation that changes nothing is not
an operation to undo. The entry is disabled for a project of one subtitle.

A file **with something to correct on opening** offers it by itself,
[in a single box](#on-opening-a-single-box): see below. Misplaced
subtitles are also [counted with the diagnostics](files.md#the-diagnostics-of-a-read),
so that you find them without scrolling the table.

On the command line, [`sort`](../subedit-cli/sort.md) does the same on
files, and `--sort` does it in passing on
[`convert`](../subedit-cli/convert.md#sorting-on-writing),
[`append`](../subedit-cli/append.md#sorting-the-result) and
[`split-file`](../subedit-cli/split-file.md#cutting-in-time-order).

## Joining repeated subtitles

`Join Repeated Subtitles` makes **a single subtitle** of the subtitles that show the
same thing in a row: a text cut into consecutive pieces, as
some segmentation tools leave it. It acts on **the whole project**.

**A repeat** has the same text, the same translation and the same format settings
(a style, a position, cue settings) as another, **and starts at the
latest when that one ends**. A run of repeats becomes the one that starts
first, with the largest end of the run; the others disappear.
`Alpha [1, 2]`, `Alpha [2, 4]` and `Alpha [4, 6]` give `Alpha [1, 6]`.

- **The file's order changes nothing about it, and it does not change it**: the subtitle kept
  is the one that starts first, wherever the others are.
- **Nothing is joined by mistake**: another text, other settings, a gap
  between two pieces, a subtitle with no text or whose end precedes its start
  stay separate.
- The status bar says what was done — `2 repeated subtitles joined`, or
  `no repeated subtitles`.

**To be done before sorting and cutting**, which read a clean run better: a
text in three pieces, plus another text over the whole range, given out
of order, sorts and cuts into lines that switch from one piece to the other; joined
first, it falls into order and merges cleanly. The opening box, below, does
all three in that order.

## Making the ranges disjoint

`Make Ranges Disjoint…` cuts the overlapping subtitles so that **no
`[start, end]` range overlaps another any more**. It acts on **the whole
project**, never on the selection: a group of overlapping subtitles
is cut where they overlap, and half of a group means nothing.

**Nothing is shortened or lengthened.** At every instant, the window shows the
same texts as before. What changes is how the timeline is cut:
it is cut at each start and each end of a group; an interval covered
by a single subtitle stays that subtitle; an interval covered by several
becomes **one** subtitle whose lines are theirs, **the earliest on line 1**.
`A [0, 5]` and `B [3, 8]` give `[0, 3] A`, `[3, 5] A / B`, `[5, 8] B`.

The dialog asks for three settings, remembered from one time to the next:

| Field | What it holds | Default |
| :---- | :------------ | :------ |
| `Shortest interval kept` | from 0 to 10,000 ms: a shorter interval is taken over by a neighbour | `80 ms` (about two frames) |
| `Most lines in a subtitle` | from 0 to 10; `No limit` at 0 | `2` |
| `Put a dash before each subtitle's text` | a dash before what each subtitle contributes to a merged subtitle | unchecked |

`OK` cuts, `Cancel` does nothing. **The lines written are selected**
afterwards, and the status bar says what was done: `2 subtitles split, 1 merged`.
A second pass changes nothing and says so: `already disjoint`.

- **A line is never duplicated.** Two subtitles with the same text give a
  single line where they overlap, and two neighbouring pieces that show the
  same thing make only one subtitle. The text of a subtitle cut into several
  pieces is otherwise repeated in each: it is what is on screen at that instant.
- **A very short interval is taken over by a neighbour**: the one whose lines
  already contain its own, otherwise the longest, extends its range to cover it.
  A whole subtitle shorter than the threshold is never taken over.
- **More lines than the limit: windows.** Three lines over three seconds
  become two halves, the first two lines then the last two;
  four lines, three thirds, and so on. Nothing is truncated.
- The translation follows the same cuts; the style and position of the first
  covering subtitle are kept.

**What could not be done in full is said in a box to close**, not in
the status bar: windows, intervals taken over, subtitles too short
left whole, styles that differ, subtitles whose end precedes their
start (left where they are).

**The entry is refused, with the reason, when the subtitles are not in
the order of their start**: the groups are runs of neighbours, and the core never
sorts by itself. `Sort Subtitles` first — or the opening box, which
does both in that order.

### All at once: `Repair Subtitles…`

`Repair Subtitles…` does the three preceding entries **one after the other**, in the only order that works: the
repeats are joined, the subtitles put in the order of their start, then the ranges made disjoint. It is the entry
for a file that is badly segmented, badly ordered and overlapping, when you know you want all three. The dialog asks for the
cutting settings (the same as `Make Ranges Disjoint…`, remembered from one time to the next) and says that it joins and sorts
first. The status bar — or the box to close, when something could not be done in full — gives **a single account**
of all three: `2 repeated subtitles joined; already in order; 2 subtitles split, 1 merged`. Each correction is its own
history entry: `Ctrl+Z` undoes them one by one. On the command line: [`repair`](../subedit-cli/repair.md).

### On opening, a single box

A file that opens with something we know how to correct **offers it in
a single box**, `Problems found in this file`, rather than opening one per
problem. Each problem found is a line with its check box, checked:

- `Join the repeated subtitles` — subtitles repeat another one they
  touch, with their number;
- `Sort the subtitles` — the subtitles are not in the order of their start,
  with their number;
- `Make the ranges disjoint` — subtitles start before the end of the previous one,
  counted **once the file is in order**, since that is where they are corrected.

**Each problem is counted where it is corrected**, on a copy to which the
corrections that precede it have already been made: a text cut into pieces is the
cause of some disorder, and joining it first may leave none — the box
then does not offer the sort.

A problem that was not found has no line. Unchecking the sort **greys out and
unchecks** the other box: ranges are made disjoint only in order.
`Fix` applies what is checked — the joining, then the sort, then the cutting, with the
settings of the last `Make Ranges Disjoint…` — and gives **a single account** of it;
`Keep as is` touches nothing. **Nothing is done without a yes**, each correction
is a history entry, and a file with nothing to correct is the subject
of no question.

On the command line, [`disjoint`](../subedit-cli/disjoint.md) does the same on files, and `--disjoint`
does it in passing on [`convert`](../subedit-cli/convert.md#making-the-ranges-disjoint),
[`append`](../subedit-cli/append.md#making-the-ranges-disjoint) and
[`split-file`](../subedit-cli/split-file.md#making-the-ranges-disjoint).

### Why not Gaupol's shortcuts

Gaupol merges with `M` and splits with `S`. A bare letter would be taken by the
table before reaching a cell being edited, and `Ctrl+S`
already saves: the two entries are reached through the menu.

## What the undo action says about it

**One history entry per gesture.** Merging five lines is undone with a single
`Ctrl+Z`, and gives back the five lines as they were — positions, texts and
everything the format carried.

| Operation | What `Undo` reads |
| :-------- | :---------------- |
| an insertion | `Undo: inserting` |
| a removal | `Undo: removing` |
| a merge | `Undo: merging` |
| a split | `Undo: splitting` |
| a join of repeats | `Undo: joining repeated subtitles` |
| a cutting of the ranges | `Undo: making the ranges disjoint` |

A correction offered on opening is one history entry per correction: the joining, the sort, then the cutting, are undone one after the other.

See [Undo and redo](undo.md).

## Shortcuts while a cell is open

`Ins` and `Del` belong to the input field as long as a cell editor
is open: the delete key erases a character there, not a
subtitle. Closing the editor — `Enter` or `Esc` — gives them back their meaning. Launched
from the menu, `Remove Subtitles` first commits the input, like the other
gestures of [Editing a cell](editing.md).

This manual names the keys as the menu displays them — see
[The menus](index.md#the-menus).
