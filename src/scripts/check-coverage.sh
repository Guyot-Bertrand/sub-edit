#!/usr/bin/env bash
# Confronte le nombre de lignes non couvertes au cliquet, et l'enregistre.
#
# Le cliquet porte sur un COMPTE DE LIGNES, pas sur un pourcentage : un
# pourcentage monte dès qu'on ajoute du code bien testé, ce qui resserre le
# cliquet sans que personne l'ait décidé. Voir docs/adr/0015-memoire-des-mesures.md.
#
# Le taux `line_percent` du résumé gcovr arrive DÉJÀ ARRONDI à une décimale ;
# on ne s'en sert jamais pour comparer. Seuls `line_total` et `line_covered`
# font foi.
#
# python3 est utilisé pour lire le JSON : c'est une dépendance déjà acquise,
# gcovr lui-même en est écrit.

set -euo pipefail

readonly REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
readonly RATCHET="${REPO_ROOT}/docs/mesures/couverture.md"

readonly RED=$'\033[31m'
readonly GREEN=$'\033[32m'
readonly RESET=$'\033[0m'

summary=""
record=0

# Nettoyage du fichier temporaire d'écriture atomique du cliquet, quelle que
# soit la façon dont le script se termine — succès, échec, interruption.
# tmp_ratchet reste vide tant que write_ratchet n'a rien créé ; une fois le
# renommage fait, le fichier n'existe plus et le rm -f ci-dessous ne fait
# rien. Le pendant, côté record-bench.sh, s'appuie sur un try/except Python ;
# ici, un trap EXIT joue le même rôle avec les moyens du bash.
#
# La fonction se termine toujours par `return 0` : un trap EXIT dont la
# dernière commande échoue (ici, le test `[[ ]]` quand tmp_ratchet est vide)
# écrase silencieusement le code de sortie du script avec le sien — un ✓
# affiché rendrait alors un exit 1, sans rapport avec ce que le script vient
# de faire.
tmp_ratchet=""
cleanup_tmp_ratchet() {
    [[ -n "${tmp_ratchet}" && -e "${tmp_ratchet}" ]] && rm -f "${tmp_ratchet}"
    return 0
}
trap cleanup_tmp_ratchet EXIT

usage() {
    cat >&2 <<'USAGE'
usage: check-coverage.sh --summary <resume.json> [--record]

  --summary  le résumé JSON produit par gcovr --json-summary
  --record   réécrit docs/mesures/couverture.md depuis ce résumé
USAGE
    exit 2
}

while (( $# > 0 )); do
    case "$1" in
        --summary) [[ $# -ge 2 ]] || usage; summary="$2"; shift 2 ;;
        --record)  record=1; shift ;;
        -h|--help) usage ;;
        *) printf 'argument inconnu : %s\n' "$1" >&2; usage ;;
    esac
done

[[ -n "${summary}" ]] || usage
[[ -f "${summary}" ]] || {
    printf 'résumé de couverture introuvable : %s\n' "${summary}" >&2
    printf '  le produire avec : make coverage\n' >&2
    exit 1
}
[[ -f "${RATCHET}" ]] || {
    printf 'cliquet introuvable : %s\n' "${RATCHET}" >&2
    exit 1
}

# --record lit build/coverage-report/summary.json sans jamais le régénérer
# (voir la recette « ratchet » du Makefile) : c'est un artefact que ce script
# ne construit pas et dont il ne possède pas la fraîcheur. Un résumé plus
# ancien que la plus récente source de src/lib/ est la trace d'un « make
# ratchet » lancé sans « make coverage » préalable, ou d'un résidu laissé par
# un autre outil (verify-gates.sh en injectait un jusqu'à ce qu'il nettoie
# derrière lui) — l'enregistrer serait figer une mesure qui ne correspond plus
# au code.
if (( record == 1 )); then
    stale_source="$(find "${REPO_ROOT}/src/lib" -type f -newer "${summary}" -print -quit)"
    if [[ -n "${stale_source}" ]]; then
        printf '%s✗%s résumé de couverture périmé : %s est plus récent que %s\n' \
            "${RED}" "${RESET}" "${stale_source#"${REPO_ROOT}"/}" "${summary}" >&2
        printf '  le régénérer avec : make coverage\n' >&2
        exit 1
    fi
fi

# Le cliquet a deux ancres textuelles : le titre « ## Relevé » (frontière entre
# le préambule et la section réécrite) et les lignes « | compte | `fichier` |
# » de sa table. Une ancre absente ou dupliquée — titre renommé à la main,
# fusion mal résolue — ne fait planter ni le sed qui la cherche ni le python
# qui compare : le premier ne coupe rien, le second continue sur une table
# tronquée. Le vérifier une fois ici, avant toute lecture ou écriture, vaut
# mieux qu'un ✓ sur un fichier déjà incohérent.
heading_count="$(grep -c '^## Relevé$' "${RATCHET}" || true)"
if [[ "${heading_count}" -ne 1 ]]; then
    printf '%s✗%s le titre « ## Relevé » de %s apparaît %s fois, une seule attendue\n' \
        "${RED}" "${RESET}" "${RATCHET#"${REPO_ROOT}"/}" "${heading_count}" >&2
    printf '  corriger le fichier à la main avant de relancer.\n' >&2
    exit 1
