#!/usr/bin/env bash
# Confronte le registre d'exigences à ce qui l'entoure : les tests qui le citent,
# et les specs de phase qui le promettent.
#
# Trois sens, tous nécessaires — voir docs/adr/0014-registre-d-exigences.md :
#
#   couverture — une exigence `implémentée` est citée par au moins un test, et
#                une exigence qui ne l'est pas ne l'est par aucun ;
#   référence  — tout tag en forme d'identifiant désigne une exigence du
#                registre ;
#   cadrage    — la table d'exigences d'une spec de phase et les lignes du
#                registre portant cette phase se disent l'une l'autre.
#
# Les tags viennent du binaire lui-même, jamais d'une analyse du C++. Catch2 les
# connaît exactement ; un grep sur TEST_CASE les devinerait — il raterait un tag
# produit par une macro et inventerait ceux d'un fichier commenté.
#
# **Le troisième sens est né de la relecture de la phase 7, après deux phases où
# le même défaut est passé.** La phase 16 avait retiré `CLI-SNAP-03` du code sans
# que sa spec suive et laissé naître `GUI-GRID-03` sans l'y inscrire ; la phase 7
# a fondu `GUI-THEME-02` dans `GUI-THEME-01` de la même façon. Deux fois de
# suite, la spec promettait un nombre d'exigences que le registre ne portait
# plus — et la seule chose qui l'a vu est un humain qui relisait les deux
# colonnes côte à côte. Ce que deux relectures ont trouvé, un contrôle le trouve
# à chaque fois.

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

# Lit la table du registre et sort « identifiant état phase barré », un par
# ligne. Les quatre champs sont sans espace, donc lisibles par un `read` nu.
#
# La comparaison des états se fait par index() et non par une expression
# régulière : les quatre mots portent des accents, et une classe de caractères
# multi-octets ne se comporte pas de la même façon selon que awk est gawk ou
# mawk. Une recherche de sous-chaîne compare des octets, partout pareil.
read_registry() {
    awk -F'|' -v pattern="${ID_PATTERN}" '
        NF < 4 { next }
        {
            raw = $2
            id = raw
            gsub(/[`~ \t]/, "", id)
            if (id !~ pattern) next

            phase = $(NF - 2)
            gsub(/[` \t]/, "", phase)

            state = $(NF - 1)
            if      (index(state, "implémentée")) label = "implémentée"
            else if (index(state, "prévue"))      label = "prévue"
            else if (index(state, "abandonnée"))  label = "abandonnée"
            else if (index(state, "remplacée"))   label = "remplacée"
            else                                  label = "inconnu"

            print id, label, phase, (index(raw, "~~") ? "barré" : "nu")
        }
    ' "${registry}"
}

