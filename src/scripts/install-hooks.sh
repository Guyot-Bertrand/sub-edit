#!/usr/bin/env bash
# Active les hooks git versionnés du projet.
#
# Plutôt que copier des scripts dans .git/hooks — où ils divergeraient
# silencieusement — on pointe git sur le répertoire versionné. Une correction
# de hook profite ainsi à tous les clones au prochain pull.

set -euo pipefail

readonly REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
readonly HOOKS_DIR="src/scripts/hooks"

chmod +x "${REPO_ROOT}/${HOOKS_DIR}"/*
git -C "${REPO_ROOT}" config core.hooksPath "${HOOKS_DIR}"

printf 'hooks git actifs : %s\n' "${HOOKS_DIR}"
printf '  pre-commit  format du code, refus du dépôt de référence\n'
printf '  commit-msg  grammaire Conventional Commits\n'
