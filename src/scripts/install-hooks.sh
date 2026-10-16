#!/usr/bin/env bash
# Activates the project's versioned git hooks.
#
# Rather than copying scripts into .git/hooks — where they would silently
# diverge — git is pointed at the versioned directory. A fix to a hook thus
# reaches every clone at the next pull.

set -euo pipefail

readonly REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
readonly HOOKS_DIR="src/scripts/hooks"

chmod +x "${REPO_ROOT}/${HOOKS_DIR}"/*
git -C "${REPO_ROOT}" config core.hooksPath "${HOOKS_DIR}"

printf 'active git hooks: %s\n' "${HOOKS_DIR}"
printf '  pre-commit  code format, refusal of the reference repository\n'
printf '  commit-msg  Conventional Commits grammar\n'
