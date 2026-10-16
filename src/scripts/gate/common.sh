#!/usr/bin/env bash
# The base of the gate steps: what each of them would otherwise repeat.
#
# **This file is sourced, not executed.** It is executable nonetheless, because
# `check-architecture.sh` requires every file of `src/scripts` to be — a script
# committed as 100644 fails in CI after a fresh clone. Run directly, it says
# what it is and stops.
#
# What it brings, and nothing else:
#
#   REPO_ROOT   the repository root, whatever the current directory
#   JOBS        the parallelism ceiling, inherited from the environment
#   step        announces the current step
#   require     fails with a useful message rather than a « command not found »
#
# **`JOBS` comes from the environment and is never guessed here.** The
# `Makefile` exports it, CI sets it to `nproc`, and a direct call can set it
# too. The default of two is written once, in the `Makefile`, with the
# measurement that justifies it; repeating it here would make two truths.

if [[ "${BASH_SOURCE[0]}" == "${0}" ]]; then
    printf 'this file is sourced, not executed: source %s\n' "${BASH_SOURCE[0]}" >&2
    exit 2
fi

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../../.." && pwd)"
readonly REPO_ROOT

JOBS="${JOBS:-2}"
readonly JOBS

readonly BOLD=$'\033[1m'
readonly GREEN=$'\033[32m'
readonly RED=$'\033[31m'
# Yellow came with #270: a step that succeeds without having been able to do
# its work is neither green nor red. Benchmarks on a busy machine are the only
# case to date.
readonly YELLOW=$'\033[33m'
readonly RESET=$'\033[0m'

step() {
    printf '%s▸ %s%s\n' "${BOLD}" "$1" "${RESET}"
}

require() {
    command -v "$1" >/dev/null 2>&1 && return 0
    printf '%smissing tool: %s%s\n' "${RED}" "$1" "${RESET}" >&2
    printf '  install it with: ./src/scripts/setup-toolchain.sh\n' >&2
    exit 1
}

# The preferred generator, without requiring it: fixing it in the presets would
# make the project unbuildable on a machine that does not have Ninja yet. Set
# here rather than in each step, and only if it is not set already — a caller
# who has chosen keeps their choice.
if [[ -z "${CMAKE_GENERATOR:-}" ]]; then
    if command -v ninja >/dev/null 2>&1; then
        export CMAKE_GENERATOR="Ninja"
    else
        export CMAKE_GENERATOR="Unix Makefiles"
    fi
fi
