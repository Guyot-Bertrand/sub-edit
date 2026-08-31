#!/usr/bin/env bash
# Le `.rpm`, installé sur une vraie Fedora — issue #266.
#
# **Une étape à part, qui n'entre dans aucune porte**, comme `packages` et pour
# une raison de plus : elle demande un conteneur et le réseau. Une porte qui
# dépend du réseau est rouge un jour de panne, pour une raison étrangère au
# dépôt. Elle se lance à la main, et une fois par semaine sur `main` —
# `.github/workflows/fedora.yml`.
#
# Le paquet est reconstruit avant d'être éprouvé : éprouver celui qui traîne
# dans l'arbre reviendrait à répondre sur une version qu'on ne sait plus nommer.

set -euo pipefail

# shellcheck source=src/scripts/gate/common.sh
source "$(dirname "${BASH_SOURCE[0]}")/common.sh"

"$(dirname "${BASH_SOURCE[0]}")/packages.sh"

step "le .rpm sur une Fedora"

"${REPO_ROOT}/src/scripts/check-rpm.sh" --build-dir "${REPO_ROOT}/build/release"
