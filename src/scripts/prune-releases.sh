#!/usr/bin/env bash
# Élague les releases de patch des milestones passées — issue #232.
#
# **La règle, en une phrase :** garder toutes les releases `vX.Y.0`, garder les
# releases de patch de la milestone en cours, supprimer celles des milestones
# passées — et ne jamais toucher un tag.
#
# « La milestone en cours » est le couple `X.Y` le plus haut parmi les releases
# présentes. Tant qu'on est en `0.10.*`, tous les `0.10.Z` restent ; le jour où
# `v0.11.0` est publiée, les releases `0.10.1` à `0.10.N` partent avec leurs
# paquets, et `v0.10.0` reste.
#
# **Seule la release s'en va, jamais le tag.** Un tag ne coûte rien, et il garde
# la possibilité de reconstruire n'importe quelle version depuis l'historique :
# `gh release delete` sans `--cleanup-tag` retire la release et ses fichiers, et
# laisse le tag où il est.
#
# **Une conséquence à savoir.** Un correctif publié sur une milestone passée —
# un `v0.9.1` poussé après `v0.10.0` — est construit, publié, puis élagué dans la
# foulée. La règle ne garde les patchs que de la milestone en cours, et elle ne
# fait pas d'exception pour le plus récent.
#
# **Sauf à la main.** Reconstruire un vieux patch par `workflow_dispatch` est un
# geste délibéré — un paquet à refaire — et l'élaguer dans la foulée annulerait ce
# qu'on vient de demander. Le script constate l'événement (`GITHUB_EVENT_NAME`,
# que les Actions renseignent d'elles-mêmes), dit qu'il n'élague pas, et sort en
# 0 sans avoir cherché ni supprimé quoi que ce soit. La garde est ici plutôt que
# dans un `if:` du workflow pour la raison de la section suivante : une
# condition de YAML ne se rejoue pas. Elle ne vaut pas pour `--dry-run`, qui ne
# supprime rien.
#
# **Deux autres choses que la règle ne disait pas.** Les brouillons sont écartés
# avant tout : `gh api …/releases` les rend aussi, et un brouillon `v0.12.0`
# écrit à la main ferait passer la milestone 0.11 au passé — ses patchs partiraient
# pour une version qui n'existe pas. Et une suppression refusée n'est pas une
# réussite : le script tente toutes les suppressions, puis sort en erreur en
# nommant chaque release qui est restée, pour que le travail de publication ne
# reste pas vert sur une release qu'on croyait élaguée.
#
# Les noms qui ne sont pas de la forme `vX.Y.Z` ne sont ni comptés ni touchés :
# une release écrite à la main sous un autre nom n'est pas l'affaire de ce script.
#
# ## Pourquoi la sélection est à part
#
# Pour la raison de `prune-runs.sh` : un script se lance en local, un bloc de YAML
# non, et une suppression qu'aucune exécution ne vérifie serait une croyance.
# `--input` donne la liste au lieu de la chercher, et `verify-gates.sh` s'en sert
# pour prouver la sélection sur une liste écrite à la main.
#
# ## Usage
#
#   prune-releases.sh [--dry-run] [--input FICHIER]
#
#   --dry-run          écrit les tags dont la release partirait, ne supprime rien
#   --input FICHIER    lit les tags des releases depuis un fichier, un par ligne,
#                      au lieu d'interroger l'API
#
# Sans `--input`, le dépôt est celui de `GITHUB_REPOSITORY` — que les Actions
# renseignent — ou, à défaut, celui du dépôt git courant. La lecture passe avec
# un jeton personnel ; la suppression demande `contents: write`.

set -euo pipefail

readonly BOLD=$'\033[1m'
readonly RESET=$'\033[0m'

usage() {
    cat <<'FIN'
prune-releases.sh [--dry-run] [--input FICHIER]

Élague les releases : garde toutes les vX.Y.0 et les patchs de la milestone en
cours, supprime les patchs des milestones passées. Les tags restent. Les
brouillons sont ignorés. Une suppression refusée fait sortir en erreur, après
que toutes ont été tentées. Sous `workflow_dispatch` (GITHUB_EVENT_NAME), rien
n'est élagué.

  --dry-run          écrit les tags dont la release partirait, ne supprime rien
  --input FICHIER    lit les tags des releases depuis un fichier, un par ligne
  -h, --help         ceci
FIN
}

dry_run=0
input=""

while (( $# > 0 )); do
    case "$1" in
        --dry-run) dry_run=1; shift ;;
        --input)
            [[ $# -ge 2 ]] || { printf '--input attend un fichier\n' >&2; exit 2; }
            input="$2"; shift 2 ;;
        -h|--help) usage; exit 0 ;;
        *) printf 'option inconnue : %s\n' "$1" >&2; usage >&2; exit 2 ;;
    esac
done

require() {
    command -v "$1" >/dev/null 2>&1 || {
        printf '%s manque : ./src/scripts/setup-toolchain.sh l installe\n' "$1" >&2
        exit 1
    }
}

repository="${GITHUB_REPOSITORY:-}"

