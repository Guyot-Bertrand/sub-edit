#!/usr/bin/env bash
# Confronte le registre d'exigences aux tests qui les citent.
#
# Deux sens, tous deux nécessaires — voir docs/adr/0014-registre-d-exigences.md :
#
#   couverture — une exigence `implémentée` est citée par au moins un test, et
#                une exigence qui ne l'est pas ne l'est par aucun ;
#   référence  — tout tag en forme d'identifiant désigne une exigence du
#                registre.
#
# Les tags viennent du binaire lui-même, jamais d'une analyse du C++. Catch2 les
# connaît exactement ; un grep sur TEST_CASE les devinerait — il raterait un tag
# produit par une macro et inventerait ceux d'un fichier commenté.

set -euo pipefail

readonly REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"

registry="${REPO_ROOT}/docs/exigences.md"
binary="${REPO_ROOT}/build/dev/bin/subedit_e2e_test"

readonly RED=$'\033[31m'
readonly GREEN=$'\033[32m'
readonly RESET=$'\033[0m'

# Forme d'un identifiant : capitales, segments séparés par un tiret, numéro sur
# deux chiffres en dernier. C'est elle qui distingue `CLI-VERSION-01` d'un tag
# ordinaire comme `e2e` ou `framerate`, sans tenir de liste.
readonly ID_PATTERN='^[A-Z][A-Z0-9]*(-[A-Z0-9]+)*-[0-9][0-9]$'

failures=0

usage() {
    cat >&2 <<USAGE
usage: check-requirements.sh [--registry <fichier>] [--binary <exécutable>]

  --registry  registre à lire      (défaut : docs/exigences.md)
  --binary    binaire à interroger (défaut : build/dev/bin/subedit_e2e_test)
USAGE
    exit 2
}

while (( $# > 0 )); do
    case "$1" in
        --registry) [[ $# -ge 2 ]] || usage; registry="$2"; shift 2 ;;
        --binary)   [[ $# -ge 2 ]] || usage; binary="$2";   shift 2 ;;
        -h|--help)  usage ;;
        *)          printf 'argument inconnu : %s\n' "$1" >&2; usage ;;
    esac
done

report_failure() {
    printf '%s✗%s %s\n' "${RED}" "${RESET}" "$*" >&2
    failures=$((failures + 1))
}

report_success() {
    printf '%s✓%s %s\n' "${GREEN}" "${RESET}" "$*"
}

[[ -f "${registry}" ]] || { printf 'registre introuvable : %s\n' "${registry}" >&2; exit 1; }
[[ -x "${binary}" ]] || {
    printf 'binaire de test introuvable : %s\n' "${binary}" >&2
    printf '  le construire avec : make build\n' >&2
    exit 1
}

# Lit la table du registre et sort « identifiant état », un par ligne.
#
# La comparaison des états se fait par index() et non par une expression
# régulière : les quatre mots portent des accents, et une classe de caractères
# multi-octets ne se comporte pas de la même façon selon que awk est gawk ou
# mawk. Une recherche de sous-chaîne compare des octets, partout pareil.
read_registry() {
    awk -F'|' -v pattern="${ID_PATTERN}" '
        NF < 4 { next }
        {
            id = $2
            gsub(/[`~ \t]/, "", id)
            if (id !~ pattern) next

            state = $(NF - 1)
            if      (index(state, "implémentée")) print id, "implémentée"
            else if (index(state, "prévue"))      print id, "prévue"
            else if (index(state, "abandonnée"))  print id, "abandonnée"
            else if (index(state, "remplacée"))   print id, "remplacée"
            else                                  print id, "inconnu"
        }
    ' "${registry}"
}

# Sort un tag par ligne.
#
# On extrait tous les groupes entre crochets de la sortie entière plutôt que
# d'ancrer sur « N espaces crochet » : Catch2 met sur une même ligne toutes les
# orthographes d'un tag, et replie la ligne à 70 colonnes quand elle dépasse.
# Ni l'en-tête ni le pied de la sortie ne contiennent de crochets.
list_tags() {
    "${binary}" --list-tags | grep -o '\[[^][]*\]' | tr -d '[]' | sort -u
}

declare -A state_of=()
duplicates=()

while read -r id state; do
    if [[ -n "${state_of[${id}]:-}" ]]; then
        duplicates+=("${id}")
    fi
    state_of["${id}"]="${state}"
done < <(read_registry)

if (( ${#state_of[@]} == 0 )); then
    report_failure "aucune exigence lue dans ${registry#"${REPO_ROOT}"/}
    la table est-elle bien formée ?"
    printf '\n%s1 écart entre le registre et les tests%s\n' "${RED}" "${RESET}" >&2
    exit 1
fi

declare -A cited=()
while read -r tag; do
    [[ -n "${tag}" ]] && cited["${tag}"]=1
done < <(list_tags)

# 1 — un identifiant n'est jamais réutilisé, donc jamais écrit deux fois.
if (( ${#duplicates[@]} > 0 )); then
    report_failure "identifiants présents plusieurs fois dans le registre :
$(printf '    %s\n' "${duplicates[@]}")
    un identifiant n'est jamais réutilisé."
else
    report_success "${#state_of[@]} identifiants, tous distincts"
fi

# 2 — chaque état affirme quelque chose de vérifiable sur la présence d'un test.
uncovered=()
unexpected=()
unknown_state=()

for id in "${!state_of[@]}"; do
    case "${state_of[${id}]}" in
        implémentée)
            [[ -n "${cited[${id}]:-}" ]] || uncovered+=("${id}")
            ;;
        prévue|abandonnée|remplacée)
            [[ -z "${cited[${id}]:-}" ]] || unexpected+=("${id} (${state_of[${id}]})")
            ;;
        *)
            unknown_state+=("${id}")
            ;;
    esac
done

if (( ${#unknown_state[@]} > 0 )); then
    report_failure "états non reconnus :
$(printf '    %s\n' "${unknown_state[@]}")
    les quatre états sont : prévue, implémentée, abandonnée, remplacée."
fi

if (( ${#uncovered[@]} > 0 )); then
    report_failure "exigences déclarées implémentées qu'aucun test ne cite :
$(printf '    %s\n' "${uncovered[@]}")
    ajouter le tag à un test de bout en bout, ou corriger l'état."
else
    report_success "toute exigence implémentée est citée par au moins un test"
fi

if (( ${#unexpected[@]} > 0 )); then
    report_failure "exigences non implémentées qu'un test cite pourtant :
$(printf '    %s\n' "${unexpected[@]}")
    un test qui cite une exigence prouve qu'elle est implémentée ;
    tourner l'état, ou retirer le test resté derrière."
else
    report_success "aucune exigence non implémentée n'est citée"
fi

# 3 — tout tag qui a la forme d'un identifiant en désigne un.
dangling=()
for tag in "${!cited[@]}"; do
    [[ "${tag}" =~ ${ID_PATTERN} ]] || continue
    [[ -n "${state_of[${tag}]:-}" ]] || dangling+=("${tag}")
done

if (( ${#dangling[@]} > 0 )); then
    report_failure "tags en forme d'identifiant qui ne désignent aucune exigence :
$(printf '    %s\n' "${dangling[@]}")
    inscrire l'exigence au registre, ou corriger le tag."
else
    report_success "tout tag en forme d'identifiant désigne une exigence"
fi

if (( failures > 0 )); then
    printf '\n%s%d écart(s) entre le registre et les tests%s\n' "${RED}" "${failures}" "${RESET}" >&2
    exit 1
fi
