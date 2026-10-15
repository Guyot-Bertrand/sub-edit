#!/usr/bin/env bash
# Proves that the quality gates close.
#
# A gate that has never been seen failing is not a gate: it is a belief. This
# script deliberately injects one defect of each kind, checks that the matching
# target fails, then restores the sources.
#
# The first six injections aim at steps of `make check`, which CI runs. The
# next five aim at `make check-local` — four times `requirements`, once
# `parallelism` — which therefore gates only the development machine: all the
# more reason for this script to prove that each one closes, since nothing else
# will exercise them.
#
# **The four of `requirements` are its four defects**: an implemented
# requirement that nothing cites, a tag that names no requirement, a registry
# requirement missing from its spec's table, and a requirement that a spec
# promises and the registry ignores. The last two were born at the review of
# phase 7, after two phases in which the gap went through.
#
# The next three aim at neither: they are the pull request checks, which only
# make sense on GitHub — a pull request body does not exist locally. Nothing
# would ever exercise them before a real pull request depended on them. They are
# invoked one at a time, check by check, so that an injection is the only thing
# that can make its run fail.
#
# The next four aim at `make manual-check`, one per way the manual's example
# generator can stop.
#
# Two more, born of #232, do not aim at a gate either: `prune-releases.sh`
# chooses the releases it deletes, and `check-release-tag.sh` decides whether a
# tag deserves to be published. Three more, born of #399, test what the first
# does with its choice, against a makeshift `gh`: a refusal that exits in error
# without skipping the deletions that follow, a draft that is not a version, and
# an on-demand rebuild that does not prune.
#
# The last one aims at no gate: clean-stale-coverage.sh repairs instead of
# refusing, and what can go wrong with it goes both ways — keeping an
# inconsistent tree, or discarding a sound one.
#
# The next six aim at `make parallelism`: two families of files that escaped
# the scan, three forms it did not recognise, and **a proof of a different
# kind** — that legitimate code is not reported. A non-reporting criterion
# cannot be shown by any injection that fails, hence `expect_gate_stays_open`.
#
# The next two aim at `make arch`: a test case name that starts with a dash, and
# a test that would read the reference repository. It is here because that
# defect shows only through CTest, never when the test binary is run by hand —
# so only in the gate.
#
# The next two aim at `make score`, the encoding detection record: a record
# that is raised, which the gate must refuse, and — of the
# `expect_gate_stays_open` kind — a record that is lowered, which it must let
# through. A check that refused a detection better than its record would be
# worse than none.
#
# The next two aim at `make fixtures`: a video fixture rebuilt with the wrong
# frame rate, and a grid fixture position moved by one millisecond. It is the
# only injection of the script that is not text — a container is unreadable in
# a diff, and that is precisely why nothing else would catch the defect. It is
# also the only one to go through ffmpeg rather than through a snippet appended
# to the file: appending text to an MP4 does not change it, ffprobe having
# already read it.
#
# The next two aim at `make untracked`, the gate that refuses what a run leaves
# behind: a file that appeared during the tests, and — of the other kind, that
# of `expect_gate_stays_open` — an untracked file that was already there and
# must above all not be reported.
#
# The next one aims at `make install-check` — #239: `install()` rules that
# forget a data file. It is the only check of the repository that looks at a
# clean installation, and the defect it catches is invisible everywhere else —
# in the build tree, the file is there because the repository contains it, not
# because a rule copied it.
#
# The next two aim at the manual's screenshots — #199: a reference that no
# longer matches the window, which `make screenshots-check` must refuse, and an
# image the manual shows without anything generating it, which
# `check-screenshots.py` must catch. The second defect is the costlier of the
# two, and the only one the comparator cannot see: a stale image displays as
# cleanly as a correct one. Four more, born of #400, test the light and dark
# pair of each screen: it is missing from the program, it is missing from the
# manual, it is exempted, it is complete.
#
# The next two aim at `make config-home`, the counterpart of the previous one on
# the other side of the repository's border: a configuration written during the
# tests, and — of the `expect_gate_stays_open` kind — a configuration already
# there that nothing touched. They run under a makeshift `XDG_CONFIG_HOME`, so
# that the proof of a gate that refuses writes to the home directory never
# writes there itself.
#
# The last two aim at no gate: `src/scripts/record-bench.sh`, which refuses
# nothing but chooses what it writes into a table that is never pruned — an
# extreme set by noise is final — and `src/scripts/prune-runs.sh`, which refuses
# nothing but chooses what it deletes. They are here for lack of a better place
# — this script is the project's only harness for what is written in shell, and
# an irreversible selection that no run checks would be a belief as well.
#
# To be replayed after any change to .clang-format, .clang-tidy, the compiler
# options, the coverage ratchet, the requirements registry or cliff.toml.
#
# Requires git-cliff, which the changelog check calls through `make changelog`,
# and ffmpeg for the video fixture: ./src/scripts/setup-toolchain.sh installs
# both.

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
# A header with few dependents: the proof of header analysis must not reanalyse
# half the tree to establish that a diagnostic in a header comes up.
readonly LEAF_HEADER="${REPO_ROOT}/src/lib/subedit/core/format/lrc_writer.hpp"
readonly MODEL_SOURCE="${REPO_ROOT}/src/lib/subedit/core/model/subtitle_index.hpp"
readonly PR_CHECK="${REPO_ROOT}/src/scripts/check-pull-request.sh"
readonly PRUNE_SCRIPT="${REPO_ROOT}/src/scripts/prune-runs.sh"
readonly RELEASE_PRUNE_SCRIPT="${REPO_ROOT}/src/scripts/prune-releases.sh"
readonly RELEASE_TAG_CHECK="${REPO_ROOT}/src/scripts/check-release-tag.sh"
readonly VIDEO_FIXTURE="${REPO_ROOT}/src/test/data/videos/cadence-25.mp4"
readonly GRID_FIXTURE="${REPO_ROOT}/src/test/data/grilles/grille-25.srt"
readonly PATTERN_EXPECTED="${REPO_ROOT}/src/test/data/motifs/attendus/common-error.cas"
readonly DETECTION_JOURNAL="${REPO_ROOT}/docs/mesures/detection-d-encodage.md"
readonly CONVERSION_JOURNAL="${REPO_ROOT}/docs/mesures/conversion.md"
readonly FORMAT_JOURNAL="${REPO_ROOT}/docs/mesures/detection-de-format.md"
readonly GUI_MANUAL_SOURCE="${REPO_ROOT}/docs/manual/subedit-gui/table.md"
readonly CAPTURE_REFERENCE="${REPO_ROOT}/docs/manual/subedit-gui/captures/table.png"
readonly INSTALLATION_SOURCE="${REPO_ROOT}/cmake/Installation.cmake"
readonly TRANSLATIONS_SOURCE="${REPO_ROOT}/cmake/Translations.cmake"
readonly DESKTOP_SOURCE="${REPO_ROOT}/packaging/io.github.guyot_bertrand.subedit.desktop"
readonly ICON_SOURCE="${REPO_ROOT}/packaging/io.github.guyot_bertrand.subedit.svg"
readonly PACKAGING_SOURCE="${REPO_ROOT}/cmake/Packaging.cmake"
# The spec whose requirements table is confronted with the registry. Phase 3 is
# chosen because it carries the most rows: an injection is lost in the crowd
# there, which is exactly the case to prove.
readonly SPEC_SOURCE="${REPO_ROOT}/docs/specs/03-cli.md"
# The clang-tidy configuration, in which proof 3 hardens a rule.
readonly TIDY_CONFIG="${REPO_ROOT}/.clang-tidy"
readonly OTHER_CAPTURE="${REPO_ROOT}/docs/manual/subedit-gui/captures/decalage.png"
# The witness file of the proof of files left behind. It backs nothing up: it is
# created by the proof and must disappear with it, even if the script dies —
# otherwise that very proof would leave a file behind.
readonly STRAY_FILE="${REPO_ROOT}/left-by-a-test.srt"
# The unreadable file that the format detection proof slips into the labelled
# corpus. Same discipline as the previous one: created by the proof, removed by
# `restore`, even if the script dies.
readonly FORMAT_STRAY="${REPO_ROOT}/src/test/data/formats/unreadable-by-a-proof.srt"

readonly RED=$'\033[31m'
readonly GREEN=$'\033[32m'
readonly BOLD=$'\033[1m'
readonly RESET=$'\033[0m'

backup_dir="$(mktemp -d)"
failures=0

# **The time of each proof** — #686. `verify-gates` went past the hour without anyone knowing
# where: every proof header goes through `proof_header`, which closes the measurement of the
# previous one, and the final report ranks the longest. Nothing is written to the repository: the
# duration is an observation of the run, to be reported in the pull request that changes it, not a
# file to keep up.
proof_names=()
proof_seconds=()
current_proof=""
proof_started=${SECONDS}

end_proof() {
    if [[ -n "${current_proof}" ]]; then
        proof_names+=("${current_proof}")
        proof_seconds+=("$((SECONDS - proof_started))")
    fi
    current_proof=""
}

proof_header() {
    end_proof
    printf "$@"
    local line
    printf -v line "$@"
    line="${line//${BOLD}/}"
    line="${line//${RESET}/}"
    current_proof="${line//$'\n'/}"
    current_proof="${current_proof#▸ }"
    proof_started=${SECONDS}
}

report_durations() {
    end_proof
    local total=0 index
    for index in "${!proof_seconds[@]}"; do
        total=$((total + proof_seconds[index]))
    done
    printf '\n%sproof durations: %d s in all, longest first%s\n' "${BOLD}" "${total}" "${RESET}"
    for index in "${!proof_seconds[@]}"; do
        printf '%6d s  %s\n' "${proof_seconds[index]}" "${proof_names[index]}"
    done | sort -rn | head -"${VERIFY_GATES_TOP:-15}"
}

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
    cp "${backup_dir}/lrc_writer.hpp" "${LEAF_HEADER}"
    cp "${backup_dir}/cadence-25.mp4" "${VIDEO_FIXTURE}"
    cp "${backup_dir}/grille-25.srt" "${GRID_FIXTURE}"
    cp "${backup_dir}/common-error.cas" "${PATTERN_EXPECTED}"
    cp "${backup_dir}/detection-d-encodage.md" "${DETECTION_JOURNAL}"
    cp "${backup_dir}/conversion.md" "${CONVERSION_JOURNAL}"
    cp "${backup_dir}/detection-de-format.md" "${FORMAT_JOURNAL}"
    cp "${backup_dir}/table.md" "${GUI_MANUAL_SOURCE}"
    cp "${backup_dir}/table.png" "${CAPTURE_REFERENCE}"
    cp "${backup_dir}/Installation.cmake" "${INSTALLATION_SOURCE}"
    cp "${backup_dir}/subedit.desktop" "${DESKTOP_SOURCE}"
    cp "${backup_dir}/subedit.svg" "${ICON_SOURCE}"
    cp "${backup_dir}/Packaging.cmake" "${PACKAGING_SOURCE}"
    cp "${backup_dir}/Translations.cmake" "${TRANSLATIONS_SOURCE}"
    cp "${backup_dir}/03-cli.md" "${SPEC_SOURCE}"
    cp "${backup_dir}/clang-tidy" "${TIDY_CONFIG}"
    rm -f "${STRAY_FILE}"
    rm -f "${FORMAT_STRAY}"
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
cp "${LEAF_HEADER}" "${backup_dir}/lrc_writer.hpp"
cp "${VIDEO_FIXTURE}" "${backup_dir}/cadence-25.mp4"
cp "${GRID_FIXTURE}" "${backup_dir}/grille-25.srt"
cp "${PATTERN_EXPECTED}" "${backup_dir}/common-error.cas"
cp "${DETECTION_JOURNAL}" "${backup_dir}/detection-d-encodage.md"
cp "${CONVERSION_JOURNAL}" "${backup_dir}/conversion.md"
cp "${FORMAT_JOURNAL}" "${backup_dir}/detection-de-format.md"
cp "${GUI_MANUAL_SOURCE}" "${backup_dir}/table.md"
cp "${CAPTURE_REFERENCE}" "${backup_dir}/table.png"
cp "${INSTALLATION_SOURCE}" "${backup_dir}/Installation.cmake"
cp "${DESKTOP_SOURCE}" "${backup_dir}/subedit.desktop"
cp "${ICON_SOURCE}" "${backup_dir}/subedit.svg"
cp "${PACKAGING_SOURCE}" "${backup_dir}/Packaging.cmake"
cp "${TRANSLATIONS_SOURCE}" "${backup_dir}/Translations.cmake"
cp "${SPEC_SOURCE}" "${backup_dir}/03-cli.md"
cp "${TIDY_CONFIG}" "${backup_dir}/clang-tidy"
trap cleanup EXIT

# Injects a defect, runs the make target expected to fail, restores.
#
# **The injected snippet is written between single quotes**, so it cannot
# contain one: an apostrophe in a C++ comment closes the string and the script
# dies on a syntax error, far from the faulty line. Write the snippet's comments
# without apostrophes.
expect_gate_closes() {
    local label="$1"
    local target="$2"
    local file="$3"
    local snippet="$4"
    # Optional: a text the gate's output must contain. A gate that fails for a reason other than
    # the injected defect does not prove it; this parameter says so for the proofs that run a
    # single test (#686), where the likeliest failure of a mistake is to run none.
    local expected="${5:-}"
    local output status=0

    proof_header '%s▸ %s%s\n' "${BOLD}" "${label}" "${RESET}"
    # An empty snippet means the injection has already been done by the caller,
    # because it does not consist in appending text at the end of a file —
    # modifying an existing block, for instance.
    [[ -z "${snippet}" ]] || printf '%s\n' "${snippet}" >> "${file}"

    output="$(make -C "${REPO_ROOT}" --no-print-directory "${target}" 2>&1)" || status=$?

    if ((status == 0)); then
        printf '  %s✗ the « %s » gate let the defect through%s\n' "${RED}" "${target}" "${RESET}"
        failures=$((failures + 1))
    elif [[ -n "${expected}" && "${output}" != *"${expected}"* ]]; then
        printf '  %s✗ « make %s » failed, but without citing « %s »%s\n' "${RED}" "${target}" "${expected}" "${RESET}"
        failures=$((failures + 1))
    else
        printf '  %s✓ « make %s » failed, as expected%s\n' "${GREEN}" "${target}" "${RESET}"
    fi

    restore
}

# The same for a pull request check. Two differences with the quality gate: the
# check is not a make target, and its defect can live in the environment — a
# pull request body is not a file of the repository.
#
# The check is named explicitly rather than running all three: a full run would
# fail on the other two anyway, and the injection would no longer prove
# anything. The arguments that follow are assignments passed to `env`.
expect_pr_check_closes() {
    local label="$1"
    local control="$2"
    shift 2

    proof_header '%s▸ %s%s\n' "${BOLD}" "${label}" "${RESET}"

    if env "$@" "${PR_CHECK}" "${control}" >/dev/null 2>&1; then
        printf '  %s✗ the « %s » check let the defect through%s\n' "${RED}" "${control}" "${RESET}"
        failures=$((failures + 1))
    else
        printf '  %s✓ the « %s » check refused, as expected%s\n' "${GREEN}" "${control}" "${RESET}"
    fi

    restore
}

