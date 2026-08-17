#!/usr/bin/env bash
# Prouve que les portes de qualité se referment.
#
# Une porte qu'on n'a jamais vue échouer n'est pas une porte : c'est une
# croyance. Ce script injecte délibérément un défaut de chaque type, vérifie que
# la cible correspondante échoue, puis rétablit les sources.
#
# Les six premières injections visent des étapes de `make check`, que la CI
# exécute. Les trois suivantes visent `make check-local` — deux fois
# `requirements`, une fois `parallelism` — qui ne gate donc que le poste de
# développement : raison de plus pour que ce script prouve que chacune se
# referme, puisque rien d'autre ne les exercera.
#
# Les trois suivantes ne visent ni l'une ni l'autre : ce sont les contrôles de
# pull request, qui n'ont de sens que sur GitHub — un corps de pull request
# n'existe pas en local. Rien ne les exercerait donc jamais avant qu'une vraie
# pull request en dépende. Ils sont invoqués un par un, contrôle par contrôle,
# pour qu'une injection soit seule à pouvoir faire échouer son exécution.
#
# Les quatre suivantes visent `make manual-check`, une par mode d'arrêt du
# générateur d'exemples du manuel.
#
# Les six suivantes visent `make parallelism` : deux familles de fichiers qui
# échappaient au balayage, trois formes qu'il ne reconnaissait pas, et **une
# preuve d'un genre différent** — que du code légitime n'est pas signalé. Un
# critère de non-signalement ne se démontre par aucune injection qui échoue,
# d'où `expect_gate_stays_open`.
#
# Les deux dernières visent `make arch` : un nom de cas de test qui commence par
# un tiret, et un test qui lirait le dépôt de référence. Elle est ici parce que ce défaut ne se voit qu'à travers CTest, jamais
# en lançant le binaire de test à la main — donc uniquement dans la porte.
#
# À rejouer après toute modification de .clang-format, .clang-tidy, des options
# de compilation, du cliquet de couverture, du registre d'exigences ou de
# cliff.toml.
#
# Exige git-cliff, que le contrôle du journal appelle par `make changelog` :
# ./src/scripts/setup-toolchain.sh l'installe.

set -euo pipefail

readonly REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
readonly LIB_SOURCE="${REPO_ROOT}/src/lib/subedit/core/version.cpp"
readonly TEST_SOURCE="${REPO_ROOT}/src/test/unit/core/version_test.cpp"
readonly REGISTRY="${REPO_ROOT}/docs/exigences.md"
readonly E2E_SOURCE="${REPO_ROOT}/src/test/e2e/cli/usage_test.cpp"
readonly MAKEFILE_SOURCE="${REPO_ROOT}/Makefile"
readonly CMAKE_SOURCE="${REPO_ROOT}/CMakeLists.txt"
readonly CHANGELOG_SOURCE="${REPO_ROOT}/CHANGELOG.md"
readonly MANUAL_SOURCE="${REPO_ROOT}/docs/manual/subedit-cli/invocation.md"
readonly HOOK_SOURCE="${REPO_ROOT}/src/scripts/hooks/pre-commit"
readonly PLAIN_SCRIPT_SOURCE="${REPO_ROOT}/src/scripts/install-hooks.sh"
readonly NESTED_CMAKE_SOURCE="${REPO_ROOT}/src/lib/CMakeLists.txt"
readonly PR_CHECK="${REPO_ROOT}/src/scripts/check-pull-request.sh"

readonly RED=$'\033[31m'
readonly GREEN=$'\033[32m'
readonly BOLD=$'\033[1m'
readonly RESET=$'\033[0m'

backup_dir="$(mktemp -d)"
failures=0

