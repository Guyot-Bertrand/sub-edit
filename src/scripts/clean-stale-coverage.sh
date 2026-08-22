#!/usr/bin/env bash
# Écarte un arbre de couverture qu'un déplacement de source a rendu incohérent.
#
# **Le défaut, et pourquoi il ne se voit pas venir.** `gcov` écrit un `.gcno`
# par unité de compilation, à côté de son `.o`. Un fichier source qui change
# d'emplacement en laisse un derrière lui : le générateur ne supprime pas les
# sorties qu'il ne produit plus, et le `.gcno` n'est de toute façon pas une
# sortie déclarée — `ninja -t cleandead` retire le `.o` et laisse le `.gcno`.
# `gcovr` tombe alors sur un fichier dont il ne sait plus retrouver la source,
# et échoue sur un message qui ne nomme ni le fichier fautif ni le remède :
#
#     Could not open output file 'c++config.h##afd....gcov'
#         (gcovr could not infer a working directory that resolved it.)
#
# **Local seulement.** L'intégration continue part d'un arbre neuf et ne
# rencontre jamais d'orphelin. C'est ce qui rend le défaut pénible : il ne
# frappe que celui qui travaille.
#
# **Le critère.** Seule une source qui DISPARAÎT orpheline un `.gcno` — un
# ajout n'orpheline rien. On garde donc la liste des sources compilées, et on
# écarte l'arbre entier dès que l'une d'elles n'est plus là. Écarter plutôt que
# retrancher : le lien entre un `.gcno` et sa source n'est pas lisible depuis
# le système de fichiers sans deviner la disposition du générateur, et deviner
# est ce qui ferait supprimer les `.gcno` des sources engendrées par `moc`.
#
# Le prix est une reconstruction complète, exactement quand une reconstruction
# se justifie : un déplacement de fichier, ce qui arrive quelques fois par an.
# Un ajout, lui, ne coûte rien.

set -euo pipefail

readonly REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"

readonly YELLOW=$'\033[33m'
readonly RESET=$'\033[0m'

build_dir="${REPO_ROOT}/build/coverage"

usage() {
    cat >&2 <<USAGE
usage: clean-stale-coverage.sh [--build-dir <répertoire>]

  --build-dir  arbre de couverture à vérifier (défaut : build/coverage)
USAGE
    exit 2
}

while (( $# > 0 )); do
    case "$1" in
        --build-dir) [[ $# -ge 2 ]] || usage; build_dir="$2"; shift 2 ;;
        -h|--help)   usage ;;
        *)           printf 'argument inconnu : %s\n' "$1" >&2; usage ;;
    esac
done

readonly MANIFEST="${build_dir}/.sources"

# Les sources compilables du projet, chemins relatifs à la racine, triées.
# `sort` sans locale pour que l'ordre ne dépende pas de l'environnement.
sources_now() {
    (cd "${REPO_ROOT}" && find src -name '*.cpp' -o -name '*.hpp' | LC_ALL=C sort)
}

# Rien à comparer : premier passage, ou arbre déjà absent.
if [[ ! -f "${MANIFEST}" ]]; then
    mkdir -p "${build_dir}"
    sources_now > "${MANIFEST}"
    exit 0
fi

# Une source du relevé précédent qui n'est plus là : l'arbre porte au moins un
# `.gcno` que plus rien ne rattache à une source.
gone=""
while IFS= read -r path; do
    [[ -n "${path}" ]] || continue
    if [[ ! -f "${REPO_ROOT}/${path}" ]]; then
        gone="${path}"
        break
    fi
done < "${MANIFEST}"

if [[ -n "${gone}" ]]; then
    printf '%sarbre de couverture écarté : %s a disparu depuis la dernière mesure%s\n' \
        "${YELLOW}" "${gone}" "${RESET}"
    printf '  un .gcno orphelin ferait échouer gcovr sans nommer sa cause.\n'
    rm -rf "${build_dir}"
    mkdir -p "${build_dir}"
fi

sources_now > "${MANIFEST}"
