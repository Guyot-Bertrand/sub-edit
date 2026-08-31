#!/usr/bin/env bash
# Le manuel : ses exemples d'appel, ses captures, ses renvois.
#
# **Trois filets, et ils ne voient pas la même chose.** Les blocs `console`
# viennent du binaire, engendrés en exécutant réellement la commande ; les
# images viennent de la vraie fenêtre, montrée sans écran ; les renvois internes
# n'étaient vérifiés par rien jusqu'à #243 — une section renommée laisse
# derrière elle des ancres qui s'affichent aussi proprement qu'un lien juste.
#
# `--check` vérifie sans rien réécrire. Sans lui, on régénère : les exemples
# sont réécrits, et les captures qui ont bougé sont promues. Le programme
# n'écrit jamais une référence — il écrit `<nom>.new.png`, et le comparateur
# décide ensuite.

set -euo pipefail

# shellcheck source=src/scripts/gate/common.sh
source "$(dirname "${BASH_SOURCE[0]}")/common.sh"

readonly CAPTURES="${REPO_ROOT}/docs/manual/subedit-gui/captures"

check=0

usage() {
    cat >&2 <<'USAGE'
usage: manual.sh [--check]

  --check  vérifie sans régénérer, et échoue sur le premier écart
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

step "exemples du manuel"

cmake --preset dev >/dev/null
cmake --build --preset dev -j "${JOBS}" --target subedit-cli subedit_screenshots

if ((check)); then
    "${REPO_ROOT}/src/scripts/generate-manual.sh" --check
else
    "${REPO_ROOT}/src/scripts/generate-manual.sh"
fi

step "captures d'écran"

"${REPO_ROOT}/build/dev/bin/subedit_screenshots" --output-dir "${CAPTURES}"

if ((check)); then
    "${REPO_ROOT}/src/scripts/compare-screenshots.py" --dir "${CAPTURES}" --check
else
    "${REPO_ROOT}/src/scripts/compare-screenshots.py" --dir "${CAPTURES}"
fi

"${REPO_ROOT}/src/scripts/check-screenshots.py"
"${REPO_ROOT}/src/scripts/check-manual-links.py"
