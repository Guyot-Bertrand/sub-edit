#!/usr/bin/env bash
# Lock of the reference repository.
#
# reference/gaupol is a clone of Gaupol, present only to be read. This script
# keeps it read-only, and opens writing only for the duration of a fetch.
#
# It is a safeguard against mistakes, not a security barrier: the owner of the
# files can always undo a chmod. That is precisely the point — making an
# accidental modification impossible without making maintenance painful.

set -euo pipefail

readonly REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
readonly REFERENCE="${REPO_ROOT}/reference/gaupol"

die() { printf 'error: %s\n' "$*" >&2; exit 1; }
info() { printf '%s\n' "$*"; }

require_reference() {
    [[ -d "${REFERENCE}/.git" ]] \
        || die "clone missing: ${REFERENCE}
  fetch it with: git clone https://github.com/otsaloma/gaupol.git '${REFERENCE}'"
}

is_locked() {
    # The clone is locked if its root is not writable.
    [[ ! -w "${REFERENCE}" ]]
}

lock() {
    require_reference
    if ! is_locked && ! git -C "${REFERENCE}" diff --quiet HEAD 2>/dev/null; then
        info "warning: the clone contains local modifications"
        git -C "${REFERENCE}" status --short
        info "restore them with: $0 restore"
    fi
    chmod -R a-w "${REFERENCE}"
    info "locked: ${REFERENCE}"
}

unlock() {
    require_reference
    chmod -R u+w "${REFERENCE}"
    info "unlocked: ${REFERENCE}"
}

# Opens writing for the duration of the fetch, closes it whatever happens.
fetch() {
    require_reference
    local was_locked=0
    is_locked && was_locked=1
    if (( was_locked )); then
        chmod -R u+w "${REFERENCE}"
        trap 'chmod -R a-w "${REFERENCE}"' EXIT
    fi
    git -C "${REFERENCE}" fetch --tags origin
    info "--- upstream changes ---"
    git -C "${REFERENCE}" log --oneline HEAD..FETCH_HEAD | head -30
    info "the working copy is not updated; to move it:"
    info "  $0 unlock && git -C '${REFERENCE}' merge --ff-only FETCH_HEAD && $0 lock"
}

# Restores the clone to the state of its HEAD, after an accidental modification.
restore() {
    require_reference
    chmod -R u+w "${REFERENCE}"
    git -C "${REFERENCE}" checkout -- .
    git -C "${REFERENCE}" clean -fd
    chmod -R a-w "${REFERENCE}"
    info "restored and locked: ${REFERENCE}"
}

status() {
    require_reference
    if is_locked; then
        info "state   : locked (read-only)"
    else
        info "state   : UNLOCKED — lock it again with: $0 lock"
    fi
    info "revision: $(git -C "${REFERENCE}" log -1 --format='%h %ad %s' --date=short)"
    local dirty
    dirty="$(git -C "${REFERENCE}" status --short)"
    if [[ -n "${dirty}" ]]; then
        info "MODIFIED:"
        printf '%s\n' "${dirty}"
        info "restore it with: $0 restore"
    else
        info "content : matches HEAD"
    fi
}

usage() {
    cat <<EOF
usage: $(basename "$0") <command>

  lock      makes the clone read-only
  unlock    reopens writing (to be used only deliberately)
  fetch     fetches upstream changes, lock restored automatically
  restore   restores the clone to the state of its HEAD, then locks
  status    shows the lock state, the revision and any differences
EOF
}

case "${1:-}" in
    lock)    lock ;;
    unlock)  unlock ;;
    fetch)   fetch ;;
    restore) restore ;;
    status)  status ;;
    *)       usage; exit 1 ;;
esac