# The reverse of expect_gate_closes: injects legitimate code that the check
# could mistake for a defect, and checks that it does not report it.
#
# A criterion of the form "this is not reported" cannot be shown by any
# injection that fails. Without a proof of this kind, it would remain a belief
# — and it is the failure mode that matters most: a check that cries wolf ends
# up disabled, and then protects nothing.
expect_gate_stays_open() {
    local label="$1"
    local target="$2"
    local file="$3"
    local snippet="$4"

    proof_header '%s▸ %s%s\n' "${BOLD}" "${label}" "${RESET}"
    printf '%s\n' "${snippet}" >> "${file}"

    if make -C "${REPO_ROOT}" --no-print-directory "${target}" >/dev/null 2>&1; then
        printf '  %s✓ « make %s » let it through, as expected%s\n' "${GREEN}" "${target}" "${RESET}"
    else
        printf '  %s✗ « make %s » reported legitimate code%s\n' "${RED}" "${target}" "${RESET}"
        failures=$((failures + 1))
    fi

    restore
}

printf '%sverifying the quality gates%s\n\n' "${BOLD}" "${RESET}"

expect_gate_closes \
    "format defect" \
    "format-check" \
    "${LIB_SOURCE}" \
    'int   badly_Formatted ( int a ,int b ) {return a+b ;}'

# -Wunused-parameter comes from -Wextra, and -Werror makes it an error.
expect_gate_closes \
    "compiler warning" \
    "build" \
    "${LIB_SOURCE}" \
    'namespace subedit::core {
int injectedWarning(int used, int unused) {
    return used;
}
} // namespace subedit::core'

# cppcoreguidelines-owning-memory: a naked owning pointer.
expect_gate_closes \
    "pattern rejected by clang-tidy" \
    "tidy" \
    "${LIB_SOURCE}" \
    'namespace subedit::core {
int* injectedOwningPointer() {
    return new int(42);
}
} // namespace subedit::core'

# Use after free: invisible to static analysis, detected by ASan.
# **A single test is run** (`GATE_PROOF_TESTS`, see gate/asan.sh) and the output must name the
# error: the failure therefore comes from the injected defect, not from elsewhere — #686.
GATE_PROOF_TESTS="injected use after free" expect_gate_closes \
    "memory error at run time" \
    "asan" \
    "${TEST_SOURCE}" \
    '#include <memory>

TEST_CASE("injected use after free", "[injected]") {
    auto owned = std::make_unique<std::string>("subedit");
    const std::string* observer = owned.get();
    owned.reset();
    CHECK(observer->size() == 7);
}' \
    "AddressSanitizer"

# Signed integer overflow: invisible to static analysis and to ASan, detected by
# UBSan. Its own proof, separate from ASan's, because the two sanitizers stop for
# different reasons — ASan kills the process by itself, UBSan does so only since
# -fno-sanitize-recover=undefined. Without that option it reported the defect and
# let the test pass, which a real overflow in the time grammar demonstrated at
# the review of phase 3.
GATE_PROOF_TESTS="injected signed overflow" expect_gate_closes \
    "undefined behaviour at run time" \
    "asan" \
    "${TEST_SOURCE}" \
    '#include <cstdint>
#include <limits>

TEST_CASE("injected signed overflow", "[injected]") {
    // Read through a reference: at -O0 the compiler cannot fold the
    // computation, so the overflow does happen during the test.
    std::int64_t largest = std::numeric_limits<std::int64_t>::max();
    const std::int64_t& read = largest;
    CHECK(read + 1 > 0);
}' \
    "signed integer overflow"

# Code the tests do not exercise: raises the number of uncovered lines beyond
# the ratchet.
expect_gate_closes \
    "coverage drop" \
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

# expect_gate_closes restores only the sources: the JSON summary and the HTML
# report that "make coverage" has just produced from the injected defect survive
# it in build/coverage-report/. Left in place, that residue still claims
# uncovered lines attributed to a function that no longer exists in any source —
# a "make ratchet" run right after would record it as is and loosen the ratchet
# against a defect that never existed.
rm -rf "${REPO_ROOT}/build/coverage-report"

# A requirement declared implemented that nothing cites. The injection appends
# a line at the end of the file, which requires the registry table to be the last
# thing in docs/exigences.md — that file says so.
#
# **The same line is added to the phase's spec**, so the caller does the
# injection itself. Without it, the framing check would fail too — a requirement
# in the registry and missing from its spec — and the proof would no longer say
# anything about the check it aims at: it would pass even if the confrontation
# with the tests were broken. A proof with two causes proves neither.
printf '%s\n' '| `CLI-PHANTOM-01` | injected requirement that nothing demonstrates | 3 | implemented |' \
    >> "${REGISTRY}"
printf '%s\n' '| `CLI-PHANTOM-01` | injected requirement that nothing demonstrates |' >> "${SPEC_SOURCE}"
expect_gate_closes \
    "implemented requirement without a test" \
    "requirements" \
    "${REGISTRY}" \
    ''

# A registry requirement that its spec's table does not carry. It is `planned`
# and not `implemented`, for the reason symmetric to the one above: a planned
# requirement needs no test, so the confrontation with the tests stays silent
# and only the framing speaks.
expect_gate_closes \
    "registry requirement missing from its spec's table" \
    "requirements" \
    "${REGISTRY}" \
    '| `CLI-PHANTOM-02` | injected requirement that its spec ignores | 3 | planned |'

# The other direction: a requirement that a spec promises and the registry does
# not carry. It is the exact defect that phase 7 found in itself — `GUI-THEME-02`
# merged into `GUI-THEME-01` without the spec saying so — and that nothing saw.
expect_gate_closes \
    "requirement promised by a spec and missing from the registry" \
    "requirements" \
    "${SPEC_SOURCE}" \
    '| `CLI-PHANTOM-03` | requirement promised by the spec and unknown to the registry |'

# A tag shaped like an identifier that names no requirement of the registry.
expect_gate_closes \
    "tag without a matching requirement" \
    "requirements" \
    "${E2E_SOURCE}" \
    'TEST_CASE("injected unknown requirement", "[e2e][CLI-NONEXISTENT-99]") {
    CHECK(true);
}'

# A Makefile recipe that hard-codes a parallelism instead of going through
# $(JOBS) — exactly what check-parallelism.sh exists to spot. Nobody refers to
# the injected target: neither `make parallelism` nor any other target runs it,
# so the expected failure can only come from the script's scan of the Makefile,
# not from a build attempt.
expect_gate_closes \
    "parallelism hard-coded in the Makefile" \
    "parallelism" \
    "${MAKEFILE_SOURCE}" \
    'injected-parallelism-target:
	@cmake --build --preset dev -j 8'

# A pull request body that describes the work without carrying the line GitHub
# reads. It is the measured defect of phase 2: five issues left open after their
# pull request was merged, because a French "closes" is not a keyword.
# A test case name that starts with a dash. CTest passes it as an argument to
# the binary, Catch2 reads one of its options in it, and the case fails — but
# only through CTest, never when the binary is run by hand. The injection is in
# a test file, and the expected failure comes from the scan of
# check-architecture.sh, not from a compilation.
# A model header that includes an operation. The boundary set by ADR 0018 had
# already faded once, when the format-specific types ended up under model/
# without anything reporting it: hence a gate rather than vigilance. The
# injection is in a header, and the expected failure comes from the scan of
# check-architecture.sh — the include at the end of the file compiles nothing.
expect_gate_closes \
    "model that depends on an operation" \
    "arch" \
    "${MODEL_SOURCE}" \
    '#include <subedit/core/format/read_result.hpp>'

expect_gate_closes \
    "test case name that looks like an option" \
    "arch" \
    "${TEST_SOURCE}" \
    'TEST_CASE("-v looks like an option to Catch2", "[injected]") {
    CHECK(true);
}'

# A French comment added to a C++ file. The language ratchet is the only check
# of this repository whose defect cannot be a typo: it counts, and one more line
# makes it overflow. Without a proof, a ratchet set at the exact count of the day
# cannot be told apart from a ratchet set too high, which would let through the
# drift it is meant to stop.
#
# The snippet is written without apostrophes or accents, like all of them here:
# it goes through a string between single quotes. Its French is the defect.
expect_gate_closes \
    "French comment added to C++" \
    "arch" \
    "${LIB_SOURCE}" \
    '// Une ligne de commentaire dans la langue qui nest pas celle des fichiers
// que ce controle surveille, pour que la porte le dise.'

expect_pr_check_closes \
    "pull request body without « Closes #N »" \
    "closes" \
    "PR_BODY=This body describes the work, but no line in it closes the issue."

# The branch version copied from that of HEAD, which serves as the base: both
# then announce the same number. The copy is needed — merely naming HEAD as the
# base would assume that the working copy has not already gone past it, which an
# uncommitted bump would contradict, and the injection would no longer prove
# anything.
head_version="$(git -C "${REPO_ROOT}" show HEAD:CMakeLists.txt \
    | sed -n 's/^[[:space:]]*VERSION[[:space:]]\+\([0-9][0-9.]*\)[[:space:]]*$/\1/p' | head -1)"
sed -i "s/^\([[:space:]]*VERSION[[:space:]]\+\)[0-9][0-9.]*[[:space:]]*$/\1${head_version}/" \
    "${CMAKE_SOURCE}"
expect_pr_check_closes \
    "version identical to the base's" \
    "version" \
    "BASE_SHA=$(git -C "${REPO_ROOT}" rev-parse HEAD)"

# A changelog edited by hand, hence different from what git-cliff produces from
# the history. It is exactly the shape a changelog that was not regenerated
# takes: a content that the command would not give back.
printf "injected line that no regeneration would give back\n" >> "${CHANGELOG_SOURCE}"
expect_pr_check_closes \
    "CHANGELOG.md not regenerated" \
    "changelog"

# The next four aim at "make manual-check", one per way the example generator
# can stop. The worst result this generator could produce is a manual silently
# emptied of its examples; that is why each of its refusals is proven, and not
# only that of the stale manual.

# A block whose output no longer matches what the command produces. The change
# is in an existing block, not in a line appended at the end of the file: hence
# the empty snippet passed to the injector.
sed -i 's/^subedit [0-9][0-9.]*$/subedit 0.9.9/' "${MANUAL_SOURCE}"
expect_gate_closes \
    "stale manual example" \
    "manual-check" \
    "${MANUAL_SOURCE}" \
    ''

# The marker keeps the spelling `<!-- exemple: … -->` that generate-manual.sh reads.
expect_gate_closes \
    "example whose command fails" \
    "manual-check" \
    "${MANUAL_SOURCE}" \
    '
<!-- exemple: echo hello; exit 3 -->
```console
$ echo hello; exit 3
hello
```'

expect_gate_closes \
    "marker not followed by a console block" \
    "manual-check" \
    "${MANUAL_SOURCE}" \
    '
<!-- exemple: subedit-cli -->
This paragraph does not start a console block.'

# A silent command: it is the case that would produce an empty block if the
# generator let it through.
expect_gate_closes \
    "example whose command produces nothing" \
    "manual-check" \
    "${MANUAL_SOURCE}" \
    '
<!-- exemple: true -->
```console
$ true
```'

# The next five aim at the widening of `make parallelism`: two families of files
# that escaped the scan, three forms of parallelism it did not recognise.

# A git hook: in a subdirectory and without a .sh extension, so doubly missed by
# the old scan — which did not descend and filtered on *.sh.
expect_gate_closes \
    "parallelism hard-coded in a git hook" \
    "parallelism" \
    "${HOOK_SOURCE}" \
    'cmake --build . -j 8'

# A nested CMakeLists: the old scan read only cmake/*.cmake and the root
# CMakeLists, leaving out the eight of src/ — the very ones that declare the test
# and benchmark targets.
expect_gate_closes \
    "-flto= hard-coded in a CMakeLists of src/" \
    "parallelism" \
    "${NESTED_CMAKE_SOURCE}" \
    'target_compile_options(subedit_core PRIVATE -flto=8)'

# `-j` without a value takes as many processes as the machine has cores: it is
# worse than a hard-coded number, and it went through.
expect_gate_closes \
    "make -j without a value" \
    "parallelism" \
    "${PLAIN_SCRIPT_SOURCE}" \
    'make -j'

expect_gate_closes \
    "cmake --build --parallel with a number" \
    "parallelism" \
    "${PLAIN_SCRIPT_SOURCE}" \
    'cmake --build . --parallel 8'

expect_gate_closes \
    "parallelism passed through MAKEFLAGS" \
    "parallelism" \
    "${PLAIN_SCRIPT_SOURCE}" \
    'MAKEFLAGS=-j8 make all'

# A test that reads the reference repository. It would pass here, where the
# clone exists, and fail everywhere else — or declare itself skipped, which is
# worse: green without proving anything.
expect_gate_closes \
    "reading the reference repository" \
    "arch" \
    "${TEST_SOURCE}" \
    '// reference/gaupol/aeidon/data/patterns'

# A video fixture rebuilt with the wrong frame rate. It is the only defect that
# the table of video-fixtures.sh exists to catch, and the only one nobody would
# otherwise see: a container is unreadable in a diff.
#
# The injection is done here rather than passed as a snippet, because appending
# text at the end of an MP4 does not change it — ffprobe reads the file before.
ffmpeg -v error -y \
    -f lavfi -i 'color=c=black:s=16x16:r=30:d=2' \
    -c:v mpeg4 -qscale:v 31 -pix_fmt yuv420p \
    -fflags +bitexact -flags:v +bitexact -map_metadata -1 \
    "${VIDEO_FIXTURE}"

expect_gate_closes \
    "video fixture at the wrong frame rate" \
    "fixtures" \
    "${VIDEO_FIXTURE}" \
    ''

# A grid fixture position moved by one millisecond. It is the smallest defect
# that subtitle-fixtures.py exists to catch, and the most instructive: one
# millisecond does not show in a file of a hundred and seventy cues, and it is
# enough to push a position off the grid.
#
# The injection is done here rather than passed as a snippet, because appending
# text at the end of the file would be a gross defect. What is proven is that
# the gate reads the positions, not the size of the file.
sed -i '0,/00:00:01,000/s//00:00:01,001/' "${GRID_FIXTURE}"

expect_gate_closes \
    "grid position moved by one millisecond" \
    "fixtures" \
    "${GRID_FIXTURE}" \
    ''

# An expected result of the correction patterns skewed by one space — issue #494.
# It is the defect that pattern-oracle.py exists to catch: an expected result
# retouched by hand rather than rewritten by the oracle, and which would no
# longer say what Gaupol does. One space, because it is the defect an eye lets
# through.
sed -i '0,/| "Hello, world"$/s//| "Hello,  world"/' "${PATTERN_EXPECTED}"

expect_gate_closes \
    "pattern expected result skewed by one space" \
    "fixtures" \
    "${PATTERN_EXPECTED}" \
    ''


# The file's only non-reporting proof, and the most important of the six added
# here: the check read its lines without removing trailing comments, so an
# example quoted in a comment passed for a workaround. No failing injection can
# show that a false positive has gone.
expect_gate_stays_open \
    "end-of-line comment quoting -j 4" \
    "parallelism" \
    "${PLAIN_SCRIPT_SOURCE}" \
    'cmake --build . # example: -j 4 to go faster'

# The two proofs of the thread bound — issue #630. `JOBS` bounds the processes, not the threads
# that libmpv and ffmpeg create: a bare `ctest` is the gate going past its two cores without
# anything saying so. The second is the non-reporting proof: the same command, passed through
# `limit-cores.sh`, must not be refused.
expect_gate_closes \
    "ctest run without limit-cores.sh" \
    "parallelism" \
    "${PLAIN_SCRIPT_SOURCE}" \
    'ctest --preset dev'

expect_gate_stays_open \
    "ctest run through limit-cores.sh" \
    "parallelism" \
    "${PLAIN_SCRIPT_SOURCE}" \
    './src/scripts/limit-cores.sh ctest --preset dev'

