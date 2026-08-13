#!/usr/bin/env bash
# Applique la configuration GitHub automatisable du dépôt.
#
# Couvre les labels, les milestones et les rulesets décrits dans
# docs/configuration-github.md. Les réglages qui n'ont pas d'API — limites
# d'interaction, approbation des workflows de fork — restent à faire dans
# l'interface ; le document les liste.
#
# Idempotent : relançable sans effet de bord.
#
# Nécessite gh authentifié avec les droits d'administration du dépôt.

set -euo pipefail

readonly REPO="${SUBEDIT_REPO:-Guyot-Bertrand/sub-edit}"

readonly BOLD=$'\033[1m'
readonly GREEN=$'\033[32m'
readonly RED=$'\033[31m'
readonly RESET=$'\033[0m'

ruleset_failures=0

info() { printf '%s▸ %s%s\n' "${BOLD}" "$*" "${RESET}"; }
done_() { printf '  %s✓%s %s\n' "${GREEN}" "${RESET}" "$*"; }

require_gh() {
    command -v gh >/dev/null 2>&1 || {
        printf 'gh est requis : ./src/scripts/setup-toolchain.sh\n' >&2
        exit 1
    }
    gh auth status >/dev/null 2>&1 || {
        printf 'gh non authentifié : gh auth login, ou exporter GH_TOKEN\n' >&2
        exit 1
    }
}

# --- Labels -----------------------------------------------------------------
#
# Les valeurs de `area:` sont exactement les scopes des messages de commit, de
# sorte qu'une issue et les commits qui la traitent portent le même vocabulaire.

apply_labels() {
    info "labels"

    local -a labels=(
        "type:bug|d73a4a|Comportement incorrect et reproductible"
        "type:story|0e8a16|Fonctionnalité vue du point de vue de l'utilisateur"
        "type:enhancement|1d76db|Évolution d'un comportement existant"
        "type:task|5319e7|Travail technique sans effet direct pour l'utilisateur"
        "type:doc|006b75|Documentation, spec, manuel ou ADR"

        "area:build|c5def5|Système de construction et chaîne d'outils"
        "area:ci|c5def5|Intégration continue"
        "area:cli|c5def5|Exécutable en ligne de commande"
        "area:core|c5def5|Modèle de données et opérations"
        "area:doc|c5def5|Documentation"
        "area:format|c5def5|Lecture et écriture des formats de sous-titres"
        "area:gui|c5def5|Interface graphique Qt"
        "area:i18n|c5def5|Internationalisation"
        "area:scripts|c5def5|Scripts d'automatisation"
        "area:test|c5def5|Tests et benchmarks"
        "area:text|c5def5|Correction et mise en forme du texte"
        "area:video|c5def5|Lecteur vidéo et calage"

        "prio:high|b60205|À traiter en priorité"
        "prio:medium|fbca04|Priorité normale"
        "prio:low|0e8a16|Peut attendre"

        "blocked|000000|En attente d'un travail ou d'une décision"
        "needs-decision|d4c5f9|Nécessite un arbitrage avant de continuer"
    )

    local entry name color description
    for entry in "${labels[@]}"; do
        IFS='|' read -r name color description <<< "${entry}"
        gh label create "${name}" --repo "${REPO}" --color "${color}" \
            --description "${description}" --force >/dev/null
    done
    done_ "${#labels[@]} labels créés ou mis à jour"

    # Les labels par défaut de GitHub font doublon avec la taxonomie `type:` et
    # invitent à des contributions que le projet n'accepte pas.
    local -a defaults=(
        bug documentation duplicate enhancement
        "good first issue" "help wanted" invalid question wontfix
    )
    local removed=0
    local label
    for label in "${defaults[@]}"; do
        if gh label delete "${label}" --repo "${REPO}" --yes >/dev/null 2>&1; then
            removed=$((removed + 1))
        fi
    done
    (( removed == 0 )) || done_ "${removed} labels par défaut supprimés"
}

# --- Milestones -------------------------------------------------------------

apply_milestones() {
    info "milestones"

    # Les phases 0 à 7 mènent au MVP, les suivantes complètent
    # l'iso-fonctionnalité. Voir docs/feuille-de-route.md.
    local -a phases=(
        "0 — Fondations|Structure, chaîne de construction et filet de vérification"
        "1 — Noyau : modèle et formats|Sous-titre, positions, SubRip et WebVTT, balises, commandes réversibles"
        "2 — Opérations d'édition|Édition de base, décalage, transformation, conversion de fréquence d'image"
        "3 — CLI : harnais|Inspection, conversion et décalage, pour valider et mesurer le noyau sans interface"
        "4 — Mentions pour malentendants|Suppression des sons entre crochets et entre parenthèses"
        "5 — Interface : édition tabulaire|Fenêtre, table éditable, ouvrir, enregistrer sous, annuler et refaire"
        "6 — Prévisualisation|Lancement d'un lecteur externe positionné au sous-titre courant"
        "7 — Finitions et première livraison|Préférences, thème, manuel, empaquetage — le MVP est livrable"
        "8 — Encodages et fins de ligne|Détection automatique, jeu complet d'encodages et de fins de ligne"
        "9 — Formats complémentaires|SubViewer 2, SSA, ASS, MicroDVD, MPL2, TMPlayer, LRC et balises riches"
        "10 — Opérations complémentaires|Durées, casse, italiques, tirets, fusion, scission, recherche"
        "11 — Traduction et multi-projets|Second document, alignement, onglets, opérations groupées"
        "12 — Moteur de correction complet|Motifs par langue, découpage de lignes, correcteur orthographique"
        "13 — CLI complète|Sous-commandes destinées à un usage réel et traitement par lot"
        "14 — Lecteur vidéo intégré et calage|Incrustation, calage à l'image près depuis la position vidéo"
        "15 — Internationalisation|Reprise des vingt locales de Gaupol"
    )

    # Le rapprochement se fait sur le numéro de phase et non sur le titre
    # complet, afin qu'un libellé révisé renomme le milestone au lieu d'en
    # créer un second.
    local existing
    existing="$(gh api "repos/${REPO}/milestones?state=all" --jq '.[] | "\(.number)|\(.title)"')"

    local phase title description number created=0 updated=0
    for phase in "${phases[@]}"; do
        IFS='|' read -r title description <<< "${phase}"
        number="$(awk -F'|' -v p="${title%% *} —" \
            'index($2, p) == 1 { print $1; exit }' <<< "${existing}")"

        if [[ -n "${number}" ]]; then
            gh api "repos/${REPO}/milestones/${number}" --method PATCH \
                --field title="${title}" \
                --field description="${description}" >/dev/null
            updated=$((updated + 1))
        else
            gh api "repos/${REPO}/milestones" --method POST \
                --field title="${title}" \
                --field description="${description}" >/dev/null
            created=$((created + 1))
        fi
    done

    done_ "${created} milestones créés, ${updated} mis à jour"
}

