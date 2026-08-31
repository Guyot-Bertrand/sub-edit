#!/usr/bin/env bash
# Prouve que les portes de qualité se referment.
#
# Une porte qu'on n'a jamais vue échouer n'est pas une porte : c'est une
# croyance. Ce script injecte délibérément un défaut de chaque type, vérifie que
# la cible correspondante échoue, puis rétablit les sources.
#
# Les six premières injections visent des étapes de `make check`, que la CI
# exécute. Les cinq suivantes visent `make check-local` — quatre fois
# `requirements`, une fois `parallelism` — qui ne gate donc que le poste de
# développement : raison de plus pour que ce script prouve que chacune se
# referme, puisque rien d'autre ne les exercera.
#
# **Les quatre de `requirements` sont ses quatre défauts** : une
# exigence implémentée que rien ne cite, un tag qui ne désigne aucune exigence,
# une exigence du registre absente de la table de sa spec, et une exigence
# qu'une spec promet et que le registre ignore. Les deux dernières sont nées à
# la relecture de la phase 7, après deux phases où l'écart est passé.
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
# La dernière ne vise aucune porte : clean-stale-coverage.sh répare au lieu de
# refuser, et ce qui peut être faux chez lui va dans les deux sens — garder un
# arbre incohérent, ou écarter un arbre sain.
#
# Les six suivantes visent `make parallelism` : deux familles de fichiers qui
# échappaient au balayage, trois formes qu'il ne reconnaissait pas, et **une
# preuve d'un genre différent** — que du code légitime n'est pas signalé. Un
# critère de non-signalement ne se démontre par aucune injection qui échoue,
# d'où `expect_gate_stays_open`.
#
# Les deux suivantes visent `make arch` : un nom de cas de test qui commence par
# un tiret, et un test qui lirait le dépôt de référence. Elle est ici parce que ce défaut ne se voit qu'à travers CTest, jamais
# en lançant le binaire de test à la main — donc uniquement dans la porte.
#
# Les deux suivantes visent `make fixtures` : une fixture vidéo refabriquée avec
# la mauvaise fréquence d'image, et une position de fixture de grille déplacée
# d'une milliseconde. C'est la seule injection du script qui ne soit pas
# du texte — un conteneur est illisible dans un diff, et c'est précisément ce
# qui fait que rien d'autre n'attraperait le défaut. Elle est aussi la seule à
# passer par ffmpeg plutôt que par un extrait ajouté en fin de fichier :
# ajouter du texte à la fin d'un MP4 ne le change pas, ffprobe l'ayant déjà lu.
#
# Les deux suivantes visent `make untracked`, la porte qui refuse ce qu'une
# exécution laisse derrière elle : un fichier apparu pendant les tests, et — de
# l'autre genre, celui d'`expect_gate_stays_open` — un fichier non suivi qui
# était déjà là et qu'il ne faut surtout pas signaler.
#
# La suivante vise `make install-check` — #239 : des règles `install()` qui
# oublient un fichier de données. C'est le seul contrôle du dépôt qui regarde une
# installation propre, et le défaut qu'il attrape est invisible partout ailleurs
# — dans l'arbre de construction, le fichier est là parce que le dépôt le
# contient, pas parce qu'une règle l'a copié.
#
# Les deux suivantes visent les captures d'écran du manuel — #199 : une
# référence qui ne correspond plus à la fenêtre, que `make screenshots-check`
# doit refuser, et une image que le manuel montre sans que rien ne l'engendre,
# que `check-screenshots.py` doit attraper. Le second défaut est le plus
# coûteux des deux, et le seul que le comparateur ne peut pas voir : une image
# périmée s'affiche aussi proprement qu'une image juste.
#
# Les deux suivantes visent `make config-home`, le pendant de la précédente de
# l'autre côté de la frontière du dépôt : une configuration écrite pendant les
# tests, et — du genre d'`expect_gate_stays_open` — une configuration déjà là que
# rien n'a touchée. Elles s'exercent sous un `XDG_CONFIG_HOME` de fortune, si
# bien que la preuve d'une porte qui refuse qu'on écrive dans le répertoire
# personnel n'y écrit elle-même jamais.
#
# Les deux dernières ne visent aucune porte : `src/scripts/record-bench.sh`, qui
# ne refuse rien mais choisit ce qu'il inscrit dans une table qu'on n'élague
# jamais — un extrême posé par du bruit est définitif — et
# `src/scripts/prune-runs.sh`, qui ne refuse rien mais choisit ce qu'il
# supprime. Elle est ici faute d'un meilleur
# endroit — ce script est le seul harnais du projet pour ce qui s'écrit en
# shell, et une sélection irréversible qu'aucune exécution ne vérifie serait
# elle aussi une croyance.
#
# À rejouer après toute modification de .clang-format, .clang-tidy, des options
# de compilation, du cliquet de couverture, du registre d'exigences ou de
# cliff.toml.
#
# Exige git-cliff, que le contrôle du journal appelle par `make changelog`, et
# ffmpeg pour la fixture vidéo : ./src/scripts/setup-toolchain.sh installe les
# deux.

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
readonly MODEL_SOURCE="${REPO_ROOT}/src/lib/subedit/core/model/subtitle_index.hpp"
readonly PR_CHECK="${REPO_ROOT}/src/scripts/check-pull-request.sh"
readonly PRUNE_SCRIPT="${REPO_ROOT}/src/scripts/prune-runs.sh"
readonly VIDEO_FIXTURE="${REPO_ROOT}/src/test/data/videos/cadence-25.mp4"
readonly GRID_FIXTURE="${REPO_ROOT}/src/test/data/grilles/grille-25.srt"
readonly GUI_MANUAL_SOURCE="${REPO_ROOT}/docs/manual/subedit-gui/table.md"
readonly CAPTURE_REFERENCE="${REPO_ROOT}/docs/manual/subedit-gui/captures/table.png"
readonly INSTALLATION_SOURCE="${REPO_ROOT}/cmake/Installation.cmake"
readonly DESKTOP_SOURCE="${REPO_ROOT}/packaging/io.github.guyot_bertrand.subedit.desktop"
readonly ICON_SOURCE="${REPO_ROOT}/packaging/io.github.guyot_bertrand.subedit.svg"
readonly PACKAGING_SOURCE="${REPO_ROOT}/cmake/Packaging.cmake"
# La spec dont la table d exigences est confrontée au registre. La phase 3 est
# choisie parce qu elle porte le plus de lignes : une injection y est perdue
# dans la foule, ce qui est bien le cas qu on veut prouver.
readonly SPEC_SOURCE="${REPO_ROOT}/docs/specs/03-cli.md"
# La configuration de clang-tidy, dont la preuve 3 durcit une règle.
readonly TIDY_CONFIG="${REPO_ROOT}/.clang-tidy"
readonly OTHER_CAPTURE="${REPO_ROOT}/docs/manual/subedit-gui/captures/decalage.png"
# Le fichier témoin de la preuve des fichiers laissés derrière. Il ne sauvegarde
# rien : il est créé par la preuve et doit disparaître avec elle, y compris si
# le script meurt — sans quoi cette preuve-là laisserait justement un fichier
# derrière elle.
readonly STRAY_FILE="${REPO_ROOT}/laisse-par-un-test.srt"

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
    cp "${backup_dir}/subtitle_index.hpp" "${MODEL_SOURCE}"
    cp "${backup_dir}/cadence-25.mp4" "${VIDEO_FIXTURE}"
    cp "${backup_dir}/grille-25.srt" "${GRID_FIXTURE}"
    cp "${backup_dir}/table.md" "${GUI_MANUAL_SOURCE}"
    cp "${backup_dir}/table.png" "${CAPTURE_REFERENCE}"
    cp "${backup_dir}/Installation.cmake" "${INSTALLATION_SOURCE}"
    cp "${backup_dir}/subedit.desktop" "${DESKTOP_SOURCE}"
    cp "${backup_dir}/subedit.svg" "${ICON_SOURCE}"
    cp "${backup_dir}/Packaging.cmake" "${PACKAGING_SOURCE}"
    cp "${backup_dir}/03-cli.md" "${SPEC_SOURCE}"
    cp "${backup_dir}/clang-tidy" "${TIDY_CONFIG}"
    rm -f "${STRAY_FILE}"
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
cp "${MODEL_SOURCE}" "${backup_dir}/subtitle_index.hpp"
cp "${VIDEO_FIXTURE}" "${backup_dir}/cadence-25.mp4"
cp "${GRID_FIXTURE}" "${backup_dir}/grille-25.srt"
cp "${GUI_MANUAL_SOURCE}" "${backup_dir}/table.md"
cp "${CAPTURE_REFERENCE}" "${backup_dir}/table.png"
cp "${INSTALLATION_SOURCE}" "${backup_dir}/Installation.cmake"
cp "${DESKTOP_SOURCE}" "${backup_dir}/subedit.desktop"
cp "${ICON_SOURCE}" "${backup_dir}/subedit.svg"
cp "${PACKAGING_SOURCE}" "${backup_dir}/Packaging.cmake"
cp "${SPEC_SOURCE}" "${backup_dir}/03-cli.md"
cp "${TIDY_CONFIG}" "${backup_dir}/clang-tidy"
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
#
# **La même ligne est ajoutée à la spec de la phase**, et l'appelant fait donc
# l'injection lui-même. Sans cela, le contrôle du cadrage échouerait lui aussi —
# une exigence au registre et absente de sa spec — et la preuve ne dirait plus
# rien du contrôle qu'elle vise : elle passerait même si la confrontation aux
# tests était cassée. Une preuve qui a deux causes n'en prouve aucune.
printf '%s\n' '| `CLI-FANTOME-01` | exigence injectée que rien ne démontre | 3 | implémentée |' \
    >> "${REGISTRY}"
