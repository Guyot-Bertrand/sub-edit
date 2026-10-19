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
# The man pages of #683 have four more, beside the five of the command-line
# manual: the pages generated from the manual pass the lint, which refuses a
# macro groff does not know and a byte outside ASCII, and the converter refuses
# a page of the manual written outside its subset — what fails the build.
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
# The French text some proofs inject on purpose lives here, as data, so that this script stays
# in English and off `.french-allowlist`.
readonly LANGUAGE_FIXTURES="${REPO_ROOT}/src/test/data/language"
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

# shellcheck source=src/scripts/lib/output.sh
source "${REPO_ROOT}/src/scripts/lib/output.sh"

# shellcheck source=src/scripts/lib/project.sh
source "${REPO_ROOT}/src/scripts/lib/project.sh"
# shellcheck source=src/scripts/gate/timing.sh
source "${REPO_ROOT}/src/scripts/gate/timing.sh"

backup_dir="$(mktemp -d)"
failures=0

# **The time of each proof** — #686. `verify-gates` went past the hour without anyone knowing
# where: every proof header goes through `proof_header`, which closes the measurement of the
# previous one, and the final report ranks the longest. Nothing is written to the repository: the
# duration is an observation of the run, to be reported in the pull request that changes it, not a
# file to keep up.
#
# **With what it was measured under**, as in `gate.sh`: the load at the start and end of each
# proof, the `JOBS` of the run, and one line per proof in the local journal that `make timings`
# reads (`gate/timing.sh`). The table is written on exit, by the trap, so that a run that dies or
# is interrupted still says where its time went.
#
# A label that comes back — « the tracked tree, as it is » opens two families — takes its rank,
# « (2) », so that the journal compares each proof with itself from one run to the next.
GATE_TIMINGS_RUN="$(date -Iseconds)"
run_started=${SECONDS}
run_load="$(timing_load)"
proof_names=()
proof_seconds=()
proof_loads=()
declare -A proof_seen=()
current_proof=""
proof_started=${SECONDS}
proof_load=""
durations_reported=0

end_proof() {
    if [[ -n "${current_proof}" ]]; then
        local seconds=$((SECONDS - proof_started)) load_end
        load_end="$(timing_load)"
        proof_names+=("${current_proof}")
        proof_seconds+=("${seconds}")
        local load
        printf -v load '%5s→%-5s' "${proof_load}" "${load_end}"
        proof_loads+=("${load}")
        timing_record verify-gates "${current_proof}" "${seconds}" "${proof_load}" "${load_end}"
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
    # No arithmetic on the key: a label quotes its datum, and `(( ))` would evaluate it.
    local rank="${proof_seen[${current_proof}]:-0}"
    rank=$((rank + 1))
    proof_seen[${current_proof}]="${rank}"
    ((rank == 1)) || current_proof+=" (${rank})"
    proof_started=${SECONDS}
    proof_load="$(timing_load)"
}