restore() {
    cp "${backup_dir}/version.cpp" "${LIB_SOURCE}"
    cp "${backup_dir}/version_test.cpp" "${TEST_SOURCE}"
    cp "${backup_dir}/exigences.md" "${REGISTRY}"
    cp "${backup_dir}/e2e_version_test.cpp" "${E2E_SOURCE}"
    cp "${backup_dir}/Makefile" "${MAKEFILE_SOURCE}"
    cp "${backup_dir}/CMakeLists.txt" "${CMAKE_SOURCE}"
    cp "${backup_dir}/CHANGELOG.md" "${CHANGELOG_SOURCE}"
    cp "${backup_dir}/invocation.md" "${MANUAL_SOURCE}"
    cp "${backup_dir}/pre-commit" "${HOOK_SOURCE}"
    cp "${backup_dir}/install-hooks.sh" "${PLAIN_SCRIPT_SOURCE}"
    cp "${backup_dir}/lib-CMakeLists.txt" "${NESTED_CMAKE_SOURCE}"
}

cleanup() {
    restore
    rm -rf "${backup_dir}"
}

cp "${LIB_SOURCE}" "${backup_dir}/version.cpp"
cp "${TEST_SOURCE}" "${backup_dir}/version_test.cpp"
cp "${REGISTRY}" "${backup_dir}/exigences.md"
cp "${E2E_SOURCE}" "${backup_dir}/e2e_version_test.cpp"
cp "${MAKEFILE_SOURCE}" "${backup_dir}/Makefile"
cp "${CMAKE_SOURCE}" "${backup_dir}/CMakeLists.txt"
cp "${CHANGELOG_SOURCE}" "${backup_dir}/CHANGELOG.md"
cp "${MANUAL_SOURCE}" "${backup_dir}/invocation.md"
cp "${HOOK_SOURCE}" "${backup_dir}/pre-commit"
cp "${PLAIN_SCRIPT_SOURCE}" "${backup_dir}/install-hooks.sh"
cp "${NESTED_CMAKE_SOURCE}" "${backup_dir}/lib-CMakeLists.txt"
trap cleanup EXIT

# Injecte un défaut, exécute la cible make attendue en échec, rétablit.
#
# **Le fragment injecté est écrit entre apostrophes simples**, donc il ne peut
# pas en contenir : une apostrophe française dans un commentaire C++ referme la
# chaîne et le script meurt sur une erreur de syntaxe, loin de la ligne fautive.
# Écrire les commentaires du fragment sans apostrophe.
expect_gate_closes() {
    local label="$1"
    local target="$2"
    local file="$3"
    local snippet="$4"

    printf '%s▸ %s%s\n' "${BOLD}" "${label}" "${RESET}"
    # Un extrait vide veut dire que l'injection a déjà été faite par l'appelant,
    # parce qu'elle ne consiste pas à ajouter du texte en fin de fichier —
    # modifier un bloc existant, par exemple.
    [[ -z "${snippet}" ]] || printf '%s\n' "${snippet}" >> "${file}"

    if make -C "${REPO_ROOT}" --no-print-directory "${target}" >/dev/null 2>&1; then
        printf '  %s✗ la porte « %s » a laissé passer le défaut%s\n' "${RED}" "${target}" "${RESET}"
        failures=$((failures + 1))
    else
        printf '  %s✓ « make %s » a échoué, comme attendu%s\n' "${GREEN}" "${target}" "${RESET}"
    fi

    restore
}

# Même chose pour un contrôle de pull request. Deux différences avec la porte de
# qualité : le contrôle n'est pas une cible make, et son défaut peut vivre dans
# l'environnement — un corps de pull request n'est pas un fichier du dépôt.
#
# Le contrôle est nommé explicitement plutôt que de lancer les trois : une
# exécution complète échouerait de toute façon sur les deux autres, et
# l'injection ne prouverait plus rien. Les arguments qui suivent sont des
# affectations passées à `env`.
expect_pr_check_closes() {
    local label="$1"
    local control="$2"
    shift 2

    printf '%s▸ %s%s\n' "${BOLD}" "${label}" "${RESET}"

    if env "$@" "${PR_CHECK}" "${control}" >/dev/null 2>&1; then
        printf '  %s✗ le contrôle « %s » a laissé passer le défaut%s\n' "${RED}" "${control}" "${RESET}"
        failures=$((failures + 1))
    else
        printf '  %s✓ le contrôle « %s » a refusé, comme attendu%s\n' "${GREEN}" "${control}" "${RESET}"
    fi

    restore
}