# Écrit en `if` plutôt qu'en `[[ … ]] && …`, pour la raison écrite dans
# `prune-runs.sh` : sous `set -e`, la forme courte tue le script au cas le plus
# courant, celui où le dépôt est déjà connu.
resolve_repository() {
    if [[ -z "${repository}" ]]; then
        require gh
        repository="$(gh repo view --json nameWithOwner --jq .nameWithOwner)"
    fi
}

# Les tags des releases publiées, un par ligne. Un brouillon n'est ni une version
# ni une release à élaguer : il est écarté ici, avant que la milestone en cours
# soit déduite et avant que la moindre suppression soit choisie.
fetch_tags() {
    if [[ -n "${input}" ]]; then
        [[ -f "${input}" ]] || { printf 'fichier introuvable : %s\n' "${input}" >&2; exit 1; }
        cat "${input}"
        return
    fi

    require gh
    resolve_repository
    gh api "repos/${repository}/releases" --paginate \
        --jq '.[] | select(.draft == false) | .tag_name'
}

# La sélection. Une fonction de la liste, et rien d'autre — ni horloge, ni
# réseau, ni état.
#
# Deux passages sur la même liste : le premier trouve la milestone en cours, le
# second condamne les patchs qui n'en sont pas. Les numéros sont comparés comme
# des nombres, et c'est tout l'objet : comparés comme du texte, `0.9` passerait
# après `0.10`.
select_doomed() {
    awk '
        match($0, /^v[0-9]+\.[0-9]+\.[0-9]+$/) {
            split(substr($0, 2), part, ".")
            count++
            major[count] = part[1] + 0
            minor[count] = part[2] + 0
            patch[count] = part[3] + 0
            name[count] = $0
            if (count == 1 || major[count] > top_major ||
                (major[count] == top_major && minor[count] > top_minor)) {
                top_major = major[count]
                top_minor = minor[count]
            }
        }
        END {
            for (i = 1; i <= count; i++) {
                current = major[i] == top_major && minor[i] == top_minor
                if (patch[i] > 0 && !current)
                    print name[i]
            }
        }
    '
}

# Une reconstruction à la demande republie le paquet et n'élague pas. Le constat
# précède la lecture des releases : pour ne rien supprimer, il n'y a rien à
# chercher. `--dry-run` y échappe, puisqu'il ne supprime rien de toute façon.
if (( dry_run == 0 )) && [[ "${GITHUB_EVENT_NAME:-}" == "workflow_dispatch" ]]; then
    printf 'reconstruction à la demande (workflow_dispatch) : aucune release élaguée\n'
    if [[ -n "${GITHUB_STEP_SUMMARY:-}" ]]; then
        printf '## Élagage des releases\n\nReconstruction à la demande : rien n'"'"'est élagué.\n' \
            >> "${GITHUB_STEP_SUMMARY}"
    fi
    exit 0
fi

tags="$(fetch_tags)"
total="$(printf '%s' "${tags}" | grep -c '' || true)"
doomed="$(printf '%s\n' "${tags}" | select_doomed)"
doomed_count="$(printf '%s' "${doomed}" | grep -c '' || true)"

if (( dry_run == 1 )); then
    [[ -z "${doomed}" ]] || printf '%s\n' "${doomed}"
    printf '%s%d releases, %d à supprimer, %d gardées%s\n' \
        "${BOLD}" "${total}" "${doomed_count}" "$(( total - doomed_count ))" "${RESET}" >&2
    exit 0
fi

require gh
resolve_repository

deleted=0
refused=()
while read -r tag; do
    [[ -n "${tag}" ]] || continue
    # `</dev/null` : sans lui, gh lirait l'entrée standard et avalerait le reste
    # de la liste. Pas de `--cleanup-tag` : c'est ce qui garde le tag.
    if gh release delete "${tag}" --repo "${repository}" --yes </dev/null >/dev/null 2>&1; then
        printf '  supprimée : %s (le tag reste)\n' "${tag}"
        deleted=$(( deleted + 1 ))
    else
        printf 'refusée : %s\n' "${tag}" >&2
        refused+=("${tag}")
    fi
done <<< "${doomed}"

printf '%s%d supprimées, %d refusées, %d restantes%s\n' \
    "${BOLD}" "${deleted}" "${#refused[@]}" "$(( total - deleted ))" "${RESET}"

if [[ -n "${GITHUB_STEP_SUMMARY:-}" ]]; then
    {
        printf '## Élagage des releases\n\n'
        printf '| | |\n| :--- | ---: |\n'
        printf '| avant | %d |\n' "${total}"
        printf '| supprimées | %d |\n' "${deleted}"
        printf '| refusées | %d |\n' "${#refused[@]}"
        printf '| après | %d |\n' "$(( total - deleted ))"
        printf '\nRègle : toutes les `vX.Y.0`, et les patchs de la milestone en cours. '
        printf 'Les tags ne sont jamais supprimés.\n'
    } >> "${GITHUB_STEP_SUMMARY}"
fi

# Une suppression refusée fait échouer le travail : sortir en 0 laisserait la
# publication verte sur une release qu'on croyait élaguée. Elles ont toutes été
# tentées avant d'en arriver là, et chacune est nommée.
if (( ${#refused[@]} > 0 )); then
    printf 'suppression refusée, la release reste : %s\n' "${refused[*]}" >&2
    exit 1
fi
