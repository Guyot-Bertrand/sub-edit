#!/usr/bin/env bash
# Les tests de bout en bout seuls, en Release.
#
# Filtrés par l'étiquette CTest `e2e` et non par nom : un test unitaire dont le
# nom s'en approcherait ne tromperait pas le filtre. Partage l'arbre release
# avec `bench.sh` et `release.sh`, qui ne le reconstruisent donc pas.
set -euo pipefail
# shellcheck source=src/scripts/gate/common.sh
source "$(dirname "${BASH_SOURCE[0]}")/common.sh"
step "tests de bout en bout (release)"
cmake --preset release -DSUBEDIT_LTO_JOBS="${JOBS}"
cmake --build --preset release -j "${JOBS}" --target subedit_e2e_test
ctest --preset release -L e2e