# Lit une spec de phase et sort « identifiant barré », un par ligne.
#
# **Toutes les lignes de tableau du fichier, et non la seule section
# « Exigences ».** Deux raisons. Un identifiant n'apparaît en première cellule
# nulle part ailleurs — c'est vérifié sur les neuf specs —, donc l'ancrage sur un
# titre n'apporte rien ; et un titre renommé ferait taire le contrôle au lieu de
# le faire échouer, ce qui est le pire des deux comportements.
read_spec_ids() {
    awk -F'|' -v pattern="${ID_PATTERN}" '
        /^\|/ {
            raw = $2
            id = raw
            gsub(/[`~ \t]/, "", id)
            if (id !~ pattern) next
            print id, (index(raw, "~~") ? "barré" : "nu")
        }
    ' "$1"
}

declare -A state_of=()
duplicates=()

declare -A phase_of=()
declare -A struck_in_registry=()

while read -r id state phase struck; do
    if [[ -n "${state_of[${id}]:-}" ]]; then
        duplicates+=("${id}")
    fi
    state_of["${id}"]="${state}"
    phase_of["${id}"]="${phase}"
    struck_in_registry["${id}"]="${struck}"
done < <(read_registry)

if (( ${#state_of[@]} == 0 )); then
    report_failure "aucune exigence lue dans ${registry#"${REPO_ROOT}"/}
    la table est-elle bien formée ?"
    printf '\n%s1 écart entre le registre, les specs et les tests%s\n' "${RED}" "${RESET}" >&2
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

# 4 — la table d'exigences d'une spec de phase dit ce que le registre porte pour
#     cette phase, et rien d'autre.
#
# **Le sens qui manquait, et qui a laissé passer deux phases.** Le registre est
# vivant — une exigence y naît, se barre, change d'état ; la spec, elle, est
# écrite une fois au cadrage et personne ne la rouvre. Sans confrontation, elle
# reste sur ce que la phase croyait promettre et non sur ce qu'elle promet.
#
# La phase se lit dans le nom du fichier — `07-finitions….md` porte la phase 7 —
# et non dans son contenu : c'est le seul endroit où elle ne peut pas se
# tromper. Une spec sans le moindre identifiant est passée : les phases 0 à 2
# n'ont pas d'exigences, et le registre n'en porte aucune pour elles, ce que le
# sens inverse vérifie de toute façon.
specs_dir="${REPO_ROOT}/docs/specs"
spec_absent=()
spec_extra=()
spec_wrong_phase=()
spec_wrong_strike=()
registry_wrong_strike=()
spec_count=0

# Le barré du registre et son état disent la même chose, et il faut que ce soit
# vrai avant de s'en servir pour juger les specs : c'est l'état qui fait foi
# ci-dessous, et un registre qui barrerait au hasard ferait accuser la spec.
for id in "${!state_of[@]}"; do
    expected="nu"
    case "${state_of[${id}]}" in
        abandonnée|remplacée) expected="barré" ;;
        *) ;;
    esac
    [[ "${struck_in_registry[${id}]}" == "${expected}" ]] \
        || registry_wrong_strike+=("${id} (${state_of[${id}]}, écrit ${struck_in_registry[${id}]})")
done

if (( ${#registry_wrong_strike[@]} > 0 )); then
    report_failure "au registre, le barré et l'état se contredisent :
$(printf '    %s\n' "${registry_wrong_strike[@]}")
    une exigence abandonnée ou remplacée est barrée, les deux autres états ne le sont pas."
fi

if [[ -d "${specs_dir}" ]]; then
    declare -A spec_of_phase=()

    for spec in "${specs_dir}"/[0-9][0-9]-*.md; do
        [[ -e "${spec}" ]] || continue
        base="$(basename "${spec}")"
        # 10# force la base dix : « 07 » sans lui serait lu en octal, et « 08 »
        # ne serait pas lu du tout.
        phase="$(( 10#${base%%-*} ))"
        spec_of_phase["${phase}"]="${base}"

        declare -A in_spec=()
        while read -r id struck; do
            [[ -n "${id}" ]] || continue
            in_spec["${id}"]="${struck}"
            spec_count=$((spec_count + 1))

            if [[ -z "${state_of[${id}]:-}" ]]; then
                spec_extra+=("${id} (${base})")
                continue
            fi
            if [[ "${phase_of[${id}]}" != "${phase}" ]]; then
                spec_wrong_phase+=("${id} (${base}, registre : phase ${phase_of[${id}]})")
                continue
            fi
            expected="nu"
            case "${state_of[${id}]}" in
                abandonnée|remplacée) expected="barré" ;;
                *) ;;
            esac
            [[ "${struck}" == "${expected}" ]] \
                || spec_wrong_strike+=("${id} (${base}, ${state_of[${id}]}, écrit ${struck})")
        done < <(read_spec_ids "${spec}")

        for id in "${!state_of[@]}"; do
            [[ "${phase_of[${id}]}" == "${phase}" ]] || continue
            [[ -n "${in_spec[${id}]:-}" ]] || spec_absent+=("${id} (${base})")
        done

        unset in_spec
    done

    # Une phase que le registre nomme et qu'aucune spec ne porte : soit la phase
    # est écrite à l'envers, soit la colonne porte une coquille. Les deux se
    # voient ici et nulle part ailleurs.
    orphan_phases=()
    for id in "${!state_of[@]}"; do
        phase="${phase_of[${id}]}"
        [[ -n "${spec_of_phase[${phase}]:-}" ]] || orphan_phases+=("${id} (phase ${phase})")
    done

    if (( ${#orphan_phases[@]} > 0 )); then
        report_failure "exigences dont la phase ne correspond à aucune spec :
$(printf '    %s\n' "${orphan_phases[@]}")
    corriger la colonne « Phase », ou écrire la spec de la phase."
    fi
fi

if (( ${#spec_extra[@]} > 0 )); then
    report_failure "exigences promises par une spec et absentes du registre :
$(printf '    %s\n' "${spec_extra[@]}")
    inscrire l'exigence au registre, ou la retirer de la spec."
fi

if (( ${#spec_absent[@]} > 0 )); then
    report_failure "exigences du registre absentes de la table de leur spec :
$(printf '    %s\n' "${spec_absent[@]}")
    ajouter la ligne à la spec de la phase — c'est elle qui promet."
fi

if (( ${#spec_wrong_phase[@]} > 0 )); then
    report_failure "exigences dont la spec et le registre ne disent pas la même phase :
$(printf '    %s\n' "${spec_wrong_phase[@]}")"
fi

if (( ${#spec_wrong_strike[@]} > 0 )); then
    report_failure "exigences dont le barré de la spec contredit l'état du registre :
$(printf '    %s\n' "${spec_wrong_strike[@]}")
    une exigence abandonnée ou remplacée est barrée des deux côtés."
fi

if (( ${#spec_extra[@]} + ${#spec_absent[@]} + ${#spec_wrong_phase[@]} + ${#spec_wrong_strike[@]} == 0 )); then
    report_success "${spec_count} exigences de spec, toutes accordées au registre"
fi

if (( failures > 0 )); then
    printf '\n%s%d écart(s) entre le registre, les specs et les tests%s\n' \
        "${RED}" "${failures}" "${RESET}" >&2
    exit 1
fi
