#!/usr/bin/env bash
# Élague les exécutions d'Actions accumulées sur le dépôt.
#
# **La règle, en une phrase :** de chaque workflow, garder les trente
# exécutions les plus récentes, plus la plus récente dont la branche est `main`
# si elle n'y est pas déjà ; supprimer le reste ; ne jamais toucher une
# exécution non terminée.
#
# Chacun des trois morceaux répond à quelque chose de précis.
#
# **Trente par workflow, et non une durée.** GitHub sait fixer une durée — le
# réglage *Artifact and log retention* — mais elle n'efface que les journaux et
# les artefacts : la ligne d'exécution reste dans la liste, avec ses étapes
# vides. Et une durée ne nettoie rien quand l'activité est concentrée sur la
# semaine écoulée, ce qui était le cas au moment d'écrire ceci : sept jours
# auraient laissé 103 des 146 exécutions, trente par workflow en laissent 60.
#
# **La plus récente sur `main`.** C'est l'analyse complète du lundi, le filet
# hebdomadaire de `ci.yml`. Sa période est de sept jours et il arrive une
# douzaine d'exécutions par jour : sans exception elle sortirait des trente en
# trois jours, et le dépôt n'aurait plus trace de la seule exécution dont
# l'absence signifie quelque chose. « La plus récente » est au singulier, et
# c'est délibéré — au pluriel, la clause ressusciterait les quarante-sept
# exécutions du déclencheur `push` sur `main` supprimé à la #106, que cet
# élagage existe justement pour effacer.
#
# **Les exécutions non terminées.** GitHub refuse de les supprimer ; les
# demander ferait échouer le passage sur une course sans intérêt.
#
# ## Pourquoi un workflow, et non ce script lancé à la main
#
# Un jeton personnel ne peut pas supprimer une exécution : l'API répond
# `403 Resource not accessible by personal access token`. Seul le `GITHUB_TOKEN`
# d'un workflow le peut, avec `permissions: actions: write`. La suppression vit
# donc dans `.github/workflows/elagage.yml`, qui appelle ce script.
#
# La *lecture*, elle, passe avec un jeton personnel. C'est ce qui rend
# `--dry-run` utile depuis un poste de développement, et c'est ce qui rend la
# sélection démontrable : `--input` lui donne une liste écrite à la main plutôt
# que de la lui faire chercher, et `verify-gates.sh` s'en sert pour prouver que
# les cinq cas qui décident de ce fichier sont traités comme annoncé.
#
# ## Usage
#
#   prune-runs.sh [--dry-run] [--input FICHIER]
#
#   --dry-run          écrit les identifiants à supprimer, ne supprime rien
#   --input FICHIER    lit la liste depuis un fichier JSONL — un objet par
#                      ligne, tel que le produit
#                      `gh api … --paginate --jq '.workflow_runs[]'` — au lieu
#                      d'interroger l'API
#
# Sans `--input`, le dépôt est celui de `GITHUB_REPOSITORY` — que les Actions
# renseignent — ou, à défaut, celui du dépôt git courant.

set -euo pipefail

# Le nombre d'exécutions gardées par workflow. Un seul nombre, écrit une fois :
# le rendre réglable par option inviterait à le régler différemment ici et là,
# et la règle cesserait de tenir en une phrase.
readonly KEEP=30

readonly BOLD=$'\033[1m'
readonly RESET=$'\033[0m'

