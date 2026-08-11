#!/usr/bin/env bash
# Vérifie qu'aucun parallélisme n'échappe à $(JOBS).
#
# Le Makefile borne le parallélisme local à deux cœurs par défaut — voir le
# commentaire au-dessus de JOBS dans le Makefile. Cette discipline ne vaut que
# tant que chaque site de parallélisme la respecte : une recette qui fige
# `-j 8` en dur, un script qui lance `xargs -P 4` ou se fie à `nproc`, un module
# CMake qui code `-flto=8`, contournent tous silencieusement le plafond. Rien
# ne les empêchait avant ce script ; c'est ce qu'il corrige.
#
# Trois familles de fichiers, trois formes de contournement :
#
#   - Makefile        : une recette (ligne commençant par une tabulation) où
#                        `-j` ou `-P` n'est pas immédiatement suivi de
#                        `$(JOBS)`.
#   - src/scripts/*.sh : une ligne de code (pas un commentaire) qui introduit
#                        du parallélisme propre — `-j`/`-P` suivi d'un nombre,
#                        un `xargs` avec `-P`, un appel à `nproc`, une commande
#                        mise en arrière-plan par un `&` final.
#   - cmake/*.cmake,
#     CMakeLists.txt   : un `-flto=` codé en dur en dehors du mécanisme
#                        `SUBEDIT_LTO_JOBS` (voir cmake/Lto.cmake).
#
# Les commentaires et les exemples de documentation ne doivent pas déclencher
# le contrôle : le commentaire du Makefile montre `make build JOBS=8` et
# `JOBS=$$(nproc) make check`, celui de cmake/Lto.cmake montre
# `-DSUBEDIT_LTO_JOBS=8`. Ce sont des lignes qui ne commencent pas par une
# tabulation dans le Makefile, ou qui commencent par `#` ailleurs : elles
# restent hors du champ de la vérification.

set -euo pipefail

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"

# --root permet de pointer la vérification sur une copie temporaire, pour la
# mettre à l'épreuve sans toucher à l'arbre de travail réel.
while [[ $# -gt 0 ]]; do
    case "$1" in
        --root)
            REPO_ROOT="$2"
            shift 2
            ;;
        *)
            printf 'argument inconnu : %s\n' "$1" >&2
            exit 1
            ;;
    esac
done
readonly REPO_ROOT

readonly RED=$'\033[31m'
readonly GREEN=$'\033[32m'
readonly RESET=$'\033[0m'

failures=0

report_failure() {
    printf '%s✗%s %s\n' "${RED}" "${RESET}" "$*" >&2
    failures=$((failures + 1))
}

report_success() {
    printf '%s✓%s %s\n' "${GREEN}" "${RESET}" "$*"
}

# Un jeton "-j"/"-P" (collé ou séparé) est légitime seulement s'il vaut
# littéralement $(JOBS). Prend un jeton et le suivant (peut être vide),
# renvoie 0 (légitime) ou 1 (contournement).
is_legitimate_jobs_token() {
    local token="$1" next="$2"

    case "${token}" in
        -j|-P)
            [[ "${next}" == '$(JOBS)' ]]
            ;;
        -j*|-P*)
            [[ "${token:2}" == '$(JOBS)' ]]
            ;;
        *)
            return 0
            ;;
    esac
}

