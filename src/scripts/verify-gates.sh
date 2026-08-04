#!/usr/bin/env bash
# Prouve que les portes de qualité se referment.
#
# Une porte qu'on n'a jamais vue échouer n'est pas une porte : c'est une
# croyance. Ce script injecte délibérément un défaut de chaque type, vérifie que
# l'étape correspondante de `make check` échoue, puis rétablit les sources.
#
# À rejouer après toute modification de .clang-format, .clang-tidy, des options
# de compilation ou du seuil de couverture.

set -euo pipefail

readonly REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
readonly LIB_SOURCE="${REPO_ROOT}/src/lib/subedit/core/version.cpp"
readonly TEST_SOURCE="${REPO_ROOT}/src/test/unit/core/version_test.cpp"

readonly RED=$'\033[31m'
readonly GREEN=$'\033[32m'
readonly BOLD=$'\033[1m'
readonly RESET=$'\033[0m'

backup_dir="$(mktemp -d)"
failures=0

restore() {
    cp "${backup_dir}/version.cpp" "${LIB_SOURCE}"
    cp "${backup_dir}/version_test.cpp" "${TEST_SOURCE}"
}

cleanup() {
    restore
    rm -rf "${backup_dir}"
}

cp "${LIB_SOURCE}" "${backup_dir}/version.cpp"
cp "${TEST_SOURCE}" "${backup_dir}/version_test.cpp"
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

printf '\n'
if (( failures > 0 )); then
    printf '%s%d porte(s) laissent passer un défaut%s\n' "${RED}" "${failures}" "${RESET}" >&2
    exit 1
fi
printf '%sles cinq portes se referment%s\n' "${GREEN}" "${RESET}"
