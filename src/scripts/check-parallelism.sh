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
# Trois familles de fichiers :
#
#   - Makefile         : les recettes, c'est-à-dire les lignes commençant par
#                        une tabulation.
#   - src/scripts/     : tout fichier, à toute profondeur, retenu sur son
#                        extension `.sh` ou sur son shebang — les hooks git
#                        (`hooks/pre-commit`, `hooks/commit-msg`) n'ont ni l'un
#                        ni l'autre à la racine, et sont de vrais scripts bash.
#   - cmake/*.cmake,
#     CMakeLists.txt   : celui de la racine et les huit de `src/`, qui déclarent
#                        les cibles de test et de benchmark — précisément là où
#                        un parallélisme codé en dur aurait le plus de chances
#                        d'apparaître.
#
# Les formes reconnues sont décrites au-dessus de `token_violation`, en un seul
# endroit dont le Makefile et les scripts dépendent tous les deux : une forme
# apprise une fois l'est partout. C'est ce qui manquait auparavant — `-j` sans
# valeur échouait dans le Makefile et passait dans un script.
#
# Les commentaires et les exemples de documentation ne doivent pas déclencher le
# contrôle : le commentaire du Makefile montre `make build JOBS=8`, celui de
# cmake/Lto.cmake montre `-DSUBEDIT_LTO_JOBS=8`. Un contrôle qui crie au loup
# finit désactivé, et ne protège alors plus rien — d'où le soin mis à retirer
# les commentaires de fin de ligne *avant* de balayer les jetons, et non après.

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

# Deux fichiers échappent au balayage des scripts, chacun pour une raison qui
# lui est propre :
#
#   - check-parallelism.sh cite `-j`, `-P`, `xargs` et `nproc` en toutes
#     lettres, comme motifs à reconnaître et non comme commandes à exécuter. Le
#     confronter à ses propres motifs ne prouverait rien d'autre qu'il se
#     contient lui-même.
#   - verify-gates.sh porte les défauts injectés qui prouvent que les portes se
#     referment, dont une recette Makefile avec `-j 8`. Ce sont des données de
#     test, pas du code exécuté. Le fichier ne passait jusqu'ici que parce que
#     le guillemet fermant était collé au chiffre : une reformulation d'un seul
#     caractère aurait fait échouer `make parallelism` sur les propres données
#     de test du projet. Mieux vaut une exclusion écrite qu'une syntaxe qui
#     tient par accident.
#
# Contrepartie de la seconde, à connaître : un vrai parallélisme qui
# apparaîtrait un jour dans verify-gates.sh ne serait plus vu. Le fichier
# n'exécute rien d'autre que des cibles make, ce qui rend le risque théorique.
readonly EXCLUDED_SCRIPTS=(
    check-parallelism.sh
    verify-gates.sh
)

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

