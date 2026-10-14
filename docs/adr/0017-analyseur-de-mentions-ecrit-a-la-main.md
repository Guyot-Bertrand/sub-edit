# 0017 — Write the mention parser by hand, without a regular expression engine

**Date:** 2026-08-15
**Status:** accepted

## Context

Phase 4 removes hearing-impaired mentions from a subtitle file: the sounds
described between square brackets or between parentheses. Gaupol does it with
two regular expressions, `\[.*?\]` and `\(.*?\)`, in `DOTALL`.

The project aims at feature parity, so the natural inclination would be to take
over those two patterns. The roadmap had in fact anticipated it, noting that
those patterns have **no back-reference** — the choice between PCRE2 and RE2,
which will come up in phase 12, does not block this one.

What remains to decide is what this phase recognises mentions with.

## Decision

**A hand-written scan**, in `src/lib/subedit/core/text/`. No regular expression
engine: neither PCRE2, nor RE2, nor `std::regex`.

## What decides: the seam is local, a pass is global

Gaupol's two patterns are not enough, and Gaupol knows it: it follows them
with **seven clean-up passes** (`_remove_leftover_hi`) that patch up what the
removal left behind — line edges, consecutive spaces, empty lines, orphaned
dialogue dashes.

Our rule, written case by case in `src/test/data/textes/mentions.cas` before
any code, says something else:

> A removed mention leaves **exactly one space between what surrounded it**,
> and **nothing at a line edge**. It is not "patch up the stray spaces":
> `Bonjour[il tousse]Marie` had none and gains one.

**It is a rule about the site of the removal.** A regular expression pass, on
the other hand, does not know where the mention was: `" {2,}" → " "` rewrites
every double space in the text, including those the file already carried and
that nobody asked to touch. The global rule is not an approximation of the
local rule, it is a different one — and on real text, the two do not give the
same result.

To stay local, one must know where the mention was: hence walk the matches one
by one and assemble the output oneself. At that point, the parser is written,
and the engine only provides "find the next `[…]`" — a fifteen-line loop. The
same reasoning holds for the mention split by a line break: the scan trivially
knows whether the removed piece contained a `\n`, whereas an expression would
have to be asked again in the form of one more capture.

What the engine would bring — two declarative, readable patterns — is
therefore not what is difficult here.

## Alternatives dismissed

- **`std::regex`** — no dependency to install, since it is the standard
  library, and it can do more than it seems: keeping numeric references fits in
  an alternation that captures what is kept,
  `(\[[ \t]*[0-9]+[ \t]*\])|\[[\s\S]*?\]|\([\s\S]*?\)` replaced by `$1`.
  Dismissed on the seam, argument above, and not on its known shortcomings —
  the absence of a `DOTALL` flag, worked around with `[\s\S]`, and its
  slowness, which would weigh on the path that the #89 fixture has just made
  measurable. It would moreover be a third engine, adopted only to be replaced
  in phase 12.
- **Settling PCRE2 or RE2 right now** — so as to do it only once. But that
  would mean choosing an engine for needs we do not know: back-references,
  per-language patterns and the spell checker arrive in phase 12, and that is
  where the comparison criteria will appear. The only use we could measure
  today — two literal delimiters — does not separate two engines.
- **Taking over Gaupol's pattern data** — dismissed in #88, for its own
  reasons: the copy would be lawful, both projects being under GPL-3, but it
  would commit us to a file format we did not choose, including an XML `.conf`
  and homonymous INI sections that no standard reader accepts.

## Consequences

**No new dependency**, which leaves the engine question whole and open for
phase 12 — it will settle it with its real criteria in front of it.

**A pure function**, `withoutHearingImpaired`, without state or context: the
cases of `mentions.cas` exercise it directly, without building a project or a
command. An engine would have imposed its own — pattern compilation, lifetime,
compilation error handling — between the cases and the rule.

**What makes the decision cheap to undo:** the rule lives behind a free
function with a single signature. Phase 12, if its engine can express the
local seam, will rewrite its body without touching the command, the CLI, or
the cases that exercise it — and they are what will tell whether the rewrite is
faithful.

**What will need watching**: the temptation to add the phase 12 patterns to
the scan one by one, until a regular expression engine has been written
without being decided. The trigger is written: the third pattern requested,
whatever it is, reopens this ADR.

## Update — #691: square brackets only, by default

The scoping said "what holds between square brackets holds between
parentheses", and the scan removed both. **A parenthesis is as often a
whispered line as a described sound**, and removing both by default erased
dialogue. The scan now takes its delimiters as a parameter
(`MentionBrackets`): **square brackets only by default**, parentheses on
request — `--parentheses` for `hearing-impaired`, the `Sound in parentheses`
box for the assistant, each of whose boxes now controls its own delimiter. The
rest of the ADR — the hand-written parser, the seam rule — is unchanged, and
the parenthesis cases live in `mentions-parentheses.cas`.
