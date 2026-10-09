# 0044 — One manual feeds the window's help and the man pages

**Date:** 2026-10-09
**Status:** accepted

Decided at the framing of phase 15 ([#655](https://github.com/Guyot-Bertrand/sub-edit/issues/655)), on the roadmap's question of whether the help and the `man` pages share a source.

## Context

The roadmap asks for an internationalised help page and `man` pages for the command line. `Help ▸ Manual` already opens the Markdown installed under `share/subedit/manual`. The command-line manual is checked against `--help` by `check-cli-manual.py`. `pandoc` is not installed, and the manual's Markdown is a small, constrained subset (headings, paragraphs, tables, code blocks, links).

## Decision

**The Markdown manual is the single source.** The window's help installs `docs/manual` and every installed language's tree (`docs/i18n/<lang>/manual`) and opens the chosen language, English when the page or the language is missing. The `man` pages are **generated** from the command-line part of the same Markdown by `src/scripts/md-to-man.py`: `subedit-cli(1)` and one page per sub-command, per language, installed under `share/man/<lang>/man1`.

## Alternatives dismissed

- **Hand-written `man` pages** — two texts that diverge, as two texts always do; the option lists would have to be checked against `--help` twice.
- **`pandoc`** — one more build dependency for a Markdown subset a converter of about a hundred lines handles, and a converter we can test with versioned fixtures.
- **Generating the `man` pages from `--help` alone** — drops the examples, the exit codes and the explanations that the manual carries and the `man` page is expected to have.

## Consequences

- A change to the manual changes the help and the `man` pages with no further step; the structure check of #656 keeps the translated trees in step.
- The converter is ours to maintain; the manual must stay inside the subset it handles, which a test on every page enforces.
- The screenshots are shared by all trees until a language's interface catalogue is installed.
