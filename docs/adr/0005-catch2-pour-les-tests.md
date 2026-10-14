# 0005 — Use Catch2 v3 for tests and benchmarks

**Date:** 2026-08-04
**Status:** accepted

## Context

Performance being a cross-cutting design goal, benchmarks are not a late
addition: they must be tooled from the foundations, so that later optimisations
are argued from measurements rather than intuitions.

## Decision

Catch2 v3, for unit tests **and** micro-benchmarks.

## Alternatives dismissed

- **GoogleTest + Google Benchmark** — the most widespread pair, with gmock
  thrown in. Dismissed because it makes two dependencies instead of one for a
  need Catch2 covers alone, and because object mocking is of little use here:
  the core is a data transformation library, testable on real values.
- **doctest + nanobench** — markedly faster compilation and a lighter footprint.
  Dismissed on the maturity of the ecosystem and the documentation; the
  compilation gain would only be felt at a scale the project does not reach.

## Consequences

Writing a benchmark next to a test takes no extra dependency, which removes the
main obstacle to writing them.

`catch_discover_tests` registers each case individually in CTest: a failure
names the faulty case, and a single case can be rerun on its own.

The benchmarks form a separate executable, not registered in CTest: they are
measured in Release, never during a test pass instrumented by the sanitizers or
coverage, which would skew the figures.