report_durations() {
    ((durations_reported == 0)) || return 0
    durations_reported=1
    end_proof
    local total=0 index
    for index in "${!proof_seconds[@]}"; do
        total=$((total + proof_seconds[index]))
    done
    printf '\n%sproof durations: %d s in all, longest first — %d proof(s), load %s→%s, JOBS=%s%s\n' \
        "${BOLD}" "${total}" "${#proof_seconds[@]}" "${run_load}" "$(timing_load)" "$(timing_jobs)" "${RESET}"
    for index in "${!proof_seconds[@]}"; do
        printf '%6d s  load %s  %s\n' "${proof_seconds[index]}" "${proof_loads[index]}" "${proof_names[index]}"
    done | sort -rn | sed -n "1,${VERIFY_GATES_TOP:-15}p"
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
    # **With its own mtime** (-p), unlike the others (#755): this header is included by some three
    # hundred translation units, and the proof compiles nothing — a fresh mtime would make the next
    # `tidy` analyse them all again, for a content that did not change.
    cp -p "${backup_dir}/subtitle_index.hpp" "${MODEL_SOURCE}"
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
    report_durations
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
cp -p "${MODEL_SOURCE}" "${backup_dir}/subtitle_index.hpp"
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
# Its French is the defect, so the snippet is read from a versioned sample,
# `src/test/data/language/injected-comment.cpp.txt`, rather than written here:
# test data is outside the language rule, this script is not.
expect_gate_closes \
    "French comment added to C++" \
    "arch" \
    "${LIB_SOURCE}" \
    "$(<"${LANGUAGE_FIXTURES}/injected-comment.cpp.txt")"

expect_pr_check_closes \
    "pull request body without « Closes #N »" \
    "closes" \
    "PR_BODY=This body describes the work, but no line in it closes the issue."

# The branch version copied from that of HEAD, which serves as the base: both
# then announce the same number. The copy is needed — merely naming HEAD as the
# base would assume that the working copy has not already gone past it, which an
# uncommitted bump would contradict, and the injection would no longer prove
# anything.
head_version="$(git -C "${REPO_ROOT}" show HEAD:CMakeLists.txt | project_version)"
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

# The marker keeps the spelling `<!-- example: … -->` that generate-manual.sh reads.
expect_gate_closes \
    "example whose command fails" \
    "manual-check" \
    "${MANUAL_SOURCE}" \
    '
<!-- example: echo hello; exit 3 -->
```console
$ echo hello; exit 3
hello
```'

expect_gate_closes \
    "marker not followed by a console block" \
    "manual-check" \
    "${MANUAL_SOURCE}" \
    '
<!-- example: subedit-cli -->
This paragraph does not start a console block.'

# A silent command: it is the case that would produce an empty block if the
# generator let it through.
expect_gate_closes \
    "example whose command produces nothing" \
    "manual-check" \
    "${MANUAL_SOURCE}" \
    '
<!-- example: true -->
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
# **Not on every run** (#686): it tests `cmake/Tidy.cmake`, which almost never moves. It runs when
# `.clang-tidy` or `cmake/Tidy.cmake` differ from `main`, or on request: `VERIFY_GATES_FULL=1`.
# It first cost a full reanalysis of the tree, twice (the key goes back to its original value on
# restore); it now reads Ninja's plan, below.
tidy_config_changed() {
    [[ -n "${VERIFY_GATES_FULL:-}" ]] && return 0
    local base
    base="$(git -C "${REPO_ROOT}" merge-base HEAD main 2>/dev/null)" || return 0
    ! git -C "${REPO_ROOT}" diff --quiet "${base}" -- .clang-tidy cmake/Tidy.cmake
}

# **The proof reads Ninja's plan instead of running the analysis.** What it must show is that a
# change of `.clang-tidy` makes the build system replay every analysis — the key in
# `cmake/Tidy.cmake`. That is a property of the plan, and `ninja -n` prints the plan in a second;
# running the 300 analyses to see them fail proves the same thing in half an hour. The rule that
# the injected line adds is not tested here: "pattern rejected by clang-tidy", above, shows that
# a finding fails the gate. The restore needs no replay either: a dry run writes nothing to
# Ninja's log, so the original key matches it again.
expect_tidy_config_replays() {
    local build_dir="${REPO_ROOT}/build/tidy"
    local total before replayed

    proof_header '%s▸ %s%s\n' "${BOLD}" "hardened configuration, analysis replayed" "${RESET}"
    cmake --preset tidy >/dev/null 2>&1 || true
    total="$(ninja -C "${build_dir}" -t targets all 2>/dev/null | grep -c '\.o:')"
    before="$(ninja -C "${build_dir}" -n 2>/dev/null | grep -c 'Building CXX')"

    printf '%s\n' '  - key: readability-identifier-naming.ParameterCase
    value: UPPER_CASE' >> "${TIDY_CONFIG}"
    cmake --preset tidy >/dev/null 2>&1 || true
    replayed="$(ninja -C "${build_dir}" -n 2>/dev/null | grep -c 'Building CXX')"

    if ((before >= total)); then
        printf '  %s✗ the tree was not up to date before the injection (%d of %d analyses pending): run make tidy first%s\n' \
            "${RED}" "${before}" "${total}" "${RESET}"
        failures=$((failures + 1))
    elif ((total > 0 && replayed >= total)); then
        printf '  %s✓ the plan replays all %d analyses, as expected%s\n' "${GREEN}" "${total}" "${RESET}"
    else
        printf '  %s✗ the plan replays %d of %d analyses after the configuration changed%s\n' \
            "${RED}" "${replayed}" "${total}" "${RESET}"
        failures=$((failures + 1))
    fi

    restore
    cmake --preset tidy >/dev/null 2>&1 || true
}

if tidy_config_changed; then
    expect_tidy_config_replays
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
# exactly of that form. Both titles are French on purpose: they are the defect,
# and they are read from `src/test/data/language/injected-titles.txt`, one per
# line, for the reason given at the French comment proof above.
expect_french_test_title_closes() {
    local one
    local -a titles
    mapfile -t titles < "${LANGUAGE_FIXTURES}/injected-titles.txt"
    if (( ${#titles[@]} != 2 )); then
        printf '  %s✗ injected-titles.txt must hold two titles, it holds %d%s\n' \
            "${RED}" "${#titles[@]}" "${RESET}"
        failures=$((failures + 1))
    fi

    for one in "${titles[@]}"; do
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
# French entry emptied is refused and named, and the same entry emptied in a seed (Dutch, one of
# the languages not yet shipped) passes — it is reported, its translated count one lower, and
# nothing fails.
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
    seed_before="$(grep '^nl ' <<<"${output}" || true)"

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
    sed -i "${empty_edit}" "${root}/src/po/nl.po"
    if output="$("${script}" --root "${root}" 2>&1)"; then
        seed_after="$(grep '^nl ' <<<"${output}" || true)"
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

# The review caveat of a machine-translated language agrees with `src/po/UNREVIEWED` — #720.
#
# The check reads `src/po/` and the Markdown of the manuals only, so the proofs run on a scratch
# copy of those tracked files. Four proofs: the copy as it is passes; French declared unreviewed,
# with the caveat in its catalogue, on every page of its manual and in the Language sections of the
# English manual, passes too — the convention as `src/po/README.md` writes it is one the check
# accepts; the same with the marker removed from one page is refused and names the page; and French
# reviewed again with its catalogue still carrying the caveat is refused, named.
expect_review_caveat_gate() {
    local script="${REPO_ROOT}/src/scripts/check-review-caveat.py"
    local root output page
    root="$(mktemp -d)"
    git -C "${REPO_ROOT}" ls-files -z src/po 'docs/manual/*.md' 'docs/i18n/*.md' \
        | (cd "${REPO_ROOT}" && tar --null -T - -cf -) | tar -xf - -C "${root}"
    local -r caveat='# Machine translation, not reviewed by a reader of the language (spec 15, D5).'

    proof_header '%s▸ the review caveat, as it is%s\n' "${BOLD}" "${RESET}"
    if "${script}" --root "${root}" >/dev/null 2>&1; then
        printf '  %s✓ « check-review-caveat.py » accepted the tree%s\n' "${GREEN}" "${RESET}"
    else
        printf '  %s✗ the check refuses the tree as it is%s\n' "${RED}" "${RESET}"
        failures=$((failures + 1))
    fi

    proof_header '%s▸ a language declared unreviewed, with its caveat everywhere%s\n' "${BOLD}" "${RESET}"
    printf 'fr\n' >>"${root}/src/po/UNREVIEWED"
    sed -i "1i ${caveat}" "${root}/src/po/fr.po"
    while IFS= read -r -d '' page; do
        sed -i '1a\\n<!-- machine-translated -->\n> Traduction automatique, en attente de relecture.' "${page}"
    done < <(find "${root}/docs/i18n/fr/manual" -name '*.md' -print0)
    while IFS= read -r -d '' page; do
        sed -i 's/^## Language$/&\n\n<!-- machine-translated: fr -->/' "${page}"
    done < <(find "${root}/docs/manual" -name '*.md' -print0)
    if output="$("${script}" --root "${root}" 2>&1)"; then
        printf '  %s✓ « check-review-caveat.py » accepted the caveat as the convention writes it%s\n' \
            "${GREEN}" "${RESET}"
    else
        printf '  %s✗ the check refuses the caveat written as the convention says:%s\n%s\n' \
            "${RED}" "${RESET}" "${output}"
        failures=$((failures + 1))
    fi

    proof_header '%s▸ a page of an unreviewed language without its marker%s\n' "${BOLD}" "${RESET}"
    sed -i '/^<!-- machine-translated -->$/d' "${root}/docs/i18n/fr/manual/index.md"
    if output="$("${script}" --root "${root}" 2>&1)"; then
        printf '  %s✗ the check let a page without its caveat through%s\n' "${RED}" "${RESET}"
        failures=$((failures + 1))
    elif [[ "${output}" == *"PAGE"*"docs/i18n/fr/manual/index.md"* && "${output}" == *"1 problem(s)"* ]]; then
        printf '  %s✓ « check-review-caveat.py » refused and named the page%s\n' "${GREEN}" "${RESET}"
    else
        printf '  %s✗ the check refused, but not for the right reason%s\n' "${RED}" "${RESET}"
        failures=$((failures + 1))
    fi

    proof_header '%s▸ a language signed off whose catalogue keeps the caveat%s\n' "${BOLD}" "${RESET}"
    rm -rf "${root}/docs"
    git -C "${REPO_ROOT}" ls-files -z 'docs/manual/*.md' 'docs/i18n/*.md' \
        | (cd "${REPO_ROOT}" && tar --null -T - -cf -) | tar -xf - -C "${root}"
    sed -i '/^fr$/d' "${root}/src/po/UNREVIEWED"
    if output="$("${script}" --root "${root}" 2>&1)"; then
        printf '  %s✗ the check let a caveat left behind through%s\n' "${RED}" "${RESET}"
        failures=$((failures + 1))
    elif [[ "${output}" == *"CATALOGUE LEFT"*"src/po/fr.po"* && "${output}" == *"1 problem(s)"* ]]; then
        printf '  %s✓ « check-review-caveat.py » refused and named the catalogue%s\n' "${GREEN}" "${RESET}"
    else
        printf '  %s✗ the check refused, but not for the right reason%s\n' "${RED}" "${RESET}"
        failures=$((failures + 1))
    fi

    rm -rf "${root}"
}

expect_review_caveat_gate

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

# Every light screenshot has its dark one — #400.
#
# **Four proofs, on a hand-written set.** The manual promised twice that every
# screen is shown under both palettes, and two screens of phase 10 came in with
# the light one only: the check verified that a shown image is produced, not
# that it has its pair. The defect is proven on a small makeshift repository — a
# three-line capture program, a page, empty files — rather than by removing a
# real image: the proof does not touch the manual's captures, and it does not
# depend on what they show one day.
#
# **Four, because the guard has two ways to refuse and two to let through.** It
# refuses a pair the program does not produce, and names the image, without
# which the message would not say where to look; it refuses a pair the program
# produces but the manual does not show, because the promise is made to the
# reader and not to the capture program; it lets a justified exemption through,
# without which a screen that truly has one palette only would be condemned to a
# fake pair; and it lets the complete set through, without which it would cry
# wolf on every run.
#
# The exemption is a list of the script, empty today: the proof fills it by
# loading the script as a module, rather than giving it an option that would
# allow exempting oneself from the command line.
expect_screenshot_pairs() {
    local script="${REPO_ROOT}/src/scripts/check-screenshots.py"
    local root output
    root="$(mktemp -d)"

    mkdir -p "${root}/src/test/tools" "${root}/docs/manual/subedit-gui/captures"
    local tool="${root}/src/test/tools/screenshots.cpp"
    local page="${root}/docs/manual/subedit-gui/page.md"
    local captures="${root}/docs/manual/subedit-gui/captures"

    # One complete screen, and a second one that has its light shot only. The
    # `-sombre` suffix is the one `check-screenshots.py` reads as dark.
    printf '%s\n' \
        'capture(w, w, d, "screen");' \
        'capture(w, w, d, "screen-sombre");' \
        'capture(w, w, d, "single");' > "${tool}"
    printf '%s\n' \
        '![Light.](captures/screen.png)' \
        '![Dark.](captures/screen-sombre.png)' \
        '![Single.](captures/single.png)' > "${page}"
    touch "${captures}/screen.png" "${captures}/screen-sombre.png" "${captures}/single.png"

    proof_header '%s▸ a light screenshot whose dark one is missing%s\n' "${BOLD}" "${RESET}"
    if output="$("${script}" --root "${root}" 2>&1)"; then
        printf '  %s✗ the guard let the missing pair through%s\n' "${RED}" "${RESET}"
        failures=$((failures + 1))
    elif [[ "${output}" != *"single.png"* || "${output}" == *"screen.png"* ]]; then
        printf '  %s✗ the guard refused without naming the right image%s\n' "${RED}" "${RESET}"
        failures=$((failures + 1))
    else
        printf '  %s✓ « check-screenshots.py » refused and named single.png, as expected%s\n' \
            "${GREEN}" "${RESET}"
    fi

    proof_header '%s▸ an exempted screen, with its reason%s\n' "${BOLD}" "${RESET}"
    if python3 - "${script}" "${root}" >/dev/null 2>&1 <<'PY'
import importlib.util
import sys

spec = importlib.util.spec_from_file_location("check_screenshots", sys.argv[1])
module = importlib.util.module_from_spec(spec)
spec.loader.exec_module(module)
module.SINGLE_PALETTE["single"] = "a proof screen, which has one palette only"
sys.exit(module.main(["--root", sys.argv[2]]))
PY
    then
        printf '  %s✓ the exemption lifted the guard, as expected%s\n' "${GREEN}" "${RESET}"
    else
        printf '  %s✗ the justified exemption did not lift the guard%s\n' "${RED}" "${RESET}"
        failures=$((failures + 1))
    fi

    proof_header '%s▸ a set where every screenshot has its pair%s\n' "${BOLD}" "${RESET}"
    printf '%s\n' 'capture(w, w, d, "single-sombre");' >> "${tool}"
    printf '%s\n' '![Single, dark.](captures/single-sombre.png)' >> "${page}"
    touch "${captures}/single-sombre.png"
    if "${script}" --root "${root}" >/dev/null 2>&1; then
        printf '  %s✓ « check-screenshots.py » let the complete set through, as expected%s\n' \
            "${GREEN}" "${RESET}"
    else
        printf '  %s✗ the guard refused a set where every screenshot has its pair%s\n' \
            "${RED}" "${RESET}"
        failures=$((failures + 1))
    fi

    # The complete set, whose page no longer shows the dark one of `single`: the
    # program produces it and the file exists, but the reader does not see it.
    proof_header '%s▸ a produced dark screenshot the manual does not show%s\n' "${BOLD}" "${RESET}"
    printf '%s\n' \
        '![Light.](captures/screen.png)' \
        '![Dark.](captures/screen-sombre.png)' \
        '![Single.](captures/single.png)' > "${page}"
    if output="$("${script}" --root "${root}" 2>&1)"; then
        printf '  %s✗ the guard let the dark one missing from the manual through%s\n' \
            "${RED}" "${RESET}"
        failures=$((failures + 1))
    elif [[ "${output}" != *"PAIR"*"single.png"* || "${output}" == *"screen.png"* ]]; then
        printf '  %s✗ the guard refused without naming the right image%s\n' "${RED}" "${RESET}"
        failures=$((failures + 1))
    else
        printf '  %s✓ « check-screenshots.py » refused and named single.png, as expected%s\n' \
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

# The cross-references of the manual — #243.
#
# The third net of the manual, after the generated blocks and the screenshots.
# A single proof: the check has no failure mode by excess that can be shown — it
# does not compare two readings, it resolves paths and anchors, and the green of
# the intact repository is already the proof that it does not cry wolf.
expect_manual_link_gate() {
    proof_header '%s▸ a manual cross-reference that leads nowhere%s\n' "${BOLD}" "${RESET}"
    printf '\n[A link to nothing](table.md#a-section-that-does-not-exist)\n' \
        >> "${GUI_MANUAL_SOURCE}"
    if "${REPO_ROOT}/src/scripts/check-manual-links.py" >/dev/null 2>&1; then
        printf '  %s✗ the check let the missing anchor through%s\n' "${RED}" "${RESET}"
        failures=$((failures + 1))
    else
        printf '  %s✓ « check-manual-links.py » refused, as expected%s\n' \
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

# The manual of `subedit-cli` and its `--help` — #546.
#
# **Five proofs, one per defect the check promises to see**, and one for green:
# a subcommand without a page, an option of the help that no table carries, an
# option of a table that the help does not know, a global option that the
# generated `subedit-cli(1)` man page forgets, and an option of a subcommand
# that its generated page forgets (#683). Every injection is made in **a copy**
# of the manual or of the pages, in a scratch directory: the check takes the
# manual and the man pages as arguments, so nothing of the repository is
# touched and `restore` has nothing to put back.
expect_cli_manual_gate() {
    local script="${REPO_ROOT}/src/scripts/check-cli-manual.py"
    local binary="${REPO_ROOT}/build/dev/bin/subedit-cli"
    local real_manual="${REPO_ROOT}/docs/manual/subedit-cli"
    local root
    root="$(mktemp -d)"

    proof_header '%s▸ the manual of subedit-cli, as it is%s\n' "${BOLD}" "${RESET}"
    if "${script}" --binary "${binary}" >/dev/null 2>&1; then
        printf '  %s✓ « check-cli-manual.py » let the intact manual through, as expected%s\n' \
            "${GREEN}" "${RESET}"
    else
        printf '  %s✗ the check refuses the intact manual%s\n' "${RED}" "${RESET}"
        failures=$((failures + 1))
    fi

    expect_cli_manual_refused() {
        local label="$1" expected="$2"
        local output
        if output="$("${script}" --binary "${binary}" --manual "${root}/manual" \
            --man-root "${root}/man" 2>&1)"; then
            printf '  %s✗ the check let it through: %s%s\n' "${RED}" "${label}" "${RESET}"
            failures=$((failures + 1))
        elif [[ "${output}" == *"${expected}"* && "${output}" == *"1 mismatch(es)"* ]]; then
            printf '  %s✓ « check-cli-manual.py » refused, and named this mismatch alone: %s%s\n' \
                "${GREEN}" "${label}" "${RESET}"
        else
            printf '  %s✗ the check refused, but not for the right reason: %s%s\n' \
                "${RED}" "${label}" "${RESET}"
            failures=$((failures + 1))
        fi
    }
    # The pages generated from the copy, as the build generates them: a defect injected into
    # the manual reaches them the way it would reach the installed ones.
    fresh_copy() {
        rm -rf "${root}/manual" "${root}/man"
        cp -r "${real_manual}" "${root}/manual"
        "${REPO_ROOT}/src/scripts/md-to-man.py" --manual "${real_manual}" \
            --output-dir "${root}/man/man1" --version 0 --manual-dir /nowhere >/dev/null
    }

    proof_header '%s▸ a subcommand without a manual page%s\n' "${BOLD}" "${RESET}"
    fresh_copy
    rm "${root}/manual/shift.md"
    expect_cli_manual_refused "shift without a page" "MISSING PAGE"

    proof_header '%s▸ an option of the help that no table carries%s\n' "${BOLD}" "${RESET}"
    fresh_copy
    sed -i 's/| `--by` |/| (removed) |/' "${root}/manual/shift.md"
    expect_cli_manual_refused "--by removed from the tables" "UNDOCUMENTED OPTION"

    proof_header '%s▸ an option of a table that the help does not know%s\n' "${BOLD}" "${RESET}"
    fresh_copy
    printf '\n| Option | Required |\n| :----- | :----- |\n| `--invented` | no |\n' \
        >> "${root}/manual/shift.md"
    expect_cli_manual_refused "--invented" "UNKNOWN OPTION"

    proof_header '%s▸ a global option that the man page forgets%s\n' "${BOLD}" "${RESET}"
    fresh_copy
    sed -i 's/\\-\\-encoding/\\-\\-forgotten/g' "${root}/man/man1/subedit-cli.1"
    expect_cli_manual_refused "--encoding missing from the man page" "MISSING GLOBAL OPTION"

    proof_header '%s▸ an option of a subcommand that its man page forgets%s\n' "${BOLD}" "${RESET}"
    fresh_copy
    sed -i 's/\\-\\-to\\-grid/\\-\\-forgotten/g' "${root}/man/man1/subedit-cli-shift.1"
    expect_cli_manual_refused "--to-grid missing from subedit-cli-shift(1)" "MAN PAGE WITHOUT OPTION"

    rm -rf "${root}"
}

expect_cli_manual_gate

# The man pages, generated and linted — #683.
#
# **Three proofs and one for green.** The pages generated from the real manual render without a
# warning (the lint does not cry wolf); a page carrying a macro groff does not know is refused by
# `check-man-pages.py`, and so is a page with a byte outside ASCII; and a page of the manual
# written outside the converter's subset — an image — makes `md-to-man.py` stop and name it, which
# is what fails the build. Everything is generated into a scratch directory, from a copy for the
# last proof: nothing of the repository is touched.
expect_man_pages_gate() {
    local generator="${REPO_ROOT}/src/scripts/md-to-man.py"
    local lint="${REPO_ROOT}/src/scripts/check-man-pages.py"
    local root output
    root="$(mktemp -d)"
    "${generator}" --manual "${REPO_ROOT}/docs/manual/subedit-cli" --output-dir "${root}/man1" \
        --version 0 --manual-dir /nowhere >/dev/null

    proof_header '%s▸ the man pages generated from the manual%s\n' "${BOLD}" "${RESET}"
    if "${lint}" "${root}/man1" >/dev/null 2>&1; then
        printf '  %s✓ « check-man-pages.py » let the generated pages through, as expected%s\n' \
            "${GREEN}" "${RESET}"
    else
        printf '  %s✗ the lint refuses the pages generated from the manual%s\n' "${RED}" "${RESET}"
        failures=$((failures + 1))
    fi

    expect_man_lint_refused() {
        local label="$1" expected="$2"
        if output="$("${lint}" "${root}/man1" 2>&1)"; then
            printf '  %s✗ the lint let it through: %s%s\n' "${RED}" "${label}" "${RESET}"
            failures=$((failures + 1))
        elif [[ "${output}" == *"${expected}"*"subedit-cli-shift.1"* && "${output}" == *"problem(s) in"* ]]; then
            printf '  %s✓ « check-man-pages.py » refused, and named the page: %s%s\n' \
                "${GREEN}" "${label}" "${RESET}"
        else
            printf '  %s✗ the lint refused, but not for the right reason: %s%s\n' \
                "${RED}" "${label}" "${RESET}"
            failures=$((failures + 1))
        fi
    }

    proof_header '%s▸ a man page with a macro groff does not know%s\n' "${BOLD}" "${RESET}"
    cp "${root}/man1/subedit-cli-shift.1" "${root}/shift.1"
    printf '.UNKNOWN macro\n' >> "${root}/man1/subedit-cli-shift.1"
    expect_man_lint_refused "an unknown macro" "WARNING"

    proof_header '%s▸ a man page with a byte outside ASCII%s\n' "${BOLD}" "${RESET}"
    cp "${root}/shift.1" "${root}/man1/subedit-cli-shift.1"
    printf 'd\xc3\xa9cale\n' >> "${root}/man1/subedit-cli-shift.1"
    expect_man_lint_refused "a UTF-8 letter" "NOT ASCII"

    proof_header '%s▸ a page of the manual outside the subset the converter handles%s\n' \
        "${BOLD}" "${RESET}"
    cp -r "${REPO_ROOT}/docs/manual/subedit-cli" "${root}/manual"
    printf '\n![a picture](shift.png)\n' >> "${root}/manual/shift.md"
    if output="$("${generator}" --manual "${root}/manual" --output-dir "${root}/copy" \
        --version 0 --manual-dir /nowhere 2>&1)"; then
        printf '  %s✗ the converter let an image through%s\n' "${RED}" "${RESET}"
        failures=$((failures + 1))
    elif [[ "${output}" == *"shift.md"*"an image"* ]]; then
        printf '  %s✓ « md-to-man.py » refused, and named the page and the construct%s\n' \
            "${GREEN}" "${RESET}"
    else
        printf '  %s✗ the converter refused, but not for the right reason%s\n' "${RED}" "${RESET}"
        failures=$((failures + 1))
    fi

    rm -rf "${root}"
}

expect_man_pages_gate

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

# The manual checks read the vocabulary of the page's language — #657.
#
# **Two proofs per check, and the same page for both**: a Spanish page of a few lines, under
# `docs/i18n/es/manual/`, is accepted when a Spanish table is supplied, and **refused, saying so,
# when it is not**. The defect is silence: a check that looks for "Shortcut" on a page that
# writes "Atajo" finds no table, and says nothing.
# The fixture is in a language with no shipped table on purpose: English and French have theirs
# since #671, and the proof is about any language that does not, whichever it is.
# Both programs are fake binaries that declare just enough to make the page agree, so that no
# other mismatch mixes with the only one looked for.
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
    for name in gui cli; do
        script="${REPO_ROOT}/src/scripts/check-${name}-manual.py"
        manual="${tree}/subedit-${name}"
        binary="${root}/fake-${name}"
        counted="1 difference(s)"
        [[ "${name}" == cli ]] && counted="1 mismatch(es)"

        proof_header '%s▸ a Spanish page, with its vocabulary table (%s)%s\n' "${BOLD}" "${name}" "${RESET}"
        if "${script}" --binary "${binary}" --manual "${manual}" --vocabulary "${table}" >/dev/null 2>&1; then
            printf '  %s✓ « check-%s-manual.py » read the Spanish page, as expected%s\n' "${GREEN}" "${name}" "${RESET}"
        else
            printf '  %s✗ the check refuses a Spanish page that has its table%s\n' "${RED}" "${RESET}"
            failures=$((failures + 1))
        fi

        proof_header '%s▸ the same page, without a vocabulary table (%s)%s\n' "${BOLD}" "${name}" "${RESET}"
        if output="$("${script}" --binary "${binary}" --manual "${manual}" 2>&1)"; then
            printf '  %s✗ the check let through a page whose language it does not know%s\n' "${RED}" "${RESET}"
            failures=$((failures + 1))
        elif [[ "${output}" == *"LANGUAGE WITHOUT VOCABULARY"* && "${output}" == *"${counted}"* ]]; then
            printf '  %s✓ « check-%s-manual.py » refused, and named this mismatch alone: the language without a table%s\n' \
                "${GREEN}" "${name}" "${RESET}"
        else
            printf '  %s✗ the check refused, but not for the right reason%s\n' "${RED}" "${RESET}"
            failures=$((failures + 1))
        fi
    done

    rm -rf "${root}"
}

expect_vocabulary_gate

# The installation rules forget a data file — #239.
#
# **The injection reduces the rules to the binaries alone**, rather than
# appending text at the end of the file: the defect here is not copying
# something, and nothing appended at the end of the file undoes an `install()`
# written higher up.
#
# A single proof and not two, unlike the neighbouring gates: this check has no
# failure mode by excess. It does not compare two readings and reports nothing
# that pre-exists — it installs into a fresh prefix on every run, so it cannot
# cry wolf about the state of the machine.
expect_installation_gate() {
    proof_header '%s▸ install() rules that forget the manual%s\n' "${BOLD}" "${RESET}"

    cat > "${INSTALLATION_SOURCE}" <<'BROKEN'
include(GNUInstallDirs)
install(TARGETS subedit-cli subedit-gui RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")
BROKEN

    if make -C "${REPO_ROOT}" --no-print-directory install-check >/dev/null 2>&1; then
        printf '  %s✗ the « install-check » gate let the missing manual through%s\n' \
            "${RED}" "${RESET}"
        failures=$((failures + 1))
    else
        printf '  %s✓ « make install-check » failed, as expected%s\n' "${GREEN}" "${RESET}"
    fi

    restore
}

expect_installation_gate

# The installation rules forget the correction patterns — #498.
#
# **The manual, for its part, is installed**: that is what sets this proof apart
# from the previous one, which removes everything but the binaries and would
# therefore say nothing of the patterns check — the gate would already fail on
# the manual. Here the manual passes, and only this check can refuse.
expect_installation_gate_without_patterns() {
    proof_header '%s▸ install() rules that forget the correction patterns%s\n' "${BOLD}" "${RESET}"

    cat > "${INSTALLATION_SOURCE}" <<'BROKEN'
include(GNUInstallDirs)
install(TARGETS subedit-cli subedit-gui RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")
install(
    DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/docs/manual/"
    DESTINATION "${CMAKE_INSTALL_DATADIR}/subedit/manual")
BROKEN

    if make -C "${REPO_ROOT}" --no-print-directory install-check >/dev/null 2>&1; then
        printf '  %s✗ the « install-check » gate let the missing patterns through%s\n' \
            "${RED}" "${RESET}"
        failures=$((failures + 1))
    else
        printf '  %s✓ « make install-check » failed, as expected%s\n' "${GREEN}" "${RESET}"
    fi

    restore
}

expect_installation_gate_without_patterns

# The language catalogues that no rule installs — #659.
#
# **They live in `cmake/Translations.cmake` and not in `Installation.cmake`**, so the previous
# proofs, which rewrite the latter, do not touch them — and none of them says anything of a
# catalogue. Here the installation stays intact (manual, patterns), and only the catalogues check
# can refuse. The defect removes the `install()` rule of the `.mo`, as a language added to
# `LINGUAS` by someone who builds without installing would.
expect_installation_gate_without_catalogues() {
    proof_header '%s▸ install() rules that forget the language catalogues%s\n' "${BOLD}" "${RESET}"

    sed -i '/^    install(FILES "\${mo}"/,+1d' "${TRANSLATIONS_SOURCE}"
    if cmp -s "${TRANSLATIONS_SOURCE}" "${backup_dir}/Translations.cmake"; then
        printf '  %s✗ the injection removed nothing: the install() rule has changed shape%s\n' "${RED}" "${RESET}"
        failures=$((failures + 1))
        restore
        return
    fi

    if make -C "${REPO_ROOT}" --no-print-directory install-check >/dev/null 2>&1; then
        printf '  %s✗ the « install-check » gate let the missing catalogues through%s\n' \
            "${RED}" "${RESET}"
        failures=$((failures + 1))
    else
        printf '  %s✓ « make install-check » failed, as expected%s\n' "${GREEN}" "${RESET}"
    fi

    restore
}

expect_installation_gate_without_catalogues

# A message template that is no longer what the code produces — #659. A new literal passed to
# `translate` and a template nobody rewrote: that is the line the translator would never see.
# The expected text is what `update-pot.sh --check` prints.
expect_gate_closes \
    "stale message template" \
    "pot-check" \
    "${LIB_SOURCE}" \
    'namespace subedit::core { const char* probeForTheProof() { return translate("injected message").data(); } }' \
    "is stale"

# A desktop file that its validation refuses — #244.
#
# **A proof distinct from the previous one, and not a repeat.** That one proves
# that a missing file is seen; this one proves that the validation is one. A
# `.desktop` that is present, installed and invalid would pass the first without
# being seen: the check would count a file where it has to read one.
#
# The injected defect is the most discreet the format knows: a `Categories=`
# with a value that does not exist. The file stays readable, the desktop shows
# it, and `desktop-file-validate` is the only thing that says so.
expect_desktop_validation_gate() {
    proof_header '%s▸ a .desktop file that its validation refuses%s\n' "${BOLD}" "${RESET}"

    printf 'Categories=ACategoryThatDoesNotExist;\n' >> "${DESKTOP_SOURCE}"

    if make -C "${REPO_ROOT}" --no-print-directory install-check >/dev/null 2>&1; then
        printf '  %s✗ the « install-check » gate let the invalid .desktop through%s\n' \
            "${RED}" "${RESET}"
        failures=$((failures + 1))
    else
        printf '  %s✓ « make install-check » failed, as expected%s\n' "${GREEN}" "${RESET}"
    fi

    restore
}

expect_desktop_validation_gate

# An icon the desktop could not read — #260.
#
# **The injected defect is the one that was shipped**, to the letter: a comment
# before the root element, which pushes « <svg » beyond the 256 bytes where
# gdk-pixbuf looks for the signature of an SVG. The file stays valid XML,
# Inkscape and Qt open it without a word, and the desktop shows an empty tile.
#
# **Two proofs**, for the two halves of the original defect: the file no desktop
# recognises, and the perfectly valid image that cannot be seen on a dark
# background. The second is the one people forget, because it does not look
# like a failure.
expect_icon_gates() {
    proof_header '%s▸ an icon that gdk-pixbuf does not recognise%s\n' "${BOLD}" "${RESET}"

    # Three hundred characters before the root: beyond the 256-byte window.
    {
        printf '<?xml version="1.0" encoding="UTF-8"?>\n<!-- '
        printf 'x%.0s' {1..300}
        printf ' -->\n'
        tail -n +2 "${backup_dir}/subedit.svg"
    } > "${ICON_SOURCE}"

    if make -C "${REPO_ROOT}" --no-print-directory install-check >/dev/null 2>&1; then
        printf '  %s✗ the « install-check » gate let the unreadable icon through%s\n' \
            "${RED}" "${RESET}"
        failures=$((failures + 1))
    else
        printf '  %s✓ « make install-check » failed, as expected%s\n' "${GREEN}" "${RESET}"
    fi

    restore

    proof_header '%s▸ an icon that cannot be seen on a dark background%s\n' "${BOLD}" "${RESET}"

    cat > "${ICON_SOURCE}" <<'INVISIBLE'
<?xml version="1.0" encoding="UTF-8"?>
<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 64 64" width="64" height="64">
  <rect x="4" y="10" width="56" height="44" rx="5" fill="#2d3646"/>
  <rect x="8" y="14" width="48" height="36" rx="2" fill="#1b2230"/>
</svg>
INVISIBLE

    if make -C "${REPO_ROOT}" --no-print-directory install-check >/dev/null 2>&1; then
        printf '  %s✗ the « install-check » gate let the invisible icon through%s\n' \
            "${RED}" "${RESET}"
        failures=$((failures + 1))
    else
        printf '  %s✓ « make install-check » failed, as expected%s\n' "${GREEN}" "${RESET}"
    fi

    restore
}

expect_icon_gates

# An `.rpm` that claims the distribution's directories — #266.
#
# **The injection reproduces the historical defect identically**: the exclusion
# list written as relative paths, `share/applications` instead of
# `/usr/share/applications`. CPack compares package paths, hence absolute ones,
# and then finds no match — the exclusion exists, it is readable, and it
# excludes nothing.
#
# That is what makes this proof necessary rather than decorative. The defect
# showed neither in the diff, nor in `rpm -qlp`, nor anywhere before a real
# Fedora refused the transaction — all that time the line was there, and the
# only thing one could have verified is what this check verifies.
expect_rpm_directory_gate() {
    proof_header '%s▸ an .rpm that claims the distribution directories%s\n' "${BOLD}" "${RESET}"

    sed -i 's|"${CPACK_PACKAGING_INSTALL_PREFIX}/|"|g' "${PACKAGING_SOURCE}"

    if make -C "${REPO_ROOT}" --no-print-directory install-check >/dev/null 2>&1; then
        printf '  %s✗ the « install-check » gate let the shared directories through%s\n' \
            "${RED}" "${RESET}"
        failures=$((failures + 1))
    else
        printf '  %s✓ « make install-check » failed, as expected%s\n' "${GREEN}" "${RESET}"
    fi

    restore
}

expect_rpm_directory_gate

# A proof of a third kind. prune-runs.sh is not a gate: it refuses nothing, it
# chooses. What can be wrong with it is therefore not letting a defect through,
# but deleting a run that had to be kept — and that error is irreversible,
# GitHub does not give a deleted run back.
#
# It cannot be exercised against GitHub. A personal token gets 403 on Actions,
# and making a proof depend on the network would turn it red on a day of outage,
# for a reason foreign to the repository. Hence --input, which gives the script
# its list instead of having it fetch one: the selection is a function of the
# list, so it can be shown on a hand-written list.
#
# The set covers the five cases that decide the outcome:
#
#   ids 1 to 35   "ci", completed        the 30 most recent stay, the 5
#                                        oldest go
#   id 99         "ci", main, the        goes: "the most recent on main",
#                 oldest of all          singular, does not spare the others
#   id 100        "ci", main, old        stays: it is the most recent on main
#   id 101        "ci", in progress      stays: GitHub refuses to delete an
#                                        unfinished run
#   ids 200 to 202 "pull request"        nothing goes: fewer than 30
#
# Case 99 is the one that matters most. Without it, "keep the most recent on
# main" would read "keep all those on main" — and would bring back the
# forty-seven runs of the push trigger removed in #106, which this pruning
# exists precisely to erase.
expect_prune_selection_holds() {
    local fixture
    local expected="1 2 3 4 5 99"
    local actual

    proof_header '%s▸ %s%s\n' "${BOLD}" "choice of the runs to delete" "${RESET}"

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

    # The status is collected rather than endured: under `set -e`, a missing
    # script or one that dies would stop the harness instead of counting a
    # failed proof, and the following proofs would never run.
    local status=0
    actual="$("${PRUNE_SCRIPT}" --dry-run --input "${fixture}" | sort -n | tr '\n' ' ')" \
        || status=$?
    actual="${actual% }"
    rm -f "${fixture}"

    if (( status != 0 )); then
        printf '  %s✗ the selection died, status %d%s\n' "${RED}" "${status}" "${RESET}"
        failures=$((failures + 1))
    elif [[ "${actual}" == "${expected}" ]]; then
        printf '  %s✓ the selection gives « %s », as expected%s\n' "${GREEN}" "${actual}" "${RESET}"
    else
        printf '  %s✗ the selection gives « %s », expected « %s »%s\n' \
            "${RED}" "${actual}" "${expected}" "${RESET}"
        failures=$((failures + 1))
    fi
}

expect_prune_selection_holds

# Of the same kind as the previous one, for releases — issue #232. Deleting a
# release takes its packages with it, and GitHub does not give them back.
#
# The set covers what decides the outcome:
#
#   v0.9.0, v0.10.0   minors           always stay
#   v0.9.3, v0.9.12   past patches     go — and 0.9.12 compared as a number
#   v0.10.2, v0.10.9  current patches  stay: the current milestone is 0.10,
#                                      which "0.9" would precede as text
#   v1.0.0-rc, notes  malformed        neither counted nor touched
#
# The second pass pushes a higher minor, v0.11.0: that is the event that tips
# the 0.10 patches into the past.
expect_release_prune_selection_holds() {
    proof_header '%s▸ %s%s\n' "${BOLD}" "choice of the releases to delete" "${RESET}"

    local fixture
    fixture="$(mktemp)"
    local status=0
    local actual

    printf '%s\n' v0.9.0 v0.9.3 v0.9.12 v0.10.0 v0.10.2 v0.10.9 v1.0.0-rc notes > "${fixture}"
    actual="$("${RELEASE_PRUNE_SCRIPT}" --dry-run --input "${fixture}" 2>/dev/null \
        | sort | tr '\n' ' ')" || status=$?
    check_release_selection "milestone 0.10 in progress" "${status}" "${actual% }" "v0.9.12 v0.9.3"

    status=0
    printf '%s\n' v0.11.0 >> "${fixture}"
    actual="$("${RELEASE_PRUNE_SCRIPT}" --dry-run --input "${fixture}" 2>/dev/null \
        | sort | tr '\n' ' ')" || status=$?
    check_release_selection "v0.11.0 published" "${status}" "${actual% }" \
        "v0.10.2 v0.10.9 v0.9.12 v0.9.3"

    rm -f "${fixture}"
}

check_release_selection() {
    local label="$1" status="$2" actual="$3" expected="$4"

    if (( status != 0 )); then
        printf '  %s✗ %s: the selection died, status %d%s\n' "${RED}" "${label}" "${status}" "${RESET}"
        failures=$((failures + 1))
    elif [[ "${actual}" == "${expected}" ]]; then
        printf '  %s✓ %s: « %s », as expected%s\n' "${GREEN}" "${label}" "${actual}" "${RESET}"
    else
        printf '  %s✗ %s: « %s », expected « %s »%s\n' \
            "${RED}" "${label}" "${actual}" "${expected}" "${RESET}"
        failures=$((failures + 1))
    fi
}

expect_release_prune_selection_holds

# Three more proofs on the same script — issue #399. The previous one tests what
# the script chooses; these test what it does with its choice, and that requires
# letting the script delete for real, against a `gh` that is not the real one.
#
# **The makeshift `gh` is the delicate part, and it touches nothing.** It is put
# at the head of the `PATH` from a single-use directory; it answers
# `gh api …/releases` with a hand-written file — and applies the script's real
# `--jq` filter, with `jq`, so that drafts are discarded by the code under test
# and not by the fixture — and `gh release delete` by writing the call to a log,
# refusing the ones it is told to. Any other command is logged as "UNEXPECTED"
# and fails. The harness refuses to launch anything if `gh` does not resolve to
# it: none of these proofs can therefore reach the remote repository, and the
# token passed is a fake.
#
# All three run under `GITHUB_EVENT_NAME=push`, the real event of a release: a
# `workflow_dispatch` guard that fired wrongly would make the first two fail.
make_fake_gh() {
    local sandbox="$1"

    mkdir -p "${sandbox}/bin"
    : > "${sandbox}/calls.log"
    cat > "${sandbox}/bin/gh" <<'FIN'
#!/usr/bin/env bash
# A makeshift gh: it reads a file, it writes a log, it goes nowhere.
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
                printf 'HTTP 403: deletion refused\n' >&2
                exit 1
            fi
        done
        ;;
    *)
        printf 'UNEXPECTED %s\n' "$*" >> "${log}"
        exit 99
        ;;
esac
FIN
    chmod +x "${sandbox}/bin/gh"
}

# Writes the API's list of releases: `tag` for a published release,
# `tag:draft` for a draft.
write_fake_releases() {
    local sandbox="$1"
    shift

    jq -n '[$ARGS.positional[] | split(":")
            | {tag_name: .[0], draft: (.[1] == "draft")}]' \
        --args "$@" > "${sandbox}/releases.json"
}

# Runs the real script against the makeshift gh. The exit status goes to
# `prune_status`, the outputs to `${sandbox}/out` and `${sandbox}/err`. The
# arguments after the directory are environment assignments.
prune_status=0
run_release_pruner() {
    local sandbox="$1"
    shift

    if [[ "$(PATH="${sandbox}/bin:${PATH}" command -v gh)" != "${sandbox}/bin/gh" ]]; then
        printf '  %s✗ gh does not resolve to the makeshift gh: nothing is run%s\n' \
            "${RED}" "${RESET}"
        prune_status=99
        return
    fi

    prune_status=0
    env -u GITHUB_STEP_SUMMARY \
        PATH="${sandbox}/bin:${PATH}" \
        GH_TOKEN=makeshift-token \
        GITHUB_REPOSITORY=example/subedit \
        FAKE_GH_DIR="${sandbox}" \
        "$@" \
        "${RELEASE_PRUNE_SCRIPT}" >"${sandbox}/out" 2>"${sandbox}/err" || prune_status=$?
}

# The tags whose deletion was attempted, in order, separated by a space.
attempted_deletions() {
    awk '$1 == "release" && $2 == "delete" { print $3 }' "$1/calls.log" | tr '\n' ' '
}

# Writes the verdict of a case: `problems` is empty when it holds.
report_release_prune_case() {
    local label="$1" problems="$2"

    if [[ -z "${problems}" ]]; then
        printf '  %s✓ %s, as expected%s\n' "${GREEN}" "${label}" "${RESET}"
    else
        printf '  %s✗ %s: %s%s\n' "${RED}" "${label}" "${problems}" "${RESET}"
        failures=$((failures + 1))
    fi
}

# A refusal is red, and it does not excuse the following deletions.
#
# Two of the three past patches are refused, the second of the three is not: a
# script that stopped at the first refusal would have attempted neither the
# second nor the third, and a script that exited with 0 would turn green a
# pruning that is not one. The same set holds the two guards of #232: `v0.9.0`
# is a minor and must never be attempted, and no call carries `--cleanup-tag`,
# which would take the tag away with the release.
expect_release_prune_fails_on_refusal() {
    proof_header '%s▸ %s%s\n' "${BOLD}" "release pruning where one deletion is refused" "${RESET}"

    local sandbox problems=""
    sandbox="$(mktemp -d)"
    make_fake_gh "${sandbox}"
    write_fake_releases "${sandbox}" v0.9.0 v0.9.3 v0.9.5 v0.9.7 v0.10.0 v0.10.2
    run_release_pruner "${sandbox}" GITHUB_EVENT_NAME=push FAKE_GH_REFUSE="v0.9.3 v0.9.7"

    local attempted
    attempted="$(attempted_deletions "${sandbox}")"
    [[ "${attempted}" == "v0.9.3 v0.9.5 v0.9.7 " ]] \
        || problems+="deletions attempted « ${attempted% } », expected « v0.9.3 v0.9.5 v0.9.7 »; "
    (( prune_status != 0 )) || problems+="exited with 0 despite two refusals; "
    grep -q 'v0.9.3' "${sandbox}/err" || problems+="the refusal of v0.9.3 is not named; "
    grep -q 'v0.9.7' "${sandbox}/err" || problems+="the refusal of v0.9.7 is not named; "
    ! grep -q 'v0.9.5' "${sandbox}/err" || problems+="v0.9.5, deleted, is named as refused; "
    ! grep -q -e '--cleanup-tag' "${sandbox}/calls.log" \
        || problems+="a call carries --cleanup-tag, which would delete the tag; "

    report_release_prune_case "a refusal makes it exit with an error, the other deletions happen" "${problems}"
    rm -rf "${sandbox}"
}

expect_release_prune_fails_on_refusal

# A draft is not a version. `gh api …/releases` returns drafts with the others;
# a `v0.11.0` draft created by hand would make 0.11 the current milestone, and
# the `v0.10.2` of the real current milestone would go with its package. The
# `v0.9.5` draft, for its part, is a past patch: it must not be deleted either,
# since it is not a release.
expect_release_prune_ignores_drafts() {
    proof_header '%s▸ %s%s\n' "${BOLD}" "release pruning with drafts present" "${RESET}"

    local sandbox problems=""
    sandbox="$(mktemp -d)"
    make_fake_gh "${sandbox}"
    write_fake_releases "${sandbox}" \
        v0.9.0 v0.9.3 v0.9.5:draft v0.10.0 v0.10.2 v0.11.0:draft
    run_release_pruner "${sandbox}" GITHUB_EVENT_NAME=push

    local attempted
    attempted="$(attempted_deletions "${sandbox}")"
    [[ "${attempted}" == "v0.9.3 " ]] \
        || problems+="deletions attempted « ${attempted% } », expected « v0.9.3 »; "
    (( prune_status == 0 )) || problems+="exit status ${prune_status}, expected 0; "

    report_release_prune_case "a draft is neither a milestone nor a release to delete" "${problems}"
    rm -rf "${sandbox}"
}

expect_release_prune_ignores_drafts

# A rebuild by hand does not prune. `release.yml` offers `workflow_dispatch`
# for a package to redo; on a patch of a closed milestone, the publication
# recreates the release and the pruning would delete it in the same job. The
# set is that of the first case, where three patches would go.
expect_release_prune_skips_manual_rebuild() {
    proof_header '%s▸ %s%s\n' "${BOLD}" "release pruning during an on-demand rebuild" "${RESET}"

    local sandbox problems=""
    sandbox="$(mktemp -d)"
    make_fake_gh "${sandbox}"
    write_fake_releases "${sandbox}" v0.9.0 v0.9.3 v0.9.5 v0.9.7 v0.10.0 v0.10.2
    run_release_pruner "${sandbox}" GITHUB_EVENT_NAME=workflow_dispatch

    local attempted
    attempted="$(attempted_deletions "${sandbox}")"
    [[ -z "${attempted}" ]] || problems+="deletions attempted « ${attempted% } », expected none; "
    (( prune_status == 0 )) || problems+="exit status ${prune_status}, expected 0; "
    grep -q 'workflow_dispatch' "${sandbox}/out" \
        || problems+="the output does not say that it does not prune on demand; "

    report_release_prune_case "an on-demand rebuild deletes nothing and says so" "${problems}"
    rm -rf "${sandbox}"
}

expect_release_prune_skips_manual_rebuild

# The guard of the release workflow, on its two refusals that need no tag to be
# set: a name that is not a version, and a version that designates no commit.
# The third — a version that disagrees with its CMakeLists — would require
# setting a tag in the repository, which a proof has no business doing.
expect_release_tag_check_refuses() {
    proof_header '%s▸ %s%s\n' "${BOLD}" "release tag refused" "${RESET}"

    local tag
    for tag in 0.10.9 v0.10 v999.999.999; do
        if "${RELEASE_TAG_CHECK}" "${tag}" >/dev/null 2>&1; then
            printf '  %s✗ check-release-tag.sh accepted « %s »%s\n' "${RED}" "${tag}" "${RESET}"
            failures=$((failures + 1))
        else
            printf '  %s✓ « %s » refused, as expected%s\n' "${GREEN}" "${tag}" "${RESET}"
        fi
    done
}

expect_release_tag_check_refuses

# A proof of a fourth kind, in the same spirit as the previous one.
# clean-stale-coverage.sh refuses nothing either: it repairs. What can be wrong
# with it is therefore letting an inconsistent tree through — a .gcno that a
# moved source has orphaned, and that makes gcovr fail with a message naming
# neither the faulty file nor the remedy.
#
# And the symmetrical error matters as much: discarding a sound tree would cost
# a full rebuild on every measure, which nobody would notice except as an
# unexplained slowness. Both directions are therefore tested.
#
# On a make-believe tree rather than the real one: the mechanism is a comparison
# of lists, it can be shown on hand-written lists, and nothing here may depend
# on a twenty-minute build.
expect_stale_coverage_is_cleared() {
    local build
    local script="${REPO_ROOT}/src/scripts/clean-stale-coverage.sh"

    proof_header '%s▸ %s%s\n' "${BOLD}" "coverage tree made stale by a move" "${RESET}"

    build="$(mktemp -d)"
    touch "${build}/witness.gcno"

    # First pass: no record, hence nothing to compare and nothing to discard.
    local status=0
    "${script}" --build-dir "${build}" >/dev/null || status=$?

    if (( status != 0 )) || [[ ! -f "${build}/witness.gcno" ]]; then
        printf '  %s✗ the first pass discarded a tree it could not judge%s\n' \
            "${RED}" "${RESET}"
        failures=$((failures + 1))
        rm -rf "${build}"
        return
    fi

    # Second pass: nothing has moved since the record, the tree must survive.
    "${script}" --build-dir "${build}" >/dev/null || status=$?
    if [[ ! -f "${build}/witness.gcno" ]]; then
        printf '  %s✗ a sound tree was discarded%s\n' "${RED}" "${RESET}"
        failures=$((failures + 1))
        rm -rf "${build}"
        return
    fi

    # Third pass: a source of the record has disappeared, as after a move. The
    # whole tree must go, witness included.
    printf 'src/lib/subedit/core/moved.cpp\n' >> "${build}/.sources"
    "${script}" --build-dir "${build}" >/dev/null || status=$?

    if [[ -f "${build}/witness.gcno" ]]; then
        printf '  %s✗ an orphaned tree was kept%s\n' "${RED}" "${RESET}"
        failures=$((failures + 1))
    else
        printf '  %s✓ sound kept, orphaned discarded%s\n' "${GREEN}" "${RESET}"
    fi

    rm -rf "${build}"
}

expect_stale_coverage_is_cleared

# A proof in the same spirit as the previous two: record-bench.sh refuses
# nothing either, it chooses what it writes into the table of extremes. And what
# it writes there is **final** — the table is never pruned — so a value set by
# noise makes the measure blind to any regression smaller than that noise.
#
# The defect it closes was found in use, in phase 6: a new measure set its two
# extremes at the first record that carried it, **even a dirty one**. Having
# nothing to compare against, the load threshold rule had nothing to refuse, and
# therefore never applied to what had just been born.
#
# On a disposable journal rather than the repository's: what the script decides
# is a function of the journal it reads, hence --journal, as prune-runs.sh has
# --input.
expect_bench_extremes_hold() {
    local script="${REPO_ROOT}/src/scripts/record-bench.sh"
    local work
    local journal
    local status=0

    proof_header '%s▸ %s%s\n' "${BOLD}" "choice of the extremes of the measurement journal" "${RESET}"

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
        '| older | 100 ns | 0.0.1 — 2020-01-01 | 300 ns | 0.0.1 — 2020-01-01 |' '' \
        '<!-- older min=100.0 max=300.0 -->' '' \
        '## Records' '' '<!-- records -->' '' > "${journal}"

    # First pass, busy machine: the new measure goes into the record and
    # **not** into the table.
    write_bench_xml "fresh" 1000
    "${script}" --xml "${work}/run.xml" --mode Release --load 9 --below 1.5 \
        --journal "${journal}" >/dev/null 2>&1 || status=$?
    # The raw comment, and not the table row: the current record also carries
    # a « | fresh | … » row, and looking for it there would amount to checking
    # that the measure is absent from the journal — which it must not be. The
    # raw comments exist only in the table of extremes.
    if (( status != 0 )) || grep -q '<!-- fresh min=' "${journal}"; then
        printf '  %s✗ a new measure set its extremes on a dirty record%s\n' \
            "${RED}" "${RESET}"
        failures=$((failures + 1))
        rm -rf "${work}"
        return
    fi

    # Second pass, quiet machine: it goes in.
    write_bench_xml "fresh" 800
    "${script}" --xml "${work}/run.xml" --mode Release --load 0.5 --below 1.5 \
        --journal "${journal}" >/dev/null 2>&1 || status=$?
    if ! grep -q '<!-- fresh min=800.0 max=800.0 -->' "${journal}"; then
        printf '  %s✗ a quiet record did not set the extremes of a new measure%s\n' \
            "${RED}" "${RESET}"
        failures=$((failures + 1))
        rm -rf "${work}"
        return
    fi

    # Third pass, same version, quiet, **worse**: the extreme this record
    # replaces no longer has a source, so it is taken back — otherwise the table
    # would quote, for a version present in the journal, a figure the journal no
    # longer shows.
    write_bench_xml "fresh" 950
    "${script}" --xml "${work}/run.xml" --mode Release --load 0.5 --below 1.5 \
        --journal "${journal}" >/dev/null 2>&1 || status=$?
    if ! grep -q '<!-- fresh min=950.0 max=950.0 -->' "${journal}"; then
        printf '  %s✗ an extreme set by the replaced record was not taken back%s\n' \
            "${RED}" "${RESET}"
        failures=$((failures + 1))
        rm -rf "${work}"
        return
    fi

    # And the envelope of another version does not move for all that.
    if ! grep -q '<!-- older min=100.0 max=300.0 -->' "${journal}"; then
        printf '  %s✗ the envelope of another version was touched%s\n' \
            "${RED}" "${RESET}"
        failures=$((failures + 1))
        rm -rf "${work}"
        return
    fi

    # **The second criterion: dispersion** — issue #202. The load reads the
    # machine, not the measure. A record taken at load 1.44 — hence admitted —
    # set a final maximum on a ticket that changed no `.cpp`, with a standard
    # deviation worth 26 % of its mean where that measure stays between 3 and
    # 8 %.
    #
    # The history of the disposable journal is written at 5 % dispersion over six
    # records, which gives a threshold of 6.25 %: the record below is quiet and
    # six times more dispersed than usual.
    printf '%s\n' \
        '# Journal' '' '<!-- extremes -->' '' \
        '| Measure | Minimum | Recorded on | Maximum | Recorded on |' \
        '| :----- | ------: | :-------- | ------: | :-------- |' \
        '| stable | 1000 ns | 0.0.1 — 2020-01-01 | 1000 ns | 0.0.1 — 2020-01-01 |' '' \
        '<!-- stable min=1000.0 max=1000.0 -->' '' \
        '## Records' '' '<!-- records -->' '' > "${journal}"
    local pass
    for pass in 1 2 3 4 5 6; do
        printf '%s\n' \
            "### 0.0.${pass} — $(date +%F) — Release — load 0.5 — pace ×1.00" '' \
            '| Measure | Mean | Standard deviation |' '| :----- | ------: | ---------: |' \
            '| stable | 1000 ns | 50 ns |' '' >> "${journal}"
    done

    write_bench_xml_with_sd "stable" 2000 520
    "${script}" --xml "${work}/run.xml" --mode Release --load 0.5 --below 1.5 \
        --journal "${journal}" >/dev/null 2>&1 || status=$?
    if grep -q '<!-- stable min=1000.0 max=2000.0 -->' "${journal}"; then
        printf '  %s✗ a record six times more dispersed than usual set its maximum%s\n' \
            "${RED}" "${RESET}"
        failures=$((failures + 1))
        rm -rf "${work}"
        return
    fi

    # And the counterpart: the measure still goes into the journal. What it is
    # refused is the envelope, not the history.
    if ! grep -q '| stable | 2 µs |' "${journal}"; then
        printf '  %s✗ a dispersed record was kept out of the journal, not only out of the table%s\n' \
            "${RED}" "${RESET}"
        failures=$((failures + 1))
        rm -rf "${work}"
        return
    fi

    printf '  %s✓ new measure kept out when busy, set when quiet, taken back when its source is replaced,%s\n' \
        "${GREEN}" "${RESET}"
    printf '  %s  and a measure too dispersed goes into the journal without touching the envelope%s\n' \
        "${GREEN}" "${RESET}"
    rm -rf "${work}"
}

expect_bench_extremes_hold

# A proof of the same kind, on the other half of the rule — issue #270.
#
# `await-quiet.sh` refuses nothing either: it decides whether the machine is
# quiet enough, and its exit status is what makes a record go into the journal
# or not. What can be wrong with it goes both ways — declaring quiet a machine
# that is not, or waiting for a lull that will never come.
#
# Both are shown without waiting for anything, by giving it a threshold that
# makes the answer certain: no `sleep` is played here.
expect_await_quiet_decides() {
    local script="${REPO_ROOT}/src/scripts/await-quiet.sh"
    local load
    local status

    proof_header '%s▸ %s%s\n' "${BOLD}" "the decision to measure or not" "${RESET}"

    # The script has a way out for a machine without /proc/loadavg — it returns
    # « unknown » and zero. This proof then has nothing to show, and saying so
    # is better than making it fail for a reason that is none of the
    # repository's business.
    if [[ ! -r /proc/loadavg ]]; then
        printf '  %s✓ no /proc/loadavg: the decision does not apply here%s\n' \
            "${GREEN}" "${RESET}"
        return
    fi

    # A threshold no load can exceed: it must return 0 at once, and write a
    # number.
    status=0
    load="$("${script}" --below 100000 --timeout 0)" || status=$?
    if (( status != 0 )) || [[ ! "${load}" =~ ^[0-9]+([.,][0-9]+)?$ ]]; then
        printf '  %s✗ a quiet machine was not recognised (status %s, load « %s »)%s\n' \
            "${RED}" "${status}" "${load}" "${RESET}"
        failures=$((failures + 1))
        return
    fi

    # A threshold no load can reach: it must give up, return 1, and write the
    # load all the same — that is what the caller shows.
    status=0
    load="$("${script}" --below 0 --timeout 0)" || status=$?
    if (( status == 0 )) || [[ ! "${load}" =~ ^[0-9]+([.,][0-9]+)?$ ]]; then
        printf '  %s✗ a busy machine was declared quiet (status %s, load « %s »)%s\n' \
            "${RED}" "${status}" "${load}" "${RESET}"
        failures=$((failures + 1))
        return
    fi

    printf '  %s✓ quiet recognised, busy refused, and the load written in both cases%s\n' \
        "${GREEN}" "${RESET}"
}

expect_await_quiet_decides

printf '\n'
# The two guards of the orchestrator — issue #269.
#
# **A filter that keeps nothing must refuse, not return zero.** `gate.sh` can
# resume a suite in the middle, which opens a failure mode that did not exist
# when the Makefile chained its targets: a typo in `--from` would play zero
# steps and return a success status. It is the same family as the empty scope of
# the static analysis, and it closes the same way — loudly.
expect_orchestrator_refuses() {
    local label="$1"
    shift

    proof_header '%s▸ %s%s\n' "${BOLD}" "${label}" "${RESET}"

    if "${REPO_ROOT}/src/scripts/gate.sh" "$@" >/dev/null 2>&1; then
        printf '  %s✗ gate.sh accepted « %s »%s\n' "${RED}" "$*" "${RESET}"
        failures=$((failures + 1))
    else
        printf '  %s✓ gate.sh refused, as expected%s\n' "${GREEN}" "${RESET}"
    fi
}

expect_orchestrator_refuses "unknown step passed to --from" check --from covrage
expect_orchestrator_refuses "filters that keep no step" check --only tidy --skip tidy

# The same guard, on the second script that can resume in the middle — issue #271.
#
# `check-installation.sh` was given `--only` for the reason that gave it to
# `gate.sh`: replaying twelve checks to fix one. The failure mode comes with it,
# and it is the worst of all — `--only pakages`, a misspelling of `packages`,
# would play zero checks and return zero, hence **an installation declared
# correct that nobody looked at.**
#
# The proof costs nothing: the refusal happens before the first check runs, so
# nothing is installed, nothing is packaged.
expect_installation_refuses_unknown_control() {
    proof_header '%s▸ %s%s\n' "${BOLD}" "unknown check passed to --only" "${RESET}"

    if "${REPO_ROOT}/src/scripts/check-installation.sh" --only pakages >/dev/null 2>&1; then
        printf '  %s✗ check-installation.sh accepted « pakages »%s\n' "${RED}" "${RESET}"
        failures=$((failures + 1))
    else
        printf '  %s✓ check-installation.sh refused, as expected%s\n' "${GREEN}" "${RESET}"
    fi
}

expect_installation_refuses_unknown_control

# The expected outputs of `--format json` — #556.
#
# **Six proofs, on a hand-written disposable directory**: a floating-point
# number, a line that is not JSON, an incomplete envelope, a dry run that states
# a destination, a change without « after », and green. The check takes the
# directory as an argument, so nothing of the repository is touched: a
# deliberately wrong expected output cannot stay behind.
expect_json_fixture_gate() {
    local script="${REPO_ROOT}/src/scripts/check-json-fixtures.py"
    local root
    root="$(mktemp -d)"

    proof_header '%s▸ an expected JSON output that contains a floating-point number%s\n' "${BOLD}" "${RESET}"
    printf '{"schema":1,"command":"inspect","file":"a.srt","ok":true,"ratio":1.5,"warnings":[]}\n' \
        > "${root}/bad.jsonl"
    if "${script}" --dir "${root}" >/dev/null 2>&1; then
        printf '  %s✗ the check let 1.5 through%s\n' "${RED}" "${RESET}"
        failures=$((failures + 1))
    else
        printf '  %s✓ « check-json-fixtures.py » refused, as expected%s\n' \
            "${GREEN}" "${RESET}"
    fi

    proof_header '%s▸ a line that is not JSON%s\n' "${BOLD}" "${RESET}"
    printf '{"schema":1,"command":\n' > "${root}/bad.jsonl"
    if "${script}" --dir "${root}" >/dev/null 2>&1; then
        printf '  %s✗ the check let a truncated line through%s\n' "${RED}" "${RESET}"
        failures=$((failures + 1))
    else
        printf '  %s✓ « check-json-fixtures.py » refused, as expected%s\n' \
            "${GREEN}" "${RESET}"
    fi

    proof_header '%s▸ an incomplete envelope: no « file »%s\n' "${BOLD}" "${RESET}"
    printf '{"schema":1,"command":"inspect","ok":true,"warnings":[]}\n' > "${root}/bad.jsonl"
    if "${script}" --dir "${root}" >/dev/null 2>&1; then
        printf '  %s✗ the check let the envelope without « file » through%s\n' \
            "${RED}" "${RESET}"
        failures=$((failures + 1))
    else
        printf '  %s✓ « check-json-fixtures.py » refused, as expected%s\n' \
            "${GREEN}" "${RESET}"
    fi

    proof_header '%s▸ a dry run that nevertheless states a destination%s\n' "${BOLD}" "${RESET}"
    printf '{"schema":1,"command":"shift","file":"a.srt","ok":true,"dry_run":true,"destination":"o.srt","counts":{},"warnings":[]}\n' \
        > "${root}/bad.jsonl"
    if "${script}" --dir "${root}" >/dev/null 2>&1; then
        printf '  %s✗ the check let a dry-run destination through%s\n' \
            "${RED}" "${RESET}"
        failures=$((failures + 1))
    else
        printf '  %s✓ « check-json-fixtures.py » refused, as expected%s\n' \
            "${GREEN}" "${RESET}"
    fi

    proof_header '%s▸ a change without « after »%s\n' "${BOLD}" "${RESET}"
    printf '{"schema":1,"command":"hearing-impaired","file":"a.srt","ok":true,"dry_run":true,"destination":null,"counts":{},"changes":[{"subtitle":1,"document":"main","before":"x"}],"warnings":[]}\n' \
        > "${root}/bad.jsonl"
    if "${script}" --dir "${root}" >/dev/null 2>&1; then
        printf '  %s✗ the check let a change without « after » through%s\n' \
            "${RED}" "${RESET}"
        failures=$((failures + 1))
    else
        printf '  %s✓ « check-json-fixtures.py » refused, as expected%s\n' \
            "${GREEN}" "${RESET}"
    fi

    proof_header '%s▸ a correct expected output, for green%s\n' "${BOLD}" "${RESET}"
    printf '{"schema":1,"command":"inspect","file":"a.srt","ok":true,"warnings":[]}\n' \
        > "${root}/bad.jsonl"
    if "${script}" --dir "${root}" >/dev/null 2>&1; then
        printf '  %s✓ « check-json-fixtures.py » let a correct expected output through%s\n' \
            "${GREEN}" "${RESET}"
    else
        printf '  %s✗ the check refused a correct expected output%s\n' "${RED}" "${RESET}"
        failures=$((failures + 1))
    fi

    rm -rf "${root}"
}

expect_json_fixture_gate

report_durations
# The total of a run that reached its end, whatever its verdict; one that died has none.
timing_record verify-gates "(total)" "$((SECONDS - run_started))" "${run_load}" "$(timing_load)"

if (( failures > 0 )); then
    printf '%s%d failed proof(s)%s\n' "${RED}" "${failures}" "${RESET}" >&2
    exit 1
fi
printf '%sthe gates close%s\n' "${GREEN}" "${RESET}"
printf '%sthe parallelism check lets legitimate code through%s\n' \
    "${GREEN}" "${RESET}"
printf '%sand the pruner chooses the expected runs%s\n' \
    "${GREEN}" "${RESET}"
printf '%sand the pruned releases are the patches of past milestones%s\n' \
    "${GREEN}" "${RESET}"
printf '%sand the pruning fails on a refusal, ignores drafts, and stays idle on demand%s\n' \
    "${GREEN}" "${RESET}"
printf '%sand the measurement journal sets an extreme only on a clean record%s\n' \
    "${GREEN}" "${RESET}"
printf '%sand an untracked file older than the tests raises no false alarm%s\n' \
    "${GREEN}" "${RESET}"
printf '%snor does a user configuration that nothing touched%s\n' \
    "${GREEN}" "${RESET}"
