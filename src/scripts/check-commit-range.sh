#!/usr/bin/env bash
# Valide la grammaire des messages de commit d'une plage.
#
# Appelé par la CI, où la plage à vérifier dépend de l'événement GitHub. Isolé
# ici plutôt qu'écrit dans le YAML : un script se teste en local, pas une étape
# de workflow.

set -euo pipefail

readonly REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && cd .. && pwd)"
readonly VALIDATOR="${REPO_ROOT}/src/scripts/check-commit-message.sh"
readonly EMPTY_SHA="0000000000000000000000000000000000000000"

EVENT="${EVENT:-}"
BASE_SHA="${BASE_SHA:-}"
HEAD_SHA="${HEAD_SHA:-}"
BEFORE_SHA="${BEFORE_SHA:-}"
AFTER_SHA="${AFTER_SHA:-HEAD}"

resolve_range() {
    if [[ "${EVENT}" == "pull_request" && -n "${BASE_SHA}" && -n "${HEAD_SHA}" ]]; then
        printf '%s..%s' "${BASE_SHA}" "${HEAD_SHA}"
        return
    fi
    # Première poussée sur une branche : pas de plage, on valide la tête seule.
    if [[ -z "${BEFORE_SHA}" || "${BEFORE_SHA}" == "${EMPTY_SHA}" ]]; then
        printf '%s~1..%s' "${AFTER_SHA}" "${AFTER_SHA}"
        return
    fi
    printf '%s..%s' "${BEFORE_SHA}" "${AFTER_SHA}"
}

range="$(resolve_range)"
printf 'plage vérifiée : %s\n\n' "${range}"

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
