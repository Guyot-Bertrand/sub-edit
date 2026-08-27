#!/usr/bin/env bash
# Refuse qu'une exécution de la porte laisse des fichiers derrière elle.
#
# **Le défaut a été payé une fois.** Les tests de bout en bout de #207 donnaient
# un nom nu à `--output`, si bien que le fichier atterrissait là où CTest les
# lance — la racine du dépôt. Quatre fichiers y sont restés, `make check` est
# passée au vert, et c'est un `git status` avant de pousser qui les a vus. Le
# harnais avait pourtant déjà `Scratch`, que tous les autres tests qui écrivent
# emploient.
#
# **Ce qui est refusé est une apparition, pas une présence.** Un fichier non
# suivi est le cas courant du travail en cours : une source neuve avant son
# `git add`, une note de côté. Refuser leur existence rendrait la porte
# inutilisable. Le contrôle compare donc deux relevés, avant et après, et ne
# parle que de la différence.
#
# `git ls-files --others --exclude-standard` plutôt que `git status --porcelain`
# filtré : il donne exactement la liste voulue, sans code de statut à analyser,
# et il honore les mêmes exclusions. C'est ce qui met **`src/data/` hors de
# portée** — le corpus privé de chaque machine est ignoré par git, donc il
# n'apparaît dans aucun des deux relevés. Vérifié plutôt que supposé, et c'est
# `.gitignore` qui le dit, avec `build/` et `reference/`.

set -euo pipefail

readonly REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"

# Sous `build/`, qui est ignoré par git : le relevé ne peut donc pas se
# dénoncer lui-même au relevé suivant.
readonly SNAPSHOT="${REPO_ROOT}/build/fichiers-non-suivis.txt"

readonly RED=$'\033[31m'
readonly GREEN=$'\033[32m'
readonly RESET=$'\033[0m'

usage() {
    cat >&2 <<'USAGE'
usage: check-untracked.sh --record | --compare

  --record   relève les fichiers non suivis, avant ce qu'on veut surveiller
  --compare  échoue sur tout fichier non suivi apparu depuis le relevé
USAGE
    exit 2
}

untracked() {
    git -C "${REPO_ROOT}" ls-files --others --exclude-standard | sort
}

record() {
    mkdir -p "$(dirname "${SNAPSHOT}")"
    untracked > "${SNAPSHOT}"
}

compare() {
    # Un relevé manquant n'est pas un arbre propre : c'est un contrôle qui n'a
    # pas eu lieu. Le dire, plutôt que de passer au vert sans rien avoir
    # comparé — un contrôle silencieusement sauté est pire que pas de contrôle,
    # puisqu'il reste vert sans rien prouver.
    if [[ ! -f "${SNAPSHOT}" ]]; then
        printf '%s✗ aucun relevé préalable : lancer --record avant --compare%s\n' \
            "${RED}" "${RESET}" >&2
        exit 1
    fi

    local appeared
    appeared="$(comm -13 "${SNAPSHOT}" <(untracked))"

    if [[ -n "${appeared}" ]]; then
        printf '%s✗ des fichiers non suivis sont apparus pendant les tests :%s\n' \
            "${RED}" "${RESET}" >&2
        printf '    %s\n' ${appeared} >&2
        printf '  un test qui écrit passe par le harnais « Scratch », jamais par un nom nu\n' >&2
        exit 1
    fi

    printf '%s✓%s aucun fichier laissé derrière\n' "${GREEN}" "${RESET}"
}

case "${1:-}" in
    --record)  record ;;
    --compare) compare ;;
    *)         usage ;;
esac
