#!/usr/bin/env bash
# Les deux binaires optimisés, et rien d'autre.
#
# Ni le banc ni le harnais de bout en bout : ils ont leurs propres étapes. Ce
# qui est demandé ici est ce qu'un utilisateur lance, pas ce qui l'éprouve.
#
# **`JOBS` gouverne le LTO**, pour la raison écrite sur `bench.sh` : sans lui,
# l'optimisation entre modules démarre autant de processus que de cœurs à chaque
# édition de liens. C'est ce qui rend cette étape préférable aux deux commandes
# `cmake` écrites à la main, qui n'en savent rien.

set -euo pipefail

# shellcheck source=src/scripts/gate/common.sh
source "$(dirname "${BASH_SOURCE[0]}")/common.sh"

step "binaires (release)"

cmake --preset release -DSUBEDIT_LTO_JOBS="${JOBS}"
cmake --build --preset release -j "${JOBS}" --target subedit-cli subedit-gui

printf '  binaires : build/release/bin/subedit-cli, build/release/bin/subedit-gui\n'
