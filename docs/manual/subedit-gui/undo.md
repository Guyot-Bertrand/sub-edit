# Undo and redo

Every operation goes into a history, and can be undone. An edited cell counts
as **one** entry: typing is not recorded keystroke by keystroke, only the
commit counts.

| Action | Shortcut | Where |
| :----- | :------- | :---- |
| `Undo` | `Ctrl+Z` | **Edit** menu, toolbar |
| `Redo` | `Ctrl+Shift+Z` | **Edit** menu, toolbar |

## The action names what it will undo

The menu does not say “Undo” but **“Undo: editing a text”**, after the
operation that will be undone. The toolbar button keeps the bare word — its
width would otherwise follow the last operation, and it would move under the
pointer — but its tooltip carries the whole label.

| Operation | What the action reads |
| :-------- | :-------------------- |
| an edited text cell | `editing a text` |
| a start, an end | `editing a start`, `editing an end` |
| a duration | `editing a duration` |
| an insertion of lines | `inserting` |
| a removal of lines | `removing` |
| a shift, `Shift Whole File onto Grid` included | `shifting` |
| a transformation | `transforming` |
| a frame rate conversion | `converting the frame rate` |
| an alignment on a frame rate | `aligning on the frame rate` |
| a removal of mentions | `removing hearing-impaired mentions` |
| putting in italics | `putting in italics` |
| taking italics out | `taking italics out` |
| a change of case | `changing the case` |
| dashes added, removed | `adding dialogue dashes`, `removing dialogue dashes` |
| a merge of lines | `merging` |
| a split of a line | `splitting` |
| texts cut, pasted | `cutting texts`, `pasting texts` |
| an adjustment of durations | `adjusting durations` |
| opening a translation | `opening a translation` |
| appending a file | `appending a file` |
| splitting the project | `splitting the project` |
| a correction accepted in `Correct Texts…`, or made by `Check Spelling…` | `correcting texts` |
| a replacement, all replacements | `replacing`, `replacing all` |

**Twenty-seven labels, and that is all the window can produce.** The core names
a twenty-eighth — `sorting` — that no action of the window reaches: sorting
only happens under a strict order policy, while the window opens its documents
under the lenient policy, which reports disorder instead of repairing it.

**`Shift Whole File onto Grid` is not told apart from a shift**, and that is
accurate: it is one, whose amount was measured rather than typed.

These are the same words as those of the command line: they are written once,
in the core.

When there is nothing to undo, the action reads plain “Undo” **and it is
disabled**: its shortcut does nothing.

## The modification mark

The window title carries a mark as long as the file differs from the one on
disk — an asterisk, or whatever the platform uses instead. **So does the tab
label**, `premier.srt*`: the title speaks only of the project you are looking
at, the label tells about those you are not looking at.

**Each tab has its own history.** Undo only undoes what the project of the
current tab received — a `Replace All` made over all projects included: it
leaves one entry in each project it touched.

**It disappears if you undo back to the starting point.** This is not a display
detail: the number of modifications is counted, not recorded as a yes-or-no, so
going back can bring it back to zero and the mark goes away.

## Two things to know

**The reference point is the last save**, or the opening of the file if there
has been none. Saving moves it: the mark disappears, and undoing afterwards
brings it back.

An operation made after an undo **clears what there was to redo**: redoing
would replay a command whose starting state no longer exists. It is the
behaviour of every editor, and it is deliberate.

**While a cell is open**, `Undo` and `Redo` — from the menu or the toolbar —
first commit the input, like the other gestures of
[Editing a cell](editing.md). What has just been typed then becomes the last
entry of the history: `Undo` undoes it, not the operation before; `Redo` has
nothing left to redo, since an input clears what there was to redo. An open
cell where nothing was typed leaves no entry: `Undo` then undoes the previous
operation.

The history keeps a thousand entries and forgets the oldest beyond that.
