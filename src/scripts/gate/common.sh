#!/usr/bin/env bash
# Le socle des étapes de la porte : ce que chacune répète sinon.
#
# **Ce fichier se source, il ne s'exécute pas.** Il est néanmoins exécutable,
# parce que `check-architecture.sh` exige que tout fichier de `src/scripts` le
# soit — un script commité en 100644 échoue en CI après un clone frais. Lancé
# directement, il dit ce qu'il est et s'arrête.
#
# Ce qu'il apporte, et rien d'autre :
#
#   REPO_ROOT   la racine du dépôt, quel que soit le répertoire courant
#   JOBS        le plafond de parallélisme, hérité de l'environnement
#   step        annonce l'étape en cours
#   require     échoue avec un message utile plutôt qu'un « command not found »
#
# **`JOBS` vient de l'environnement et n'est jamais deviné ici.** Le `Makefile`
# l'exporte, la CI le pose à `nproc`, et un appel direct peut le poser aussi.
# Le défaut de deux est écrit une seule fois, dans le `Makefile`, avec la mesure
# qui le justifie ; le répéter ici ferait deux vérités.

if [[ "${BASH_SOURCE[0]}" == "${0}" ]]; then
    printf 'ce fichier se source, il ne s exécute pas : source %s\n' "${BASH_SOURCE[0]}" >&2
    exit 2
fi

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../../.." && pwd)"
readonly REPO_ROOT

JOBS="${JOBS:-2}"
readonly JOBS

readonly BOLD=$'\033[1m'
readonly GREEN=$'\033[32m'
readonly RED=$'\033[31m'
# Le jaune est entré avec #270 : une étape qui réussit sans avoir pu faire son
# travail n'est ni verte ni rouge. Les benchmarks sur machine occupée sont le
# seul cas à ce jour.
readonly YELLOW=$'\033[33m'
readonly RESET=$'\033[0m'

step() {
    printf '%s▸ %s%s\n' "${BOLD}" "$1" "${RESET}"
}

require() {
    command -v "$1" >/dev/null 2>&1 && return 0
    printf '%soutil manquant : %s%s\n' "${RED}" "$1" "${RESET}" >&2
    printf '  l installer avec : ./src/scripts/setup-toolchain.sh\n' >&2
    exit 1
}

# Le générateur préféré, sans être exigé : le figer dans les presets rendrait le
# projet inconstructible sur une machine qui n'a pas encore Ninja. Posé ici
# plutôt que dans chaque étape, et seulement s'il ne l'est pas déjà — un
# appelant qui a choisi garde son choix.
if [[ -z "${CMAKE_GENERATOR:-}" ]]; then
    if command -v ninja >/dev/null 2>&1; then
        export CMAKE_GENERATOR="Ninja"
    else
        export CMAKE_GENERATOR="Unix Makefiles"
    fi
fi
