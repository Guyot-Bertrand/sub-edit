#!/usr/bin/env bash
# Refuses that a run of the tests touches the configuration of whoever runs them.
#
# **The counterpart of check-untracked.sh, on the other side of the border.**
# That one records the files `git ls-files` does not track, so it only sees the
# working tree: a file written in the home directory is invisible to it, though
# it is exactly the same defect — a test that writes where it has no business
# writing. Phase 7 brings persisted preferences, and with them the first chance
# to commit it.
#
# **Two kinds of damage, and the second is the more insidious.** The test
# destroys its author's settings, which shows. And it comes to depend on what
# that author already had: a test that passes for someone who never ran the
# binary fails for someone who ran it once, or the other way round.
#
# **What is watched is, by name, ours**, and not the whole configuration
# directory. A gate runs for fifteen minutes, during which the machine's
# browser, editor and window manager write into `~/.config` without owing us
# anything. A check that reported them would cry wolf at every run, and a check
# that cries wolf ends up disabled.
#
# **What is refused is a change, not a presence.** Whoever develops subedit ends
# up running subedit, and their configuration exists. The check therefore
# compares two fingerprints, before and after, and speaks only of the
# difference — appearance, disappearance or change.
#
# It honours `XDG_CONFIG_HOME` for the same reason the tests move it: it is the
# variable every standard location derives from. It is also what lets
# verify-gates.sh prove that the gate closes without writing once into the real
# home directory.

set -euo pipefail

readonly REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"

readonly CONFIG_HOME="${XDG_CONFIG_HOME:-${HOME}/.config}"
# **Two directories, and the second belongs to another program.** Enchant keeps
# the personal word list in `enchant/`, shared by everything that uses it —
# Gaupol's included, D6 of docs/specs/12-correction.md. A test that added a word
# to a real spell checker's dictionary would write into the user's home, and
# into other software than ours: they are watched together, and the tests that
# touch Enchant move `ENCHANT_CONFIG_DIR`.
#
# **A third, since #566: `$XDG_DATA_HOME/subedit`**, where the user puts their
# correction patterns (ADR 0037). The command line *reads* them — which the
# check does not see — and a test that wrote a test pattern there would have it
# read by every later run, its own included.
readonly DATA_HOME="${XDG_DATA_HOME:-${HOME}/.local/share}"
readonly WATCHED_DIRECTORIES=("${CONFIG_HOME}/subedit" "${CONFIG_HOME}/enchant" "${DATA_HOME}/subedit")

# Under `build/`, which git ignores — like the snapshot of check-untracked.sh,
# and for the same reason: a tracked snapshot would be one more file not to
# commit.
readonly SNAPSHOT="${REPO_ROOT}/build/configuration-utilisateur.txt"

readonly RED=$'\033[31m'
readonly GREEN=$'\033[32m'
readonly RESET=$'\033[0m'

usage() {
    cat >&2 <<'USAGE'
usage: check-config-home.sh --record | --compare

  --record   records the state of the user's configuration, before the tests
  --compare  fails if the tests touched it
USAGE
    exit 2
}

# Path, size and modification time of each file, or the word "absent".
#
# The time as well as the size: a value replaced by another of the same length —
# a "dark" theme turned "none" — would change nothing to the latter. And the
# word "absent" rather than an empty snapshot, so that the most common case, a
# directory that does not exist, is told apart from a directory that is present
# and empty.
fingerprint() {
    local directory
    for directory in "${WATCHED_DIRECTORIES[@]}"; do
        if [[ ! -e "${directory}" ]]; then
            printf 'absent %s\n' "${directory}"
            continue
        fi

        find "${directory}" -printf '%y %s %T@ %p\n' | LC_ALL=C sort
    done
}

record() {
    mkdir -p "$(dirname "${SNAPSHOT}")"
    fingerprint > "${SNAPSHOT}"
}

compare() {
    # A missing snapshot is not an untouched configuration: it is a check that
    # did not take place. Say so, rather than going green without having
    # compared anything.
    if [[ ! -f "${SNAPSHOT}" ]]; then
        printf '%s✗ no prior snapshot: run --record before --compare%s\n' \
            "${RED}" "${RESET}" >&2
        exit 1
    fi

    local difference
    difference="$(diff "${SNAPSHOT}" <(fingerprint) || true)"

    if [[ -n "${difference}" ]]; then
        printf '%s✗ the tests touched the user'\''s configuration:%s\n' \
            "${RED}" "${RESET}" >&2
        printf '    %s\n' "${WATCHED_DIRECTORIES[@]}" >&2
        printf '%s\n' "${difference}" >&2
        printf '  a test never resolves a configuration location: it receives\n' >&2
        printf '  one, or goes through the harness that moves XDG_CONFIG_HOME\n' >&2
        exit 1
    fi

    printf '%s✓%s the user'\''s configuration is untouched\n' "${GREEN}" "${RESET}"
}

case "${1:-}" in
    --record)  record ;;
    --compare) compare ;;
    *)         usage ;;
esac
