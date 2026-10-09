#!/usr/bin/env bash
# Les tests sous ASan et UBSan. Le preset asan enregistre aussi les tests de
# bout en bout, sans filtre d'étiquette : c'est ici qu'ils tournent.
#
# **`GATE_PROOF_TESTS`, réservée à `verify-gates.sh`** (#686) : une expression régulière de noms de
# tests, et seuls ceux-là tournent. La preuve d'une porte injecte UN test défectueux et attend
# que l'étape échoue ; rejouer pour cela les trois mille autres sous sanitizers coûtait un quart
# d'heure par preuve. L'étape reste la vraie — même arbre, mêmes options, même `ctest` — et
# Une expression qui ne désigne
# aucun test fait rendre zéro à `ctest` : la preuve le voit, puisqu'elle attend un échec. `gate.sh` refuse la variable pour `check` et `check-local` :
# une porte réduite ne se lance pas par mégarde.
set -euo pipefail
# shellcheck source=src/scripts/gate/common.sh
source "$(dirname "${BASH_SOURCE[0]}")/common.sh"
step "tests sous sanitizers"
cmake --preset asan
cmake --build --preset asan -j "${JOBS}"
if [[ -n "${GATE_PROOF_TESTS:-}" ]]; then
    printf '%s  seuls les tests « %s » tournent (preuve de porte)%s\n' "${YELLOW}" "${GATE_PROOF_TESTS}" "${RESET}"
    "${REPO_ROOT}/src/scripts/limit-cores.sh" ctest --preset asan -R "${GATE_PROOF_TESTS}"
else
    "${REPO_ROOT}/src/scripts/limit-cores.sh" ctest --preset asan
fi