printf '%s\n' '| `CLI-FANTOME-01` | exigence injectée que rien ne démontre |' >> "${SPEC_SOURCE}"
expect_gate_closes \
    "exigence implémentée sans test" \
    "requirements" \
    "${REGISTRY}" \
    ''

# Exigence du registre que la table de sa spec ne porte pas. Elle est `prévue`
# et non `implémentée`, pour la raison symétrique de celle ci-dessus : une
# exigence prévue n'a besoin d'aucun test, donc la confrontation aux tests se
# tait et seul le cadrage parle.
expect_gate_closes \
    "exigence du registre absente de la table de sa spec" \
    "requirements" \
    "${REGISTRY}" \
    '| `CLI-FANTOME-02` | exigence injectée que sa spec ignore | 3 | prévue |'

# Le sens inverse : une exigence qu'une spec promet et que le registre ne porte
# pas. C'est le défaut exact que la phase 7 a trouvé chez elle — `GUI-THEME-02`
# fondue dans `GUI-THEME-01` sans que la spec le dise — et que rien ne voyait.
expect_gate_closes \
    "exigence promise par une spec et absente du registre" \
    "requirements" \
    "${SPEC_SOURCE}" \
    '| `CLI-FANTOME-03` | exigence promise par la spec et inconnue du registre |'

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
# En-tête du modèle qui inclut une opération. La frontière posée par l'ADR 0018
# s'était déjà effacée une fois, quand les types propres aux formats se sont
# retrouvés sous model/ sans que rien ne le signale : d'où une porte plutôt
# qu'une vigilance. L'injection porte sur un en-tête, et l'échec attendu vient
# du balayage de check-architecture.sh — l'inclusion en fin de fichier ne
# compile rien.
expect_gate_closes \
    "modèle qui dépend d'une opération" \
    "arch" \
    "${MODEL_SOURCE}" \
    '#include <subedit/core/format/read_result.hpp>'

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

