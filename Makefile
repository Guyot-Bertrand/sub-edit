# An ergonomic front end to CMake.
#
# This file adds no build logic: it shortens command lines. The rule that
# matters is `check`, described in docs/specs/00-fondations.md.

.DEFAULT_GOAL := help
SHELL := /bin/bash
.SHELLFLAGS := -eu -o pipefail -c

# Number of parallel jobs, for compilation and for static analysis.
#
# **Two cores by default, no more.** The development machine runs other
# projects at the same time, and a build that takes every core makes their
# tests fail on timeouts: a defect elsewhere, caused here. Two is the agreed
# ceiling for local work — a rule, not a case-by-case judgement, precisely
# because the threshold is invisible: you do not see what you break next door
# until you have broken it. The figure comes from a measurement: `make check`
# on a single core takes about 17 minutes, about 15 of them for clang-tidy
# alone.
#
# Raise it when needed, without touching this file:
#
#     make build JOBS=8
#     JOBS=$$(nproc) make check
#
# The CI passes `nproc` explicitly: its machine is its own, and the ceiling
# does not concern it.
#
# src/scripts/check-parallelism.sh checks mechanically that no recipe of this
# file and no script of src/scripts/ bypasses $(JOBS) with hard-wired
# parallelism.
#
# Tests stay sequential whatever happens. This is not an oversight: `ctest`
# parallelises only on an explicit `-j`, and giving it one would merge the
# `.gcda` files of several concurrent runs, hence a wrong coverage rate with no
# sign but a warning drowned in the output.
#
# **`JOBS` bounds processes, not threads** — #630. libmpv creates some for each player and ffmpeg
# for each encoding: one player test case kept more than five cores busy. Every `ctest` of the
# gate therefore goes through src/scripts/limit-cores.sh, which sets the affinity to `$(JOBS)`
# cores; `make parallelism` refuses a `ctest` that does not.
JOBS ?= 2

# **`JOBS` and `BENCH_MAX_LOAD` are exported, and read by the steps.**
#
# The steps live in src/scripts/gate/ since #269, and a recipe variable would
# not reach them. The default of each is written here, with the measurement
# that justifies it, and the steps take it up without redefining it: two places
# to set a figure would make two truths.
#
# One and a half, not two, for the load: the threshold was lowered on the very
# day it was set. The reading of version 0.3.5, taken at 1.88 — under the
# threshold, hence accepted — set a maximum of 853 µs for writing 4000
# subtitles, where the envelope of the seventeen previous readings ran from 492
# to 641. The readings taken under 1.4 only set minima.
#
# The figure remains a heuristic, and the log is there to refine it: each
# reading carries its load, so the relation between the two can now be read.
BENCH_MAX_LOAD ?= 1.5
export BENCH_MAX_LOAD
export JOBS

# git-cliff installs into ~/.local/bin, which ~/.profile adds to the PATH only
# at the next login. We do not depend on the configuration of the user's
# shell.
export PATH := $(HOME)/.local/bin:$(PATH)

# Ninja is preferred, without being required: fixing it in the presets would
# make the project unbuildable on a machine that does not have it yet.
export CMAKE_GENERATOR ?= $(shell command -v ninja >/dev/null 2>&1 && echo Ninja || echo "Unix Makefiles")

# **Static analysis no longer has a knob, and that is the subject of #269.**
#
# It had one — `TIDY_BASE` — and a two-hundred-line script that computed a
# scope from a git diff, with a list of "governing files" that made everything
# start over. This file was one of them: a comment added to a recipe took the
# analysis from 2 files to 223.
#
# Yet clang-tidy never reads a Makefile. What the result of a translation unit
# depends on — its source, its headers, its command line — the build system
# already knows, exactly. clang-tidy is therefore hooked to the compile rule of
# each source, under the `tidy` preset, and there is nothing left to compute or
# pass. See cmake/Tidy.cmake.
#
# To analyse everything again: `rm -rf build/tidy`.

