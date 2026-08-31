#!/usr/bin/env bash
# Le format des sources, appliqué ou vérifié.
#
# `--check` ne modifie rien et échoue sur le premier écart ; sans lui, il
# réécrit. C'est la même liste de fichiers dans les deux cas, et c'est ce qui
# fait que `make format` répare exactement ce que `make format-check` refuse.

set -euo pipefail

# shellcheck source=src/scripts/gate/common.sh
source "$(dirname "${BASH_SOURCE[0]}")/common.sh"

check=0

usage() {
    cat >&2 <<'USAGE'
usage: format.sh [--check]

  --check  vérifie sans modifier, et échoue sur le premier écart
USAGE
    exit 2
}

while (($# > 0)); do
    case "$1" in
    --check) check=1; shift ;;
    -h | --help) usage ;;
    *) printf 'argument inconnu : %s\n' "$1" >&2; usage ;;
    esac
done

require clang-format

# `find` et non une variable du Makefile : la liste se calcule où elle sert, et
# un fichier ajouté entre deux appels y entre sans que rien d'autre le sache.
mapfile -t sources < <(find "${REPO_ROOT}/src" \( -name '*.cpp' -o -name '*.hpp' \) | sort)

if ((check)); then
    step "format"
    clang-format --dry-run --Werror "${sources[@]}"
else
    step "formatage"
    clang-format -i "${sources[@]}"
fi