# L'inverse d'expect_gate_closes : injecte du code légitime que le contrôle
# pourrait confondre avec un défaut, et vérifie qu'il ne le signale pas.
#
# Un critère de la forme « ceci n'est pas signalé » ne se démontre par aucune
# injection qui échoue. Sans une preuve de ce genre, il resterait une croyance
# — et c'est le mode d'échec qui compte le plus : un contrôle qui crie au loup
# finit désactivé, et ne protège alors plus rien.
expect_gate_stays_open() {
    local label="$1"
    local target="$2"
    local file="$3"
    local snippet="$4"

    printf '%s▸ %s%s\n' "${BOLD}" "${label}" "${RESET}"
    printf '%s\n' "${snippet}" >> "${file}"

    if make -C "${REPO_ROOT}" --no-print-directory "${target}" >/dev/null 2>&1; then
        printf '  %s✓ « make %s » a laissé passer, comme attendu%s\n' "${GREEN}" "${target}" "${RESET}"
    else
        printf '  %s✗ « make %s » a signalé du code légitime%s\n' "${RED}" "${target}" "${RESET}"
        failures=$((failures + 1))
    fi

    restore
}

printf '%svérification des portes de qualité%s\n\n' "${BOLD}" "${RESET}"

expect_gate_closes \
    "défaut de format" \
    "format-check" \
    "${LIB_SOURCE}" \
    'int   badly_Formatted ( int a ,int b ) {return a+b ;}'

# -Wunused-parameter vient de -Wextra, et -Werror en fait une erreur.
expect_gate_closes \
    "warning de compilation" \
    "build" \
    "${LIB_SOURCE}" \
    'namespace subedit::core {
int injectedWarning(int used, int unused) {
    return used;
}
} // namespace subedit::core'

# cppcoreguidelines-owning-memory : un pointeur propriétaire nu.
expect_gate_closes \
    "motif rejeté par clang-tidy" \
    "tidy" \
    "${LIB_SOURCE}" \
    'namespace subedit::core {
int* injectedOwningPointer() {
    return new int(42);
}
} // namespace subedit::core'

# Usage après libération : invisible pour l'analyse statique, détecté par ASan.
expect_gate_closes \
    "erreur mémoire à l'exécution" \
    "asan" \
    "${TEST_SOURCE}" \
    '#include <memory>

TEST_CASE("injected use after free", "[injected]") {
    auto owned = std::make_unique<std::string>("subedit");
    const std::string* observer = owned.get();
    owned.reset();
    CHECK(observer->size() == 7);
}'

# Débordement d'entier signé : invisible pour l'analyse statique et pour ASan,
# détecté par UBSan. Sa propre preuve, distincte de celle d'ASan, parce que les
# deux sanitizers s'arrêtent pour des raisons différentes — ASan tue le
# processus de lui-même, UBSan ne le fait que depuis
# -fno-sanitize-recover=undefined. Sans cette option il signalait le défaut et
# laissait le test passer, ce qu'un vrai débordement de la grammaire du temps a
# démontré à la relecture de la phase 3.
expect_gate_closes \
    "comportement indéfini à l'exécution" \
    "asan" \
    "${TEST_SOURCE}" \
    '#include <cstdint>
#include <limits>

TEST_CASE("injected signed overflow", "[injected]") {
    // Lu à travers une référence : à -O0 le compilateur ne peut pas replier le
    // calcul, et le débordement a donc bien lieu pendant le test.
    std::int64_t largest = std::numeric_limits<std::int64_t>::max();
    const std::int64_t& read = largest;
    CHECK(read + 1 > 0);
}'

# Code non exercé par les tests : fait grimper le nombre de lignes non
# couvertes au-delà du cliquet.
expect_gate_closes \
    "chute de la couverture" \
    "coverage" \
    "${LIB_SOURCE}" \
    'namespace subedit::core {
