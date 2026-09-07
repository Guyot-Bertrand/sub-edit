#!/usr/bin/env bash
# Le score de la détection de format, rejoué et confronté à son relevé.
#
# **Le même geste que `score.sh`, sur l'autre inconnue de la phase 9.** Avec
# deux formats la question était fermée d'avance ; avec neuf, `.sub` et `.txt`
# désignent chacune deux formats et une détection peut se tromper. Ce qu'on
# mesure ici est donc un classifieur, exactement comme #290 l'a établi pour
# l'encodage.
#
# **Le corpus privé n'est jamais lu ici**, comme pour les deux autres relevés :
# absent d'une machine sur deux, il ferait dire deux choses à la même porte.
#
# Ce que l'étape construit est la détection du projet sur la ligne de commande,
# et rien d'autre : le noyau et un programme de cinquante lignes.

set -euo pipefail

# shellcheck source=src/scripts/gate/common.sh
source "$(dirname "${BASH_SOURCE[0]}")/common.sh"

step "score de détection de format"

cmake --preset dev >/dev/null
cmake --build --preset dev -j "${JOBS}" --target subedit_detect_format

"${REPO_ROOT}/src/scripts/score-format-detection.py" \
    --detector "${REPO_ROOT}/build/dev/bin/subedit_detect_format {}" \
    --journal