# Une fixture vidéo refabriquée avec la mauvaise fréquence. C est le seul défaut
# que la table de video-fixtures.sh existe pour attraper, et le seul que
# personne ne verrait autrement : un conteneur est illisible dans un diff.
#
# L injection est faite ici plutôt que passée en extrait, parce qu ajouter du
# texte a la fin d un MP4 ne le change pas — ffprobe lit le fichier avant.
ffmpeg -v error -y \
    -f lavfi -i 'color=c=black:s=16x16:r=30:d=2' \
    -c:v mpeg4 -qscale:v 31 -pix_fmt yuv420p \
    -fflags +bitexact -flags:v +bitexact -map_metadata -1 \
    "${VIDEO_FIXTURE}"

expect_gate_closes \
    "fixture vidéo à la mauvaise fréquence" \
    "fixtures" \
    "${VIDEO_FIXTURE}" \
    ''

# Une position de fixture de grille déplacée d une milliseconde. C est le plus
# petit défaut que subtitle-fixtures.py existe pour attraper, et le plus
# instructif : une milliseconde ne se voit pas dans un fichier de cent soixante-
# dix répliques, et elle suffit à sortir une position de la grille.
#
# L injection est faite ici plutôt que passée en extrait, parce qu ajouter du
# texte en fin de fichier serait un défaut grossier. Ce qui est prouvé, c est
# que la porte lit les positions, et non la taille du fichier.
sed -i '0,/00:00:01,000/s//00:00:01,001/' "${GRID_FIXTURE}"

expect_gate_closes \
    "position de grille déplacée d une milliseconde" \
    "fixtures" \
    "${GRID_FIXTURE}" \
    ''


# La seule preuve de non-signalement du fichier, et la plus importante des six
# ajoutées ici : le contrôle lisait ses lignes sans retirer les commentaires de
# fin, donc un exemple cité dans un commentaire passait pour un contournement.
# Aucune injection qui échoue ne peut démontrer qu'un faux positif a disparu.
expect_gate_stays_open \
    "commentaire de fin de ligne citant -j 4" \
    "parallelism" \
    "${PLAIN_SCRIPT_SOURCE}" \
    'cmake --build . # exemple : -j 4 pour aller plus vite'

# Les trois preuves de l analyse statique — issue #269.
#
# **Elles ont changé de nature avec le mécanisme.** Il y en avait une, et elle
# attrapait un défaut réel de tidy-scope.sh : il calculait son périmètre avec
# `git diff base...HEAD`, qui ne voit que ce qui est commité, donc rendait zéro
# fichier sur une branche sans commit — une porte verte qui n avait rien
# analysé. Ce script-là n existe plus : clang-tidy est accroché à la règle de
# compilation de chaque source, et git n entre plus dans le calcul.
#
# Restent trois choses à prouver, une par entrée que le système de construction
# doit voir : la source, ses en-têtes, et la clé qui porte le reste.
#
# La première invocation construit l arbre `build/tidy` en entier, ce qui coûte
# une dizaine de minutes ; les deux suivantes sont incrémentales, et c est
# précisément la propriété qu on éprouve.
expect_tidy_closes() {
    local label="$1"
    local file="$2"
    local snippet="$3"

    printf '%s▸ %s%s\n' "${BOLD}" "${label}" "${RESET}"
    printf '%s\n' "${snippet}" >> "${file}"

    if make -C "${REPO_ROOT}" --no-print-directory tidy >/dev/null 2>&1; then
        printf '  %s✗ la porte « tidy » a laissé passer le défaut%s\n' "${RED}" "${RESET}"
        failures=$((failures + 1))
    else
        printf '  %s✓ « make tidy » a échoué, comme attendu%s\n' "${GREEN}" "${RESET}"
    fi

    restore
}

# 1 — un défaut dans une source non commitée. C est le cas que l ancien
# mécanisme ratait, et il ne peut plus se poser : rien ne consulte git.
expect_tidy_closes \
    "défaut dans une source non commitée" \
    "${LIB_SOURCE}" \
    'namespace { int probeForTheProof() { int value = 1; return value; } }'

# 2 — **un défaut dans un en-tête**, et c est la preuve qui n existait pas.
#
# L ancienne fermeture d en-têtes se faisait par `grep` du nom de fichier dans
# les sources : elle attrapait un en-tête cité dans un commentaire et pouvait
# manquer une inclusion indirecte. Ici c est le fichier de dépendances que le
# compilateur a écrit qui décide, donc les unités qui incluent celui-ci sont
# réanalysées, et elles seules.
expect_tidy_closes \
    "défaut dans un en-tête, atteint par ses dépendants" \
    "${MODEL_SOURCE}" \
    'namespace subedit::core { inline int probeForTheProof() { int value = 1; return value; } }'

# 3 — **la configuration change, et l analyse le voit.**
#
# C est le trou que le mécanisme aurait laissé sans la clé de `cmake/Tidy.cmake`
# — voir ce fichier : le contenu de `.clang-tidy` n est pas sur la ligne de
# commande, donc Ninja ne le regarde pas. On y ajoute une règle de nommage que
# tout le projet viole ; sans la clé, l arbre resterait vert sur une réponse
# périmée, ce qui est le pire mode d échec possible.
#
# L injection se fait en fin de fichier, ce qui exige que la liste CheckOptions
# soit la dernière chose de `.clang-tidy` — c est écrit dans ce fichier.
expect_tidy_closes \
    "configuration durcie, analyse rejouée" \
    "${TIDY_CONFIG}" \
    '  - key: readability-identifier-naming.ParameterCase
    value: UPPER_CASE'

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