int injectedUncovered(int value) {
    if (value > 0) {
        return value + 1;
    }
    if (value < 0) {
        return value - 1;
    }
    return 0;
}
} // namespace subedit::core'

# expect_gate_closes ne restaure que les sources : le résumé JSON et le rapport
# HTML que « make coverage » vient de produire à partir du défaut injecté lui
# survivent dans build/coverage-report/. Laissé en place, ce résidu affirme
# encore des lignes non couvertes attribuées à une fonction qui n'existe plus
# dans aucune source — un « make ratchet » lancé juste après l'enregistrerait
# tel quel et desserrerait le cliquet contre un défaut qui n'a jamais existé.
rm -rf "${REPO_ROOT}/build/coverage-report"

# Exigence déclarée implémentée que rien ne cite. L'injection se fait en
# ajoutant une ligne en fin de fichier, ce qui exige que la table du registre
# soit la dernière chose de docs/exigences.md — c'est écrit dans ce fichier.
expect_gate_closes \
    "exigence implémentée sans test" \
    "requirements" \
    "${REGISTRY}" \
    '| `CLI-FANTOME-01` | exigence injectée que rien ne démontre | 3 | implémentée |'

# Tag en forme d'identifiant qui ne désigne aucune exigence du registre.
expect_gate_closes \
    "tag sans exigence correspondante" \
    "requirements" \
    "${E2E_SOURCE}" \
    'TEST_CASE("injected unknown requirement", "[e2e][CLI-INEXISTANT-99]") {
    CHECK(true);
}'

# Recette Makefile qui code un parallélisme en dur au lieu de passer par
# $(JOBS) — exactement ce que check-parallelism.sh existe pour repérer. La
# cible injectée n'est référencée par personne : ni `make parallelism` ni
# aucune autre cible ne l'exécute, donc l'échec attendu ne peut venir que du
# balayage du Makefile par le script, pas d'une tentative de construction.
expect_gate_closes \
    "parallélisme codé en dur dans le Makefile" \
    "parallelism" \
    "${MAKEFILE_SOURCE}" \
    'cible-injectee-parallelisme:
	@cmake --build --preset dev -j 8'

# Corps de pull request qui décrit le travail sans porter la ligne que GitHub
# lit. C'est le défaut mesuré de la phase 2 : cinq issues restées ouvertes après
# la fusion de leur pull request, parce que « Ferme #22 » n'est pas un mot-clé.
# Nom de cas de test qui commence par un tiret. CTest le passe en argument au
# binaire, Catch2 y lit une de ses options, et le cas échoue — mais seulement à
# travers CTest, jamais quand on lance le binaire à la main. L'injection porte
# sur un fichier de test, et l'échec attendu vient du balayage de
# check-architecture.sh, pas d'une compilation.
expect_gate_closes \
    "nom de cas de test qui passe pour une option" \
    "arch" \
    "${TEST_SOURCE}" \
    'TEST_CASE("-v looks like an option to Catch2", "[injected]") {
    CHECK(true);
}'

expect_pr_check_closes \
    "corps de pull request sans « Closes #N »" \
    "closes" \
    "PR_BODY=Ce corps décrit le travail, mais aucune ligne n'y ferme l'issue."

# Version de la branche recopiée depuis celle de HEAD, qui sert de base : les
# deux annoncent alors le même numéro. La recopie est nécessaire — se contenter
# de désigner HEAD comme base supposerait que la copie de travail ne l'a pas
# déjà dépassé, ce qu'un bump non encore commité démentirait, et l'injection ne
# prouverait plus rien.
head_version="$(git -C "${REPO_ROOT}" show HEAD:CMakeLists.txt \
    | sed -n 's/^[[:space:]]*VERSION[[:space:]]\+\([0-9][0-9.]*\)[[:space:]]*$/\1/p' | head -1)"
sed -i "s/^\([[:space:]]*VERSION[[:space:]]\+\)[0-9][0-9.]*[[:space:]]*$/\1${head_version}/" \
    "${CMAKE_SOURCE}"
