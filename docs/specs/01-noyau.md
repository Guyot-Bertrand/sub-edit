# Phase 1 — Core: data model and formats

**State:** design approved, ready to be split
**Milestone:** `1 — Noyau : modèle et formats`
**Framing issue:** #1

## Goal

Lay down the data model of `subedit_core` and the reading and writing of the two
priority formats, together with the reversible-command architecture that will
carry every operation of the following phases.

At the end of this phase, the library can open a `.srt` or a `.vtt`, render its
content faithfully, and write it back — with no interface and no editing
operation, which belong to phase 2.

## Scope

**Included** — position types, subtitle, project, format-specific data, SubRip
and WebVTT reading and writing, format detection, UTF-8 decoding, diagnostics,
command history, corpus of test files.

**Excluded, by phase** — automatic encoding detection and encoding sets
(phase 8); other formats and a rich markup model (phase 9); editing operations
(phase 2); a markup-aware parser (phase 4, where it becomes necessary).

The outline comes from the user's priorities, recalled in
[`../feuille-de-route.md`](../feuille-de-route.md): open and save SubRip and
WebVTT, in UTF-8, with Unix line endings.

## Applicable decisions

| ADR | Decision |
| :-- | :------- |
| [0006](../adr/0006-positions-en-millisecondes.md) | positions in whole milliseconds, strong types, rational frame rate |
| [0011](../adr/0011-numero-d-image-en-type-fort.md) | frame number as the strong type `Frame` |
| [0007](../adr/0007-cpp23-et-std-expected.md) | C++23 and `std::expected` |
| [0008](../adr/0008-lecture-au-mieux-avec-diagnostics.md) | open as best one can, report diagnostics |
| [0009](../adr/0009-texte-en-chaine-brute.md) | text stored as a raw string |
| [0010](../adr/0010-annulation-par-commandes.md) | commands carrying their own inverse |

## Module layout

```
src/lib/subedit/core/
├── time/        Timestamp, Duration, Frame, FrameRate, conversions, formatting
├── model/       Subtitle, Project, format-specific extras
├── format/      reading and writing, detection, diagnostics
└── command/     Command, CompositeCommand, History
```

Each module can be tested on its own. `time` depends on nothing, `model` on
`time`, `format` on `model`, `command` on `model`.

## Position types

```cpp
class Duration;   // signed milliseconds
class Timestamp;  // signed milliseconds, absolute position
class Frame;      // signed frame number
class FrameRate;  // exact rational
```

Not implicitly convertible to one another nor to an integer. Allowed
arithmetic:

```
Timestamp − Timestamp → Duration
Timestamp ± Duration  → Timestamp
Duration  ± Duration  → Duration
Duration  × integer   → Duration
Frame     ± Frame     → Frame
Timestamp + Timestamp → does not compile
Timestamp − Frame     → does not compile
```

None of these classes has a public constructor: one goes through named
factories — `Timestamp::fromMilliseconds`, `Frame::fromNumber`,
`FrameRate::create` — so that the unit is written at the call site.

`Frame` is the type of the "frame view" that
[0006](../adr/0006-positions-en-millisecondes.md) names without typing it;
[0011](../adr/0011-numero-d-image-en-type-fort.md) decides it and says why.
Since a frame has no sub-unit, a single type serves as position and as count,
hence `Frame + Frame` while `Timestamp + Timestamp` is refused.

`FrameRate` is a `numerator / denominator` rational. Gaupol's eight values are
exact: `24000/1001`, `24/1`, `25/1`, `30000/1001`, `30/1`, `50/1`,
`60000/1001`, `60/1`. Custom frame rates remain representable.

### Conversions and rounding

Rounding **to the nearest integer, half away from zero** — symmetric around the
origin, unlike the round-half-to-even that Gaupol inherits from Python.

Two guarantees, to be covered by tests:

- **`frame → milliseconds → frame` is the identity**, for any rate up to 60
  frames per second. At 60 fps a frame lasts 16.67 ms: the millisecond grid is
  finer than the frame grid. This is what makes it safe to unify the internal
  representation on milliseconds, including for the natively frame-based
  formats of phase 9.
