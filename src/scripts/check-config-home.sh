#!/usr/bin/env bash
# Refuse qu'une exécution des tests touche la configuration de qui les lance.
#
# **Le pendant de check-untracked.sh, de l'autre côté de la frontière.** Celui-là
# relève les fichiers non suivis par `git ls-files`, donc il ne voit que l'arbre
# de travail : un fichier écrit dans le répertoire personnel lui est invisible,
# alors que c'est exactement le même défaut — un test qui écrit là où il n'a rien
# à écrire. La phase 7 apporte des préférences persistées, et avec elles la
# première occasion de le commettre.
#
# **Deux dégâts, et le second est le plus vicieux.** Le test détruit les réglages
# de son auteur, ce qui se voit. Et il devient dépendant de ce que cet auteur
# avait déjà : un test qui passe chez qui n'a jamais lancé le binaire échoue chez
# qui l'a lancé une fois, ou l'inverse.
#
# **Ce qui est surveillé est nommément le nôtre**, et non le répertoire de
# configuration entier. Une porte tourne quinze minutes, pendant lesquelles le
# navigateur, l'éditeur et le gestionnaire de fenêtres de la machine écrivent
# dans `~/.config` sans rien nous devoir. Un contrôle qui les signalerait
# crierait au loup à chaque exécution, et un contrôle qui crie au loup finit
# désactivé.
#
# **Ce qui est refusé est une modification, pas une présence.** Qui développe
# subedit finit par lancer subedit, et sa configuration existe. Le contrôle
# compare donc deux empreintes, avant et après, et ne parle que de la
# différence — apparition, disparition ou changement.
#
# Il honore `XDG_CONFIG_HOME` pour la même raison que les tests le déplacent :
# c'est la variable dont tout emplacement standard dérive. C'est aussi ce qui
# permet à verify-gates.sh de prouver que la porte se referme sans écrire une
# seule fois dans le vrai répertoire personnel.

set -euo pipefail

readonly REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"

readonly CONFIG_HOME="${XDG_CONFIG_HOME:-${HOME}/.config}"
readonly WATCHED="${CONFIG_HOME}/subedit"

# Sous `build/`, qui est ignoré par git — comme le relevé de check-untracked.sh,
# et pour la même raison : un relevé versionné serait un fichier de plus à ne
# pas commiter.
readonly SNAPSHOT="${REPO_ROOT}/build/configuration-utilisateur.txt"

readonly RED=$'\033[31m'
readonly GREEN=$'\033[32m'
readonly RESET=$'\033[0m'

usage() {
    cat >&2 <<'USAGE'
usage: check-config-home.sh --record | --compare

  --record   relève l'état de la configuration de l'utilisateur, avant les tests
  --compare  échoue si les tests l'ont touchée
USAGE
    exit 2
}

# Chemin, taille et date de modification de chaque fichier, ou le mot « absent ».
#
# La date autant que la taille : une valeur remplacée par une autre de même
# longueur — un thème « dark » devenu « none » — ne changerait rien à la
# seconde. Et le mot « absent » plutôt qu'un relevé vide, pour que le cas le
# plus courant, celui du répertoire qui n'existe pas, se distingue d'un
# répertoire présent et vide.
fingerprint() {
    if [[ ! -e "${WATCHED}" ]]; then
        printf 'absent\n'
        return
    fi

    find "${WATCHED}" -printf '%y %s %T@ %p\n' | LC_ALL=C sort
}

record() {
    mkdir -p "$(dirname "${SNAPSHOT}")"
    fingerprint > "${SNAPSHOT}"
}

compare() {
    # Un relevé manquant n'est pas une configuration intacte : c'est un contrôle
    # qui n'a pas eu lieu. Le dire, plutôt que de passer au vert sans rien avoir
    # comparé.
    if [[ ! -f "${SNAPSHOT}" ]]; then
        printf '%s✗ aucun relevé préalable : lancer --record avant --compare%s\n' \
            "${RED}" "${RESET}" >&2
        exit 1
    fi

    local difference
    difference="$(diff "${SNAPSHOT}" <(fingerprint) || true)"

    if [[ -n "${difference}" ]]; then
        printf '%s✗ les tests ont touché la configuration de l'\''utilisateur :%s\n' \
            "${RED}" "${RESET}" >&2
        printf '    %s\n' "${WATCHED}" >&2
        printf '%s\n' "${difference}" >&2
        printf '  un test ne résout jamais un emplacement de configuration : il en\n' >&2
        printf '  reçoit un, ou passe par le harnais qui déplace XDG_CONFIG_HOME\n' >&2
        exit 1
    fi

    printf '%s✓%s la configuration de l'\''utilisateur est intacte\n' "${GREEN}" "${RESET}"
}

case "${1:-}" in
    --record)  record ;;
    --compare) compare ;;
    *)         usage ;;
esac
