# Coverage ratchet

The number of lines of `src/lib/` that the tests do not exercise. **The gate
fails if it rises.**

Why a count of lines and not a percentage: a percentage rises as soon as
well-tested code is added, so the ratchet rises, so the next piece of work finds
itself held tighter without anyone having decided it. A count of uncovered
lines reacts only to what we really want to watch. The decision and the
alternatives set aside are in
[ADR 0015](../adr/0015-memoire-des-mesures.md).

## What the gate does

| Situation | Reaction |
| :-------- | :------- |
| more uncovered lines than here | failure, naming the files that gained some |
| as many | passes, silently |
| fewer | passes, and invites running `make ratchet` |

`make ratchet` rewrites this file from the last measurement. **It never writes
of its own accord:** a versioned file moves only if someone asks for it.

## What the uncovered lines are, by family

**Reread as a whole at the review of phase 9**, and the reason for doing so is
in the shape of the figure: it rises by one or two lines at a time, each for a
sound reason, each justified here on its date. Nothing then forces the question
that matters — **does the total still say anything?**

It does, provided it is not read as a quantity. The sixty-seven lines fall into
four families, and they are not equal:

| Family | Lines | What a test would have to do to reach it |
| :----- | ----: | :--------------------------------------- |
| a Qt modal loop | 46 | click; `exec()` hands control to no one else |
| the outside world failing | 11 | make a `fork` fail, provoke an `EINTR`, fill a disk |
| libmpv, for real | 4 | a library and a display server inside the gate |
| **a guard the caller makes unreachable** | **6** | nothing: it would have to be reached by a path the code forbids |

The first three are **irreducible by construction**, and their count moves only
with the surface: one more modal, one more system call. Justifying them one by
one is the right gesture, and it is what the rest of this file has done since
phase 5.

**The fourth is the one that grows, and the only one open to debate.** They are
five guards — a saturated product one of whose factors is zero, a duration
shared into zero parts, a `togglePlayback` without a film, a dialog that would
have kept nothing. Each protects a dereference or a division, each is
**correct**, and none is reachable: what makes them unreachable is precisely
what makes them necessary — the caller already checks.

**The lesson, and it has a date.** In phase 9, two functions of the tag pivot
carried `switch` arms of the same kind; they were **rewritten** rather than
justified, because there the unreachable arm could disappear without weakening
anything — `applyOpening` on the HTML side, `appendOff` on the SSA side. It is
the trade-off to make again every time: **a guard that can be removed without
losing the protection is removed; a guard that really protects is kept and
counted.** Today's five are of the second kind.

What this table changes, from now on: a raise is judged against the family it
falls into, and not against the total. Five more modal lines are a fact of
surface; a sixth unreachable guard was a question — #434 asked it, and answers
it further down.

## What the ratchet let through, and why

Raising it is a decision, not an adjustment. Each raise is justified here,
without which the ratchet no longer measures anything.

**Two lines of the table model, in phase 5 — and the raise was one too many.**
They had been put down to the `std::unreachable()` placed after the exhaustive
`switch` statements, which no test can reach by construction. That was wrong:
`gcovr` does not count those lines. The two uncovered ones were the row and
column guards of `data()`, perfectly reachable — but the tests meant to exercise
them built their index with `index()`, which refuses to leave the table and
returns an invalid index. They therefore stumbled on the first guard and never
reached the other two.

Corrected in issue #129, by building these indexes with `createIndex` — which is
what the header exposes it for. The model is covered in full, and the ratchet
comes back down from 7 to 5.

The lesson is worth writing down: **a raise justified by a reading of the code
rather than by a measurement is a raise to check.** The reasoning was plausible,
it held for a whole phase, and it pointed at the wrong lines.

**Thirty-one lines of `QtPrompts`, in phase 5.** It is the largest raise of the
project, and it fits in one sentence: these lines open a Qt modal box, which
runs its own event loop until a human clicks. A test that reaches one never
gets control back.

**Measured, and named** — the four methods of `qt_prompts.cpp`, plus `run` and
the constructor in the header. There are no others, and each is either the call
to `QFileDialog` or `QMessageBox`, or the "the user chose nothing" guard, or the
construction of the return value from what the box returned.

**What this raise buys.** The `Prompts` interface gathers there everything that
is not reachable, instead of spreading it through the window. In exchange, the
paths where the faults live — the user cancels, chooses to discard, chooses to
save first, the write fails — are walked by tests, which no other arrangement
allowed. `MainWindow` and `DiagnosticsPanel` are covered in full.

