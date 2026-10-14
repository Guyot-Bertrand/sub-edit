# Phase 7 — Finishing touches and first release

Framing: #237. Initialisation: #236.

## Goal

What is missing for a third party to install and use the tool.

Six phases have produced software that works and does not install. This one is
not after one more feature: it is after the distance between "it works on my
machine" and "someone else uses it".

## What sets it apart

**Its deliverable is not code but an installation.** No unit test says that a
third party can install the tool; that is proven by installing, into a prefix
that is not the build tree. This is the purpose of #239.

**A good part of what it changes can only be seen by eye.** A light theme and a
dark theme, a geometry remembered from one session to the next, a complete
manual: three things whose default verification is "someone looks and finds it
fine". This is the purpose of #199.

**Its preliminary analysis is partly already done**, and this is the first
time. `gaupol/config.py` had been read when phase 5 was framed; this framing
read it again and extended it, and the result is in
[ADR 0022](../adr/0022-configuration-au-noyau-et-tolerance-par-option.md).

## The preliminary analysis, and what it yielded

Read in `reference/gaupol`: `gaupol/config.py`, `gaupol/style.py`,
`gaupol/applicationman.py`, `gaupol/agents/help.py`, `gaupol/actions/edit.py`,
`gaupol/dialogs/insert.py`, `data/`, `PACKAGING.md`, `Makefile`, `flatpak/`.

Four results, two of which correct what the roadmap expected.

**`style.py` does not deal with the theme.** The roadmap listed it under the
light and dark theme; it contains only CSS and a customisable editor font. The
theme lives in `applicationman.py`, and it does live there.

**Gaupol has exactly our constraint, one version ahead.** Its theme is an
enumeration with three values — system, light, dark — persisted in the
configuration. "System" is only resolved on GTK 4.20 and later; below that,
`_update_theme` **does nothing at all** and lets the toolkit decide. This is the
twin of our situation under Qt 6.4.2, and it is what turns our constraint into
designed behaviour rather than a dead end.

**`Help ▸ Browse documentation` opens a URL**, not a local file.
`gaupol.util.show_uri(gaupol.DOCUMENTATION_URL)`. The only document Gaupol
installs is a manual page, `data/gaupol.1`.

**Installation comes before format.** The `Makefile` installs the binary, icons,
`.desktop`, AppStream, manual page and translations with `DESTDIR` and
`PREFIX`; the Flatpak manifest builds on top of it. Gaupol does not choose a
format, it produces a correct installation. This is what
[ADR 0023](../adr/0023-deb-et-rpm-pour-la-premiere-livraison.md) takes up.

## Scope

