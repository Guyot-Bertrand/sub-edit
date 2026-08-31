#!/usr/bin/env bash
# L'analyse statique, une propriété de la compilation — issue #269.
#
# **Rien à choisir ici, et c'est tout le propos.** Le périmètre n'est pas
# calculé : clang-tidy est accroché à la règle de compilation de chaque source,
# donc le système de construction rejoue exactement ce dont une entrée a changé
# — la source, un en-tête de son fichier de dépendances, ou sa ligne de
# commande. Voir `cmake/Tidy.cmake` pour ce que cela remplace et pourquoi.
#
# Pour tout réanalyser : `rm -rf build/tidy`.
set -euo pipefail
# shellcheck source=src/scripts/gate/common.sh
source "$(dirname "${BASH_SOURCE[0]}")/common.sh"
step "analyse statique"
cmake --preset tidy
cmake --build --preset tidy -j "${JOBS}"