# **The bench, which opens players, is in the same case as `ctest`** — review of phase 14:
# `gate/bench.sh` ran `subedit_bench` bare, and its libmpv threads went past the gate's two
# cores. The second proof is the non-reporting one.
expect_gate_closes \
    "the bench run without limit-cores.sh" \
    "parallelism" \
    "${PLAIN_SCRIPT_SOURCE}" \
    '"${REPO_ROOT}/build/release/bin/subedit_bench" --reporter console'

expect_gate_stays_open \
    "the bench run through limit-cores.sh" \
    "parallelism" \
    "${PLAIN_SCRIPT_SOURCE}" \
    '"${REPO_ROOT}/src/scripts/limit-cores.sh" "${REPO_ROOT}/build/release/bin/subedit_bench" --reporter console'

# The three proofs of static analysis — issue #269.
#
# **They changed nature with the mechanism.** There was one, and it caught a
# real defect of tidy-scope.sh: it computed its scope with `git diff
# base...HEAD`, which sees only what is committed, so it returned zero files on
# a branch without a commit — a green gate that had analysed nothing. That
# script no longer exists: clang-tidy is hooked to the compile rule of each
# source, and git no longer enters the computation.
#
# Two things remain to prove here, plus a conditional one, one per input that
# the build system must see: the source, its headers, and the key that carries
# the rest.
#
# The first invocation builds the `build/tidy` tree in full, which costs about
# ten minutes; the next ones are incremental, and that is precisely the property
# under test.
expect_tidy_closes() {
    local label="$1"
    local file="$2"
    local snippet="$3"

    proof_header '%s▸ %s%s\n' "${BOLD}" "${label}" "${RESET}"
    printf '%s\n' "${snippet}" >> "${file}"

    if make -C "${REPO_ROOT}" --no-print-directory tidy >/dev/null 2>&1; then
        printf '  %s✗ the « tidy » gate let the defect through%s\n' "${RED}" "${RESET}"
        failures=$((failures + 1))
    else
        printf '  %s✓ « make tidy » failed, as expected%s\n' "${GREEN}" "${RESET}"
    fi

    restore
}

# 1 — a defect in an uncommitted source: it is already the proof "pattern rejected by
# clang-tidy", above — same injection in an uncommitted source, same `tidy` target, and nothing
# in the mechanism consults git. Replaying it here cost one more analysis (#686).

# 2 — **a defect in a header**, and it is the proof that did not exist.
#
# The old closure over headers was done by a `grep` of the file name in the
# sources: it caught a header quoted in a comment and could miss an indirect
# include. Here it is the dependency file the compiler wrote that decides, so
# the units that include this one are reanalysed, and they alone.
expect_tidy_closes \
    "defect in a header, brought up by its dependents" \
    "${LEAF_HEADER}" \
    'namespace subedit::core { inline int probeForTheProof() { int value = 1; return value; } }'

# 3 — **the configuration changes, and the analysis sees it.**
#
# It is the hole the mechanism would have left without the key of
# `cmake/Tidy.cmake` — see that file: the content of `.clang-tidy` is not on the
# command line, so Ninja does not look at it. A naming rule that the whole
# project violates is added to it; without the key, the tree would stay green on
# a stale answer, which is the worst failure mode possible.
#
# The injection is made at the end of the file, which requires the CheckOptions
# list to be the last thing in `.clang-tidy` — that file says so.
# **Not on every run** (#686): it tests `cmake/Tidy.cmake`, which almost never moves, and it
# costs a full reanalysis of the tree — twice, since the key goes back to its original value on
# restore. It runs when `.clang-tidy` or `cmake/Tidy.cmake` differ from `main`, or on request:
# `VERIFY_GATES_FULL=1`.
tidy_config_changed() {
    [[ -n "${VERIFY_GATES_FULL:-}" ]] && return 0
    local base
    base="$(git -C "${REPO_ROOT}" merge-base HEAD main 2>/dev/null)" || return 0
    ! git -C "${REPO_ROOT}" diff --quiet "${base}" -- .clang-tidy cmake/Tidy.cmake
}

if tidy_config_changed; then
    expect_tidy_closes \
        "hardened configuration, analysis replayed" \
        "${TIDY_CONFIG}" \
        '  - key: readability-identifier-naming.ParameterCase
        value: UPPER_CASE'
else
    printf '%s▸ hardened configuration, analysis replayed%s — skipped, .clang-tidy and cmake/Tidy.cmake unchanged (VERIFY_GATES_FULL=1 runs it)\n' "${BOLD}" "${RESET}"
fi

# A generated file dropped under src/. It would pass the four gates that filter
# on src/ — format, static analysis, coverage, scope — because none of them can
# tell a written line from a produced one.
#
# The injection carries the marker that moc, uic and rcc write, assembled here
# for the same reason as in the check: written in full, it would make this
# script a generated file in the eyes of the check.
expect_generated_source_closes() {
    local marker="All changes made in this file"
    local victim="${REPO_ROOT}/src/lib/subedit/core/moc_probe.cpp"

    proof_header '%s▸ %s%s\n' "${BOLD}" "generated file dropped under src/" "${RESET}"
    printf '/**** %s will be lost! ****/\n' "${marker}" > "${victim}"

    if make -C "${REPO_ROOT}" --no-print-directory arch >/dev/null 2>&1; then
        printf '  %s✗ the « arch » gate let the defect through%s\n' "${RED}" "${RESET}"
        failures=$((failures + 1))
    else
        printf '  %s✓ « make arch » failed, as expected%s\n' "${GREEN}" "${RESET}"
    fi

    rm -f "${victim}"
    restore
}

expect_generated_source_closes

# A test case title in French — issue #273.
#
# **The proof is about a net, not a certainty.** Recognising a language is a
# heuristic: the check looks for a non-ASCII character or one of the function
# words that exist only in French. What is shown is therefore that it catches
# what it was written for — the hundred titles that a single phase had left in
# French — and not that no French ever escapes it.
#
# The injection carries both halves: an accented title, then a French title
# without a single accent. The second is the one that matters, because it is the
# one a naive check would let through, and because two of the hundred were
# exactly of that form. Both titles are French on purpose: they are the defect.
expect_french_test_title_closes() {
    local one two

    for one in "un décalage négatif remonte les positions" \
               "un fichier absent ne fait dire mot"; do
        proof_header '%s▸ %s%s\n' "${BOLD}" "a test title in French: « ${one} »" "${RESET}"

        printf '\nTEST_CASE("%s", "[version]") {\n    CHECK(true);\n}\n' "${one}" \
            >> "${TEST_SOURCE}"

        if make -C "${REPO_ROOT}" --no-print-directory arch >/dev/null 2>&1; then
            printf '  %s✗ the « arch » gate let the title through%s\n' "${RED}" "${RESET}"
            failures=$((failures + 1))
        else
            printf '  %s✓ « make arch » failed, as expected%s\n' "${GREEN}" "${RESET}"
        fi

        restore
    done

    # And the counterpart: an English title that quotes a French datum must not
    # be reported. It is the check's only exemption, and an exemption nobody
    # verifies is an exemption that goes stale.
    proof_header '%s▸ %s%s\n' "${BOLD}" "an English title quoting a French datum" "${RESET}"
    if make -C "${REPO_ROOT}" --no-print-directory arch >/dev/null 2>&1; then
        printf '  %s✓ « make arch » let it through, as expected%s\n' "${GREEN}" "${RESET}"
    else
        printf '  %s✗ the « arch » gate reported the exempted title%s\n' "${RED}" "${RESET}"
        failures=$((failures + 1))
    fi
}

expect_french_test_title_closes

# French outside the locales, and an allowlist that grows — #660.
#
# The check reads what git tracks, so the proofs run on a scratch repository
# made of the tracked files of this one, committed once: that commit is the base
# the list may not outgrow. Nothing of the repository is touched, and the
# scratch directory goes with the proof. Three proofs: the copy as it is passes
# (the check does not cry wolf on the real tree), a French sentence in a new file
# is refused, and the same file refused again once its line is added to the list
# — the list descends, it never grows.
expect_repository_language_gate() {
    local script="${REPO_ROOT}/src/scripts/check-comment-language.py"
    local root output
    root="$(mktemp -d)"
    git -C "${REPO_ROOT}" ls-files -z | (cd "${REPO_ROOT}" && tar --null -T - -cf -) \
        | tar -xf - -C "${root}"
    git -C "${root}" init --quiet
    git -C "${root}" add --all
    git -C "${root}" -c user.name=proof -c user.email=proof@example.invalid \
        commit --quiet -m base

    proof_header '%s▸ the tracked tree, as it is%s\n' "${BOLD}" "${RESET}"
    if "${script}" --root "${root}" --base HEAD >/dev/null 2>&1; then
        printf '  %s✓ « check-comment-language.py » accepted the tree%s\n' "${GREEN}" "${RESET}"
    else
        printf '  %s✗ the check refuses the tree as it is%s\n' "${RED}" "${RESET}"
        failures=$((failures + 1))
    fi

    proof_header '%s▸ a French sentence in a new file%s\n' "${BOLD}" "${RESET}"
    cp "${REPO_ROOT}/src/test/data/language/french.md" "${root}/zz-proof.md"
    git -C "${root}" add zz-proof.md
    if output="$("${script}" --root "${root}" --base HEAD 2>&1)"; then
        printf '  %s✗ the check let the French file through%s\n' "${RED}" "${RESET}"
        failures=$((failures + 1))
    elif [[ "${output}" == *"UNLISTED"*"zz-proof.md"* ]]; then
        printf '  %s✓ « check-comment-language.py » refused the new file%s\n' "${GREEN}" "${RESET}"
    else
        printf '  %s✗ the check refused, but not for the new file%s\n' "${RED}" "${RESET}"
        failures=$((failures + 1))
    fi

    proof_header '%s▸ the same file, added to the allowlist%s\n' "${BOLD}" "${RESET}"
    printf 'zz-proof.md\n' >> "${root}/.french-allowlist"
    git -C "${root}" add .french-allowlist
    if output="$("${script}" --root "${root}" --base HEAD 2>&1)"; then
        printf '  %s✗ the check let the allowlist grow%s\n' "${RED}" "${RESET}"
        failures=$((failures + 1))
    elif [[ "${output}" == *"GROWN"*"zz-proof.md"* && "${output}" != *"UNLISTED"* ]]; then
        printf '  %s✓ « check-comment-language.py » refused the added entry%s\n' "${GREEN}" "${RESET}"
    else
        printf '  %s✗ the check refused, but not for the added entry%s\n' "${RED}" "${RESET}"
        failures=$((failures + 1))
    fi

    rm -rf "${root}"
}

expect_repository_language_gate

# A user-visible string outside the translation API, and a ratchet that rises — #661.
#
# The scan reads what git tracks, so the proofs run on a scratch repository made of the tracked
# files of this one, committed once: that commit is the base whose total the ratchet may not
# exceed. Nothing of the repository is touched, and the scratch directory goes with the proof.
# Four proofs: the copy as it is passes, an unwrapped label in a new source is refused, the same
# label wrapped in `tr` passes, and the unwrapped one recorded into the ratchet is refused as
# growth — the ratchet descends, it never rises.
expect_untranslated_gate() {
    local script="${REPO_ROOT}/src/scripts/check-translatable-strings.py"
    local root output
    local -r source="src/lib/subedit/gui/zz_proof.cpp"
    root="$(mktemp -d)"
    git -C "${REPO_ROOT}" ls-files -z | (cd "${REPO_ROOT}" && tar --null -T - -cf -) \
        | tar -xf - -C "${root}"
    git -C "${root}" init --quiet
    git -C "${root}" add --all
    git -C "${root}" -c user.name=proof -c user.email=proof@example.invalid \
        commit --quiet -m base

    proof_header '%s▸ the tracked tree, as it is%s\n' "${BOLD}" "${RESET}"
    if "${script}" --root "${root}" --base HEAD >/dev/null 2>&1; then
        printf '  %s✓ « check-translatable-strings.py » accepted the tree%s\n' "${GREEN}" "${RESET}"
    else
        printf '  %s✗ the check refuses the tree as it is%s\n' "${RED}" "${RESET}"
        failures=$((failures + 1))
    fi

    proof_header '%s▸ an unwrapped label in a new source%s\n' "${BOLD}" "${RESET}"
    printf 'void proof(QLabel& label) { label.setText(QStringLiteral("Nobody translates this")); }\n' \
        > "${root}/${source}"
    git -C "${root}" add "${source}"
    if output="$("${script}" --root "${root}" --base HEAD 2>&1)"; then
        printf '  %s✗ the check let the unwrapped label through%s\n' "${RED}" "${RESET}"
        failures=$((failures + 1))
    elif [[ "${output}" == *"NEW"*"zz_proof.cpp"* ]]; then
        printf '  %s✓ « check-translatable-strings.py » refused the label%s\n' "${GREEN}" "${RESET}"
    else
        printf '  %s✗ the check refused, but not for the label%s\n' "${RED}" "${RESET}"
        failures=$((failures + 1))
    fi

    proof_header '%s▸ the same label, wrapped in tr%s\n' "${BOLD}" "${RESET}"
    printf 'void proof(QLabel& label) { label.setText(tr("Nobody translates this")); }\n' \
        > "${root}/${source}"
    if "${script}" --root "${root}" --base HEAD >/dev/null 2>&1; then
        printf '  %s✓ « check-translatable-strings.py » accepted the wrapped label%s\n' \
            "${GREEN}" "${RESET}"
    else
        printf '  %s✗ the check refuses the wrapped label%s\n' "${RED}" "${RESET}"
        failures=$((failures + 1))
    fi

    proof_header '%s▸ the unwrapped label, recorded into the ratchet%s\n' "${BOLD}" "${RESET}"
    printf 'void proof(QLabel& label) { label.setText(QStringLiteral("Nobody translates this")); }\n' \
        > "${root}/${source}"
    "${script}" --root "${root}" --record >/dev/null
    if output="$("${script}" --root "${root}" --base HEAD 2>&1)"; then
        printf '  %s✗ the check let the ratchet rise%s\n' "${RED}" "${RESET}"
        failures=$((failures + 1))
    elif [[ "${output}" == *"GROWN"* && "${output}" != *"NEW"* ]]; then
        printf '  %s✓ « check-translatable-strings.py » refused the raised ratchet%s\n' \
            "${GREEN}" "${RESET}"
    else
        printf '  %s✗ the check refused, but not for the raised ratchet%s\n' "${RED}" "${RESET}"
        failures=$((failures + 1))
    fi

    rm -rf "${root}"
}

expect_untranslated_gate

