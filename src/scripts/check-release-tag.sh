#!/usr/bin/env bash
# Refuse de publier un tag qui ne désigne pas une version de `main` — issue #232.
#
#   check-release-tag.sh TAG
#
# Trois conditions, et une release ne se construit qu'une fois les trois
# vérifiées :
#
#   forme      le tag s'écrit `vX.Y.Z` ;
#   version    le `project(VERSION)` du commit tagué porte le même numéro —
#              l'invariant 4 de `check-architecture.sh`, qui ne s'exécute
#              qu'en local et que rien ne rejouait au moment de publier ;
#   branche    le commit tagué appartient à `main`. Un tag posé sur une branche
#              de travail publierait un état que personne n'a fusionné.
#
# Lue dans le commit et non dans l'arbre de travail, pour la raison de
# l'invariant 4 : un tag désigne un commit.
#
# Il faut l'historique complet et `origin/main` : le checkout du workflow le
# demande par `fetch-depth: 0`.

set -euo pipefail

readonly REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"

refuse() {
    printf 'tag refusé : %s\n' "$1" >&2
    exit 1
}

[[ $# -eq 1 ]] || { printf 'usage : check-release-tag.sh TAG\n' >&2; exit 2; }
readonly TAG="$1"

[[ "${TAG}" =~ ^v[0-9]+\.[0-9]+\.[0-9]+$ ]] \
    || refuse "« ${TAG} » ne s'écrit pas vX.Y.Z"

commit="$(git -C "${REPO_ROOT}" rev-parse --verify --quiet "${TAG}^{commit}")" \
    || refuse "${TAG} ne désigne aucun commit"

declared="$(git -C "${REPO_ROOT}" show "${commit}:CMakeLists.txt" 2>/dev/null \
    | sed -n 's/^[[:space:]]*VERSION[[:space:]]\+\([0-9][0-9.]*\)[[:space:]]*$/\1/p' | head -1)"

[[ "v${declared}" == "${TAG}" ]] \
    || refuse "${TAG} et CMakeLists.txt (${declared:-absent}) ne s'accordent pas"

git -C "${REPO_ROOT}" merge-base --is-ancestor "${commit}" origin/main 2>/dev/null \
    || refuse "${TAG} désigne un commit qui n'est pas sur main"

printf '%s : version %s, sur main\n' "${TAG}" "${declared}"
