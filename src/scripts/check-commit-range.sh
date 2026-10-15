#!/usr/bin/env bash
# Validates the grammar of the commit messages of a range.
#
# Called by the CI, where the range to check depends on the GitHub event. Kept
# here rather than written in the YAML: a script can be tested locally, a
# workflow step cannot.

set -euo pipefail

readonly REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && cd .. && pwd)"
readonly VALIDATOR="${REPO_ROOT}/src/scripts/check-commit-message.sh"
readonly EMPTY_SHA="0000000000000000000000000000000000000000"

EVENT="${EVENT:-}"
BASE_SHA="${BASE_SHA:-}"
HEAD_SHA="${HEAD_SHA:-}"
BEFORE_SHA="${BEFORE_SHA:-}"
AFTER_SHA="${AFTER_SHA:-HEAD}"

narrowed=0
range=""

# Sets the global variables rather than writing to the output: called through
# command substitution, the function would run in a subshell and the narrowing
# flag would be lost.
resolve_range() {
    if [[ "${EVENT}" == "pull_request" && -n "${BASE_SHA}" && -n "${HEAD_SHA}" ]]; then
        range="${BASE_SHA}..${HEAD_SHA}"
        return
    fi
    # First push of a branch: GitHub gives no base, so we fall back on the
    # head alone. That is a real narrowing of what the check covers — it is
    # reported rather than suffered in silence.
    if [[ -z "${BEFORE_SHA}" || "${BEFORE_SHA}" == "${EMPTY_SHA}" ]]; then
        narrowed=1
        range="${AFTER_SHA}~1..${AFTER_SHA}"
        return
    fi
    range="${BEFORE_SHA}..${AFTER_SHA}"
}

resolve_range
printf 'range checked: %s\n' "${range}"
(( narrowed == 0 )) || printf \
    'warning: first push of this branch, only the last commit is checked\n'
printf '\n'

failures=0
while IFS= read -r sha; do
    [[ -n "${sha}" ]] || continue
    subject="$(git log -1 --format=%s "${sha}")"
    if "${VALIDATOR}" "${subject}" >/dev/null 2>&1; then
        printf '  ✓ %s %s\n' "${sha:0:8}" "${subject}"
    else
        printf '  ✗ %s %s\n' "${sha:0:8}" "${subject}"
        "${VALIDATOR}" "${subject}" || true
        failures=$((failures + 1))
    fi
done < <(git rev-list --no-merges "${range}" 2>/dev/null || true)

(( failures == 0 )) || exit 1