fi
if grep -qE '^(<{7} |={7}$|>{7} )' "${RATCHET}"; then
    printf '%s✗%s marqueur de conflit git détecté dans %s\n' \
        "${RED}" "${RESET}" "${RATCHET#"${REPO_ROOT}"/}" >&2
    printf '  résoudre la fusion à la main avant de relancer.\n' >&2
    exit 1
fi

# Sort « total » puis une ligne « compte<TAB>fichier » par fichier concerné,
# du plus fourni au moins fourni. Sort en échec (code non nul) si le JSON est
# illisible, incomplet, ou dégénéré — line_total à zéro ou files vide, ce
# qu'un --filter erroné ou un build sans .gcda produirait, et qui ne constitue
# un état légitime pour aucun résumé de ce projet.
read_summary() {
    python3 - "$1" <<'PY'
import json, sys

with open(sys.argv[1], encoding="utf-8") as handle:
    data = json.load(handle)

line_total = data["line_total"]
line_covered = data["line_covered"]
files = data.get("files", [])

if line_total <= 0 or not files:
    sys.exit(1)

print(line_total - line_covered)
rows = [
    (entry["line_total"] - entry["line_covered"], entry["filename"])
    for entry in files
    if entry["line_total"] > entry["line_covered"]
]
for count, name in sorted(rows, key=lambda row: (-row[0], row[1])):
    print(f"{count}\t{name}")
PY
}

# La substitution de processus (mapfile < <(...)) n'est pas couverte par
# pipefail : un python3 en échec y serait invisible, et la porte annoncerait
# un succès sur une lecture tronquée. La substitution de commande, elle, l'est
# — measured_raw="$(...)" transmet fidèlement le code de sortie de
# read_summary, capturé ici par le test explicite plutôt que par set -e, pour
# pouvoir répondre par un ✗ exploitable au lieu d'une pile Python.
if ! measured_raw="$(read_summary "${summary}" 2>/dev/null)"; then
    printf '%s✗%s résumé de couverture illisible ou vide : %s\n' \
        "${RED}" "${RESET}" "${summary}" >&2
    printf '  le régénérer avec : make coverage\n' >&2
    exit 1
fi
mapfile -t measured <<< "${measured_raw}"
readonly measured_total="${measured[0]}"

# gcovr 7.0 émet des entiers pour line_total et line_covered, mais le résumé
# JSON ne le garantit pas : une version future qui les émettrait en flottant
# ferait échouer `(( ))` plus bas par une erreur de syntaxe arithmétique — une
# erreur que `if (( ... ))` avale silencieusement (test faux, script non
# interrompu), au point de retomber dans la branche verte finale avec un total
# mesuré comme « 53.0 » affiché tel quel. Un compte de lignes n'est légitime
# que sous forme d'entier ; le vérifier ici transforme un échec silencieux en ✗.
[[ "${measured_total}" =~ ^[0-9]+$ ]] || {
    printf '%s✗%s total de couverture non entier : %s\n' \
        "${RED}" "${RESET}" "${measured_total}" >&2
    printf '  résumé suspect : %s\n' "${summary}" >&2
    exit 1
}

# L'ancre du cliquet : une ligne « total : N » dans un bloc de code.
recorded_total="$(sed -n 's/^[[:space:]]\+total[[:space:]]*:[[:space:]]*\([0-9]\+\)[[:space:]]*$/\1/p' \
    "${RATCHET}" | head -1)"

[[ -n "${recorded_total}" ]] || {
    printf '%s✗%s aucune ligne « total : N » lisible dans %s\n' \
        "${RED}" "${RESET}" "${RATCHET#"${REPO_ROOT}"/}" >&2
    exit 1
}

# Le motif ci-dessus ne capture que des chiffres, donc recorded_total est déjà
# forcément un entier s'il a matché — ce test documente l'invariant et protège
# symétriquement contre un futur assouplissement du motif.
[[ "${recorded_total}" =~ ^[0-9]+$ ]] || {
    printf '%s✗%s total relevé non entier : %s dans %s\n' \
        "${RED}" "${RESET}" "${recorded_total}" "${RATCHET#"${REPO_ROOT}"/}" >&2
    exit 1
}

