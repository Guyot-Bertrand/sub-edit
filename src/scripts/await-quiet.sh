#!/usr/bin/env bash
# Attend que la machine soit assez calme pour qu'une mesure veuille dire
# quelque chose, et écrit la charge finale.
#
# Une mesure de performance prise pendant qu'un navigateur compile du
# JavaScript ne dit rien du code : elle dit l'état de la machine. Le journal des
# performances en a fait les frais — le relevé de la version 0.2.15, pris sous
# une charge de 7,2, a posé douze maxima d'un coup, dont un à 3,4 fois sa valeur
# habituelle, sur un ticket qui ne touchait aucun chemin mesuré.
#
# Attendre plutôt que refuser tout de suite : la cause la plus fréquente d'une
# charge élevée ici est la cible qui précède — `make check-local` construit et
# lance les tests de bout en bout juste avant d'appeler les benchmarks, et la
# moyenne d'une minute met une minute à l'oublier. Quelques dizaines de secondes
# suffisent le plus souvent.
#
# Sortie standard : la charge finale, un nombre, toujours écrite.
# Code de retour : 0 si elle est passée sous le seuil, 1 sinon. L'appelant
# décide ce qu'il fait d'une mesure prise sur une machine occupée ; ce script ne
# décide que d'attendre.

set -euo pipefail

readonly INTERVAL=5

below=2.0
timeout=180

usage() {
    cat >&2 <<'USAGE'
usage: await-quiet.sh [--below <charge>] [--timeout <secondes>]

  --below    seuil de charge sous lequel mesurer   (défaut : 2.0)
  --timeout  attente maximale, en secondes         (défaut : 180)

Écrit la charge finale sur la sortie standard. Rend 0 si elle est passée sous
le seuil, 1 si le délai a expiré — la charge est écrite dans les deux cas.
USAGE
    exit 2
}

while (( $# > 0 )); do
    case "$1" in
        --below)   [[ $# -ge 2 ]] || usage; below="$2";   shift 2 ;;
        --timeout) [[ $# -ge 2 ]] || usage; timeout="$2"; shift 2 ;;
        -h|--help) usage ;;
        *) printf 'argument inconnu : %s\n' "$1" >&2; usage ;;
    esac
done

# La moyenne d'une minute, et non le nombre de tâches exécutables : celui-ci est
# instantané, donc un échantillon peut tomber entre deux rafales et déclarer
# calme une machine qui ne l'est pas.
current_load() {
    cut -d' ' -f1 /proc/loadavg
}

# awk plutôt que bc : awk est dans coreutils de fait, bc est un paquet de plus,
# et la règle de dépendances du projet vaut aussi pour les scripts.
is_below() {
    awk -v load="$1" -v limit="$2" 'BEGIN { exit !(load < limit) }'
}

# Pas de /proc — un autre système, un conteneur exotique. Le contrôle ne
# s'applique pas plutôt que d'échouer : mieux vaut une mesure non qualifiée
# qu'une chaîne de construction qui refuse de tourner.
if [[ ! -r /proc/loadavg ]]; then
    printf 'inconnue\n'
    exit 0
fi

waited=0
load="$(current_load)"
while ! is_below "${load}" "${below}"; do
    if (( waited >= timeout )); then
        printf '%s\n' "${load}"
        exit 1
    fi
    sleep "${INTERVAL}"
    waited=$(( waited + INTERVAL ))
    load="$(current_load)"
done

printf '%s\n' "${load}"