# La porte refuse ce qu une exécution a laissé derrière elle — #226.
#
# **Deux preuves et non une**, parce que le contrôle a deux modes d échec et
# qu ils ne coûtent pas la même chose. Laisser passer un fichier oublié est ce
# qui est arrivé une fois, à la #207, et personne ne l a su avant un git status.
# Signaler un fichier non suivi qui était déjà là — une source neuve avant son
# git add — serait pire : le contrôle crierait au loup à chaque journée de
# travail, et un contrôle qui crie au loup finit désactivé.
#
# Elle ne passe pas par expect_gate_closes : le défaut ne s injecte pas en
# ajoutant du texte à un fichier du dépôt, il consiste à en faire apparaître un.
expect_untracked_gate() {
    local script="${REPO_ROOT}/src/scripts/check-untracked.sh"

    printf '%s▸ un test qui laisse un fichier derrière lui%s\n' "${BOLD}" "${RESET}"
    "${script}" --record >/dev/null
    : > "${STRAY_FILE}"
    if make -C "${REPO_ROOT}" --no-print-directory untracked >/dev/null 2>&1; then
        printf '  %s✗ la porte « untracked » a laissé passer le fichier%s\n' "${RED}" "${RESET}"
        failures=$((failures + 1))
    else
        printf '  %s✓ « make untracked » a échoué, comme attendu%s\n' "${GREEN}" "${RESET}"
    fi
    rm -f "${STRAY_FILE}"

    printf '%s▸ un fichier non suivi déjà là avant le relevé%s\n' "${BOLD}" "${RESET}"
    : > "${STRAY_FILE}"
    "${script}" --record >/dev/null
    if make -C "${REPO_ROOT}" --no-print-directory untracked >/dev/null 2>&1; then
        printf '  %s✓ « make untracked » a laissé passer, comme attendu%s\n' "${GREEN}" "${RESET}"
    else
        printf '  %s✗ la porte « untracked » a signalé un fichier antérieur%s\n' "${RED}" "${RESET}"
        failures=$((failures + 1))
    fi
    rm -f "${STRAY_FILE}"

    # Le relevé est refait sans le témoin, pour que les preuves suivantes ne
    # partent pas d un état que celle-ci a écrit.
    "${script}" --record >/dev/null
}

expect_untracked_gate

# La porte refuse qu une exécution touche la configuration de l utilisateur.
#
# **Deux preuves et non une**, pour la raison qui en demande deux à la
# précédente. Laisser passer une configuration écrite est le défaut que la porte
# existe pour attraper. Signaler une configuration que rien n a touchée serait
# pire : qui développe subedit finit par lancer subedit, et un contrôle qui
# crierait au loup à chaque porte finirait désactivé.
#
# **Elle s exerce sous un XDG_CONFIG_HOME de fortune.** Prouver qu une porte
# refuse qu on écrive dans le répertoire personnel en y écrivant serait commettre
# le défaut pour montrer qu il est refusé. La variable est celle dont tout
# emplacement standard dérive, et c est précisément celle que le harnais des
# tests déplace : la preuve emprunte le mécanisme qu elle démontre.
expect_config_home_gate() {
    local script="${REPO_ROOT}/src/scripts/check-config-home.sh"
    local fake
    fake="$(mktemp -d)"

    printf '%s▸ un test qui écrit dans la configuration de l utilisateur%s\n' "${BOLD}" "${RESET}"
    XDG_CONFIG_HOME="${fake}" "${script}" --record >/dev/null
    mkdir -p "${fake}/subedit"
    printf 'theme = dark\n' > "${fake}/subedit/settings.conf"
    if XDG_CONFIG_HOME="${fake}" make -C "${REPO_ROOT}" --no-print-directory config-home \
        >/dev/null 2>&1; then
        printf '  %s✗ la porte « config-home » a laissé passer l écriture%s\n' "${RED}" "${RESET}"
        failures=$((failures + 1))
    else
        printf '  %s✓ « make config-home » a échoué, comme attendu%s\n' "${GREEN}" "${RESET}"
    fi

    printf '%s▸ une configuration déjà là que rien n a touchée%s\n' "${BOLD}" "${RESET}"
    XDG_CONFIG_HOME="${fake}" "${script}" --record >/dev/null
    if XDG_CONFIG_HOME="${fake}" make -C "${REPO_ROOT}" --no-print-directory config-home \
        >/dev/null 2>&1; then
        printf '  %s✓ « make config-home » a laissé passer, comme attendu%s\n' "${GREEN}" "${RESET}"
    else
        printf '  %s✗ la porte « config-home » a signalé une configuration intacte%s\n' \
            "${RED}" "${RESET}"
        failures=$((failures + 1))
    fi

    rm -rf "${fake}"

    # Le relevé est refait sous le vrai répertoire, pour que ce qui suit ne parte
    # pas de l empreinte d un XDG_CONFIG_HOME qui n existe plus.
    "${script}" --record >/dev/null
}

expect_config_home_gate

# Les captures du manuel — #199.
#
# **Deux preuves, pour les deux pièces qui ne voient pas la même chose.** Le
# comparateur ne voit que les captures qu une exécution vient de produire : il
# sait dire « cette image a bougé », et rien d autre. Le garde-fou voit les trois
# listes — ce que le programme engendre, ce que le manuel montre, ce qui existe
# sur le disque — et c est la seule pièce qui puisse attraper une image que plus
# rien ne réengendre.
expect_screenshot_gates() {
    printf '%s▸ une capture qui ne correspond plus à la fenêtre%s\n' "${BOLD}" "${RESET}"
    # Une autre capture du même dépôt, aux dimensions différentes : le défaut
    # est injecté sans fabriquer d image, et sans qu il faille en décrire une.
    cp "${OTHER_CAPTURE}" "${CAPTURE_REFERENCE}"
    if make -C "${REPO_ROOT}" --no-print-directory screenshots-check >/dev/null 2>&1; then
        printf '  %s✗ la porte « screenshots-check » a laissé passer l image%s\n' \
            "${RED}" "${RESET}"
        failures=$((failures + 1))
    else
        printf '  %s✓ « make screenshots-check » a échoué, comme attendu%s\n' \
            "${GREEN}" "${RESET}"
    fi
    restore

    printf '%s▸ une image que le manuel montre et que rien n engendre%s\n' "${BOLD}" "${RESET}"
    printf '\n![Une image que personne n engendre.](captures/inexistante.png)\n' \
        >> "${GUI_MANUAL_SOURCE}"
    if "${REPO_ROOT}/src/scripts/check-screenshots.py" >/dev/null 2>&1; then
        printf '  %s✗ le garde-fou a laissé passer l image absente%s\n' "${RED}" "${RESET}"
        failures=$((failures + 1))
    else
        printf '  %s✓ « check-screenshots.py » a refusé, comme attendu%s\n' "${GREEN}" "${RESET}"
    fi
    restore
}