**What was taken out of there rather than counted.** Two decisions lived in
those methods and had no business there: which format a filter designates, and
what a button of the confirmation box stands for. Exposed, tested, they are no
longer in this count — that is what brought it down from thirty-six to
twenty-five.

**Two more lines in phase 8, and the same rule applied twice.** `Save As…` now
asks for the encoding, the line ending and the mark, which could have added
thirteen untestable lines to `QtPrompts`. It adds **one**: everything that is
built and read back — the box, the widget of the three fields, what is drawn
from it — lives in `save_shape.cpp`, where a test fills it and reads it without
ever entering an event loop. What remains in `QtPrompts` is `exec`, and that
alone.

The second line is in `save_shape.cpp`: the guard of a box that would not carry
the shape, or that would have kept nothing. No test reaches it —
`saveDialogFor` always sets the shape, and an accepted box always carries a
name — and it stays, because the next step dereferences both.

**Two more lines in phase 5, and for the same reason.** Issue #132 adds
`QtPrompts::run` — "show this dialog, say whether it was accepted". It is the
seam of the dialogs the project writes itself, and it is narrower than the
previous ones: the three operation dialogs are ordinary widgets that a test
builds, fills and queries, so only the modal loop leaves the count. One
`exec()`, one line, rather than one method per dialog.

The first measurement of that issue gave six more in `MainWindow`: two
cancelled dialogs and three guards no user reaches, the validation button
following the state of the dialog. Tested rather than justified — a guard that
no test goes through is a promise nobody checks, and the fake `Prompts`
validates without looking at the button, which is exactly the situation they
protect against.

**Three more lines in phase 5, and still the same reason.** Issue #133 adds
`reportOutcome` — the notice that says what an operation did, distinct from the
warning that says what failed: two icons, two meanings, and a user who reads
"1 subtitle cleaned, 1 removed" has been warned of nothing.

Its confirmation cost nothing: the removal dialog is an `OperationDialog`
without a field, so it goes through the `run` already in place. An operation
without a setting did not need one more question.

**One line given back, at the review of the phase.** The constructor of
`QtPrompts` was counted: no test built one, for lack of anything to ask of it.
The review gave it something — the boxes were set on `nullptr`, so on no window
— and the window now tells it so itself. The test that exercises this builds the
class, which covers the line into the bargain. **Thirty-five, against
thirty-six.**

The rest of `qt_prompts.cpp` does not move, and will not: it is the
`QFileDialog` and the `QMessageBox`, and their event loop.

**Five more lines in phase 9, and the same rule once again.** Issue #352 adds
`QtPrompts::aboutLoss` — the modal that announces what a target format will not
carry, **before** writing, and whose answer can still be "no". It is a
`QMessageBox::warning` and nothing else: the title, the text, the two buttons,
the default one, and the comparison that says which was clicked. A test that
reaches it enters the modal loop and never gets control back.

**What this raise buys, and it is measurable.** Everything that decides lives
elsewhere and is covered: what is lost is counted by `convertFor`, the words are
those of `noticeOf`, and the window — refusing, writing anyway, asking nothing
when nothing is lost — is exercised through the fake `Prompts`. What remains
here is the call, and that alone.

**Five and not six**: the method fits in one expression, which the formatter
spreads over five lines. There is no guard to take out nor decision to expose,
as `choiceOf` had been for the unsaved-changes box — the comparison with `Save`
is the very return value of the call.

**Sixty-six, against sixty-one.**

**The alternative, weighed and set aside.** One can drive a modal box from a
test, by scheduling its closing before entering the loop. That would have given
a green figure at the price of a fragile test that exercises Qt's dialog rather
than our four lines of glue. The ratchet counts and justifies; it does not
demand zero.

**Three lines of `startProcess`, in phase 6 — and they speak only to a lack of
memory.** Issue #164 launches an outside program, which the project had never
done. The three lines are the two guards of the `posix_spawn` preparation and
the recording of their error code: `posix_spawn_file_actions_init` and
`…_addopen` can fail only on `ENOMEM` — or on `EBADF` for an invalid
descriptor, and the three passed here are `0`, `1` and `2`.

**Measured rather than read**, because this file itself says that a raise
justified by a reading of the code is a raise to check. The initial reading was
wrong: it was believed that an output file impossible to write would make
`addopen` fail, so that a test would reach it. Not at all — `addopen` only
records the intent, and it is `posix_spawn` that returns `ENOENT` at the moment
of opening. The test exists, it passes, and it goes through none of the three
lines.