usage() {
    cat <<'FIN'
prune-runs.sh [--dry-run] [--input FICHIER]

Élague les exécutions d'Actions : de chaque workflow, garde les trente plus
récentes, plus la plus récente sur `main`, et supprime le reste. Les exécutions
non terminées ne sont jamais touchées.

  --dry-run          écrit les identifiants à supprimer, ne supprime rien
  --input FICHIER    lit la liste depuis un fichier JSONL au lieu de l'API
  -h, --help         ceci

Supprimer exige le GITHUB_TOKEN d'un workflow : un jeton personnel reçoit 403
sur les Actions. Depuis un poste de développement, seul --dry-run fonctionne.
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

require jq

repository="${GITHUB_REPOSITORY:-}"

# Le dépôt visé. Les Actions renseignent GITHUB_REPOSITORY ; en local il faut le
# demander à git. Résolu à part de la lecture, parce que la suppression en a
# besoin même quand la liste vient d'un fichier.
#
# Écrit en `if` plutôt qu'en `[[ … ]] && return` : sous `set -e`, ce dernier fait
# rendre 1 à la fonction quand le test échoue, et tue le script au cas le plus
# courant — celui où le dépôt est déjà connu.
resolve_repository() {
    if [[ -z "${repository}" ]]; then
        require gh
        repository="$(gh repo view --json nameWithOwner --jq .nameWithOwner)"
    fi
}

# La liste, sous forme d'un objet JSON par ligne.
fetch_runs() {
    if [[ -n "${input}" ]]; then
        [[ -f "${input}" ]] || { printf 'fichier introuvable : %s\n' "${input}" >&2; exit 1; }
        cat "${input}"
        return
    fi

    resolve_repository
    gh api "repos/${repository}/actions/runs" --paginate \
        --jq '.workflow_runs[]
              | {id, workflow_id, name, head_branch, status, created_at}'
}

# La sélection. Une fonction de la liste, et rien d'autre : ni horloge, ni
# réseau, ni état — c'est ce qui la rend démontrable sur un jeu écrit à la main.
#
# `index(…)` rend `null` quand l'identifiant est absent, et un entier sinon —
# y compris zéro, qui est vrai en jq. La négation se comporte donc bien pour le
# premier élément, ce qu'un `> 0` aurait manqué.
select_doomed() {
    jq -rs --argjson keep "${KEEP}" '
        group_by(.workflow_id)
        | map(
            (sort_by(.created_at) | reverse) as $runs
            | ($runs[0:$keep] | map(.id))          as $recent
            | ([$runs[] | select(.head_branch == "main")][0].id) as $latest_main
            | $runs
            | map(. as $run
                  | select($run.status == "completed"
                           and ($recent | index($run.id) | not)
                           and ($run.id != $latest_main)))
          )
        | flatten
        | .[].id
    '
}

runs="$(fetch_runs)"
total="$(printf '%s' "${runs}" | grep -c '' || true)"
doomed="$(printf '%s\n' "${runs}" | select_doomed)"
doomed_count="$(printf '%s' "${doomed}" | grep -c '' || true)"
kept=$(( total - doomed_count ))

if (( dry_run == 1 )); then
    [[ -z "${doomed}" ]] || printf '%s\n' "${doomed}"
    printf '%s%d exécutions, %d à supprimer, %d gardées%s\n' \
        "${BOLD}" "${total}" "${doomed_count}" "${kept}" "${RESET}" >&2
    exit 0
fi

require gh
resolve_repository

deleted=0
refused=0
while read -r id; do
    [[ -n "${id}" ]] || continue
    # `</dev/null` : sans lui, un appel qui lirait l'entrée standard avalerait
    # le reste de la liste et la boucle s'arrêterait après un tour.
    if gh api -X DELETE "repos/${repository}/actions/runs/${id}" --silent \
        </dev/null 2>/dev/null; then
        deleted=$(( deleted + 1 ))
    else
        # Une exécution peut disparaître entre la lecture et la suppression, ou
        # être devenue non supprimable. Ça ne justifie pas d'abandonner les
        # suivantes : le passage suivant les reprendra de toute façon.
        printf 'refusée : %s\n' "${id}" >&2
        refused=$(( refused + 1 ))
    fi
done <<< "${doomed}"

# Ce qui reste vraiment, et non ce qu'on comptait garder : une suppression
# refusée laisse son exécution en place.
printf '%s%d supprimées, %d refusées, %d restantes%s\n' \
    "${BOLD}" "${deleted}" "${refused}" "$(( total - deleted ))" "${RESET}"

# La mesure demandée par la #123, refaite à chaque passage plutôt que relevée
# une fois : un chiffre figé dans un document vieillit, celui-ci non.
if [[ -n "${GITHUB_STEP_SUMMARY:-}" ]]; then
    {
        printf '## Élagage des exécutions\n\n'
        printf '| | |\n| :--- | ---: |\n'
        printf '| avant | %d |\n' "${total}"
        printf '| supprimées | %d |\n' "${deleted}"
        printf '| refusées | %d |\n' "${refused}"
        printf '| après | %d |\n' "$(( total - deleted ))"
        printf '\nRègle : les %d plus récentes de chaque workflow, ' "${KEEP}"
        printf 'plus la plus récente sur `main`.\n'
    } >> "${GITHUB_STEP_SUMMARY}"
fi
