#!/usr/bin/env bash
# Les deux paquets natifs, produits depuis l'arbre release — ADR 0023.
#
# **Une étape à part, qui n'entre dans aucune porte.** `check-installation.sh`
# construit déjà les deux paquets et confronte leurs listes de fichiers — c'est
# là qu'ils sont éprouvés. Celle-ci existe pour les avoir sous la main, nommés
# et rangés, le jour où on les publie.

set -euo pipefail

# shellcheck source=src/scripts/gate/common.sh
source "$(dirname "${BASH_SOURCE[0]}")/common.sh"

require cpack
require rpmbuild

"$(dirname "${BASH_SOURCE[0]}")/release.sh"

step "paquets natifs"

(cd "${REPO_ROOT}/build/release" && cpack -G "DEB;RPM" >/dev/null)

ls -1 "${REPO_ROOT}"/build/release/subedit*.deb "${REPO_ROOT}"/build/release/subedit*.rpm \
    | sed 's/^/  /'
