#!/usr/bin/env bash
# Le score de la détection d'encodage, rejoué et confronté à son relevé.
#
# **Il existait, et personne ne s'en servait deux fois** — issue #311.
# `score-encoding-detection.py` date de #290, et le 9/9 du journal avait été
# relevé à la main à la version 0.8.16 : rien ne le rejouait, donc il
# vieillissait sans que personne l'apprenne. C'est la forme la plus discrète du
# défaut que la relecture de la phase 7 a inscrit — là, une vérification passait
# par un outil qui n'était pas celui qui comptait ; ici l'outil est le bon.
#
# **Le corpus privé n'est jamais lu ici.** Il est absent de toute machine qui ne
# l'a pas, donc une porte qui le lirait dirait deux choses différentes selon le
# poste. `--prive` reste un geste à la main.
#
# Ce que l'étape construit est la détection du projet sur la ligne de commande,
# et rien d'autre : le noyau et un programme de quarante lignes.

set -euo pipefail

# shellcheck source=src/scripts/gate/common.sh
source "$(dirname "${BASH_SOURCE[0]}")/common.sh"

step "score de détection d encodage"

cmake --preset dev >/dev/null
cmake --build --preset dev -j "${JOBS}" --target subedit_detect_encoding

"${REPO_ROOT}/src/scripts/score-encoding-detection.py" \
    --detector "${REPO_ROOT}/build/dev/bin/subedit_detect_encoding {}" \
    --journal