- **`milliseconds → frame → milliseconds` is not.** Sub-frame precision is
  lost. Expected behaviour, tested as such.

Intermediate calculations are done in `std::int64_t`: the largest plausible
value is of the order of `10⁷ frames × 1000 × 1001`, i.e. ~10¹³, well within
bounds.

### Formatting

`HH:MM:SS,mmm` for SubRip, `HH:MM:SS.mmm` or `MM:SS.mmm` for WebVTT. Negative
positions carry a leading sign. **Saturation at `99:59:59,999`** on writing, as
in Gaupol — a format constraint, not a representation one.

Parsing a timestamp is deliberately permissive, like Gaupol: fields of 1 or 2
digits, milliseconds of 1 to 3 digits — or absent —, optional hours in WebVTT,
comma or dot as decimal separator, negative sign accepted, surrounding blanks
ignored.

Two limits to this permissiveness, decided while writing it:

- **minutes or seconds out of their range are refused.** `00:60:00` is not a
  60-minute timestamp, it is a damaged file; refusing lets the reader emit a
  diagnostic on the line rather than invent a position. Parsing returns
  `std::optional<Timestamp>` — a single cause of failure, no information that
  an `expected` would carry in addition.
- **hours come back when the position reaches one hour**, even if the caller
  asked for them to be omitted: `61:01.500` cannot be read back.

## Model

```cpp
enum class Document { Main, Translation };

struct Subtitle {
    Timestamp start;
    Timestamp end;
    std::string mainText;         // raw, format markup included
    std::string translationText;
    FormatExtras extras;
};
```

`duration()` is derived: `end - start`. It is not stored. `text(Document)`
gives access to either text, for reading and writing, so that the operations of
phase 2 name their target instead of hard-coding it.

**A subtitle carries both texts and a single pair of positions.** It is
Gaupol's model, and it directly expresses the fact that the translation has no
timing of its own. Since the need for a translation document is not confirmed,
the model accommodates it without anything else building it.

**`end ≥ start` is not an invariant.** An accepted exception to the design
principles, justified in [0008](../adr/0008-lecture-au-mieux-avec-diagnostics.md):
a real file may contain an inconsistent subtitle, and the user must be able to
see it to fix it. The anomaly produces a diagnostic.

### Format-specific data

The two MVP formats carry data that must survive the round trip — losing the
`settings` of a WebVTT cue means losing its on-screen positioning.

```cpp
struct Rectangle     { int x1, x2, y1, y2; };
struct SubRipExtras  { std::optional<Rectangle> coordinates; };
struct WebVttExtras  { std::string id, settings, style, comment; };

using FormatExtras = std::variant<std::monostate, SubRipExtras, WebVttExtras>;
```

A `variant` rather than Gaupol's bag of dynamic attributes: the compiler knows
which fields exist, and the exhaustiveness of the handling can be checked.
`std::monostate` covers the formats without data of their own.

The coordinates are an **option**, not four zero integers: Gaupol writes them
only if they are not all zero, so "absent" and "all zero" are two distinct
states that must both survive the round trip.

### Project

```cpp
enum class Newline { Lf, CrLf, Cr };

struct SourceFile {
    std::optional<std::filesystem::path> path;   // absent if never opened nor saved
    Newline     newline;
    bool        hadUtf8Bom;
    std::string header;                          // WebVTT
};

class Project {
    std::vector<Subtitle> m_subtitles;
    FrameRate  m_frameRate;         // 24000/1001 by default, as in Gaupol
    SourceFile m_sourceFile;
};
```

Subtitles are reached through `subtitleAt(SubtitleIndex)`, for reading as for
writing — that is what a command needs to modify a subtitle without rebuilding
the whole vector. An out-of-bounds index throws `std::out_of_range`: it is a
programming error, and failing loudly is better than reading what follows the
vector.

**`SubtitleIndex` is a strong type**, as the design principles announce. Two
numberings coexist — the zero-based position, on the code side, and the
one-based number, the one the user reads and SubRip writes. Mixing them up is
the classic off-by-one of a subtitle editor; the conversion is written once, in
the type.

`std::vector` rather than a cleverer structure: accesses are essentially
sequential and by index, insertions and removals grouped. A different structure
would have to be justified by a measurement.