expect_screenshot_gates

# Les renvois du manuel — #243.
#
# Le troisième filet du manuel, après les blocs engendrés et les captures. Une
# seule preuve : le contrôle n a pas de mode d échec par excès qui se démontre
# — il ne compare pas deux relevés, il résout des chemins et des ancres, et le
# vert du dépôt intact est déjà la preuve qu il ne crie pas au loup.
expect_manual_link_gate() {
    printf '%s▸ un renvoi du manuel qui ne mène nulle part%s\n' "${BOLD}" "${RESET}"
    printf '\n[Un renvoi vers rien](table.md#une-section-qui-nexiste-pas)\n' \
        >> "${GUI_MANUAL_SOURCE}"
    if "${REPO_ROOT}/src/scripts/check-manual-links.py" >/dev/null 2>&1; then
        printf '  %s✗ le contrôle a laissé passer l ancre absente%s\n' "${RED}" "${RESET}"
        failures=$((failures + 1))
    else
        printf '  %s✓ « check-manual-links.py » a refusé, comme attendu%s\n' \
            "${GREEN}" "${RESET}"
    fi
    restore
}

expect_manual_link_gate

# Les règles d installation oublient un fichier de données — #239.
#
# **L injection réduit les règles aux seuls binaires**, plutôt que d ajouter du
# texte à la fin du fichier : le défaut consiste ici à ne pas copier quelque
# chose, et rien de ce qu on ajoute en fin de fichier ne défait un `install()`
# écrit plus haut.
#
# Une seule preuve et non deux, contrairement aux portes voisines : ce contrôle
# n a pas de mode d échec par excès. Il ne compare pas deux relevés et ne
# signale rien qui préexiste — il installe dans un préfixe neuf à chaque
# exécution, donc il ne peut pas crier au loup sur l état de la machine.
expect_installation_gate() {
    printf '%s▸ des règles install() qui oublient le manuel%s\n' "${BOLD}" "${RESET}"

    cat > "${INSTALLATION_SOURCE}" <<'BROKEN'
include(GNUInstallDirs)
install(TARGETS subedit-cli subedit-gui RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")
BROKEN

    if make -C "${REPO_ROOT}" --no-print-directory install-check >/dev/null 2>&1; then
        printf '  %s✗ la porte « install-check » a laissé passer le manuel absent%s\n' \
            "${RED}" "${RESET}"
        failures=$((failures + 1))
    else
        printf '  %s✓ « make install-check » a échoué, comme attendu%s\n' "${GREEN}" "${RESET}"
    fi

    restore
}

expect_installation_gate

# Un fichier de bureau que sa validation refuse — #244.
#
# **Une preuve distincte de la précédente, et pas une redite.** Celle-ci prouve
# qu un fichier absent est vu ; celle-là prouve que la validation en est une. Un
# `.desktop` présent, installé, et invalide passerait la première sans être vu :
# le contrôle compterait un fichier là où il faut en lire un.
#
# Le défaut injecté est le plus discret que le format connaisse : un
# `Categories=` dont une valeur n existe pas. Le fichier reste lisible, le
# bureau l affiche, et `desktop-file-validate` est la seule chose qui le dise.
expect_desktop_validation_gate() {
    printf '%s▸ un fichier .desktop que sa validation refuse%s\n' "${BOLD}" "${RESET}"

    printf 'Categories=UneCategorieQuiNExistePas;\n' >> "${DESKTOP_SOURCE}"

    if make -C "${REPO_ROOT}" --no-print-directory install-check >/dev/null 2>&1; then
        printf '  %s✗ la porte « install-check » a laissé passer le .desktop invalide%s\n' \
            "${RED}" "${RESET}"
        failures=$((failures + 1))
    else
        printf '  %s✓ « make install-check » a échoué, comme attendu%s\n' "${GREEN}" "${RESET}"
    fi

    restore
}

expect_desktop_validation_gate

# Une icône que le bureau ne saurait pas lire — #260.
#
# **Le défaut injecté est celui qui a été livré**, à la lettre : un commentaire
# avant la balise racine, qui repousse « <svg » au-delà des 256 octets où
# gdk-pixbuf cherche la signature d un SVG. Le fichier reste un XML valide,
# Inkscape et Qt l ouvrent sans un mot, et le bureau affiche une tuile vide.
#
# **Deux preuves**, pour les deux moitiés du défaut d origine : le fichier
# qu aucun bureau ne reconnaît, et l image parfaitement valide qui ne se voit
# pas sur un fond sombre. La seconde est celle qu on oublie, parce qu elle ne
# ressemble pas à une panne.
expect_icon_gates() {
    printf '%s▸ une icône que gdk-pixbuf ne reconnaît pas%s\n' "${BOLD}" "${RESET}"

    # Trois cents caractères devant la racine : au-delà de la fenêtre de 256.
    {
        printf '<?xml version="1.0" encoding="UTF-8"?>\n<!-- '
        printf 'x%.0s' {1..300}
        printf ' -->\n'
        tail -n +2 "${backup_dir}/subedit.svg"
    } > "${ICON_SOURCE}"

    if make -C "${REPO_ROOT}" --no-print-directory install-check >/dev/null 2>&1; then
        printf '  %s✗ la porte « install-check » a laissé passer l icône illisible%s\n' \
            "${RED}" "${RESET}"
        failures=$((failures + 1))
    else
        printf '  %s✓ « make install-check » a échoué, comme attendu%s\n' "${GREEN}" "${RESET}"
    fi

    restore

    printf '%s▸ une icône qui ne se voit pas sur un fond sombre%s\n' "${BOLD}" "${RESET}"

    cat > "${ICON_SOURCE}" <<'INVISIBLE'
<?xml version="1.0" encoding="UTF-8"?>
<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 64 64" width="64" height="64">
  <rect x="4" y="10" width="56" height="44" rx="5" fill="#2d3646"/>
  <rect x="8" y="14" width="48" height="36" rx="2" fill="#1b2230"/>
</svg>
INVISIBLE

    if make -C "${REPO_ROOT}" --no-print-directory install-check >/dev/null 2>&1; then
        printf '  %s✗ la porte « install-check » a laissé passer l icône invisible%s\n' \
            "${RED}" "${RESET}"
        failures=$((failures + 1))
    else
        printf '  %s✓ « make install-check » a échoué, comme attendu%s\n' "${GREEN}" "${RESET}"
    fi

    restore
}

expect_icon_gates

# Un `.rpm` qui revendique les répertoires de la distribution — #266.
#
# **L injection reproduit le défaut historique à l identique** : la liste
# d exclusions écrite en chemins relatifs, `share/applications` au lieu de
# `/usr/share/applications`. CPack compare des chemins de paquet, donc absolus,
# et ne trouve alors aucune correspondance — l exclusion existe, elle est
# lisible, et elle n exclut rien.
#
# C est ce qui rend cette preuve nécessaire plutôt que décorative. Le défaut
# ne se voyait ni dans le diff, ni dans `rpm -qlp`, ni nulle part avant qu une
# vraie Fedora refuse la transaction — pendant tout ce temps, la ligne était
# là, et la seule chose qu on aurait pu vérifier est ce que ce contrôle vérifie.
expect_rpm_directory_gate() {
    printf '%s▸ un .rpm qui revendique les répertoires de la distribution%s\n' "${BOLD}" "${RESET}"

    sed -i 's|"${CPACK_PACKAGING_INSTALL_PREFIX}/|"|g' "${PACKAGING_SOURCE}"

    if make -C "${REPO_ROOT}" --no-print-directory install-check >/dev/null 2>&1; then
        printf '  %s✗ la porte « install-check » a laissé passer les répertoires partagés%s\n' \
            "${RED}" "${RESET}"
        failures=$((failures + 1))
    else
        printf '  %s✓ « make install-check » a échoué, comme attendu%s\n' "${GREEN}" "${RESET}"
    fi

    restore
}

expect_rpm_directory_gate

# Une preuve d un troisième genre. prune-runs.sh n est pas une porte : il ne
# refuse rien, il choisit. Ce qui peut être faux chez lui n est donc pas de
# laisser passer un défaut, mais de supprimer une exécution qu il fallait garder
# — et cette erreur-là est irréversible, GitHub ne rend pas un run supprimé.
#
# Elle ne peut pas s exercer contre GitHub. Un jeton personnel reçoit 403 sur
# les Actions, et faire dépendre une preuve du réseau la rendrait rouge un jour
# de panne, pour une raison étrangère au dépôt. D où --input, qui donne au
# script sa liste au lieu de la lui faire chercher : la sélection est une
# fonction de la liste, donc elle se démontre sur une liste écrite à la main.
#
# Le jeu couvre les cinq cas qui décident du fichier :
#
#   ids 1 à 35    « ci », terminées      les 30 plus récentes restent, les 5
#                                        plus vieilles partent
#   id 99         « ci », main, la plus  part : « la plus récente sur main », au
#                 vieille de toutes      singulier, n épargne pas les autres
#   id 100        « ci », main, vieille  reste : c est la plus récente sur main
#   id 101        « ci », en cours       reste : GitHub refuse de supprimer une
#                                        exécution non terminée
#   ids 200 à 202 « pull request »       rien ne part : moins de 30
#
# Le cas 99 est celui qui compte le plus. Sans lui, « garder le plus récent sur
# main » se lirait « garder tous ceux sur main » — et ressusciterait les
# quarante-sept exécutions du déclencheur push supprimé à la #106, que cet
# élagage existe précisément pour effacer.
expect_prune_selection_holds() {
    local fixture
    local expected="1 2 3 4 5 99"
    local actual

    printf '%s▸ %s%s\n' "${BOLD}" "choix des exécutions à supprimer" "${RESET}"

    fixture="$(mktemp)"
    local index
    for index in $(seq 1 35); do
        printf '{"id":%d,"workflow_id":1,"name":"ci","head_branch":"feat/x","status":"completed","created_at":"2026-01-01T00:%02d:00Z"}\n' \
            "${index}" "${index}" >> "${fixture}"
    done
    printf '%s\n' \
        '{"id":99,"workflow_id":1,"name":"ci","head_branch":"main","status":"completed","created_at":"2024-01-01T00:00:00Z"}' \
        '{"id":100,"workflow_id":1,"name":"ci","head_branch":"main","status":"completed","created_at":"2025-01-01T00:00:00Z"}' \
        '{"id":101,"workflow_id":1,"name":"ci","head_branch":"feat/y","status":"in_progress","created_at":"2025-01-02T00:00:00Z"}' \
        '{"id":200,"workflow_id":2,"name":"pull request","head_branch":"feat/z","status":"completed","created_at":"2026-01-01T01:00:00Z"}' \
        '{"id":201,"workflow_id":2,"name":"pull request","head_branch":"feat/z","status":"completed","created_at":"2026-01-01T01:01:00Z"}' \
        '{"id":202,"workflow_id":2,"name":"pull request","head_branch":"feat/z","status":"completed","created_at":"2026-01-01T01:02:00Z"}' \
        >> "${fixture}"

    # Le statut est recueilli plutôt que subi : sous `set -e`, un script absent
    # ou qui meurt arrêterait le harnais au lieu de compter une preuve en
    # échec, et les preuves suivantes ne seraient jamais exécutées.
    local status=0
    actual="$("${PRUNE_SCRIPT}" --dry-run --input "${fixture}" | sort -n | tr '\n' ' ')" \
        || status=$?
    actual="${actual% }"
    rm -f "${fixture}"

    if (( status != 0 )); then
        printf '  %s✗ la sélection est morte, code %d%s\n' "${RED}" "${status}" "${RESET}"
        failures=$((failures + 1))
    elif [[ "${actual}" == "${expected}" ]]; then
        printf '  %s✓ la sélection donne « %s », comme attendu%s\n' "${GREEN}" "${actual}" "${RESET}"
    else
        printf '  %s✗ la sélection donne « %s », attendu « %s »%s\n' \
            "${RED}" "${actual}" "${expected}" "${RESET}"
        failures=$((failures + 1))
    fi
}

expect_prune_selection_holds

# Une preuve d un quatrième genre, et du même esprit que la précédente.
# clean-stale-coverage.sh ne refuse rien non plus : il répare. Ce qui peut être
# faux chez lui est donc de laisser passer un arbre incohérent — un .gcno qu un
# déplacement de source a orphelin, et qui fait échouer gcovr sur un message qui
# ne nomme ni le fichier fautif ni le remède.
#
# Et l erreur symétrique compte autant : écarter un arbre sain coûterait une
# reconstruction complète à chaque mesure, ce que personne ne remarquerait
# autrement que par une lenteur inexpliquée. Les deux sens sont donc éprouvés.
#
# Sur un arbre de mensonge plutôt que sur le vrai : la mécanique est une
# comparaison de listes, elle se démontre sur des listes écrites à la main, et
# rien ici ne doit dépendre d une compilation de vingt minutes.
expect_stale_coverage_is_cleared() {
    local build
    local script="${REPO_ROOT}/src/scripts/clean-stale-coverage.sh"

    printf '%s▸ %s%s\n' "${BOLD}" "arbre de couverture périmé par un déplacement" "${RESET}"

    build="$(mktemp -d)"
    touch "${build}/temoin.gcno"

    # Premier passage : pas de relevé, donc rien à comparer et rien à écarter.
    local status=0
    "${script}" --build-dir "${build}" >/dev/null || status=$?

    if (( status != 0 )) || [[ ! -f "${build}/temoin.gcno" ]]; then
        printf '  %s✗ le premier passage a écarté un arbre qu il ne pouvait pas juger%s\n' \
            "${RED}" "${RESET}"
        failures=$((failures + 1))
        rm -rf "${build}"
        return
    fi

    # Deuxième passage : rien n a bougé depuis le relevé, l arbre doit survivre.
    "${script}" --build-dir "${build}" >/dev/null || status=$?
    if [[ ! -f "${build}/temoin.gcno" ]]; then
        printf '  %s✗ un arbre sain a été écarté%s\n' "${RED}" "${RESET}"
        failures=$((failures + 1))
        rm -rf "${build}"
        return
    fi

    # Troisième passage : une source du relevé a disparu, comme après un
    # déplacement. L arbre entier doit partir, témoin compris.
    printf 'src/lib/subedit/core/deplacee.cpp\n' >> "${build}/.sources"
    "${script}" --build-dir "${build}" >/dev/null || status=$?

    if [[ -f "${build}/temoin.gcno" ]]; then
        printf '  %s✗ un arbre orphelin a été gardé%s\n' "${RED}" "${RESET}"
        failures=$((failures + 1))
    else
        printf '  %s✓ sain gardé, orphelin écarté%s\n' "${GREEN}" "${RESET}"
    fi

    rm -rf "${build}"
}

expect_stale_coverage_is_cleared

# Une preuve du même esprit que les deux précédentes : record-bench.sh ne refuse
# rien non plus, il choisit ce qu il inscrit dans la table des extrêmes. Et ce
# qu il y inscrit est **définitif** — la table n est jamais élaguée — donc une
# valeur posée par du bruit rend la mesure aveugle à toute régression plus
# petite que ce bruit.
#
# Le défaut qu elle ferme a été trouvé à l usage, en phase 6 : une mesure neuve
# posait ses deux extrêmes au premier relevé qui la portait, **même sale**.
# Faute d avoir quelque chose à comparer, la règle du seuil de charge n avait
# rien à refuser, et ne s appliquait donc jamais à ce qui venait de naître.
#
# Sur un journal jetable plutôt que sur celui du dépôt : ce que le script décide
# est une fonction du journal qu il lit, d où --journal, comme prune-runs.sh a
# --input.
expect_bench_extremes_hold() {
    local script="${REPO_ROOT}/src/scripts/record-bench.sh"
    local work
    local journal
    local status=0

    printf '%s▸ %s%s\n' "${BOLD}" "choix des extrêmes du journal des mesures" "${RESET}"

    work="$(mktemp -d)"
    journal="${work}/performances.md"

    write_bench_xml() {
        printf '%s\n' \
            '<?xml version="1.0" encoding="UTF-8"?>' \
            '<Catch2TestRun name="verify">' \
            '  <TestCase name="t">' \
            "    <BenchmarkResults name=\"$1\" samples=\"1\" resamples=\"0\" iterations=\"1\" clockResolution=\"1\" estimatedDuration=\"1\">" \
            "      <mean value=\"$2\" lowerBound=\"$2\" upperBound=\"$2\" ci=\"0.95\"/>" \
            '      <standardDeviation value="1" lowerBound="1" upperBound="1" ci="0.95"/>' \
            '    </BenchmarkResults>' \
            '  </TestCase>' \
            '</Catch2TestRun>' > "${work}/run.xml"
    }

    printf '%s\n' \
        '# Journal' '' '<!-- extrêmes -->' '' \
        '| Mesure | Minimum | Relevé le | Maximum | Relevé le |' \
        '| :----- | ------: | :-------- | ------: | :-------- |' \
        '| ancienne | 100 ns | 0.0.1 — 2020-01-01 | 300 ns | 0.0.1 — 2020-01-01 |' '' \
        '<!-- ancienne min=100.0 max=300.0 -->' '' \
        '## Relevés' '' '<!-- relevés -->' '' > "${journal}"

    # Premier passage, machine occupée : la mesure neuve entre au relevé et
    # **pas** dans la table.
    write_bench_xml "neuve" 1000
    "${script}" --xml "${work}/run.xml" --mode Release --load 9 --below 1.5 \
        --journal "${journal}" >/dev/null 2>&1 || status=$?
    # Le commentaire brut, et non la ligne du tableau : le relevé courant
    # porte lui aussi une ligne « | neuve | … », et la chercher là reviendrait
    # à vérifier que la mesure est absente du journal — ce qu'elle ne doit pas
    # être. Les commentaires bruts n'existent que dans la table des extrêmes.
    if (( status != 0 )) || grep -q '<!-- neuve min=' "${journal}"; then
        printf '  %s✗ une mesure neuve a posé ses extrêmes sur un relevé sale%s\n' \
            "${RED}" "${RESET}"
        failures=$((failures + 1))
        rm -rf "${work}"
        return
    fi

    # Deuxième passage, machine calme : elle y entre.
    write_bench_xml "neuve" 800
    "${script}" --xml "${work}/run.xml" --mode Release --load 0.5 --below 1.5 \
        --journal "${journal}" >/dev/null 2>&1 || status=$?
    if ! grep -q '<!-- neuve min=800.0 max=800.0 -->' "${journal}"; then
        printf '  %s✗ un relevé calme n a pas posé les extrêmes d une mesure neuve%s\n' \
            "${RED}" "${RESET}"
        failures=$((failures + 1))
        rm -rf "${work}"
        return
    fi

    # Troisième passage, même version, calme, **pire** : l extrême que ce relevé
    # remplace n a plus de source, donc il est repris — sans quoi la table
    # citerait, pour une version présente au journal, un chiffre qu elle n y
    # montre plus.
    write_bench_xml "neuve" 950
    "${script}" --xml "${work}/run.xml" --mode Release --load 0.5 --below 1.5 \
        --journal "${journal}" >/dev/null 2>&1 || status=$?
    if ! grep -q '<!-- neuve min=950.0 max=950.0 -->' "${journal}"; then
        printf '  %s✗ un extrême posé par le relevé remplacé n a pas été repris%s\n' \
            "${RED}" "${RESET}"
        failures=$((failures + 1))
        rm -rf "${work}"
        return
    fi

    # Et l enveloppe d une autre version ne bouge pas pour autant.
    if ! grep -q '<!-- ancienne min=100.0 max=300.0 -->' "${journal}"; then
        printf '  %s✗ l enveloppe d une autre version a été touchée%s\n' \
            "${RED}" "${RESET}"
        failures=$((failures + 1))
        rm -rf "${work}"
        return
    fi

    printf '  %s✓ neuve écartée à chaud, posée au calme, reprise quand sa source est remplacée%s\n' \
        "${GREEN}" "${RESET}"
    rm -rf "${work}"
}

expect_bench_extremes_hold

printf '\n'
# Les deux gardes de l orchestrateur — issue #269.
#
# **Un filtre qui ne retient rien doit refuser, pas rendre zéro.** `gate.sh`
# sait reprendre une suite au milieu, ce qui ouvre un mode d échec qui
# n existait pas quand le Makefile enchaînait ses cibles : une faute de frappe
# dans `--from` jouerait zéro étape et rendrait un code de succès. C est la
# même famille que le périmètre vide de l analyse statique, et elle se referme
# de la même façon — bruyamment.
expect_orchestrator_refuses() {
    local label="$1"
    shift

    printf '%s▸ %s%s\n' "${BOLD}" "${label}" "${RESET}"

    if "${REPO_ROOT}/src/scripts/gate.sh" "$@" >/dev/null 2>&1; then
        printf '  %s✗ gate.sh a accepté « %s »%s\n' "${RED}" "$*" "${RESET}"
        failures=$((failures + 1))
    else
        printf '  %s✓ gate.sh a refusé, comme attendu%s\n' "${GREEN}" "${RESET}"
    fi
}

expect_orchestrator_refuses "étape inconnue passée à --from" check --from covrage
expect_orchestrator_refuses "filtres qui ne retiennent aucune étape" check --only tidy --skip tidy

if (( failures > 0 )); then
    printf '%s%d preuve(s) en échec%s\n' "${RED}" "${failures}" "${RESET}" >&2
    exit 1
fi
printf '%sles quarante-quatre portes se referment%s\n' "${GREEN}" "${RESET}"
printf '%sle contrôle de parallélisme laisse passer le code légitime%s\n' \
    "${GREEN}" "${RESET}"
printf '%set l élagueur choisit les exécutions attendues%s\n' \
    "${GREEN}" "${RESET}"
printf '%set le journal des mesures ne pose un extrême que sur un relevé propre%s\n' \
    "${GREEN}" "${RESET}"
printf '%set un fichier non suivi antérieur aux tests ne fait pas crier au loup%s\n' \
    "${GREEN}" "${RESET}"
printf '%sni une configuration d utilisateur que rien n a touchée%s\n' \
    "${GREEN}" "${RESET}"
