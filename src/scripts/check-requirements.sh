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

# **Plusieurs binaires, et non un seul.** Une exigence de ligne de commande se
# prouve en lançant le binaire et en lisant ce qu'il écrit ; une exigence
# d'interface se prouve en construisant la fenêtre dans le processus du test et
# en la pilotant. Les deux sont « ce que le binaire montre » au sens du registre,
# mais elles ne peuvent pas vivre dans le même exécutable : le harnais de bout en
# bout ne lie aucune bibliothèque de ce qu'il éprouve, et c'est ce qui fait sa
# valeur.
#
# Le contrôle réunit donc les tags de tous les binaires qu'on lui nomme.
binaries=()

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
  --binary    binaire à interroger, répétable
              (défaut : les harnais de bout en bout et d'interface)
USAGE
    exit 2
}

while (( $# > 0 )); do
    case "$1" in
        --registry) [[ $# -ge 2 ]] || usage; registry="$2"; shift 2 ;;
        --binary)   [[ $# -ge 2 ]] || usage; binaries+=("$2"); shift 2 ;;
        -h|--help)  usage ;;
        *)          printf 'argument inconnu : %s\n' "$1" >&2; usage ;;
    esac
done

# Le défaut ne peut être posé qu'ici : une valeur mise avant l'analyse des
# arguments s'ajouterait à celles que l'appelant donne au lieu de leur céder la
# place.
if (( ${#binaries[@]} == 0 )); then
    binaries=("${REPO_ROOT}/build/dev/bin/subedit_e2e_test"
              "${REPO_ROOT}/build/dev/bin/subedit_gui_test")
fi

report_failure() {
    printf '%s✗%s %s\n' "${RED}" "${RESET}" "$*" >&2
    failures=$((failures + 1))
}

report_success() {
    printf '%s✓%s %s\n' "${GREEN}" "${RESET}" "$*"
}

[[ -f "${registry}" ]] || { printf 'registre introuvable : %s\n' "${registry}" >&2; exit 1; }
for binary in "${binaries[@]}"; do
    [[ -x "${binary}" ]] || {
        printf 'binaire de test introuvable : %s\n' "${binary}" >&2
        printf '  le construire avec : make build\n' >&2
        exit 1
    }
done

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

# Interroge le binaire et capture sa sortie et son code de retour à part,
# avant tout traitement par grep/tr/sort.
#
# Un pipeline placé dans une substitution de processus (`< <(cmd | grep …)`)
# n'est pas vu par `set -e` : un échec du binaire — plantage, option
# --list-tags disparue, sortie Catch2 qui change de forme — serait sinon
# avalé en silence, et `cited` resterait vide sans qu'aucune ligne ne le
# signale. Un contrôleur qui répond « tout va bien » quand il n'a rien pu
# lire est pire qu'aucun contrôleur.
#
# stdout et stderr sont capturés à part, dans un fichier temporaire pour ce
# dernier : les fondre avec `2>&1` ferait extraire des tags depuis les
# diagnostics du binaire — un `[IDENTIFIANT]` mentionné dans un message
# d'erreur serait alors pris pour une citation réelle, dans un sens comme
# dans l'autre. stderr reste néanmoins lu et affiché en cas d'échec, pour
# que le diagnostic d'un plantage reste visible.
list_tags_stderr="$(mktemp)"
trap 'rm -f "${list_tags_stderr}"' EXIT

list_tags_stdout=""
list_tags_status=0
for binary in "${binaries[@]}"; do
    one=""
    one="$("${binary}" --list-tags 2>"${list_tags_stderr}")" || {
        report_failure "le binaire de test a échoué sur --list-tags :
$(sed 's/^/    /' "${list_tags_stderr}")
    vérifier que ${binary} répond à --list-tags, et que Catch2 n'a pas changé sa sortie."
        list_tags_status=1
        continue
    }
    list_tags_stdout+="${one}"$'\n'
done

if (( list_tags_status != 0 )); then
    :
else
    # On extrait tous les groupes entre crochets de la sortie standard
    # entière plutôt que d'ancrer sur « N espaces crochet » : Catch2 met sur
    # une même ligne toutes les orthographes d'un tag, et replie la ligne à
    # 70 colonnes quand elle dépasse. Ni l'en-tête ni le pied de la sortie ne
    # contiennent de crochets. Le « || true » évite qu'un grep sans
    # correspondance — sortie sans le moindre tag — ne fasse échouer
    # silencieusement la substitution de processus : c'est au garde qui suit
    # de le dire, pas à `set -e` de l'avaler.
    while read -r tag; do
        [[ -n "${tag}" ]] && cited["${tag}"]=1
    done < <(printf '%s' "${list_tags_stdout}" | grep -o '\[[^][]*\]' | tr -d '[]' | sort -u || true)

    # Symétrique au garde sur ${#state_of[@]} plus haut : zéro tag n'est
    # jamais légitime ici, le binaire de test porte toujours au moins [e2e].
    if (( ${#cited[@]} == 0 )); then
        report_failure "aucun tag lu dans la sortie standard des binaires interrogés
    la forme de la sortie de Catch2 a-t-elle changé ? le binaire de test porte
    toujours au moins le tag [e2e] ; son absence signale un défaut, jamais un
    état normal."
    fi
fi

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
