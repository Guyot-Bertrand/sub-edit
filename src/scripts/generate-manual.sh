#!/usr/bin/env bash
# Réécrit les exemples d'appel du manuel avec ce que les commandes produisent.
#
# Le manuel doit porter un exemple d'appel réel par commande. Recopié à la main,
# un tel exemple est de la documentation périmée en sursis : le seul numéro de
# version qu'il contenait a été corrigé douze fois pendant la phase 2, et rien
# ne vérifiait qu'il était vrai.
#
# Le projet a déjà l'idiome : CHANGELOG.md est généré depuis l'historique, puis
# commité. Un exemple d'appel est de la même nature — on ne l'écrit plus, on
# déclare quelle commande l'illustre.
#
#     <!-- exemple: subedit-cli --help fichier.srt; echo $? -->
#     ```console
#     $ subedit-cli --help fichier.srt; echo $?
#     subedit 0.2.4
#     0
#     ```
#
# Le marqueur porte une ligne de shell entière, jouée par `sh -c`. C'est ce qui
# permet aux exemples d'enchaîner une commande et la lecture de son code de
# retour, comme un utilisateur le ferait.
#
# Deux modes :
#
#   (sans argument)  réécrit les blocs dans les fichiers
#   --check          compare sans écrire, et nomme les blocs périmés
#
# `--check` compare la sortie des commandes au contenu du fichier, sans
# consulter git. Un `git diff` après régénération se tromperait dans un cas
# courant : une section de manuel fraîchement écrite et non commitée n'est pas
# périmée, mais le diff la verrait et refuserait.

set -euo pipefail

readonly REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
readonly MANUAL_DIR="${REPO_ROOT}/docs/manual"
# Le preset dev, et non release : la sortie d'un programme ne dépend pas de son
# niveau d'optimisation, et la porte de qualité construit déjà celui-là.
readonly BIN_DIR="${REPO_ROOT}/build/dev/bin"
readonly FENCE_OPEN='```console'
readonly FENCE_CLOSE='```'

readonly RED=$'\033[31m'
readonly GREEN=$'\033[32m'
readonly BOLD=$'\033[1m'
readonly RESET=$'\033[0m'

mode="write"
case "${1:-}" in
    "") ;;
    --check) mode="check" ;;
    *)
        printf 'usage : %s [--check]\n' "$(basename "$0")" >&2
        exit 2
        ;;
esac
readonly mode

rewritten=0
stale=0

# Le fichier en cours de réécriture, pour que l'arrêt sur erreur ne laisse pas
# de résidu derrière lui.
output_file=""
trap 'rm -f "${output_file}"' EXIT

# Un arrêt nomme toujours le fichier et la ligne : un manuel a plusieurs blocs,
# et « la commande a échoué » sans dire laquelle n'aide personne.
die() {
    local location="$1"
    shift
    printf '%s✗ %s%s\n' "${RED}" "${location}" "${RESET}" >&2
    local line
    for line in "$@"; do
        printf '    %s\n' "${line}" >&2
    done
    exit 1
}

# Renseigne example_output et example_status plutôt que d'écrire sur la sortie :
# l'appelant a besoin des deux, et une substitution de commande n'en rend qu'un.
example_output=""
example_status=0

run_example() {
    local command="$1"

    # Répertoire temporaire : un « fichier.srt » d'exemple ne doit jamais
    # désigner un fichier du dépôt, ni y laisser de trace.
    local workdir
    workdir="$(mktemp -d)"

    example_status=0
    # Les deux sorties sont fusionnées, dans l'ordre où l'utilisateur les
    # verrait. L'entrée est fermée : un exemple qui attendrait une saisie
    # bloquerait la génération au lieu d'échouer.
    example_output="$(
        cd "${workdir}" \
            && PATH="${BIN_DIR}:${PATH}" sh -c "${command}" 2>&1 </dev/null
    )" || example_status=$?

    rm -rf "${workdir}"
}

# Le bloc tel qu'il devrait être : l'invite, la commande, puis la sortie.
render_block() {
    local command="$1"
    local output="$2"
    printf '$ %s\n%s\n' "${command}" "${output}"
}