**What was covered rather than justified.** The first measurement gave four:
the fourth was `LaunchErrorKind::Failed`, the case neither "absent" nor "not
ours". An executable file that is not a program produces it — the system
answers `ENOEXEC` — and the test that exercises it is the only one of the three
refusals that shows the table is one, and not two special cases.

**One line of `RealFileSystem::filesIn`, in phase 6.** Listing a directory
reads its entries one by one, and each advance can fail: the directory can
disappear between two reads. The uncovered line is the resulting refusal.

**What keeping it buys.** Without it, the iterator becomes "end" and the
function returns the partial list it had — silently. A caller would read "here
are the files" where it should read "I could not finish", and that is exactly
the kind of lie a seam to the system must not tell. The price is one line, and
it is paid once.

**Why no test reaches it.** A read would have to fail between two entries, from
outside, on a directory stream already open: removing the permissions does not
touch an open stream, and deleting the directory while it is being read does not
make `readdir` fail on Linux. It is the same family as the three lines of
`start_process.cpp` — system refusals a test cannot provoke.

**One more line in `start_process.cpp`, in phase 6.** #172 adds
`runAndCapture` to it, which asks a program a question and waits for its answer
through a pipe. The uncovered line is the refusal returned when `pipe` fails —
which requires having exhausted the process's descriptors, a state a test cannot
build without taking the whole test binary down with it.

It is the fourth in this file and the same family as the other three: system
refusals, reachable only by bringing it to its knees. The rest of the function
is walked, including its two useful error exits — an absent program, and an
executable file that is not a program.

**One line of `MpvPlayer::create`, in phase 6.** Building a player can fail in
three ways — no memory for a handle, an option this version of libmpv does not
know, an initialisation it refuses — and the uncovered line is the refusal that
comes out of it. The three are written as a single ordinary answer rather than
as three checks, precisely so that there is only one: to whoever asks for a
player, they are the same event.

**What was restructured rather than counted.** The first measurement gave five.
Four were the same defensive guard — "the property did not answer although a
video is open" — repeated in the duration, the position and the playback state.
Nothing makes an open file forget how long it lasts, so no test could walk
them. Written as a single expression, where the absence of an answer is a result
of the same rank as the number, they disappear from the count without anything
being lost — and the code gains by it.

**Eight more lines in `QtPrompts`, in phase 6.** #175 adds `videoToOpen` to it,
the video chooser: a call to `QFileDialog`, the "nothing was chosen" guard, and
the path returned. It is the third method of this class and the same reason as
the previous ones — a modal box runs its own event loop until a human clicks.

**What was taken out of there rather than counted**, as the previous times: the
filter. `videoFilters` assembles the list of extensions the core recognises, and
it is something this class could get wrong on its own — a chooser that offered a
file the rest of the program refuses to call a video would be a trap. Exposed,
it is exercised by an ordinary test, which holds the filter against
`isVideoFile` extension by extension.

**Eight more lines in phase 6, and all say the same thing: there is no screen
here.** #176 puts the video in the window, and adopting a native window is an
X11 mechanism — the tests run on the Qt `offscreen` platform, which provides
none.

| Lines | Where | What they do |
| ----: | :---- | :----------- |
| 3 | `mpv_player.cpp` | set `wid` and name the X11 context, when a window is given |
| 4 | `player_factory.cpp` | build a real `MpvPlayer` for that window |
| 1 | `main_window.cpp` | the `Play / Pause` guard when no film is open |

The first seven need an X11 session to be walked. This is not a testing
inconvenience but the very property being measured: a player without a screen
is what #178 settled for everything else, and these seven lines are exactly
where the screen becomes necessary again. They were **checked by hand**, under
`QT_QPA_PLATFORM=xcb` — and that check brought back **two** defects the code
then carried: the context left to mpv's choice opened a window beside ours
rather than inside it, and the window adopted before being on screen was never
mapped, so the panel stayed empty. Seven lines no test walks are seven lines
where two defects held.

The eighth is a defensive guard behind a disabled action. Qt does not trigger a
disabled action, so nothing can walk it; removing it would make the absence of
a crash depend on the state of a widget.