BOLD := \033[1m
GREEN := \033[32m
RED := \033[31m
RESET := \033[0m

define step
	@printf '$(BOLD)▸ %s$(RESET)\n' $(1)
endef

# Fails with a useful message rather than a "command not found" error.
define require
	@command -v $(1) >/dev/null 2>&1 || { \
		printf '$(RED)missing tool: %s$(RESET)\n' $(1) >&2; \
		printf '  install it with: ./src/scripts/setup-toolchain.sh\n' >&2; \
		exit 1; \
	}
endef

.PHONY: help
help: ## Show this help
	@printf '$(BOLD)subedit$(RESET) — available targets\n\n'
	@grep -hE '^[a-z0-9-]+:.*?## ' $(MAKEFILE_LIST) \
		| sort \
		| awk 'BEGIN {FS = ":.*?## "}; {printf "  $(BOLD)%-12s$(RESET) %s\n", $$1, $$2}'
	@printf '\ngenerator: $(CMAKE_GENERATOR)\n'
	@printf 'parallel jobs: $(JOBS)   (raise with JOBS=N)\n'

.PHONY: setup
setup: ## Install the toolchain and the git hooks
	@./src/scripts/setup-toolchain.sh

.PHONY: build
build: ## Build the dev preset
	@./src/scripts/gate.sh build

# The release counterpart of `build`, and it produces only what a user runs.
#
# **It exists because we did without it.** The workaround was to run
# `make bench` and interrupt it once linking was over: it works, and it has
# three defects — it also builds `subedit_bench`, which is of no use here; it
# leaves a half-done run, of which one no longer knows whether it wrote to the
# measurement log; and it has to be watched to know when to interrupt.
#
# Neither the bench nor the end-to-end harness, then: they have their own
# targets. What is asked for here is what a user runs, not what tests it.
#
# `JOBS` governs LTO, for the reason written on `bench`: without it,
# cross-module optimisation goes to `-flto=auto`, that is as many processes as
# cores, at every link — parallelism that appears in no `-j` and that saturates
# a machine on which something else is being done. That is also what makes
# this target preferable to the two `cmake` commands written by hand, which
# know nothing of it.
#
# **It runs nothing.** It is what packaging will install — ADR 0023,
# issue #244.
.PHONY: release
release: ## Produce the two optimised binaries, and nothing else
	@./src/scripts/gate.sh release

# Does not run the end-to-end tests: they register with CTest only under the
# asan and release presets — see SUBEDIT_REGISTER_E2E in
# src/test/e2e/CMakeLists.txt. `make asan` is the fastest way to see them run
# in a development loop; `make e2e` targets release alone.
.PHONY: test
test: ## Build and run the tests (end-to-end excluded — see make asan)
	$(call step,"tests (dev)")
	@cmake --preset dev
	@cmake --build --preset dev -j $(JOBS)
	@./src/scripts/limit-cores.sh ctest --preset dev

.PHONY: bench
bench: ## Run the benchmarks in release and add the figures to the log
	@./src/scripts/gate.sh bench

.PHONY: format
format: ## Apply clang-format
	@./src/scripts/gate.sh format

.PHONY: format-check
format-check: ## Check the format without modifying anything
	@./src/scripts/gate.sh format-check

.PHONY: tidy
tidy: ## Static analysis — clang-tidy on the compilation of each source
	@./src/scripts/gate.sh tidy

.PHONY: arch
arch: ## Check the architecture invariants
	$(call step,"architecture invariants")
	@./src/scripts/check-architecture.sh

.PHONY: parallelism
parallelism: ## Check that no parallelism bypasses $(JOBS)
	$(call step,"bounded parallelism")
	@./src/scripts/check-parallelism.sh

# User-visible strings outside the translation API — issue #661. `untranslated` (in `check`)
# holds the count to its ratchet; `untranslated-record` rewrites the ratchet once a conversion
# has lowered it, and the gate refuses a record above the one at the merge base.
.PHONY: untranslated
untranslated: ## Refuse a user-visible string outside the translation API beyond the ratchet
	@./src/scripts/gate.sh untranslated

.PHONY: untranslated-record
untranslated-record: ## Rewrite .untranslated-ratchet from the counts of the tree
	@./src/scripts/check-translatable-strings.py --record

# The message catalogues — issues #659 and #667. `pot` rewrites the template from the code, `po`
# merges it into every catalogue of src/po/ (the installed languages of LINGUAS and the seeds from
# Gaupol), `pot-check` (in `check-local`) refuses a stale template. The seeds themselves are written
# by `src/scripts/seed-catalogues.py`, run by hand: see src/po/README.md.
.PHONY: pot
pot: ## Rewrite src/po/subedit.pot from the code
	@./src/scripts/update-pot.sh

.PHONY: po
po: ## Merge the template into every catalogue of src/po/, seeds included
	@./src/scripts/update-po.sh

.PHONY: pot-check
pot-check: ## Check that src/po/subedit.pot is what the code produces
	@./src/scripts/gate.sh pot-check

# Translations that fall behind their source — issue #670. The template is merged into a scratch
# copy of every catalogue, as `make po` would; an installed language must come out complete, a
# seed is only reported. In `check-local`, after `pot-check`.
.PHONY: translations
translations: ## Refuse an installed language with an untranslated, fuzzy or stale entry
	@./src/scripts/gate.sh translations

.PHONY: fixtures
fixtures: ## Check that the generated fixtures are what their table says
	@./src/scripts/gate.sh fixtures

# The score of encoding detection, and its recording — issue #311.
#
# **Two targets for the two gestures**, like `coverage` and `ratchet`: one
# measures and refuses a regression, the other writes a measurement to the log.
# A score that rises is good news that calls for rewriting the log, and nothing
# should rewrite it on its own — that would erase the comparison one came to
# make.
.PHONY: score
score: ## Replay the encoding detection score and compare it with the record
	@./src/scripts/gate.sh score

.PHONY: score-record
score-record: ## Record the measured detection score as the new record
	$(call step,"encoding detection record")
	@cmake --preset dev >/dev/null
	@cmake --build --preset dev -j $(JOBS) --target subedit_detect_encoding
	@./src/scripts/score-encoding-detection.py --record \
		--detector './build/dev/bin/subedit_detect_encoding {}'

# The score of format detection, and its recording — issue #340.
#
# **Two targets for the two gestures**, like `score` and `score-record`. A
# confusion, for its part, fails on its own: a format opened as what it is not
# is what the project's rule forbids, and a rising rate does not make up for
# it.
.PHONY: score-format
score-format: ## Replay the format detection score and compare it with the record
	@./src/scripts/gate.sh score-format

.PHONY: score-format-record
score-format-record: ## Record the format detection score as the new record
	$(call step,"format detection record")
	@cmake --preset dev >/dev/null
	@cmake --build --preset dev -j $(JOBS) --target subedit_detect_format
	@./src/scripts/score-format-detection.py --record \
		--detector './build/dev/bin/subedit_detect_format {}'

# The conversion loss, and its recording — issue #339.
#
# **Two targets for the two gestures**, like `score` and `score-record`. A loss
# that shrinks is good news that calls for rewriting the log, and nothing
# should rewrite it on its own: that would erase the comparison one came to
# make.
.PHONY: conversion
conversion: ## Replay what a conversion loses and compare it with the record
	@./src/scripts/gate.sh conversion

.PHONY: conversion-record
conversion-record: ## Record the measured conversion loss as the new record
	$(call step,"conversion loss record")
	@cmake --preset dev >/dev/null
	@cmake --build --preset dev -j $(JOBS) --target subedit-cli
	@./src/scripts/measure-conversion-loss.py --record \
		--binary ./build/dev/bin/subedit-cli

.PHONY: manual
manual: ## Regenerate the manual's invocation examples and screenshots
	@./src/scripts/gate.sh manual

.PHONY: manual-check
manual-check: ## Check that the manual's examples, screenshots and cross-references are up to date
	@./src/scripts/gate.sh manual-check

.PHONY: requirements
requirements: ## Compare the requirements register with the tests and the specs
	@./src/scripts/gate.sh requirements

# Builds and runs only the end-to-end harness, filtered by the CTest label
# `e2e` rather than by test name — a unit test name that came close to it
# would not fool it. Shares the release build with `make bench`: the second
# invocation recompiles nothing.
.PHONY: e2e
e2e: ## Run only the end-to-end tests (release)
	@./src/scripts/gate.sh e2e

.PHONY: packages
packages: ## Produce the .deb and the .rpm from the release tree
	@./src/scripts/gate.sh packages

.PHONY: install-check
install-check: ## Install into a temporary prefix and run the installed binary
	@./src/scripts/gate.sh install-check

# **Outside `check-local`, and deliberately so** — issue #266. It downloads
# nearly three hundred megabytes and needs the network; a gate that depends on
# the network is red on the day of an outage, for a reason foreign to the
# repository. What guards pull requests is the local check that `install-check`
# now carries: an `.rpm` owns only its own directories.
.PHONY: rpm-check
rpm-check: ## Install the .rpm on a containerised Fedora, and run what is installed
	@./src/scripts/gate.sh rpm-check

.PHONY: asan
asan: ## Run the tests under ASan and UBSan
	@./src/scripts/gate.sh asan

.PHONY: coverage
coverage: ## Measure the coverage of the libraries
	@./src/scripts/gate.sh coverage

.PHONY: ratchet
ratchet: ## Record the measured coverage as the new ratchet
	$(call step,"coverage ratchet")
	@./src/scripts/check-coverage.sh \
		--summary build/coverage-report/summary.json --record

.PHONY: untracked
untracked: ## Refuse an untracked file that appeared since the last snapshot
	@./src/scripts/gate.sh untracked

.PHONY: config-home
config-home: ## Refuse a run that touches the user's configuration
	@./src/scripts/gate.sh config-home

# **The order of the steps is no longer here but in src/scripts/gate.sh**,
# with the reason for each. What moving it gains: the gate can resume in the
# middle, which a chain of sub-`make` calls could not do.
#
#     ./src/scripts/gate.sh check --from coverage
#     ./src/scripts/gate.sh check --only tidy
#     ./src/scripts/gate.sh --list
#
# A coverage failure at the thirty-fifth minute used to cost the first
# thirty-four again.
.PHONY: check
check: ## Quality gate — format, warnings, tidy, tests under ASan, coverage
	@./src/scripts/gate.sh check

# `check` is the gate — .github/workflows/ci.yml calls only this target, when
# it is started by hand, and since #232 that is the only way it runs.
# Everything that goes into it therefore gates every pull request; nothing is
# added to it lightly.
#
# `check-local` is the one command to run before opening a pull request: it
# chains everything we want to see pass locally without having it gate the
# CI.
#
# **It runs before `check`, not after** — decision of the end-of-phase-8
# review, ADR 0015. The procedure prescribed the reverse, and the bench it
# chains then met the tail of a quarter of an hour of compilation: three
# readings lost out of six in phase 8, six out of thirteen in phase 7. This
# order is also the one the principle already wanted — cheapest to most
# expensive, and `check-local` lasts a few minutes where `check` lasts
# seventeen.
#
# The internal order goes from cheapest to most expensive, so that a failure
# costs seconds rather than the whole chain: bounded parallelism (a grep,
# under a second), video fixtures (two calls to ffprobe, under a second too),
# encoding detection score (the core and a forty-line program), manual
# examples (the CLI binary only), requirements (incremental dev build),
# end-to-end tests (release build), installation into a temporary prefix —
# which shares that release tree and so does not rebuild it —, then
# benchmarks. `parallelism` comes first precisely because it builds nothing —
# making it wait behind `requirements`, which compiles, would cost a
# hard-coded `-j 8` the time of a whole build before it was heard.
#
# `manual-check` is here and not in `check`, which has a consequence worth
# knowing: **the CI will not see a stale manual.** Putting it there would bring
# `docs/manual/**` into the scope of the gate, from which `docs/**` is
# excluded, and a touch of prose would reopen fifteen minutes of compilation.
# `bench` has no binary verdict — that is intended, the project's rule requires
# replaying the benchmarks for every issue, and chaining them here is what
# guarantees it rather than relying on the memory of whoever opens the pull
# request. It therefore stays chained: what changed is what precedes it, not
# it.
.PHONY: check-local
check-local: ## The one local command to run before a pull request
	@./src/scripts/gate.sh check-local

.PHONY: verify-gates
verify-gates: ## Prove that each gate closes on its defect
	@./src/scripts/verify-gates.sh

# The durations of the last runs of each gate step, phase and proof, side by side, read from the
# local journal that `check`, `check-local` and `verify-gates` append to — build/gate-timings.tsv,
# which `make clean` erases. A latest run above one and a half times the median of the previous
# ones is flagged, with the load and the JOBS it ran under. `RUNS=N` shows N runs, `SUITE=check`
# one suite.
.PHONY: timings
timings: ## Compare the last durations of each gate step and proof
	@./src/scripts/timings.sh $(RUNS) $(SUITE)

.PHONY: changelog
changelog: ## Regenerate CHANGELOG.md from the commit history
	$(call require,git-cliff)
	$(call step,"changelog")
	@git-cliff --output CHANGELOG.md

.PHONY: clean
clean: ## Remove the build directories
	@rm -rf build
	@printf 'build/ removed\n'
