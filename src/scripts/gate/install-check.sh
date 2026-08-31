#!/usr/bin/env bash
# L'installation dans un préfixe temporaire, et ce qu'on y lance.
#
# **Aucun autre contrôle ne dit cela.** Tout le reste s'exécute depuis l'arbre
# de construction, où un fichier de données est présent par accident de
# disposition — il est là parce que le dépôt le contient, pas parce qu'une règle
# l'a copié. Le défaut ne se voit qu'à la première installation propre.
set -euo pipefail
# shellcheck source=src/scripts/gate/common.sh
source "$(dirname "${BASH_SOURCE[0]}")/common.sh"
"$(dirname "${BASH_SOURCE[0]}")/release.sh"
step "installation dans un préfixe temporaire"
"${REPO_ROOT}/src/scripts/check-installation.sh" --build-dir "${REPO_ROOT}/build/release"
