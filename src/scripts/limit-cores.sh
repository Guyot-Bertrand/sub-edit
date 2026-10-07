#!/usr/bin/env bash
# Lance une commande sur au plus $JOBS cœurs — fils compris.
#
# **Ce que `JOBS` ne bornait pas** — issue #630. `JOBS` plafonne le nombre de processus que `make`
# et `ninja` lancent. Il ne dit rien des fils qu'un processus crée de lui-même, et libmpv en crée à
# chaque lecteur : mesuré, un seul cas de test de lecteur — trois dixièmes de seconde — occupe plus
# de cinq cœurs en rafale, et `ffmpeg`, qui fabrique la vidéo du banc, huit. La porte, qui se
# donnait deux cœurs, en prenait donc bien davantage chaque fois qu'elle passait par là, sans que
# rien de ce qu'elle lit — `make parallelism` y compris — ne puisse le voir.
#
# **L'affinité est la seule borne que les fils héritent.** Posée sur le processus qui lance les
# autres, elle vaut pour tous ceux que libmpv, Qt ou ffmpeg démarrent ensuite, quelle que soit la
# façon dont ils comptent les cœurs. Mesuré sur les cas du lecteur : le temps système tombe de
# 1,6 s à 0,2 s et la durée ne s'allonge pas — des fils qui ne peuvent pas se disperser se gênent
# moins qu'ils ne se gênaient.
#
# **Les derniers cœurs, et non les premiers** : le bureau et les interruptions vivent sur les
# premiers, et une porte qui occupe deux cœurs le fait mieux loin de ce que la personne qui la
# lance utilise. Sans `taskset`, ou sur une machine qui refuse ces cœurs, la commande tourne quand
# même, et le dit — une porte qui s'arrêterait là pénaliserait la machine la plus modeste.
#
# `JOBS` vient de l'environnement, comme pour toutes les étapes ; la CI le pose à tous ses cœurs,
# et la commande n'est alors pas contrainte.
#
#   limit-cores.sh ctest --preset asan

set -euo pipefail

if (($# == 0)); then
    printf 'usage : %s COMMANDE [ARGUMENT…]\n' "$(basename "$0")" >&2
    exit 2
fi

readonly JOBS="${JOBS:-2}"
online="$(getconf _NPROCESSORS_ONLN)"
readonly online

# Tous les cœurs sont permis : rien à borner.
if ((JOBS >= online)); then
    exec "$@"
fi

if ! command -v taskset >/dev/null 2>&1; then
    printf '%s : taskset est absent, la commande tourne sur tous les cœurs\n' "$(basename "$0")" >&2
    exec "$@"
fi

cores="$((online - JOBS))-$((online - 1))"
if ! taskset --cpu-list "${cores}" true 2>/dev/null; then
    printf '%s : les cœurs %s sont refusés, la commande tourne sur tous les cœurs\n' \
        "$(basename "$0")" "${cores}" >&2
    exec "$@"
fi

exec taskset --cpu-list "${cores}" "$@"