| In the phase | Out of the phase |
| :----------- | :--------------- |
| persisted preferences | the preferences nobody asks for |
| light, dark, system theme | an application-specific theme |
| insertion and removal from the window | merging, splitting — phase 10 |
| complete user manual | translating the manual — phase 15 |
| `install()` rules, `.deb` and `.rpm` packages | Flatpak, AppImage — ruled out, ADR 0023 and #265 |
| `Help ▸ Manual` re-enabled | context help |
| a single opening recipe (#154) | |

## Applicable decisions

**D1 — the configuration is a core type, tolerant option by option.**
[ADR 0022](../adr/0022-configuration-au-noyau-et-tolerance-par-option.md). It
reads and writes through `core::FileSystem`, **receives** its location, and has
neither a version number nor a migration. An unknown key is ignored, an
unreadable value leaves the default in place with a diagnostic, and **an option
left at its default is rewritten commented out**.

**D2 — two native packages, `.deb` and `.rpm`, after `install()` rules.**
[ADR 0023](../adr/0023-deb-et-rpm-pour-la-premiere-livraison.md). Installation
first, formats next; Flatpak and AppImage deferred with their reason — and
**ruled out since**, the question having been investigated after the phase:
#265.

**Two, and the second is what puts the decision to the test.** "Installation
first" is an assertion as long as a single format exercises it; an `.rpm` built
from the same rules verifies it, since **if it required touching up the
installation, the installation was wrong.**

**What really differs between the two is not the list of files but the declared
dependencies**: Qt 6 and libmpv do not carry the same package names on Debian
and on Fedora. That is where the package can be wrong without anything showing
it at build time.

**And an acknowledged asymmetry, written rather than kept quiet**: an `.rpm`
built on Ubuntu cannot be installed there. The `.deb` is proven end to end by
#239; the `.rpm` is proven up to its content — list of files and declared
dependencies, through `rpm -qlp` and `rpm -qp --requires`. Shipping both while
letting it be believed that they were tested alike would be the real defect.

**D3 — the theme is a three-valued preference, and "system" does nothing.**
Under Qt 6.4.2 there is no colour scheme API — `QStyleHints::colorScheme`
arrived in 6.5 — so the application can neither ask the system what it prefers
nor be notified when it changes its mind. "System" therefore means **leaving
the palette to the platform theme**, which the window already does today.
"Light" and "dark" set an explicit palette.

This is Gaupol's behaviour under GTK earlier than 4.20, and it is honest: it
delivers the two themes the user can ask for, and it does not invent a reading
of the system that it cannot perform.

**D4 — and it is D3 that makes the theme testable.** Because "light" and "dark"
are palettes *that we set*, a test can set them too, and a screenshot can show
both of them. Had the theme been a reading of the system, neither would be
possible under Qt 6.4.

**Direct consequence for #199: two screenshots per screen, and they are
attainable.** The question left open by #236 is settled here.

**D5 — a preference is persisted if forgetting it is annoying, and if
remembering it is risk-free.** The criterion is the exclusion one, because that
is the one that is always missing: a preference is easy to add and impossible
to withdraw once a user relies on it.

What gets in: window geometry, maximised state, column widths, theme. **Plus
three, added after the fact** — the position of the handle between the video
and the table and the last opened directory, which #254 carries, then the
insertion side, `edit.insert-placement`, which D7 made persistent and which this
list did not count. The paragraph below says why the directory gets in where
the last *file* does not.

**The default frame rate, however, does not get in — and this list promised it
throughout the phase.** Nothing reads it: no dialog uses it, no document
inherits it. A preference nobody uses is a box that lies, and #241 was right not
to write it. The word is withdrawn here rather than the gap filled, and the
question has been investigated and then closed: it **will not come** —
[#267](https://github.com/Guyot-Bertrand/sub-edit/issues/267).

Seven options, then, and the configuration file carries seven.

What does not get in, and why: **the last opened file.** Remembering it means
reopening at launch a document the user did not ask for, and the strict mode of
ADR 0012 makes opening not neutral. The list of recent files is something else —
a feature, not a preference — and it does not belong to this phase.

**Remembering a directory is not remembering a file**, and this is the cut the
criterion makes where it is not expected: a directory *points* a dialog, it
opens nothing. The two halves of the criterion do not decide at the same place,
and it is the second — "remembering it is risk-free" — that one forgets to
apply.

**What the review of #240 ruled out**, and which is worth writing down so that
it is not proposed again: the collapsed diagnostics panel — collapsing it costs
a click; the player's volume — there is no volume setting before the playback
bar; the default save format — it comes from the opened file, ADR 0018; and
**any number that belongs to a document** — a shift duration, two
transformation marks, true for one file and false for the next.

It also measured what the design had already saved. Three quarters of Gaupol's
hundred or so options are a dialog's sticky state, six of them `target` options
that say what an operation applies to. `targetOf()` deduces it from the
selection, so none of them has an equivalent here.

**D6 — `Help ▸ Manual` opens the installed manual, not a URL.** Gaupol opens a
URL; we do not follow it here, and for a precise reason: **a URL describes
`main`, not the installed version.** A user reading the manual of a version
they do not have is served worse than a user without a manual.

The manual remains Markdown — that is its source form and the one the
repository reviews. `QTextDocument::setMarkdown` can render it with no external
tool or browser, hence with no new dependency. **To be checked in the issue**:
our sections use tables, and Qt's Markdown rendering has its limits; if they
bite, the alternative is HTML rendering at build time, with the dependency it
requires.

**D7 — insertion follows Gaupol's rule, because it is already in the core.**
`InsertCommand::blank` implements its rule for sharing the available time, and
its comment says so. The gesture follows:

| | |
| :--- | :--- |
| how many | asked, one by default |
| where | before or after the selection, as chosen, and **that choice is persisted** |
| the index | the **last** selected, plus one if "after" |
| empty file | index zero, and no selection is required |
| removal | the selection, `Del` key |

The last selected and not the first: this is the point one gets wrong if one
does not read it, and Gaupol has done it this way for twenty years.

**D8 — the trigger of ADR 0019 does not fire, and it is written here.** The ADR
asked to measure before building a `Session` that announces a change of
structure. #236 established that **the view's share is not measurable** in the
bench harness — it has no `QApplication`, and adding one would make it run
without a screen, which the bench's comment already rules out by name.

What remains is the model's figure: `réinitialisation du modèle après une ligne
retirée`, around 10 µs on four thousand subtitles. An insertion is a human
gesture. **Building an optimisation whose benefit is not measurable is exactly
what the ADR asked to avoid**, so it is not built.

The trigger is reformulated rather than abandoned: it reopens the day someone
measures on a machine with a screen and finds that it stings, or the day a
producer changes the structure faster than a human clicks.

## The core

### `core/config/settings.hpp`

The type of D1. It carries the values, their defaults, and nothing else:
neither a default location nor a singleton.

```
[[nodiscard]] std::expected<Settings, ConfigError> readSettings(const FileSystem&, const std::filesystem::path&);
[[nodiscard]] std::expected<void, FileError>       writeSettings(const FileSystem&, const std::filesystem::path&, const Settings&);
```

An absent file is not an error: it is `Settings{}`. What produces a diagnostic
is a value that could not be read — and the diagnostic accompanies the default
value rather than replacing it.

### `core/edit/`

Nothing new. `InsertCommand::blank` and `RemoveCommand` have existed since
phase 2 and never had a surface. **This phase is their first end-to-end
proof**, which is a result in itself: two core commands lived until now without
any user being able to trigger them.

### What #154 moves

`gui/opening.cpp` and `gui/saving.cpp` do not contain a line of Qt. They move
down to the core, and the window gains from it the ability to name the real
cause of an opening failure. The issue carries its own finding and this framing
does not replay it.

## The window

| Entry | Menu | Shortcut |
| :---- | :--- | :------- |
| `Insert Subtitles…` | Edit | `Ins` |
| `Remove Subtitles` | Edit | `Del` |
| `Preferences…` | Edit | — |
| `Manual` | Help | — |

The first two after `Undo`/`Redo`, with a separator between the two groups:
undoing is what one does *to* an edit, inserting and removing *are* edits.

`Preferences…` carries the theme, and only the theme. The geometry and the
column widths do not show there — they are set by moving the window, and a
preference that already has a gesture does not need a field; the insertion side
is set in the dialog that uses it. That leaves the theme, the only one that has
no gesture elsewhere.

**The framing added the default frame rate to it, and the phase did not deliver
it** — see D5: nothing would read it. The row is corrected here rather than left
saying what does not exist; the manual already had it right. The question was
reopened after the phase and closed for good: **only one preference has a
dialog, and it will be the theme.**

## The installation

What the `install()` rules put in place, with paths from `GNUInstallDirs`:

| File | Where |
| :--- | :---- |
| `subedit-gui`, `subedit-cli` | `${CMAKE_INSTALL_BINDIR}` |
| icon | `${CMAKE_INSTALL_DATADIR}/icons/hicolor/scalable/apps` |
| `.desktop` | `${CMAKE_INSTALL_DATADIR}/applications` |
| AppStream | `${CMAKE_INSTALL_DATADIR}/metainfo` |
| manual | `${CMAKE_INSTALL_DATADIR}/subedit/manual` |
| `subedit-cli.1` | `${CMAKE_INSTALL_MANDIR}/man1` |

**Two of these rules have existed since #239** — the binaries and the manual —
because an installation check has nothing to prove without them. The other four
name files the repository does not contain yet: the icon, the `.desktop`, the
AppStream metadata and the manual page of `subedit-cli` come with #244, and
their rules are added in `cmake/Installation.cmake`.

**What is packaged is what `make release` produces, issue #247.** The target
has existed since this phase; before it, to get an optimised binary, one ran
`make bench` and interrupted it. It builds neither the bench nor the end-to-end
harness — what is asked for is what a user runs, not what tests it.

## Tests

**The configuration is tested without writing anywhere**, through
`InMemoryFileSystem` and a given location. This is what #238 puts in place, and
it is why the check of #226 has nothing to do with it: no test touches a real
location.

**The theme is testable because it is set** — D4. A test asks for "dark" and
reads the resulting palette.

**The installation is tested in a temporary prefix** — #239 — and the prefix
leaves nothing behind, which the check of #226 would see. `make
install-check` installs, runs the two installed binaries, and compares the
deposited manual with the repository's — a list that is **computed** and not
copied, otherwise an added chapter would go missing silently.

What it does not prove is written in its header rather than left to be believed
proven: nothing about the native packages, which do not exist yet, and nothing
of what no Ubuntu machine can say about an `.rpm` — it is built there and not
installed there.

**The screenshots are doubled** — D4 again.

## Measurements

Nothing new is expected on the bench. Reading a configuration is a file of a
few lines read once at startup; measuring it would be measuring `readFile`.

The only figure that matters for this phase already exists, and D8 has
interpreted it.

## Requirements

| ID | Requirement |
| :- | :---------- |
| `GUI-CONFIG-01` | the window recovers its geometry and its columns from one session to the next |
| `GUI-CONFIG-02` | an unreadable value leaves the default in place, and says so |
| `GUI-CONFIG-03` | an option left at its default is rewritten commented out, so a changed default takes effect |
| `GUI-THEME-01` | the theme is chosen among three values, is remembered, and "system" sets nothing |
| `GUI-THEME-02` | "light" and "dark" set an explicit palette |
| `GUI-INSERT-01` | inserting places the subtitles after the last selected one, or before as chosen |
| `GUI-INSERT-02` | inserting into an empty document requires no selection |
| `GUI-REMOVE-01` | removing takes away the selection, and the operation can be undone |
| `GUI-MANUAL-01` | `Help ▸ Manual` opens the installed manual |

**Nine, and packaging carries none of them.** This is not an oversight: the
registry is confronted with the **tests** by `check-requirements.sh`, and the
installation is not proven by a test but by a target that installs then runs —
#239. A requirement no test can cite would be a dead row in the registry.

**They enter the registry at the start of the issue, in the `planned` state**,
as phase 16 held to. It was the first where the rule really served; this one is
the second, and it is here that we will know whether it holds outside the
favourable case.

**Nine, and the registry long carried only eight.** #241 merged `GUI-THEME-02`
into `GUI-THEME-01` without anything saying so, which cost a promise: the
remaining row says that the theme has three values and is remembered, it does
not say that "light" and "dark" set a palette. Yet it is precisely that palette
that holds D3 and D4, and two cases of `theme_test.cpp` already proved it
without anything tying them to a promise. The end-of-phase review therefore
returns `GUI-THEME-02` to the registry and gives it its tags, rather than
endorsing the merge.

**And it gives the defect its check**, because this is the second phase in a
row where it gets through — phase 16 had withdrawn `CLI-SNAP-03` without its
spec following and let `GUI-GRID-03` be born without recording it there.
`check-requirements.sh` now confronts **each spec's table with the registry
rows that carry its phase**, in both directions, with the phase and the
strikethrough. It found four more gaps along the way, all earlier: three
`CLI-VERSION` replaced and a `CLI-OUTPUT-06` born along the way, absent from the
phase 3 spec, and a `CLI-INSPECT-04` absent from the phase 5 one.

## Manual

It is the heaviest deliverable of the phase, and the only one that touches
everything before it: the six interface sections have been prose without a
safety net since #116, and this phase adds preferences, a theme, two operations
and an installation to them.

The order matters: **#199 before the writing**, otherwise the screenshots are
taken by hand and the manual once again describes an interface on trust.
`docs/manual/subedit-cli/installation.md` is to be redone entirely — it says
today that the tool is launched from the build tree.

## Split into issues

| Issue | Subject | Depends on |
| :---- | :------ | :--------- |
| #247 | a target that produces the binaries in Release | — |
| #238 | isolating the configuration location | — |
| #199 | generated screenshots | D4 |
| #154 | a single opening recipe | — |
| #240 | the persisted configuration | #238, D1 |
| #241 | the light and dark theme | #240, D3 |
| #254 | the handle and the last directory remembered | #240, D5 |
| #242 | inserting and removing from the window | D7 |
| #243 | the complete user manual | #199, #240, #241, #242, #254 |
| #244 | packaging: `install()`, `.deb` and `.rpm` | #243, #239, #247 |
| #245 | `Help ▸ Manual` re-enabled | #244, D6 |

#154 depends on nothing and comes early: it reworks the opening path that #240
hooks into, and its visible half lands before #243 describes it.

#245 is last by construction, and it is a chain with no shortcut: the manual
must be complete to be installed, packaging must be settled to decide where it
lives, and the entry is only re-enabled once both are done.

#254 was born from the review of #240 rather than from this framing, and it
touches the same type as #241: doing them together avoids a conflict on
`core::Settings`.

## Deferrals

| What is deferred | Where |
| :--------------- | :---- |
| a Fedora container to test the `.rpm` | [#266](https://github.com/Guyot-Bertrand/sub-edit/issues/266) |
| a Qt bump to read the system's colour scheme | outside the phase; D3 delivers without it, and ADR 0001 would reopen |
| the list of recent files | outside the phase — a feature, not a preference, D5 |
| translating the manual | phase 15 |

**The four deferrals that designated no one have been investigated, and three of
them died of it.** This is the result one does not expect from a clean-up: a
condition with no referent survives because nobody looks at it, and looking at
it is often enough to conclude it.

| What was deferred | What was decided |
| :---------------- | :--------------- |
| the trigger of ADR 0019, reformulated by D8 | **abandoned** — [#264](https://github.com/Guyot-Bertrand/sub-edit/issues/264). D8 is sufficient: the view's share is not measurable on the bench, the model's is of the order of 10 µs, and an insertion is a human gesture |
| Flatpak and AppImage, deferred by ADR 0023 | **ruled out** — [#265](https://github.com/Guyot-Bertrand/sub-edit/issues/265). `.deb` and `.rpm` are enough for the intended outline; elsewhere, one builds from source, and the manual describes that path in full |
| the default frame rate in the preferences | **ruled out** — [#267](https://github.com/Guyot-Bertrand/sub-edit/issues/267). None of the three possible readers needed it, and D5 was therefore right the first time |
| a Fedora container to test the `.rpm` | **kept**, and alone of the four: it is the only packaging gap that remains open |

Three of them were phrased as a condition — "the day someone measures", "when
someone actually installs the package", "the day something reads it". A
condition with no referent ends up forgotten; this is the defect that the course
of a phase describes, and it is the second review in a row to find it. **Three
"maybe"s have become three "no"s, and that is a gain**: a written "no" is not
proposed again.

## What the end-of-phase review brought out

Seven lines of inquiry, discussed and kept, each with its issue. None is
corrected here — that is the rule of the critical look.

| Issue | What it carries |
| :---- | :-------------- |
| [#268](https://github.com/Guyot-Bertrand/sub-edit/issues/268) | does each check go through the tool that counts? The manual is never rendered by Qt, the manual page never by groff |
| [#269](https://github.com/Guyot-Bertrand/sub-edit/issues/269) | a comment added to the `Makefile` takes clang-tidy from 2 files to 223 |
| [#270](https://github.com/Guyot-Bertrand/sub-edit/issues/270) | six of the phase's thirteen records are taken above the load threshold |
| [#271](https://github.com/Guyot-Bertrand/sub-edit/issues/271) | `check-installation.sh` has tripled: one thing, or two? |
| [#272](https://github.com/Guyot-Bertrand/sub-edit/issues/272) | `subedit-gui` has no manual page, and Gaupol has only one, for its window |
| [#273](https://github.com/Guyot-Bertrand/sub-edit/issues/273) | 75 test titles in French against 1188 in English, all born in this phase |
| [#274](https://github.com/Guyot-Bertrand/sub-edit/issues/274) | two shortcuts that Qt does not set under X11, and that the manual promised |

## What the measurement settled

**Phase 16 had concluded the opposite, and its conclusion is wrong.** It had
recorded, among its three "assertions refuted by measurement":

> "the slot's guard is unreachable" → **Qt invokes the slot of a disabled
> action**

Qt does not invoke it. `QAction::trigger()` and `QAction::activate(Trigger)` on
a disabled action emit nothing:

```
enabled, trigger()   -> 1
disabled, trigger()  -> 0
disabled, activate() -> 0
```

The case that was supposed to settle it — `the entry that is out does nothing
when triggered` — **could not do so**: its two assertions, the status bar
unchanged and undo still disabled, are true whether the slot is reached or not.
Its comment nevertheless said "This case is what says so".

The case now counts the emissions rather than inferring them, and it also checks
the positive direction — the same action, re-enabled, does reach the slot. That
is what distinguishes "measured" from "nothing moved".

Three comments from this very phase already relied on the real behaviour: that
is why `an empty removal does not enter the history` and `back forced with no
history does nothing` re-enable the action by hand to reach the guard they test.
`docs/mesures/couverture.md` has also said so since phase 6 — "Qt does not
trigger a disabled action, so nothing can walk it". The fourth, older, said the
opposite; it is corrected.

**The lesson of phase 16 holds nonetheless, and it turns around:** a case
written to settle something only settles it if its assertions distinguish the
two answers. That one distinguished neither, and for a whole phase it served to
support the wrong one.

## Completion criteria

- [x] The six implementation issues are closed, plus #154, #199, #238, #239, #247, #254
- [x] A third party installs the package and runs the tool without the build tree
- [x] The manual describes what exists, screenshots included, in both themes
- [x] `Help ▸ Manual` opens the installed manual
- [x] The end-of-phase review is done — #263