process_file() {
    local file="$1"
    local relative="${file#"${REPO_ROOT}/"}"

    output_file="$(mktemp)"

    local lineno=0
    local line=""

    while IFS= read -r line <&3; do
        lineno=$((lineno + 1))

        if [[ "${line}" != *'<!-- exemple:'* ]]; then
            printf '%s\n' "${line}" >>"${output_file}"
            continue
        fi

        local marker_line=${lineno}
        local location="${relative}:${marker_line}"

        [[ "${line}" =~ ^[[:space:]]*'<!-- exemple:'[[:space:]]*(.*[^[:space:]])[[:space:]]*'-->'[[:space:]]*$ ]] \
            || die "${location}" \
                "marqueur illisible : ${line}" \
                "forme attendue : <!-- exemple: <ligne de shell> -->"
        local command="${BASH_REMATCH[1]}"

        local fence=""
        IFS= read -r fence <&3 || die "${location}" \
            "le marqueur est la dernière ligne du fichier" \
            "un marqueur annonce le bloc qui le suit ; il en faut un."
        lineno=$((lineno + 1))

        [[ "${fence}" == "${FENCE_OPEN}" ]] || die "${location}" \
            "le marqueur n'est pas suivi d'un bloc ${FENCE_OPEN}" \
            "trouvé à la place : ${fence}"

        # Le bloc existant est lu pour être comparé, jamais pour être conservé.
        local previous=""
        local closed=0
        local block_line=""
        while IFS= read -r block_line <&3; do
            lineno=$((lineno + 1))
            if [[ "${block_line}" == "${FENCE_CLOSE}" ]]; then
                closed=1
                break
            fi
            previous+="${block_line}"$'\n'
        done
        (( closed == 1 )) || die "${location}" \
            "le bloc ouvert après le marqueur n'est jamais refermé"

        run_example "${command}"

        if (( example_status == 127 )); then
            die "${location}" \
                "commande introuvable : ${command}" \
                "les exemples s'exécutent avec ${BIN_DIR#"${REPO_ROOT}/"} en tête du PATH ;" \
                "construire d'abord, avec « make build »."
        fi

        if (( example_status != 0 )); then
            die "${location}" \
                "la commande a rendu le code ${example_status} : ${command}" \
                "un exemple du manuel illustre un appel qui marche." \
                "sortie :" \
                "${example_output}"
        fi

        # Un bloc vide n'illustre rien, et le pire résultat serait un manuel
        # silencieusement vidé de ses exemples. Une commande muette est donc une
        # erreur, et non un bloc vide écrit sans un mot.
        [[ -n "${example_output}" ]] || die "${location}" \
            "la commande n'a rien produit : ${command}" \
            "un bloc vide n'illustre rien ; le manuel ne sera pas vidé en silence."

        local expected
        expected="$(render_block "${command}" "${example_output}")"

        {
            printf '%s\n' "${line}"
            printf '%s\n' "${FENCE_OPEN}"
            printf '%s\n' "${expected}"
            printf '%s\n' "${FENCE_CLOSE}"
        } >>"${output_file}"

        if [[ "${previous}" == "${expected}"$'\n' ]]; then
            continue
        fi

        if [[ "${mode}" == "check" ]]; then
            stale=$((stale + 1))
            printf '  %s✗%s %s — le bloc ne correspond plus à « %s »\n' \
                "${RED}" "${RESET}" "${location}" "${command}"
        else
            rewritten=$((rewritten + 1))
            printf '  %s✓%s %s — « %s »\n' \
                "${GREEN}" "${RESET}" "${location}" "${command}"
        fi
    done 3<"${file}"

    if [[ "${mode}" == "write" ]]; then
        cp "${output_file}" "${file}"
    fi
    rm -f "${output_file}"
    output_file=""
}

if [[ ! -d "${BIN_DIR}" ]]; then
    die "${BIN_DIR#"${REPO_ROOT}/"}" \
        "répertoire de construction absent" \
        "les exemples ont besoin des binaires : lancer « make build »."
fi

if [[ "${mode}" == "check" ]]; then
    printf '%sexemples du manuel — vérification%s\n' "${BOLD}" "${RESET}"
else
    printf '%sexemples du manuel%s\n' "${BOLD}" "${RESET}"
fi

while IFS= read -r manual_file; do
    process_file "${manual_file}"
done < <(find "${MANUAL_DIR}" -name '*.md' -type f | sort)

if [[ "${mode}" == "check" ]]; then
    if (( stale > 0 )); then
        printf '%s%d bloc(s) périmé(s) — lancer « make manual »%s\n' \
            "${RED}" "${stale}" "${RESET}" >&2
        exit 1
    fi
    printf '%sles exemples sont à jour%s\n' "${GREEN}" "${RESET}"
else
    if (( rewritten > 0 )); then
        printf '%s%d bloc(s) réécrit(s)%s\n' "${GREEN}" "${rewritten}" "${RESET}"
    else
        printf 'aucun bloc à réécrire\n'
    fi
fi