# A translation that falls behind its source — #670.
#
# The check reads the catalogues of `src/po/` only, so the proofs run on a scratch copy of the
# files git tracks there; nothing of the repository is touched, and the copy goes with the proof.
# Three proofs: the copy as it is passes (the check does not cry wolf on the shipped French), one
# French entry emptied is refused and named, and the same entry emptied in a seed passes — it is
# reported, its translated count one lower, and nothing fails.
expect_stale_translation_gate() {
    local script="${REPO_ROOT}/src/scripts/check-translations.py"
    local root output seed_before seed_after
    root="$(mktemp -d)"
    git -C "${REPO_ROOT}" ls-files -z src/po | (cd "${REPO_ROOT}" && tar --null -T - -cf -) \
        | tar -xf - -C "${root}"
    local -r empty_edit='/^msgid "&Edit"$/{n;s/^msgstr ".*"$/msgstr ""/}'

    proof_header '%s▸ the catalogues, as they are%s\n' "${BOLD}" "${RESET}"
    if output="$("${script}" --root "${root}" 2>&1)"; then
        printf '  %s✓ « check-translations.py » accepted the catalogues%s\n' "${GREEN}" "${RESET}"
    else
        printf '  %s✗ the check refuses the catalogues as they are%s\n' "${RED}" "${RESET}"
        failures=$((failures + 1))
    fi
    seed_before="$(grep '^de ' <<<"${output}" || true)"

    proof_header '%s▸ an entry emptied in an installed language%s\n' "${BOLD}" "${RESET}"
    cp "${root}/src/po/fr.po" "${root}/fr.po.orig"
    sed -i "${empty_edit}" "${root}/src/po/fr.po"
    if output="$("${script}" --root "${root}" 2>&1)"; then
        printf '  %s✗ the check let the empty French entry through%s\n' "${RED}" "${RESET}"
        failures=$((failures + 1))
    elif [[ "${output}" == *"UNTRANSLATED  fr.po: '&Edit'"* ]]; then
        printf '  %s✓ « check-translations.py » refused and named the entry%s\n' "${GREEN}" "${RESET}"
    else
        printf '  %s✗ the check refused, but did not name the entry%s\n' "${RED}" "${RESET}"
        failures=$((failures + 1))
    fi
    mv "${root}/fr.po.orig" "${root}/src/po/fr.po"

    proof_header '%s▸ the same entry emptied in a seed%s\n' "${BOLD}" "${RESET}"
    sed -i "${empty_edit}" "${root}/src/po/de.po"
    if output="$("${script}" --root "${root}" 2>&1)"; then
        seed_after="$(grep '^de ' <<<"${output}" || true)"
        if [[ -n "${seed_before}" && "${seed_after}" != "${seed_before}" ]]; then
            printf '  %s✓ « check-translations.py » reported the seed and passed%s\n' "${GREEN}" "${RESET}"
        else
            printf '  %s✗ the check passed without counting the emptied entry%s\n' "${RED}" "${RESET}"
            failures=$((failures + 1))
        fi
    else
        printf '  %s✗ the check refuses a seed%s\n' "${RED}" "${RESET}"
        failures=$((failures + 1))
    fi

    rm -rf "${root}"
}

expect_stale_translation_gate

# The gate refuses what a run has left behind — #226.
#
# **Two proofs and not one**, because the check has two failure modes and they
# do not cost the same. Letting a forgotten file through is what happened once,
# at #207, and nobody knew before a git status. Reporting an untracked file that
# was already there — a new source before its git add — would be worse: the
# check would cry wolf every working day, and a check that cries wolf ends up
# disabled.
#
# It does not go through expect_gate_closes: the defect is not injected by
# appending text to a file of the repository, it consists in making one appear.
expect_untracked_gate() {
    local script="${REPO_ROOT}/src/scripts/check-untracked.sh"

    proof_header '%s▸ a test that leaves a file behind%s\n' "${BOLD}" "${RESET}"
    "${script}" --record >/dev/null
    : > "${STRAY_FILE}"
    if make -C "${REPO_ROOT}" --no-print-directory untracked >/dev/null 2>&1; then
        printf '  %s✗ the « untracked » gate let the file through%s\n' "${RED}" "${RESET}"
        failures=$((failures + 1))
    else
        printf '  %s✓ « make untracked » failed, as expected%s\n' "${GREEN}" "${RESET}"
    fi
    rm -f "${STRAY_FILE}"

    proof_header '%s▸ an untracked file already there before the record%s\n' "${BOLD}" "${RESET}"
    : > "${STRAY_FILE}"
    "${script}" --record >/dev/null
    if make -C "${REPO_ROOT}" --no-print-directory untracked >/dev/null 2>&1; then
        printf '  %s✓ « make untracked » let it through, as expected%s\n' "${GREEN}" "${RESET}"
    else
        printf '  %s✗ the « untracked » gate reported an earlier file%s\n' "${RED}" "${RESET}"
        failures=$((failures + 1))
    fi
    rm -f "${STRAY_FILE}"

    # The record is made again without the witness, so that the next proofs do
    # not start from a state that this one wrote.
    "${script}" --record >/dev/null
}

expect_untracked_gate

# An encoding fixture that is no longer what its table announces — issue #289.
#
# **It is the only family of fixtures whose alteration is invisible.** A grid
# can be reread, a video can be asked again with ffprobe; a CP1252 file does not
# even say what it is, and a changed byte looks in every way like a right byte.
# `--check` rebuilds it from the table and compares byte by byte — that is all
# that separates a fixture from a belief.
#
# The injection changes **a single byte**, precisely the one that tells CP1252
# from Latin-1: the 0x80-0x9f range. It is the case the check must see, and the
# one a text comparison would miss — both decode without error.
expect_encoding_fixture_gate() {
    local victim="${REPO_ROOT}/src/test/data/encodages/cp1252.srt"

    proof_header '%s▸ an encoding fixture altered by one byte%s\n' "${BOLD}" "${RESET}"

    cp "${victim}" "${backup_dir}/cp1252.srt"
    python3 - "${victim}" <<'PYTHON'
import sys
path = sys.argv[1]
data = bytearray(open(path, "rb").read())
for index, byte in enumerate(data):
    if 0x80 <= byte <= 0x9F:
        data[index] = 0x2E  # a full stop, perfectly lawful in Latin-1
        break
open(path, "wb").write(bytes(data))
PYTHON

    if make -C "${REPO_ROOT}" --no-print-directory fixtures >/dev/null 2>&1; then
        printf '  %s✗ the « fixtures » gate let the changed byte through%s\n' \
            "${RED}" "${RESET}"
        failures=$((failures + 1))
    else
        printf '  %s✓ « make fixtures » failed, as expected%s\n' "${GREEN}" "${RESET}"
    fi

    cp "${backup_dir}/cp1252.srt" "${victim}"
}

expect_encoding_fixture_gate

# The detection score, confronted with its record — issue #311.
#
# **Two proofs, because the check has three outcomes and only one is a
# refusal.** A score that drops is a regression and must fail; a score that
# rises is good news and must above all not fail, otherwise the first pull
# request that improves detection would run into its own improvement. The third
# case, equality, is the one every other run of this script already goes
# through.
#
# **The injection is in the record and not in the detection**, and that is what
# makes it exact: what the check compares is a figure of the journal with a
# measured figure, so moving the first tests the comparison itself, whatever the
# value of the second. A detection degraded by hand would prove the same thing
# less surely, and it would have to be recompiled.
expect_detection_score_gates() {
    local recorded
    recorded="$(sed -n 's/^ *labelled corpus *: *\([0-9]*\)\/\([0-9]*\) *$/\1 \2/p' \
        "${DETECTION_JOURNAL}")"
    local right="${recorded% *}"
    local total="${recorded#* }"

    proof_header '%s▸ a detection score below its record%s\n' "${BOLD}" "${RESET}"

    sed -i "s|^ *labelled corpus *:.*$|    labelled corpus: ${total}/${total}|" \
        "${DETECTION_JOURNAL}"

    if make -C "${REPO_ROOT}" --no-print-directory score >/dev/null 2>&1; then
        printf '  %s✗ the « score » gate let the drop through%s\n' "${RED}" "${RESET}"
        failures=$((failures + 1))
    else
        printf '  %s✓ « make score » failed, as expected%s\n' "${GREEN}" "${RESET}"
    fi

    restore

    proof_header '%s▸ a detection score above its record%s\n' "${BOLD}" "${RESET}"

    sed -i "s|^ *labelled corpus *:.*$|    labelled corpus: $((right - 1))/${total}|" \
        "${DETECTION_JOURNAL}"

    if make -C "${REPO_ROOT}" --no-print-directory score >/dev/null 2>&1; then
        printf '  %s✓ « make score » let it through, as expected%s\n' "${GREEN}" "${RESET}"
    else
        printf '  %s✗ the « score » gate refused a better detection%s\n' "${RED}" "${RESET}"
        failures=$((failures + 1))
    fi

    restore
}

expect_detection_score_gates

# The conversion loss, and its record — issue #339.
#
# **The same two proofs as for the score, and for the same reason**: the check
# has three outcomes and only one is a refusal. A loss that worsens is a
# regression and must fail; a loss that shrinks is good news and must above all
# not fail, otherwise the first pull request that improves a conversion would run
# into its own progress.
#
# **The injection is in the record and not in the conversion.** Moving the
# journal's figure tests the comparison itself, whatever the measured value;
# degrading a conversion by hand would prove the same thing less surely, and it
# would have to be recompiled.
expect_conversion_loss_gates() {
    local recorded
    recorded="$(sed -n 's/^ *intact round trips *: *\([0-9]*\)\/\([0-9]*\) *$/\1 \2/p' \
        "${CONVERSION_JOURNAL}")"
    local kept="${recorded% *}"
    local total="${recorded#* }"

    proof_header '%s▸ a conversion loss greater than its record%s\n' "${BOLD}" "${RESET}"

    sed -i "s|^ *intact round trips *:.*$|    intact round trips: ${total}/${total}|" \
        "${CONVERSION_JOURNAL}"

    if make -C "${REPO_ROOT}" --no-print-directory conversion >/dev/null 2>&1; then
        printf '  %s✗ the « conversion » gate let the loss through%s\n' "${RED}" "${RESET}"
        failures=$((failures + 1))
    else
        printf '  %s✓ « make conversion » failed, as expected%s\n' "${GREEN}" "${RESET}"
    fi

    restore

    proof_header '%s▸ a conversion loss smaller than its record%s\n' "${BOLD}" "${RESET}"

    sed -i "s|^ *intact round trips *:.*$|    intact round trips: $((kept - 1))/${total}|" \
        "${CONVERSION_JOURNAL}"

    if make -C "${REPO_ROOT}" --no-print-directory conversion >/dev/null 2>&1; then
        printf '  %s✓ « make conversion » let it through, as expected%s\n' "${GREEN}" "${RESET}"
    else
        printf '  %s✗ the « conversion » gate refused a better conversion%s\n' \
            "${RED}" "${RESET}"
        failures=$((failures + 1))
    fi

    restore
}

expect_conversion_loss_gates

# The format detection score, and its record — issue #340.
#
# **The same two proofs as for the other two records**, and for the same
# reason: the check has three outcomes and only one is a refusal.
#
# **The drop is injected into the corpus, not into the record** — and it is a
# correction. The first version raised the record to `total/total`: right as
# long as detection missed files, silent since the record reached 19/19.
# Raising a perfect score to itself changes nothing, the gate rightly let it
# through, and the proof failed without any gate being at fault. A file
# labelled SubRip that the detector refuses makes the rate drop whatever the
# record, which is the situation to prove.
#
# **The check's third outcome is not tested here**, and that is deliberate: a
# confusion fails on its own, whatever the rate, and injecting one would require
# a degraded detector — hence a recompilation, to prove a three-line branch.
expect_format_score_gates() {
    local recorded
    recorded="$(sed -n 's/^ *recognised formats *: *\([0-9]*\)\/\([0-9]*\) *$/\1 \2/p' \
        "${FORMAT_JOURNAL}")"
    local right="${recorded% *}"
    local total="${recorded#* }"

    proof_header '%s▸ a format detection score below its record%s\n' "${BOLD}" "${RESET}"

    printf 'not a subtitle\n' > "${FORMAT_STRAY}"

    if make -C "${REPO_ROOT}" --no-print-directory score-format >/dev/null 2>&1; then
        printf '  %s✗ the « score-format » gate let the drop through%s\n' "${RED}" "${RESET}"
        failures=$((failures + 1))
    else
        printf '  %s✓ « make score-format » failed, as expected%s\n' "${GREEN}" "${RESET}"
    fi

    restore

    proof_header '%s▸ a format detection score above its record%s\n' "${BOLD}" "${RESET}"

    sed -i "s|^ *recognised formats *:.*$|    recognised formats: $((right - 1))/${total}|" \
        "${FORMAT_JOURNAL}"

    if make -C "${REPO_ROOT}" --no-print-directory score-format >/dev/null 2>&1; then
        printf '  %s✓ « make score-format » let it through, as expected%s\n' "${GREEN}" "${RESET}"
    else
        printf '  %s✗ the « score-format » gate refused a better detection%s\n' \
            "${RED}" "${RESET}"
        failures=$((failures + 1))
    fi

    restore
}

expect_format_score_gates

# The scorer cannot name a file — issue #290.
#
# **What can go wrong here is not one green too many, it is a leak.** The
# private corpus is never cited: no title, no file name, no figure attributed to
# one of them. A tool that walks it and *can* name it ends up naming it.
#
# It did. The first version passed the detection command to the shell with the
# path substituted in plain text; the corpus names carry spaces and brackets, so
# `file` received split arguments, complained about each one, and its
# complaints — which quoted the names — were read as encodings and then printed.
#
# Two guards came out of it, and this proof exercises both: the path is one
# element of argv, and what the detector returns is refused unless it has the
# shape of an encoding name. The detector of this proof does exactly what `file`
# did — it writes a path.
expect_scorer_never_names_a_file() {
    local work chatty output
    work="$(mktemp -d)"
    chatty="${work}/chatty.sh"

    proof_header '%s▸ a detector that spits out a path%s\n' "${BOLD}" "${RESET}"

    printf '#!/bin/sh\necho "cannot open %s"\n' \
        "/no/real/file/bears this name.srt" > "${chatty}"
    chmod +x "${chatty}"

    output="$("${REPO_ROOT}/src/scripts/score-encoding-detection.py" \
        --detector "${chatty} {}" 2>&1 || true)"

    if grep -q "bears this name" <<< "${output}"; then
        printf '  %s✗ the scorer printed what the detector gave it%s\n' \
            "${RED}" "${RESET}"
        failures=$((failures + 1))
    else
        printf '  %s✓ the answer was refused, as expected%s\n' "${GREEN}" "${RESET}"
    fi

    rm -rf "${work}"
}

expect_scorer_never_names_a_file

# The gate refuses that a run touches the user's configuration.
#
# **Two proofs and not one**, for the reason that asks for two from the
# previous one. Letting a written configuration through is the defect the gate
# exists to catch. Reporting a configuration that nothing touched would be
# worse: whoever develops subedit ends up running subedit, and a check that
# cried wolf at every gate would end up disabled.
#
# **It runs under a makeshift XDG_CONFIG_HOME.** Proving that a gate refuses
# writes to the home directory by writing there would be committing the defect
# to show that it is refused. The variable is the one every standard location
# derives from, and it is precisely the one the test harness moves: the proof
# borrows the mechanism it demonstrates.
expect_config_home_gate() {
    local script="${REPO_ROOT}/src/scripts/check-config-home.sh"
    local fake
    fake="$(mktemp -d)"

    proof_header '%s▸ a test that writes into the user configuration%s\n' "${BOLD}" "${RESET}"
    XDG_CONFIG_HOME="${fake}" "${script}" --record >/dev/null
    mkdir -p "${fake}/subedit"
    printf 'theme = dark\n' > "${fake}/subedit/settings.conf"
    if XDG_CONFIG_HOME="${fake}" make -C "${REPO_ROOT}" --no-print-directory config-home \
        >/dev/null 2>&1; then
        printf '  %s✗ the « config-home » gate let the write through%s\n' "${RED}" "${RESET}"
        failures=$((failures + 1))
    else
        printf '  %s✓ « make config-home » failed, as expected%s\n' "${GREEN}" "${RESET}"
    fi

    proof_header '%s▸ a configuration already there that nothing touched%s\n' "${BOLD}" "${RESET}"
    XDG_CONFIG_HOME="${fake}" "${script}" --record >/dev/null
    if XDG_CONFIG_HOME="${fake}" make -C "${REPO_ROOT}" --no-print-directory config-home \
        >/dev/null 2>&1; then
        printf '  %s✓ « make config-home » let it through, as expected%s\n' "${GREEN}" "${RESET}"
    else
        printf '  %s✗ the « config-home » gate reported an untouched configuration%s\n' \
            "${RED}" "${RESET}"
        failures=$((failures + 1))
    fi

    rm -rf "${fake}"

    # The record is made again under the real directory, so that what follows
    # does not start from the fingerprint of an XDG_CONFIG_HOME that no longer
    # exists.
    "${script}" --record >/dev/null
}

