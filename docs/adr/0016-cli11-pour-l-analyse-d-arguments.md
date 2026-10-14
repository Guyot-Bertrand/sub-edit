# 0016 — Use CLI11 for argument parsing

**Date:** 2026-08-14
**Status:** accepted

## Context

Phase 3 gives the binary five subcommands — `inspect`, `convert`, `shift`,
`transform`, `framerate` — each with its options, its default values, its
forbidden combinations and its refusals. Until now the binary parsed nothing:
it wrote its version and ignored everything passed to it.

The project manual requires every option to be described with its long and
short form, whether it is required, its default value, the closed set of its
accepted values, and the error message for each refusal. That is a heavy
constraint, and it bears precisely on what an argument parser produces.

## Decision

CLI11, resolved by `find_package` on the distribution package
(`libcli11-dev`), as [ADR 0004](0004-gestion-des-dependances.md) requires.

## Alternatives dismissed

- **An implementation of our own** — zero dependencies, which is the project's
  natural inclination. Dismissed on the ratio between what it costs and what it
  avoids: five subcommands, their options, their help and their error messages
  are exactly the material where a hand-written parser drifts — not by
  breaking, but by gradually becoming inconsistent from one subcommand to the
  next. The manual would make that inconsistency visible without fixing it.
- **cxxopts** — also packaged and lighter. Dismissed because it has no notion
  of subcommand: dispatching, per-subcommand help and validation of
  combinations would remain to be written, that is, the bulk of what we were
  trying not to write.

## Consequences

The help is **generated** from the option declarations, not maintained
alongside them. Combined with `make manual`, delivered by issue #52, this
closes the loop: the declaration produces the help, the help produces the
manual's block, and nothing is copied at any step. An option added without a
word in the manual becomes impossible rather than unlikely.

One more dependency to install, added to `setup-toolchain.sh`. It concerns only
`src/exe/`: the core stays dependency-free, and nothing in `src/lib/` knows
CLI11 — a CLI is only one caller among others, and the phase 5 window will be a
second one.

**What makes the decision cheap to undo:** argument parsing lives in a single
file of `src/exe/`, which translates a command line into calls to the core.
Changing it means rewriting that file, without touching either the core or the
end-to-end tests, which go through the binary and ignore what is inside it.