# Les lignes de données de la table actuelle (« | compte | `fichier` | »), en
# excluant l'en-tête et la ligne de séparation qui ne matchent pas ce motif.
mapfile -t recorded_rows < <(sed -n 's/^| *\([0-9][0-9]*\) *| `\([^`]*\)` *|$/\1\t\2/p' "${RATCHET}")

write_ratchet() {
    local -a new_rows=()
    local index
    for (( index = 1; index < ${#measured[@]}; index++ )); do
        new_rows+=("${measured[index]}")
    done

    # Idempotence : si le total et la ventilation n'ont pas bougé, ne rien
    # réécrire. Sans ce test, write_ratchet reposait la date du jour à chaque
    # appel même sur une mesure inchangée — un fichier versionné qui bouge
    # sans qu'on ait rien à y dire.
    local unchanged=1
    if [[ "${measured_total}" != "${recorded_total}" ]] \
        || [[ "${#new_rows[@]}" -ne "${#recorded_rows[@]}" ]]; then
        unchanged=0
    else
        for (( index = 0; index < ${#new_rows[@]}; index++ )); do
            if [[ "${new_rows[index]}" != "${recorded_rows[index]}" ]]; then
                unchanged=0
                break
            fi
        done
    fi

    if (( unchanged == 1 )); then
        printf '%s✓%s cliquet déjà à jour : rien à réenregistrer\n' \
            "${GREEN}" "${RESET}"
        return 0
    fi

    local version
    version="$(sed -n 's/^[[:space:]]*VERSION[[:space:]]\+\([0-9][0-9.]*\)[[:space:]]*$/\1/p' \
        "${REPO_ROOT}/CMakeLists.txt" | head -1)"
    [[ -n "${version}" ]] || {
        printf '%s✗%s version illisible dans %s/CMakeLists.txt\n' \
            "${RED}" "${RESET}" "${REPO_ROOT}" >&2
        exit 1
    }

    local today
    today="$(date +%Y-%m-%d)"

    # Écriture atomique : un fichier temporaire à côté de la cible, renommé
    # ensuite — même principe que record-bench.sh. mktemp le crée en 0600 ;
    # on aligne son mode sur celui du fichier original avant de le publier,
    # sans quoi le renommage remplacerait un fichier suivi en 644 par un
    # fichier illisible du groupe et des autres. tmp_ratchet est nettoyé par
    # le trap EXIT posé en tête de script si l'écriture est interrompue.
    local original_mode
    original_mode="$(stat -c '%a' "${RATCHET}")"
    tmp_ratchet="$(mktemp "${RATCHET}.XXXXXX")"

    {
        # Le préambule est conservé tel quel : seule la section « Relevé » est
        # réécrite. On coupe au titre de section pour ne pas perdre la prose.
        sed '/^## Relevé$/,$d' "${RATCHET}"
        printf '## Relevé\n\n    total : %s\n\n' "${measured_total}"
        printf 'Relevé sur la version %s, le %s.\n\n' "${version}" "${today}"
        printf '| Lignes | Fichier |\n| -----: | :------ |\n'
        local row
        for row in "${new_rows[@]}"; do
            printf '| %s | `%s` |\n' "${row%%$'\t'*}" "${row#*$'\t'}"
        done
    } > "${tmp_ratchet}"
    chmod "${original_mode}" "${tmp_ratchet}"
    mv "${tmp_ratchet}" "${RATCHET}"
    tmp_ratchet=""
    printf '%s✓%s cliquet enregistré : %s ligne(s) non couverte(s)\n' \
        "${GREEN}" "${RESET}" "${measured_total}"
}

if (( record == 1 )); then
    write_ratchet
    exit 0
fi

if (( measured_total > recorded_total )); then
    printf '%s✗%s la couverture a reculé : %s ligne(s) non couverte(s), contre %s au relevé\n' \
        "${RED}" "${RESET}" "${measured_total}" "${recorded_total}" >&2
    printf '\n  ventilation actuelle :\n' >&2
    for (( index = 1; index < ${#measured[@]}; index++ )); do
        printf '    %4s  %s\n' \
            "${measured[index]%%$'\t'*}" "${measured[index]#*$'\t'}" >&2
    done
    printf '\n  comparer à la table de %s pour voir ce qui a bougé.\n' \
        "${RATCHET#"${REPO_ROOT}"/}" >&2
    printf '  couvrir les lignes ajoutées, ou justifier le relèvement du cliquet.\n' >&2
    exit 1
fi

if (( measured_total < recorded_total )); then
    printf '%s✓%s la couverture progresse : %s ligne(s) non couverte(s), contre %s au relevé\n' \
        "${GREEN}" "${RESET}" "${measured_total}" "${recorded_total}"
    printf '  « make ratchet » enregistre ce progrès.\n'
    exit 0
fi

printf '%s✓%s %s ligne(s) non couverte(s), comme au relevé\n' \
    "${GREEN}" "${RESET}" "${measured_total}"
