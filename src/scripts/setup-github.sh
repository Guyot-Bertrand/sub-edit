#!/usr/bin/env bash
# Applies the repository's GitHub configuration that can be automated.
#
# Covers the labels, milestones and rulesets described in
# docs/configuration-github.md. The settings that have no API — interaction
# limits, approval of fork workflows — remain to be done in the interface; the
# document lists them.
#
# Idempotent: can be rerun with no side effect.
#
# Requires gh authenticated with administration rights on the repository.
#
# **What is a GitHub identifier stays as it is**: the label names, the
# milestone titles and the ruleset name are what exists on GitHub, and some are
# French. Changing a milestone title here would rename the milestone; changing
# the ruleset's name would create a second one.

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
        printf 'gh is required: ./src/scripts/setup-toolchain.sh\n' >&2
        exit 1
    }
    gh auth status >/dev/null 2>&1 || {
        printf 'gh is not authenticated: gh auth login, or export GH_TOKEN\n' >&2
        exit 1
    }
}

# --- Labels -----------------------------------------------------------------
#
# The values of `area:` are exactly the scopes of commit messages, so that an
# issue and the commits that address it carry the same vocabulary.

apply_labels() {
    info "labels"

    local -a labels=(
        "type:bug|d73a4a|Incorrect and reproducible behaviour"
        "type:story|0e8a16|Feature seen from the user's point of view"
        "type:enhancement|1d76db|Change to an existing behaviour"
        "type:task|5319e7|Technical work with no direct effect for the user"
        "type:doc|006b75|Documentation, spec, manual or ADR"

        "area:build|c5def5|Build system and toolchain"
        "area:ci|c5def5|Continuous integration"
        "area:cli|c5def5|Command-line executable"
        "area:core|c5def5|Data model and operations"
        "area:doc|c5def5|Documentation"
        "area:format|c5def5|Reading and writing of subtitle formats"
        "area:gui|c5def5|Qt graphical interface"
        "area:i18n|c5def5|Internationalization"
        "area:scripts|c5def5|Automation scripts"
        "area:test|c5def5|Tests and benchmarks"
        "area:text|c5def5|Correction and formatting of text"
        "area:video|c5def5|Video player and timing"

        "prio:high|b60205|To be handled first"
        "prio:medium|fbca04|Normal priority"
        "prio:low|0e8a16|Can wait"

        "blocked|000000|Waiting for some work or a decision"
        "needs-decision|d4c5f9|Needs a decision before going on"
    )

    local entry name color description
    for entry in "${labels[@]}"; do
        IFS='|' read -r name color description <<< "${entry}"
        gh label create "${name}" --repo "${REPO}" --color "${color}" \
            --description "${description}" --force >/dev/null
    done
    done_ "${#labels[@]} labels created or updated"

    # GitHub's default labels duplicate the `type:` taxonomy and invite
    # contributions the project does not accept.
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
    (( removed == 0 )) || done_ "${removed} default labels deleted"
}

# --- Milestones -------------------------------------------------------------

apply_milestones() {
    info "milestones"

    # Phases 0 to 7 lead to the MVP, the following ones complete feature
    # parity. See docs/feuille-de-route.md. The titles, before the `|`, are
    # GitHub's: they stay as they are; the descriptions after it are prose.
    local -a phases=(
        "0 — Fondations|Structure, build chain and verification net"
        "1 — Noyau : modèle et formats|Subtitle, positions, SubRip and WebVTT, tags, reversible commands"
        "2 — Opérations d'édition|Basic editing, shift, transformation, frame rate conversion"
        "3 — CLI : harnais|Inspection, conversion and shift, to validate and measure the core without an interface"
        "4 — Mentions pour malentendants|Removal of sounds in brackets and in parentheses"
        "5 — Interface : édition tabulaire|Window, editable table, open, save as, undo and redo"
        "6 — Le lecteur intégré|Play the video in the window, the line drawn over the picture"
        "7 — Finitions et première livraison|Preferences, theme, manual, packaging — the MVP can be delivered"
        "8 — Encodages et fins de ligne|Automatic detection, full set of encodings and line endings"
        "9 — Formats complémentaires|SubViewer 2, SSA, ASS, MicroDVD, MPL2, TMPlayer, LRC and rich tags"
        "10 — Opérations complémentaires|Durations, case, italics, dashes, merge, split, search"
        "11 — Traduction et multi-projets|Second document, alignment, tabs, grouped operations"
        "12 — Moteur de correction complet|Patterns per language, line breaking, spell checker"
        "13 — CLI complète|Subcommands meant for real use, and batch processing"
        "14 — Calage fin|Set start and end from the video position, step frame by frame, burn in the timecode"
        "15 — Internationalisation|The twenty locales of Gaupol, taken over"
    )

    # The match is made on the phase number and not on the full title, so
    # that a revised label renames the milestone instead of creating a second
    # one.
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

    done_ "${created} milestones created, ${updated} updated"
}

# --- Rulesets ---------------------------------------------------------------
#
# A single ruleset, `protection de l'historique` (its name on GitHub), which
# exempts nobody. The owner is the only one who writes; the real risk is
# therefore not a malicious third party but an accidental force-push of their
# own. The rule has to apply to them too to make sense.
#
# **The second one, `porte de qualité`, was deleted in #232.** It required the
# check of `ci.yml`, which no longer runs on pull requests: a rule that waits
# for a check nothing will produce protects from nothing. This script therefore
# does not recreate it.

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

    # Writing a ruleset falls under the "Administration" permission, which
    # fine-grained tokens do not grant by default. The failure is reported, not
    # fatal: the rest of the configuration has already been applied.
    if gh api "${endpoint}" --method "${method}" --input - <<< "${payload}" >/dev/null 2>&1; then
        done_ "\"${name}\" $([[ -n "${id}" ]] && echo "updated" || echo "created")"
        return
    fi

    ruleset_failures=$((ruleset_failures + 1))
    printf '  %s✗%s "%s" — write refused\n' "${RED}" "${RESET}" "${name}"
}

apply_rulesets() {
    info "rulesets on the default branch"

    create_ruleset "protection de l'historique" '{
        "name": "protection de l'\''historique",
        "target": "branch",
        "enforcement": "active",
        "conditions": {"ref_name": {"include": ["~DEFAULT_BRANCH"], "exclude": []}},
        "rules": [{"type": "deletion"}, {"type": "non_fast_forward"}]
    }'
}

report_manual_steps() {
    info "to do in the interface — no API, or out of the token's reach"

    if (( ruleset_failures > 0 )); then
        printf '  The ruleset requires the "Administration" permission of the token.\n'
        printf '  Two ways:\n'
        printf '    add Administration: Read and write to the token, then rerun this script\n'
        printf '    or create it by hand: Settings → Rules → Rulesets\n\n'
    fi

    printf '  Settings → Actions → Fork pull request workflows\n'
    printf '      → Require approval for all external contributors\n'
    printf '  Settings → Moderation → Interaction limits\n'
    printf '      → Limit to prior contributors, to renew every six months\n'
    printf '  see docs/configuration-github.md\n'
}

require_gh
apply_labels
apply_milestones
apply_rulesets
report_manual_steps
