#!/usr/bin/env bash
# La porte de qualité, et l'ordre de ses étapes — issue #269.
#
# ## Ce qu'il apporte, et pourquoi il existe
#
# Les étapes vivaient dans des recettes du `Makefile`, qui les enchaînait par
# des sous-`make`. Ça marche, et ça a un défaut qu'on paie à chaque échec tardif
# — **on ne sait pas reprendre au milieu.** Une couverture qui casse à la
# trente-cinquième minute faisait repayer les trente-quatre premières, format et
# analyse statique comprises, alors que rien n'y avait bougé.
#
#     ./src/scripts/gate.sh --list
#     ./src/scripts/gate.sh check --from coverage
#     ./src/scripts/gate.sh check --only tidy
#     ./src/scripts/gate.sh check-local --skip bench
#
# **L'ordre reste ici et nulle part ailleurs.** C'est la seule chose que ce
# fichier sait et que les étapes ignorent : chacune se lance seule, sans savoir
# qui la précède. `src/scripts/gate/<étape>.sh` porte ce qu'elle fait ; ce
# fichier-ci porte quand.
#
# ## Ce qu'il ne fait pas
#
# **Il ne remplace pas `make`.** Les cibles restent, réduites à un appel : elles
# sont l'interface qu'on tape, et `verify-gates.sh` les invoque pour prouver que
# chaque porte se referme. Ce script est ce qu'elles appellent, et ce qu'on
# appelle directement quand on veut reprendre au milieu.
#
# **Il ne saute rien en silence.** Un nom d'étape inconnu, à `--from`, `--only`
# ou `--skip`, fait échouer l'invocation. Une faute de frappe qui ferait
# n'exécuter aucune étape et rendre zéro serait le pire de tous les défauts :
# une porte verte qui n'a rien fait.

set -euo pipefail

readonly REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
readonly STEPS_DIR="${REPO_ROOT}/src/scripts/gate"

readonly BOLD=$'\033[1m'
readonly GREEN=$'\033[32m'
readonly RED=$'\033[31m'
readonly RESET=$'\033[0m'

# **L'ordre de `check`, et il n'est pas arbitraire.**
#
# Le relevé des fichiers non suivis encadre tout le reste, et non les seuls
# tests : ce qui est surveillé est ce que la porte laisse derrière elle, d'où
# qu'il vienne. Le format passe avant la compilation parce qu'il coûte une
# seconde et qu'un échec de format doit coûter une seconde. L'analyse statique
# vient après la compilation parce qu'elle en est une.
readonly CHECK_STEPS=(
    untracked-record
    config-home-record
    format-check
    arch
    build
    tidy
    asan
    coverage
    untracked
    config-home
)

# **L'ordre de `check-local`, du moins cher au plus cher**, pour qu'un échec
# coûte des secondes plutôt que la chaîne entière : parallélisme (un grep, sous
# la seconde), fixtures (deux appels à ffprobe), score de détection (le noyau et
# un programme de quarante lignes), manuel (un binaire de plus, et Qt),
# exigences (compilation incrémentale), bout en bout (build release),
# installation — qui partage cet arbre — puis les benchmarks.
readonly LOCAL_STEPS=(
    parallelism
    fixtures
    score
    manual-check
    requirements
    e2e
    install-check
    bench
)

# Ce que chaque nom d'étape lance. Les étapes qui portent de la logique vivent
# dans `gate/` ; celles qui sont déjà un script à elles sont appelées telles
# quelles, plutôt que d'être enveloppées pour l'être.
run_step() {
    case "$1" in
    untracked-record) "${REPO_ROOT}/src/scripts/check-untracked.sh" --record ;;
    config-home-record) "${REPO_ROOT}/src/scripts/check-config-home.sh" --record ;;
    format-check) "${STEPS_DIR}/format.sh" --check ;;
    format) "${STEPS_DIR}/format.sh" ;;
    arch) printf '%s▸ invariants d architecture%s\n' "${BOLD}" "${RESET}"
        "${REPO_ROOT}/src/scripts/check-architecture.sh" ;;
    build) "${STEPS_DIR}/build.sh" ;;
    tidy) "${STEPS_DIR}/tidy.sh" ;;
    asan) "${STEPS_DIR}/asan.sh" ;;
    coverage) "${STEPS_DIR}/coverage.sh" ;;
    untracked) printf '%s▸ fichiers laissés derrière%s\n' "${BOLD}" "${RESET}"
        "${REPO_ROOT}/src/scripts/check-untracked.sh" --compare ;;
    config-home) printf '%s▸ configuration de l utilisateur%s\n' "${BOLD}" "${RESET}"
        "${REPO_ROOT}/src/scripts/check-config-home.sh" --compare ;;
    parallelism) printf '%s▸ parallélisme maîtrisé%s\n' "${BOLD}" "${RESET}"
        "${REPO_ROOT}/src/scripts/check-parallelism.sh" ;;
    fixtures) "${STEPS_DIR}/fixtures.sh" ;;
    score) "${STEPS_DIR}/score.sh" ;;
    manual-check) "${STEPS_DIR}/manual.sh" --check ;;
    manual) "${STEPS_DIR}/manual.sh" ;;
    requirements) "${STEPS_DIR}/requirements.sh" ;;
    e2e) "${STEPS_DIR}/e2e.sh" ;;
    install-check) "${STEPS_DIR}/install-check.sh" ;;
    bench) "${STEPS_DIR}/bench.sh" ;;
    release) "${STEPS_DIR}/release.sh" ;;
    packages) "${STEPS_DIR}/packages.sh" ;;
    rpm-check) "${STEPS_DIR}/rpm-check.sh" ;;
    *)
        printf '%sétape inconnue : %s%s\n' "${RED}" "$1" "${RESET}" >&2
        return 1
        ;;
    esac
}