expect_config_home_gate

# The manual's screenshots — #199.
#
# **Two proofs, for the two pieces that do not see the same thing.** The
# comparator sees only the screenshots a run has just produced: it can say "this
# image has moved", and nothing else. The guard sees the three lists — what the
# program generates, what the manual shows, what exists on disk — and it is the
# only piece that can catch an image that nothing generates any more.
expect_screenshot_gates() {
    proof_header '%s▸ a screenshot that no longer matches the window%s\n' "${BOLD}" "${RESET}"
    # Another screenshot of the same repository, with different dimensions: the
    # defect is injected without making an image, and without having to describe
    # one.
    cp "${OTHER_CAPTURE}" "${CAPTURE_REFERENCE}"
    if make -C "${REPO_ROOT}" --no-print-directory screenshots-check >/dev/null 2>&1; then
        printf '  %s✗ the « screenshots-check » gate let the image through%s\n' \
            "${RED}" "${RESET}"
        failures=$((failures + 1))
    else
        printf '  %s✓ « make screenshots-check » failed, as expected%s\n' \
            "${GREEN}" "${RESET}"
    fi
    restore

    proof_header '%s▸ an image the manual shows and nothing generates%s\n' "${BOLD}" "${RESET}"
    printf '\n![An image nobody generates.](captures/missing.png)\n' \
        >> "${GUI_MANUAL_SOURCE}"
    if "${REPO_ROOT}/src/scripts/check-screenshots.py" >/dev/null 2>&1; then
        printf '  %s✗ the guard let the missing image through%s\n' "${RED}" "${RESET}"
        failures=$((failures + 1))
    else
        printf '  %s✓ « check-screenshots.py » refused, as expected%s\n' "${GREEN}" "${RESET}"
    fi
    restore
}

expect_screenshot_gates

# Chaque capture claire a sa sombre — #400.
#
# **Quatre preuves, sur un jeu écrit à la main.** Le manuel promettait deux fois
# que chaque écran se montre sous les deux palettes, et deux écrans de la phase
# 10 sont entrés avec la seule claire : le contrôle vérifiait qu une image
# montrée est engendrée, pas qu elle a sa paire. Le défaut se prouve sur un
# petit dépôt de fortune — un programme de capture de trois lignes, une page, des
# fichiers vides — plutôt qu en retirant une vraie image : la preuve ne touche
# pas aux captures du manuel, et elle ne dépend pas de ce qu elles montrent un
# jour.
#
# **Quatre, parce que la garde a deux façons de refuser et deux de laisser
# passer.** Elle refuse une paire que le programme n engendre pas, et nomme
# l image, sans quoi le message ne dirait pas où chercher ; elle refuse une
# paire que le programme engendre mais que le manuel ne montre pas, parce que la
# promesse est faite au lecteur et non au programme de capture ; elle laisse
# passer une exemption motivée, sans quoi un écran qui n a vraiment qu une
# palette serait condamné à la fausse paire ; et elle laisse passer le jeu
# complet, sans quoi elle crierait au loup à chaque exécution.
#
# L exemption est une liste du script, vide aujourd hui : la preuve la remplit en
# chargeant le script comme un module, plutôt que de lui donner une option qui
# permettrait de s exempter depuis la ligne de commande.
expect_screenshot_pairs() {
    local script="${REPO_ROOT}/src/scripts/check-screenshots.py"
    local root output
    root="$(mktemp -d)"

    mkdir -p "${root}/src/test/tools" "${root}/docs/manual/subedit-gui/captures"
    local tool="${root}/src/test/tools/screenshots.cpp"
    local page="${root}/docs/manual/subedit-gui/page.md"
    local captures="${root}/docs/manual/subedit-gui/captures"

    # Un écran complet, et un second qui n a que sa claire.
    printf '%s\n' \
        'capture(w, w, d, "ecran");' \
        'capture(w, w, d, "ecran-sombre");' \
        'capture(w, w, d, "seul");' > "${tool}"
    printf '%s\n' \
        '![Clair.](captures/ecran.png)' \
        '![Sombre.](captures/ecran-sombre.png)' \
        '![Seul.](captures/seul.png)' > "${page}"
    touch "${captures}/ecran.png" "${captures}/ecran-sombre.png" "${captures}/seul.png"

    proof_header '%s▸ une capture claire dont la sombre manque%s\n' "${BOLD}" "${RESET}"
    if output="$("${script}" --root "${root}" 2>&1)"; then
        printf '  %s✗ le garde-fou a laissé passer la paire manquante%s\n' "${RED}" "${RESET}"
        failures=$((failures + 1))
    elif [[ "${output}" != *"seul.png"* || "${output}" == *"ecran.png"* ]]; then
        printf '  %s✗ le garde-fou a refusé sans nommer la bonne image%s\n' "${RED}" "${RESET}"
        failures=$((failures + 1))
    else
        printf '  %s✓ « check-screenshots.py » a refusé et nommé seul.png, comme attendu%s\n' \
            "${GREEN}" "${RESET}"
    fi

    proof_header '%s▸ un écran exempté, avec sa raison%s\n' "${BOLD}" "${RESET}"
    if python3 - "${script}" "${root}" >/dev/null 2>&1 <<'PY'
import importlib.util
import sys

spec = importlib.util.spec_from_file_location("check_screenshots", sys.argv[1])
module = importlib.util.module_from_spec(spec)
spec.loader.exec_module(module)
module.SINGLE_PALETTE["seul"] = "un écran de preuve, qui n a qu une palette"
sys.exit(module.main(["--root", sys.argv[2]]))
PY
    then
        printf '  %s✓ l exemption a levé la garde, comme attendu%s\n' "${GREEN}" "${RESET}"
    else
        printf '  %s✗ l exemption motivée n a pas levé la garde%s\n' "${RED}" "${RESET}"
        failures=$((failures + 1))
    fi

    proof_header '%s▸ un jeu où chaque capture a sa paire%s\n' "${BOLD}" "${RESET}"
    printf '%s\n' 'capture(w, w, d, "seul-sombre");' >> "${tool}"
    printf '%s\n' '![Seul, sombre.](captures/seul-sombre.png)' >> "${page}"
    touch "${captures}/seul-sombre.png"
    if "${script}" --root "${root}" >/dev/null 2>&1; then
        printf '  %s✓ « check-screenshots.py » a laissé passer le jeu complet, comme attendu%s\n' \
            "${GREEN}" "${RESET}"
    else
        printf '  %s✗ le garde-fou a refusé un jeu où chaque capture a sa paire%s\n' \
            "${RED}" "${RESET}"
        failures=$((failures + 1))
    fi

    # Le jeu complet, dont la page ne montre plus la sombre de `seul` : le
    # programme l engendre et le fichier existe, mais le lecteur ne la voit pas.
    proof_header '%s▸ une sombre engendrée que le manuel ne montre pas%s\n' "${BOLD}" "${RESET}"
    printf '%s\n' \
        '![Clair.](captures/ecran.png)' \
        '![Sombre.](captures/ecran-sombre.png)' \
        '![Seul.](captures/seul.png)' > "${page}"
    if output="$("${script}" --root "${root}" 2>&1)"; then
        printf '  %s✗ le garde-fou a laissé passer la sombre absente du manuel%s\n' \
            "${RED}" "${RESET}"
        failures=$((failures + 1))
    elif [[ "${output}" != *"PAIR"*"seul.png"* || "${output}" == *"ecran.png"* ]]; then
        printf '  %s✗ le garde-fou a refusé sans nommer la bonne image%s\n' "${RED}" "${RESET}"
        failures=$((failures + 1))
    else
        printf '  %s✓ « check-screenshots.py » a refusé et nommé seul.png, comme attendu%s\n' \
            "${GREEN}" "${RESET}"
    fi

    rm -rf "${root}"
}

expect_screenshot_pairs

# One set of screenshots per language — #672.
#
# Every manual tree shows its own captures, taken under its own language, and
# the guard reads each tree on its own, whatever its language. Three proofs on
# scratch copies, nothing of the repository touched: a capture of a
# translated tree deleted is refused and named; a translated page showing an
# image of its tree that nothing produces is refused as frozen; a translated
# page borrowing the English set is refused as foreign — a window in the wrong
# language. The first is a copy of the real program and manuals; the others a
# hand-written set, as the pair proofs above.
expect_screenshot_trees() {
    local script="${REPO_ROOT}/src/scripts/check-screenshots.py"
    local root output
    root="$(mktemp -d)"

    proof_header '%s▸ a French capture deleted%s\n' "${BOLD}" "${RESET}"
    mkdir -p "${root}/real/src/test/tools" "${root}/real/docs"
    cp "${REPO_ROOT}/src/test/tools/screenshots.cpp" "${root}/real/src/test/tools/"
    cp -r "${REPO_ROOT}/docs/manual" "${REPO_ROOT}/docs/i18n" "${root}/real/docs/"
    rm "${root}/real/docs/i18n/fr/manual/subedit-gui/captures/fenetre.png"
    if output="$("${script}" --root "${root}/real" 2>&1)"; then
        printf '  %s✗ the guard let a missing French capture through%s\n' "${RED}" "${RESET}"
        failures=$((failures + 1))
    elif [[ "${output}" == *"BROKEN"*"fenetre.png"*"i18n/fr/manual/subedit-gui/captures"* ]]; then
        printf '  %s✓ « check-screenshots.py » refused and named fenetre.png of the French set%s\n' \
            "${GREEN}" "${RESET}"
    else
        printf '  %s✗ the guard refused, but not for the right reason%s\n' "${RED}" "${RESET}"
        failures=$((failures + 1))
    fi

    mkdir -p "${root}/set/src/test/tools" "${root}/set/docs/manual/subedit-gui/captures" \
        "${root}/set/docs/i18n/fr/manual/subedit-gui/captures"
    local english="${root}/set/docs/manual/subedit-gui"
    local french="${root}/set/docs/i18n/fr/manual/subedit-gui"
    printf '%s\n' 'capture(w, w, d, "ecran");' 'capture(w, w, d, "ecran-sombre");' \
        > "${root}/set/src/test/tools/screenshots.cpp"
    printf '%s\n' '![Light.](captures/ecran.png)' '![Dark.](captures/ecran-sombre.png)' \
        > "${english}/page.md"
    cp "${english}/page.md" "${french}/page.md"
    touch "${english}/captures/ecran.png" "${english}/captures/ecran-sombre.png" \
        "${french}/captures/ecran.png" "${french}/captures/ecran-sombre.png"
    if ! "${script}" --root "${root}/set" >/dev/null 2>&1; then
        printf '  %s✗ the guard refuses two complete sets%s\n' "${RED}" "${RESET}"
        failures=$((failures + 1))
    fi

    proof_header '%s▸ a French page showing a capture nothing produces%s\n' "${BOLD}" "${RESET}"
    touch "${french}/captures/gelee.png"
    printf '%s\n' '![Frozen.](captures/gelee.png)' >> "${french}/page.md"
    if output="$("${script}" --root "${root}/set" 2>&1)"; then
        printf '  %s✗ the guard let a frozen capture through%s\n' "${RED}" "${RESET}"
        failures=$((failures + 1))
    elif [[ "${output}" == *"FROZEN"*"gelee.png"* ]]; then
        printf '  %s✓ « check-screenshots.py » refused and named gelee.png, as expected%s\n' \
            "${GREEN}" "${RESET}"
    else
        printf '  %s✗ the guard refused, but not for the right reason%s\n' "${RED}" "${RESET}"
        failures=$((failures + 1))
    fi

    proof_header '%s▸ a French page showing the English capture%s\n' "${BOLD}" "${RESET}"
    cp "${english}/page.md" "${french}/page.md"
    printf '%s\n' '![English.](../../../../manual/subedit-gui/captures/ecran.png)' >> "${french}/page.md"
    if output="$("${script}" --root "${root}/set" 2>&1)"; then
        printf '  %s✗ the guard let a borrowed capture through%s\n' "${RED}" "${RESET}"
        failures=$((failures + 1))
    elif [[ "${output}" == *"FOREIGN"*"page.md"* ]]; then
        printf '  %s✓ « check-screenshots.py » refused the borrowed capture, as expected%s\n' \
            "${GREEN}" "${RESET}"
    else
        printf '  %s✗ the guard refused, but not for the right reason%s\n' "${RED}" "${RESET}"
        failures=$((failures + 1))
    fi

    rm -rf "${root}"
}

expect_screenshot_trees

