# 0043 — Write the whole repository in English; French is a locale

**Date:** 2026-10-09
**Status:** accepted

Replaces the language split of [#312](https://github.com/Guyot-Bertrand/sub-edit/issues/312) (C++ and test titles in English, everything else in French). Decided by the maintainer on 2026-10-09 and framed in phase 15 ([#655](https://github.com/Guyot-Bertrand/sub-edit/issues/655)).

## Context

The split of #312 held while the product spoke one language. Phase 15 gives it several, and the maintainer asked for the documentation to be translated into English and for issues to be written in English. About 220 000 hand-written words are in French: manuals 72 k, specs 88 k, ADRs 42 k, roadmap 15 k, `CLAUDE.md` 4 k; scripts, `Makefile` and CMake comments add about 17 000 lines.

## Decision

Everything is written in English: C++, scripts, build files, documentation, commit messages, pull request titles, issue titles and bodies, test titles. **French exists as a locale only**: the catalogue `src/po/fr.po` and the translated manual `docs/i18n/fr/manual/`. Further languages are added the same way. History is not rewritten.

The translation of what exists is done in place, in batches, behind a **descending allowlist** in the comment-language guard: French is refused everywhere except the two locale trees and the files still listed; an entry can be removed and never added.

## Alternatives dismissed

- **Keep both languages as sources** — every change written twice, and the two texts diverge.
- **Drop French altogether** — the interface and the manual would no longer follow the language for the maintainer's own language, the first one shipped.
- **Translate the manuals only and leave the rest French** — a repository in two languages with no end date, and contributors who do not read French still cannot read the specs and ADRs. Decided against on 2026-10-09: translate all of it.
- **Rewrite the git history** — would break every link to a commit and every tag.

## Consequences

- One language for contributors; the manual is the only French text and is a translation with a structure check ([#656](https://github.com/Guyot-Bertrand/sub-edit/issues/656)).
- The repository is mixed until the last batch; the allowlist makes it finite and measurable.
- Documentation guards that matched French words need a vocabulary per language ([#657](https://github.com/Guyot-Bertrand/sub-edit/issues/657)).
- Translation is done by Claude and read by the maintainer in English; identifiers (requirement IDs, ADR numbers, anchors, option names) never change.