# --- Rulesets ---------------------------------------------------------------
#
# Deux rulesets plutôt qu'un seul, parce qu'un ruleset n'a qu'une liste de
# dérogation pour toutes ses règles :
#
#   - « protection de l'historique » ne déroge à personne. Le propriétaire est
#     seul à écrire ; le risque réel n'est donc pas un tiers malveillant mais
#     un force-push accidentel de sa part. Il faut que la règle s'applique
#     aussi à lui pour avoir un sens.
#   - « porte de qualité » déroge à l'administrateur, ce qui préserve le push
#     direct sur main. Sans cette dérogation, la règle bloquerait tout push,
#     puisque la CI ne s'exécute qu'après.
#
# Le ruleset n'exige qu'un seul contexte, « porte de qualité ». Le job
# « contrôles de pull request », livré par #51, en est délibérément absent : il
# est jeune, et un contrôle jeune qui bloque une fusion coûte plus cher qu'il ne
# rapporte tant qu'on n'a pas vu s'il produit des faux positifs. Il échoue en
# rouge, ce qui suffit à le rendre visible pendant l'observation. À rediscuter à
# l'ouverture de la phase 4. Le job « messages de commit » est dans le même cas,
# et pour la même raison.

ruleset_exists() {
    gh api "repos/${REPO}/rulesets" --jq ".[] | select(.name == \"$1\") | .id" 2>/dev/null
}

create_ruleset() {
    local name="$1"
    local payload="$2"

    local id
    id="$(ruleset_exists "${name}")"

    local method="POST"
    local endpoint="repos/${REPO}/rulesets"
    if [[ -n "${id}" ]]; then
        method="PUT"
        endpoint="repos/${REPO}/rulesets/${id}"
    fi

    # Écrire un ruleset relève de la permission « Administration », que les
    # jetons à granularité fine n'accordent pas par défaut. L'échec est
    # signalé, pas fatal : le reste de la configuration a déjà été appliqué.
    if gh api "${endpoint}" --method "${method}" --input - <<< "${payload}" >/dev/null 2>&1; then
        done_ "« ${name} » $([[ -n "${id}" ]] && echo "mis à jour" || echo "créé")"
        return
    fi

    ruleset_failures=$((ruleset_failures + 1))
    printf '  %s✗%s « %s » — écriture refusée\n' "${RED}" "${RESET}" "${name}"
}

apply_rulesets() {
    info "rulesets sur la branche par défaut"

    create_ruleset "protection de l'historique" '{
        "name": "protection de l'\''historique",
        "target": "branch",
        "enforcement": "active",
        "conditions": {"ref_name": {"include": ["~DEFAULT_BRANCH"], "exclude": []}},
        "rules": [{"type": "deletion"}, {"type": "non_fast_forward"}]
    }'

    create_ruleset "porte de qualité" '{
        "name": "porte de qualité",
        "target": "branch",
        "enforcement": "active",
        "bypass_actors": [
            {"actor_id": 5, "actor_type": "RepositoryRole", "bypass_mode": "always"}
        ],
        "conditions": {"ref_name": {"include": ["~DEFAULT_BRANCH"], "exclude": []}},
        "rules": [{
            "type": "required_status_checks",
            "parameters": {
                "strict_required_status_checks_policy": false,
                "required_status_checks": [{"context": "porte de qualité"}]
            }
        }]
    }'
}

report_manual_steps() {
    info "à faire dans l'interface — sans API ou hors portée du jeton"

    if (( ruleset_failures > 0 )); then
        printf '  Les rulesets exigent la permission « Administration » du jeton.\n'
        printf '  Deux voies :\n'
        printf '    ajouter Administration: Read and write au jeton, puis relancer ce script\n'
        printf '    ou les créer à la main : Settings → Rules → Rulesets\n\n'
    fi

    printf '  Settings → Actions → Fork pull request workflows\n'
    printf '      → Require approval for all external contributors\n'
    printf '  Settings → Moderation → Interaction limits\n'
    printf '      → Limit to prior contributors, à renouveler tous les six mois\n'
    printf '  voir docs/configuration-github.md\n'
}

require_gh
apply_labels
apply_milestones
apply_rulesets
report_manual_steps
