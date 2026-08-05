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

narrowed=0
range=""

# Affecte les variables globales plutôt que d'écrire sur la sortie : appelée
# par substitution de commande, la fonction s'exécuterait dans un sous-shell et
# l'indicateur de réduction serait perdu.
resolve_range() {
    if [[ "${EVENT}" == "pull_request" && -n "${BASE_SHA}" && -n "${HEAD_SHA}" ]]; then
        range="${BASE_SHA}..${HEAD_SHA}"
        return
    fi
    # Première poussée d'une branche : GitHub ne fournit pas de base, on se
    # rabat sur la tête seule. C'est une réduction réelle de la couverture de
    # la vérification — elle est signalée plutôt que subie en silence.
    if [[ -z "${BEFORE_SHA}" || "${BEFORE_SHA}" == "${EMPTY_SHA}" ]]; then
        narrowed=1
        range="${AFTER_SHA}~1..${AFTER_SHA}"
        return
    fi
    range="${BEFORE_SHA}..${AFTER_SHA}"
}

resolve_range
printf 'plage vérifiée : %s\n' "${range}"
(( narrowed == 0 )) || printf \
    'attention : première poussée de cette branche, seul le dernier commit est vérifié\n'
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