# Retire un commentaire de fin de ligne, en respectant les guillemets : un `#`
# à l'intérieur d'une chaîne simple ou double ('run #1', "run #1") ne débute
# pas un commentaire, seul celui qui apparaît hors guillemet et en tête de mot
# (début de ligne, ou précédé d'une espace ou d'un métacaractère shell comme
# `&`, `;`, `|`, `(`) en débute un — `tache &#collé` est un commentaire valide
# sans espace, `echo "run #1" &` n'en est pas un. Une analyse par caractère,
# pas un motif : un motif fondé sur " #" se ferait justement piéger par le
# premier cas.
#
# Les guillemets échappés par `\` à l'intérieur d'une chaîne double sont suivis.
# Sans cela, `printf "a \" # b" &` voyait le `\"` comme une fermeture, prenait
# le `#` pour un commentaire, et laissait le `&` final échapper au contrôle.
#
# Limite qui reste, écrite ici pour que ce commentaire ne promette pas plus que
# le code ne tient : l'échappement n'est suivi qu'à un niveau, et les
# substitutions (`$(...)`, `` ` ``) ne sont pas analysées. Aller plus loin
# reviendrait à écrire un analyseur shell, disproportionné pour ce que ce
# contrôle protège.
strip_trailing_comment() {
    local line="$1"
    local -i i len=${#line}
    local c prev="" in_single=0 in_double=0
    local result=""

    for ((i = 0; i < len; i++)); do
        c="${line:i:1}"

        if (( in_single )); then
            [[ "${c}" == "'" ]] && in_single=0
        elif (( in_double )); then
            # Dans une chaîne double, `\` protège le caractère suivant : les
            # deux sont recopiés d'un bloc, sans que le second soit interprété.
            if [[ "${c}" == '\' ]] && (( i + 1 < len )); then
                result+="${c}${line:i+1:1}"
                prev="${line:i+1:1}"
                i+=1
                continue
            fi
            [[ "${c}" == '"' ]] && in_double=0
        elif [[ "${c}" == "'" ]]; then
            in_single=1
        elif [[ "${c}" == '"' ]]; then
            in_double=1
        elif [[ "${c}" == '#' ]]; then
            case "${prev}" in
                ''|[[:space:]]|'&'|';'|'|'|'(')
                    break
                    ;;
            esac
        fi

        result+="${c}"
        prev="${c}"
    done

    printf '%s' "${result}"
}

# Une valeur de parallélisme est légitime quand elle délègue à $(JOBS) plutôt
# que de décider elle-même.
is_jobs_value() {
    case "$1" in
        '$(JOBS)'|'${JOBS}'|'"${JOBS}"'|'$JOBS'|'"$JOBS"') return 0 ;;
        *) return 1 ;;
    esac
}

# La ligne invoque-t-elle un outil qui parallélise ? Sert à distinguer un `-j`
# de construction d'un `-j` qui se trouve être le drapeau d'un autre programme
# — `jq -j`, par exemple. Sans cette distinction, signaler tout `-j` sans
# valeur serait précisément le faux positif que ce contrôle doit éviter.
invokes_parallel_tool() {
    [[ "$1" =~ (^|[^[:alnum:]_-])(make|cmake|ninja|xargs)([^[:alnum:]_-]|$) ]]
}

# Rend la raison du contournement, ou rien. Les formes reconnues :
#
#   -j 8, -P 4, -j8, -P4          un nombre de processus figé
#   -j, -P sans nombre            pire encore : autant de processus que de
#                                 cœurs, sur une ligne qui construit
#   --parallel 8, --parallel=8    la forme longue de cmake --build
#   MAKEFLAGS=-j8                 le parallélisme passé par l'environnement
#
# La ligne reçue doit déjà être débarrassée de son commentaire de fin : c'est à
# l'appelant de le faire, parce que lui seul sait si le fichier est un shell.
token_violation() {
    local line="$1"

    local -a tokens
    read -ra tokens <<< "${line}"

    local i token next
    for i in "${!tokens[@]}"; do
        token="${tokens[${i}]}"
        next="${tokens[$((i + 1))]:-}"

        case "${token}" in
            -j|-P)
                is_jobs_value "${next}" && continue
                if [[ "${next}" =~ ^[0-9]+$ ]]; then
                    printf 'parallélisme codé en dur (%s %s)' "${token}" "${next}"
                    return 0
                fi
                if invokes_parallel_tool "${line}"; then
                    if [[ -z "${next}" ]]; then
                        printf 'parallélisme non borné (%s sans valeur)' "${token}"
                    else
                        printf 'parallélisme hors de $(JOBS) (%s %s)' "${token}" "${next}"
                    fi
                    return 0
                fi
                ;;
            -j[0-9]*|-P[0-9]*)
                printf 'parallélisme codé en dur (%s)' "${token}"
                return 0
                ;;
            --parallel)
                is_jobs_value "${next}" && continue
                printf 'parallélisme codé en dur (--parallel %s)' "${next:-sans valeur}"
                return 0
                ;;
            --parallel=*)
                is_jobs_value "${token#--parallel=}" && continue
                printf 'parallélisme codé en dur (%s)' "${token}"
                return 0
                ;;
            *MAKEFLAGS=*)
                # Le motif est large à dessein : dans une recette Makefile, le
                # `@` qui tait la ligne est collé au jeton — `@MAKEFLAGS=-j8`.
                #
                # Le `-j` doit être en tête de mot : `--jobserver-auth` en
                # contient un au sens du texte, pas au sens des options.
                if [[ "${token#*MAKEFLAGS=}" =~ (^|[^[:alnum:]-])-j ]]; then
                    printf 'parallélisme passé par MAKEFLAGS (%s)' "${token}"
                    return 0
                fi
                ;;
        esac
    done

    return 0
}

