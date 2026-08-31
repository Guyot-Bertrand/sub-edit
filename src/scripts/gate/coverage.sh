#!/usr/bin/env bash
# La couverture des bibliothèques, mesurée et confrontée au cliquet.
#
# Six gestes, et deux d'entre eux existent contre un défaut précis :
#
#   * `clean-stale-coverage.sh` écarte un arbre où traîne un `.gcno` que plus
#     aucune source ne rattache — le résidu d'un fichier déplacé, sur lequel
#     gcovr échoue en nommant ni le fautif ni le remède ;
#   * les `.gcda` sont effacés après la compilation et avant les tests. Ceux
#     d'une exécution antérieure survivent à une recompilation et gcov les
#     fusionne : un fichier modifié depuis donnerait un taux faux, sans autre
#     signe qu'un avertissement noyé dans la sortie.
#
# Les tests restent séquentiels : `ctest` ne parallélise que sur `-j` explicite,
# et le lui donner ferait fusionner les `.gcda` de plusieurs exécutions
# concurrentes.

set -euo pipefail

# shellcheck source=src/scripts/gate/common.sh
source "$(dirname "${BASH_SOURCE[0]}")/common.sh"

require gcovr

readonly REPORT="${REPO_ROOT}/build/coverage-report"

step "couverture"

"${REPO_ROOT}/src/scripts/clean-stale-coverage.sh"

cmake --preset coverage
cmake --build --preset coverage -j "${JOBS}"

find "${REPO_ROOT}/build/coverage" -name '*.gcda' -delete

ctest --preset coverage

mkdir -p "${REPORT}"
gcovr --root "${REPO_ROOT}" "${REPO_ROOT}/build/coverage/src" \
    --filter 'src/lib/' \
    --exclude-unreachable-branches --exclude-throw-branches \
    --print-summary \
    --json-summary "${REPORT}/summary.json" \
    --html-details "${REPORT}/index.html" \
    --txt "${REPORT}/summary.txt"

printf 'rapport : build/coverage-report/index.html\n'

"${REPO_ROOT}/src/scripts/check-coverage.sh" --summary "${REPORT}/summary.json"
