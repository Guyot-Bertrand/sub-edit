#!/usr/bin/env bash
# Prouve que les portes de qualité se referment.
#
# Une porte qu'on n'a jamais vue échouer n'est pas une porte : c'est une
# croyance. Ce script injecte délibérément un défaut de chaque type, vérifie que
# la cible correspondante échoue, puis rétablit les sources.
#
# Les cinq premières injections visent des étapes de `make check`, que la CI
# exécute. Les trois dernières visent `make check-local` — deux fois
# `requirements`, une fois `parallelism` — qui ne gate donc que le poste de
# développement : raison de plus pour que ce script prouve que chacune se
# referme, puisque rien d'autre ne les exercera.
#
# À rejouer après toute modification de .clang-format, .clang-tidy, des options
# de compilation, du seuil de couverture ou du registre d'exigences.

set -euo pipefail

readonly REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
readonly LIB_SOURCE="${REPO_ROOT}/src/lib/subedit/core/version.cpp"
readonly TEST_SOURCE="${REPO_ROOT}/src/test/unit/core/version_test.cpp"
readonly REGISTRY="${REPO_ROOT}/docs/exigences.md"
readonly E2E_SOURCE="${REPO_ROOT}/src/test/e2e/cli/version_test.cpp"
readonly MAKEFILE_SOURCE="${REPO_ROOT}/Makefile"

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
trap cleanup EXIT

# Injecte un défaut, exécute la cible make attendue en échec, rétablit.
expect_gate_closes() {
    local label="$1"
    local target="$2"
    local file="$3"
    local snippet="$4"

    printf '%s▸ %s%s\n' "${BOLD}" "${label}" "${RESET}"
    printf '%s\n' "${snippet}" >> "${file}"

    if make -C "${REPO_ROOT}" --no-print-directory "${target}" >/dev/null 2>&1; then
        printf '  %s✗ la porte « %s » a laissé passer le défaut%s\n' "${RED}" "${target}" "${RESET}"
        failures=$((failures + 1))
    else
        printf '  %s✓ « make %s » a échoué, comme attendu%s\n' "${GREEN}" "${target}" "${RESET}"
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

# Code non exercé par les tests : fait chuter la couverture sous le seuil.
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

printf '\n'
if (( failures > 0 )); then
    printf '%s%d porte(s) laissent passer un défaut%s\n' "${RED}" "${failures}" "${RESET}" >&2
    exit 1
fi
printf '%sles huit portes se referment%s\n' "${GREEN}" "${RESET}"
