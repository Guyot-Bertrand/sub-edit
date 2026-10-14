# 0008 — Open files as best we can, reporting diagnostics

**Date:** 2026-08-05
**Status:** accepted

## Context

Real subtitle files are often imperfect: produced by OCR, fetched from the web,
written by approximate tools, edited by hand. The intended user works precisely
on such files — removing hearing-impaired annotations is among their priorities,
which implies salvaged sources.

The analysis of Gaupol shows an **asymmetric** tolerance: its regular
expressions are deliberately lenient on timestamps — fields of 1 or 2 digits,
milliseconds of 1 to 3 digits, optional hours in WebVTT, comma or period,
negative times — but its structure is not protected. An `.srt` whose first
non-empty block is not a timestamp produces an `IndexError`, whereas the
documented exceptions are `IOError` and `UnicodeError`.

In other words: Gaupol is tolerant about what it anticipated and fails
unusably on the rest.

## Decision

Reading **opens as best it can and reports**. Two clearly separated levels:

- **Failure** — the file is unreadable or undecodable: access denied, a byte
  sequence invalid for the encoding, no recognisable subtitle. Reading returns
  an error, nothing is opened.
- **Diagnostic** — the file is usable but something was not understood or looks
  doubtful: ignored line, inconsistent timestamp, end before start, overlap,
  unknown format block. The subtitles are returned **with** the list of
  diagnostics.

Each diagnostic carries a **severity**, a **line number**, an **enumerated
category** and an optional detail. The category is an `enum class`, not a
string: it must be translatable and testable without comparing text.

## Alternatives dismissed

- **Fail outright on the slightest anomaly** — safer in appearance. Dismissed:
  refusing to open a file of two thousand subtitles because of one doubtful line
  leaves the user with no recourse, when the tool in their hands is precisely an
  editor.
- **Silent tolerance, like Gaupol** — the simplest. Dismissed because the user
  cannot know they have lost something. A file opened and then saved after
  quietly dropping three lines is a more serious defect than a refusal to open.
- **Repair automatically** — swap an end that precedes the start, fix overlaps.
  Dismissed: that is an editorial decision, and it belongs to the user. The
  corresponding operations will exist as explicit, undoable commands.

## Consequences

All reading functions return a `std::expected` — see
[0007](0007-cpp23-et-std-expected.md) — whose success carries the diagnostics.
The signature therefore tells the truth: success does not imply the absence of
problems.

**One invariant is deliberately not enforced**: `end ≥ start` is not guaranteed
by the model. The design principles ask for invalid states to be made
unrepresentable, but a real file can contain a subtitle whose end precedes its
start, and the user must be able to see it in order to fix it. Making that state
impossible would make the file impossible to open. It is an accepted exception,
and the only one of its kind.

The phase 5 interface will have to present these diagnostics — a banner, a
browsable list — without blocking the opening. The phase 3 command line will
write them to standard error and count them in its exit code.

The test corpus must contain genuinely malformed files, otherwise this
mechanism would never be put to the test.
