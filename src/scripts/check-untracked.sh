#!/usr/bin/env bash
# Refuses that a run of the gate leaves files behind.
#
# **The defect has been paid for once.** The end-to-end tests of #207 gave a
# bare name to `--output`, so the file landed where CTest runs them — the root
# of the repository. Four files stayed there, `make check` went green, and it
# was a `git status` before pushing that saw them. The harness already had
# `Scratch`, which every other test that writes uses.
#
# **What is refused is an appearance, not a presence.** An untracked file is the
# ordinary case of work in progress: a new source before its `git add`, a note
# on the side. Refusing their existence would make the gate unusable. The check
# therefore compares two snapshots, before and after, and speaks only of the
# difference.
#
# `git ls-files --others --exclude-standard` rather than a filtered
# `git status --porcelain`: it gives exactly the list wanted, with no status
# code to parse, and it honours the same exclusions. That is what puts
# **`src/data/` out of reach** — each machine's private corpus is ignored by
# git, so it appears in neither snapshot. Checked rather than assumed, and
# `.gitignore` says so, along with `build/` and `reference/`.

set -euo pipefail

readonly REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"

# Under `build/`, which git ignores: the snapshot therefore cannot report
# itself in the next snapshot.
readonly SNAPSHOT="${REPO_ROOT}/build/fichiers-non-suivis.txt"

readonly RED=$'\033[31m'
readonly GREEN=$'\033[32m'
readonly RESET=$'\033[0m'

usage() {
    cat >&2 <<'USAGE'
usage: check-untracked.sh --record | --compare

  --record   records the untracked files, before what is to be watched
  --compare  fails on any untracked file that appeared since the record
USAGE
    exit 2
}

untracked() {
    git -C "${REPO_ROOT}" ls-files --others --exclude-standard | sort
}

record() {
    mkdir -p "$(dirname "${SNAPSHOT}")"
    untracked > "${SNAPSHOT}"
}

compare() {
    # A missing snapshot is not a clean tree: it is a check that did not take
    # place. Say so, rather than going green without having compared anything —
    # a check silently skipped is worse than no check, since it stays green
    # without proving anything.
    if [[ ! -f "${SNAPSHOT}" ]]; then
        printf '%s✗ no prior snapshot: run --record before --compare%s\n' \
            "${RED}" "${RESET}" >&2
        exit 1
    fi

    local appeared
    appeared="$(comm -13 "${SNAPSHOT}" <(untracked))"

    if [[ -n "${appeared}" ]]; then
        printf '%s✗ untracked files appeared during the tests:%s\n' \
            "${RED}" "${RESET}" >&2
        printf '    %s\n' ${appeared} >&2
        printf '  a test that writes goes through the "Scratch" harness, never through a bare name\n' >&2
        exit 1
    fi

    printf '%s✓%s no file left behind\n' "${GREEN}" "${RESET}"
}

case "${1:-}" in
    --record)  record ;;
    --compare) compare ;;
    *)         usage ;;
esac