> **Covered since [#484](https://github.com/Guyot-Bertrand/sub-edit/issues/484)**, on 2026-09-25: the
> guard followed playback into `VideoPane`, which is called directly, with no disabled action in front —
> `video_pane_test.cpp` walks it by asking for playback before a film is open.

**What was covered rather than counted.** The composition of the cue — the
passage from a subtitle text to the ASS event that the overlay draws — was taken
out of `showSubtitle` as a free function, `assEventOf`, for the reason that had
taken the filter out of the chooser in #175: it is something this file could get
wrong on its own, and nothing that goes to libmpv is read back. `showSubtitle`
is walked by two cases — a player that has a film, one that does not.

**Two more lines in `QtPrompts`, in phase 7 — and they are the same ones.**
#254 gives the "open" box a directory, so that it no longer opens where the
process was launched. The call to `QFileDialog::getOpenFileName` therefore goes
from three arguments to four and takes four lines instead of two.

Nothing new became unreachable: it is the same modal call, longer to write. The
document already said, in phase 6, that "the rest of `qt_prompts.cpp` does not
move, and will not: it is the `QFileDialog` and the `QMessageBox`, and their
event loop". It still does not move — it is just written on two more lines.
**Forty, against thirty-eight.**

**What was covered rather than counted**, in the same diff: reading and writing
the three new options, the two palettes of the theme, the share given to the
handle, and the remembered directory. Six successive refusals of the ratchet
each gave a test case — a boolean `false` value nobody wrote, a zero width, a
line without an equals sign, the two overloads that resolve the location, a
light theme that was written without being read back, a preferences dialog that
was accepted without ever being cancelled.

## Four lines up in phase 9, and they came back down

**The ratchet had gone from 61 to 65, and it was the first time it rose for
code meant to disappear.** The four lines were in
`core/format/subtitle_file.cpp`: the refusal `readAs` gave to the seven formats
of phase 9 that did not have a reader yet.

**They were not reachable, and that was exactly why they existed.** `readAs`
always receives what `detectFormat` recognised, and the two pieces of knowledge
agreed — but **phase 9 was going to change them nine times**, once per format
issue. The day one moved ahead without the other, this refusal was what
separated a named error from undefined behaviour.

The alternative was a `std::unreachable()`, which costs no line and pays for the
same error in silent corruption. Four uncovered lines were cheaper.

**They went away on their own**, as announced. Each format delivered removed its
label from the group; TMPlayer and LRC removed the branch, and the ratchet came
back down without anyone having to remember it. With it went
`WriteErrorKind::NoWriter` — an enumerator nothing emitted any more — and the
failure branch of `textOf`, which could no longer be taken: nine formats out of
nine have a writer, and a writer cannot fail.

It is the shape of the bet that matters, more than its result: **the ratchet was
not raised with a promise to lower it one day; the condition under which it
would come down was written down.** The condition arrived, and the gesture made
itself.

**What was covered rather than counted**, in the same diff: the default values
and the comparison of `SubStationAlphaExtras`, which the first draft had written
without exercising them — eight more lines at the ratchet's refusal, and a test
case that says what a faulty comparison would cost.

**One more line in `main_window.cpp`, in phase 11 — the sixth unreachable guard
announced above.** #434 adds `Append File…`, whose core can legitimately return
a null command — `appendFile` refuses an appended project that carries no
subtitle, a case its own tests build directly. From the window, `openProject`
never returns such a project: the nine readers all refuse a read that would
find no subtitle, before even choosing a format. The window's guard therefore
protects a path the code already forbids, exactly the neighbouring family.

**Kept rather than removed**, the same weighing as the four lines of `readAs` in
phase 9: the alternative is a `std::unreachable()`, which costs no line and pays
the day a tenth format — or a loosened reader — would break the invariant with
silent corruption rather than with a "nothing was appended". An uncovered line
remains cheaper than a null dereference in case of a future reader's error.

> **Covered since 2026-09-26, [#486](https://github.com/Guyot-Bertrand/sub-edit/issues/486).** The
> guard followed `Append File…` into `ProjectOperations`, which reads the appended file through its
> `View` interface: a double of it returns an empty project no reader would return, and the case that
> reaches it is in `project_operations_test.cpp`. Unreachable from the window, it no longer is from its
> collaborator — the gain [ADR 0035](../adr/0035-les-operations-sortent-de-la-fenetre.md) expected from
> an operation testable without a window.

## One more line in `hearing_impaired_correction.cpp`, in phase 12

**#500 adds Gaupol's seven clean-ups after the engine's hearing-impaired
patterns**, each an expression and a replacement written by hand — `\1`,
`\1 \2`, or nothing. `ReplacementTemplate::parse` asks, for every replacement,
for a function that resolves a named group — the `\g<name>` that Python can
read — and the only one there is to give it here always returns its absence:
none of the seven replacements names a group, all being written `\1`, `\1 \2`
or empty.

**The uncovered line is the body of that function.** It is called only if a
replacement contains a `\g<…>` — and writing one for the sole purpose of making
it run would change an expression faithfully ported from Gaupol for a reason
that owes it nothing. It is the same family as the guards already counted
above: nothing reaches it because what it is given never asks for it.

**Kept rather than rewritten.** The interface of `ReplacementTemplate::parse`
also serves `common_errors.cpp`, where a replacement can really name a group —
removing it here would break that generality to save one line on a call that has
no use for it.

## One more line in `line_breaking.cpp`, in phase 12

**#502 ports Gaupol's `Liner`, and one of its guards cannot be reached, by
construction of the algorithm itself — not by what a caller checks before.**
`breakAmong` looks for the best break among the candidates `possibleBreaksOf`
gives it; for more than two lines, each candidate kept by `possibleBreaksOf` is
kept precisely because `possibleBreaksOf` has already checked, going down the
same recursion, that the rest of the text admits at least one break at the
requested number of lines. The guard `if (!later.breaks.has_value()) continue;`
assumes the failure of what `possibleBreaksOf` has just guaranteed.

**A proof, not a reading.** By induction on `nlines`: at `nlines == 2`,
`breakAmong` always tries its first candidate before its own stopping threshold
can be worth less than infinity, so it always returns a break as soon as
`possibleBreaksOf` gives it one. At `nlines == k`, the stopping threshold can
become finite only after a candidate has succeeded — so no candidate earlier
than the one the induction hypothesis guarantees can make the loop exit before
reaching it. This guarantee holds for the current implementation of the two
functions; a change to either — the order of the candidates, the pruning
threshold — undoes it, and that is why the guard stays written rather than
removed.

**Kept for that reason, and because Gaupol carries it too.**
`aeidon.Liner._break_lines` has exactly the same `if value[0] is None:
continue`, invisible to the test coverage of a language that does not measure
its branches that way. Removing it would make `breakAmong` code that can no
longer be read on its own — each recursive call would assume, without saying
so, what this note proves.

## One more line in `correction_run.cpp`, in phase 12

**#504 adds `applyCorrections`, which composes one command per project from
what was accepted.** It assembles `commands` in two ways — a `SetTextCommand`
pushed directly for each text replaced or emptied without removal, a
`RemoveCommand` for the group of emptied subtitles that are removed — before
refusing to compose anything if `commands` turns out empty.

**A proof, not a reading.** `applyCorrections` builds a group only for a project
that has at least one accepted `ProposedCorrection` (`groups` takes an entry
only when `found->items.push_back` has something to put there), so
`group.items` is never empty. For each of its items, the loop takes one of three
branches: `item->proposed` has a value, and a `SetTextCommand` is pushed
directly into `commands`; or it has none and `removeBlankSubtitles` is true, and
the index joins `emptied`; or it has none and `removeBlankSubtitles` is false,
and an empty `SetTextCommand` is pushed directly. The first item of the group
necessarily takes one of the three, so on leaving the loop, either `commands`
already holds an entry, or `emptied` holds one — and in that second case,
`if (!emptied.empty())` pushes the `RemoveCommand` that was missing. `commands`
can therefore never be empty when the guard reads it.

**Kept rather than removed**, for the reason this file repeats at every entry of
this family: the alternative is to compose an empty `CompositeCommand` and apply
it anyway, which nothing would prevent if a future caller loosened the invariant
this reading proves. An uncovered line remains cheaper than a history command
that changes nothing.

## Five more lines in the wizard, at the end-of-phase-12 review (#505)

The same gesture as the two previous entries — an exhaustive `switch` made safe
by the compiler, a risk accepted and already written elsewhere — applied to four
points that the end-of-issue review measured rather than assumed.

**Three fallback guards, one block each.** `CorrectionTargetPage::taskChecked`,
`CorrectionConfirmationPage::reasonOf` and the local `taskSettingsOf` of the
`CorrectionWizard` constructor share the same shape: a `switch` that covers
every value of an enumeration of four or six members, followed by a `return`
that nothing reaches as long as the enumeration does not gain a fifth without
updating the `switch` — which the compiler already reports elsewhere
(`-Wswitch`). Written out in full rather than as `std::unreachable()` to stay
readable without documentation alongside; counted all the same, since `gcovr`
does not tell the difference.

**Two lines in `CorrectionController`, and the risk #505 had already named
before the measurement confirmed it.** `pageOwning` answers `nullptr` when no
open page carries the address of the project an accepted correction targeted —
the guard of "discarding closes the window while a computation is still
running", and its counterpart, the `continue` that skips the corresponding
project in the application loop. That address is never dereferenced, only
compared: the risk lies entirely in the fact that it can name a project already
closed, which is precisely what the guard protects against. Reproducing it in a
test would require closing a project while a correction is still promised to it
— either real concurrency, or direct manipulation of memory to simulate it, both
costlier and more fragile than the line they would prove.

## Six more lines in `qt_prompts.cpp`, in phase 14 (#621)

`Prompts::proposeShiftedReopening` is the question the window asks when a
translation turned out to be shifted by a constant amount: reopen shifted, yes
or no. Its production implementation is **a modal box** —
`QMessageBox::question` — whose `exec()` cannot be tested, exactly like
`aboutLoss` and `aboutUnsavedChanges` beside it; the six lines (the header, the
text, the answers, the call, the return, the brace) join the forty-six of the
other methods of this file, which do not move.

**What is covered rather than counted**, in the same diff: all the rest of
#621, the search for the shift, the cut at eight candidates and the refusal when
two shifts are equally good — a case built so that each one cleans the opening
on its own, which is what makes the refusal the right answer —, and the window,
which answers the question through the `FakePrompts` double.

## Nine more lines in `qt_prompts.cpp`, in phase 22 (#779)

`Transcribe Audio…` adds two questions to `Prompts`, and their production implementations are **modal boxes**,
which `exec()` makes untestable, exactly like `aboutLoss` and `proposeShiftedReopening` beside them:

- `confirmReplacement`, the question asked before the subtitles of a project are replaced by a transcription
  (`QMessageBox::warning`);
- `fileToChoose`, the file chooser behind the `Browse…` buttons of the preferences (`QFileDialog::getOpenFileName`),
  for the model and for the program alike.

The nine lines are the headers, the texts, the answers and the calls of these two. **What lets the total rise by
no more** is a refactoring in the same diff: the three file choosers of the file now share one `pathOf`, which took
the lines of `fileToOpen` and `videoToOpen` down by about as many as the new chooser costs.

**What is covered rather than counted**, in the same diff: everything the questions are about — the dialog, which
asks the chooser through the `FakePrompts` double; the confirmation asked before the work and not after, and the
refusal that leaves the project as it was; the replacement as one undoable entry; the cancellation during the
extraction and during the transcription; the tools missing; the whole pipeline both surfaces share.

**One line more than first counted**: the chooser, born as `modelToOpen` in the dialog, moved to the preferences when
the program and the model became settings, and a second `ChosenFile` kind chooses between two titles and two filters
— one more line in a modal box, none anywhere else. The window's side of it (the `Browse…` buttons, the settings
that keep the answer) is covered through `FakePrompts`.

## Record

    total: 81

Recorded on version 1.0.11, on 2026-10-24.

| Lines | File |
| ----: | :--- |
| 59 | `src/lib/subedit/gui/qt_prompts.cpp` |
| 3 | `src/lib/subedit/core/io/real_file_system.cpp` |
| 3 | `src/lib/subedit/core/process/start_process.cpp` |
| 2 | `src/lib/subedit/core/edit/insert_command.cpp` |
| 2 | `src/lib/subedit/gui/correction_controller.cpp` |
| 2 | `src/lib/subedit/gui/mpv_player.cpp` |
| 1 | `src/lib/subedit/core/text/correction_run.cpp` |
| 1 | `src/lib/subedit/core/text/hearing_impaired_correction.cpp` |
| 1 | `src/lib/subedit/core/text/line_breaking.cpp` |
| 1 | `src/lib/subedit/core/time/ratio.hpp` |
| 1 | `src/lib/subedit/core/wording/correction.cpp` |
| 1 | `src/lib/subedit/gui/correction_target_page.cpp` |
| 1 | `src/lib/subedit/gui/correction_wizard.cpp` |
| 1 | `src/lib/subedit/gui/player_factory.cpp` |
| 1 | `src/lib/subedit/gui/project_files.cpp` |
| 1 | `src/lib/subedit/gui/save_shape.cpp` |
