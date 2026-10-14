# 0003 — Target Linux first, without closing the door to other systems

**Date:** 2026-08-04
**Status:** accepted

## Context

The end user works on Linux, like Gaupol, whose Windows installers have not been
produced since version 1.3.1. Targeting three systems from the start would
multiply the cost of every decision — dependencies, packaging, video player,
tests — for a need that does not exist yet.

But a later port must not require changing the technical foundation.

## Decision

Linux is the only platform built, tested and packaged. On the other hand, **no
choice may make a port costly**: at comparable quality, the portable option is
always chosen.

## Alternatives dismissed

- **Linux only, with no regard for portability** — the simplest in the short
  term. Dismissed because non-portable choices are discovered late, once they
  are embedded: that is what cost Gaupol Windows.
- **Three platforms from the start** — would multiply the work in every phase
  for a hypothetical need.

## Consequences

This decision concretely steers several later choices:

- Qt rather than GTK — see [0001](0001-cpp20-et-qt6.md);
- no direct call to POSIX APIs where the standard library or Qt offers an
  equivalent;
- `std::filesystem` rather than the POSIX path-handling functions;
- the choice of video backend (phase 6) will have to take its availability on
  Windows into account.

The trigger that would reopen this decision: a real need expressed for Windows.
It would also reopen [0004](0004-gestion-des-dependances.md), since the
dependencies would then have to be versioned identically on both systems.
