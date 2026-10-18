#!/usr/bin/env bash
# Validates a commit message against the Conventional Commits grammar.
#
# Used by the commit-msg hook locally, by CI on the pushed history, and on pull
# request titles. Argument: a file holding the message, or the message itself.
# Messages are written in English (ADR 0043); the grammar does not check that.

set -euo pipefail

readonly TYPES='feat|fix|perf|refactor|test|docs|build|ci|chore|revert'
readonly SCOPES='build|ci|cli|core|doc|format|gui|i18n|scripts|test|text|video'
readonly SUBJECT_MAX_LENGTH=72

# shellcheck source=src/scripts/lib/output.sh
source "$(dirname "${BASH_SOURCE[0]}")/lib/output.sh"

input="${1:?usage: $(basename "$0") <file|message>}"
if [[ -f "${input}" ]]; then
    message="$(cat "${input}")"
else
    message="${input}"
fi

# Git's comment lines are not part of the message.
subject="$(printf '%s\n' "${message}" | grep -v '^#' | sed '/^[[:space:]]*$/d' | head -1)"

reject() {
    printf '%sinvalid commit message%s\n\n' "${RED}" "${RESET}" >&2
    printf '  %s\n\n' "${subject}" >&2
    printf '%s\n' "$*" >&2
    cat >&2 <<EOF

Expected grammar:

  <type>(<scope>): <description>

  types  : ${TYPES//|/, }
  scopes : ${SCOPES//|/, }   (optional)

Examples:

  feat(core): read SubRip files
  fix(cli): exit with a non-zero status on a partial failure
  docs: roadmap of the eight phases
EOF
    exit 1
}

# Merge commits are written by git, not by a person.
[[ "${subject}" =~ ^Merge\  ]] && exit 0

[[ -n "${subject}" ]] || reject "the message is empty."

[[ "${subject}" =~ ^(${TYPES})(\((${SCOPES})\))?!?:\ .+ ]] \
    || reject "the header does not follow the grammar, or the scope is not in the list."

(( ${#subject} <= SUBJECT_MAX_LENGTH )) \
    || reject "the header is ${#subject} characters long, at most ${SUBJECT_MAX_LENGTH}."

[[ ! "${subject}" =~ \.$ ]] \
    || reject "the header does not end with a period."