expect_pr_check_closes \
    "version identique à celle de la base" \
    "version" \
    "BASE_SHA=$(git -C "${REPO_ROOT}" rev-parse HEAD)"

# Journal modifié à la main, donc différent de ce que git-cliff produit à partir
# de l'historique. C'est exactement la forme que prend un journal non régénéré :
# un contenu que la commande ne redonnerait pas.
printf "ligne injectée qu'aucune régénération ne redonnerait\n" >> "${CHANGELOG_SOURCE}"
expect_pr_check_closes \
    "CHANGELOG.md non régénéré" \
    "changelog"

# Les quatre suivantes visent « make manual-check », une par mode d'arrêt du
# générateur d'exemples. Le pire résultat que ce générateur puisse produire est
# un manuel silencieusement vidé de ses exemples ; c'est pourquoi chacun de ses
# refus se prouve, et non seulement celui du manuel périmé.

# Bloc dont la sortie ne correspond plus à ce que la commande produit. La
# modification porte sur un bloc existant, pas sur une ligne ajoutée en fin de
# fichier : d'où l'extrait vide passé à l'injecteur.
sed -i 's/^subedit [0-9][0-9.]*$/subedit 0.9.9/' "${MANUAL_SOURCE}"
expect_gate_closes \
    "exemple du manuel périmé" \
    "manual-check" \
    "${MANUAL_SOURCE}" \
    ''

expect_gate_closes \
    "exemple dont la commande échoue" \
    "manual-check" \
    "${MANUAL_SOURCE}" \
    '
<!-- exemple: echo bonjour; exit 3 -->
```console
$ echo bonjour; exit 3
bonjour
```'

expect_gate_closes \
    "marqueur non suivi d'un bloc console" \
    "manual-check" \
    "${MANUAL_SOURCE}" \
    '
<!-- exemple: subedit-cli -->
Ce paragraphe ne commence pas un bloc console.'

# Commande muette : c'est le cas qui produirait un bloc vide si le générateur
# le laissait passer.
expect_gate_closes \
    "exemple dont la commande ne produit rien" \
    "manual-check" \
    "${MANUAL_SOURCE}" \
    '
<!-- exemple: true -->
```console
$ true
```'

# Les cinq suivantes visent l'élargissement de `make parallelism` : deux
# familles de fichiers qui échappaient au balayage, trois formes de
# parallélisme qu'il ne reconnaissait pas.

# Hook git : dans un sous-répertoire et sans extension .sh, donc doublement
# manqué par l'ancien balayage — qui ne descendait pas et filtrait sur *.sh.
expect_gate_closes \
    "parallélisme codé en dur dans un hook git" \
    "parallelism" \
    "${HOOK_SOURCE}" \
    'cmake --build . -j 8'

# CMakeLists imbriqué : l'ancien balayage ne lisait que cmake/*.cmake et le
# CMakeLists de la racine, en laissant les huit de src/ — ceux-là mêmes qui
# déclarent les cibles de test et de benchmark.
expect_gate_closes \
    "-flto= codé en dur dans un CMakeLists de src/" \
    "parallelism" \
    "${NESTED_CMAKE_SOURCE}" \
    'target_compile_options(subedit_core PRIVATE -flto=8)'

# `-j` sans valeur prend autant de processus que la machine a de cœurs : c'est
# pire qu'un nombre en dur, et ça passait.
expect_gate_closes \
    "make -j sans valeur" \
    "parallelism" \
    "${PLAIN_SCRIPT_SOURCE}" \
    'make -j'

expect_gate_closes \
    "cmake --build --parallel avec un nombre" \
    "parallelism" \
    "${PLAIN_SCRIPT_SOURCE}" \
    'cmake --build . --parallel 8'

expect_gate_closes \
    "parallélisme passé par MAKEFLAGS" \
    "parallelism" \
    "${PLAIN_SCRIPT_SOURCE}" \
    'MAKEFLAGS=-j8 make all'