`SourceFile` is not decoration: rewriting without BOM or CRLF a file that had
them would rewrite every one of its lines, and the user would find a diff where
they expected a corrected subtitle. The path is a **value** there, not a
handle — disk access goes through the injected abstraction of issue #5.

`Project` decides nothing: it holds the state and gives it back. Modifications
go through the reversible commands, which arrive with the operations. The file
format is not in it yet: it will appear with detection, in issue #8, rather
than be introduced here with no user.

## Reading and writing

```cpp
struct Diagnostic {
    Severity     severity;   // Warning | Recovered
    int          line;       // 1-based
    DiagnosticKind kind;     // enumeration, translatable, testable without comparing text
    std::string  detail;     // optional context
};

struct ReadResult {
    std::vector<Subtitle>   subtitles;
    std::string             header;       // WebVTT
    Newline                 newline;      // detected, preserved
    bool                    hadUtf8Bom;
    std::vector<Diagnostic> diagnostics;
};

class SubtitleReader {
public:
    virtual ~SubtitleReader() = default;
    [[nodiscard]] virtual std::expected<ReadResult, ReadError>
    read(std::string_view content) const = 0;
};
```

**The reader receives already decoded content, not a path.** File system access
goes through an injected abstraction: a test reads a string, with no disk and
no temporary file.

A strict distinction, laid down by
[0008](../adr/0008-lecture-au-mieux-avec-diagnostics.md):

| Case | Handling |
| :--- | :------- |
| Inaccessible file, bytes invalid in UTF-8, no recognisable subtitle | `std::unexpected` |
| Skipped line, inconsistent timestamp, end before start, overlap, unknown block | diagnostic, reading continues |

`DiagnosticKind` enumerates the anomalies the corpus must cover: skipped line,
unreadable timestamp, short fields, end before start, overlap, missing or
inconsistent numbering, text before any timestamp, unclosed tag, unknown block,
mixed line endings. `Severity` distinguishes what the reader **left as is** for
lack of a way to decide — `Warning` — from what it **settled**, such as a
regenerated numbering: `Recovered`.

The line number stays an `int`. It is a deliberate exception to the strong
types rule: it is only ever displayed, never mixed into a calculation with a
subtitle index, and one more type would buy nothing here.

### File system

```cpp
class FileSystem {                       // primitives only
    virtual bool exists(path) const = 0;
    virtual expected<string, FileError> readFile(path) const = 0;
    virtual expected<void, FileError>   writeFile(path, string_view) = 0;
    virtual expected<void, FileError>   rename(from, to) = 0;
    virtual expected<void, FileError>   remove(path) = 0;
};

expected<void, FileError> writeAtomically(FileSystem&, path, string_view);
```

The interface is **deliberately primitive**. Writing safely is a *policy*, not
a primitive: `writeAtomically` builds it on top of these operations, so that it
is written once and tested, instead of being reimplemented — and possibly
faked — by each implementation. The same reasoning brings `fileErrorKindOf` out
of the shadows: the `std::error_code` → `FileErrorKind` mapping is shared, hence
testable against the codes themselves rather than against a device one would
have to make fail.

Only three values in `FileErrorKind` — `NotFound`, `PermissionDenied`, `Io` —
because three is what a caller can act on: try again elsewhere, ask for
permissions, or give up.

Two implementations. `RealFileSystem` opens in binary mode, since a stream that
translated line endings would ruin detection. `InMemoryFileSystem` **lives in the
library**, not in the tests: every module that touches files needs it, and its
ability to fail on demand is the only honest way to test an interrupted save —
waiting for a real full disk is not a test.

Writing is **atomic** — write to a neighbouring temporary file then rename, as
in Gaupol. An interrupted save does not destroy the existing file, and the
temporary file is removed if the rename fails, so as to leave nothing lying
around next to the file.

**A limit to be aware of:** atomicity applies to the *replacement*, a rename
within a directory being atomic. It does not guarantee *durability* against a
power cut, which would require flushing the file and its directory to the
device. Gaupol does not do so either, and the failure the user needs protecting
from is the frequent one: a save that fails midway.

