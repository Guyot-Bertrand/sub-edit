#!/usr/bin/env bash
# Ce qu'une conversion perd, rejoué et confronté à son relevé.
#
# **Le même geste que `score.sh`, sur l'autre inconnue de la phase.** Une
# détection a un taux ; une conversion a une perte. Ni l'une ni l'autre ne se
# regarde une fois : ce qui compte est la comparaison à la mesure précédente, et
# une mesure qu'on ne rejoue pas vieillit sans que personne l'apprenne — c'est
# ce que l'issue #311 a constaté du score, qui avait dormi une phase entière.
#
# **Le corpus privé n'est jamais lu ici**, comme pour le score : absent d'une
# machine sur deux, il ferait dire deux choses à la même porte.
#
# Ce que l'étape construit est la ligne de commande elle-même. La mesure porte
# donc sur ce que l'utilisateur obtient, et non sur un chemin de test.

set -euo pipefail

# shellcheck source=src/scripts/gate/common.sh
source "$(dirname "${BASH_SOURCE[0]}")/common.sh"

step "perte de conversion"

cmake --preset dev >/dev/null
cmake --build --preset dev -j "${JOBS}" --target subedit-cli

"${REPO_ROOT}/src/scripts/measure-conversion-loss.py" \
    --binary "${REPO_ROOT}/build/dev/bin/subedit-cli" \
    --journal