usage() {
    cat >&2 <<'USAGE'
usage: gate.sh <check|check-local> [--from <étape>] [--only <étape>] [--skip <étape>]
       gate.sh --list
       gate.sh <étape>

  --from   reprend à cette étape, et joue tout ce qui suit
  --only   ne joue que celle-là, répétable
  --skip   joue tout sauf celle-là, répétable
  --list   écrit les étapes de chaque suite, dans l'ordre

Sans suite nommée, le premier argument est une étape et elle est jouée seule.
USAGE
    exit 2
}

list_steps() {
    printf '%scheck%s\n' "${BOLD}" "${RESET}"
    printf '  %s\n' "${CHECK_STEPS[@]}"
    printf '%scheck-local%s\n' "${BOLD}" "${RESET}"
    printf '  %s\n' "${LOCAL_STEPS[@]}"
}

# Vrai si `$1` est l'un des arguments suivants. Sert aux trois filtres, qui
# posent la même question sur trois listes différentes.
contains() {
    local needle="$1"
    shift
    local one
    for one in "$@"; do
        [[ "${one}" == "${needle}" ]] && return 0
    done
    return 1
}

(($# > 0)) || usage

if [[ "$1" == "--list" ]]; then
    list_steps
    exit 0
fi

suite="$1"
shift

case "${suite}" in
check) steps=("${CHECK_STEPS[@]}") ;;
check-local) steps=("${LOCAL_STEPS[@]}") ;;
-h | --help) usage ;;
*)
    # Une étape nommée seule : pas de filtre, pas de suite.
    (($# == 0)) || usage
    run_step "${suite}"
    exit $?
    ;;
esac

from=""
only=()
skip=()

while (($# > 0)); do
    case "$1" in
    --from) [[ $# -ge 2 ]] || usage; from="$2"; shift 2 ;;
    --only) [[ $# -ge 2 ]] || usage; only+=("$2"); shift 2 ;;
    --skip) [[ $# -ge 2 ]] || usage; skip+=("$2"); shift 2 ;;
    -h | --help) usage ;;
    *) printf 'argument inconnu : %s\n' "$1" >&2; usage ;;
    esac
done

# **Tout nom donné doit désigner une étape de la suite**, et le contrôle a lieu
# avant qu'une seule ne tourne. Sans lui, `--from covrage` jouerait zéro étape
# et rendrait zéro : une porte verte qui n'a rien fait, ce que ce projet
# refuse partout ailleurs.
for named in ${from:+"${from}"} ${only+"${only[@]}"} ${skip+"${skip[@]}"}; do
    if ! contains "${named}" "${steps[@]}"; then
        printf '%s✗ « %s » n est pas une étape de %s%s\n' "${RED}" "${named}" "${suite}" "${RESET}" >&2
        printf '  les étapes sont :\n' >&2
        printf '    %s\n' "${steps[@]}" >&2
        exit 2
    fi
done

selected=()
reached=0
for one in "${steps[@]}"; do
    [[ -n "${from}" && "${one}" == "${from}" ]] && reached=1
    [[ -n "${from}" && "${reached}" -eq 0 ]] && continue
    ((${#only[@]} == 0)) || contains "${one}" "${only[@]}" || continue
    ((${#skip[@]} == 0)) || ! contains "${one}" "${skip[@]}" || continue
    selected+=("${one}")
done

# Le même garde que ci-dessus, de l'autre côté : les noms étaient bons et la
# combinaison ne retient pourtant rien — `--only a --skip a`, par exemple.
if ((${#selected[@]} == 0)); then
    printf '%s✗ aucune étape retenue%s\n' "${RED}" "${RESET}" >&2
    exit 2
fi

printf '%s%s — %d étape(s)%s\n' "${BOLD}" "${suite}" "${#selected[@]}" "${RESET}"

for one in "${selected[@]}"; do
    run_step "${one}"
done

printf '%s✓ %s franchie%s\n' "${GREEN}" "${suite}" "${RESET}"
