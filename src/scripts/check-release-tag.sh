#!/usr/bin/env bash
# Refuses to publish a tag that does not name a version of `main` — issue #232.
#
#   check-release-tag.sh TAG
#
# Three conditions, and a release is built only once all three hold:
#
#   form       the tag is written `vX.Y.Z`;
#   version    the `project(VERSION)` of the tagged commit carries the same
#              number — invariant 4 of `check-architecture.sh`, which runs
#              only locally and which nothing replayed at publishing time;
#   branch     the tagged commit belongs to `main`. A tag set on a working
#              branch would publish a state that nobody merged.
#
# Read from the commit and not from the working tree, for the reason of
# invariant 4: a tag names a commit.
#
# It needs the full history and `origin/main`: the workflow's checkout asks for
# them with `fetch-depth: 0`.

set -euo pipefail

readonly REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
# shellcheck source=src/scripts/lib/project.sh
source "$(dirname "${BASH_SOURCE[0]}")/lib/project.sh"

refuse() {
    printf 'tag refused: %s\n' "$1" >&2
    exit 1
}

[[ $# -eq 1 ]] || { printf 'usage: check-release-tag.sh TAG\n' >&2; exit 2; }
readonly TAG="$1"

[[ "${TAG}" =~ ^v[0-9]+\.[0-9]+\.[0-9]+$ ]] \
    || refuse "\"${TAG}\" is not written vX.Y.Z"

commit="$(git -C "${REPO_ROOT}" rev-parse --verify --quiet "${TAG}^{commit}")" \
    || refuse "${TAG} names no commit"

declared="$(git -C "${REPO_ROOT}" show "${commit}:CMakeLists.txt" 2>/dev/null \
    | project_version)"

[[ "v${declared}" == "${TAG}" ]] \
    || refuse "${TAG} and CMakeLists.txt (${declared:-absent}) disagree"

git -C "${REPO_ROOT}" merge-base --is-ancestor "${commit}" origin/main 2>/dev/null \
    || refuse "${TAG} names a commit that is not on main"

printf '%s: version %s, on main\n' "${TAG}" "${declared}"