# Les renvois du manuel — #243.
#
# Le troisième filet du manuel, après les blocs engendrés et les captures. Une
# seule preuve : le contrôle n a pas de mode d échec par excès qui se démontre
# — il ne compare pas deux relevés, il résout des chemins et des ancres, et le
# vert du dépôt intact est déjà la preuve qu il ne crie pas au loup.
expect_manual_link_gate() {
    proof_header '%s▸ un renvoi du manuel qui ne mène nulle part%s\n' "${BOLD}" "${RESET}"
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

# The structure of a translated manual — #656.
#
# Two proofs: the faithful fixture passes (the check does not cry wolf), and one drift, a code
# block that is no longer the source's, is refused and named alone. The drift is injected into a
# copy of the versioned fixtures in a scratch directory, so nothing of the repository is touched.
# The other drifts (pages, headings, tables, links, images, inline code, list items) are replayed
# by `--check-fixtures`, which is a step of the fixtures gate.
expect_translation_structure_gate() {
    local script="${REPO_ROOT}/src/scripts/check-translation-structure.py"
    local root output
    root="$(mktemp -d)"
    cp -r "${REPO_ROOT}/src/test/data/traductions/docs" "${root}/docs"

    proof_header '%s▸ a faithful translation of the manual%s\n' "${BOLD}" "${RESET}"
    if "${script}" --source "${root}/docs/manual" --translation "${root}/docs/i18n/es/manual" \
        >/dev/null 2>&1; then
        printf '  %s✓ « check-translation-structure.py » accepted the faithful translation%s\n' \
            "${GREEN}" "${RESET}"
    else
        printf '  %s✗ the check refuses a faithful translation%s\n' "${RED}" "${RESET}"
        failures=$((failures + 1))
    fi

    proof_header '%s▸ a translation whose code block differs from the source%s\n' "${BOLD}" "${RESET}"
    sed -i 's/^done$/hecho/' "${root}/docs/i18n/es/manual/sub/page.md"
    if output="$("${script}" --source "${root}/docs/manual" \
        --translation "${root}/docs/i18n/es/manual" 2>&1)"; then
        printf '  %s✗ the check let a changed code block through%s\n' "${RED}" "${RESET}"
        failures=$((failures + 1))
    elif [[ "${output}" == *"CODE BLOCK"*"sub/page.md"* && "${output}" == *"1 difference(s)"* ]]; then
        printf '  %s✓ « check-translation-structure.py » refused and named the code block%s\n' \
            "${GREEN}" "${RESET}"
    else
        printf '  %s✗ the check refused, but not for the right reason%s\n' "${RED}" "${RESET}"
        failures=$((failures + 1))
    fi

    rm -rf "${root}"
}

expect_translation_structure_gate

# Le manuel de `subedit-cli` et son `--help` — #546.
#
# **Quatre preuves, une par défaut que le contrôle promet de voir**, et une pour
# le vert : une sous-commande sans page, une option de l'aide qu'aucun tableau ne
# porte, une option d'un tableau que l'aide ne connaît pas, et une option
# globale que la page `subedit-cli(1)` oublie. Chaque injection porte sur **une
# copie** du manuel, dans un répertoire jetable : le contrôle accepte le manuel
# et la page de manuel en arguments, si bien que rien du dépôt n'est touché et
# que `restore` n'a rien à rétablir.
expect_cli_manual_gate() {
    local script="${REPO_ROOT}/src/scripts/check-cli-manual.py"
    local binary="${REPO_ROOT}/build/dev/bin/subedit-cli"
    local real_manual="${REPO_ROOT}/docs/manual/subedit-cli"
    local real_man_page="${REPO_ROOT}/packaging/subedit-cli.1.in"
    local root
    root="$(mktemp -d)"

    proof_header '%s▸ le manuel de subedit-cli, tel qu il est%s\n' "${BOLD}" "${RESET}"
    if "${script}" --binary "${binary}" >/dev/null 2>&1; then
        printf '  %s✓ « check-cli-manual.py » a laissé passer le manuel intact, comme attendu%s\n' \
            "${GREEN}" "${RESET}"
    else
        printf '  %s✗ le contrôle refuse le manuel intact%s\n' "${RED}" "${RESET}"
        failures=$((failures + 1))
    fi

    expect_cli_manual_refused() {
        local label="$1" expected="$2"
        local output
        if output="$("${script}" --binary "${binary}" --manual "${root}/manual" \
            --man-page "${root}/subedit-cli.1.in" 2>&1)"; then
            printf '  %s✗ le contrôle a laissé passer : %s%s\n' "${RED}" "${label}" "${RESET}"
            failures=$((failures + 1))
        elif [[ "${output}" == *"${expected}"* && "${output}" == *"1 mismatch(es)"* ]]; then
            printf '  %s✓ « check-cli-manual.py » a refusé, et nommé seul cet écart : %s%s\n' \
                "${GREEN}" "${label}" "${RESET}"
        else
            printf '  %s✗ le contrôle a refusé, mais pas pour la bonne raison : %s%s\n' \
                "${RED}" "${label}" "${RESET}"
            failures=$((failures + 1))
        fi
    }
    fresh_copy() {
        rm -rf "${root}/manual" "${root}/subedit-cli.1.in"
        cp -r "${real_manual}" "${root}/manual"
        cp "${real_man_page}" "${root}/subedit-cli.1.in"
    }

    proof_header '%s▸ une sous-commande sans page de manuel%s\n' "${BOLD}" "${RESET}"
    fresh_copy
    rm "${root}/manual/shift.md"
    expect_cli_manual_refused "shift sans page" "MISSING PAGE"

    proof_header '%s▸ une option de l aide qu aucun tableau ne porte%s\n' "${BOLD}" "${RESET}"
    fresh_copy
    sed -i 's/| `--by` |/| (retirée) |/' "${root}/manual/shift.md"
    expect_cli_manual_refused "--by retirée des tableaux" "UNDOCUMENTED OPTION"

    proof_header '%s▸ une option d un tableau que l aide ne connaît pas%s\n' "${BOLD}" "${RESET}"
    fresh_copy
    printf '\n| Option | Requis |\n| :----- | :----- |\n| `--inventee` | non |\n' \
        >> "${root}/manual/shift.md"
    expect_cli_manual_refused "--inventee" "UNKNOWN OPTION"

    proof_header '%s▸ une option globale que la page de manuel oublie%s\n' "${BOLD}" "${RESET}"
    fresh_copy
    sed -i 's/\\-\\-encoding/\\-\\-oubliee/' "${root}/subedit-cli.1.in"
    expect_cli_manual_refused "--encoding absente de la page de manuel" "MISSING GLOBAL OPTION"

    rm -rf "${root}"
}

expect_cli_manual_gate

# The manual of `subedit-gui` and the window's shortcuts — #612, #672.
#
# **One proof per defect the check promises to see**, and one for green: a table row naming an
# action the window does not have, a shortcut written that the action does not answer, a "none"
# written of an action that has one, a page whose tables are headed in another language's words,
# **an action whose shortcut is written nowhere** — in the English tree and in the French one,
# each against the window in its own language — a French table quoting the English label, and a
# tree in a language the window cannot be built in. As for the command-line manual, every
# injection is made in **a copy** of the manual: the check takes the manual as an argument, and
# the language of a copy is that of its path (`docs/i18n/<lang>/manual/`), so nothing of the
# repository is touched. One proof injects on the window's side — a fake program adding an action
# to the real list —, because the defect is writing nothing, and one does not remove from a manual
# what was not in it.
expect_gui_manual_gate() {
    local script="${REPO_ROOT}/src/scripts/check-gui-manual.py"
    local binary="${REPO_ROOT}/build/dev/bin/subedit_list_shortcuts"
    local real_manual="${REPO_ROOT}/docs/manual/subedit-gui"
    local real_french="${REPO_ROOT}/docs/i18n/fr/manual/subedit-gui"
    local root
    root="$(mktemp -d)"
    local french="${root}/docs/i18n/fr/manual/subedit-gui"

    proof_header '%s▸ the manual of subedit-gui, as it is%s\n' "${BOLD}" "${RESET}"
    if "${script}" --binary "${binary}" >/dev/null 2>&1; then
        printf '  %s✓ « check-gui-manual.py » let the intact manual through, as expected%s\n' \
            "${GREEN}" "${RESET}"
    else
        printf '  %s✗ the check refuses the intact manual%s\n' "${RED}" "${RESET}"
        failures=$((failures + 1))
    fi

    expect_gui_manual_refused() {
        local label="$1" expected="$2" with_binary="${3:-${binary}}" manual="${4:-${root}/manual}"
        shift 4 2>/dev/null || shift $#
        local output
        if output="$("${script}" --binary "${with_binary}" --manual "${manual}" "$@" 2>&1)"; then
            printf '  %s✗ the check let it through: %s%s\n' "${RED}" "${label}" "${RESET}"
            failures=$((failures + 1))
        elif [[ "${output}" == *"${expected}"* && "${output}" == *"1 difference(s)"* ]]; then
            printf '  %s✓ « check-gui-manual.py » refused, and named this difference alone: %s%s\n' \
                "${GREEN}" "${label}" "${RESET}"
        else
            printf '  %s✗ the check refused, but not for the right reason: %s%s\n' \
                "${RED}" "${label}" "${RESET}"
            failures=$((failures + 1))
        fi
    }
    fresh_manual() {
        rm -rf "${root}/manual" "${root}/docs"
        cp -r "${real_manual}" "${root}/manual"
        mkdir -p "$(dirname "${french}")"
        cp -r "${real_french}" "${french}"
    }

    proof_header '%s▸ a table row naming an action the window does not have%s\n' "${BOLD}" "${RESET}"
    fresh_manual
    printf '\n| Command | Shortcut |\n| :------ | :------- |\n| Nonexistent | `Ctrl+Q` |\n' \
        >> "${root}/manual/video.md"
    expect_gui_manual_refused "an invented action" "UNKNOWN ACTION"

    proof_header '%s▸ a shortcut written that the action does not answer%s\n' "${BOLD}" "${RESET}"
    fresh_manual
    sed -i 's/| `Ctrl+P` | plays if/| `Ctrl+Q` | plays if/' "${root}/manual/player.md"
    expect_gui_manual_refused "Ctrl+Q for Play / Pause" "UNKNOWN SHORTCUT"

    proof_header '%s▸ a "none" written of an action that has a shortcut%s\n' "${BOLD}" "${RESET}"
    fresh_manual
    sed -i 's/| `Ctrl+P` | plays if/| none | plays if/' "${root}/manual/player.md"
    expect_gui_manual_refused "none for Play / Pause" "FORGOTTEN SHORTCUT"

    proof_header '%s▸ a French page in the English tree, read as English%s\n' "${BOLD}" "${RESET}"
    fresh_manual
    cp "${real_french}/player.md" "${root}/manual/player.md"
    expect_gui_manual_refused "the French player.md in the English tree" "PAGE LANGUAGE IN DOUBT"

    proof_header '%s▸ an action whose shortcut is written nowhere%s\n' "${BOLD}" "${RESET}"
    fresh_manual
    cat > "${root}/fake-program" <<FAKE
#!/usr/bin/env bash
"${binary}" "\$@"
printf 'New action\tCtrl+Alt+Q\n'
FAKE
    chmod +x "${root}/fake-program"
    expect_gui_manual_refused "Ctrl+Alt+Q" "UNDOCUMENTED SHORTCUT" "${root}/fake-program"

    proof_header '%s▸ a shortcut row removed from the English manual%s\n' "${BOLD}" "${RESET}"
    fresh_manual
    sed -i '/`Ctrl+Shift+W`/d' "${root}/manual/files.md"
    expect_gui_manual_refused "Ctrl+Shift+W (English)" "UNDOCUMENTED SHORTCUT"

    proof_header '%s▸ the French manual, against the French window%s\n' "${BOLD}" "${RESET}"
    fresh_manual
    if "${script}" --binary "${binary}" --manual "${french}" >/dev/null 2>&1; then
        printf '  %s✓ « check-gui-manual.py » let the intact French manual through, as expected%s\n' \
            "${GREEN}" "${RESET}"
    else
        printf '  %s✗ the check refuses the intact French manual%s\n' "${RED}" "${RESET}"
        failures=$((failures + 1))
    fi

    proof_header '%s▸ a shortcut row removed from the French manual%s\n' "${BOLD}" "${RESET}"
    fresh_manual
    sed -i '/`Ctrl+Shift+W`/d' "${french}/files.md"
    expect_gui_manual_refused "Ctrl+Shift+W (French)" "UNDOCUMENTED SHORTCUT" "${binary}" "${french}"

    proof_header '%s▸ a French table quoting the English label%s\n' "${BOLD}" "${RESET}"
    fresh_manual
    sed -i 's/| Tout fermer | `Ctrl+Shift+W` |/| Close All | `Ctrl+Shift+W` |/' "${french}/files.md"
    expect_gui_manual_refused "Close All in the French tree" "UNKNOWN ACTION" "${binary}" "${french}"

    # A language with a vocabulary table, so that the page is read, and no catalogue, so that the
    # window cannot be built in it: the French table lent under another name.
    proof_header '%s▸ a manual in a language the window cannot be built in%s\n' "${BOLD}" "${RESET}"
    fresh_manual
    local unsupported="${root}/docs/i18n/xx/manual/subedit-gui"
    mkdir -p "$(dirname "${unsupported}")"
    cp -r "${real_french}" "${unsupported}"
    python3 -c 'import json, sys; sys.path.insert(0, sys.argv[1]); import doc_vocabulary; print(json.dumps({"xx": doc_vocabulary.TABLES["fr"]}))' \
        "${REPO_ROOT}/src/scripts" > "${root}/vocabulary-xx.json"
    expect_gui_manual_refused "a manual in xx" "WINDOW LANGUAGE UNAVAILABLE" "${binary}" "${unsupported}" \
        --vocabulary "${root}/vocabulary-xx.json"

    rm -rf "${root}"
}

expect_gui_manual_gate

# Les contrôles du manuel lisent le vocabulaire de la langue de la page — #657.
#
# **Deux preuves par contrôle, et la même page pour les deux** : une page espagnole de quelques
# lignes, sous `docs/i18n/es/manual/`, est acceptée quand une table espagnole est fournie, et
# **refusée, en le disant, quand elle ne l'est pas**. Le défaut est le silence : un contrôle qui
# cherche « Shortcut » sur une page qui écrit « Atajo » ne trouve aucun tableau, et se tait.
# The fixture is in a language with no shipped table on purpose: English and French have theirs
# since #671, and the proof is about any language that does not, whichever it is.
# Les deux programmes sont de faux binaires qui déclarent juste de quoi faire concorder la page,
# pour qu'aucun autre écart ne se mêle au seul qu'on cherche.
expect_vocabulary_gate() {
    local fixture="${REPO_ROOT}/src/test/data/vocabulaire"
    local root
    root="$(mktemp -d)"

    cat > "${root}/fake-gui" <<'FAKE'
#!/usr/bin/env bash
printf 'Play / Pause\tCtrl+P\n'
FAKE
    cat > "${root}/fake-cli" <<'FAKE'
#!/usr/bin/env bash
case "$*" in
--help) printf 'Fake.\nUsage: fake\n\nOptions:\n  -h,--help  Print\n\nSubcommands:\n  shift  Move\n' ;;
"shift --help") printf 'Move.\nUsage: fake shift\n\nOptions:\n  -h,--help  Print\n  --by TEXT  Amount\n' ;;
esac
FAKE
    chmod +x "${root}/fake-gui" "${root}/fake-cli"

    local tree="${fixture}/docs/i18n/es/manual" table="${fixture}/vocabulary-es.json"
    local name script manual binary output counted
    local -a extra
    for name in gui cli; do
        script="${REPO_ROOT}/src/scripts/check-${name}-manual.py"
        manual="${tree}/subedit-${name}"
        binary="${root}/fake-${name}"
        extra=()
        counted="1 difference(s)"
        [[ "${name}" == cli ]] && extra=(--man-page "${root}/absent.1.in") && counted="1 écart(s)"

        proof_header '%s▸ a Spanish page, with its vocabulary table (%s)%s\n' "${BOLD}" "${name}" "${RESET}"
        if "${script}" --binary "${binary}" --manual "${manual}" "${extra[@]}" --vocabulary "${table}" >/dev/null 2>&1; then
            printf '  %s✓ « check-%s-manual.py » read the Spanish page, as expected%s\n' "${GREEN}" "${name}" "${RESET}"
        else
            printf '  %s✗ the check refuses a Spanish page that has its table%s\n' "${RED}" "${RESET}"
            failures=$((failures + 1))
        fi

        proof_header '%s▸ la même page, sans table de vocabulaire (%s)%s\n' "${BOLD}" "${name}" "${RESET}"
        if output="$("${script}" --binary "${binary}" --manual "${manual}" "${extra[@]}" 2>&1)"; then
            printf '  %s✗ le contrôle a laissé passer une page dont il ne connaît pas la langue%s\n' "${RED}" "${RESET}"
            failures=$((failures + 1))
        elif [[ "${output}" == *"LANGUAGE WITHOUT VOCABULARY"* && "${output}" == *"${counted}"* ]]; then
            printf '  %s✓ « check-%s-manual.py » a refusé, et nommé seul cet écart : la langue sans table%s\n' \
                "${GREEN}" "${name}" "${RESET}"
        else
            printf '  %s✗ le contrôle a refusé, mais pas pour la bonne raison%s\n' "${RED}" "${RESET}"
            failures=$((failures + 1))
        fi
    done

    rm -rf "${root}"
}

expect_vocabulary_gate

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
    proof_header '%s▸ des règles install() qui oublient le manuel%s\n' "${BOLD}" "${RESET}"

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