# Un test qui lit le dépôt de référence. Il passerait ici, où le clone existe,
# et échouerait partout ailleurs — ou se déclarerait ignoré, ce qui est pire :
# vert sans rien prouver.
expect_gate_closes \
    "lecture du dépôt de référence" \
    "arch" \
    "${TEST_SOURCE}" \
    '// reference/gaupol/aeidon/data/patterns'


# La seule preuve de non-signalement du fichier, et la plus importante des six
# ajoutées ici : le contrôle lisait ses lignes sans retirer les commentaires de
# fin, donc un exemple cité dans un commentaire passait pour un contournement.
# Aucune injection qui échoue ne peut démontrer qu'un faux positif a disparu.
expect_gate_stays_open \
    "commentaire de fin de ligne citant -j 4" \
    "parallelism" \
    "${PLAIN_SCRIPT_SOURCE}" \
    'cmake --build . # exemple : -j 4 pour aller plus vite'

# La preuve que le périmètre restreint de clang-tidy ne peut pas rendre un vert
# vide.
#
# **Le défaut qu'elle attrape a existé.** tidy-scope.sh calculait son périmètre
# avec `git diff base...HEAD`, qui ne voit que ce qui est commité : juste en
# intégration continue, faux en local, où la porte se lance avant de commiter.
# Sur une branche sans commit, il rendait zéro fichier — donc une porte verte
# qui n avait rien analysé.
#
# Elle ne peut pas passer par expect_gate_closes : la cible porte ici une
# affectation, et cette fonction passe son argument en un seul mot.
expect_restricted_tidy_closes() {
    printf '%s▸ %s%s\n' "${BOLD}" "défaut dans un fichier non commité, périmètre restreint" "${RESET}"
    printf '\nnamespace { int probeForTheProof() { int value = 1; return value; } }\n' \
        >> "${LIB_SOURCE}"

    if make -C "${REPO_ROOT}" --no-print-directory tidy TIDY_BASE=HEAD >/dev/null 2>&1; then
        printf '  %s✗ la porte « tidy » a laissé passer le défaut%s\n' "${RED}" "${RESET}"
        failures=$((failures + 1))
    else
        printf '  %s✓ « make tidy TIDY_BASE=HEAD » a échoué, comme attendu%s\n' "${GREEN}" "${RESET}"
    fi

    restore
}

expect_restricted_tidy_closes

# Un fichier engendré déposé sous src/. Il passerait les quatre portes qui
# filtrent sur src/ — format, analyse statique, couverture, périmètre — parce
# qu aucune ne sait distinguer une ligne écrite d une ligne produite.
#
# L injection porte le marqueur qu écrivent moc, uic et rcc, assemblé ici pour
# la même raison que dans le contrôle : écrit en clair, il ferait de ce script
# un fichier engendré aux yeux du contrôle.
expect_generated_source_closes() {
    local marker="All changes made in this file"
    local victim="${REPO_ROOT}/src/lib/subedit/core/moc_probe.cpp"

    printf '%s▸ %s%s\n' "${BOLD}" "fichier engendré déposé sous src/" "${RESET}"
    printf '/**** %s will be lost! ****/\n' "${marker}" > "${victim}"

    if make -C "${REPO_ROOT}" --no-print-directory arch >/dev/null 2>&1; then
        printf '  %s✗ la porte « arch » a laissé passer le défaut%s\n' "${RED}" "${RESET}"
        failures=$((failures + 1))
    else
        printf '  %s✓ « make arch » a échoué, comme attendu%s\n' "${GREEN}" "${RESET}"
    fi

    rm -f "${victim}"
    restore
}

expect_generated_source_closes

printf '\n'
if (( failures > 0 )); then
    printf '%s%d porte(s) laissent passer un défaut%s\n' "${RED}" "${failures}" "${RESET}" >&2
    exit 1
fi
printf '%sles vingt-cinq portes se referment%s\n' "${GREEN}" "${RESET}"
printf '%set le contrôle de parallélisme laisse passer le code légitime%s\n' \
    "${GREEN}" "${RESET}"