Two lines of `RealFileSystem` remain uncovered — the checks that follow a read
and the closing of a stream. Reaching them would require a device that fails
mid-operation; they are kept as guards, and reported here rather than removed
to make a figure.

### Splitting into lines

```cpp
std::vector<std::string_view> splitLines(std::string_view content);
```

Shared by the formats, which are all line-based. The three line endings are
recognised, none enters the text, and a final line ending does not invent an
empty line. The views point into the content: a file of several thousand
subtitles is split without copying a single line.

`trimmedBlanks` and `isBlank` go with it: every reader needs the same answer to
"is this line empty?", and three copies would be three opportunities to
diverge. On the writing side, `appendWithEnding` plays the same role — a text is
kept with line feeds, the line ending is a property of the file, decided at the
moment of writing it.

### Writing

```cpp
struct WriteRequest {
    std::span<const Subtitle> subtitles;
    Document         document;   // which of the two texts goes into the file
    Newline          newline;    // LF by default
    std::string_view header;     // WebVTT; empty for formats without a header
};

class SubtitleWriter {
    virtual std::string write(const WriteRequest&) const = 0;
};
```

**A writer returns a string, it does not touch the disk.** Taking it there is
the business of `writeAtomically`, which knows how not to destroy the previous
version — a concern no format has to repeat.

The line ending is the one it is given, LF by default, as the roadmap asks. The
original file's ending is not lost for all that: it is kept in `SourceFile`,
and it is the caller who decides to put it back, rather than the writer doing
so behind its back.

### SubRip

Numbering regenerated on writing. Comma as decimal separator. Extended
coordinates `X1/X2/Y1/Y2` written only if present and non-zero.

**What opens a subtitle is the timestamp line** — neither the number above it
nor the empty line before it. Real files have missing numbers, numbers that do
not follow one another, and separate their blocks as they please; only the
timestamp is reliable. The number is therefore taken back *a posteriori* from
the pending lines, since it belongs to the subtitle that begins and not to the
one that ends.

A trap found in Gaupol: its reading concatenates any non-timestamp line to the
previous subtitle, which produces an undocumented exception when the first
block is not a timestamp — a translator's note at the head of the file is
enough. Our reader emits one diagnostic per line and looks for the next block.

Two rules of caution, decided while writing the reader:

- **a line that meant to be a timestamp without managing it is reported then
  kept as text.** Destroying a line one did not understand would be worse than
  showing it as is;
- **half-written coordinates disqualify the whole line.** Keeping the
  timestamps and throwing away the rest would be a decision we have no
  standing to make.

On writing, the layout is Gaupol's, closing empty line included, also after the
last block. Hence the exact form of the round-trip guarantee:

- **faithful byte for byte** on a file already in this layout;
- **idempotent** otherwise — the first save normalises, no later one touches
  anything any more.

The numbering is the only thing that does not come back identical, and that is
intended: it is what makes a file whose numbers do not follow one another heal
by being opened then saved.

### WebVTT

Parsing by blocks separated by empty lines, with the states `header`, `style`,
`comment`, `identifier`, `timestamp`, `text`. `STYLE` and `NOTE` blocks are
attached to the subtitle **that follows** — that is what the format says, and
that is what makes them survive the round trip.

**The signature is required.** A file that does not start with `WEBVTT` is
refused — `ReadErrorKind::UnknownFormat` — rather than read by guesswork. The
detection of issue #8 needs readers that can say no, failing which it would
assume a format instead of recognising it.

An isolated line followed by no timestamp was not a cue identifier but a block
of a kind we do not handle — `REGION`, or whatever a later revision of the
format will add. It produces an `UnknownBlock` diagnostic.

An unreadable timestamp is reported then kept as text, exactly as in SubRip and
for the same reason. Two readers on the same foundation that handled the same
anomaly differently would be a trap in themselves.

The tags the format allows in the text — `<v Locuteur>`, `<c.classe>`,
`<ruby>` — are **not** interpreted, in accordance with
[0009](../adr/0009-texte-en-chaine-brute.md). A reader that normalised them
would make the file come back modified.

On writing: `WEBVTT` header, cue identifier if it exists, settings attached to
the timestamp line, `STYLE` and `NOTE` blocks restored. A file without any cue
remains a WebVTT file: the signature alone is valid, whereas writing nothing
would produce what no reader accepts.