# Les règles d installation oublient les motifs de correction — #498.
#
# **Le manuel, lui, est installé** : c est ce qui distingue cette preuve de la
# précédente, qui retire tout sauf les binaires et ne dirait donc rien du
# contrôle des motifs — la porte échouerait déjà sur le manuel. Ici, le manuel
# passe, et seul ce contrôle peut refuser.
expect_installation_gate_without_patterns() {
    proof_header '%s▸ des règles install() qui oublient les motifs de correction%s\n' "${BOLD}" "${RESET}"

    cat > "${INSTALLATION_SOURCE}" <<'BROKEN'
include(GNUInstallDirs)
install(TARGETS subedit-cli subedit-gui RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")
install(
    DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/docs/manual/"
    DESTINATION "${CMAKE_INSTALL_DATADIR}/subedit/manual")
BROKEN

    if make -C "${REPO_ROOT}" --no-print-directory install-check >/dev/null 2>&1; then
        printf '  %s✗ la porte « install-check » a laissé passer les motifs absents%s\n' \
            "${RED}" "${RESET}"
        failures=$((failures + 1))
    else
        printf '  %s✓ « make install-check » a échoué, comme attendu%s\n' "${GREEN}" "${RESET}"
    fi

    restore
}

expect_installation_gate_without_patterns

# Les catalogues de langue qu'aucune règle n'installe — #659.
#
# **Ils vivent dans `cmake/Translations.cmake` et non dans `Installation.cmake`**, si bien que
# les preuves précédentes, qui réécrivent ce dernier, ne les touchent pas — et qu'aucune ne dit
# rien d'un catalogue. Ici l'installation reste intacte (manuel, motifs), et seul le contrôle des
# catalogues peut refuser. Le défaut retire la règle `install()` du `.mo`, ce que ferait une
# langue ajoutée à `LINGUAS` par quelqu'un qui compile sans installer.
expect_installation_gate_without_catalogues() {
    proof_header '%s▸ des règles install() qui oublient les catalogues de langue%s\n' "${BOLD}" "${RESET}"

    sed -i '/^    install(FILES "\${mo}"/,+1d' "${TRANSLATIONS_SOURCE}"
    if cmp -s "${TRANSLATIONS_SOURCE}" "${backup_dir}/Translations.cmake"; then
        printf '  %s✗ l injection n a rien retiré : la règle install() a changé de forme%s\n' "${RED}" "${RESET}"
        failures=$((failures + 1))
        restore
        return
    fi

    if make -C "${REPO_ROOT}" --no-print-directory install-check >/dev/null 2>&1; then
        printf '  %s✗ la porte « install-check » a laissé passer les catalogues absents%s\n' \
            "${RED}" "${RESET}"
        failures=$((failures + 1))
    else
        printf '  %s✓ « make install-check » a échoué, comme attendu%s\n' "${GREEN}" "${RESET}"
    fi

    restore
}

expect_installation_gate_without_catalogues

# Un gabarit des messages qui n'est plus ce que le code produit — #659. Un littéral neuf passé à
# `translate` et un gabarit qu'on n'a pas réécrit : c'est la ligne que le traducteur ne verrait jamais.
expect_gate_closes \
    "gabarit des messages périmé" \
    "pot-check" \
    "${LIB_SOURCE}" \
    'namespace subedit::core { const char* probeForTheProof() { return translate("injected message").data(); } }' \
    "périmé"

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
    proof_header '%s▸ un fichier .desktop que sa validation refuse%s\n' "${BOLD}" "${RESET}"

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
    proof_header '%s▸ une icône que gdk-pixbuf ne reconnaît pas%s\n' "${BOLD}" "${RESET}"

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

    proof_header '%s▸ une icône qui ne se voit pas sur un fond sombre%s\n' "${BOLD}" "${RESET}"

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
    proof_header '%s▸ un .rpm qui revendique les répertoires de la distribution%s\n' "${BOLD}" "${RESET}"

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

    proof_header '%s▸ %s%s\n' "${BOLD}" "choix des exécutions à supprimer" "${RESET}"

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

# Du même genre que la précédente, pour les releases — issue #232. Supprimer
# une release emporte ses paquets, et GitHub ne les rend pas.
#
# Le jeu couvre ce qui décide du fichier :
#
#   v0.9.0, v0.10.0   mineures        restent toujours
#   v0.9.3, v0.9.12   patchs passés   partent — et 0.9.12 compté comme nombre
#   v0.10.2, v0.10.9  patchs en cours restent : la milestone en cours est 0.10,
#                                     que « 0.9 » précéderait comme texte
#   v1.0.0-rc, notes  hors forme      ni comptés ni touchés
#
# Le second passage pousse une mineure plus haute, v0.11.0 : c'est l'événement
# qui fait basculer les patchs de 0.10 dans le passé.
expect_release_prune_selection_holds() {
    proof_header '%s▸ %s%s\n' "${BOLD}" "choix des releases à supprimer" "${RESET}"

    local fixture
    fixture="$(mktemp)"
    local status=0
    local actual

    printf '%s\n' v0.9.0 v0.9.3 v0.9.12 v0.10.0 v0.10.2 v0.10.9 v1.0.0-rc notes > "${fixture}"
    actual="$("${RELEASE_PRUNE_SCRIPT}" --dry-run --input "${fixture}" 2>/dev/null \
        | sort | tr '\n' ' ')" || status=$?
    check_release_selection "milestone 0.10 en cours" "${status}" "${actual% }" "v0.9.12 v0.9.3"

    status=0
    printf '%s\n' v0.11.0 >> "${fixture}"
    actual="$("${RELEASE_PRUNE_SCRIPT}" --dry-run --input "${fixture}" 2>/dev/null \
        | sort | tr '\n' ' ')" || status=$?
    check_release_selection "v0.11.0 publiée" "${status}" "${actual% }" \
        "v0.10.2 v0.10.9 v0.9.12 v0.9.3"

    rm -f "${fixture}"
}

check_release_selection() {
    local label="$1" status="$2" actual="$3" expected="$4"

    if (( status != 0 )); then
        printf '  %s✗ %s : la sélection est morte, code %d%s\n' "${RED}" "${label}" "${status}" "${RESET}"
        failures=$((failures + 1))
    elif [[ "${actual}" == "${expected}" ]]; then
        printf '  %s✓ %s : « %s », comme attendu%s\n' "${GREEN}" "${label}" "${actual}" "${RESET}"
    else
        printf '  %s✗ %s : « %s », attendu « %s »%s\n' \
            "${RED}" "${label}" "${actual}" "${expected}" "${RESET}"
        failures=$((failures + 1))
    fi
}

expect_release_prune_selection_holds

# Trois preuves de plus sur le même script — issue #399. La précédente éprouve
# ce que le script choisit ; celles-ci éprouvent ce qu'il fait de son choix, et
# cela demande de laisser le script supprimer pour de bon, contre un `gh` qui
# n'est pas le vrai.
#
# **Le `gh` de fortune est le point délicat, et il ne touche à rien.** Il est
# posé en tête du `PATH` d'un répertoire à usage unique, il répond à
# `gh api …/releases` par un fichier écrit à la main — et applique le vrai
# filtre `--jq` du script, avec `jq`, pour que les brouillons soient écartés
# par le code qu'on éprouve et non par la fixture — et à `gh release delete` en
# écrivant l'appel dans un journal, en refusant ceux qu'on lui désigne. Toute
# autre commande est notée « INATTENDU » et échoue. Le harnais refuse de lancer
# quoi que ce soit si `gh` ne se résout pas vers lui : aucune de ces preuves ne
# peut donc atteindre le dépôt distant, et le jeton passé est un faux.
#
# Les trois s'exécutent sous `GITHUB_EVENT_NAME=push`, l'événement réel de la
# publication : une garde de `workflow_dispatch` qui se déclencherait à tort
# ferait échouer les deux premières.
make_fake_gh() {
    local sandbox="$1"

    mkdir -p "${sandbox}/bin"
    : > "${sandbox}/calls.log"
    cat > "${sandbox}/bin/gh" <<'FIN'
#!/usr/bin/env bash
# Un gh de fortune : il lit un fichier, il écrit un journal, il ne va nulle part.
set -u
log="${FAKE_GH_DIR}/calls.log"
case "${1:-} ${2:-}" in
    "api "*)
        expression=""
        arguments=("$@")
        for ((index = 0; index < ${#arguments[@]} - 1; index++)); do
            if [[ "${arguments[index]}" == "--jq" ]]; then
                expression="${arguments[index + 1]}"
            fi
        done
        printf 'api %s\n' "$*" >> "${log}"
        jq -r "${expression}" "${FAKE_GH_DIR}/releases.json"
        ;;
    "release delete")
        printf 'release delete %s\n' "${*:3}" >> "${log}"
        for refused in ${FAKE_GH_REFUSE:-}; do
            if [[ "$3" == "${refused}" ]]; then
                printf 'HTTP 403 : suppression refusée\n' >&2
                exit 1
            fi
        done
        ;;
    *)
        printf 'INATTENDU %s\n' "$*" >> "${log}"
        exit 99
        ;;
esac
FIN
    chmod +x "${sandbox}/bin/gh"
}

# Écrit la liste des releases de l'API : `tag` pour une release publiée,
# `tag:draft` pour un brouillon.
write_fake_releases() {
    local sandbox="$1"
    shift

    jq -n '[$ARGS.positional[] | split(":")
            | {tag_name: .[0], draft: (.[1] == "draft")}]' \
        --args "$@" > "${sandbox}/releases.json"
}

# Lance le vrai script contre le gh de fortune. Le code de sortie va dans
# `prune_status`, les sorties dans `${sandbox}/out` et `${sandbox}/err`. Les
# arguments après le répertoire sont des affectations d'environnement.
prune_status=0
run_release_pruner() {
    local sandbox="$1"
    shift

    if [[ "$(PATH="${sandbox}/bin:${PATH}" command -v gh)" != "${sandbox}/bin/gh" ]]; then
        printf '  %s✗ gh ne se résout pas vers le gh de fortune : rien n est lancé%s\n' \
            "${RED}" "${RESET}"
        prune_status=99
        return
    fi

    prune_status=0
    env -u GITHUB_STEP_SUMMARY \
        PATH="${sandbox}/bin:${PATH}" \
        GH_TOKEN=jeton-de-fortune \
        GITHUB_REPOSITORY=exemple/subedit \
        FAKE_GH_DIR="${sandbox}" \
        "$@" \
        "${RELEASE_PRUNE_SCRIPT}" >"${sandbox}/out" 2>"${sandbox}/err" || prune_status=$?
}

# Les tags dont la suppression a été tentée, dans l'ordre, séparés par un espace.
attempted_deletions() {
    awk '$1 == "release" && $2 == "delete" { print $3 }' "$1/calls.log" | tr '\n' ' '
}

# Écrit le verdict d'un cas : `problems` est vide s'il est tenu.
report_release_prune_case() {
    local label="$1" problems="$2"

    if [[ -z "${problems}" ]]; then
        printf '  %s✓ %s, comme attendu%s\n' "${GREEN}" "${label}" "${RESET}"
    else
        printf '  %s✗ %s : %s%s\n' "${RED}" "${label}" "${problems}" "${RESET}"
        failures=$((failures + 1))
    fi
}

# Un refus est rouge, et il ne dispense pas des suppressions suivantes.
#
# Deux des trois patchs passés sont refusés, le second des trois ne l'est pas :
# un script qui s'arrêterait au premier refus n'aurait tenté ni le deuxième ni
# le troisième, et un script qui sortirait en 0 rendrait vert un élagage qui
# n'en est pas un. Le même jeu tient les deux gardes de #232 : la `v0.9.0` est
# une mineure et ne doit jamais être tentée, et aucun appel ne porte
# `--cleanup-tag`, qui emporterait le tag avec la release.
expect_release_prune_fails_on_refusal() {
    proof_header '%s▸ %s%s\n' "${BOLD}" "élagage de releases dont une suppression est refusée" "${RESET}"

    local sandbox problems=""
    sandbox="$(mktemp -d)"
    make_fake_gh "${sandbox}"
    write_fake_releases "${sandbox}" v0.9.0 v0.9.3 v0.9.5 v0.9.7 v0.10.0 v0.10.2
    run_release_pruner "${sandbox}" GITHUB_EVENT_NAME=push FAKE_GH_REFUSE="v0.9.3 v0.9.7"

    local attempted
    attempted="$(attempted_deletions "${sandbox}")"
    [[ "${attempted}" == "v0.9.3 v0.9.5 v0.9.7 " ]] \
        || problems+="suppressions tentées « ${attempted% } », attendu « v0.9.3 v0.9.5 v0.9.7 » ; "
    (( prune_status != 0 )) || problems+="sorti en 0 malgré deux refus ; "
    grep -q 'v0.9.3' "${sandbox}/err" || problems+="le refus de v0.9.3 n est pas nommé ; "
    grep -q 'v0.9.7' "${sandbox}/err" || problems+="le refus de v0.9.7 n est pas nommé ; "
    ! grep -q 'v0.9.5' "${sandbox}/err" || problems+="v0.9.5, supprimée, est nommée comme refusée ; "
    ! grep -q -e '--cleanup-tag' "${sandbox}/calls.log" \
        || problems+="un appel porte --cleanup-tag, qui supprimerait le tag ; "

    report_release_prune_case "un refus fait sortir en erreur, les autres suppressions ont lieu" "${problems}"
    rm -rf "${sandbox}"
}

expect_release_prune_fails_on_refusal

# Un brouillon n'est pas une version. `gh api …/releases` les rend avec les
# autres ; un brouillon `v0.11.0` créé à la main ferait de 0.11 la milestone en
# cours, et le `v0.10.2` de la vraie milestone en cours partirait avec son
# paquet. Le brouillon `v0.9.5`, lui, est un patch passé : il ne doit pas être
# supprimé non plus, puisqu'il n'est pas une release.
expect_release_prune_ignores_drafts() {
    proof_header '%s▸ %s%s\n' "${BOLD}" "élagage de releases en présence de brouillons" "${RESET}"

    local sandbox problems=""
    sandbox="$(mktemp -d)"
    make_fake_gh "${sandbox}"
    write_fake_releases "${sandbox}" \
        v0.9.0 v0.9.3 v0.9.5:draft v0.10.0 v0.10.2 v0.11.0:draft
    run_release_pruner "${sandbox}" GITHUB_EVENT_NAME=push

    local attempted
    attempted="$(attempted_deletions "${sandbox}")"
    [[ "${attempted}" == "v0.9.3 " ]] \
        || problems+="suppressions tentées « ${attempted% } », attendu « v0.9.3 » ; "
    (( prune_status == 0 )) || problems+="code de sortie ${prune_status}, attendu 0 ; "

    report_release_prune_case "un brouillon n'est ni une milestone ni une release à supprimer" "${problems}"
    rm -rf "${sandbox}"
}

expect_release_prune_ignores_drafts

# Une reconstruction à la main n'élague pas. `release.yml` offre
# `workflow_dispatch` pour un paquet à refaire ; sur un patch d'une milestone
# close, la publication recrée la release et l'élagage la supprimerait dans le
# même travail. Le jeu est celui du premier cas, où trois patchs partiraient.
expect_release_prune_skips_manual_rebuild() {
    proof_header '%s▸ %s%s\n' "${BOLD}" "élagage de releases lors d'une reconstruction à la demande" "${RESET}"

    local sandbox problems=""
    sandbox="$(mktemp -d)"
    make_fake_gh "${sandbox}"
    write_fake_releases "${sandbox}" v0.9.0 v0.9.3 v0.9.5 v0.9.7 v0.10.0 v0.10.2
    run_release_pruner "${sandbox}" GITHUB_EVENT_NAME=workflow_dispatch

    local attempted
    attempted="$(attempted_deletions "${sandbox}")"
    [[ -z "${attempted}" ]] || problems+="suppressions tentées « ${attempted% } », attendu aucune ; "
    (( prune_status == 0 )) || problems+="code de sortie ${prune_status}, attendu 0 ; "
    grep -q 'workflow_dispatch' "${sandbox}/out" \
        || problems+="la sortie ne dit pas qu elle n élague pas à la demande ; "

    report_release_prune_case "une reconstruction à la demande ne supprime rien et le dit" "${problems}"
    rm -rf "${sandbox}"
}

