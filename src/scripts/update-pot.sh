#!/usr/bin/env bash
# Le gabarit des messages, extrait du code : src/po/subedit.pot — issue #659.
#
# **Ce que le gabarit liste** : chaque littéral passé à l'API de `core/i18n/messages.hpp`
# (`translate`, `translateIn`, `translatePlural`), dans `src/lib` et `src/exe`. Les tests n'en
# font pas partie : leurs messages ne sont pas ceux de l'utilisateur. Les mots-clés de la
# fenêtre (`tr`) s'ajouteront avec le ticket qui y branche l'API.
#
# **Reproductible, octet pour octet** — c'est ce que le ticket exige, et ce que `--check`
# éprouve : les sources sont triées, les emplacements n'écrivent que le fichier (un numéro de
# ligne changerait à chaque insertion et noierait le diff d'un traducteur), et la seule ligne
# que xgettext date lui-même, `POT-Creation-Date`, est ramenée à la valeur de gabarit
# qu'écrit l'outil quand il ne sait pas l'heure. Pas de version non plus : un bump du patch
# ne doit pas réécrire le gabarit.
#
#   update-pot.sh           réécrit src/po/subedit.pot
#   update-pot.sh --check   échoue si le gabarit versionné n'est pas ce que le code produit

set -euo pipefail

readonly REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
readonly POT="${REPO_ROOT}/src/po/subedit.pot"

RED=$'\033[31m'
GREEN=$'\033[32m'
RESET=$'\033[0m'

mode="write"
case "${1:-}" in
"") ;;
--check) mode="check" ;;
*)
    printf 'usage: update-pot.sh [--check]\n' >&2
    exit 2
    ;;
esac

if ! command -v xgettext >/dev/null 2>&1; then
    printf '%s✗ xgettext est introuvable%s\n' "${RED}" "${RESET}" >&2
    printf '  l'\''installer avec : ./src/scripts/setup-toolchain.sh\n' >&2
    exit 1
fi

extract() {
    cd "${REPO_ROOT}"
    local files
    files="$(find src/lib src/exe -type f \( -name '*.cpp' -o -name '*.hpp' \) | LC_ALL=C sort)"

    # `-k` seul retire les mots-clés par défaut (`_`, `gettext`…) : seuls ceux du projet comptent.
    # shellcheck disable=SC2086  # la liste est un mot par fichier, sans espace dans un chemin du dépôt
    xgettext --language=C++ --from-code=UTF-8 --force-po --sort-by-file \
        --add-location=file --add-comments=TRANSLATORS \
        --package-name=subedit --copyright-holder='the subedit authors' \
        --msgid-bugs-address='https://github.com/Guyot-Bertrand/sub-edit/issues' \
        -k -ktranslate:1 -ktranslateIn:1c,2 -ktranslatePlural:1,2 \
        --output=- ${files} |
        sed 's/^"POT-Creation-Date: .*\\n"$/"POT-Creation-Date: YEAR-MO-DA HO:MI+ZONE\\n"/'
}

if [[ "${mode}" == "write" ]]; then
    extract >"${POT}.new"
    mv "${POT}.new" "${POT}"
    printf '%s✓%s %s\n' "${GREEN}" "${RESET}" "src/po/subedit.pot"
    exit 0
fi

if diff -u "${POT}" <(extract) >&2; then
    printf '%s✓%s le gabarit src/po/subedit.pot est ce que le code produit\n' "${GREEN}" "${RESET}"
else
    printf '%s✗%s src/po/subedit.pot est périmé : « make pot » le réécrit\n' "${RED}" "${RESET}" >&2
    exit 1
fi
