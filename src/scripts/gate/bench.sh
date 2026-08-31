#!/usr/bin/env bash
# Les benchmarks, en Release, et le relevé versé au journal.
#
# **`JOBS` gouverne aussi l'optimisation entre modules.** Sans lui, le LTO part
# en `-flto=auto`, c'est-à-dire autant de processus que de cœurs, à chaque
# édition de liens — un parallélisme qui n'apparaît dans aucun `-j` et qui
# sature une machine sur laquelle on fait autre chose.
#
# La charge est relevée avant de mesurer : une mesure prise pendant qu'un
# navigateur compile du JavaScript ne dit rien du code, elle dit l'état de la
# machine.
#
# **Au-dessus du seuil, rien n'est inscrit** — issue #270. Le relevé entrait au
# journal, sans droit de fixer d'extrême, et disait sa charge en en-tête. C'était
# une demi-mesure au sens propre : une section de version que personne ne peut
# comparer à rien, puisqu'on ne sait pas ce qui, dedans, vient du code. Presque
# un relevé sur deux de la phase 7 était de ceux-là.
#
# **Une mesure qu'on ne peut pas lire vaut moins qu'une mesure absente**, parce
# qu'elle occupe la place de celle qui manque : le journal a une section par
# version, donc la section sale rend invisible le fait qu'il n'y a pas eu de
# mesure. Ne rien écrire le dit.
#
# Les benchmarks tournent quand même, et leur sortie console reste sous les yeux
# de qui a lancé la commande : ce qui est refusé est l'inscription, pas le
# regard.
#
# **Ce n'est pas un échec.** La machine occupée n'est pas un défaut du code, et
# `make check-local` n'a aucune raison de rougir pour elle.

set -euo pipefail

# shellcheck source=src/scripts/gate/common.sh
source "$(dirname "${BASH_SOURCE[0]}")/common.sh"

readonly BENCH_MAX_LOAD="${BENCH_MAX_LOAD:-1.5}"
readonly XML="${REPO_ROOT}/build/release/bench.xml"

step "benchmarks (release)"

cmake --preset release -DSUBEDIT_LTO_JOBS="${JOBS}"
cmake --build --preset release -j "${JOBS}" --target subedit_bench

# Le code de retour est lu, et non écarté : c'est lui qui décide si le relevé
# sera inscrit. La charge finale est écrite dans les deux cas.
quiet=0
load="$("${REPO_ROOT}/src/scripts/await-quiet.sh" --below "${BENCH_MAX_LOAD}")" || quiet=$?

"${REPO_ROOT}/build/release/bin/subedit_bench" \
    --reporter console \
    --reporter "xml::out=${XML}"

if ((quiet != 0)); then
    printf '\n%s⚠%s charge %s, seuil %s — rien n'\''est versé au journal.\n' \
        "${YELLOW}" "${RESET}" "${load}" "${BENCH_MAX_LOAD}" >&2
    printf '  les chiffres ci-dessus disent l'\''état de la machine autant que celui du code ;\n' >&2
    printf '  rejouer « make bench » quand elle sera calme, ou noter dans la PR qu'\''il n'\''y a\n' >&2
    printf '  pas de mesure pour cette version.\n' >&2
    exit 0
fi

"${REPO_ROOT}/src/scripts/record-bench.sh" \
    --xml "${XML}" --mode Release --load "${load}" --below "${BENCH_MAX_LOAD}"
