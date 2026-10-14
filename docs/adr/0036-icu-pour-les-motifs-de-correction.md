# 0036 — ICU to apply the correction patterns

**Date:** 2026-09-26
**Status:** accepted

Decided while scoping phase 12, issue
[#493](https://github.com/Guyot-Bertrand/sub-edit/issues/493). Reopens
[ADR 0017](0017-analyseur-de-mentions-ecrit-a-la-main.md), as it had itself
foreseen.

## Context

Gaupol's correction patterns — 81 records: 63 common errors, 3 capitalization,
6 hearing-impaired, 9 line-break — are written for Python's `re` module.
**25 carry a lookaround** and **38 replacements a backreference**: RE2, which
has neither, is ruled out without measurement.

Two candidates remain:

- **PCRE2**, which the roadmap had named since phase 0 for its closeness to
  Python's syntax;
- **ICU**, which nobody named at the time and which is nevertheless **already in
  the tree twice**: a core dependency since [ADR 0027](0027-icu-pour-les-encodages.md),
  and the engine of the window's search since phase 10
  (`core/edit/search.cpp`, `icu::RegexMatcher`), without an ADR — the index
  flagged it as a third candidate.

ADR 0017 set its own trigger: "the third pattern requested, whatever it is,
reopens this ADR". Phase 12 requests four more for hearing-impaired mentions alone.

## What was measured

**A throwaway measurement**, written outside the repository for this scoping,
as #492 had decided: both engines apply the same patterns, with the semantics of
Gaupol's `Finder` (search in the already modified text, advance by one character
after an empty match, `Repeat` that loops back), and their outputs are checked
against those of the Python oracle of
[#494](https://github.com/Guyot-Bertrand/sub-edit/issues/494). ICU 74.2, PCRE2
10.42, GCC 13 at `-O2`, loaded machine (load average 3.5 to 5): the durations
are orders of magnitude, not a bench record.

**Time**, the 63 common-error patterns all applied, `Repeat` included, to
85,000 real subtitles (the private corpus, repeated):

| Engine | Duration | Per film of 4,000 subtitles |
| :----- | -------: | --------------------------: |
| PCRE2, compiled on the fly (JIT) | 1.09 s | 0.05 s |
| ICU | 2.85 s | 0.13 s |
| PCRE2, interpreted | 3.95 s | 0.19 s |

On texts where almost everything needs correcting — the inputs of #494,
repeated up to 85,000 —, the ratio does not change: 0.42 s, 0.99 s, 1.14 s.
Compiling the 81 patterns takes less than four milliseconds in each.

**Fidelity**, on the 18,563 subtitles of the private corpus, each pattern type
applied as a substitution:

| Type | PCRE2 10.42 vs Python | ICU vs Python | ICU, `\w` and `\b` rewritten |
| :--- | --------------------: | ------------: | ---------------------------: |
| common errors | 0 | 0 | 0 |
| capitalization | 0 | **8** | 0 |
| hearing-impaired | 0 | 0 | 0 |
| line breaks | 0 | **12** | 0 |

ICU's twenty discrepancies have a single cause: **a letter written in
decomposed form**, `i` followed by a combining accent. For Python, `\w` is
`str.isalnum()` plus the underscore, and a combining accent is not one; for
ICU, `\w` includes marks, and `\b` ignores them. Rewriting `\w`, `\W`, `\b` and
`\B` at load time with Python's class — `[\p{L}\p{N}_]` — brings ICU back to
zero discrepancies on all four types, for **a cost of 10 %** (0.68 s instead of
0.61 s on the corpus alone).

**And PCRE2's agreement is an accident of version.** Its version 10.43 changed
`\w` in Unicode mode to follow Perl: it now includes non-spacing marks and
connector punctuation (PCRE2 changelog, 10.43, item 32). The 10.42 on this
machine is Ubuntu 24.04's; Fedora ships a 10.44. The `.rpm` package would
therefore link an engine that does not produce the same texts as the `.deb`.
**The rewriting of `\w` is due whatever the engine** — and the argument that
made PCRE2 preferable, "compatible with Python's syntax", falls with it.

**What neither reads as is**: `\Z`, the absolute end in Python, which both
write `\z`. PCRE2 additionally refuses `\uXXXX`, which it writes `\x{XXXX}`,
and ICU refuses Python-style named groups, `(?P<name>…)` and `(?P=name)` — which
no shipped pattern uses, but which a user pattern may write. Replacements,
written `\1` and `\040` for Python, are understood by neither: that is decision
D3 of the spec, which converts them at load time.

## Decision

**ICU**, through `icu::RegexMatcher`, the engine the search already uses. The
patterns are translated from Python's syntax to ICU's **at load time**, once,
by a core function that knows the closed list of what it rewrites — `\Z`, `\w`,
`\W`, `\b`, `\B`, `(?P<…>)`, `(?P=…)` — and that refuses, rather than guesses,
what it cannot translate.

## What decides

**Time does not settle it.** 0.13 s to correct a whole film, in an assistant
that then presents each change for confirmation: the difference with PCRE2 is
invisible. The phase's hot spot is elsewhere — the length measurement, called
for every line-break candidate, and not the patterns, which line breaking
applies only once per text to place its penalties.

**A single syntax for the user.** The search field already reads ICU
expressions. A pattern written by the user in a pattern file, and the
expression typed in `Find and Replace…`, will obey the same rules; with PCRE2,
two engines would answer differently to the same expression in the same
program.

**No additional dependency.** ICU has been in the core since phase 8, and
`subedit-cli` already pulls it in. PCRE2 would be the core's third external
dependency, and, with the JIT, a page of executable memory written at run time
— which some security policies refuse.

**A safeguard against a badly written pattern.** `RegexMatcher::setTimeLimit`
bounds the catastrophic backtracking a user pattern can trigger; the engine
stops, the correction of that text is abandoned and reported. PCRE2 has its
equivalent — the question does not settle anything, but it needed its answer.

## Alternatives dismissed

- **PCRE2 with JIT** — 2.6 times faster, for a difference no user will see on a
  film, at the price of a dependency, a second engine in the program and a
  behaviour that changes from one distribution to another. If one day a use
  makes the time visible — batch processing of hundreds of files in phase 13 —,
  the measurement is to be redone, and the engine interface (D1 of the spec) is
  what makes the replacement local.
- **RE2** — neither lookaround nor backreference: 25 patterns out of 81 could
  not be written.
- **`std::regex`** — no lookbehind, known slowness, and ADR 0017 already
  dismissed it for mentions.
- **Rewriting the patterns by hand in a common subset** — Gaupol's files would
  stop being Gaupol's, and the decision on the format
  ([ADR 0037](0037-lire-les-motifs-de-gaupol-tels-quels.md)) rests on reading
  them as they are.

## Consequences

**What becomes of ADR 0017.** Its trigger has fired, and its decision **holds
for its two patterns**: brackets and parentheses stay with the hand-written
scan, for the reason it gave — the seam is local to the removal site. The four
other hearing-impaired records go through the engine. That is decision D7 of the
spec; 0017 is not superseded.

**A syntax translation to maintain.** It lives in the core, it is exercised by
the 184 cases of #494 and by those of #495, and it refuses what it does not
know: a pattern that does not translate is **disabled and named** — file,
record, reason —, never applied at random.
[ADR 0008](0008-lecture-au-mieux-avec-diagnostics.md) already says how to read
on a best-effort basis while reporting what is wrong.

**The search may benefit from it, without being obliged to.** It has its own
replacement expansion (`$1`), written for what a user types; the phase does not
change it.

**What would justify reopening**: a measurement, in the permanent bench the
phase installs along with the engine, where correcting a file becomes
noticeable — beyond one second for a film —, or a Gaupol pattern ICU cannot
express.
