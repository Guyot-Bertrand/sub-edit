# 0004 — Resolve dependencies through system packages

**Date:** 2026-08-04
**Status:** accepted

## Context

The project will depend on Qt 6, a regular expression engine, ICU and a spell
checker. Two C++ package managers could have been adopted from the outset:
Conan 2 and vcpkg.

## Decision

Dependencies are resolved by `find_package` on the distribution's packages. No
C++ package manager is set up.

Catch2 is the exception, fetched by `FetchContent`: it compiles in a few seconds
and its version must be the same everywhere, otherwise a distribution update
could break the tests.

## Alternatives dismissed

- **vcpkg right now** — would guarantee identical versions on every platform,
  but compiles Qt from source: hours on the first build and on every CI run with
  a cold cache.
- **Conan 2 right now** — better than vcpkg on versioning, build profiles and
  binary packages. Dismissed because it does not keep its promise where it would
  matter: ConanCenter's Qt recipe is heavy and often rebuilds from source. We
  would pay for a second build system to save installing a few packages.

## Consequences

The first build is fast and depends only on `apt`. In return, the dependency
versions are the distribution's, and so vary from one machine to another —
acceptable as long as the target is a single platform.

**What makes the decision cheap to undo:** Conan and vcpkg both work by
interposing themselves on `find_package`. As long as the project's CMake is
written around that mechanism, and not around hard-coded paths, adopting them
later will take a manifest file, not a rewrite.

The trigger: an actual Windows port — see [0003](0003-linux-d-abord.md).