Hours are omitted if no cue reaches one hour, as in Gaupol — with one
correction: **the decision looks at all the cues, not the last one.** Gaupol's
shortcut assumes the file is sorted; a file that is not would come out with
minutes beyond fifty-nine, which no reader accepts. All the timestamps of a file
are written the same way: mixing both forms would be legal and unreadable.

This is the format's only normalisation — a file written with its hours while
nothing reaches one hour comes back without them.

### Format detection

```cpp
enum class SubtitleFormat { SubRip, WebVtt };

std::optional<SubtitleFormat> detectFormat(std::string_view content);
```

By pattern on the content, like Gaupol. In case of ambiguity, the most specific
format wins; in case of failure, an explicit reading error, never an assumed
format.

| Format | What recognises it |
| :----- | :----------------- |
| WebVTT | the `WEBVTT` signature at the head, empty lines aside |
| SubRip | a timestamp line **with a comma** |

The signature decides whatever follows: a WebVTT file whose timestamps were
written with commas is a malformed WebVTT, not a SubRip.

**The comma is the tie-breaker.** A file whose timestamps use the dot and that
carries no signature is claimed by nobody — not WebVTT for lack of a signature,
not SubRip for lack of a comma — and refusing it is better than picking one at
random.

**The rules match what the readers accept, deliberately.** A detection that
promised a format its reader then refuses would be worse than no detection at
all. That is why the signature is required in upper case as the format wants,
whereas Gaupol accepts it in any case: recognising `webvtt` would produce a
file that no browser accepts.

### Encoding

UTF-8 only in this phase, and **checked** rather than assumed. Reading Latin-1
as UTF-8 does not fail: it replaces accents with gibberish, and the user only
notices once the file has been saved over. Overlong encodings and lone
surrogates are refused too — they are not curiosities, they are how a length
check is bypassed.

The **UTF-8 BOM is detected**, removed on reading and restored on writing if it
was present — it is commonly met, even though the other encodings belong to
phase 8. A BOM left in the text would appear as an invisible character at the
head of the first subtitle.

The line ending type is detected and preserved. The most frequent ending wins;
a file that mixes several — assembled from two others, which happens more often
than it should — produces a `MixedNewlines` diagnostic rather than a silent
normalisation. Writing uses LF.

### Opening and saving

```cpp
enum class Utf8Bom { Absent, Present };

std::expected<ReadResult, ReadError> readSubtitles(std::string_view content);
std::string writeSubtitles(SubtitleFormat, const WriteRequest&, Utf8Bom);
```

What links the encoding, detection and readers together. `readSubtitles`
checks the bytes, removes the BOM, looks at the line endings, recognises the
format and calls its reader — in that order, because everything after it reads
text and invalid bytes are not text. `ReadResult` carries the chosen format,
each reader filling in its own.

`Utf8Bom` is an enumeration and not a boolean: `writeSubtitles(format,
request, true)` says nothing, at the call site, about what is true.

Two distinct failures, and telling them apart is what makes it possible to say
something useful to the user: a file no format wants returns `UnknownFormat`; a
file that is recognised but contains no subtitles returns `NoSubtitleFound`.

## Reversible commands

```cpp
class Command {
public:
    virtual ~Command() = default;
    virtual void apply(Project&) = 0;
    virtual void revert(Project&) = 0;
    [[nodiscard]] virtual std::vector<Change> describe() const = 0;
};

struct Change {
    ChangeKind                 kind;      // Positions | MainText | TranslationText
                                          // | Insertion | Removal
    std::vector<SubtitleIndex> indices;
};

bool affects(ChangeKind, Document);
```

`describe()` returns **a list** and not a single `Change`: a composite command
reports what all the commands it contains report, and the union is made without
the foundation having to merge indices.

`Change` carries the affected indices and the nature of the change. The
interface of phase 5 will use it to refresh only what moves. The core knows no
signal mechanism: it returns the information, the caller does what it wants
with it.

The documents concerned are **deduced** from the nature of the change, rather
than carried twice: a subtitle has only one pair of positions for its two texts,
so a shift makes both documents stale, as does an insertion or a removal. Only
a text change concerns a single document.