# Invariant 1 — chaque recette du Makefile qui parallélise passe par $(JOBS).
#
# Seules les lignes de recette sont lues, c'est-à-dire celles qui commencent par
# une tabulation. Le reste du fichier — dont les commentaires qui montrent
# `make build JOBS=8` — n'est pas du code.
check_makefile() {
    local makefile="${REPO_ROOT}/Makefile"
    [[ -f "${makefile}" ]] || { report_success "aucun Makefile, rien à vérifier"; return 0; }

    local -a offenders=()
    local line reason
    while IFS= read -r line; do
        [[ "${line}" == $'\t'* ]] || continue

        reason="$(token_violation "${line}")"
        if [[ -n "${reason}" ]]; then offenders+=("${reason} : ${line#$'\t'}"); fi
    done < "${makefile}"

    if (( ${#offenders[@]} > 0 )); then
        report_failure "Makefile : parallélisme qui contourne \$(JOBS) :
$(printf '    %s\n' "${offenders[@]}")"
    else
        report_success "Makefile : tout le parallélisme passe par \$(JOBS)"
    fi
}

# Les fichiers de src/scripts/ qui sont des scripts shell : ceux qui portent
# l'extension, et ceux qui portent un shebang. Les hooks git n'ont que le
# second — ils s'installent sous le nom que git leur impose, sans extension.
collect_scripts() {
    local file base first excluded
    while IFS= read -r -d '' file; do
        base="$(basename "${file}")"

        excluded=0
        local candidate
        for candidate in "${EXCLUDED_SCRIPTS[@]}"; do
            if [[ "${base}" == "${candidate}" ]]; then
                excluded=1
                break
            fi
        done
        (( excluded == 0 )) || continue

        if [[ "${file}" == *.sh ]]; then
            printf '%s\0' "${file}"
            continue
        fi

        first=""
        IFS= read -r first < "${file}" 2>/dev/null || true
        if [[ "${first}" =~ ^#!.*(bash|sh)([[:space:]]|$) ]]; then
            printf '%s\0' "${file}"
        fi
    done < <(find "${REPO_ROOT}/src/scripts" -type f -print0 2>/dev/null | sort -z || true)
}

# Invariant 2 — les scripts n'introduisent aucun parallélisme à eux.
#
# Un ✓ par invariant, pas un ✓ par fichier balayé : la liste des fautifs ne
# sort que sur échec, sans quoi chaque script ajouté au projet ajouterait une
# ligne permanente à une sortie censée rester lisible.
check_scripts() {
    local -a offenders=()
    local script relative line trimmed without_comment reason

    while IFS= read -r -d '' script; do
        relative="${script#"${REPO_ROOT}"/}"

        while IFS= read -r line; do
            trimmed="${line#"${line%%[![:space:]]*}"}"
            [[ -z "${trimmed}" || "${trimmed}" == '#'* ]] && continue

            # Le commentaire de fin de ligne part d'abord : sans quoi
            # `cmake --build . # exemple : -j 4` serait signalé comme un
            # contournement, alors que la commande n'en contient aucun.
            without_comment="$(strip_trailing_comment "${line}")"

            reason="$(token_violation "${without_comment}")"

            if [[ -z "${reason}" ]] \
                && [[ "${without_comment}" =~ (^|[^[:alnum:]_])nproc([^[:alnum:]_]|$) ]]; then
                reason="appel à nproc"
            fi

            if [[ -z "${reason}" ]]; then
                # `tâche & # commentaire` reste une tâche mise en arrière-plan :
                # c'est la fin de la commande qu'il faut tester, pas la fin du
                # commentaire.
                local trailing="${without_comment%"${without_comment##*[![:space:]]}"}"
                if [[ "${trailing}" == *'&' && "${trailing}" != *'&&' ]]; then
                    reason="commande mise en arrière-plan (&)"
                fi
            fi

            [[ -n "${reason}" ]] && offenders+=("${relative} : ${reason} : ${trimmed}")
        done < "${script}"
    done < <(collect_scripts)

    if (( ${#offenders[@]} > 0 )); then
        report_failure "src/scripts : parallélisme propre à un script :
$(printf '    %s\n' "${offenders[@]}")"
    else
        report_success "src/scripts : aucun script n'introduit son propre parallélisme"
    fi
}

# Les fichiers CMake du projet : les modules, le CMakeLists de la racine, et
# ceux de src/ — ces derniers déclarent les cibles de test et de benchmark.
# Les `|| true` ne sont pas de la superstition : sous `pipefail`, un
# `find répertoire-absent | sort` fait échouer le pipeline, ce qui sous `set -e`
# interrompt la collecte dès sa première ligne — les balayages suivants ne sont
# alors jamais atteints, et le contrôle rend un verdict vert sans avoir rien lu.
# Le défaut ne se voyait pas sur le dépôt, où tous ces répertoires existent ;
# il apparaissait sous `--root`, c'est-à-dire précisément quand on cherche à
# mettre le contrôle à l'épreuve.
collect_cmake() {
    find "${REPO_ROOT}/cmake" -maxdepth 1 -name '*.cmake' -print0 2>/dev/null | sort -z || true
    if [[ -f "${REPO_ROOT}/CMakeLists.txt" ]]; then
        printf '%s\0' "${REPO_ROOT}/CMakeLists.txt"
    fi
    find "${REPO_ROOT}/src" -name 'CMakeLists.txt' -type f -print0 2>/dev/null | sort -z || true
}

# Invariant 3 — le LTO ne code aucun nombre de processus en dehors de
# SUBEDIT_LTO_JOBS, et aucun fichier CMake ne parallélise de lui-même.
check_cmake() {
    local -a offenders=()
    local file relative line trimmed reason

    while IFS= read -r -d '' file; do
        relative="${file#"${REPO_ROOT}"/}"

        while IFS= read -r line; do
            trimmed="${line#"${line%%[![:space:]]*}"}"
            [[ -z "${trimmed}" || "${trimmed}" == '#'* ]] && continue

            if [[ "${line}" == *'-flto='* && "${line}" != *'-flto=${SUBEDIT_LTO_JOBS}'* ]]; then
                offenders+=("${relative} : -flto= codé en dur : ${trimmed}")
                continue
            fi

            reason="$(token_violation "${line}")"
            [[ -n "${reason}" ]] && offenders+=("${relative} : ${reason} : ${trimmed}")
        done < "${file}"
    done < <(collect_cmake)

    if (( ${#offenders[@]} > 0 )); then
        report_failure "cmake : parallélisme codé en dur :
$(printf '    %s\n' "${offenders[@]}")"
    else
        # Établi : aucun de ces fichiers ne code -flto= en dehors de
        # SUBEDIT_LTO_JOBS, ni ne parallélise autrement. Ça ne dit rien de ceux
        # qui ne mentionnent le sujet nulle part — c'est pourquoi la phrase
        # reste vraie pour eux aussi.
        report_success "cmake : aucun parallélisme codé en dur"
    fi
}

check_makefile
check_scripts
check_cmake

if (( failures > 0 )); then
    printf '\n%s%d contournement(s) de $(JOBS)%s\n' "${RED}" "${failures}" "${RESET}" >&2
    exit 1
fi
