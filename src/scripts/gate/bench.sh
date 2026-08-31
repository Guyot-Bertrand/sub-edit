#!/usr/bin/env bash
# Les benchmarks, en Release, et le relevé versé au journal.
#
# **`JOBS` gouverne aussi l'optimisation entre modules.** Sans lui, le LTO part
# en `-flto=auto`, c'est-à-dire autant de processus que de cœurs, à chaque
# édition de liens — un parallélisme qui n'apparaît dans aucun `-j` et qui
# sature une machine sur laquelle on fait autre chose.
#
# La charge est relevée avant de mesurer, et le relevé la porte : une mesure
# prise pendant qu'un navigateur compile du JavaScript ne dit rien du code, elle
# dit l'état de la machine. `await-quiet.sh` attend ce qui vient et renonce sur
# ce qui ne vient pas ; `record-bench.sh` décide ce qu'il fait d'un relevé pris
# au-dessus du seuil — il l'inscrit, sans le laisser fixer d'extrême.

set -euo pipefail

# shellcheck source=src/scripts/gate/common.sh
source "$(dirname "${BASH_SOURCE[0]}")/common.sh"

readonly BENCH_MAX_LOAD="${BENCH_MAX_LOAD:-1.5}"
readonly XML="${REPO_ROOT}/build/release/bench.xml"

step "benchmarks (release)"

cmake --preset release -DSUBEDIT_LTO_JOBS="${JOBS}"
cmake --build --preset release -j "${JOBS}" --target subedit_bench

# Le `|| true` est délibéré : le script rend 1 quand la charge n'est pas
# descendue, ce qui n'est pas une raison de ne pas mesurer. Il écrit la charge
# finale dans les deux cas, et c'est elle qui compte.
load="$("${REPO_ROOT}/src/scripts/await-quiet.sh" --below "${BENCH_MAX_LOAD}" || true)"

"${REPO_ROOT}/build/release/bin/subedit_bench" \
    --reporter console \
    --reporter "xml::out=${XML}"

"${REPO_ROOT}/src/scripts/record-bench.sh" \
    --xml "${XML}" --mode Release --load "${load}" --below "${BENCH_MAX_LOAD}"