# Invariant 1 — chaque recette du Makefile qui parallélise passe par $(JOBS).
check_makefile() {
    local makefile="${REPO_ROOT}/Makefile"
    [[ -f "${makefile}" ]] || { report_success "aucun Makefile, rien à vérifier"; return 0; }

    local -a offenders=()
    local line
    while IFS= read -r line; do
        [[ "${line}" == $'\t'* ]] || continue

        local -a tokens
        read -ra tokens <<< "${line}"

        local i token next
        for i in "${!tokens[@]}"; do
            token="${tokens[${i}]}"
            case "${token}" in
                -j*|-P*)
                    next="${tokens[$((i + 1))]:-}"
                    if ! is_legitimate_jobs_token "${token}" "${next}"; then
                        offenders+=("${line#$'\t'}")
                        break
                    fi
                    ;;
            esac
        done
    done < "${makefile}"

    if (( ${#offenders[@]} > 0 )); then
        report_failure "Makefile : parallélisme qui contourne \$(JOBS) :
$(printf '    %s\n' "${offenders[@]}")"
    else
        report_success "Makefile : tout le parallélisme passe par \$(JOBS)"
    fi
}

# Invariant 2 — les scripts n'introduisent aucun parallélisme à eux.
#
# Ce script lui-même est exclu du balayage : il cite -j, -P, xargs et nproc en
# toutes lettres, comme motifs à reconnaître plutôt que comme commandes à
# exécuter. Le confronter à ses propres motifs ne prouverait rien d'autre
# qu'il se contient lui-même.
#
# Un ✓ par invariant, pas un ✓ par fichier balayé : la liste des fautifs ne
# sort que sur échec, sans quoi chaque script ou fichier cmake ajouté au
# projet ajouterait une ligne permanente à une sortie censée rester lisible.
check_scripts() {
    local self
    self="$(basename "${BASH_SOURCE[0]}")"

    local -a offenders=()
    local script relative
    while IFS= read -r -d '' script; do
        relative="${script#"${REPO_ROOT}"/}"
        [[ "$(basename "${script}")" == "${self}" ]] && continue

        local line trimmed
        while IFS= read -r line; do
            trimmed="${line#"${line%%[![:space:]]*}"}"
            [[ -z "${trimmed}" || "${trimmed}" == '#'* ]] && continue

            local reason=""

            local -a tokens
            read -ra tokens <<< "${line}"
            local i token next
            for i in "${!tokens[@]}"; do
                token="${tokens[${i}]}"
                case "${token}" in
                    -j|-P)
                        next="${tokens[$((i + 1))]:-}"
                        if [[ "${next}" =~ ^[0-9]+$ ]]; then
                            reason="parallélisme codé en dur (${token} ${next})"
                        elif [[ "${token}" == "-P" ]] && [[ " ${line} " == *' xargs '* ]]; then
                            reason="xargs -P propre au script"
                        fi
                        ;;
                    -j[0-9]*|-P[0-9]*)
                        reason="parallélisme codé en dur (${token})"
                        ;;
                esac
                [[ -n "${reason}" ]] && break
            done

            if [[ -z "${reason}" ]] && [[ "${line}" =~ (^|[^[:alnum:]_])nproc([^[:alnum:]_]|$) ]]; then
                reason="appel à nproc"
            fi

            if [[ -z "${reason}" ]]; then
                # Un commentaire de fin de ligne ne change pas la nature de la
                # commande : `tâche & # commentaire` reste une tâche mise en
                # arrière-plan. On le retire avant de chercher le `&` final —
                # sans quoi c'est la fin du commentaire qui est testée, pas la
                # fin de la commande.
                local rtrimmed="${line%"${line##*[![:space:]]}"}"
                local without_comment="${rtrimmed%% #*}"
                without_comment="${without_comment%"${without_comment##*[![:space:]]}"}"
                if [[ "${without_comment}" == *'&' && "${without_comment}" != *'&&' ]]; then
                    reason="commande mise en arrière-plan (&)"
                fi
            fi

            [[ -n "${reason}" ]] && offenders+=("${relative} : ${reason} : ${trimmed}")
        done < "${script}"
    done < <(find "${REPO_ROOT}/src/scripts" -maxdepth 1 -name '*.sh' -print0 2>/dev/null)

    if (( ${#offenders[@]} > 0 )); then
        report_failure "src/scripts : parallélisme propre à un script :
$(printf '    %s\n' "${offenders[@]}")"
    else
        report_success "src/scripts : aucun script n'introduit son propre parallélisme"
    fi
}

# Invariant 3 — le LTO ne code aucun nombre de processus en dehors de
# SUBEDIT_LTO_JOBS.
#
# Un ✓ par invariant, comme check_scripts : la liste des fichiers fautifs ne
# sort que sur échec.
check_cmake() {
    local -a files=()
    local f
    while IFS= read -r -d '' f; do
        files+=("${f}")
    done < <(find "${REPO_ROOT}/cmake" -maxdepth 1 -name '*.cmake' -print0 2>/dev/null)
    [[ -f "${REPO_ROOT}/CMakeLists.txt" ]] && files+=("${REPO_ROOT}/CMakeLists.txt")

    local -a offenders=()
    local relative line trimmed
    for f in "${files[@]}"; do
        relative="${f#"${REPO_ROOT}"/}"

        while IFS= read -r line; do
            trimmed="${line#"${line%%[![:space:]]*}"}"
            [[ -z "${trimmed}" || "${trimmed}" == '#'* ]] && continue

            if [[ "${line}" == *'-flto='* && "${line}" != *'-flto=${SUBEDIT_LTO_JOBS}'* ]]; then
                offenders+=("${relative} : ${trimmed}")
            fi
        done < "${f}"
    done

    if (( ${#offenders[@]} > 0 )); then
        report_failure "cmake : -flto= codé en dur hors de SUBEDIT_LTO_JOBS :
$(printf '    %s\n' "${offenders[@]}")"
    else
        # Établi : aucun de ces fichiers ne code -flto= en dehors de
        # SUBEDIT_LTO_JOBS. Ça ne dit rien de ceux qui ne mentionnent pas le
        # LTO du tout — c'est pourquoi la phrase reste vraie pour eux aussi.
        report_success "cmake : aucun -flto= codé en dur hors de SUBEDIT_LTO_JOBS"
    fi
}

check_makefile
check_scripts
check_cmake

if (( failures > 0 )); then
    printf '\n%s%d contournement(s) de \$(JOBS)%s\n' "${RED}" "${failures}" "${RESET}" >&2
    exit 1
fi
