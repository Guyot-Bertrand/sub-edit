# 0007 — Move the project to C++23 and report errors through `std::expected`

**Date:** 2026-08-05
**Status:** accepted — partly supersedes [0001](0001-cpp20-et-qt6.md)

## Context

A subtitle editor spends its time reading imperfect files. Decision
[0008](0008-lecture-au-mieux-avec-diagnostics.md) requires opening as best we
can **and** reporting what was not understood: a reading function must
therefore be able to return either a result or an error, and in both cases a
list of diagnostics.

This shape is naturally modelled by a result type. `std::expected` belongs to
C++23, whereas [0001](0001-cpp20-et-qt6.md) chose C++20.

## Decision

The project moves to **C++23**, and functions whose failure is an expected
outcome return `std::expected<T, E>`.

Exceptions stay reserved for what is truly exceptional: memory exhaustion,
violation of an internal invariant, programming error.

## Checks made before deciding

Measured on 2026-08-05 on the target machine, rather than assumed:

- **`std::expected` works with the already installed GCC 13.3**, in
  `-std=c++23`. No compiler change, no extra package.
- **clang-tidy 18 did not see the type.** libstdc++ keeps `<expected>` behind
  `__cpp_concepts >= 202002L`, a value Clang 18 does not declare: the header's
  content vanished, and static analysis would have been blind on every file
  using it.
- **clang-tidy 20.1.2, available in the Ubuntu repositories, reads it without
  error.** The toolchain was updated accordingly.

GCC 13's C++23 mode is still partial: `std::print`, *deducing this*,
`std::ranges::to` and `std::generator` require GCC 14, available in the
repositories. None of them is needed; the day one becomes so, a package will be
enough.

## Alternatives dismissed

- **A home-made result type in C++20** — some thirty lines, no dependency on a
  partial standard mode. Dismissed because the real cost is not writing the type
  but maintaining it and teaching its semantics to every reader, whereas
  `std::expected` is documented everywhere and well known.
- **Exceptions** — the shortest to write. Dismissed on two counts: an exception
  carries a **list** of diagnostics accompanying a partial result badly, which
  is precisely our case; and propagating one exception per malformed line over a
  file of several thousand lines is expensive on a hot path.
- **Return codes with an output parameter** — forces the result to be built
  before knowing whether it is valid, and is silently bypassed by ignoring the
  code. `[[nodiscard]]` on `std::expected` makes the omission visible.
- **Working around clang-tidy 18 with `-D__cpp_concepts=202002L`** — tested,
  working. Dismissed because it makes the analyser believe it supports a level
  of *concepts* it does not have, and so makes it follow libstdc++ paths it
  handles badly. Fixing a diagnostic by lying to the tool that produces it fixes
  nothing.

## Consequences

`CMAKE_CXX_STANDARD` moves to 23. The other decisions of
[0001](0001-cpp20-et-qt6.md) — Qt 6, a core with no dependency on the
interface — remain valid.

The exclusion of C++ modules remains in force: their tooling has not improved.

The toolchain now depends on clang-tidy ≥ 19. The `Makefile` takes the most
recent version available and falls back on `clang-tidy` without requiring a
specific version to be installed; the CI explicitly installs version 20.

The trigger that would reopen this decision: a target platform whose compiler
does not provide `std::expected`. MSVC has provided it since Visual Studio 2022
17.3, so the Windows port of [0003](0003-linux-d-abord.md) is not affected.
