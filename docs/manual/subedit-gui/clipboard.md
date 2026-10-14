# Cutting, copying and pasting texts

Three entries of the `Edit` menu, under `Undo` and `Redo`, and above
`Find and Replace…`.

| Entry | Shortcut | What it does |
| :---- | :------- | :----------- |
| `Cut Texts` | `Ctrl+X` | copies the texts of the selection, then empties them |
| `Copy Texts` | `Ctrl+C` | copies the texts of the selection |
| `Paste Texts` | `Ctrl+V` | writes the copied texts starting from the first selected line |

**It is texts that travel, and nothing else**: no start, no end, no whole
line. Pasting therefore moves no position — the timing of the line that
receives a text stays its own. That is what Gaupol does.

## What they act on

**With a translation, the text of the current column**: the translation if the
current cell is in the `Translation` column, the main text elsewhere —
see [The table](table.md#the-text-the-operations-target). `Cut Texts` therefore empties
the translation and leaves the main text, and `Paste Texts` writes into the
column you are in. **The clipboard carries no document**: you
can copy a cell of the main text and paste it into the translation, and
the other way round.

**The selection, and never the whole file.** The three entries are disabled as long
as nothing is selected. Elsewhere in the window, “nothing selected” means
“the whole file”; here, an unlucky `Ctrl+X` would empty all the texts
of the document, and a paste would have no line to start at.

While a cell is open, `Ctrl+X`, `Ctrl+C` and `Ctrl+V` belong
to the input field: they cut, copy and paste characters, just as `Del`
erases a character there and not a subtitle. Launched from the menu, the entries first commit
the input: `Copy Texts` then copies the text as it has just been
typed. See [Editing a cell](editing.md).

## Copying

`Copy Texts` copies the text of each selected line, from the first to the
last. **A discontinuous selection keeps its shape**: a line left out
of the selection becomes a hole, and the paste will not overwrite it.

**A selected line whose text is empty becomes a hole too**:
pasted, it leaves the target line as it is. Copying a single empty text therefore
pastes nothing.

The system clipboard receives these texts **as plain text, separated by an
empty line** — a hole is written there as an empty text, and that is why an empty
text cannot be anything but a hole: the paste gives the same result
whether the copy comes from this window or from another program. What you copy here can
therefore be pasted into any program, and what you copy elsewhere can be pasted here.

Copying does not modify the document, and does not enter the history.

## Cutting

`Cut Texts` copies, then **empties the texts** of the selected lines. The lines
stay, with their positions: cutting is not removing — to remove
lines, it is [`Remove Subtitles`](lines.md#removing).

Cutting texts that are already all empty does nothing, and does not enter the
history.

## Pasting

`Paste Texts` writes the clipboard's texts **starting from the first selected
line**, one line per text, downwards.

| What the clipboard contains | What happens |
| :-------------------------- | :----------- |
| a text | it replaces that of the line |
| a hole, or a copied empty text | the line keeps its text |
| more texts than there are lines left | lines are added at the end to receive them |

The added lines are born like lines inserted after the last one: three
seconds each, one after the other. **The window says so**, in an information box:

```text
inserted 2 subtitles to fit the clipboard
```

After the paste, **the written lines are selected**.

A paste that changes nothing — the same texts on the same lines — does not enter
the history.

### Tags of another format are translated

A copy made in this window **remembers the format of its document**, and
that memory survives the opening of another file. Pasting into a document of
another format therefore translates the tags, as `Save As…` does:
`{\i1}Bonjour{\i0}` copied from an Advanced SSA is pasted as `<i>Bonjour</i>` in a
SubRip.

**What the target format cannot write is lost, and it is said**, in
the words of `Save As…`:

```text
pasting SubRip texts into LRC: 1 tag dropped
```

Unlike `Save As…`, nothing is asked beforehand: the paste is undone with
`Ctrl+Z`, which gives the texts back as they were.

**A text copied outside `subedit` has no format**, and is pasted as is:
there is nothing to translate from nowhere. It is also the case of a copy from
this window that the system clipboard has replaced since — it is the
content of the system clipboard that counts.

When both messages are due, they fit in a single box, separated by
a semicolon.

## What the undo action says about it

Each gesture is **one** history entry, whatever the number of
lines it writes or adds.

| Operation | What `Undo` reads |
| :-------- | :---------------- |
| cutting | `Undo: cutting texts` |
| pasting | `Undo: pasting texts` |

See [Undo and redo](undo.md).