`History` holds two stacks of `std::unique_ptr<Command>`, one modification
counter per document, and can group. A composite command contains a sequence of
commands, counts as a single entry, and **is undone in reverse order** — undoing
"insert then shift" starting with the insertion would shift subtitles that no
longer exist.

The **modification counter** is zero when the document matches the file on
disk, rises with each action, falls with each undo. Undoing back to the save
point brings it back there, which a boolean could not do. It becomes negative
if one undoes past a save, which is a difference like any other.

**`History` is not a member of `Project`.** ADR 0010 says the history belongs
to the project; this is to be understood as "it belongs to the core, not to the
interface", which the comparison with `QUndoStack` establishes. Two concrete
reasons stand against including it: `model` would no longer depend only on
`time` but also on `command`, against the module layout; and above all a
`Project` would stop being copyable, since a stack of `unique_ptr` cannot be
copied. The caller holds the two together.

The **size of the history is bounded**, to 1000 entries by default. The figure
is **provisional**: ADR 0010 promises to measure the real footprint before
fixing it, and no concrete command exists yet to be measured. The bound is
there so that memory is bounded; the number will be decided in phase 2.

This phase provides **no concrete command** — they come with the operations, in
phase 2. It provides the foundation and tests proving it on dummy commands: one
that actually modifies a project, the other that records the order in which it
was called.

## Tests

The corpus lives in `src/test/data/`, organised into `valides/` and
`malformes/`.

Malformed cases to cover at a minimum: empty file; UTF-8 BOM; mixed line
endings; timestamp with short fields; end earlier than start; overlapping
subtitles; missing or inconsistent numbering in SubRip; text line before any
timestamp; unclosed tag; unknown WebVTT block; bytes invalid in UTF-8.

Files from real use are worth more than made-up files. A second corpus,
specific to each workstation, therefore lives outside the repository in
`src/data/`: a test reads it if it is present and declares itself skipped
otherwise, which is the case in continuous integration. The versioned corpus
covers the forms we thought of, the local one meets the others.

Benchmarks set up from this phase, because they serve as a reference. The
document is **generated** rather than read: a benchmark that depended on a file
would measure the disk as much as the code.

| Measurement, on 4000 subtitles | Reference |
| :----------------------------- | --------: |
| reading a `.srt` | ~3.0 ms |
| writing the same | ~0.64 ms |
| frame rate conversion | ~0.33 ms |

Points of comparison between two states of the code, not promises.

## Split into issues

In order of processing. The numbers are those of the GitHub issues.

| Issue | Work | Nature |
| :---- | :--- | :----- |
| #2 | `Timestamp`, `Duration`, `FrameRate` types, conversions, formatting, timestamp parsing | **checkpoint** |
| #3 | `Subtitle`, `Project`, typed extras | structuring |
| #4 | `Command`, `CompositeCommand`, `History`, modification counter | structuring |
| #5 | Diagnostics, file system abstraction, atomic writing | structuring |
| #6 | SubRip reading and writing | mechanical |
| #7 | WebVTT reading and writing | mechanical |
| #8 | Format detection, UTF-8 BOM, line endings | mechanical |
| #9 | Test corpus and reference benchmarks | mechanical |

Issue #1 is the framing one, closed by this spec.

**Issue #2 is the agreed checkpoint**: it carries the decisions that are
costliest to undo, and will be reviewed before the rest builds on it. Issues 2
to 4 remain structuring; the following ones are short-loop, validated by the
corpus.

## Completion criteria

- [ ] A real `.srt` and `.vtt` open, are read back and are rewritten identical
      byte for byte, renumbering aside
- [ ] The malformed files of the corpus produce the expected diagnostics,
      without exception or silent loss
- [ ] `frame → ms → frame` is checked exact for the eight frame rates
- [ ] The history undoes and redoes a dummy command, grouped and isolated, and
      the modification counter comes back to zero at the save point
- [ ] `make check` passes, module coverage ratchet held
- [ ] Reading, writing and frame rate conversion benchmarks in place
- [ ] Manual section and CHANGELOG entry up to date