expect_release_prune_skips_manual_rebuild

# Le garde du workflow de release, sur ses deux refus qui ne demandent aucun tag
# à poser : un nom qui n est pas une version, et une version qui ne désigne
# aucun commit. Le troisième — une version en désaccord avec son CMakeLists —
# exigerait de poser un tag dans le dépôt, ce qu une preuve n a pas à faire.
expect_release_tag_check_refuses() {
    proof_header '%s▸ %s%s\n' "${BOLD}" "tag de release refusé" "${RESET}"

    local tag
    for tag in 0.10.9 v0.10 v999.999.999; do
        if "${RELEASE_TAG_CHECK}" "${tag}" >/dev/null 2>&1; then
            printf '  %s✗ check-release-tag.sh a accepté « %s »%s\n' "${RED}" "${tag}" "${RESET}"
            failures=$((failures + 1))
        else
            printf '  %s✓ « %s » refusé, comme attendu%s\n' "${GREEN}" "${tag}" "${RESET}"
        fi
    done
}

expect_release_tag_check_refuses

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

    proof_header '%s▸ %s%s\n' "${BOLD}" "arbre de couverture périmé par un déplacement" "${RESET}"

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

    proof_header '%s▸ %s%s\n' "${BOLD}" "choix des extrêmes du journal des mesures" "${RESET}"

    work="$(mktemp -d)"
    journal="${work}/performances.md"

    write_bench_xml_with_sd() {
        printf '%s\n' \
            '<?xml version="1.0" encoding="UTF-8"?>' \
            '<Catch2TestRun name="verify">' \
            '  <TestCase name="t">' \
            "    <BenchmarkResults name=\"$1\" samples=\"1\" resamples=\"0\" iterations=\"1\" clockResolution=\"1\" estimatedDuration=\"1\">" \
            "      <mean value=\"$2\" lowerBound=\"$2\" upperBound=\"$2\" ci=\"0.95\"/>" \
            "      <standardDeviation value=\"$3\" lowerBound=\"$3\" upperBound=\"$3\" ci=\"0.95\"/>" \
            '    </BenchmarkResults>' \
            '  </TestCase>' \
            '</Catch2TestRun>' > "${work}/run.xml"
    }

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
        '# Journal' '' '<!-- extremes -->' '' \
        '| Measure | Minimum | Recorded on | Maximum | Recorded on |' \
        '| :----- | ------: | :-------- | ------: | :-------- |' \
        '| ancienne | 100 ns | 0.0.1 — 2020-01-01 | 300 ns | 0.0.1 — 2020-01-01 |' '' \
        '<!-- ancienne min=100.0 max=300.0 -->' '' \
        '## Records' '' '<!-- records -->' '' > "${journal}"

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

    # **Le second critère : la dispersion** — issue #202. La charge lit la
    # machine, pas la mesure. Un relevé pris à charge 1,44 — donc admis — a posé
    # un maximum définitif sur un ticket qui ne changeait aucun `.cpp`, avec un
    # écart-type valant 26 % de sa moyenne là où cette mesure-là se tient entre
    # 3 et 8 %.
    #
    # L historique du journal jetable est écrit à 5 % de dispersion sur six
    # relevés, ce qui donne un seuil de 6,25 % : le relevé ci-dessous est calme
    # et six fois plus dispersé que d habitude.
    printf '%s\n' \
        '# Journal' '' '<!-- extremes -->' '' \
        '| Measure | Minimum | Recorded on | Maximum | Recorded on |' \
        '| :----- | ------: | :-------- | ------: | :-------- |' \
        '| stable | 1000 ns | 0.0.1 — 2020-01-01 | 1000 ns | 0.0.1 — 2020-01-01 |' '' \
        '<!-- stable min=1000.0 max=1000.0 -->' '' \
        '## Records' '' '<!-- records -->' '' > "${journal}"
    local passe
    for passe in 1 2 3 4 5 6; do
        printf '%s\n' \
            "### 0.0.${passe} — $(date +%F) — Release — load 0.5 — pace ×1.00" '' \
            '| Measure | Mean | Standard deviation |' '| :----- | ------: | ---------: |' \
            '| stable | 1000 ns | 50 ns |' '' >> "${journal}"
    done

    write_bench_xml_with_sd "stable" 2000 520
    "${script}" --xml "${work}/run.xml" --mode Release --load 0.5 --below 1.5 \
        --journal "${journal}" >/dev/null 2>&1 || status=$?
    if grep -q '<!-- stable min=1000.0 max=2000.0 -->' "${journal}"; then
        printf '  %s✗ un relevé six fois plus dispersé que d habitude a posé son maximum%s\n' \
            "${RED}" "${RESET}"
        failures=$((failures + 1))
        rm -rf "${work}"
        return
    fi

    # Et le pendant : la mesure entre quand même au journal. Ce qui lui est
    # refusé est l enveloppe, pas l historique.
    if ! grep -q '| stable | 2 µs |' "${journal}"; then
        printf '  %s✗ un relevé dispersé a été écarté du journal, pas seulement de la table%s\n' \
            "${RED}" "${RESET}"
        failures=$((failures + 1))
        rm -rf "${work}"
        return
    fi

    printf '  %s✓ neuve écartée à chaud, posée au calme, reprise quand sa source est remplacée,%s\n' \
        "${GREEN}" "${RESET}"
    printf '  %s  et une mesure trop dispersée entre au journal sans toucher à l enveloppe%s\n' \
        "${GREEN}" "${RESET}"
    rm -rf "${work}"
}

expect_bench_extremes_hold

# Une preuve du même genre, sur l autre moitié de la règle — issue #270.
#
# `await-quiet.sh` ne refuse rien non plus : il décide si la machine est assez
# calme, et c est son code de retour qui fait qu un relevé entre au journal ou
# n y entre pas. Ce qui peut être faux chez lui va dans les deux sens — déclarer
# calme une machine qui ne l est pas, ou attendre une accalmie qui ne viendra
# jamais.
#
# Les deux se démontrent sans attendre quoi que ce soit, en lui donnant un seuil
# qui rend la réponse certaine : aucun `sleep` n est joué ici.
expect_await_quiet_decides() {
    local script="${REPO_ROOT}/src/scripts/await-quiet.sh"
    local load
    local status

    proof_header '%s▸ %s%s\n' "${BOLD}" "la décision de mesurer ou non" "${RESET}"

    # Le script a une échappatoire pour une machine sans /proc/loadavg — il rend
    # « unknown » et zéro. Cette preuve-là n a alors rien à démontrer, et le
    # dire vaut mieux que la faire échouer pour une raison qui ne regarde pas le
    # dépôt.
    if [[ ! -r /proc/loadavg ]]; then
        printf '  %s✓ pas de /proc/loadavg : la décision ne s applique pas ici%s\n' \
            "${GREEN}" "${RESET}"
        return
    fi

    # Un seuil qu aucune charge ne peut dépasser : il doit rendre 0 tout de
    # suite, et écrire un nombre.
    status=0
    load="$("${script}" --below 100000 --timeout 0)" || status=$?
    if (( status != 0 )) || [[ ! "${load}" =~ ^[0-9]+([.,][0-9]+)?$ ]]; then
        printf '  %s✗ une machine calme n a pas été reconnue (code %s, charge « %s »)%s\n' \
            "${RED}" "${status}" "${load}" "${RESET}"
        failures=$((failures + 1))
        return
    fi

    # Un seuil qu aucune charge ne peut atteindre : il doit renoncer, rendre 1,
    # et écrire la charge quand même — c est elle que l appelant affiche.
    status=0
    load="$("${script}" --below 0 --timeout 0)" || status=$?
    if (( status == 0 )) || [[ ! "${load}" =~ ^[0-9]+([.,][0-9]+)?$ ]]; then
        printf '  %s✗ une machine occupée a été déclarée calme (code %s, charge « %s »)%s\n' \
            "${RED}" "${status}" "${load}" "${RESET}"
        failures=$((failures + 1))
        return
    fi

    printf '  %s✓ calme reconnu, occupation refusée, et la charge écrite dans les deux cas%s\n' \
        "${GREEN}" "${RESET}"
}

expect_await_quiet_decides

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

    proof_header '%s▸ %s%s\n' "${BOLD}" "${label}" "${RESET}"

    if "${REPO_ROOT}/src/scripts/gate.sh" "$@" >/dev/null 2>&1; then
        printf '  %s✗ gate.sh a accepté « %s »%s\n' "${RED}" "$*" "${RESET}"
        failures=$((failures + 1))
    else
        printf '  %s✓ gate.sh a refusé, comme attendu%s\n' "${GREEN}" "${RESET}"
    fi
}

expect_orchestrator_refuses "étape inconnue passée à --from" check --from covrage
expect_orchestrator_refuses "filtres qui ne retiennent aucune étape" check --only tidy --skip tidy

# La même garde, sur le second script qui sait reprendre au milieu — issue #271.
#
# `check-installation.sh` a reçu `--only` pour la raison qui l a donné à
# `gate.sh` : rejouer douze contrôles pour en corriger un. Le mode d échec vient
# avec, et c est le pire de tous — `--only paqets` jouerait zéro contrôle et
# rendrait zéro, donc **une installation déclarée correcte que personne n a
# regardée.**
#
# La preuve ne coûte rien : le refus a lieu avant que le premier contrôle ne
# tourne, donc rien n est installé, rien n est empaqueté.
expect_installation_refuses_unknown_control() {
    proof_header '%s▸ %s%s\n' "${BOLD}" "contrôle inconnu passé à --only" "${RESET}"

    if "${REPO_ROOT}/src/scripts/check-installation.sh" --only paqets >/dev/null 2>&1; then
        printf '  %s✗ check-installation.sh a accepté « paqets »%s\n' "${RED}" "${RESET}"
        failures=$((failures + 1))
    else
        printf '  %s✓ check-installation.sh a refusé, comme attendu%s\n' "${GREEN}" "${RESET}"
    fi
}

expect_installation_refuses_unknown_control

# Les attendus de `--format json` — #556.
#
# **Six preuves, sur un dossier jetable écrit à la main** : un nombre à
# virgule, une ligne qui n'est pas du JSON, une enveloppe incomplète, un lancement
# à blanc qui dit une destination, un changement sans « after », et le vert.
# Le contrôle accepte le dossier en argument, si bien que rien du dépôt n'est
# touché : un attendu volontairement faux ne peut pas rester derrière.
expect_json_fixture_gate() {
    local script="${REPO_ROOT}/src/scripts/check-json-fixtures.py"
    local root
    root="$(mktemp -d)"

    proof_header '%s▸ un attendu JSON qui contient un nombre à virgule%s\n' "${BOLD}" "${RESET}"
    printf '{"schema":1,"command":"inspect","file":"a.srt","ok":true,"ratio":1.5,"warnings":[]}\n' \
        > "${root}/bad.jsonl"
    if "${script}" --dir "${root}" >/dev/null 2>&1; then
        printf '  %s✗ le contrôle a laissé passer 1.5%s\n' "${RED}" "${RESET}"
        failures=$((failures + 1))
    else
        printf '  %s✓ « check-json-fixtures.py » a refusé, comme attendu%s\n' \
            "${GREEN}" "${RESET}"
    fi

    proof_header '%s▸ une ligne qui n est pas du JSON%s\n' "${BOLD}" "${RESET}"
    printf '{"schema":1,"command":\n' > "${root}/bad.jsonl"
    if "${script}" --dir "${root}" >/dev/null 2>&1; then
        printf '  %s✗ le contrôle a laissé passer une ligne tronquée%s\n' "${RED}" "${RESET}"
        failures=$((failures + 1))
    else
        printf '  %s✓ « check-json-fixtures.py » a refusé, comme attendu%s\n' \
            "${GREEN}" "${RESET}"
    fi

    proof_header '%s▸ une enveloppe incomplète : pas de « file »%s\n' "${BOLD}" "${RESET}"
    printf '{"schema":1,"command":"inspect","ok":true,"warnings":[]}\n' > "${root}/bad.jsonl"
    if "${script}" --dir "${root}" >/dev/null 2>&1; then
        printf '  %s✗ le contrôle a laissé passer l enveloppe sans « file »%s\n' \
            "${RED}" "${RESET}"
        failures=$((failures + 1))
    else
        printf '  %s✓ « check-json-fixtures.py » a refusé, comme attendu%s\n' \
            "${GREEN}" "${RESET}"
    fi

    proof_header '%s▸ un lancement à blanc qui dit pourtant une destination%s\n' "${BOLD}" "${RESET}"
    printf '{"schema":1,"command":"shift","file":"a.srt","ok":true,"dry_run":true,"destination":"o.srt","counts":{},"warnings":[]}\n' \
        > "${root}/bad.jsonl"
    if "${script}" --dir "${root}" >/dev/null 2>&1; then
        printf '  %s✗ le contrôle a laissé passer une destination à blanc%s\n' \
            "${RED}" "${RESET}"
        failures=$((failures + 1))
    else
        printf '  %s✓ « check-json-fixtures.py » a refusé, comme attendu%s\n' \
            "${GREEN}" "${RESET}"
    fi

    proof_header '%s▸ un changement sans « after »%s\n' "${BOLD}" "${RESET}"
    printf '{"schema":1,"command":"hearing-impaired","file":"a.srt","ok":true,"dry_run":true,"destination":null,"counts":{},"changes":[{"subtitle":1,"document":"main","before":"x"}],"warnings":[]}\n' \
        > "${root}/bad.jsonl"
    if "${script}" --dir "${root}" >/dev/null 2>&1; then
        printf '  %s✗ le contrôle a laissé passer un changement sans « after »%s\n' \
            "${RED}" "${RESET}"
        failures=$((failures + 1))
    else
        printf '  %s✓ « check-json-fixtures.py » a refusé, comme attendu%s\n' \
            "${GREEN}" "${RESET}"
    fi

    proof_header '%s▸ un attendu juste, pour le vert%s\n' "${BOLD}" "${RESET}"
    printf '{"schema":1,"command":"inspect","file":"a.srt","ok":true,"warnings":[]}\n' \
        > "${root}/bad.jsonl"
    if "${script}" --dir "${root}" >/dev/null 2>&1; then
        printf '  %s✓ « check-json-fixtures.py » a laissé passer un attendu juste%s\n' \
            "${GREEN}" "${RESET}"
    else
        printf '  %s✗ le contrôle a refusé un attendu juste%s\n' "${RED}" "${RESET}"
        failures=$((failures + 1))
    fi

    rm -rf "${root}"
}

expect_json_fixture_gate

report_durations

if (( failures > 0 )); then
    printf '%s%d preuve(s) en échec%s\n' "${RED}" "${failures}" "${RESET}" >&2
    exit 1
fi
printf '%sles portes se referment%s\n' "${GREEN}" "${RESET}"
printf '%sle contrôle de parallélisme laisse passer le code légitime%s\n' \
    "${GREEN}" "${RESET}"
printf '%set l élagueur choisit les exécutions attendues%s\n' \
    "${GREEN}" "${RESET}"
printf '%set les releases élaguées sont les patchs des milestones passées%s\n' \
    "${GREEN}" "${RESET}"
printf '%set l élagage échoue sur un refus, ignore les brouillons, et reste au repos à la demande%s\n' \
    "${GREEN}" "${RESET}"
printf '%set le journal des mesures ne pose un extrême que sur un relevé propre%s\n' \
    "${GREEN}" "${RESET}"
printf '%set un fichier non suivi antérieur aux tests ne fait pas crier au loup%s\n' \
    "${GREEN}" "${RESET}"
printf '%sni une configuration d utilisateur que rien n a touchée%s\n' \
    "${GREEN}" "${RESET}"
