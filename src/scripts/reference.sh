#!/usr/bin/env bash
# Verrou du dépôt de référence.
#
# reference/gaupol est un clone de Gaupol, présent uniquement pour être lu.
# Ce script le maintient non inscriptible, et n'ouvre l'écriture que le temps
# d'un fetch.
#
# Il s'agit d'un garde-fou contre l'erreur, pas d'une barrière de sécurité :
# le propriétaire des fichiers peut toujours défaire un chmod. C'est
# précisément le but — rendre la modification accidentelle impossible sans
# rendre la maintenance pénible.

set -euo pipefail

readonly REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
readonly REFERENCE="${REPO_ROOT}/reference/gaupol"

die() { printf 'erreur : %s\n' "$*" >&2; exit 1; }
info() { printf '%s\n' "$*"; }

require_reference() {
    [[ -d "${REFERENCE}/.git" ]] \
        || die "clone absent : ${REFERENCE}
  le récupérer avec : git clone https://github.com/otsaloma/gaupol.git '${REFERENCE}'"
}

is_locked() {
    # Le clone est verrouillé si sa racine n'est pas inscriptible.
    [[ ! -w "${REFERENCE}" ]]
}

lock() {
    require_reference
    if ! is_locked && ! git -C "${REFERENCE}" diff --quiet HEAD 2>/dev/null; then
        info "attention : le clone contient des modifications locales"
        git -C "${REFERENCE}" status --short
        info "les restaurer avec : $0 restore"
    fi
    chmod -R a-w "${REFERENCE}"
    info "verrouillé : ${REFERENCE}"
}

unlock() {
    require_reference
    chmod -R u+w "${REFERENCE}"
    info "déverrouillé : ${REFERENCE}"
}

# Ouvre l'écriture le temps du fetch, referme quoi qu'il arrive.
fetch() {
    require_reference
    local was_locked=0
    is_locked && was_locked=1
    if (( was_locked )); then
        chmod -R u+w "${REFERENCE}"
        trap 'chmod -R a-w "${REFERENCE}"' EXIT
    fi
    git -C "${REFERENCE}" fetch --tags origin
    info "--- nouveautés amont ---"
    git -C "${REFERENCE}" log --oneline HEAD..FETCH_HEAD | head -30
    info "la copie de travail n'est pas mise à jour ; pour la déplacer :"
    info "  $0 unlock && git -C '${REFERENCE}' merge --ff-only FETCH_HEAD && $0 lock"
}

# Rétablit le clone dans l'état de son HEAD, après une modification accidentelle.
restore() {
    require_reference
    chmod -R u+w "${REFERENCE}"
    git -C "${REFERENCE}" checkout -- .
    git -C "${REFERENCE}" clean -fd
    chmod -R a-w "${REFERENCE}"
    info "restauré et verrouillé : ${REFERENCE}"
}

status() {
    require_reference
    if is_locked; then
        info "état    : verrouillé (lecture seule)"
    else
        info "état    : DÉVERROUILLÉ — le relancer avec : $0 lock"
    fi
    info "révision: $(git -C "${REFERENCE}" log -1 --format='%h %ad %s' --date=short)"
    local dirty
    dirty="$(git -C "${REFERENCE}" status --short)"
    if [[ -n "${dirty}" ]]; then
        info "MODIFIÉ :"
        printf '%s\n' "${dirty}"
        info "le restaurer avec : $0 restore"
    else
        info "contenu : conforme à HEAD"
    fi
}

usage() {
    cat <<EOF
usage : $(basename "$0") <commande>

  lock      rend le clone non inscriptible
  unlock    rouvre l'écriture (à n'utiliser que délibérément)
  fetch     récupère les évolutions amont, verrou rétabli automatiquement
  restore   rétablit le clone dans l'état de son HEAD, puis verrouille
  status    affiche l'état du verrou, la révision et les écarts
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
